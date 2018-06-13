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
#ifndef PLAYLISTOHRD_H_
#define PLAYLISTOHRD_H_

#include <QDebug>

#include "HelperStructs/MetaData.h"
#include "playlist.h"
#include "upadapt/ohrdadapt.h"
#include "upadapt/ohifadapt.h"

class PlaylistOHRD : public Playlist {
    Q_OBJECT

public:
    // We take ownership of the OHRadio object
    PlaylistOHRD(OHRad *ohrd, OHInf *ohif, QObject *parent = 0);

    virtual ~PlaylistOHRD() {
        delete m_ohrdo;
        delete m_ohifo;
    }

    virtual void update_state();

signals:
    void sig_track_metadata(const MetaData&);

public slots:

    // These receives changes from the remote state.
    void onRemoteCurrentTrackid(int id);
    void onRemoteTpState_impl(int, const char *) {}
    void onRemoteSecsInSong_impl(quint32) {}
    
    // The following are connected to GUI signals, for responding to
    // user actions.
    void psl_change_track_impl(int idx);
    void psl_clear_playlist_impl() {}
    void psl_play();
    void psl_pause();
    void psl_stop(); 
    void psl_forward() {}
    void psl_backward() {}
    void psl_remove_rows(const QList<int>&, bool = true) {}
    void psl_insert_tracks(const MetaDataList&, int) {}

    // Set from scratch after reading changes from device
    void onRemoteMetaArray(const MetaDataList&);
    void psl_seek(int) {}

private:
    // My link to the OpenHome Renderer
    OHRad *m_ohrdo;
    OHInf *m_ohifo;
};

#endif /* PLAYLISTOHRD_H_ */
