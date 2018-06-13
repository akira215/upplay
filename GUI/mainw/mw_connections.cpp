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

#include "mainw.h"

void GUI_Player::setupConnections()
{
    connect(ui->player_w->playctl(), SIGNAL(playrequested()),
            this, SLOT(onPlayActivated()));
    connect(ui->player_w->playctl(), SIGNAL(pauserequested()),
            this, SLOT(onPauseActivated()));
    connect(ui->player_w->playctl(), SIGNAL(forwardrequested()),
            this, SLOT(onForwardActivated()));
    connect(ui->player_w->playctl(), SIGNAL(backwardrequested()),
            this, SLOT(onBackwardActivated()));
    connect(ui->player_w->playctl(), SIGNAL(stoprequested()),
            this, SLOT(onStopActivated()));

    connect(ui->player_w->volume(), SIGNAL(muteChanged(bool)),
            this, SLOT(onMuteActivated(bool)));
    connect(ui->player_w->volume(), SIGNAL(volumeChanged(int)),
            this, SIGNAL(sig_volume_changed(int)));

    connect(ui->player_w->progress(), SIGNAL(seekRequested(int)),
            this, SIGNAL(sig_seek(int)));
    
    // file
    connect(ui->actionSelect_Media_Renderer, SIGNAL(triggered(bool)),
            this, SIGNAL(sig_choose_renderer()));
    connect(ui->actionSelect_OH_Source, SIGNAL(triggered(bool)),
            this, SIGNAL(sig_choose_source()));
    connect(ui->actionSave_Playlist, SIGNAL(triggered(bool)),
            this, SIGNAL(sig_save_playlist()));
    connect(ui->actionLoad_Playlist, SIGNAL(triggered(bool)),
            this, SIGNAL(sig_load_playlist()));
    connect(ui->actionOpen_Songcast, SIGNAL(triggered(bool)),
            this, SIGNAL(sig_open_songcast()));
    connect(ui->action_Close, SIGNAL(triggered(bool)), this,
            SLOT(really_close(bool)));

    // view
    connect(ui->action_viewLibrary, SIGNAL(toggled(bool)), this,
            SLOT(showLibrary(bool)));
    connect(ui->action_viewSearchPanel, SIGNAL(toggled(bool)), this,
            SIGNAL(showSearchPanel(bool)));
    connect(ui->action_Dark, SIGNAL(toggled(bool)), this,
            SLOT(changeSkin(bool)));

    connect(ui->action_smallPlaylistItems, SIGNAL(toggled(bool)), this,
            SLOT(small_playlist_items_toggled(bool)));
    connect(ui->action_Fullscreen, SIGNAL(toggled(bool)), this,
            SLOT(show_fullscreen_toggled(bool)));
    connect(ui->action_Preferences, SIGNAL(triggered(bool)), this,
            SIGNAL(sig_preferences()));


    // about
    connect(ui->action_about, SIGNAL(triggered(bool)), this, SLOT(about(bool)));
    connect(ui->action_help, SIGNAL(triggered(bool)), this, SLOT(help(bool)));


    // Keyboard shortcuts
    QList<QKeySequence> lst;

    lst << QKeySequence(Qt::Key_MediaTogglePlayPause) <<
        QKeySequence(Qt::Key_MediaPlay) << QKeySequence(Qt::Key_MediaPause) <<
        QKeySequence(Qt::Key_Space);
    QAction* play_pause_action = createAction(lst);
    lst.clear();
    connect(play_pause_action, SIGNAL(triggered()),
            ui->player_w->playctl(), SLOT(onPlayClicked()));
    
    lst << QKeySequence(Qt::Key_MediaNext) <<
        QKeySequence(Qt::ControlModifier | Qt::Key_Right);
    QAction* fwd_action = createAction(lst);
    lst.clear();
    connect(fwd_action, SIGNAL(triggered()),
            ui->player_w->playctl(), SLOT(onForwardClicked()));
        
    lst << QKeySequence(Qt::Key_MediaPrevious) <<
        QKeySequence(Qt::ControlModifier | Qt::Key_Left);
    QAction* bwd_action = createAction(lst);
    lst.clear();
    connect(bwd_action, SIGNAL(triggered()),
            ui->player_w->playctl(), SLOT(onBackwardClicked()));
    
    QAction* stop_action = createAction(
        QKeySequence(Qt::ControlModifier | Qt::Key_Space));
    connect(stop_action, SIGNAL(triggered()),
            ui->player_w->playctl(), SLOT(onStopClicked()));

    lst << QKeySequence(Qt::ControlModifier | Qt::Key_Up) <<
        QKeySequence(Qt::Key_Plus);
    QAction* louder_action = createAction(lst);
    lst.clear();
    connect(louder_action, SIGNAL(triggered()),
            this, SLOT(onVolumeHigherActivated()));

    lst << QKeySequence(Qt::ControlModifier | Qt::Key_Down) <<
        QKeySequence(Qt::Key_Minus);
    QAction* leiser_action = createAction(lst);
    lst.clear();
    connect(leiser_action, SIGNAL(triggered()),
            this, SLOT(onVolumeLowerActivated()));
    
    QAction* two_perc_plus_action = createAction(
        QKeySequence(Qt::Key_Right));
    connect(two_perc_plus_action, SIGNAL(triggered()),
            this, SLOT(onJumpForwardActivated()));

    QAction* two_perc_minus_action = createAction(
        QKeySequence(Qt::Key_Left));
    connect(two_perc_minus_action, SIGNAL(triggered()),
            this, SLOT(onJumpBackwardActivated()));

}
