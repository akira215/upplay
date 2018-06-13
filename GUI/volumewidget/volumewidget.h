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

#ifndef _VOLUMEWIDGET_H_INCLUDED_
#define _VOLUMEWIDGET_H_INCLUDED_

#include <QString>

#include "volumewidgetif.h"

namespace Ui {
    class VolumeWidget;
}

class VolumeWidget: public VolumeWidgetIF {
    Q_OBJECT;
public:
    VolumeWidget(QWidget *parent = 0, bool horiz = false);

public slots:
    // Volume range is always enforced as 0-100

    // Set value and do whatever we do when volume changes (emit
    // signals)
    virtual void set(int value);
    // Increment/decrement value and do whatever we do when volume
    // changes (emit signals)
    virtual void step(int steps);

    virtual void toggleMute();
    
    // Set-up display, keep quiet
    virtual void setUi(int value);
    virtual void setMuteUi(bool);

    virtual void updateSkin();
                                              
private slots:
    // Connected to slider
    virtual void onVolumeSliderChanged(int value);
    virtual void onMuteClicked();

private:
    void init();
    void setupButton(int);
    
    Ui::VolumeWidget *ui;
    bool m_mute;
};

class VolumeHWidget : public VolumeWidget {
public:
    VolumeHWidget(QWidget *parent = 0)
        : VolumeWidget(parent, true) {
    }
};

    
#endif /* _VOLUMEWIDGET_H_INCLUDED_ */
