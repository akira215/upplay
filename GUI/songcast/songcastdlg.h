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
#ifndef _SCTOOL_H_INCLUDED_
#define _SCTOOL_H_INCLUDED_

#include "ui_songcastdlg.h"

#include <QCheckBox>

class UncheckCheckBox;

class SongcastDLG : public QDialog, public Ui::SongcastDLG {
    Q_OBJECT;

public:
    SongcastDLG(QWidget * parent = 0) 
	: QDialog(parent)
    {
	setupUi(this);
        init();
    }

    void createControls(unsigned int numsenders, unsigned int numreceivers);
    
    // Return index of checked sender, or -1 (at most one can be checked)
    int selectedSenderIdx();

    bool receiverOnRequested(unsigned int i);
    bool receiverOffRequested(unsigned int i);
    QCheckBox* senderButton(int row);
    QLabel* senderLabel(int row);
    UncheckCheckBox* receiverOffButton(int row);
    UncheckCheckBox* receiverOnButton(int row);
    QLabel* receiverLabel(int row);
    
    // These are needed, because gridLayout::numRows is unreliable...
    static const int SNDWPERLINE;
    static const int RCVWPERLINE;

    int numSenderRows();
    int numReceiverRows();
                         
public slots:
    void apply();
signals:
    void sig_apply(SongcastDLG *);
    
private:
    void init();
};
    

// Used for sets of button where only one can be on (but not
// radiobuttons: all can be off)
class UncheckCheckBox : public QCheckBox {
    Q_OBJECT;
public:
    UncheckCheckBox(const QString& text, QWidget *parent = 0)
        : QCheckBox(text, parent) {}
public slots:
    void setUnChecked(bool on) {
        if (on)
            setChecked(false);
    }
};

#endif /* _SCTOOL_H_INCLUDED_ */
