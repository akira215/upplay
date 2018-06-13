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

#include "songcastdlg.h"

#include <QPushButton>
#include <QDialogButtonBox>

const int SongcastDLG::SNDWPERLINE = 2;
const int SongcastDLG::RCVWPERLINE = 3;


void SongcastDLG::init()
{
    QPushButton* applyButton = buttonBox->button(QDialogButtonBox::Apply);
    connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));

    delete initsbutton;
    delete initslabel;
    delete initrbutton1;
    delete initrbutton2;
    delete initrlabel;
}

static void btnSzPolicy(QWidget *w)
{
    QSizePolicy szPol(QSizePolicy::Fixed, QSizePolicy::Fixed);
    szPol.setHorizontalStretch(0);
    szPol.setVerticalStretch(0);
    szPol.setHeightForWidth(w->sizePolicy().hasHeightForWidth());
    w->setSizePolicy(szPol);
}

void SongcastDLG::createControls(unsigned int numsenders,
                                 unsigned int numreceivers)
{
    QLayoutItem *child;
    while ((child = sndGridLayout->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }
    while ((child = rcvGridLayout->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }

    if (numsenders == 0) {
        QLabel *lbl = new QLabel(tr("No Songcast Senders found"), this);
        sndGridLayout->addWidget(lbl, 0, 0, 1, 1);
    } else {
        QLabel *lbl;
        QRadioButton *btn;
            
        for (unsigned int i = 0; i < numsenders; i++) {
            btn = new QRadioButton(sndGroupBox);
            btnSzPolicy(btn);
            lbl = new QLabel(sndGroupBox);
            sndGridLayout->addWidget(btn, i, 0, 1, 1);
            sndGridLayout->addWidget(lbl, i, 1, 1, 1);
        }
    }

    if (numreceivers == 0) {
        QLabel *lbl = new QLabel(tr("No Songcast Receivers found"), this);
        rcvGridLayout->addWidget(lbl, 0, 0, 1, 1);
    } else {
        QLabel *lbl;
        UncheckCheckBox *on, *off;
            
        for (unsigned int i = 0; i < numreceivers; i++) {
            on = new UncheckCheckBox(tr("Link and start"), rcvGroupBox);
            btnSzPolicy(on);
            off = new UncheckCheckBox(tr("Stop"), rcvGroupBox);
            btnSzPolicy(off);
            lbl = new QLabel(rcvGroupBox);

            rcvGridLayout->addWidget(off, i, 0, 1, 1);
            rcvGridLayout->addWidget(on,  i, 1, 1, 1);
            rcvGridLayout->addWidget(lbl, i, 2, 1, 1);
            connect(on, SIGNAL(toggled(bool)), off, SLOT(setUnChecked(bool)));
            connect(off, SIGNAL(toggled(bool)), on, SLOT(setUnChecked(bool)));
        }
    }
}

int SongcastDLG::numReceiverRows()
{
    return rcvGridLayout->count() / SongcastDLG::RCVWPERLINE;
}
int SongcastDLG::numSenderRows()
{
    return sndGridLayout->count() / SongcastDLG::SNDWPERLINE;
}

void SongcastDLG::apply()
{
    emit sig_apply(this);
}

int SongcastDLG::selectedSenderIdx()
{
    int senderidx = -1;
    // Note: quite unbelievably, gridLayout::rowCount() is not the number
    // of actual rows, but the size of the internal allocation. So can't use it
    // after deleting rows...
    for (int i = 0; i < numSenderRows(); i++) {
        QRadioButton *btn = (QRadioButton*)
            sndGridLayout->itemAtPosition(i, 0)->widget();
        if (btn->isChecked()) {
            senderidx = i;
            break;
        }
    }
    return senderidx;
}

bool SongcastDLG::receiverOnRequested(unsigned int i)
{
    return receiverOnButton(i)->isChecked();
}
bool SongcastDLG::receiverOffRequested(unsigned int i)
{
    return receiverOffButton(i)->isChecked();
}

UncheckCheckBox* SongcastDLG::receiverOffButton(int row)
{
    return (UncheckCheckBox*)rcvGridLayout->itemAtPosition(row, 0)->widget();
}
UncheckCheckBox* SongcastDLG::receiverOnButton(int row)
{
    return (UncheckCheckBox*)rcvGridLayout->itemAtPosition(row, 1)->widget();
}
QLabel* SongcastDLG::receiverLabel(int row)
{
    return (QLabel *)rcvGridLayout->itemAtPosition(row, 2)->widget();
}
QCheckBox* SongcastDLG::senderButton(int row)
{
    return (QCheckBox *)sndGridLayout->itemAtPosition(row, 0)->widget();
}
QLabel* SongcastDLG::senderLabel(int row)
{
    return (QLabel *)sndGridLayout->itemAtPosition(row, 1)->widget();
}
