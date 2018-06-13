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

#ifndef _PLAYERVWIDGET_H_INCLUDED_
#define _PLAYERVWIDGET_H_INCLUDED_
#include <QWidget>

#include "ui_playervwidget.h"

class VolumeWidgetIF;
class ProgressWidgetIF;
class MDataWidgetIF;
class PlayCtlWidgetIF;

class PlayerVWidget : public QWidget, public Ui::PlayerVWidget {
    Q_OBJECT;
public:
    PlayerVWidget(QWidget *parent = 0);

    VolumeWidgetIF* volume() {
        return volumectl_w;
    }
    PlayCtlWidgetIF* playctl() {
        return playctl_w;
    }
    ProgressWidget* progress() {
        return progress_w;
    }
    MDataWidget* mdata() {
        return mdata_w;
    }
    virtual QLayoutItem *takeCoverWidget() {
        return horizontalLayout->takeAt(0);
    }
};

#endif /* _PLAYERWIDGET_H_INCLUDED_ */
