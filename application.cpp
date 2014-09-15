/* application.cpp */

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

#include <iostream>
#include <fstream>
#include <string>
#include <functional>

using namespace std;

#include <QApplication>
#include <QMap>
#include <QSharedMemory>
#include <QMessageBox>

#include "application.h"
#include "GUI/player/GUI_Player.h"
#include "GUI/playlist/GUI_Playlist.h"
#include "GUI/renderchoose/renderchoose.h"
#include "playlist/PlaylistAVT.h"
#include "playlist/PlaylistOH.h"
#include "HelperStructs/Helper.h"
#include "HelperStructs/CSettingsStorage.h"
#include "HelperStructs/Style.h"
#include "HelperStructs/globals.h"

#include "libupnpp/upnpplib.hxx"
#include "libupnpp/control/mediarenderer.hxx"
#include "libupnpp/control/renderingcontrol.hxx"
#include "libupnpp/control/discovery.hxx"
using namespace UPnPClient;

UPnPDeviceDirectory *superdir;

static MRDH getRenderer(const string& name, bool isfriendlyname)
{
    if (superdir == 0) {
        superdir = UPnPDeviceDirectory::getTheDir();
        if (superdir == 0) {
            cerr << "Can't create UPnP discovery object" << endl;
            exit(1);
        }
    }

    UPnPDeviceDesc ddesc;
    if (isfriendlyname) {
        if (superdir->getDevByFName(name, ddesc)) {
            return MRDH(new MediaRenderer(ddesc));
        }
        cerr << "getDevByFname failed" << endl;
    } else {
        if (superdir->getDevByUDN(name, ddesc)) {
            return MRDH(new MediaRenderer(ddesc));
        }
        cerr << "getDevByFname failed" << endl;
    }
    return MRDH();
}

bool Application::is_initialized()
{
    return _initialized;
}

bool Application::setupRenderer(const string& uid)
{
    delete rdco;
    rdco = 0;
    delete avto;
    avto = 0;
    delete ohplo;
    ohplo = 0;

    MRDH rdr = getRenderer(uid, false);
    if (!rdr) {
        cerr << "Renderer " << uid << " not found" << endl;
        return false;
    }

    AVTH avt = rdr->avt();
    if (!avt) {
        cerr << "Device " << uid << 
            " has no AVTransport service" << endl;
        return false;
    }

    RDCH rdc = rdr->rdc();
    if (!rdc) {
        cerr << "Device " << uid << 
            " has no RenderingControl service" << endl;
        return false;
    }

    rdco = new RenderingControlQO(rdc);
    avto = new AVTPlayer(avt);

    OHPLH ohpl = rdr->ohpl();
    if (ohpl) {
        ohplo = new OHPlayer(ohpl);
        qDebug() << "setupRenderer: deleting old playlist, creating OH one";
        delete playlist;
        playlist = new PlaylistOH();
    } else {
        ohplo = 0;
    }

    QString fn = QString::fromUtf8(rdr->m_desc.friendlyName.c_str());
    if (player) {
        player->setRendererName(fn);
    }
    return true;
}

void Application::chooseRenderer()
{
    vector<UPnPDeviceDesc> devices;
    if (!MediaRenderer::getDeviceDescs(devices) || devices.empty()) {
        QMessageBox::warning(0, "Upplay", 
                             tr("No Media Renderers found."));
        return;
    }
    RenderChooseDLG dlg(player);
    for (auto it = devices.begin(); it != devices.end(); it++) {
        dlg.rndsLW->addItem(QString::fromUtf8(it->friendlyName.c_str()));
    }
    if (!dlg.exec())
        return;

    int row = dlg.rndsLW->currentRow();
    if (row < 0 || row >= int(devices.size())) {
        cerr << "Internal error: bad row after renderer choose dlg" << endl;
        return;
    }

    QString friendlyname = QString::fromUtf8(devices[row].friendlyName.c_str());
    if (!setupRenderer(devices[row].UDN)) {
        QMessageBox::warning(0, "Upplay", tr("Can't connect to ") + 
                             friendlyname);
        return;
    }
    settings->setPlayerUID(QString::fromUtf8(devices[row].UDN.c_str()));
    init_connections();
}

