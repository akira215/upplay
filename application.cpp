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

#include "application.h"

#include <iostream>
#include <fstream>
#include <string>

#include <QApplication>
#include <QMessageBox>
#include <QDir>

#include "libupnpp/upnpplib.hxx"
#include "libupnpp/log.hxx"
#include "libupnpp/control/discovery.hxx"
#include "libupnpp/control/mediarenderer.hxx"
#include "libupnpp/control/renderingcontrol.hxx"

#include "GUI/mainw/mainw.h"
#include "GUI/playlist/GUI_Playlist.h"
#include "GUI/prefs/prefs.h"
#include "GUI/renderchoose/renderchoose.h"
#include "GUI/sourcechoose/sourcechoose.h"
#include "GUI/songcast/songcastdlg.h"
#include "notifications/notifications.h"
#include "HelperStructs/CSettingsStorage.h"
#include "HelperStructs/Helper.h"
#include "HelperStructs/Style.h"
#include "HelperStructs/globals.h"
#include "dirbrowser/dirbrowser.h"
#include "playlist/playlist.h"
#include "playlist/playlistavt.h"
#include "playlist/playlistnull.h"
#include "playlist/playlistohpl.h"
#include "playlist/playlistohrcv.h"
#include "playlist/playlistohrd.h"
#include "playlist/playlistlocrd.h"
#include "upadapt/avtadapt.h"
#include "upadapt/ohpladapt.h"
#include "upadapt/songcast.h"
#include "upqo/ohproduct_qo.h"
#include "upqo/ohradio_qo.h"
#include "upqo/ohreceiver_qo.h"
#include "upqo/ohtime_qo.h"
#include "upqo/ohvolume_qo.h"
#include "upqo/renderingcontrol_qo.h"

using namespace std;
using namespace UPnPClient;

#ifndef deleteZ
#define deleteZ(X) {delete X; X = 0;}
#endif

#define CONNECT(a,b,c,d) m_app->connect(a, SIGNAL(b), c, SLOT(d), \
                                        Qt::UniqueConnection)

static UPPrefs g_prefs;

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


Application::Application(QApplication* qapp, QObject *parent)
    : QObject(parent), m_player(0), m_cdb(0), m_rdco(0),
      m_avto(0), m_ohtmo(0), m_ohvlo(0), m_ohpro(0),
      m_ui_playlist(0), m_sctool(0), m_notifs(0), m_settings(0), m_app(qapp),
      m_initialized(false), m_playlistIsPlaylist(false),
      m_ohsourcetype(OHProductQO::OHPR_SourceUnknown)
{
    m_settings = CSettingsStorage::getInstance();

    QString version = UPPLAY_VERSION;
    m_settings->setVersion(version);

    m_player = new GUI_Player(this);
    g_prefs.setParent(m_player);
    m_player->enableSourceSelect(false);
    
    m_ui_playlist = new GUI_Playlist(m_player->getParentOfPlaylist(), 0);
    m_player->setPlaylistWidget(m_ui_playlist);

    m_cdb = new DirBrowser(m_player->getParentOfLibrary(), 0);
    m_player->setLibraryWidget(m_cdb);

    m_notifs = new UpplayNotifications(this);

    init_connections();
    string uid = qs2utf8s(m_settings->getPlayerUID());
    if (uid.empty()) {
        QTimer::singleShot(0, this, SLOT(chooseRenderer()));
    } else {
        if (!setupRenderer(uid)) {
            cerr << "Can't connect to previous media renderer" << endl;
            QTimer::singleShot(0, this, SLOT(chooseRenderer()));
        }
    }

    m_player->setWindowTitle("Upplay " + version);
    m_player->setWindowIcon(QIcon(Helper::getIconPath("logo.png")));
    m_player->setPlaylist(m_ui_playlist);
    m_player->setStyle(m_settings->getPlayerStyle());
    m_player->show();

    m_ui_playlist->resize(m_player->getParentOfPlaylist()->size());

    m_player->ui_loaded();

    m_initialized = true;
}

