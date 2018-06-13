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
#include <string>
#include <set>
using namespace std;

#include <QString>

#include <libupnpp/upnpavutils.hxx>
#include <libupnpp/control/cdircontent.hxx>

#include "HelperStructs/MetaData.h"
#include "upadapt/upputils.h"

using namespace UPnPClient;
using namespace UPnPP;

// Upnp field to qstring
static QString uf2qs(const UPnPDirObject *dop, const char *nm, bool isres)
{
    string val;
    bool ret = isres? dop->getrprop(0, nm, val) : dop->getprop(nm, val);
    if (ret) 
        return QString::fromUtf8(val.c_str());
    else
        return QString();
}

bool udirentToMetadata(const UPnPDirObject *dop, MetaData *mdp)
{
    if (dop == 0 || mdp == 0)
        return false;
    // Can't do anything with the ids, they have nothing to do with
    // the upnp id and parentid
    mdp->id = -1;
    mdp->artist_id = -1;
    mdp->album_id = -1;

    mdp->title = u8s2qs(dop->m_title);
    // qDebug() << "udirentToMetadata: title: " << mdp->title;
    mdp->artist = uf2qs(dop, "upnp:artist", false);
    mdp->album = uf2qs(dop, "upnp:album", false);
    mdp->rating = 0;
    QString dur = uf2qs(dop, "duration", true); 
    if (!dur.isEmpty()) {
        string sdur((const char*)dur.toUtf8());
        mdp->length_ms = upnpdurationtos(sdur) * 1000;
    } else {
        mdp->length_ms = 0;
    }
    // UPnP DIDL defines a "dc:date" property, YYYY-MM-DD,
    // but I've never seen it set...
    mdp->year = 0;
    if (dop->m_resources.size() == 0) {
        mdp->filepath = "";
    } else {
        mdp->filepath = 
            QString::fromLocal8Bit(dop->m_resources[0].m_uri.c_str());
    }
    mdp->track_num = uf2qs(dop, "upnp:originalTrackNumber", false).toInt();
    mdp->bitrate = uf2qs(dop, "bitrate", true).toInt();
    mdp->is_extern = false;
    mdp->filesize = uf2qs(dop, "size", true).toLongLong();;
    mdp->comment = "";
    mdp->discnumber = 0;
    mdp->genres << uf2qs(dop, "upnp:genre", false);
    mdp->n_discs = -1;
    mdp->is_extern = false;
    mdp->pl_selected = false;
    mdp->pl_dragged = false;
    mdp->pl_playing = false;
    mdp->unused1 = false;
    mdp->is_disabled = false;
    mdp->didl = u8s2qs(dop->getdidl());
    mdp->albumArtURI = uf2qs(dop, "upnp:albumArtURI", false);
    return true;
}

static std::set<string> specfields{"upnp:artist", "upnp:album", "upnp:genre",
        "upnp:albumArtURI", "upnp:originalTrackNumber"};

void metaDataToHtml(const MetaData* metap, QString& html)
{
    // We reparse from didl because we want to catch everything, even
    // what does not go into the MetaData fields. Also it may happen
    // that we have something in MetaData which does not come from the
    // Didl data
    
    UPnPDirContent dir;
    UPnPDirObject dirent;
    bool okdidl(true);
    // qDebug() << "metaDataToHtml: DIDL: " << metap->didl;
    if (metap->didl.isEmpty() || !dir.parse(qs2utf8s(metap->didl)) ||
        dir.m_items.empty()) {
        okdidl = false;
        //qDebug() << "metaDataToHtml: no DIDL data or no items";
    } else {
        //qDebug() << "metaDataToHtml: DIDL parsed ok";
        dirent = dir.m_items[0];
    }
    html = "";
    QString val;

    // Some fields are treated specially (see specfields list above),
    // because we want them in order.
    val = okdidl ? u8s2qs(dirent.m_title) : metap->title;
    if (!val.isEmpty()) html += "<p>" + val + "</p>";

    val = okdidl ? u8s2qs(dirent.f2s("upnp:artist", false)) : metap->artist;
    if (!val.isEmpty()) html += "<p>" + val + "</p>";
    
    val = okdidl ? u8s2qs(dirent.f2s("upnp:album", false)) : metap->album;
    if (!val.isEmpty()) html += "<p>" + val + "</p>";

    val = okdidl ? u8s2qs(dirent.f2s("upnp:genre", false)) :
        (metap->genres.size() ? metap->genres[0]: QString());
    if (!val.isEmpty()) html += "<b>Genre: </b>" + val + "<br/>";
    val = okdidl?u8s2qs(dirent.f2s("upnp:originalTrackNumber", false)) : "";
    if (!val.isEmpty()) html += "<b>Original track number:</b> " + val + "<br/>";

    if (okdidl) {
        // Dump all the rest
        for (auto it : dirent.m_props) {
            if (specfields.find(it.first) == specfields.end() &&
                !it.second.empty()) {
                html += "<b>" + u8s2qs(it.first) + "</b> : " +
                    u8s2qs(it.second) + "<br/>";
            }
        }
        val = u8s2qs(dirent.f2s("upnp:albumArtURI", false));
        if (!val.isEmpty()) html += "<b>Album Art URI</b>: " + val + "<br/>";

        for (auto resit : dirent.m_resources) {
            html += "<p><b>Resource URI:</b> " + u8s2qs(resit.m_uri) + "<br/>";
            html += "<blockquote>";
            for (auto it : resit.m_props) {
                html += "<b>" + u8s2qs(it.first) + "</b> : " +
                    u8s2qs(it.second) + "<br/>";
            }
            html += "</blockquote></p>";
        }
    }
    // qDebug() << "metaDataToHtml: html now: " << html;
}