Application::Application(QApplication* qapp, int, 
                         QTranslator* translator, QObject *parent)
    : QObject(parent), player(0), playlist(0), cdb(0), rdco(0),
      avto(0), ohplo(0), ui_playlist(0), settings(0), app(qapp),
      _initialized(false)
{
    settings = CSettingsStorage::getInstance();

    QString version = getVersion();
    settings->setVersion(version);

    player = new GUI_Player(translator);
    playlist = new PlaylistAVT();

    ui_playlist = new GUI_Playlist(player->getParentOfPlaylist(), 0);

    cdb = new CDBrowser(player->getParentOfLibrary());
    
    rdco = 0;
    avto = 0;

    string uid = qs2utf8s(settings->getPlayerUID());
    if (uid.empty()) {
        QTimer::singleShot(0, this, SLOT(chooseRenderer()));
    } else {
        if (!setupRenderer(uid)) {
            cerr << "Can't connect to media renderer" << endl;
            exit(1);
        }
    }

    init_connections();

    player->setWindowTitle("Upplay " + version);
    player->setWindowIcon(QIcon(Helper::getIconPath() + "logo.png"));
    player->setPlaylist(ui_playlist);
    player->setStyle(settings->getPlayerStyle() );
    player->show();

    ui_playlist->resize(player->getParentOfPlaylist()->size());

    player->ui_loaded();

    _initialized = true;
}

Application::~Application()
{
}


void Application::renderer_connections()
{
    if (ohplo) {
        qDebug() << "Connecting ohplo::metaDataReady(MetaDataList& mdv)";
        PlaylistOH *ploh = dynamic_cast<PlaylistOH*>(playlist);
        if (ploh == 0) {
            cerr << "OpenHome player but Playlist not openhome!";
            abort();
        }
        CONNECT(ploh, sig_clear_playlist(), ohplo, clear());

        CONNECT(ohplo, metaDataReady(const MetaDataList&),
                playlist, psl_new_ohpl(const MetaDataList&));
        CONNECT(ohplo, currentTrack(int), ploh, psl_currentTrack(int));
        CONNECT(ohplo, sig_PLModeChanged(Playlist_Mode),
                playlist, psl_mode_changed(Playlist_Mode));
        CONNECT(ohplo, sig_PLModeChanged(Playlist_Mode),
                ui_playlist, setMode(Playlist_Mode));
        CONNECT(ploh, sig_insert_tracks(const MetaDataList&, int),
                ohplo, insertTracks(const MetaDataList&, int));
        // seekIndex() is directly connected to an ui double-click,
        // this is for changing the current title to the first
        // selected when clicking play in stopped mode
        CONNECT(ploh, sig_row_activated(int), ohplo, seekIndex(int));

        CONNECT(ui_playlist, sig_rows_removed(const QList<int>&, bool), 
                ohplo, removeTracks(const QList<int>&, bool));
        CONNECT(ui_playlist, row_activated(int),  
                ohplo, seekIndex(int));

        CONNECT(playlist, sig_mode_changed(Playlist_Mode),
                ohplo, changeMode(Playlist_Mode));
        CONNECT(playlist, sig_pause(), ohplo, pause());
        CONNECT(playlist, sig_stop(),  ohplo, stop());
        CONNECT(playlist, sig_resume_play(), ohplo, play());
        CONNECT(playlist, sig_forward(), ohplo, next());
        CONNECT(playlist, sig_backward(), ohplo, previous());

        CONNECT(player, search(int), ohplo, seekPC(int));

        CONNECT(avto, secsInSongChanged(quint32), 
                player, setCurrentPosition(quint32));
        CONNECT(ohplo, sig_audioState(int, const char *), playlist, 
                psl_new_transport_state(int, const char *));

    } else {
        PlaylistAVT *plavt = dynamic_cast<PlaylistAVT*>(playlist);
        if (plavt == 0) {
            cerr << "!OpenHome player but Playlist not AVT !";
            abort();
        }
        CONNECT(plavt, sig_play_now(const MetaData&, int, bool),
                avto, changeTrack(const MetaData&, int, bool));
        CONNECT(plavt, sig_next_track_to_play(const MetaData&),
                avto, infoNextTrack(const MetaData&));
        CONNECT(avto, endOfTrackIsNear(), 
                plavt, psl_prepare_for_end_of_track());
        CONNECT(avto, newTrackPlaying(const QString&), 
                plavt, psl_ext_track_change(const QString&));
        CONNECT(avto, sig_currentMetadata(const MetaData&),
                plavt, psl_onCurrentMetadata(const MetaData&));
        // the search (actually seek) param is in percent
        CONNECT(player, search(int), avto, seekPC(int));

        CONNECT(avto, secsInSongChanged(quint32), 
                player, setCurrentPosition(quint32));
        CONNECT(avto, sig_audioState(int, const char*), playlist, 
                psl_new_transport_state(int, const char *));
        CONNECT(avto, stoppedAtEOT(), playlist, psl_forward());

        CONNECT(playlist, sig_stop(),  avto, stop());
        CONNECT(playlist, sig_resume_play(), avto, play());
        CONNECT(playlist, sig_pause(), avto, pause());
    }
    CONNECT(player, sig_volume_changed(int), rdco, setVolume(int));
    CONNECT(rdco, volumeChanged(int), player, setVolumeUi(int));
}