Application::~Application()
{
    delete m_player;
}

bool Application::is_initialized()
{
    return m_initialized;
}

void Application::chooseRenderer()
{
    MetaData md;
    getIdleMeta(&md);
    vector<UPnPDeviceDesc> devices;
    if (!MediaRenderer::getDeviceDescs(devices) || devices.empty()) {
        QMessageBox::warning(0, "Upplay",
                             tr("No Media Renderers found."));
        return;
    }

    QSettings settings;
    bool ohonly = settings.value("ohonly").toBool();

    RenderChooseDLG dlg(m_player);
    if (ohonly) {
        dlg.setWindowTitle(tr("Select Renderer (OpenHome Only)"));
    }
    vector<UPnPDeviceDesc*> filtered_devices;
    for (vector<UPnPDeviceDesc>::iterator it = devices.begin(); 
         it != devices.end(); it++) {
        if (ohonly) {
            UPnPClient::MRDH  rdr = getRenderer(it->UDN, false);
            if (!rdr || !rdr->hasOpenHome()) {
                cerr << "Renderer " << it->friendlyName << "(" << it->UDN
                     << ") not found or no openhome services (the option to"
                    " only use openhome renderers is set\n"<< endl;
                continue;
            }
        }
        filtered_devices.push_back(&(*it));
        QString fname = u8s2qs(it->friendlyName);
        if (!m_renderer_friendly_name.compare(fname)) {
            QListWidgetItem *item = new QListWidgetItem(fname);
            QFont font = dlg.rndsLW->font();
            font.setBold(true);
            item->setFont(font);
            dlg.rndsLW->addItem(item);
        } else {
            dlg.rndsLW->addItem(fname);
        }
    }
    if (!dlg.exec()) {
        return;
    }

    int row = dlg.rndsLW->currentRow();
    if (row < 0 || row >= int(filtered_devices.size())) {
        cerr << "Internal error: bad row after renderer choose dlg" << endl;
        return;
    }

    UPnPDeviceDesc& chosen(*filtered_devices[row]);
    MetaDataList curmeta;
    if (m_playlist) {
        m_playlist->get_metadata(curmeta);
    }

    m_renderer_friendly_name = u8s2qs(chosen.friendlyName);
    if (!setupRenderer(chosen.UDN)) {
        QMessageBox::warning(0, "Upplay", tr("Can't connect to ") +
                             m_renderer_friendly_name);
        m_renderer_friendly_name = "";
        return;
    }
    m_settings->setPlayerUID(u8s2qs(chosen.UDN));

    if (m_playlist && !dlg.keepRB->isChecked()) {
        if (dlg.replRB->isChecked()) {
            m_playlist->psl_clear_playlist();
        }
        m_playlist->psl_add_tracks(curmeta);
    }
}

void Application::chooseSource()
{
    if (m_ohpro) {
        chooseSourceOH();
    } else {
        // Not ready yet
        return;
        chooseSourceAVT();
    }
}

void Application::chooseSourceOH()
{
    vector<UPnPClient::OHProduct::Source> srcs;
    if (!m_ohpro->getSources(srcs)) {
        return;
    }
    qDebug() << "Application::chooseSource: got " << srcs.size() << " sources";
    int cur = -1;
    m_ohpro->sourceIndex(&cur);

    vector<int> rowtoidx;
    SourceChooseDLG dlg(m_player);
    for (unsigned int i = 0; i < srcs.size(); i++) {
        if (!srcs[i].visible)
            continue;
        QString stype = u8s2qs(srcs[i].type + "\t(" + srcs[i].name + ")");
        if (int(i) == cur) {
            QListWidgetItem *item = new QListWidgetItem(stype);
            QFont font = dlg.rndsLW->font();
            font.setBold(true);
            item->setFont(font);
            dlg.rndsLW->addItem(item);
        } else {
            dlg.rndsLW->addItem(stype);
        }
        rowtoidx.push_back(i);
    }
    if (!dlg.exec()) {
        return;
    }

    int row = dlg.rndsLW->currentRow();
    if (row < 0 || row >= int(rowtoidx.size())) {
        qDebug() << "Internal error: bad row after source choose dlg";
        return;
    }
    int idx = rowtoidx[row];
    if (idx != cur) {
        m_ohpro->setSourceIndex(idx);
    }
}


