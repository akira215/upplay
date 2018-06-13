/* application.h */

/* Copyright (C) 2013  Lucio Carreras
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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <memory>

#include <QObject>
#include <QApplication>

#include "libupnpp/control/mediarenderer.hxx"
#include "upqo/ohproduct_qo.h"

class AVTPlayer;
class CSettingsStorage;
class DirBrowser;
class GUI_Player;
class GUI_Playlist;
class MetaData;
class OHProductQO;
class OHRadioQO;
class OHTimeQO;
class OHVolumeQO;
class Playlist;
class RenderingControlQO;
class SongcastTool;
class UpplayNotifications;

class Application : public QObject
{
    Q_OBJECT;

public:
    Application(QApplication* qapp, QObject *parent = 0);
    virtual ~Application();

    bool is_initialized();
                        
    void getIdleMeta(MetaData* mdp);
                        
public slots:
    void chooseRenderer();
    void chooseSource();
    void openSongcast();
    void reconnectOrChoose();
    void onSourceTypeChanged(OHProductQO::SourceType);
    void onDirSortOrder();
    
private:

    GUI_Player   *m_player;
    std::shared_ptr<Playlist> m_playlist;
    DirBrowser   *m_cdb;

    UPnPClient::MRDH    m_rdr;
    RenderingControlQO *m_rdco;
    AVTPlayer    *m_avto;
    OHTimeQO     *m_ohtmo;
    OHVolumeQO   *m_ohvlo;
    OHProductQO  *m_ohpro;
    
    GUI_Playlist *m_ui_playlist;
    SongcastTool *m_sctool;
    UpplayNotifications *m_notifs;
    
    CSettingsStorage *m_settings;
    QApplication     *m_app;

    bool             m_initialized;
    // Can we send titles into the playlist (e.g. not OHradio).
    bool             m_playlistIsPlaylist;
    OHProductQO::SourceType m_ohsourcetype;
    QString          m_renderer_friendly_name;

    void clear_renderer();
    void init_connections();
    void renderer_connections();
    void playlist_connections();
    bool setupRenderer(const std::string& uid);
    void createPlaylistForOpenHomeSource();
    void chooseSourceOH();
    void chooseSourceAVT();
};


#endif // APPLICATION_H
