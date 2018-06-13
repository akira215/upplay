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
#ifndef GUI_SIMPLEPLAYER_H
#define GUI_SIMPLEPLAYER_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTranslator>

#include <QList>
#include <QCloseEvent>
#include <QKeySequence>
#include <QTranslator>
#include <QWidget>
#include <QAction>
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>

#ifdef UPPLAY_HORIZONTAL_LAYOUT
#include "ui_mainhw.h"
#else
#include "ui_mainw.h"
#endif

#include "trayicon.h"
#include "HelperStructs/Helper.h"

class GUI_Playlist;
class GUI_Library_windowed;
class GUI_InfoDialog;
class GUI_LibraryPath;
class GUI_Notifications;
class QTemporaryFile;
class Application;

class GUI_Player : public QMainWindow {
    Q_OBJECT
public:
    explicit GUI_Player(Application *upapp, QWidget *parent = 0);
    ~GUI_Player();

public slots:

    void really_close(bool=false);
    void trayItemActivated (QSystemTrayIcon::ActivationReason reason);

    // For restoring from settings
    void setVolume(int vol);
    
    // Reflect externally triggered audio changes in ui
    void update_track(const MetaData& in);
    // Systray notification. 
    void onNotify(const MetaData& md);
    void setCurrentPosition(quint32 pos_sec);
    void stopped();
    void playing();
    void paused();
    void setVolumeUi(int volume_percent);
    void setMuteUi(bool);

    void enableSourceSelect(bool);
    
signals:
    /* Player*/
    void play();
    void pause();
    void stop();
    void backward();
    void forward();
    void sig_mute(bool ison);
    void sig_volume_changed (int);
    void sig_seek(int secs);

    /* File */
    void sig_choose_renderer();
    void sig_choose_source();
    void sig_save_playlist();
    void sig_load_playlist();
    void sig_open_songcast();

    /* Preferences / View */
    void show_playlists();
    void show_small_playlist_items(bool);
    void sig_skin_changed(bool);
    void showSearchPanel(bool);
    void sig_sortprefs();
    void sig_preferences();

private slots:
    // These are connected to ui signals (either tray or main ctl),
    // and coordinate stuff.
    void onPlayActivated();
    void onPauseActivated();
    void onStopActivated();
    void onBackwardActivated();
    void onForwardActivated();
    void onJumpForwardActivated();
    void onJumpBackwardActivated();
    void onMuteActivated(bool);
    void onVolumeStepActivated(int val);
    void onVolumeHigherActivated();
    void onVolumeLowerActivated();
    void showLibrary(bool, bool resize=true);
    void changeSkin(bool);
    void small_playlist_items_toggled(bool);
    void show_fullscreen_toggled(bool);
    void about(bool b=false);
    void help(bool b=false);

    void sl_no_cover_available();
    void sl_cover_fetch_done(QNetworkReply*);

public:
    void setPlaylist(GUI_Playlist* playlist);
    QWidget *getParentOfPlaylist();
    QWidget *getParentOfLibrary();
    void setLibraryWidget(QWidget* );
    void setPlaylistWidget(QWidget* );

    void ui_loaded();
    void setStyle(int);

protected:
    void closeEvent(QCloseEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void resizeEvent(QResizeEvent* e);
    void moveEvent(QMoveEvent* e);

private:

    Application      *m_upapp;

    Ui::Upplay       *ui;

    GUI_Playlist*           ui_playlist;
    GUI_Library_windowed*   ui_library;
    GUI_LibraryPath*        ui_libpath;
    GUI_InfoDialog*         ui_info_dialog;
    GUI_Notifications*      ui_notifications;
    QNetworkAccessManager  *m_netmanager;

    QString                 m_class_name;
    GUI_TrayIcon *          m_trayIcon;

    QString                 m_skinSuffix;

    MetaData            m_metadata;
    bool                m_metadata_available;
    bool                m_overridemin2tray;

    int                 m_library_width;
    int                 m_library_stretch_factor;
    CSettingsStorage*   m_settings;
    QTemporaryFile*     m_covertempfile;
    // Last request for cover data: ignore others
    void                *m_currentCoverReply; 

    void setupTrayActions ();
    void idleDisplay();
    
    void setupConnections();
    void total_time_changed(qint64);
    void fetch_cover(const QString&);
    QAction* createAction(QKeySequence key_sequence);
    QAction* createAction(QList<QKeySequence>& key_sequences);
};


#endif // GUI_SIMPLEPLAYER_H