// Avt radio is not ready yet, this is not used for now.
void Application::chooseSourceAVT()
{
    vector<int> rowtoidx;
    SourceChooseDLG dlg(m_player);
    dlg.rndsLW->addItem("Playlist");
    dlg.rndsLW->addItem("Radio");
    if (!dlg.exec()) {
        return;
    }
    int row = dlg.rndsLW->currentRow();
    if (m_playlist) {
        m_playlist->psl_stop();
    }
    m_player->stopped();
    if (row == 1) {
        QString fn = QDir(Helper::getSharePath()).filePath("radiolist.xml");
        m_playlist = shared_ptr<Playlist>(new PlaylistLOCRD(m_avto,
                                                            fn.toLocal8Bit()));
        m_playlistIsPlaylist = false;
    } else {
        m_playlist = shared_ptr<Playlist>(new PlaylistAVT(m_avto,
                                                          m_rdr->desc()->UDN));
        m_playlistIsPlaylist = true;
    }
    playlist_connections();
}

void Application::openSongcast()
{
    SongcastDLG *scdlg;
    if (!m_sctool) {
        scdlg = new SongcastDLG(m_player);
        m_sctool = new SongcastTool(scdlg, this);
    } else {
        scdlg = m_sctool->dlg();
        m_sctool->initControls();
    }
    if (scdlg) {
        scdlg->hide();
        scdlg->show();
    }
}

void Application::clear_renderer()
{
    m_rdr = UPnPClient::MRDH();
    deleteZ(m_rdco);
    deleteZ(m_avto);
    deleteZ(m_ohtmo);
    deleteZ(m_ohvlo);
    deleteZ(m_ohpro);
    m_ohsourcetype = OHProductQO::OHPR_SourceUnknown;
}

void Application::reconnectOrChoose()
{
    string uid = qs2utf8s(m_settings->getPlayerUID());
    if (uid.empty() || !setupRenderer(uid)) {
        clear_renderer();
        m_playlist = shared_ptr<Playlist>(new PlaylistNULL());
        m_playlistIsPlaylist = false;
        playlist_connections();
        if (QMessageBox::warning(0, "Upplay",
                                 tr("Connection to current rendererer lost. "
                                    "Choose again ?"),
                                 QMessageBox::Cancel | QMessageBox::Ok, 
                                 QMessageBox::Ok) == QMessageBox::Ok) {
            chooseRenderer();
        }
    }
}

