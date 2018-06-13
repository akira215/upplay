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
#include "songcast.h"

#include <iostream>
#include <vector>

#include <QTimer>
#include <QDebug>

#include "libupnpp/control/linnsongcast.hxx"

#include "GUI/songcast/songcastdlg.h"
#include "upadapt/upputils.h"

using namespace std;
using namespace UPnPClient;
using namespace Songcast;

class SongcastTool::Internal {
public:
    SongcastDLG *dlg;
    vector<ReceiverState> receivers;
    vector<SenderState> senders;
};

SongcastTool::SongcastTool(SongcastDLG *dlg, QObject *parent)
    : QObject(parent)
{
    m = new Internal;
    m->dlg = dlg;
    connect(m->dlg, SIGNAL(sig_apply(SongcastDLG*)),
            this, SLOT(onSongcastApply()));
    connect(m->dlg, SIGNAL(accepted()), this, SLOT(onSongcastApply()));
    initControls();
}

void SongcastTool::initControls()
{
    listSenders(m->senders);
    listReceivers(m->receivers);
    m->dlg->createControls(m->senders.size(), m->receivers.size());
    for (unsigned int i = 0; i < m->senders.size(); i++) {
        connect(m->dlg->senderButton(i), SIGNAL(toggled(bool)),
                this, SLOT(enableOnButtons()));
    }
    syncReceivers();
}

SongcastDLG *SongcastTool::dlg()
{
    return m->dlg;
}

string SongcastTool::senderNameFromUri(const string& uri)
{
    for (unsigned int i = 0; i < m->senders.size(); i++) {
        if (uri == m->senders[i].uri)
            return m->senders[i].nm;
    }
    return string();
}

void SongcastTool::onSongcastApply()
{
    int senderidx = m->dlg->selectedSenderIdx();

    for (unsigned int i = 0; i < m->receivers.size(); i++) {
        if (m->dlg->receiverOffRequested(i)) {
            stopReceiver(m->receivers[i]);
        } else  if (senderidx != -1 && m->dlg->receiverOnRequested(i)) {
            setReceiverPlaying(m->receivers[i],
                               m->senders[senderidx].uri,
                               m->senders[senderidx].meta);
        }
    }
    QTimer::singleShot(500, this, SLOT(syncReceivers()));
}

// This is called when a sender is selected, to enable appropriate on buttons
void SongcastTool::enableOnButtons()
{
    int senderidx = m->dlg->selectedSenderIdx();

    for (unsigned int i = 0; i < m->receivers.size(); i++) {
        ReceiverState::SCState st(m->receivers[i].state);
        bool isplaying =  st ==  ReceiverState::SCRS_PLAYING;
        m->dlg->receiverOnButton(i)->setEnabled(!isplaying && senderidx >= 0);
    }
}

QString SongcastTool::receiverText(int i, bool isplaying)
{
    QString rcvdesc("<b>");
    rcvdesc += u8s2qs(m->receivers[i].nm + "</b>");
    string snm = senderNameFromUri(m->receivers[i].uri);
    if (!snm.empty()) {
        if (isplaying) {
            rcvdesc += tr(" (playing from: ");
        } else {
            rcvdesc += tr(" (connected to: ");
        }
        rcvdesc += u8s2qs(snm) + ")";
    }
    return rcvdesc;
}

// Synchronize ui state with actual receiver state. This will not list
// new devices or delete old ones. Also set the senders names (this is
// used right after creating the grids)
void SongcastTool::syncReceivers()
{
    int senderidx = m->dlg->selectedSenderIdx();

    for (unsigned int i = 0; i < m->senders.size(); i++) {
        m->dlg->senderLabel(i)->setText(u8s2qs(m->senders[i].nm));
    }
            
    for (unsigned int i = 0; i < m->receivers.size(); i++) {
        string udn = m->receivers[i].UDN;
        getReceiverState(udn, m->receivers[i], false);
        ReceiverState::SCState st(m->receivers[i].state);
        bool isplaying = st ==  ReceiverState::SCRS_PLAYING;

        m->dlg->receiverOffButton(i)->setChecked(false);
        m->dlg->receiverOnButton(i)->setChecked(false);
        m->dlg->receiverOffButton(i)->setEnabled(isplaying);
        m->dlg->receiverOnButton(i)->setEnabled(!isplaying && senderidx >= 0);
        m->dlg->receiverLabel(i)->setText(receiverText(i, isplaying));
    }
}
