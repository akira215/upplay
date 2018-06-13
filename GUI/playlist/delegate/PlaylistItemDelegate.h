/* Copyright (C) 2012  Lucio Carreras
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
 */
#ifndef PLAYLISTITEMDELEGATEINTERFACE_H_
#define PLAYLISTITEMDELEGATEINTERFACE_H_

#include <QObject>
#include <QLabel>
#include <QListView>
#include <QItemDelegate>
#include "GUI/playlist/entry/GUI_PlaylistEntry.h"


class PlaylistItemDelegate : public QItemDelegate {

public:
    PlaylistItemDelegate(QListView *parent, bool compact);
    virtual ~PlaylistItemDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const;

    void setMaxWidth(int w);
    int rowHeight() const;

private:
    int     _max_width;
    bool    _compact;
    QListView*  _parent;
    GUI_PlaylistEntry* _pl_entry;
};

#endif /* PLAYLISTITEMDELEGATEINTERFACE_H_ */
