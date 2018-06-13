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
#include "directslider.h"

#include <cmath>

#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QStyleOptionSlider>

// A slider where a click sets the position to the click location
// instead of moving by a fixed amount as does the standard widget.
// http://stackoverflow.com/questions/11132597/qslider-mouse-direct-jump

static int adjustedVal(double halfHandleWidth, int pos, int sz, 
                       int min, int max)
{
    if (pos < halfHandleWidth)
        pos = halfHandleWidth;
    if (pos > sz - halfHandleWidth)
        pos = sz - halfHandleWidth;
    // Get new dimensions accounting for slider handle width
    double newWidth = (sz - halfHandleWidth) - halfHandleWidth;
    double normalizedPosition = (pos - halfHandleWidth)  / newWidth ;

    return min + ((max-min) * normalizedPosition);
}

void DirectSlider::setValueNoSigs(int val) {
    blockSignals(true);
    setValue(val);
    blockSignals(false);
}

void DirectSlider::mousePressEvent(QMouseEvent *event)
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, 
                                       QStyle::SC_SliderHandle, this);

    if (event->button() == Qt::LeftButton &&
        sr.contains(event->pos()) == false) {
        double halfHandleWidth = (0.5 * sr.width()) + 0.5; // Correct rounding
      
        int newVal;
        if (orientation() == Qt::Vertical) {
            newVal = adjustedVal(halfHandleWidth, 
                                 height() - event->y(), height(), 
                                 minimum(), maximum());
        } else {
            newVal = adjustedVal(halfHandleWidth, event->x(), width(), 
                                 minimum(), maximum());
        }
        setValue(newVal);
        event->accept();
    } else {
        QSlider::mousePressEvent(event);
    }
}
