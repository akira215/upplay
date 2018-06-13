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
#ifndef _DIRBROWSER_H_INCLUDED_
#define _DIRBROWSER_H_INCLUDED_
#include "ui_dirbrowser.h"

#include <memory>

#include "HelperStructs/MetaData.h"
#include "playlist/playlist.h"
#include "cdbrowser.h"

// The DirBrowser object has multiple tabs, each displaying the server
// list of a directory listing or search result, and a hideable search
// panel at the bottom
class DirBrowser : public QWidget {
    Q_OBJECT;

public:
    DirBrowser(QWidget *parent, std::shared_ptr<Playlist> pl);
    bool insertActive();
    CDBrowser *currentBrowser();
    void doSearch(const QString&, bool);
    bool randPlayActive() {return m_randplayer != 0;}
    bool have_playlist() {return bool(m_pl);}

public slots:
    void setPlaylist(std::shared_ptr<Playlist> pl);
    void setStyleSheet(bool);
    void addTab();
    void closeTab(int);
    void closeCurrentTab();
    void onCurrentTabChanged(int);
    void changeTabTitle(QWidget *, const QString&);
    void onSearchcapsChanged();
    void onSortprefs();

    void openSearchPanel();
    void closeSearchPanel();
    void showSearchPanel(bool);
    void toggleSearchKind();
    void onSearchKindChanged(int);
    void onSearchTextChanged(const QString&);
    void searchNext();
    void searchPrev();
    void returnPressedInSearch();
    void serverSearch();

    void onInsertDone() {m_insertactive = false;}
    void setInsertActive(bool onoff);
    void onBrowseInNewTab(QString UDN, std::vector<CDBrowser::CtPathElt>);
    void onRandTracksToPlaylist(const MetaDataList& mdl);
    void onEnterRandPlay(RandPlayer::PlayMode mode, const
                         std::vector<UPnPClient::UPnPDirObject>&);
    void onRandStop();
    void onRandDone();

signals:
    void sig_next_group_html(QString);
    void sig_sort_order();
    
private:
    void setupTabConnections(int i);
    void setupTabConnections(CDBrowser* w);

    Ui::DirBrowser *ui;
    std::shared_ptr<Playlist> m_pl;
    bool m_insertactive;
    RandPlayer *m_randplayer;
};


#endif /* _DIRBROWSER_H_INCLUDED_ */
