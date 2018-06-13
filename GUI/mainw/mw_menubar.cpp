/* Copyright (C) 2012  Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainw.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

#include <libupnpp/upnpplib.hxx>

#include "HelperStructs/CSettingsStorage.h"

/** VIEW **/

void GUI_Player::showLibrary(bool b, bool resize)
{
    m_settings->setNoShowLibrary(!b);
    int old_width = this->width();
    int lib_width = this->ui->library_widget->width();
    int new_width = old_width;
    this->ui->library_widget->setVisible(b);

    if (!b) {
        // invisble
        QSizePolicy p = this->ui->library_widget->sizePolicy();
        m_library_stretch_factor = p.horizontalStretch();

        p.setHorizontalStretch(0);
        this->ui->library_widget->setSizePolicy(p);
        m_library_width = lib_width;
        new_width = old_width - lib_width;
        //this->setMinimumSize(300, 400);
    } else {
        // visible
        QSizePolicy p = this->ui->library_widget->sizePolicy();
        p.setHorizontalStretch(m_library_stretch_factor);
        this->ui->library_widget->setSizePolicy(p);
        new_width = old_width + m_library_width;
        //this->setMinimumSize(650, 400);
    }

    if (resize) {
        QRect rect = this->geometry();
        rect.setWidth(new_width);
        rect.setHeight(this->height());
        this->setGeometry(rect);
    }
}

void GUI_Player::show_fullscreen_toggled(bool b)
{
    // may happend because of F11 too
    ui->action_Fullscreen->setChecked(b);
    if (b) {
        this->showFullScreen();
    } else {
        this->showNormal();
    }

    m_settings->setPlayerFullscreen(b);
}

// prvt slot
void GUI_Player::small_playlist_items_toggled(bool b)
{
    m_settings->setShowSmallPlaylist(b);
    emit show_small_playlist_items(b);
}

/** PREFERENCES END **/

void GUI_Player::help(bool)
{
    QString link = Helper::createLink("http://www.lesbonscomptes.com/upplay");

    QMessageBox::information(this, tr("Help"), tr("See links on") + "<br />" 
                             + link);
}

// private slot
void GUI_Player::about(bool b)
{
    Q_UNUSED(b);

    QString version = m_settings->getVersion();
    QString link = Helper::createLink("http://www.lesbonscomptes.com/upplay");

    QMessageBox infobox(this);
    infobox.setParent(this);
    QPixmap p =  QPixmap(Helper::getIconPath("logo.png")).
        scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    infobox.setIconPixmap(p);
    infobox.setWindowFlags(Qt::Dialog);
    infobox.setModal(true);

    infobox.setWindowTitle(tr("About Upplay"));
    infobox.setText("<b><font size=\"+2\">Upplay Player " + 
                    version + "</font></b>");
    infobox.setInformativeText(
        QString("") +
        UPnPP::LibUPnP::versionString().c_str() + "<br/>" +
        tr("Based on Sayonara, written by Lucio Carreras") + "<br/>" +
        tr("License") + ": GPL<br/>" +
        "Copyright " + UPPLAY_COPYRDATES + "<br/><br/>" + link + "<br/><br/>"
        );
    infobox.setStandardButtons(QMessageBox::Ok);
    infobox.button(QMessageBox::Ok)->setFocusPolicy(Qt::NoFocus);
    infobox.exec();
}
