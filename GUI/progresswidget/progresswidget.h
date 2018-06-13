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

#ifndef _PROGRESSWIDGET_H_INCLUDED_
#define _PROGRESSWIDGET_H_INCLUDED_


#include <QString>

#include "progresswidgetif.h"
#include "ui_progresswidget.h"

class ProgressWidget: public ProgressWidgetIF, public Ui::ProgressWidget {
    Q_OBJECT;
public:
    ProgressWidget(QWidget *parent = 0);

    virtual int currentValuePc();
                                
public slots:
    // All times are in seconds

    virtual void setTotalTime(int secs);

    // Set value and do whatever we do when it changes (emit
    // signals)
    virtual void seek(int secs);
    // Increment/decrement value and do whatever we do when it
    // changes (emit signals). steps are in units of step_unit (x %)
    virtual void step(int steps);
    virtual void setStepValuePc(int percent);
    virtual void setStepValueSecs(int secs);
    // Set-up display, keep quiet
    virtual void setUi(int secs);

    virtual void setSkinSuffix(const QString& s) {
        m_skinSuffix = s;
    }

private slots:
    // Connected to slider
    virtual void onProgressSliderChanged(int pc);

private:
    void showTimes(int secs);

    int m_totalsecs;
    // Values for step in percent or secs. Only one can be valid (> 0)
    // Initial default 2%
    int m_step_pc;
    int m_step_secs;
    
    QString m_skinSuffix;
};


#endif /* _PROGRESSWIDGET_H_INCLUDED_ */
