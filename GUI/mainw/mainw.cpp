/* GUI_Player.cpp */

/* Copyright (C) 2012  Lucio Carreras
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

#include "mainw.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QPalette>
#include <QUrl>
#include <QtNetwork/QNetworkRequest>
#include <QTemporaryFile>

#include "trayicon.h"
#include "GUI/playlist/GUI_Playlist.h"
#include "HelperStructs/CSettingsStorage.h"
#include "HelperStructs/Style.h"
#include "HelperStructs/globals.h"
#include "application.h"

GUI_Player* obj_ref = 0;

#ifdef Q_OS_UNIX
#include <signal.h>

void signal_handler(int sig)
{

    if (sig == SIGWINCH && obj_ref) {
        qDebug() << "show everything";
        obj_ref->setHidden(false);
        obj_ref->showNormal();
        obj_ref->activateWindow();
    }

    else if (sig == 15 && obj_ref) {
        qDebug() << "extern close event";
        obj_ref->really_close();
    } else {

        qDebug() << "signal " << sig << " received";
    }
}

#endif

GUI_Player::GUI_Player(Application *upapp, QWidget *parent) :
    QMainWindow(parent), m_upapp(upapp), ui(new Ui::Upplay), m_covertempfile(0),
    m_currentCoverReply(0)
{
    ui->setupUi(this);

    ui->action_viewLibrary->setText(tr("&Library"));
    ui->action_Fullscreen->setShortcut(QKeySequence("F11"));
    ui->action_Dark->setShortcut(QKeySequence("F10"));

    m_settings = CSettingsStorage::getInstance();
    m_netmanager = new QNetworkAccessManager(this);
    connect(m_netmanager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(sl_cover_fetch_done(QNetworkReply*)));

    m_metadata_available = false;
    m_overridemin2tray = false;

    ui_playlist = 0;

    m_skinSuffix = "";
    m_class_name = "Player";

    bool showSmallPlaylistItems = m_settings->getShowSmallPlaylist();
    ui->action_smallPlaylistItems->setChecked(showSmallPlaylistItems);

    QSizePolicy p = ui->library_widget->sizePolicy();
    m_library_stretch_factor = p.horizontalStretch();

    bool show_library = !m_settings->getNoShowLibrary();
    ui->action_viewLibrary->setChecked(show_library);
    this->showLibrary(show_library);

    bool is_fullscreen = m_settings->getPlayerFullscreen();
    if (!is_fullscreen) {
        QSize size = m_settings->getPlayerSize();
        QPoint pos = m_settings->getPlayerPos();
#ifdef _WIN32
        qDebug() << "Init size " << size << " pos " << pos;
        if (pos.x() < 20 || pos.y() < 30) {
            pos = QPoint(20, 30);
        }
        if (size.width() < 0 || size.height() < 0) {
            size = QSize(800, 600);
        }
        qDebug() << "Req size " << size << " pos " << pos;
#endif
        // Use the defaults if the prefs are not set yet
        QRect rect = this->geometry();
        rect.setX(pos.x());
        rect.setY(pos.y());
        rect.setWidth(size.width());
        rect.setHeight(size.height());
        this->setGeometry(rect);
    }
    
    m_library_width = 600;

    /* TRAY ACTIONS */
    this->setupTrayActions();

    /* SIGNALS AND SLOTS */
    this->setupConnections();
    ui_info_dialog = 0;

    changeSkin(m_settings->getPlayerStyle() == 1);
    idleDisplay();
}


GUI_Player::~GUI_Player()
{
    delete m_covertempfile;
}

QAction* GUI_Player::createAction(QList<QKeySequence>& seq_list)
{
    QAction* action = new QAction(this);

    action->setShortcuts(seq_list);
    action->setShortcutContext(Qt::ApplicationShortcut);
    this->addAction(action);
    connect(this, SIGNAL(destroyed()), action, SLOT(deleteLater()));

    return action;
}

QAction* GUI_Player::createAction(QKeySequence seq)
{
    QList<QKeySequence> seq_list;
    seq_list << seq;
    return createAction(seq_list);
}

