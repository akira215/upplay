/* Copyright (C) 2014 J.F.Dockes
 *	 This program is free software; you can redistribute it and/or modify
 *	 it under the terms of the GNU General Public License as published by
 *	 the Free Software Foundation; either version 2 of the License, or
 *	 (at your option) any later version.
 *
 *	 This program is distributed in the hope that it will be useful,
 *	 but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	 GNU General Public License for more details.
 *
 *	 You should have received a copy of the GNU General Public License
 *	 along with this program; if not, write to the
 *	 Free Software Foundation, Inc.,
 *	 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _OHRECEIVER_QO_INCLUDED
#define _OHRECEIVER_QO_INCLUDED

#include <string>

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QTimer>

#include "libupnpp/control/ohreceiver.hxx"

// OpenHome Receiver service interface. Just for being able to start/stop
// for now.
class OHReceiverQO : public QObject, public UPnPClient::VarEventReporter {
Q_OBJECT

public:
    OHReceiverQO(UPnPClient::OHRCH ohp, QObject *parent = 0)
        : QObject(parent), m_srv(ohp), m_timer(0), m_errcnt(0) {
        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(testconn()));
        m_timer->start(2000);
        m_srv->installReporter(this);
    }

    virtual ~OHReceiverQO() {
        m_srv->installReporter(0);
    }

    virtual void changed(const char *nm, int value) {
        //qDebug() << "OHReceiverQO: changed: " << nm << " (int): " << value;
        if (!strcmp(nm, "TransportState")) {
            emit tpStateChanged(value);
        }
    }
    virtual void changed(const char * nm, const char *value) {
        Q_UNUSED(nm);
        Q_UNUSED(value);
        //qDebug() << "OHReceiverQO: changed: " << nm << " (char*): " << value;
    }

public slots:

    // Ping renderer to check it's still there.
    virtual void testconn() {
        UPnPClient::OHPlaylist::TPState val;
        if (m_srv->transportState(&val) != 0) {
            if (m_errcnt++ > 2) {
                emit connectionLost();
            }
            return;
        }
        m_errcnt = 0;
    }

    // Read state from the remote. Used when starting up.
    virtual void fetchState() {
        //qDebug() << "OHReceiverQO::fetchState()";
        UPnPClient::OHPlaylist::TPState tpst;
        if (m_srv->transportState(&tpst) == 0)
            emit tpStateChanged(tpst);
    }

    virtual bool play() {
        //qDebug() << "OHReceiverQO::play()";
        return m_srv->play() == 0;
    }

    virtual bool stop() {
        //qDebug() << "OHReceiverQO::stop()";
        return m_srv->stop() == 0;
    }

signals:
    void tpStateChanged(int);
    void connectionLost();
    
private:
    UPnPClient::OHRCH m_srv;
    QTimer *m_timer;
    int m_errcnt;
};

#endif // _OHRECEIVER_QO_INCLUDED
