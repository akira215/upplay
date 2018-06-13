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

#ifndef _OHRDADAPT_H_INCLUDED_
#define _OHRDADAPT_H_INCLUDED_

// Adapter for the ohradio service interface object, OHRadioQO. This
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

#include "upqo/ohradio_qo.h"
#include "upadapt/upputils.h"

using namespace UPnPP;

// Note: can't call this OHRadio because UPnPClient has it too (and we
// sometimes use the unqualified name)
class OHRad : public OHRadioQO {
Q_OBJECT

public:
    OHRad(UPnPClient::OHRDH ohrd, QObject *parent = 0)
        : OHRadioQO(ohrd, parent) {
        connect(this, SIGNAL(trackArrayChanged()),
                this, SLOT(translateMetaData()));
        connect(this, SIGNAL(tpStateChanged(int)), 
                this, SLOT(playerState(int)));
    }

private slots:

    void playerState(int ps) {
        std::string s;
        AudioState as = AUDIO_UNKNOWN;
        switch (ps) {
        case UPnPClient::OHPlaylist::TPS_Unknown:
        case UPnPClient::OHPlaylist::TPS_Buffering:
        default:
            s = "Unknown";
            break;
        case UPnPClient::OHPlaylist::TPS_Paused:
            as = AUDIO_PAUSED;
            s = "Paused";
            break;
        case UPnPClient::OHPlaylist::TPS_Playing:
            as = AUDIO_PLAYING;
            s = "Playing";
            break;
        case UPnPClient::OHPlaylist::TPS_Stopped:
            as = AUDIO_STOPPED;
            s = "Stopped";
            break;
        }
        //qDebug() << "OHRad::playerState: " << s.c_str();
        if (as != AUDIO_UNKNOWN) {
            emit audioStateChanged(as, s.c_str());
        }
    }

    void translateMetaData() {
        //qDebug() << "OHPlayer::translateMetaData()";
        MetaDataList mdv;
        for (std::vector<int>::iterator idit = m_idsv.begin(); 
             idit != m_idsv.end(); idit++) {
            std::unordered_map<int, UPnPClient::UPnPDirObject>::iterator poolit 
                = m_metapool.find(*idit);
            if (poolit == m_metapool.end()) {
                qDebug() << "OHPlayer::translateMetaData: "
                    "no data found for " << *idit << "!!!";
                continue;
            }
            UPnPClient::UPnPDirObject& ude = poolit->second;
            if (ude.m_resources.empty()) {
                LOGDEB("translateMetadata: no resources for:"  <<
                       ude.dump() << endl);
                //continue; does not seem to be an issue in fact. Only used
                // for avt. Funny, this is the exact same problem which bubble
                // had initially with ohpl tracks added through mpd (no uri)...
            }
            MetaData md;
            udirentToMetadata(&ude, &md);
            md.id = *idit;
            md.pl_playing = md.id == m_curid;
            
            mdv.push_back(md);
        }
        emit metadataArrayChanged(mdv);
    }


signals:
    void audioStateChanged(int as, const char *);
    void metadataArrayChanged(const MetaDataList& mdv);
};


#endif /* _OHRDADAPT_H_INCLUDED_ */
