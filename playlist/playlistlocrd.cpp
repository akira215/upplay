/* Copyright (C) 2014 J.F.Dockes
 *	 This program is free software; you can redistribute it and/or modify
 *	 it under the terms of the GNU General Public License as published by
 *	 the Free Software Foundation; either version 2 of the License, or
 *	 (at your option) any later version.
 *
 *	 This program is distributed in the hope that it will be useful,
 *	 but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	 GNU General Public License for more details.
 *
 *	 You should have received a copy of the GNU General Public License
 *	 along with this program; if not, write to the
 *	 Free Software Foundation, Inc.,
 *	 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <QDebug>
#include <QXmlSimpleReader>

#include "playlistlocrd.h"

#include "libupnpp/log.hxx"

#include "HelperStructs/MetaData.h"
#include "HelperStructs/Helper.h"

using namespace UPnPP;

PlaylistLOCRD::PlaylistLOCRD(AVTPlayer *avtp, const char *fn, QObject *parent)
    : Playlist(parent), m_avto(avtp)
{
    parseList(fn);
    
    // Connections from UPnP renderer to local playlist
    connect(m_avto, SIGNAL(newTrackPlaying(const QString&)),
            this, SLOT(onExtTrackChange(const QString&)));
    connect(m_avto, SIGNAL(sig_currentMetadata(const MetaData&)),
            this, SLOT(onCurrentMetadata(const MetaData&)));
    connect(m_avto, SIGNAL(audioStateChanged(int, const char*)),
            this, SLOT(onRemoteTpState(int, const char *)));
    connect(m_avto,  SIGNAL(connectionLost()), this, SIGNAL(connectionLost()));

    // Connections from local playlist to UPnP renderer
    connect(this, SIGNAL(sig_pause()), m_avto, SLOT(pause()));
    connect(this, SIGNAL(sig_stop()),  m_avto, SLOT(stop()));
    connect(this, SIGNAL(sig_resume_play()), m_avto, SLOT(play()));
}

void PlaylistLOCRD::onExtTrackChange(const QString&)
{
    // ??
}

void PlaylistLOCRD::onCurrentMetadata(const MetaData& md)
{
    emit sig_track_metadata(md);
}

void PlaylistLOCRD::update_state()
{
    emit sig_playlist_updated(m_meta, m_play_idx, 0);
}

void PlaylistLOCRD::psl_change_track_impl(int row) {
    if (row < 0 || row >= int(m_meta.size()))
        return;

    m_play_idx = row;
    m_meta.setCurPlayTrack(row);

    m_avto->changeTrack(m_meta[row], 0, true);
    emit sig_playing_track_changed(row);
    emit sig_track_metadata(m_meta[row]);
}

void PlaylistLOCRD::psl_play() 
{
    if (m_tpstate ==  AUDIO_STOPPED && valid_row(m_selection_min_row)) {
    } else {
        emit sig_resume_play();
    }
}

void PlaylistLOCRD::psl_pause() 
{
    emit sig_pause();
}

void PlaylistLOCRD::psl_stop() 
{
    emit sig_stop();
}


// Radiolist format. This is XML translated from the json file coming
// with MediaPlayer. There are multiple <body> elements, for different
// kind of entries which we do not distinguish. Each body has multiple
// children, which we are interested in. In each <children> element
// (should be <child> really), we get title, url, etc.
// We store the whole thing in a MetadataList
//
// <radiolist>
//   <body>
//     <key>presetUrls</key>
//     <children>
//       <text>Naim Radio</text>
//       <URL>http://37.130.228.60:8090/listen.pls</URL>
//       <image>http://d1i6vahw24eb07.cloudfront.net/s0q.png</image>
//       <preset_id>u46</preset_id>
//     </children>
//     <children>
//       ...
//     </children>
//   </body>
//   <body>
//     <key>stations</key>
//     <children>
//       <text>96.2 | The Revolution (Rock)</text>
//       <URL>http://opml.radiotime.com/Tune.ashx?id=s45508</URL>
//       <image>http://d1i6vahw24eb07.cloudfront.net/s45508q.png</image>
//       <preset_id>s45508</preset_id>
//     </children>
//     <children>
//       ...
//     </children>
//    </body>
// </radiolist>

// Handler for parsing saved simple search data
class RadioListXMLHandler : public QXmlDefaultHandler {
public:
    RadioListXMLHandler(MetaDataList *mdv)
        : m_mdv(mdv) {
        resetTemps();
        m_mdv->clear();
    }
    bool endElement(const QString & /* namespaceURI */,
		    const QString & /* localName */,
		    const QString &qName) {
        if (!qName.compare("children")) {
            MetaData md;
            md.title = m_ttext;
            // Not too sure what this subtext field means, but it does appear
            // to be current info, which we don't update, so replace with dup
            // of text like mediaplayer does.
            //md.artist = m_tsubtext.isEmpty() ? m_ttext : m_tsubtext;
            md.artist = QString::fromUtf8("[ ") +  m_ttext + " ]";
            md.filepath = m_turl;
            md.albumArtURI = m_timgurl;
            md.track_num = m_tpreset;
            //qDebug() << "parseRadioList: pushing tt " << md.title <<
            //" url " << md.filepath << " art " << m_timgurl << " preset " <<
            //m_tpreset;
            m_mdv->push_back(md);
            resetTemps();
        } else if (!qName.compare("text")) {
            m_ttext = m_chars.trimmed();
            m_chars.clear();
        } else if (!qName.compare("subtext")) {
            m_tsubtext = m_chars.trimmed();
            m_chars.clear();
        } else if (!qName.compare("URL")) {
            m_turl = m_chars.trimmed();
            m_chars.clear();
        } else if (!qName.compare("image")) {
            m_timgurl = m_chars.trimmed();
            m_chars.clear();
        } else if (!qName.compare("preset_id")) {
            // 1st char is a flag
            string s = qs2utf8s(m_chars.trimmed());
            if (s.size() > 1)
                m_tpreset = atoi(&(s.c_str()[1]));
            m_chars.clear();
        } else {
            m_chars.clear();
        }

        return true;
    }

    bool characters(const QString &str) {
        m_chars += str;
        return true;
    }

private:
    // The object we set up
    MetaDataList *m_mdv;
    void resetTemps() {
        m_chars.clear();
        m_ttext.clear();
        m_tsubtext.clear();
        m_turl.clear();
        m_timgurl.clear();
        m_tpreset = -1;
    }

    // Temporary data while parsing.
    QString m_chars;

    QString m_ttext;
    QString m_tsubtext;
    QString m_turl;
    QString m_timgurl;
    int     m_tpreset;
};

bool PlaylistLOCRD::parseList(const char *fnm)
{
    QByteArray data = Helper::readFileToByteArray(fnm);
    if (data.isEmpty()) {
        return false;
    }

    RadioListXMLHandler handler(&m_meta);
    QXmlSimpleReader reader;
    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);

    QXmlInputSource xmlInputSource;
    xmlInputSource.setData(QString::fromUtf8(data.data()));

    if (!reader.parse(xmlInputSource)) {
        qDebug() << "PlaylistLOCRD:: could not parse radio list data";
        return false;
    }
    return true;
}

