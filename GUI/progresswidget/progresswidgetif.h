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

#ifndef _PROGRESSWIDGETIF_H_INCLUDED_
#define _PROGRESSWIDGETIF_H_INCLUDED_

#include <QWidget>
#include <QString>

// Abstract interface for a volume control.
class ProgressWidgetIF : public QWidget {
    Q_OBJECT;

public:
    ProgressWidgetIF(QWidget *parent = 0)
        : QWidget(parent) {
    }
    virtual ~ProgressWidgetIF() {}

    virtual int currentValuePc() = 0;
public slots:
    // All times are in seconds

    virtual void setTotalTime(int value) = 0;

    // Set value and do whatever we do when it changes (emit
    // signals)
    virtual void seek(int value) = 0;
    // Increment/decrement value and do whatever we do when it
    // changes (emit signals). steps are in units of step_unit (x %)
    virtual void step(int steps) = 0;
    virtual void setStepValuePc(int percent) = 0;
    virtual void setStepValueSecs(int secs) = 0;
    // Set-up display, keep quiet
    virtual void setUi(int value) = 0;

    // Change skin if we can
    virtual void setSkinSuffix(const QString&) {
    }
    
signals:
    // Emitted when the requested position changes, either because the
    // user acted on the slider, or seek() was called. Value in
    // seconds
    void seekRequested(int secs);
};

#endif /* _PROGRESSWIDGETIF_H_INCLUDED_ */
