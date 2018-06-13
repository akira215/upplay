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
#ifndef _PLAYCTLWIDGET_H_INCLUDED_
#define _PLAYCTLWIDGET_H_INCLUDED_

#include "playctlwidgetif.h"
#include "ui_playctlwidget.h"

class MetaData;

class PlayCtlWidget : public PlayCtlWidgetIF, public Ui::PlayCtlWidget {
    Q_OBJECT;

public:
    PlayCtlWidget(QWidget *parent = 0);
    virtual ~PlayCtlWidget() {}

    bool playing();
                  
public slots:
    // These are used to set the ui state in sync with externally
    // caused player state changes
    virtual void onStopped();
    virtual void onPaused();
    virtual void onPlaying();

    // These are connected to the buttons and trigger signals, but can also be
    // used, e.g. for keyboard shortcuts
    virtual void onPlayClicked();
    virtual void onStopClicked();
    virtual void onBackwardClicked();
    virtual void onForwardClicked();

    // Give access to the stop button in case they want to hide it.
    virtual QLayoutItem *takeStopWidget();
    
private:
    bool m_playing;
};

#endif /* _PLAYCTLWIDGET_H_INCLUDED_ */
