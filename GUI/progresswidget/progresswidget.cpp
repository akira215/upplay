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
#include "progresswidget.h"

#include "HelperStructs/Helper.h"

ProgressWidget::ProgressWidget(QWidget *parent)
    : ProgressWidgetIF(parent), m_totalsecs(0), m_step_pc(2), m_step_secs(-1)
{
    setupUi(this);
    songProgress->setMinimum(0);
    songProgress->setMaximum(100);
    connect(songProgress, SIGNAL(valueChanged(int)),
            this, SLOT(onProgressSliderChanged(int)));
}

void ProgressWidget::setTotalTime(int secs)
{
    QString length_str = Helper::cvtMsecs2TitleLengthString(secs * 1000, true);
    endTime->setText(length_str);
    m_totalsecs = secs;
}

void ProgressWidget::seek(int secs)
{
    if (secs < 0) {
        secs = 0;
    } else if (secs > m_totalsecs) {
        secs = m_totalsecs;
    }
    setUi(secs);
    emit seekRequested(secs);
}

void ProgressWidget::step(int steps)
{
    int secs = m_step_pc > 0 ? (m_step_pc * steps * m_totalsecs) / 100 :
        m_step_secs * steps;
    int current = (songProgress->value() * m_totalsecs) / 100;
    current += secs;
    seek(current);
}

void ProgressWidget::setStepValuePc(int percent)
{
    if (percent > 0 && percent <= 100) {
        m_step_secs = -1;
        m_step_pc = percent;
    }
}

void ProgressWidget::setStepValueSecs(int secs)
{
    if (secs > 0) {
        m_step_secs = secs;
        m_step_pc = -1;
    }
}

void ProgressWidget::showTimes(int secs)
{
    QString curPosString = Helper::cvtMsecs2TitleLengthString(secs*1000);
    curTime->setText(curPosString);
    if (secs > 0) {
        curPosString =
            Helper::cvtMsecs2TitleLengthString((secs - m_totalsecs)*1000);
        endTime->setText(curPosString);
    } else {
        curPosString = Helper::cvtMsecs2TitleLengthString(m_totalsecs*1000);
        endTime->setText(curPosString);
    }        
}

void ProgressWidget::setUi(int secs)
{
    int pc = m_totalsecs ? (secs * 100) / m_totalsecs : 0;
    songProgress->setValueNoSigs(pc);
    showTimes(secs);
}

int ProgressWidget::currentValuePc()
{
    return songProgress->value();
}

void ProgressWidget::onProgressSliderChanged(int pc)
{
    if (pc > 100) {
        pc = 100;
    } else if (pc < 0) {
        pc = 0;
    }
    int secs = (pc * m_totalsecs) / 100;
    showTimes(secs);
    emit seekRequested(secs);
}
