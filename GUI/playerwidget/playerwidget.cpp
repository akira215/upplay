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
#include "playerhwidget.h"
#include "playervwidget.h"

PlayerVWidget::PlayerVWidget(QWidget *parent)
    : QWidget(parent)
{
        setupUi(this);

        // Reproduce the old setup by moving the stop button around
        QLayoutItem *item = playctl_w->takeStopWidget();
        // Curiously this does not work
        //bottomHLayout->addItem(item);
        // But this does
        bottomHLayout->addWidget(item->widget());
        // I guess that this should be done, but I also suspect that
        // I'm seeing related memory issues
        //      item->invalidate(); delete item;
}

PlayerHWidget::PlayerHWidget(QWidget *parent)
    : QWidget(parent)
{
        setupUi(this);
}
