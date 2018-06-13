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
#ifndef _OHTIME_QO_INCLUDED
#define _OHTIME_QO_INCLUDED

#include <string>

#include <QObject>
#include <QThread>
#include <QDebug>

#include "libupnpp/control/ohtime.hxx"

class OHTimeQO : public QObject, public UPnPClient::VarEventReporter {
Q_OBJECT

public:
    OHTimeQO(UPnPClient::OHTMH ohp, QObject *parent = 0)
        : QObject(parent), m_srv(ohp)
    {
        m_srv->installReporter(this);
    }
    virtual ~OHTimeQO() {
        m_srv->installReporter(0);
    }

    // TrackCount, Duration and Seconds
    virtual void changed(const char *nm, int value)
    {
        //qDebug() << "OHTM: Changed: " << nm << " (int): " << value;
        if (!strcmp(nm, "TrackCount")) {
            emit trackCountChanged(value);
        } else if (!strcmp(nm, "Duration")) {
            emit durationChanged(value);
        } else if (!strcmp(nm, "Seconds")) {
            emit secsInSongChanged(value);
        }
    }
    virtual void changed(const char *, const char *) {
        //qDebug() << "OHTM: Changed: " << n << " (char*): " << v;
    }

public slots:
    virtual bool time(UPnPClient::OHTime::Time& tm) {
        return m_srv->time(tm) == 0;
    }

signals:
    void trackCountChanged(int);
    void durationChanged(int);
    void secsInSongChanged(quint32);
                                         
private:
    UPnPClient::OHTMH m_srv;
};

#endif // _OHTIME_QO_INCLUDED
