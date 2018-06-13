/* Copyright (C) 2014 J.F.Dockes
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
#ifndef _AVTRANSPORT_QO_INCLUDED
#define _AVTRANSPORT_QO_INCLUDED

#include <string>

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QTimer>

#include "libupnpp/control/avtransport.hxx"
#include "libupnpp/control/cdircontent.hxx"

using namespace UPnPClient;

class AVTMetadata {
public:
    virtual std::string getDidl() const = 0;
};

class AVTransportQO : public QObject, public UPnPClient::VarEventReporter {
    Q_OBJECT;

public:
    AVTransportQO(UPnPClient::AVTH avt, QObject *parent = 0)
        : QObject(parent), m_srv(avt), m_timer(0), m_errcnt(0),
          m_cursecs(-1),
          m_sent_end_of_track_sig(false),
          m_in_ending(false),
          m_tpstate(AVTransport::Unknown) {

        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
        m_timer->start(1000);

        qRegisterMetaType<UPnPClient::UPnPDirObject>
            ("UPnPClient::UPnPDirObject");

        m_srv->installReporter(this);

        // We are handling the playlist: set the renderer in "normal"
        // mode. Actually, I don't think that this is relevant at all,
        // we never consider that the renderer may have an internal
        // playlist.
        m_srv->setPlayMode(UPnPClient::AVTransport::PM_Normal);
    }

    virtual ~AVTransportQO() {
        m_srv->installReporter(0);
    }

    const char *tpstatetostr(int tps) {
        switch (tps) {
        default:
        case UPnPClient::AVTransport::Unknown:
            return "Unknown";
        case UPnPClient::AVTransport::Stopped:
            return "Stopped";
        case UPnPClient::AVTransport::Playing:
            return "Playing";
        case UPnPClient::AVTransport::Transitioning:
            return "Transitionning";
        case UPnPClient::AVTransport::PausedPlayback:
            return "PausedPlay";
        case UPnPClient::AVTransport::PausedRecording:
            return "PausedRecord";
        case UPnPClient::AVTransport::Recording:
            return "Recording";
        case UPnPClient::AVTransport::NoMediaPresent:
            return "No Media";
        }
    }

    virtual void changed(const char *nm, int value) {
        if (!strcmp(nm, "CurrentTrackDuration")) {
            // This is normally part of LastChange? but some renderers
            // apparently don't send it (bubble?). So use the value
            // from GetPositionInfo
            //qDebug() << "AVT: Changed: " << nm << " (int): " << value;
            m_cursecs = value;
        } else if (!strcmp(nm, "TransportState")) {
            //qDebug() << "AVT: Changed: " << nm << " " << tpstatetostr(value);
            m_tpstate = AVTransport::TransportState(value);
            emit tpStateChanged(value);
            if (m_in_ending &&
                (value == UPnPClient::AVTransport::Stopped ||
                 value == UPnPClient::AVTransport::NoMediaPresent)) {
                m_in_ending = false;
                qDebug() << "AVT EVT: emitting stoppedAtEOT";
                emit stoppedAtEOT();
            }
        } else if (!strcmp(nm, "CurrentTransportActions")) {
            //qDebug() << "AVT: Changed: " << nm << " (int): " << value;
            emit tpActionsChanged(value);
        }
    }

    // Note about uri change detection:
    // We use a changed uri to reset the state for sending out a setNextURI
    //
    // This fails if 2 identical URIs are consecutive in the track
    // list. The consequence in this situation is that, if there is a
    // 3rd track and it should be gapless with the second, this won't
    // happen, there will be a stop/start. This does not seem like a
    // big deal.
    //
    // This could be mitigated by adding a time based track-change
    // detection (there is infortunately no sequential "tracks played"
    // counter in UPnP). If the time goes back more than a few seconds
    // without a command from us, then we have a track change.
    virtual void changed(const char *nm, const char *value) {
        //qDebug() << "AVT: Changed: " << nm << " (char*): " << value;
        if (!strcmp(nm, "AVTransportURI")) {
            if (m_cururi.compare(value) &&
                    (m_tpstate == UPnPClient::AVTransport::Playing ||
                     m_tpstate == UPnPClient::AVTransport::Transitioning ||
                     m_tpstate == UPnPClient::AVTransport::PausedPlayback)) {
                //qDebug() << "AVT: ext track change: cur [" << m_cururi.c_str()
                //         << "] new [" << value << "]";
                setcururi(value);
                emit newTrackPlaying(QString::fromUtf8(value));
            }
        }
    }

    virtual void changed(const char *nm, UPnPClient::UPnPDirObject meta) {
        if (!strcmp(nm, "AVTransportURIMetaData")) {
            qDebug() << "AVT: Changed: " << nm << " (dirc): " <<
                     meta.dump().c_str();
            // Don't use this if no resources are set. XBMC/Kodi does
            // this for some reason. Else we'd end-up with
            // resource-less unplayable entries in the
            // playlist. Scheduling a state update is not useful
            // either because the data will have the same
            // problem. Kodi only emits useful metadata when
            // explicitely told to switch tracks (not even at regular
            // track changes).
            if (!meta.m_resources.empty() &&
                    (m_tpstate == UPnPClient::AVTransport::Playing ||
                     m_tpstate == UPnPClient::AVTransport::PausedPlayback)) {
                emit currentMetadata(meta);
            }
        }
    }

public slots:

    virtual void play() {
        m_srv->play();
    }
    virtual void stop() {
        setcururi("");
        m_in_ending = false;
        m_srv->stop();
    }
    virtual void pause() {
        m_srv->pause();
    }

    virtual void changeTrack(const std::string& uri, const AVTMetadata* md) {
        qDebug() << "AVT:changeTrack: " << uri.c_str();
        m_srv->setAVTransportURI(uri, md->getDidl());
        // Don't do this: wait for the renderer data, else we risk
        // flickering if an event reports the old track.
        // setcururi(uri);
    }

    virtual void prepareNextTrack(const std::string& uri,
                                  const AVTMetadata* md) {
        qDebug() << "AVT:prepareNextTrack: " << uri.c_str();
        m_srv->setNextAVTransportURI(uri, md->getDidl());
    }

    // Seek to point. Parameter in seconds
    virtual void seek(int secs) {
        qDebug() << "AVT: seek to " << secs << " S. m_cursecs " << m_cursecs;
        m_srv->seek(UPnPClient::AVTransport::SEEK_REL_TIME, secs);
    }

    virtual void fetchState() {
        update(true);
    }

    // Called by timer every sec
    virtual void update(bool force = false) {
        UPnPClient::AVTransport::PositionInfo info;
        int error;
        if ((error = m_srv->getPositionInfo(info)) != 0) {
            qDebug() << "getPositionInfo failed with error " << error;
            if (m_errcnt++ > 4) {
                emit connectionLost();
            }
            return;
        }
        m_errcnt = 0;
        m_cursecs = info.trackduration;

        UPnPClient::AVTransport::TransportInfo tinfo;
        if ((error = m_srv->getTransportInfo(tinfo)) != 0) {
            qDebug() << "getTransportInfo failed with error " << error;
            return;
        }

        //qDebug() << "AVT: update: posinfo: reltime " << info.reltime <<
        //    " tdur " << info.trackduration << " meta " <<
        //    info.trackmeta.dump().c_str();

        if (m_tpstate == UPnPClient::AVTransport::Playing) {
            // Time-related stuff
            emit secsInSongChanged(info.reltime);
            if (m_cursecs > 0) {
                if (info.reltime > m_cursecs - 10) {
                    if (!m_sent_end_of_track_sig) {
                        qDebug() << "AVT: Emitting endOfTrackIsNear()";
                        emit endOfTrackIsNear();
                        m_sent_end_of_track_sig = true;
                    }
                    m_in_ending = true;
                } else if (info.reltime > 0 && info.reltime < 5) {
                    // This is for the case where we are playing 2
                    // consecutive identical URIs: heuristic try to detect
                    // the change
                    if (m_in_ending == true) {
                        qDebug() << "AVT: was in end, seeing start: trkswitch";
                        setcururi(info.trackuri);
                        emit newTrackPlaying(u8s2qs(info.trackuri));
                    }
                }
            }
        }
        
        if (force || tinfo.tpstate != m_tpstate) {
            qDebug() << "AVT: emitting tpStateChanged:" <<
                tpstatetostr(tinfo.tpstate);
            emit tpStateChanged(tinfo.tpstate);
            m_tpstate = tinfo.tpstate;
        }
        if (force || (m_cururi.compare(info.trackuri) &&
                      (m_tpstate == UPnPClient::AVTransport::Playing ||
                       m_tpstate == UPnPClient::AVTransport::Transitioning ||
                       m_tpstate == UPnPClient::AVTransport::PausedPlayback))) {
            qDebug() << "AVT: update: ext track change: cur [" <<
                     m_cururi.c_str() << "] new [" <<
                     info.trackuri.c_str() << "]";
            setcururi(info.trackuri);
            if (!info.trackmeta.m_resources.empty()) {
                // Don't emit bogus meta which would unplayable
                // entries in the playlist. The only moment when Kodi
                // will emit usable metadata is when told to change
                // tracks. MediaInfo returns the same data.
                qDebug() << "AVT: emitting currentMetadata";
                emit currentMetadata(info.trackmeta);
            }
            qDebug() << "AVT: emitting newTrackPlaying:" <<
                u8s2qs(info.trackuri);
            emit newTrackPlaying(u8s2qs(info.trackuri));
        }
        if (m_in_ending &&
            (tinfo.tpstate == UPnPClient::AVTransport::Stopped ||
             tinfo.tpstate == UPnPClient::AVTransport::NoMediaPresent)) {
            m_in_ending = false;
            qDebug() << "AVT: emitting stoppedAtEOT";
            emit stoppedAtEOT();
        }
    }

signals:
    void secsInSongChanged(quint32);
    void endOfTrackIsNear();
    void newTrackPlaying(QString);
    void tpStateChanged(int);
    void tpActionsChanged(int);
    void stoppedAtEOT();
    void currentMetadata(UPnPClient::UPnPDirObject);
    void connectionLost();

private:
    UPnPClient::AVTH m_srv;
    QTimer *m_timer;
    int m_errcnt;
    int m_cursecs;
    bool m_sent_end_of_track_sig;
    bool m_in_ending;
    std::string m_cururi;
    AVTransport::TransportState m_tpstate;

    void setcururi(const std::string& uri) {
        qDebug() << "setcururi: " << uri.c_str();
        m_cururi = uri;
        m_sent_end_of_track_sig = false;
        if (uri != "") {
            // Don't reset m_in_ending if uri is null: we often get an
            // uri change to "" before we get the transport state
            // change event. Resetting m_in_ending would prevent the
            // emission of the stoppedAtEOT signal
            m_in_ending = false;
        }
    }
    QString u8s2qs(const std::string us) {
        return QString::fromUtf8(us.c_str());
    }

};

#endif // _AVTRANSPORT_QO_INCLUDED
