/* Copyright (C) 2014 J.F.Dockes
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
#ifndef _RANDPLAY_H_INCLUDED_
#define _RANDPLAY_H_INCLUDED_

#include <vector>
#include <algorithm>

#include <QObject>
#include <QString>

#include <libupnpp/config.h>
#include <libupnpp/control/cdirectory.hxx>

#include "HelperStructs/Helper.h"

class RandPlayer : public QObject {
    Q_OBJECT;

public:
    enum PlayMode {PM_TRACKS, PM_GROUPS};

    RandPlayer(PlayMode mode,
               const std::vector<UPnPClient::UPnPDirObject>& entries,
               QObject *parent = 0);
    virtual ~RandPlayer();
                         
public slots:
    void playNextSlice();

signals:
    void sig_randplay_done();
    void sig_tracks_to_playlist(const MetaDataList&);
    // While playing by groups: preview of next to play. May be
    // connected to the playlist clear tooltip (as this is what will
    // be played if the current playlist is cleared)
    void sig_next_group_html(QString);
    
private:
    PlayMode m_playmode;
    std::vector<UPnPClient::UPnPDirObject> m_entries;
    // Next group to play, for preview
    std::vector<UPnPClient::UPnPDirObject> m_nextgroup;
    void selectNextGroup();
    QString groupHtml(std::vector<UPnPClient::UPnPDirObject>& ents);
};

#endif /* _RANDPLAY_H_INCLUDED_ */