// This is called from the playlist when new track info is known
// (possibly up from the audio player)
void GUI_Player::update_track(const MetaData& md)
{
    qDebug() << "GUI_Player::update_track()";
    if (!m_metadata.compare(md)) {
        return;
    }
    m_metadata = md;

    total_time_changed(md.length_ms);

    ui->player_w->mdata()->setData(md);

    this->setWindowTitle(QString("Upplay - ") + md.title);

    if (!md.albumArtURI.isEmpty()) {
        fetch_cover(md.albumArtURI);
    } else {
        m_currentCoverReply = 0;
        sl_no_cover_available();
    }

    ui->player_w->progress()->setEnabled(true);

    m_trayIcon->set_enable_play(true);
    m_trayIcon->set_enable_fwd(true);
    m_trayIcon->set_enable_bwd(true);

    ui->player_w->progress()->setEnabled(true);

    m_metadata_available = true;
}
void GUI_Player::onNotify(const MetaData& md)
{
    m_trayIcon->songChangedMessage(md);
}

void GUI_Player::fetch_cover(const QString& URI)
{
    if (!m_netmanager) {
        m_currentCoverReply = 0;
        return;
    }
    m_currentCoverReply = m_netmanager->get(QNetworkRequest(QUrl(URI)));
}

void GUI_Player::setStyle(int style)
{

    bool dark = (style == 1);
    changeSkin(dark);
    ui->action_Dark->setChecked(dark);
}

void GUI_Player::changeSkin(bool dark)
{
    QString stylesheet = Style::get_style(dark);
    qApp->setStyleSheet(stylesheet);

    if (dark) {
        Helper::setStyleSubDir("dark");
    } else {
        Helper::setStyleSubDir("");
    }

    m_settings->setPlayerStyle(dark ? 1 : 0);
    m_trayIcon->change_skin(stylesheet);

    ui->player_w->volume()->updateSkin();
    emit sig_skin_changed(dark);
}

void GUI_Player::enableSourceSelect(bool enb)
{
    ui->actionSelect_OH_Source->setEnabled(enb);
}

/** TRAY ICON **/
void GUI_Player::setupTrayActions()
{
    m_trayIcon = new GUI_TrayIcon(this);

    connect(m_trayIcon, SIGNAL(sig_stop_clicked()),
            this, SLOT(onStopActivated()));
    connect(m_trayIcon, SIGNAL(sig_bwd_clicked()),
            this, SLOT(onBackwardActivated()));
    connect(m_trayIcon, SIGNAL(sig_fwd_clicked()),
            this, SLOT(onForwardActivated()));
    connect(m_trayIcon, SIGNAL(sig_play_clicked()),
            this, SLOT(onPlayActivated()));
    connect(m_trayIcon, SIGNAL(sig_pause_clicked()),
            this, SLOT(onPauseActivated()));
    connect(m_trayIcon, SIGNAL(sig_mute_clicked()),
            ui->player_w->volume(), SLOT(toggleMute()));
    connect(m_trayIcon, SIGNAL(sig_close_clicked()),
            this, SLOT(really_close()));
    connect(m_trayIcon, SIGNAL(sig_show_clicked()), this, SLOT(showNormal()));
    connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayItemActivated(QSystemTrayIcon::ActivationReason)));
    connect(m_trayIcon, SIGNAL(sig_volume_changed_by_wheel(int)),
            this, SLOT(onVolumeStepActivated(int)));

    m_trayIcon->setPlaying(false);

    m_trayIcon->show();
}

void GUI_Player::trayItemActivated(QSystemTrayIcon::ActivationReason reason)
{
    QSettings settings;

    switch (reason) {

    case QSystemTrayIcon::Trigger:

        if (this->isMinimized() || isHidden()) {
            this->setHidden(false);
            this->showNormal();
            this->activateWindow();
        } else if (settings.value("min2tray").toBool()) {
            this->setHidden(true);
        } else {
            this->showMinimized();
        }

        break;
    case QSystemTrayIcon::MiddleClick:
        m_trayIcon->songChangedMessage(m_metadata);
        break;
    default:
        break;
    }
}

/** TRAY ICON END **/


/** LIBRARY AND PLAYLIST **/

QWidget* GUI_Player::getParentOfPlaylist()
{
    return ui->playlist_widget ? ui->playlist_widget->parentWidget() : 
        this->centralWidget();
}

QWidget* GUI_Player::getParentOfLibrary()
{
    return ui->library_widget ? ui->library_widget->parentWidget() : 
        this->centralWidget();
}

