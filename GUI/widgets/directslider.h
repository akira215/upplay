/*
 * Copyright (C) 2015 J.F. Dockes
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DIRECTSLIDER_H_
#define DIRECTSLIDER_H_

#include <QWidget>
#include <QSlider>

class QMouseEvent;

// A slider where a click sets the position to the click location
// instead of moving by a fixed amount as does the standard widget.
// http://stackoverflow.com/questions/11132597/qslider-mouse-direct-jump
class DirectSlider: public QSlider {
    Q_OBJECT

public:
    DirectSlider(QWidget* parent = 0)
        : QSlider(parent) {
    }

public slots:
    void setValueNoSigs(int val);

protected:
    virtual void mousePressEvent (QMouseEvent *event);
};

#endif /* DIRECTSLIDER_H_ */
