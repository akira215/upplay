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

#include "playlistohpl.h"
#include "upqo/ohtime_qo.h"
#include "libupnpp/log.hxx"

using namespace UPnPP;

    // We take ownership of the OHPlayer object
PlaylistOHPL::PlaylistOHPL(OHPlayer *ohpl, OHTimeQO *ohtm, QObject * parent)
    : Playlist(parent), m_ohplo(ohpl), m_ohtmo(ohtm),
      m_cursongsecs(0), m_lastsong(false), m_closetoend(false)
{
    // Connections from OpenHome renderer to local playlist
    connect(m_ohplo, SIGNAL(metadataArrayChanged(const MetaDataList&)),
            this, SLOT(onRemoteMetaArray(const MetaDataList&)));
    connect(m_ohplo, SIGNAL(currentTrackId(int)),
            this, SLOT(onRemoteCurrentTrackid(int)));
    connect(m_ohplo, SIGNAL(audioStateChanged(int, const char *)),
            this, SLOT(onRemoteTpState(int, const char *)));
    connect(m_ohplo, SIGNAL(connectionLost()), this, SIGNAL(connectionLost()));
    
    // Connections from local playlist to openhome
    connect(this, SIGNAL(sig_clear_playlist()), m_ohplo, SLOT(clear()));
    connect(this, SIGNAL(sig_insert_tracks(const MetaDataList&, int)),
            m_ohplo, SLOT(insertTracks(const MetaDataList&, int)));
    connect(this, SIGNAL(sig_tracks_removed(const QList<int>&)),
            m_ohplo, SLOT(removeTracks(const QList<int>&)));
    connect(this, SIGNAL(sig_mode_changed(Playlist_Mode)),
            m_ohplo, SLOT(changeMode(Playlist_Mode)));
    connect(this, SIGNAL(sig_sync()), m_ohplo, SLOT(sync()));
    connect(this, SIGNAL(sig_pause()), m_ohplo, SLOT(pause()));
    connect(this, SIGNAL(sig_stop()),  m_ohplo, SLOT(stop()));
    connect(this, SIGNAL(sig_resume_play()), m_ohplo, SLOT(play()));
    connect(this, SIGNAL(sig_forward()), m_ohplo, SLOT(next()));
    connect(this, SIGNAL(sig_backward()), m_ohplo, SLOT(previous()));
}

static bool samelist(const MetaDataList& mdv1, const MetaDataList& mdv2)
{
    if (mdv1.size() != mdv2.size())
        return false;
    for (unsigned int i = 0; i < mdv1.size(); i++) {
        if (mdv1[i].filepath.compare(mdv2[i].filepath) ||
            mdv1[i].id != mdv2[i].id)
            return false;
    }
    return true;
}

void PlaylistOHPL::onRemoteMetaArray(const MetaDataList& mdv)
{
    qDebug() << "PlaylistOHPL::onRemoteMetaArray: " << mdv.size() << " entries";
    if (mdv.empty() || !samelist(mdv, m_meta)) {
        m_meta = mdv;

        emit sig_playlist_updated(m_meta, m_play_idx, 0);
    }
}

void PlaylistOHPL::psl_seek(int secs)
{
    m_ohplo->seek(secs);
}

void PlaylistOHPL::maybeSetDuration(bool needsig)
{
    if (m_play_idx< 0 || m_play_idx >= int(m_meta.size()) || !m_ohtmo) {
        return;
    }
    MetaData& meta(m_meta[m_play_idx]);
    if (meta.length_ms <= 0) {
        UPnPClient::OHTime::Time tm;
        if (m_ohtmo->time(tm)) {
            meta.length_ms = tm.duration * 1000;
            if (needsig) {
                emit sig_track_metadata(meta);
            }
        }
    }
    // Set the songsec every time, it's cheap and it makes
    // things work when the duration is not in the didl (else
    // there are order of events issues which result in unset
    // songsecs in ohpladapt
    m_ohplo->setSongSecs(meta.length_ms / 1000);
}

void PlaylistOHPL::onRemoteCurrentTrackid(int id)
{
    qDebug() << "PlaylistOHPL::onRemoteCurrentTrackid: " << id;

    if (id <= 0) {
        return;
    } 

    for (vector<MetaData>::iterator it = m_meta.begin(); 
         it != m_meta.end(); it++) {
        if (it->id == id) {
            if (m_play_idx != it - m_meta.begin()) {
                m_play_idx = it - m_meta.begin();
                if (m_play_idx == int(m_meta.size()) - 1) {
                    m_lastsong = true;
                }
                //qDebug() << " new track index " << m_play_idx;
            }
            maybeSetDuration(false);

            // Emit the current index in any case to let the playlist
            // UI scroll to show the currently playing track (some
            // time after a user interaction scrolled it off)
            emit sig_playing_track_changed(m_play_idx);
            // If the track id change was caused by the currently
            // playing track having been removed, the play index did
            // not change but the metadata did, so emit metadata in
            // all cases.
            emit sig_track_metadata(*it);
            m_cursongsecs = it->length_ms / 1000;
            return;
        }
    }
    resetPosState();
    LOGINF("PlaylistOHPL::onRemoteCurrentTrackid: track not found in array\n");
}

void PlaylistOHPL::onRemoteSecsInSong_impl(quint32 secs)
{
    if (m_lastsong && m_cursongsecs > 0 && int(secs) > int(m_cursongsecs) - 3) {
        //qDebug() << "PlaylistOHPL::onRemoteSecsInSong_impl: close to end";
        m_closetoend = true;
    }
    maybeSetDuration(true);
}

void PlaylistOHPL::onRemoteTpState_impl(int, const char *)
{
    //qDebug() << "PlaylistOHPL::onRemoteTpState_impl: " << sst;
    if (m_tpstate == AUDIO_STOPPED && m_closetoend == true) {
        resetPosState();
        emit sig_playlist_done();
    }
}

void PlaylistOHPL::psl_clear_playlist_impl()
{
    // Tell the Open Home Playlist to do it.
    emit sig_clear_playlist();
    // If we're playing, signal playlist done (this is to inform the
    // "random play by group" thing to start next group
    emit sig_playlist_done();
}

void PlaylistOHPL::psl_change_track_impl(int idx) {
    m_ohplo->seekIndex(idx);
}

void PlaylistOHPL::psl_play() 
{
    if (m_tpstate ==  AUDIO_STOPPED && valid_row(m_selection_min_row)) {
        m_ohplo->seekIndex(m_selection_min_row);
    } else {
        emit sig_resume_play();
    }
}

void PlaylistOHPL::psl_pause() 
{
    emit sig_pause();
}

void PlaylistOHPL::psl_stop() 
{
    emit sig_stop();
}

void PlaylistOHPL::psl_forward() 
{
    emit sig_forward();
}

void PlaylistOHPL::psl_backward() 
{
    emit sig_backward();
}

void PlaylistOHPL::psl_insert_tracks(const MetaDataList& meta, int afteridx)
{
    qDebug() << "PlaylistOHPL::psl_insert_tracks ntracks " << meta.size() << 
        " afteridx" << afteridx;
    emit sig_insert_tracks(meta, afteridx);
}

void PlaylistOHPL::psl_remove_rows(const QList<int>& rows, bool)
{
    if (rows.contains(m_play_idx)) {
        m_play_idx = -1;
    }
    emit sig_tracks_removed(rows);
}
