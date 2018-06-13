/* Copyright (C) 2016 J.F.Dockes
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
#include <string>

#include <QProcess>
#include <QSettings>
#include <QDebug>

#include "upadapt/upputils.h"
#include "utils/smallut.h"
#include "notifications.h"
#include "audioscrobbler.h"

using namespace std;

UpplayNotifications::UpplayNotifications(QObject *parent)
    : QObject(parent), m_scrobbler(0), m_playing(false)
{
}
void UpplayNotifications::onStopped()
{
    m_playing = false;
}
void UpplayNotifications::onPaused()
{
    m_playing = false;
}
void UpplayNotifications::onPlaying()
{
    m_playing = true;
}
void UpplayNotifications::songProgress(quint32 secs)
{
    if (m_scrobbler) {
        m_scrobbler->maybeScrobble(secs, m_prevmeta);
    }
}

void UpplayNotifications::notify(const MetaData& meta)
{
//    qDebug() << "UpplayNotifications::notify: playing " << m_playing <<
//        "title" << meta.title << "artist" << meta.artist <<
//        "prevtit" << m_prevmeta.title << "prevart" << m_prevmeta.artist;
    if (meta.title == "") {
        return;
    }
    if (meta.title == m_prevmeta.title && 
        meta.artist == m_prevmeta.artist) {
        //qDebug() << "same difference";
        return;
    }

    bool prevempty = m_prevmeta.title == "" && m_prevmeta.artist == "";
    m_prevmeta = meta;

    QSettings settings;
    if (settings.value("lastfmscrobble").toBool()) {
        if (m_scrobbler == 0) {
            m_scrobbler = new AudioScrobbler(this);
        }
    } else {
        if (m_scrobbler) {
            delete m_scrobbler;
            m_scrobbler = 0;
        }
    }
    
    if (!settings.value("shownotifications").toBool()) {
        return;
    }
    if (prevempty || !m_playing) {
        // Only notify on an actual track change, not when we just
        // start up and a song is playing.
        return;
    }

#ifndef _WIN32
    bool usenotificationcmd = settings.value("usenotificationcmd").toBool();
    if (usenotificationcmd) {
        // Retrieve notification command as string list
        QString cmdstring = settings.value("notificationcmd").toString();
        if (cmdstring.isEmpty()) {
            qDebug() << "UpplayNotifications::notify: no cmd";
            return;
        }
        vector<string> cmdlist;
        stringToStrings(qs2utf8s(cmdstring), cmdlist);
        if (cmdlist.empty()) {
            qDebug() << "UpplayNotifications::notify: cmd list empty??";
            return;
        }

        QString command = u8s2qs(cmdlist[0]);
        QStringList qlist;
        if (cmdlist.size() > 1) {
            for (unsigned int i = 1; i < cmdlist.size(); i++) {
                qlist.push_back(u8s2qs(cmdlist[i]));
            }
        }

        QString msg = meta.title + " / " + meta.artist;
        qlist.push_back(msg);
        //qDebug() << "UpplayNotifications::notify: starting cmd " << command;
        QProcess::startDetached(command, qlist, "/tmp");
        return;
    }
#endif

    // Else, or always on windows, tell the gui to do its thing
    emit notifyNeeded(meta);
    return;
}