bool Application::setupRenderer(const string& uid)
{
    clear_renderer();

    bool needs_playlist = true;
    
    // The media renderer object is not used directly except for
    // providing handles to the services. Note that the lib will
    // return anything implementing either renderingcontrol or
    // ohproduct
    m_rdr = getRenderer(uid, false);
    if (!m_rdr) {
        cerr << "Renderer " << uid << " not found" << endl;
        return false;
    }
    m_renderer_friendly_name = u8s2qs(m_rdr->desc()->friendlyName);

    bool needavt = true;
    OHPRH ohpr = m_rdr->ohpr();
    if (ohpr) {
        // This is an OpenHome media renderer
        m_ohpro = new OHProductQO(ohpr);
        connect(m_ohpro, SIGNAL(sourceTypeChanged(OHProductQO::SourceType)),
                this, SLOT(onSourceTypeChanged(OHProductQO::SourceType)));

        // Try to use the time service
        OHTMH ohtm = m_rdr->ohtm();
        if (ohtm) {
            qDebug() << "Application::setupRenderer: OHTm ok, no need for avt";
            m_ohtmo = new OHTimeQO(ohtm);
            // no need for AVT then
            needavt = false;
        }

        // Create appropriate Playlist object depending on type of
        // source. Some may need m_ohtmo, so keep this behind its
        // creation
        createPlaylistForOpenHomeSource();
        needs_playlist = false;

        // Move this out of the if when avt radio is ready
        m_player->enableSourceSelect(true);
    } else {
        m_player->enableSourceSelect(false);
    }

    // It would be possible in theory to be connected to an openhome
    // playlist without a time service?? and use avt for time updates
    // instead.
    if (needavt) {
        AVTH avt = m_rdr->avt();
        if (!avt) {
            qDebug() << "Renderer: AVTransport missing but we need it";
            return false;
        }
        m_avto = new AVTPlayer(avt);
    }

    // Keep this after avt object creation !
    if (needs_playlist) {
        qDebug() <<"Application::setupRenderer: using AVT playlist";
        m_playlist = shared_ptr<Playlist>(new PlaylistAVT(m_avto,
                                                         m_rdr->desc()->UDN));
        m_playlistIsPlaylist = true;
    }


    // Use either renderingControl or ohvolume for volume control.
    RDCH rdc = m_rdr->rdc();
    if (rdc) {
        qDebug() << "Application::setupRenderer: using Rendering Control";
        m_rdco = new RenderingControlQO(rdc);
    } else {
        OHVLH ohvl = m_rdr->ohvl();
        if (!ohvl) {
            qDebug() << "Device implements neither RenderingControl nor "
                "OHVolume";
            return false;
        }
        qDebug() << "Application::setupRenderer: using OHVolume";
        m_ohvlo =  new OHVolumeQO(ohvl);
    }
    
    renderer_connections();
    playlist_connections();

    return true;
}

void Application::createPlaylistForOpenHomeSource()
{
    m_ohsourcetype = m_ohpro->getSourceType();

    switch (m_ohsourcetype) {

    case OHProductQO::OHPR_SourceRadio:
    {
        OHRDH ohrd = m_rdr->ohrd();
        if (!ohrd) {
            qDebug() << "Application::createPlaylistForOpenHomeSource: "
                "radio mode, but can't connect";
            return;
        }
        OHIFH ohif = m_rdr->ohif();
        m_playlist = shared_ptr<Playlist>(
            new PlaylistOHRD(new OHRad(ohrd), ohif ? new OHInf(ohif) : 0));
        m_playlistIsPlaylist = false;
    }
    break;

    case OHProductQO::OHPR_SourceReceiver:
    {
        OHRCH ohrc = m_rdr->ohrc();
        if (!ohrc) {
            qDebug() << "Application::createPlaylistForOpenHomeSource: "
                "receiver mode, but can't connect";
            return;
        }
        m_playlist = shared_ptr<Playlist>(
            new PlaylistOHRCV(ohrc, u8s2qs(m_rdr->desc()->friendlyName)));
        m_playlistIsPlaylist = false;
    }
    break;

    case OHProductQO::OHPR_SourcePlaylist:
    {
        OHPLH ohpl = m_rdr->ohpl();
        if (ohpl) {
            m_playlist = shared_ptr<Playlist>(
                new PlaylistOHPL(new OHPlayer(ohpl), m_ohtmo));
            m_playlistIsPlaylist = true;
        }
    }
    break;

    default:
    {
        m_playlist = shared_ptr<Playlist>(new PlaylistNULL());
        m_playlistIsPlaylist = false;
    }
    break;
    }

    if (!m_playlist) {
        qDebug() << "Application::createPlaylistForOpenHomeSource: "
            "could not create playlist object";
    }
}

void Application::onDirSortOrder()
{
    g_prefs.onShowPrefs(UPPrefs::PTAB_DIRSORT);
}

