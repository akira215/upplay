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
#ifndef _OHPLADAPT_H_INCLUDED_
#define _OHPLADAPT_H_INCLUDED_
#include <string>
#include <iostream>
using namespace std;

#include <QDebug>
#include <QApplication>

#include "HelperStructs/MetaData.h"
#include "HelperStructs/globals.h"
#include "HelperStructs/PlaylistMode.h"

#include "upqo/ohplaylist_qo.h"
#include "upadapt/upputils.h"

using namespace UPnPP;

// Note: can't call this OHPlaylist because UPnPClient has it too (and we
// sometimes use the unqualified name)
class OHPlayer : public OHPlaylistQO {
Q_OBJECT

public:
    OHPlayer(UPnPClient::OHPLH ohpl, QObject *parent = 0)
        : OHPlaylistQO(ohpl, parent), m_songsecs(-1),
          m_ininsert(false) {
        connect(this, SIGNAL(trackArrayChanged()),
                this, SLOT(translateMetaData()));
        connect(this, SIGNAL(tpStateChanged(int)), 
                this, SLOT(playerState(int)));
        connect(this, SIGNAL(currentTrackId(int)),
                this, SLOT(onTrackIdChanged(int)));
        connect(this, SIGNAL(shuffleChanged(bool)), 
                this, SLOT(onShuffleState(bool)));
        connect(this, SIGNAL(repeatChanged(bool)), 
                this, SLOT(onRepeatState(bool)));
    }

public slots:

    // Seek to time in seconds
    void seek(int secs) {
        if (m_songsecs == -1) {
            std::unordered_map<int, UPnPClient::UPnPDirObject>::iterator
                poolit = m_metapool.find(m_curid);
            if (poolit != m_metapool.end()) {
                UPnPClient::UPnPDirObject& ude = poolit->second;
                std::string sval;
                if (ude.getrprop(0, "duration", sval)) {
                    m_songsecs = UPnPP::upnpdurationtos(sval);
                }
            }
        }

        if (m_songsecs == -1) {
            return;
        }
        seekSecondAbsolute(secs);
    }

    // The duration is sometimes not set in the didl metadata and the
    // playlist app object may be able to retrieve it from ohtime. We
    // then get the info through this method.
    void setSongSecs(int secs) {
        m_songsecs = secs;
    }
    
    // Insert after idx
    void insertTracks(const MetaDataList& meta, int idx) {
        while (m_ininsert) {
            qDebug() << "OHPlayer::insertTracks: already active";
            return;
        }
        m_ininsert = true;

        asyncArrayUpdates(false);
        qDebug() << "OHPlayer::insertTracks: afteridx " << idx << 
            " idsv size " << m_idsv.size();
        if (idx < -1 || idx >= int(m_idsv.size())) {
            qDebug() << "OHPlayer::insertTracks: BAD AFTERIDX " << idx << 
                " idsv size " << m_idsv.size();
            if (idx > 0) {
                idx = int(m_idsv.size()-1);
            } else {
                m_ininsert = false;
                return;
            }
        }
        int afterid = idx == -1 ? 0 : m_idsv[idx];
        int counter = 0;
        for (vector<MetaData>::const_iterator it = meta.begin();
             it != meta.end(); it++) {
            if (counter++ > 10) {
                sync();
                qApp->processEvents();
                counter = 0;
            }
            if (!insert(afterid, qs2utf8s(it->filepath),
                        qs2utf8s(it->didl), &afterid)) {
                qDebug() << "OHPlayer::insertTracks: insert failed";
                break;
            }
        }
        qDebug() << "OHPlayer::insertTracks: sync at end";
        // Get rid of, e.g. queued "queue empty" events
        qApp->processEvents();
        sync();
        asyncArrayUpdates(true);
        m_ininsert = false;
    }

    void removeTracks(const QList<int>& lidx) {
        //qDebug() << "OHPlayer::removeTracks";
        std::vector<int> ids;
        ids.reserve(lidx.size());
        for (QList<int>::const_iterator it = lidx.begin(); 
             it != lidx.end(); it++) {
            if (*it >= 0 && *it < int(m_idsv.size())) {
                //qDebug() << "OHPlayer::removeTracks: " << m_idsv[*it];
                ids.push_back(m_idsv[*it]);
            } else {
                qDebug() << "OHPlayer::removeTracks: bad index" << *it;
            }
        }
        int counter = 0;
        for (std::vector<int>::const_iterator it = ids.begin(); 
             it != ids.end(); it++) {
            if (counter++ > 50) {
                qApp->processEvents();
                counter = 0;
            }
            if (!deleteId(*it)) {
                qDebug() << "OHPlayer::removeTracks: retrying delete " << *it;
                deleteId(*it);
            }
        }
    }

    void  changeMode(Playlist_Mode mode) {
        setRepeat(mode.repAll);
        setShuffle(mode.shuffle);
        m_mode = mode;
    }


private slots:
    void playerState(int ps) {
        std::string s;
        AudioState as = AUDIO_UNKNOWN;
        switch (ps) {
        case UPnPClient::OHPlaylist::TPS_Unknown:
        case UPnPClient::OHPlaylist::TPS_Buffering:
        default:
            s = "Unknown";
            break;
        case UPnPClient::OHPlaylist::TPS_Paused:
            as = AUDIO_PAUSED;
            s = "Paused";
            break;
        case UPnPClient::OHPlaylist::TPS_Playing:
            as = AUDIO_PLAYING;
            s = "Playing";
            break;
        case UPnPClient::OHPlaylist::TPS_Stopped:
            as = AUDIO_STOPPED;
            s = "Stopped";
            break;
        }
        if (as != AUDIO_UNKNOWN) {
            emit audioStateChanged(as, s.c_str());
        }
    }

    void onShuffleState(bool st) {
        m_mode.shuffle = st;
        emit playlistModeChanged(m_mode);
    }
    void onRepeatState(bool st) {
        m_mode.repAll = st;
        emit playlistModeChanged(m_mode);
    }
    void onTrackIdChanged(int) {
        m_songsecs = -1;
    }

    void translateMetaData() {
        //qDebug() << "OHPlayer::translateMetaData()";
        MetaDataList mdv;
        for (std::vector<int>::iterator idit = m_idsv.begin(); 
             idit != m_idsv.end(); idit++) {
            std::unordered_map<int, UPnPClient::UPnPDirObject>::iterator poolit 
                = m_metapool.find(*idit);
            if (poolit == m_metapool.end()) {
                qDebug() << "OHPlayer::translateMetaData: "
                    "no data found for " << *idit << "!!!";
                continue;
            }
            UPnPClient::UPnPDirObject& ude = poolit->second;
            if (ude.m_resources.empty()) {
                LOGDEB("translateMetadata: no resources for:"  <<
                       ude.dump() << endl);
                //continue; does not seem to be an issue in fact. Only used
                // for avt. Funny, this is the exact same problem which bubble
                // had initially with ohpl tracks added through mpd (no uri)...
            }
            MetaData md;
            udirentToMetadata(&ude, &md);
            md.id = *idit;
            md.pl_playing = md.id == m_curid;
            
            mdv.push_back(md);
        }
        emit metadataArrayChanged(mdv);
    }

signals:
    void audioStateChanged(int as, const char *);
    void metadataArrayChanged(const MetaDataList& mdv);
    void playlistModeChanged(Playlist_Mode);

private:
    int m_songsecs;
    Playlist_Mode m_mode;
    bool m_ininsert;
};

#endif /* _OHPLADAPT_H_INCLUDED_ */