// Establish the connections we can do without a renderer, and call
// renderer_connection() if we can
void Application::init_connections()
{
    // At startup, we don't necessarily have a renderer setup
    // The playlist is an AVT one by default and can be connected.
    if (avto && rdco) {
        renderer_connections();
    }

    CONNECT(player, show_small_playlist_items(bool), 
            ui_playlist, psl_show_small_playlist_items(bool));
    CONNECT(player, sig_choose_renderer(), this, chooseRenderer());
    CONNECT(player, play(), playlist, psl_play());
    CONNECT(player, pause(), playlist, psl_pause());
    CONNECT(player, stop(), playlist, psl_stop());
    CONNECT(player, forward(), playlist, psl_forward());
    CONNECT(player, backward(), playlist, psl_backward());

    CONNECT(playlist, sig_track_metadata(const MetaData&, int, bool),
            player, update_track(const MetaData&, int, bool));
    CONNECT(playlist, sig_stopped(),  player, stopped());
    CONNECT(playlist, sig_paused(),  player, paused());
    CONNECT(playlist, sig_playing(),  player, playing());
    CONNECT(playlist, sig_playing_track_changed(int), 
            ui_playlist, track_changed(int));
    CONNECT(playlist, sig_playlist_updated(MetaDataList&, int, int), 
            ui_playlist, fillPlaylist(MetaDataList&, int, int));
    CONNECT(ui_playlist, selection_min_row(int),
            playlist, psl_selection_min_row(int));
    CONNECT(ui_playlist, playlist_mode_changed(const Playlist_Mode&), 
            playlist, psl_change_mode(const Playlist_Mode&));
    CONNECT(ui_playlist, dropped_tracks(const MetaDataList&, int), 
            playlist, psl_insert_tracks(const MetaDataList&, int));
    CONNECT(ui_playlist, sig_rows_removed(const QList<int>&, bool), 
            playlist, psl_remove_rows(const QList<int>&, bool));
    CONNECT(ui_playlist, row_activated(int),  
            playlist, psl_change_track(int));
    CONNECT(ui_playlist, clear_playlist(), playlist, psl_clear_playlist());

    CONNECT(cdb, 
            sig_tracks_to_playlist(PlaylistAddMode, bool, const MetaDataList&),
            playlist, psl_add_tracks(PlaylistAddMode, bool,
                                     const MetaDataList&));
}


QString Application::getVersion()
{
    return UPPLAY_VERSION;
}

QMainWindow* Application::getMainWindow()
{
    return this->player;
}