void Application::onSourceTypeChanged(OHProductQO::SourceType tp)
{
    //qDebug() << "Application::onSourceTypeChanged: " << int(tp);
    if (tp == m_ohsourcetype) {
        //qDebug() << "Application::onSourceTypeChanged: same type";
        return;
    }
    
    if (!m_ohpro) {
        // Not possible cause ohpro is the sender of this signal.. anyway
        qDebug() <<"Application::onSourceTypeChanged: no OHProduct!!";
        return;
    }
    createPlaylistForOpenHomeSource();
    playlist_connections();
}

void Application::getIdleMeta(MetaData* mdp)
{
    QString sourcetype;
    if (m_ohpro) {
        vector<OHProduct::Source> sources;
        if (m_ohpro->getSources(sources)) {
            int idx;
            if (m_ohpro->sourceIndex(&idx)) {
                if (idx >= 0 && idx < int(sources.size())) {
                    sourcetype = u8s2qs(sources[idx].name);
                }
            }
        }
    }

    mdp->title = QString::fromUtf8("Upplay ") + UPPLAY_VERSION;
    if (m_renderer_friendly_name.isEmpty()) {
        mdp->artist = "No renderer connected";
    } else {
        mdp->artist = tr("Renderer: ") + m_renderer_friendly_name;
        if (!sourcetype.isEmpty()) {
            mdp->artist += QString::fromUtf8(" (") + sourcetype + ")";
        }
    }
}

// We may switch the playlist when an openhome renderer switches sources. So
// set the playlist connections in a separate function
void Application::playlist_connections()
{
    if (m_playlistIsPlaylist)
        m_cdb->setPlaylist(m_playlist);
    else
        m_cdb->setPlaylist(shared_ptr<Playlist>());

    // Use either ohtime or avt for time updates
    if (m_ohtmo) {
        CONNECT(m_ohtmo, secsInSongChanged(quint32),
                m_playlist.get(), onRemoteSecsInSong(quint32));
        CONNECT(m_ohtmo, secsInSongChanged(quint32),
                m_notifs, songProgress(quint32));
    } else if (m_avto) {
        CONNECT(m_avto, secsInSongChanged(quint32),
                m_playlist.get(), onRemoteSecsInSong(quint32));
        CONNECT(m_avto, secsInSongChanged(quint32),
                m_notifs, songProgress(quint32));
    }

    CONNECT(m_player, play(), m_playlist.get(), psl_play());
    CONNECT(m_player, pause(), m_playlist.get(), psl_pause());
    CONNECT(m_player, stop(), m_playlist.get(), psl_stop());
    CONNECT(m_player, forward(), m_playlist.get(), psl_forward());
    CONNECT(m_player, backward(), m_playlist.get(), psl_backward());
    CONNECT(m_player, sig_load_playlist(), m_playlist.get(), psl_load_playlist());
    CONNECT(m_player, sig_save_playlist(), m_playlist.get(), psl_save_playlist());
    CONNECT(m_player, sig_seek(int), m_playlist.get(), psl_seek(int));

    CONNECT(m_playlist.get(), connectionLost(), this, reconnectOrChoose());
    CONNECT(m_playlist.get(), playlistModeChanged(Playlist_Mode),
            m_ui_playlist, setPlayerMode(Playlist_Mode));
    CONNECT(m_playlist.get(), sig_track_metadata(const MetaData&),
            m_player, update_track(const MetaData&));

    CONNECT(m_playlist.get(), sig_track_metadata(const MetaData&),
            m_notifs, notify(const MetaData&));
    CONNECT(m_playlist.get(), sig_stopped(),  m_notifs, onStopped());
    CONNECT(m_playlist.get(), sig_paused(),  m_notifs, onPaused());
    CONNECT(m_playlist.get(), sig_playing(),  m_notifs, onPlaying());
    CONNECT(m_notifs, notifyNeeded(const MetaData&),
            m_player, onNotify(const MetaData&));
            
    CONNECT(m_playlist.get(), sig_stopped(),  m_player, stopped());
    CONNECT(m_playlist.get(), sig_paused(),  m_player, paused());
    CONNECT(m_playlist.get(), sig_playing(),  m_player, playing());
    CONNECT(m_playlist.get(), sig_playing_track_changed(int),
            m_ui_playlist, track_changed(int));
    CONNECT(m_playlist.get(), sig_playlist_updated(MetaDataList&, int, int),
            m_ui_playlist, fillPlaylist(MetaDataList&, int, int));
    CONNECT(m_ui_playlist, selection_min_row(int),
            m_playlist.get(), psl_selection_min_row(int));
    CONNECT(m_ui_playlist, playlist_mode_changed(const Playlist_Mode&),
            m_playlist.get(), psl_change_mode(const Playlist_Mode&));
    CONNECT(m_ui_playlist, dropped_tracks(const MetaDataList&, int),
            m_playlist.get(), psl_insert_tracks(const MetaDataList&, int));
    CONNECT(m_ui_playlist, sig_rows_removed(const QList<int>&, bool),
            m_playlist.get(), psl_remove_rows(const QList<int>&, bool));
    CONNECT(m_ui_playlist, sig_sort_tno(),
            m_playlist.get(), psl_sort_by_tno());
    CONNECT(m_ui_playlist, row_activated(int),
            m_playlist.get(), psl_change_track(int));
    CONNECT(m_ui_playlist, clear_playlist(),
            m_playlist.get(), psl_clear_playlist());

    m_playlist->update_state();
}

