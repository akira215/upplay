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
#ifndef GUI_TRAYICON_H
#define GUI_TRAYICON_H

#include "HelperStructs/MetaData.h"

#include <QSystemTrayIcon>

class QAction;
class QMenu;
class CSettingsStorage;

/**
 * Small class to be used as tray icon
 */
class GUI_TrayIcon : public QSystemTrayIcon {
    Q_OBJECT;

public:

    GUI_TrayIcon(QObject *parent = 0);

    virtual bool event(QEvent * e);
    void set_timeout(int timeout_ms);

    void set_enable_play(bool);
    void set_enable_stop(bool);
    void set_enable_mute(bool);
    void set_enable_fwd(bool);
    void set_enable_bwd(bool);
    void set_enable_show(bool);

    int get_vol_step();

public slots:
    void songChangedMessage(const MetaData& md);
    void setPlaying(bool);
    void setMute(bool mute);
    void change_skin(QString stylesheet);
    void stop();

signals:
    /**
     * this event is fired, if we have a mouse wheel event
     * @param delta bigger then 0 when mouse wheel has moved forward
     * smaller when moved backwards
     */
    void sig_volume_changed_by_wheel(int delta);
    void sig_play_clicked();
    void sig_pause_clicked();
    void sig_fwd_clicked();
    void sig_bwd_clicked();
    void sig_show_clicked();
    void sig_hide_clicked();
    void sig_close_clicked();
    void sig_mute_clicked();
    void sig_stop_clicked();

private slots:
    void play_clicked();
    void stop_clicked();
    void fwd_clicked();
    void bwd_clicked();
    void show_clicked();
    void close_clicked();
    void mute_clicked();

private:
    QAction*    m_closeAction;
    QAction*    m_playAction;
    QAction*    m_stopAction;
    QAction*    m_muteAction;
    QAction*    m_fwdAction;
    QAction*    m_bwdAction;
    QAction*    m_showAction;
    QMenu*      m_trayContextMenu;

    QIcon                   m_playIcon;
    QIcon                   m_pauseIcon;
    int                     m_timeout;
    int                     m_vol_step;

    bool                    m_playing;
    bool                    m_mute;
    CSettingsStorage*       m_settings;

    bool            _mute;
};

#endif
