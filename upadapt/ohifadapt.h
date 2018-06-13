/* Copyright (C) 2015 J.F.Dockes
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

#ifndef _OHIFADAPT_H_INCLUDED_
#define _OHIFADAPT_H_INCLUDED_

// Adapter for the ohinfo service interface object, OHInfoQO. This
// interfaces to the libupnpp thready interface, and translates to Qt
// signals, using still UPNPish data. We translate the data to upplay
// formats.

#include <string>
#include <iostream>
using namespace std;

#include <QDebug>
#include <QApplication>

#include "HelperStructs/MetaData.h"
#include "HelperStructs/globals.h"

#include "upqo/ohinfo_qo.h"
#include "upadapt/upputils.h"

using namespace UPnPP;

// Note: can't call this OHInfo because UPnPClient has it too (and we
// sometimes use the unqualified name)
class OHInf : public OHInfoQO {
Q_OBJECT

public:
    OHInf(UPnPClient::OHIFH ohrd, QObject *parent = 0)
        : OHInfoQO(ohrd, parent) {
        connect(this, SIGNAL(metaChanged(UPnPClient::UPnPDirObject)),
                this, SLOT(translateMeta(UPnPClient::UPnPDirObject)));
    }

    bool metatext(MetaData& md) {
        UPnPClient::UPnPDirObject dirent;
        if (OHInfoQO::metatext(&dirent)) {
            udirentToMetadata(&dirent, &md);
            return true;
        }
        qDebug() << "OHInf::metatext: action failed";
        return false;
    }

signals:
    void metatextChanged(const MetaData&);

private slots:

    void translateMeta(UPnPClient::UPnPDirObject dirent) {
        //qDebug() << "OHInfo:translateMetaData()";
        MetaData md;
        udirentToMetadata(&dirent, &md);
        emit metatextChanged(md);
    }
};


#endif /* _OHIFADAPT_H_INCLUDED_ */
