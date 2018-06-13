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
#ifndef PLAYLISTOHRCV_H_
#define PLAYLISTOHRCV_H_

#include <QDebug>

#include "HelperStructs/MetaData.h"
#include "playlist.h"
#include "upqo/ohreceiver_qo.h"

// Incomplete: we don't process the remote transportstate changes. For now,
// can only be used for stopping in case of extreme need...
class PlaylistOHRCV : public Playlist {
    Q_OBJECT

public:
    PlaylistOHRCV(UPnPClient::OHRCH ohrc, const QString& fnm,
                  QObject *parent = 0)
        : Playlist(parent), m_ohrc(ohrc), m_renderer(fnm) {
    }

    virtual ~PlaylistOHRCV() {
    }

    virtual void update_state() {
        MetaDataList mdv;
        MetaData md;
        md.title = "Songcast receiver mode";
        md.artist = tr("Renderer: ") + m_renderer;
        mdv.push_back(md);
        emit sig_playlist_updated(mdv, 0, 0);
        emit sig_track_metadata(md);
    }

signals:
    void sig_track_metadata(const MetaData&);

public slots:

    void psl_change_track_impl(int) {}
    void psl_clear_playlist_impl() {}
    void psl_play() {m_ohrc->play();}
    void psl_pause() {}
    void psl_stop() {m_ohrc->stop();}
    void psl_forward() {}
    void psl_backward() {}
    void psl_remove_rows(const QList<int>&, bool = true) {}
    void psl_insert_tracks(const MetaDataList&, int) {}
    void psl_seek(int) {}
private:
    UPnPClient::OHRCH m_ohrc;
    QString m_renderer;
};

#endif /* PLAYLISTOHRCV_H_ */
