/* Copyright (C) 2014 J.F.Dockes
 *       This program is free software; you can redistribute it and/or modify
 *       it under the terms of the GNU General Public License as published by
 *       the Free Software Foundation; either version 2 of the License, or
 *       (at your option) any later version.
 *
 *       This program is distributed in the hope that it will be useful,
 *       but WITHOUT ANY WARRANTY; without even the implied warranty of
 *       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *       GNU General Public License for more details.
 *
 *       You should have received a copy of the GNU General Public License
 *       along with this program; if not, write to the
 *       Free Software Foundation, Inc.,
 *       59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "rreaper.h"

#include <list>
#include <queue>
#include <iostream>
#include <unordered_set>

#include <QDebug>


using namespace std;

struct CtDesc {
    CtDesc(const string &objid, const string &path)
        : m_objid(objid), m_ttpath(path) {
    }
    string m_objid;
    string m_ttpath;
};

class RecursiveReaper::Internal {
public:
    Internal()
        : status(UPNP_E_SUCCESS), cancel(false) {
    }
    UPnPClient::CDSH serv;
    queue<CtDesc> ctobjids;
    std::unordered_set<string> allctobjids;
    int status;
    bool cancel;
};

static const char cchar_pthsep = '\x01';

string RecursiveReaper::ttpathPrintable(const std::string in)
{
    string out;
    for (unsigned int i = 0;i < in.size(); i++) {
        if (in[i] == cchar_pthsep) {
            out += "||";
        } else {
            out += in[i];
        }
    }
    return out;
}

RecursiveReaper::RecursiveReaper(UPnPClient::CDSH server, string objid, 
                                 QObject *parent)
    : QThread(parent)
{
    m = new Internal;
    m->serv = server;
    m->ctobjids.push(CtDesc(objid, ""));
    m->allctobjids.insert(objid);
}

RecursiveReaper::~RecursiveReaper()
{
    delete m;
}

void RecursiveReaper::run()
{
    //qDebug() << "RecursiveReaper::run";
    m->status = UPNP_E_SUCCESS;
    while (!m->ctobjids.empty()) {
        if (m->cancel) {
            qDebug() << "RecursiveReaper:: cancelled";
            break;
        }
        // We don't stop on a container scan error, minimserver for one 
        // sometimes has dialog hiccups with libupnp, this is not fatal.
        scanContainer(&m->ctobjids.front());
        m->ctobjids.pop();
    }
    emit done(m->status);
    //qDebug() << "RecursiveReaper::done";
}

void RecursiveReaper::setCancel()
{
    m->cancel = true;
}

bool RecursiveReaper::scanContainer(const CtDesc* ctdesc)
{
    //qDebug() << "RecursiveReaper::scanCT: objid:" << objid.c_str();

    int offset = 0;
    int toread = 20; // read small count the first time
    int total = 1000;// Updated on first read.
    int count;

    while (offset < total) {
        if (m->cancel) {
            qDebug() << "RecursiveReaper:: cancelled";
            break;
        }
        UPnPClient::UPnPDirContent& slice = 
            *(new UPnPClient::UPnPDirContent());

        // Read entries
        m->status = m->serv->readDirSlice(ctdesc->m_objid, offset, toread,
                                          slice,  &count, &total);
        if (m->status != UPNP_E_SUCCESS) {
            return false;
        }
        offset += count;

        // Put containers aside for later exploration
        for (vector<UPnPClient::UPnPDirObject>::iterator it = 
                 slice.m_containers.begin();
             it != slice.m_containers.end(); it++) {
            if (it->m_title.empty())
                continue;
            if (m->serv->getKind() == 
                UPnPClient::ContentDirectory::CDSKIND_MINIM &&
                it->m_title.size() >= 2 && it->m_title[0] == '>' && 
                it->m_title[1] == '>') {
                continue;
            }
            if (m->allctobjids.find(it->m_id) != m->allctobjids.end()) {
                qDebug() << "scanContainer: loop detected";
                continue;
            }
            //qDebug()<< "scanContainer: pushing objid " << it->m_id.c_str()
            // << " title " << it->m_title.c_str();
            m->allctobjids.insert(it->m_id);
            m->ctobjids.push(CtDesc(it->m_id,
                                    ctdesc->m_ttpath + cchar_pthsep +
                                    it->m_title));
        }
        slice.m_containers.clear();

        // Make items available
        if (!slice.m_items.empty()) {
            //qDebug() << "RecursiveReaper::scanCT got " << 
            //    slice.m_items.size() << " items";
            for (vector<UPnPClient::UPnPDirObject>::iterator it = 
                     slice.m_items.begin();
                 it != slice.m_items.end(); it++) {
                it->m_props["upplay:ctpath"] = ctdesc->m_ttpath;
            }
            emit sliceAvailable(&slice);
        }
        toread = m->serv->goodSliceSize();
    }
    
    return true;
}
