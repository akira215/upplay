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
#ifndef _OHVOLUME_QO_INCLUDED
#define _OHVOLUME_QO_INCLUDED

#include <string>

#include <QObject>
#include <QThread>
#include <QDebug>

#include "libupnpp/control/ohvolume.hxx"

class OHVolumeQO : public QObject, public UPnPClient::VarEventReporter {
    Q_OBJECT

public:
    OHVolumeQO(UPnPClient::OHVLH ohp, QObject *parent = 0)
        : QObject(parent), m_srv(ohp) {
        m_srv->installReporter(this);
    }
    virtual ~OHVolumeQO() {
        m_srv->installReporter(0);
    }

    virtual void changed(const char *nm, int value) {
        //qDebug() << "OHVL: Changed: " << nm << " (int): " << value;
        if (!strcmp(nm, "Volume")) {
            emit volumeChanged(value);
        } else if (!strcmp(nm, "Mute")) {
            emit muteChanged(value);
        }
    }
    virtual void changed(const char *, const char *) {
        //qDebug() << "OHTM: Changed: " << n << " (char*): " << v;
    }

public slots:
    void setVolume(int vol) {
        //qDebug() << "RDR::setVolume " << vol;
        m_srv->setVolume(vol);
    }
    void setMute(bool mute) {
        m_srv->setMute(mute);
    }
    int volume() {
        int vol;
        m_srv->volume(&vol);
        return vol;
    }
    bool mute()  {
        bool mt;
        m_srv->mute(&mt);
        return mt;
    }

signals:
    void volumeChanged(int);
    void muteChanged(bool);

private:
    UPnPClient::OHVLH m_srv;
};

#endif // _OHVOLUME_QO_INCLUDED
