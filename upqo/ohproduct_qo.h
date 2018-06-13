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
#ifndef _OHPRODUCT_QO_INCLUDED
#define _OHPRODUCT_QO_INCLUDED

#include <string>

#include <QObject>
#include <QThread>
#include <QDebug>

#include "libupnpp/control/ohproduct.hxx"

class OHProductQO : public QObject, public UPnPClient::VarEventReporter {
Q_OBJECT

public:
    OHProductQO(UPnPClient::OHPRH ohp, QObject *parent = 0)
        : QObject(parent), m_srv(ohp)
    {
        m_srv->installReporter(this);
        connect(this, SIGNAL(_sourceIndexChanged(int)),
                this, SLOT(onSourceIndexChanged(int)));
    }
    virtual ~OHProductQO() {
        m_srv->installReporter(0);
    }

    virtual void changed(const char *nm, int value)
    {
        //qDebug() << "OHProductQo: Changed: " << nm << " (int): " << value;
        if (!strcmp(nm, "SourceIndex")) {
            emit _sourceIndexChanged(value);
        } 
    }
    virtual void changed(const char *nm, const char *value) {
        Q_UNUSED(nm);
        Q_UNUSED(value);
        //qDebug() << "OHProductQO: Changed: " << nm << " (char*): " << value;
    }

    enum SourceType{OHPR_SourceUnknown, OHPR_SourcePlaylist, OHPR_SourceRadio,
                    OHPR_SourceReceiver};
    SourceType srcStringToType(const std::string& stype) {
        qDebug() << "OHProductQO::getSourceType: " << stype.c_str();
        if (!stype.compare("Playlist")) {
            return OHPR_SourcePlaylist;
        } else if (!stype.compare("Radio")) {
            return OHPR_SourceRadio;
        } else if (!stype.compare("Receiver")) {
            return OHPR_SourceReceiver;
        }
        return OHPR_SourceUnknown;
    }
        
    SourceType getSourceType() {
        int idx;
        std::vector<UPnPClient::OHProduct::Source> srcs;
        std::string st;
        if (sourceIndex(&idx) && getSources(srcs) &&
            idx >= 0 && idx < int(srcs.size())) {
            st = srcs[idx].type;
        }
        return srcStringToType(st);
    }
 
public slots:
    virtual bool sourceIndex(int *idxp) {
        return m_srv->sourceIndex(idxp) == 0;
    }
    virtual bool getSources(std::vector<UPnPClient::OHProduct::Source>& srcs) {
        return m_srv->getSources(srcs) == 0;
    }
    virtual bool setSourceIndex(int index) {
        return m_srv->setSourceIndex(index) == 0;
    }
    
signals:
    void sourceTypeChanged(OHProductQO::SourceType);

    // This is used internally. Use sourceTypeChanged instead
    void _sourceIndexChanged(int);

private slots:
    void onSourceIndexChanged(int idx) {
        std::vector<UPnPClient::OHProduct::Source> srcs;
        std::string st;
        if (getSources(srcs) && idx >= 0 && idx < int(srcs.size())) {
            st = srcs[idx].type;
        }
        emit sourceTypeChanged(srcStringToType(st));
    }
private:
    UPnPClient::OHPRH m_srv;
};

#endif // _OHPRODUCT_QO_INCLUDED
