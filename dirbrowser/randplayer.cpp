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
#include <cstdlib>

#include <QDebug>

#include <math.h>
#include <time.h>

#include "randplayer.h"
#include "HelperStructs/Helper.h"
#include "upadapt/upputils.h"

using namespace std;

static const unsigned int slicesize(30);

RandPlayer::RandPlayer(PlayMode mode,
                       const std::vector<UPnPClient::UPnPDirObject>& entries,
                       QObject *parent)
    : QObject(parent), m_playmode(mode), m_entries(entries)
{
    qsrand((unsigned int)time(0));
    if (m_playmode == PM_TRACKS) {
        std::random_shuffle(m_entries.begin(), m_entries.end());
    }
}
RandPlayer::~RandPlayer()
{
    emit sig_next_group_html("");
}

static bool sameValues(UPnPClient::UPnPDirObject& ref,
                       UPnPClient::UPnPDirObject& e)
{
    return !e.f2s("upnp:album", false).compare(ref.f2s("upnp:album", false)) &&
        !e.f2s("upplay:ctpath", false).compare(ref.f2s("upplay:ctpath", false));
}

// When playing by groups, if we just select random tracks and play
// the group they're in, big groups get selected more probably. So we
// random select in the group starts instead; And yes it's inefficient
// to recompute the group starts each time, we could do it once and
// then prune the list in parallel with the main list. But... you
// know... so many MIPS! We're only doing this every few minutes, and
// doing otherwise is not that simple (all posterior start indices
// need to be recomputed after we erase a group etc.)
static vector<unsigned int>
findGStarts(vector<UPnPClient::UPnPDirObject>& ents)
{
    vector<unsigned int> out;
    UPnPClient::UPnPDirObject ref;
    for (unsigned int i = 0; i < ents.size(); i++) {
        if (!sameValues(ref, ents[i])) {
            out.push_back(i);
            ref = ents[i];
        }
    }
    return out;
}

void RandPlayer::selectNextGroup()
{
    m_nextgroup.clear();
    if (m_entries.empty())
        return;

    vector<unsigned int> vgstarts = findGStarts(m_entries);

    // Pick a random start
    double fstart = (double(qrand()) / double(RAND_MAX)) *
        (vgstarts.size() - 1);
    int istart = vgstarts[round(fstart)];

    // Reference values
    auto entref = m_entries[istart];

    // Look back to beginning of section. Not needed any more now that
    // we pick up group starts. Just in case we change our minds
    while (istart > 0) {
        istart--;
        if (!sameValues(entref, m_entries[istart])) {
            istart++;
            break;
        }
    }

    // Look forward to end, and store entries. Could use the next
    // group index instead. Just kept the initial code where we
    // selected a random track
    vector<UPnPClient::UPnPDirObject>::iterator last =
        m_entries.begin() + istart;
    while (last != m_entries.end()) {
        if (!sameValues(entref, *last)) {
            break;
        }
        m_nextgroup.push_back(*last++);
    }
    // Erase used entries.
    m_entries.erase(m_entries.begin() + istart, last);
}

QString RandPlayer::groupHtml(std::vector<UPnPClient::UPnPDirObject>& ents)
{
    string html = "<i></i>";
    for (vector<UPnPClient::UPnPDirObject>::iterator it = ents.begin();
         it != ents.end(); it++) {
        html += string("<b>") + Helper::escapeHtml(it->m_title) + "</b><br />";
        html += Helper::escapeHtml(it->f2s("upnp:album", false)) + "<br />";
        html += Helper::escapeHtml(it->f2s("upnp:artist", false)) + "<br />";
    }
    return QString::fromUtf8(html.c_str());
}

void RandPlayer::playNextSlice()
{
    qDebug() << "RandPlayer: " << m_entries.size() << " remaining";

    if (m_entries.empty() && m_nextgroup.empty()) {
        emit sig_randplay_done();
        return;
    }

    vector<UPnPClient::UPnPDirObject> ents;

    if (m_playmode == PM_TRACKS) {
        // The tracks are already shuffle, just take the front slicesize
        vector<UPnPClient::UPnPDirObject>::iterator last =
            m_entries.size() > slicesize ? m_entries.begin() + slicesize :
            m_entries.end();

        ents.insert(ents.begin(), m_entries.begin(), last);
        m_entries.erase(m_entries.begin(), last);
    } else {
        if (m_nextgroup.empty()) {
            // 1st time: non-empty list, and empty next-group
            selectNextGroup();
            if (m_nextgroup.empty()) {
                // ?? This should really not happen !
                emit sig_randplay_done();
                return;
            }
        }

        ents = m_nextgroup;
        selectNextGroup();
        emit sig_next_group_html(groupHtml(m_nextgroup));
    }

    qDebug() << "RandPlayer: sending " << ents.size() << " entries to pl";
    MetaDataList mdl;
    mdl.resize(ents.size());
    for (unsigned int i = 0; i <  ents.size(); i++) {
        udirentToMetadata(&ents[i], &mdl[i]);
    }

    emit sig_tracks_to_playlist(mdl);
}
