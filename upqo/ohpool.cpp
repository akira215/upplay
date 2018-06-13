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

#include "ohpool.h"

#include <unordered_set>
#include <string>
#include <vector>

#include <QDebug>

#include "libupnpp/control/ohplaylist.hxx"
#include "libupnpp/control/ohradio.hxx"

using namespace std;

template <class T>
bool ohupdmetapool(const vector<int>& nids, int curid,
                   std::unordered_map<int, UPnPClient::UPnPDirObject>& metapool,
                   T srv)
{
    // Clean up metapool entries not in ids. We build a set with
    // the new ids list first. For small lists it does not matter,
    // for big ones, this will prevent what would otherwise be a
    // linear search the repeated search to make this
    // quadratic. We're sort of O(n * log(n)) instead.
    if (!metapool.empty() && !nids.empty()){
        std::unordered_set<int> tmpset(nids.begin(), nids.end());
        for (std::unordered_map<int, UPnPClient::UPnPDirObject>::iterator it
                 = metapool.begin(); it != metapool.end(); ) {
            if (tmpset.find(it->first) == tmpset.end()) {
                it = metapool.erase(it);
            } else {
                it++;
            }
        }
    }

    // Find ids for which we have no metadata. Always re-read current title
    vector<int> unids; // unknown
    for (vector<int>::const_iterator it = nids.begin(); 
         it != nids.end(); it++) {
        if (metapool.find(*it) == metapool.end() || curid == *it)
            unids.push_back(*it);
    }
    if (!unids.empty()) {
        //qDebug() << "OHPL::onIdArrayChanged: need metadata for: " 
        //       << ivtos(unids).c_str();
    }
    // Fetch needed metadata, 10 entries at a time
    const unsigned int batchsize(10);
    for (unsigned int i = 0; i < unids.size();) {
        unsigned int j = 0;
        vector<int> metaslice;
        for (; j < batchsize && (i+j) < unids.size(); j++) {
            metaslice.push_back(unids[i+j]);
        }

        //qDebug() << "OHPL::onIdArrayChanged: Requesting metadata for " 
        //<< vtos(metaslice).c_str();
        vector<UPnPClient::OHPlaylist::TrackListEntry> entries;
        int ret;
        if ((ret = srv->readList(metaslice, &entries))) {
            qDebug() << "OHPL: readList failed: " << ret;
            return false;
        }
        for (vector<UPnPClient::OHPlaylist::TrackListEntry>::iterator
                 it = entries.begin(); it != entries.end(); it++) {
            // Kazoo for example does not set a resource (uri)
            // inside the dirent.  Set it from the uri field in
            // this case.
            if (it->dirent.m_resources.empty()) {
                UPnPClient::UPnPResource res;
                res.m_uri = it->url;
                it->dirent.m_resources.push_back(res);
            }
            metapool[it->id] = it->dirent;
        }
        i += j;
    }

#if 0
    qDebug() << "Metadata Pool now: ";
    for (std::unordered_map<int, UPnPClient::UPnPDirObject>::const_iterator it
             =  metapool.begin(); it != metapool.end(); it++) {
        qDebug() << "Id " << it->first << "->\n" << it->second.dump().c_str();
    }
#endif
    return true;
}

template bool
ohupdmetapool<UPnPClient::OHPLH>
(const vector<int>&, int, 
 std::unordered_map<int, UPnPClient::UPnPDirObject>& ,
 UPnPClient::OHPLH);
template bool
ohupdmetapool<UPnPClient::OHRDH>
(const vector<int>&, int, 
 std::unordered_map<int, UPnPClient::UPnPDirObject>& ,
 UPnPClient::OHRDH);
