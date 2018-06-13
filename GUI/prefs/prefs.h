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
#ifndef UPPLAY_PREFS_H_INCLUDED
#define UPPLAY_PREFS_H_INCLUDED

#include <QWidget>

namespace confgui {
class ConfTabsW;
}

class UPPrefs : public QObject {
    Q_OBJECT

public:
    UPPrefs() {}
    UPPrefs(QWidget *parent) 
        : m_parent(parent) {
    }
    
    void setParent(QWidget *p) {
        m_parent = p;
    }

    enum Tab {PTAB_APP, PTAB_UPNP, PTAB_LASTFM, PTAB_DIRSORT};
                                                              
public slots:
    void onShowPrefs();
    void onShowPrefs(Tab);

signals:
    void sig_prefsChanged();

private:
    QWidget *m_parent{nullptr};
    confgui::ConfTabsW *m_w{nullptr};
};

#endif
