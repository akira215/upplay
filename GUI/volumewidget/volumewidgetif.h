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
#ifndef VOLUMEWIDGETIF_INCLUDED
#define VOLUMEWIDGETIF_INCLUDED
#include <QWidget>
#include <QString>

// Abstract interface for a volume control.
class VolumeWidgetIF : public QWidget {
    Q_OBJECT;

public:
    VolumeWidgetIF(QWidget *parent = 0)
        : QWidget(parent) {
    }
    virtual ~VolumeWidgetIF() {}

public slots:
    // Volume range is always enforced as 0-100

    // Set volume value and do whatever we do when volume changes
    // (emit signals)
    virtual void set(int value) = 0;
    // Increment/decrement value and do whatever we do when volume
    // changes (emit signals)
    virtual void step(int steps) = 0;

    // Do what we do when mute is clicked
    virtual void toggleMute() = 0;
    
    // Set-up display, keep quiet
    virtual void setUi(int value) = 0;
    virtual void setMuteUi(bool) = 0;

    virtual void volumeHigher() {
        step(5);
    }
    virtual void volumeLower() {
        step(-5);
    }
    virtual void updateSkin() {
    }
    
signals:
    void volumeChanged(int);
    void muteChanged(bool);
};

#endif
