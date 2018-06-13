/* Copyright (C) 2011  Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <sys/stat.h>
#include <sys/types.h>

#include <ctime>

#include <QList>
#include <QDebug>
#include <QDir>
#include <QFileDialog>

#include "upadapt/upputils.h"
#include "playlist.h"
#include "HelperStructs/Helper.h"


using namespace std;

Playlist::Playlist(QObject* parent) 
    : QObject (parent), m_play_idx(-1), m_selection_min_row(-1),
      m_insertion_point(-1), m_tpstate(AUDIO_UNKNOWN)

{
    m_play_idx = -1;
    m_selection_min_row = -1;
    m_tpstate = AUDIO_UNKNOWN;
}

// GUI -->
void Playlist::psl_clear_playlist()
{
    m_meta.clear();
    m_play_idx = -1;
    m_selection_min_row = -1;
    m_insertion_point = -1;
    psl_clear_playlist_impl();
}

// Remove one row
void Playlist::remove_row(int row)
{
    //qDebug() << "Playlist::remove_row";
    QList<int> remove_list;
    remove_list << row;
    psl_remove_rows(remove_list);
}

void Playlist::onRemoteTpState(int tps, const char *sst)
{
//    qDebug() << "Playlist::onRemoteTpState " << s <<
//        " play_idx " << m_play_idx;
//    if (m_play_idx >= 0 && m_play_idx < int(m_meta.size())) 
//        qDebug() << "     meta[idx].pl_playing " << 
//            m_meta[m_play_idx].pl_playing;

    m_tpstate = tps;
    switch (tps) {
    case AUDIO_UNKNOWN:
    case AUDIO_STOPPED:
    default:
        //qDebug() << "Playlist::onRemoteTpState: STOPPED";
        emit sig_stopped();
        break;
    case AUDIO_PLAYING:
        //qDebug() << "Playlist::onRemoteTpState: PLAYING";
        emit sig_playing();
        break;
    case AUDIO_PAUSED:
        //qDebug() << "Playlist::onRemoteTpState: PAUSED";
        emit sig_paused();
        break;
    }
    onRemoteTpState_impl(tps, sst);
}

Playlist_Mode Playlist::mode()
{
    return CSettingsStorage::getInstance()->getPlaylistMode();
}

// GUI -->
void Playlist::psl_change_mode(const Playlist_Mode& mode)
{
    CSettingsStorage::getInstance()->setPlaylistMode(mode);
    emit sig_mode_changed(mode);
}

void Playlist::get_metadata(MetaDataList& out)
{
    out = m_meta;
}

class MetaDataCmp {
public:
    enum SortCrit {SC_TIT, SC_ART, SC_ALB, SC_Y, SC_FN, SC_TNO};

    MetaDataCmp(const vector<SortCrit>& crits)
        : m_crits(crits) {}
    bool operator()(const MetaData& o1, const MetaData& o2) {
        int rel;
        QString s1, s2;
        for (unsigned int i = 0; i < m_crits.size(); i++) {
            SortCrit crit = m_crits[i];
            rel = obgetprop(o1, crit, s1).compare(obgetprop(o2, crit, s2));
            if (rel < 0)
                return true;
            else if (rel > 0)
                return false;
        }
        return false;
    }
    const QString& obgetprop(const MetaData& o, SortCrit crit, QString& stor) {

        qint32 val;
        switch(crit) {
        case SC_TIT: return o.title;
        case SC_ART: return o.artist;
        case SC_ALB: return o.album;
        case SC_FN: return o.filepath;
        case SC_Y: val = o.year; goto valtostr;
        case SC_TNO: val = o.track_num;
        valtostr: {
                char num[30];
                sprintf(num, "%010d", int(val));
                stor = num;
                return stor;
            } 
        default:
            return nullstr;
        }
    }

    vector<SortCrit> m_crits;
    static QString nullstr;
};
QString MetaDataCmp::nullstr;

static void mdsort(const MetaDataList& inlist, MetaDataList& outlist,
                   const vector<MetaDataCmp::SortCrit> crits)
{
    MetaDataCmp cmpo(crits);
    outlist = inlist;
    std::sort(outlist.begin(), outlist.end(), cmpo);
}

void Playlist::psl_sort_by_tno()
{
    vector<MetaDataCmp::SortCrit> crits;
    // Makes no sense to sort by tno independantly of album
    crits.push_back(MetaDataCmp::SC_ALB);
    crits.push_back(MetaDataCmp::SC_TNO);
    MetaDataList md;
    mdsort(m_meta, md, crits);
    psl_clear_playlist();
    psl_insert_tracks(md, -1);
}

void Playlist::psl_add_tracks(const MetaDataList& v_md)
{
    qDebug() << "Playlist::psl_add_tracks: " <<
        " ninserttracks " <<  v_md.size() << 
        " m_meta.size() " << m_meta.size();

    emit sig_sync();

    Playlist_Mode playlist_mode = CSettingsStorage::getInstance()->getPlaylistMode();
    if (playlist_mode.replace)
        psl_clear_playlist();

    int afteridx = -1;

    if (playlist_mode.append) {
        afteridx = m_meta.size() - 1;
    } else {
        if (m_insertion_point >= 0) {
            afteridx = m_insertion_point;
        } else if (m_play_idx >= 0) {
            afteridx = m_play_idx;

            // } else if (m_selection_min_row >= 0) { 
            // afteridx = m_selection_min_row;
        } else {
            // The more natural thing to do if neither playing nore
            // selection is to append. Else clicking on several tracks
            // inserts them in reverse order
            afteridx = m_meta.size() - 1;
        }
    }

    psl_insert_tracks(v_md, afteridx);
    m_insertion_point = afteridx + 1;

    if (playlist_mode.playAdded) {
        psl_change_track(afteridx+1);
        psl_play();
    }

    emit sig_insert_done();
}

static string maybemakesavedir()
{
    QDir topdata(Helper::getHomeDataPath());
    if (!topdata.exists("pl") && !topdata.mkdir("pl")) {
        qDebug() << "Could not create: " << topdata.absoluteFilePath("pl");
    }
    return qs2utf8s(topdata.absoluteFilePath("pl"));
}

void Playlist::psl_load_playlist()
{
    string savedir = maybemakesavedir();
    if (savedir.empty())
        return;
    QString fn =  QFileDialog::getOpenFileName(
        0, tr("Open Playlist"), u8s2qs(savedir), tr("Saved playlists (*.pl)"));
    if (!fn.isNull()) {
        MetaDataList meta;
        meta.unSerialize(fn);
        psl_clear_playlist();
        psl_add_tracks(meta);
    }
}

void Playlist::psl_save_playlist()
{
    string savedir = maybemakesavedir();
    if (savedir.empty())
        return;
    QString fn =  QFileDialog::getSaveFileName(
        0, tr("Save Current Playlist"), u8s2qs(savedir), 
        tr("Saved playlists (*.pl)"));
    if (!fn.isNull()) {
        if (fn.indexOf(".pl") != fn.size() - 3)
            fn += ".pl";
        qDebug() << "Saving to " << fn;
        m_meta.serialize(fn);
    }
}

void Playlist::psl_selection_min_row(int row)
{
    //qDebug() << "Playlist::psl_selection_min_row: now: " << row;
    m_selection_min_row = row;
}
