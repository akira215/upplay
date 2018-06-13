// GUI_PlayerButtons.cpp

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
#include <iostream>
using namespace std;

#include "mainw.h"
#include "trayicon.h"
#include "application.h"

/** Slots connected to player or trayicon signals **/
void GUI_Player::onPlayActivated()
{
    // We don't know which control caused this, so make sure all ui are setup
    m_trayIcon->setPlaying(true);
    ui->player_w->playctl()->onPlaying();

    emit play();
}

void GUI_Player::onPauseActivated()
{
    // We don't know which control caused this, so make sure all ui are setup
    m_trayIcon->setPlaying(false);
    ui->player_w->playctl()->onPaused();

    emit pause();
}

void GUI_Player::onStopActivated()
{
    m_trayIcon->setPlaying(false);
    m_trayIcon->stop();

    ui->player_w->playctl()->onStopped();

    idleDisplay();
    emit stop();
}

void GUI_Player::onMuteActivated(bool mute)
{
    m_trayIcon->setMute(mute);
    ui->player_w->volume()->setMuteUi(mute);

    emit sig_mute(mute);
}

void GUI_Player::idleDisplay()
{
    MetaData md;
    m_upapp->getIdleMeta(&md);
    ui->player_w->mdata()->setData(md);

    this->setWindowTitle("Upplay");
    ui->player_w->progress()->setUi(0);

    m_currentCoverReply = 0;
    sl_no_cover_available();
}

void GUI_Player::onBackwardActivated()
{
    // ui->albumCover->setFocus();
    int cur_pos_sec =
        (m_metadata.length_ms * ui->player_w->progress()->currentValuePc())
        / 100000;
    if (cur_pos_sec > 3) {
        emit sig_seek(0);
    } else {
        emit backward();
    }
}

void GUI_Player::onForwardActivated()
{
    //ui->albumCover->setFocus();
    emit forward();
}


/** PROGRESS BAR **/

void GUI_Player::total_time_changed(qint64 total_time)
{
    ui->player_w->progress()->setTotalTime(total_time/1000);
}

void GUI_Player::onJumpForwardActivated()
{
    ui->player_w->progress()->step(1);
}

void GUI_Player::onJumpBackwardActivated()
{
    ui->player_w->progress()->step(-1);
}

// This is called from the external world to update the position
void GUI_Player::setCurrentPosition(quint32 pos_sec)
{
    ui->player_w->progress()->setUi(pos_sec);
}

/** PROGRESS BAR END **/

/** PLAYER BUTTONS END **/



/** VOLUME **/
// Called e.g. after reading saved volume from settings: adjust ui and
// set audio volume.
void GUI_Player::setVolume(int pc)
{
    ui->player_w->volume()->set(pc);
}

// Called from audio when volume has been changed by another player.
void GUI_Player::setVolumeUi(int pc)
{
    ui->player_w->volume()->setUi(pc);
}

void GUI_Player::setMuteUi(bool ismute)
{
    ui->player_w->volume()->setMuteUi(ismute);
    m_trayIcon->setMute(ismute);
}

void GUI_Player::onVolumeStepActivated(int val)
{
    int vol_step = m_trayIcon->get_vol_step();
    ui->player_w->volume()->step(vol_step * val);
}

void GUI_Player::onVolumeHigherActivated()
{
    //qDebug() << "GUI_PLayer::volumeHigher";
    ui->player_w->volume()->volumeHigher();
}

void GUI_Player::onVolumeLowerActivated()
{
    //qDebug() << "GUI_PLayer::volumeLower";
    ui->player_w->volume()->volumeLower();
}

/** VOLUME END **/


