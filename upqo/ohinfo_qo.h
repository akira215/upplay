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
#ifndef _OHINFO_QO_INCLUDED
#define _OHINFO_QO_INCLUDED

#include <string>

#include <QObject>
#include <QThread>
#include <QDebug>

#include "libupnpp/control/ohinfo.hxx"
#include "ohpool.h"

class OHInfoQO : public QObject, public UPnPClient::VarEventReporter {
Q_OBJECT

public:
    OHInfoQO(UPnPClient::OHIFH ohp, QObject *parent = 0)
        : QObject(parent), m_srv(ohp) {
        m_srv->installReporter(this);
        qRegisterMetaType<UPnPClient::UPnPDirObject>("UPnPClient::UPnPDirObject");
    }

    virtual ~OHInfoQO() {
        m_srv->installReporter(0);
    }

    virtual void changed(const char *nm, int value) {
        Q_UNUSED(nm);Q_UNUSED(value);
        //qDebug() << "OHInfoQO: changed: " << nm << " (int): " << value;
    }
    virtual void changed(const char *nm, std::vector<int> ids) {
        Q_UNUSED(nm); Q_UNUSED(ids);
        //qDebug() << "OHInfoQO: changed: " << nm << " (vector<int>)";
    }
    virtual void changed(const char * nm, const char *value) {
        Q_UNUSED(nm);Q_UNUSED(value);
        //qDebug() << "OHInfoQO: changed: " << nm << " (char*): " << value;
    }
    virtual void changed(const char *nm, UPnPClient::UPnPDirObject dirent) {
        Q_UNUSED(nm);
        //qDebug() << "OHInfoQO: changed: " << nm << " (UPnPDirObject)";
        emit metaChanged(dirent);
    };

    // Read state from the remote. Used when starting up, to avoid
    // having to wait for events.
    virtual bool metatext(UPnPClient::UPnPDirObject *dirent) {
        int ret = m_srv->metatext(dirent);
        //qDebug() << "OHInfoQO::metatext: ret " << ret << " data " <<
        //    dirent->dump().c_str();
        return ret == 0;
    }

signals:
    void metaChanged(UPnPClient::UPnPDirObject);

private:
    UPnPClient::OHIFH m_srv;
};

#endif // _OHINFO_QO_INCLUDED
