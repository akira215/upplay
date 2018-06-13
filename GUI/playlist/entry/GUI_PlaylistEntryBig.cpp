/* Copyright (C) 2011  Lucio Carreras
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

#include "GUI/playlist/entry/GUI_PlaylistEntryBig.h"
#include "HelperStructs/CSettingsStorage.h"
#include "HelperStructs/Helper.h"
#include "HelperStructs/MetaData.h"


GUI_PlaylistEntryBig::GUI_PlaylistEntryBig(QWidget* parent)
    : GUI_PlaylistEntry(parent)
{
    this->ui = new Ui::PlaylistEntryBig();
    this->ui->setupUi(this);
}

GUI_PlaylistEntryBig::~GUI_PlaylistEntryBig()
{
    delete this->ui;
}

int GUI_PlaylistEntryBig::fontheight()
{
    return ui->pl_lab_title->fontMetrics().height();
}

void GUI_PlaylistEntryBig::setContent(const MetaData& md, int idx)
{
    QString titlestr;

    if (CSettingsStorage::getInstance()->getPlaylistNumbers()) {
        titlestr = QString::number(idx) + ". " + md.title.trimmed();
    } else {
        titlestr = md.title.trimmed();
    }

    this->ui->pl_lab_title->setText(titlestr);
    this->ui->pl_lab_artist->setText(md.artist.trimmed());
    this->ui->pl_lab_time->setText(
        Helper::cvtMsecs2TitleLengthString(md.length_ms));
    if (md.album == "") {
        this->ui->pl_lab_album->setText("");
    } else {
        this->ui->pl_lab_album->setText(
            QString(" [ ") + Helper::get_album_w_disc(md) + " ]");
    }
}
