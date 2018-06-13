/* Copyright (C) 2015 J.F.Dockes
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef PLAYLISTLOCRD_H_
#define PLAYLISTLOCRD_H_

#include <QDebug>

#include "HelperStructs/MetaData.h"
#include "playlist.h"
#include "upadapt/avtadapt.h"

// Local handler for radio stations (to emulate the openhome radio service
// for a simple avt player).
// *** THIS IS UNFINISHED and not activated, because the radio description data
// is actually complicated. Code to look at to complete:
//   MediaPlayer/src/org/rpi/radio/ChannelReaderJSON.java and parsers/
//   https://github.com/brianhornsby/plugin.audio.tuneinradio
//   http://brianhornsby.com/kodi_addons/tuneinradio
// No C++ code unfortunately. The tunein data is recursive, and you
// need to parse misc playlist and other formats before getting down
// to an actually playable url.
class PlaylistLOCRD : public Playlist {
    Q_OBJECT

public:
    // We just borrow the AVTPlayer object, it will be deleted by our parent.
    PlaylistLOCRD(AVTPlayer *avtp, const char *listfile, QObject *parent = 0);

    virtual ~PlaylistLOCRD() { }

    virtual void update_state();

signals:
    void sig_track_metadata(const MetaData&);

public slots:

    // Slots connected to avt player signals
    void onExtTrackChange(const QString& uri);
    void onCurrentMetadata(const MetaData&);

    // The following are connected to GUI signals, for responding to
    // user actions.
    void psl_change_track_impl(int);
    void psl_play();
    void psl_pause();
    void psl_stop(); 

    void psl_clear_playlist_impl() {}
    void psl_forward() {}
    void psl_backward() {}
    void psl_remove_rows(const QList<int>&, bool = true) {}
    void psl_insert_tracks(const MetaDataList&, int) {}
    void psl_seek(int) {}

private:
    bool parseList(const char *fnm);

    AVTPlayer *m_avto;
};

#endif /* PLAYLISTLOCRD_H_ */
