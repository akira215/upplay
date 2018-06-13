/* GUI_TrayIcon.cpp */

/* Copyright (C) 2012  gleugner
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

#include "trayicon.h"

#include <QAction>
#include <QMenu>
#include <QEvent>
#include <QWheelEvent>
#include <QHoverEvent>
#include <QDebug>
#include <QIcon>
#include <QPixmap>
#include <QSettings>

#include "HelperStructs/CSettingsStorage.h"
#include "HelperStructs/Helper.h"


GUI_TrayIcon::GUI_TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
{

    m_settings = CSettingsStorage::getInstance();
    m_playing = false;
    m_mute = false;

    QIcon play_icon = QIcon(Helper::getIconPath("play.png"));
    QIcon pause_icon = QIcon(Helper::getIconPath("pause.png"));
    QIcon sayo_icon = QIcon(Helper::getIconPath("logo.png"));
    QPixmap play_pixmap = play_icon.pixmap(24, 24);
    QPixmap pause_pixmap = pause_icon.pixmap(24, 24);
    QPixmap sayo_pixmap = sayo_icon.pixmap(24, 24);
    setIcon(QIcon(sayo_pixmap));

    m_playIcon = QIcon(play_pixmap);
    m_pauseIcon = QIcon(pause_pixmap);

    m_vol_step = 5;

    QSettings settings;
    m_timeout = m_settings->getNotificationTimeout();

    m_playAction = new QAction(tr("Play"), this);
    m_playAction->setIcon(QIcon(Helper::getIconPath("play.png")));
    m_stopAction = new QAction(tr("Stop"), this);
    m_stopAction->setIcon(QIcon(Helper::getIconPath("stop.png")));
    m_bwdAction = new QAction(tr("Previous"), this);
    m_bwdAction->setIcon(QIcon(Helper::getIconPath("bwd.png")));
    m_fwdAction = new QAction(tr("Next"), this);
    m_fwdAction->setIcon(QIcon(Helper::getIconPath("fwd.png")));
    m_muteAction = new QAction(tr("Mute"), this);
    m_muteAction->setIcon(QIcon(Helper::getIconPath("vol_mute.png")));
    m_closeAction = new QAction(tr("Close"), this);
    m_closeAction->setIcon(QIcon(Helper::getIconPath("close.png")));
    m_showAction = new QAction(tr("Show"), this);

    m_trayContextMenu = new QMenu();
    m_trayContextMenu->addAction(m_playAction);
    m_trayContextMenu->addAction(m_stopAction);
    m_trayContextMenu->addSeparator();
    m_trayContextMenu->addAction(m_fwdAction);
    m_trayContextMenu->addAction(m_bwdAction);
    m_trayContextMenu->addSeparator();
    m_trayContextMenu->addAction(m_muteAction);
    m_trayContextMenu->addSeparator();
    m_trayContextMenu->addAction(m_showAction);
    m_trayContextMenu->addAction(m_closeAction);
    setContextMenu(m_trayContextMenu);

    connect(m_playAction, SIGNAL(triggered()), this, SLOT(play_clicked()));
    connect(m_fwdAction, SIGNAL(triggered()), this, SLOT(fwd_clicked()));
    connect(m_bwdAction, SIGNAL(triggered()), this, SLOT(bwd_clicked()));
    connect(m_stopAction, SIGNAL(triggered()), this, SLOT(stop_clicked()));
    connect(m_showAction, SIGNAL(triggered()), this, SLOT(show_clicked()));
    connect(m_closeAction, SIGNAL(triggered()), this, SLOT(close_clicked()));
    connect(m_muteAction, SIGNAL(triggered()), this, SLOT(mute_clicked()));
}

void GUI_TrayIcon::change_skin(QString stylesheet)
{
    this->m_trayContextMenu->setStyleSheet(stylesheet);
    this->setMute(_mute);
}

bool GUI_TrayIcon::event(QEvent * e)
{
    if (e->type() == QEvent::Wheel) {
        QWheelEvent * wheelEvent = dynamic_cast <QWheelEvent *>(e);
        emit sig_volume_changed_by_wheel(wheelEvent->delta() > 0 ? 1 : -1);
    }

    return true;
}

void GUI_TrayIcon::songChangedMessage(const MetaData& md)
{
    if (isSystemTrayAvailable()) {
        showMessage("Upplay", md.title + tr(" by ") + md.artist,
                    QSystemTrayIcon::Information, 1000/*m_timeout*/);
    }
}

void  GUI_TrayIcon::set_timeout(int timeout_ms)
{
    m_timeout = timeout_ms;
}

void GUI_TrayIcon::set_enable_play(bool b)
{
    m_playAction->setEnabled(b);
}

void GUI_TrayIcon::set_enable_stop(bool b)
{
    m_stopAction->setEnabled(b);
}

void GUI_TrayIcon::set_enable_mute(bool)
{
}

void GUI_TrayIcon::set_enable_fwd(bool b)
{
    m_fwdAction->setEnabled(b);
}

void GUI_TrayIcon::set_enable_bwd(bool b)
{
    m_bwdAction->setEnabled(b);
}

void GUI_TrayIcon::set_enable_show(bool b)
{
    m_showAction->setEnabled(b);
}

void GUI_TrayIcon::play_clicked()
{
    qDebug() << "tray: play clicked";
    if (!m_playing) {
        emit sig_play_clicked();
    } else {
        emit sig_pause_clicked();
    }
}

void GUI_TrayIcon::stop_clicked()
{
    emit sig_stop_clicked();
}

void GUI_TrayIcon::stop()
{
}

void GUI_TrayIcon::fwd_clicked()
{
    emit sig_fwd_clicked();
}

void GUI_TrayIcon::bwd_clicked()
{
    emit sig_bwd_clicked();
}

void GUI_TrayIcon::show_clicked()
{
    emit sig_show_clicked();
}

void GUI_TrayIcon::close_clicked()
{
    emit sig_close_clicked();
}

void GUI_TrayIcon::mute_clicked()
{
    emit sig_mute_clicked();
}

void GUI_TrayIcon::setMute(bool mute)
{
    _mute = mute;

    QString suffix = "";
    int style = CSettingsStorage::getInstance()->getPlayerStyle();

    if (style == 1) {
        suffix = "_dark";
    }

    if (!mute) {
        m_muteAction->setIcon(QIcon(Helper::getIconPath("vol_3.png")));
        m_muteAction->setText(tr("Mute"));
    }  else {
        m_muteAction->setIcon(QIcon(Helper::getIconPath("vol_mute.png")));
        m_muteAction->setText(tr("Unmute"));
    }
}

void GUI_TrayIcon::setPlaying(bool playing)
{
    //qDebug() << "GUI_TrayIcon::setPlaying: " << playing;
    m_playing = playing;

    if (playing) {
        m_playAction->setIcon(m_pauseIcon);
        m_playAction->setText(tr("Pause"));
    } else {
        m_playAction->setIcon(m_playIcon);
        m_playAction->setText(tr("Play"));
    }
}

int GUI_TrayIcon::get_vol_step()
{
    return m_vol_step;
}
