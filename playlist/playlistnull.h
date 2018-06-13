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
#ifndef PLAYLISTNULL_H_
#define PLAYLISTNULL_H_

#include <QDebug>

#include "HelperStructs/MetaData.h"
#include "playlist.h"

// This is a placeholder playlist object which does nothing, for when the
// renderer is using a source we can't control.

class PlaylistNULL : public Playlist {
    Q_OBJECT

public:
    PlaylistNULL(QObject *parent = 0)
        : Playlist(parent) {
    }

    virtual ~PlaylistNULL() { }

    virtual void update_state() {
        // Send out empty data.
        emit sig_track_metadata(MetaData());
        emit sig_playlist_updated(m_meta, -1, 0);
    }

signals:
    void sig_track_metadata(const MetaData&);

public slots:
    void psl_change_track_impl(int) {}
    void psl_clear_playlist_impl() {}
    void psl_play() {}
    void psl_pause() {}
    void psl_stop() {}
    void psl_forward() {}
    void psl_backward() {}
    void psl_remove_rows(const QList<int>&, bool = true) {}
    void psl_insert_tracks(const MetaDataList&, int) {}
    void psl_seek(int) {}
};

#endif /* PLAYLISTNULL_H_ */
