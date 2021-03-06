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

#include "playlist/Playlist.h"
#include "cdbrowser.h"

// The DirBrowser object has multiple tabs, each displaying the server
// list of a directory listing or search result, and a hideable search
// panel at the bottom
class DirBrowser : public QWidget {
    Q_OBJECT
public:
    DirBrowser(QWidget *parent, Playlist *pl);
    bool insertActive();
    CDBrowser *currentBrowser();
    void doSearch(const QString&, bool);

public slots:
    void setPlaylist(Playlist *pl);
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

private:
    void setupTabConnections(int i);
    void setupTabConnections(CDBrowser* w);

    Ui::DirBrowser *ui;
    Playlist *m_pl;
    bool m_insertactive;
};


#endif /* _DIRBROWSER_H_INCLUDED_ */