// Direct renderer-Player connections (not going through the Playlist):
// volume and time mostly.
void Application::renderer_connections()
{
    // Use either ohtime or avt for time updates
    if (m_ohtmo) {
        CONNECT(m_ohtmo, secsInSongChanged(quint32),
                m_player, setCurrentPosition(quint32));
    } else if (m_avto) {
        CONNECT(m_avto, secsInSongChanged(quint32),
                m_player, setCurrentPosition(quint32));
    }
    if (m_ohvlo) {
        CONNECT(m_player, sig_volume_changed(int), m_ohvlo, setVolume(int));
        CONNECT(m_player, sig_mute(bool), m_ohvlo, setMute(bool));
        CONNECT(m_ohvlo, volumeChanged(int), m_player, setVolumeUi(int));
        CONNECT(m_ohvlo, muteChanged(bool), m_player, setMuteUi(bool));
        // Set up the initial volume from the renderer value
        m_player->setVolumeUi(m_ohvlo->volume());
    } else if (m_rdco) {
        CONNECT(m_player, sig_volume_changed(int), m_rdco, setVolume(int));
        CONNECT(m_player, sig_mute(bool), m_rdco, setMute(bool));
        CONNECT(m_rdco, volumeChanged(int), m_player, setVolumeUi(int));
        CONNECT(m_rdco, muteChanged(bool), m_player, setMuteUi(bool));
        // Set up the initial volume from the renderer value
        m_player->setVolumeUi(m_rdco->volume());
    }
}

// Connections which make sense without a renderer.
void Application::init_connections()
{
    CONNECT(m_player, show_small_playlist_items(bool),
            m_ui_playlist, psl_show_small_playlist_items(bool));
    CONNECT(m_player, sig_choose_renderer(), this, chooseRenderer());
    CONNECT(m_player, sig_open_songcast(), this, openSongcast());
    CONNECT(m_player, sig_choose_source(), this, chooseSource());
    CONNECT(m_player, sig_skin_changed(bool), m_cdb, setStyleSheet(bool));
    CONNECT(m_player, showSearchPanel(bool), m_cdb, showSearchPanel(bool));
    CONNECT(m_player, sig_preferences(), &g_prefs, onShowPrefs());
    CONNECT(&g_prefs, sig_prefsChanged(), m_cdb, onSortprefs());
    CONNECT(m_cdb, sig_next_group_html(QString),
            m_ui_playlist, psl_next_group_html(QString));
    CONNECT(m_cdb, sig_sort_order(), this, onDirSortOrder());
    CONNECT(m_player, sig_sortprefs(), m_cdb, onSortprefs());
}
