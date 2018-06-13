/* Copyright (C) 2015 J.F.Dockes for any changes relative to the VLC version
 *
 * Copyright (C) 2006-2011 the VideoLAN team
 *
 * Authors: Clément Stenac <zorglub@videolan.org>
 *          Jean-Baptiste Kempf <jb@videolan.org>
 *          Ludovic Fauvet <etix@videolan.org>
 *
 * This work is derived from VLC work under GPLv2+, which was itself derived
 * from Amarok's work under GPLv2+
 *  - Mark Kretschmann
 *  - Gábor Lehel
 *
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
#ifndef _SOUNDSLIDER_H_INCLUDED_
#define _SOUNDSLIDER_H_INCLUDED_

#include <QAbstractSlider>
#include <QPainter>
#include <QDebug>

/* Sound Slider inherited directly from QAbstractSlider */
class QPaintEvent;


/* VLC used // 125 % (+6dB) */
#define SOUNDMAX  100

#ifndef Q_DECL_OVERRIDE
#define Q_DECL_OVERRIDE
#endif
#ifndef DPI_REF_VALUE
#define DPI_REF_VALUE 96.0
#endif
#ifndef AOUT_VOLUME_DEFAULT
#define AOUT_VOLUME_DEFAULT 256
#endif

class SoundSlider : public QAbstractSlider
{
    Q_OBJECT
public:
    SoundSlider(QWidget *_parent, float _i_step, char *psz_colors, int max = SOUNDMAX );
    void setMuted( bool ); /* Set Mute status */

protected:
    const static int paddingL = 3;
    const static int paddingR = 2;

    void paintEvent( QPaintEvent *) Q_DECL_OVERRIDE;
    void wheelEvent( QWheelEvent *event ) Q_DECL_OVERRIDE;
    void mousePressEvent( QMouseEvent * ) Q_DECL_OVERRIDE;
    void mouseMoveEvent( QMouseEvent * ) Q_DECL_OVERRIDE;
    void mouseReleaseEvent( QMouseEvent * ) Q_DECL_OVERRIDE;

    void processReleasedButton();

private:
    bool isSliding; /* Whether we are currently sliding by user action */
    bool b_mouseOutside; /* Whether the mouse is outside or inside the Widget */
    int i_oldvalue; /* Store the old Value before changing */
    float f_step; /* How much do we increase each time we wheel */
    bool b_isMuted;
    int wlength;
    int wheight;

    QPixmap pixGradient; /* Gradient pix storage */
    QPixmap pixGradient2; /* Muted Gradient pix storage */
    QPixmap pixOutside; /* OutLine pix storage */
    QPainter painter;
    QColor background;
    QColor foreground;
    QFont textfont;
    QRect textrect;

    void changeValue( int x ); /* Function to modify the value from pixel x() */
};

#endif /* _SOUNDSLIDER_H_INCLUDED_ */
