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
#ifndef _AUDIOSCROBBLER_H_INCLUDED_
#define _AUDIOSCROBBLER_H_INCLUDED_

#include <QObject>
#include <QString>
#include <QHash>

#include <string>

#include "HelperStructs/MetaData.h"

class QNetworkAccessManager;
class QNetworkReply;
class QByteArray;

// Send notifications to Last.FM
// This creates a QNetworkAccessManager, which is normally unique per
// app.  If there other uses for the manager appear in upplay, this
// should be changed for the manager to be created by the app and
// passed as constructor parameter.
//
// The Scrobbler code proper was created using code from MPDAS, by
// Henrik Friedrichsen, modified to use the Qt library instead of
// libcurl.
class AudioScrobbler : public QObject {
    Q_OBJECT;

public:
    AudioScrobbler(QObject *parent = 0);

public slots:
    void maybeScrobble(time_t cursecs, const MetaData& meta);

private slots:
    void replyFinished(QNetworkReply*);
    
private:

    void maybeQueue();
    void processQueue();
    void scrobbleFailure();
    bool scrobble(const MetaData&);
    bool nowPlaying(const MetaData&);
    bool loveTrack(const MetaData& meta);
    void handshake();
    void openURL(const QString& url, const char *data = 0);
    std::string createScrobbleMessage(const MetaData& meta);
    bool checkFailure(std::string response);
    
    // Current song playing. maybeScrobble uses this to decide when to
    // queue data for scrobbling
    MetaData m_curmeta;
    time_t   m_curpos;    // current position in song
    time_t   m_starttime; // time started playing
    bool     m_sent;
    
    // Songs which need scrobbling. The queue can grow if Last.FM is
    // unaccessible for a time, but we don't save it to disk for now
    // (unlike MPDAS).
    QHash<QString, MetaData> m_queue;

    // Last try processing the queue. We wait 5mn after a failure
    time_t m_lastqfail;
    // Current error count. We try to reconnect after 3
    int m_failcount;
    // Logged in ?
    bool m_authed;
    std::string m_sessionid;
    QNetworkAccessManager *m_netman;
    bool m_netactive;
};

#endif /* _AUDIOSCROBBLER_H_INCLUDED_ */