void GUI_Player::setPlaylistWidget(QWidget* w)
{
    if (ui->playlist_widget) {
        w->setSizePolicy(ui->playlist_widget->sizePolicy());
    } else {
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(2);
        sizePolicy.setVerticalStretch(2);
        sizePolicy.setHeightForWidth(w->sizePolicy().hasHeightForWidth());
        w->setSizePolicy(sizePolicy);
    }
    delete ui->playlist_widget;
    ui->playlist_widget = w;
    ui->verticalLayout->addWidget(ui->playlist_widget);
#ifdef UPPLAY_HORIZONTAL_LAYOUT
    QLayoutItem *cover = ui->player_w->takeCoverWidget();
    QWidget *cw = cover->widget();
    cw->setMaximumSize(QSize(9000, 9000));
    cw->setSizePolicy(ui->coverWidget->sizePolicy());
    ui->verticalLayout->insertWidget(0, cw, 1);
    delete ui->coverWidget;
    ui->verticalLayout->update();
#endif
}

void GUI_Player::setLibraryWidget(QWidget* w)
{
    if (ui->library_widget) {
        w->setSizePolicy(ui->library_widget->sizePolicy());
    } else {
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(2);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(w->sizePolicy().hasHeightForWidth());
        w->setSizePolicy(sizePolicy);
    }

    delete ui->library_widget;
    ui->library_widget = w;
    ui->splitter->addWidget(w);
    ui->splitter->restoreState(m_settings->getSplitterState());
    bool show_library = !m_settings->getNoShowLibrary();
    this->showLibrary(show_library, false);
}

void GUI_Player::setPlaylist(GUI_Playlist* playlist)
{
    ui_playlist = playlist;
    if (ui_playlist) {
        ui_playlist->show();
        ui_playlist->resize(ui->playlist_widget->size());
        QAction* action = createAction(QKeySequence(tr("Ctrl+P")));
        connect(action, SIGNAL(triggered()), ui_playlist, SLOT(setFocus()));
    }
}

void GUI_Player::stopped()
{
    //qDebug() << "void GUI_Player::stopped()";
    ui->player_w->playctl()->onStopped();
    m_metadata_available = false;
    m_metadata = MetaData();
    idleDisplay();
}

void GUI_Player::playing()
{
    //qDebug() << "void GUI_Player::playing()";
    ui->player_w->playctl()->onPlaying();
    m_trayIcon->setPlaying(true);
}
void GUI_Player::paused()
{
    //qDebug() << "void GUI_Player::paused()";
    ui->player_w->playctl()->onPaused();
    m_trayIcon->setPlaying(false);
}

/** LIBRARY AND PLAYLIST END **/


void GUI_Player::ui_loaded()
{
#ifdef Q_OS_UNIX
    obj_ref = this;
    signal(SIGWINCH, signal_handler);
#endif

    changeSkin(m_settings->getPlayerStyle() == 1);
    this->ui->action_Fullscreen->setChecked(m_settings->getPlayerFullscreen());
}

void GUI_Player::moveEvent(QMoveEvent *e)
{
    QMainWindow::moveEvent(e);

    QPoint p = this->pos();
    m_settings->setPlayerPos(p);
}

void GUI_Player::resizeEvent(QResizeEvent*e)
{
    QMainWindow::resizeEvent(e);
    m_settings->setPlayerSize(this->size());
    if (ui_playlist)
        ui_playlist->resize(ui->playlist_widget->size());
}

void GUI_Player::keyPressEvent(QKeyEvent* e)
{

    e->accept();

    switch (e->key()) {

    case (Qt::Key_F10):
        ui->action_Dark->setChecked(!ui->action_Dark->isChecked());
        break;

    case (Qt::Key_F11):
        show_fullscreen_toggled(!this->isFullScreen());
        break;

    default:
        break;
    }
}

void GUI_Player::closeEvent(QCloseEvent* e)
{
    m_settings->setSplitterState(ui->splitter->saveState());
    QSettings settings;
    if (!m_overridemin2tray && settings.value("min2tray").toBool()) {
        e->ignore();
        this->hide();
    }
}

void GUI_Player::really_close(bool)
{
    m_overridemin2tray = true;
    this->close();
}
