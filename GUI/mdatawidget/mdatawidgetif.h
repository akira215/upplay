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

#ifndef _MDATAWIDGETIF_H_INCLUDED_
#define _MDATAWIDGETIF_H_INCLUDED_

#include <QWidget>

class MetaData;

class MDataWidgetIF : public QWidget {
    Q_OBJECT;

public:
    MDataWidgetIF(QWidget *parent = 0, bool horiz = false)
        : QWidget(parent) {
        Q_UNUSED(horiz);
    }
    virtual ~MDataWidgetIF() {}

public slots:
    virtual void setData(const MetaData& md) = 0;
};


#endif /* _MDATAWIDGETIF_H_INCLUDED_ */
