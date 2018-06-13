/* Copyright (C) 2011  Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef PLAYLIST_H_
#define PLAYLIST_H_

#include <iostream>

#include <QObject>
#include <QList>
#include <QMap>
#include <QStringList>

#include "HelperStructs/MetaData.h"
#include "HelperStructs/PlaylistMode.h"
#include "HelperStructs/globals.h"
#include "HelperStructs/CSettingsStorage.h"

class Playlist : public QObject {
    Q_OBJECT

public:
    Playlist(QObject * parent=0);
    virtual ~Playlist() {}

    virtual uint get_num_tracks() {
        return m_meta.size();
    }
    virtual void get_metadata(MetaDataList&);
    virtual Playlist_Mode mode();
    virtual void update_state() {}
    
signals:
    // New playlist (for displaying in ui_playlist)
    void sig_playlist_updated(MetaDataList&, int playidx, int/*unused*/);
    // Inform the ui_playlist about what row to highlight
    void sig_playing_track_changed(int row);
    // Current track info for displaying in the control area
    void sig_track_metadata(const MetaData&);
    // Make sure we have the latest state of the world
    void sig_sync();
    // Stopped at end of Playlist
    void sig_playlist_done();
    
    // These request action from the audio part
    void sig_stop();
    void sig_pause();
    void sig_resume_play();
    void sig_forward();
    void sig_backward();
    // Only connect in OHPlaylist mode
    void sig_mode_changed(Playlist_Mode);

    // These report a state transition of the audio part
    void sig_stopped();
    void sig_paused();
    void sig_playing();

    // Done inserting tracks in the playlist
    void sig_insert_done();

    void connectionLost();

    // Only ever emitted by the openhome variant
    void playlistModeChanged(Playlist_Mode);
                                           
public slots:
    virtual void psl_insert_tracks(const MetaDataList&, int afteridx) = 0;
    virtual void psl_add_tracks(const MetaDataList&);
    virtual void psl_sort_by_tno();
    virtual void psl_load_playlist();
    virtual void psl_save_playlist();

    virtual void psl_change_track(int num) {
        m_insertion_point = -1;
        psl_change_track_impl(num);
    }
    virtual void psl_seek(int secs) = 0;

    virtual void psl_change_track_impl(int) = 0;

    // Information from the remote end
    virtual void onRemoteTpState(int, const char *);
    // Maybe also tell the subclass about these
    virtual void onRemoteTpState_impl(int, const char *) {}
    void onRemoteSecsInSong(quint32 s) {
        onRemoteSecsInSong_impl(s);
    }
    virtual void onRemoteSecsInSong_impl(quint32) {}
    
    // Mode change requested by UI
    virtual void psl_change_mode(const Playlist_Mode&);
    virtual void psl_clear_playlist();
    virtual void psl_clear_playlist_impl() = 0;
    virtual void psl_play() = 0;
    virtual void psl_pause() = 0;
    virtual void psl_stop() = 0;
    virtual void psl_forward() = 0;
    virtual void psl_backward() = 0;

    // Note: in openhome mode, drag-dropping a selection containing
    // the currently playing track results in a track change because
    // this is seen as 2 separate remove/insert operations. After the
    // remove, the playing track has changed, and there is no way to
    // restore it when the insert occurs. We would need additional
    // input from the playlist ui which is the only code with the info
    // to link the remove/insert ops. Even then, it would be
    // complicated to restore the right playing track because we'd
    // have to wait for the playlist update from the remote player to
    // do it. Even them this would cause the sound to flicker. It
    // seems that the only option is to acknowledge the issue. We now stop
    // playing in this situation (when removing the currently playing track)
    virtual void psl_remove_rows(const QList<int> &, bool select_next=true) 
        = 0;

    virtual void psl_selection_min_row(int);

protected:
    MetaDataList m_meta;

    int	m_play_idx;
    int m_selection_min_row;
    int m_insertion_point;
    int m_tpstate;

    virtual void remove_row(int row);
    virtual bool valid_row(int row) {
        return row >= 0 && row < int(m_meta.size());
    }
};

#endif /* PLAYLIST_H_ */
