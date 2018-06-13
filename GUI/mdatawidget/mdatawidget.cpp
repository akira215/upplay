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

#include <QDebug>

#include "mdatawidget.h"
#include "HelperStructs/MetaData.h"
#include "HelperStructs/Helper.h"
#include "upadapt/upputils.h"

MDataWidget::MDataWidget(QWidget *parent, bool horiz)
    : MDataWidgetIF(parent)
{
    setupUi(this, horiz);
}

void MDataWidget::setData(const MetaData& md)
{
    // Sometimes ignore the date
    QString albtxt;
    if (md.year < 1000 || md.album.contains(QString::number(md.year))) {
        albtxt = md.album.trimmed();
    } else {
        albtxt = md.album.trimmed() + " (" + QString::number(md.year) + ")";
    }
    lab_album->setText(albtxt);
    QString html;
    metaDataToHtml(&md, html);
    lab_album->setToolTip(html);

    lab_artist->setText(md.artist);
    lab_artist->setToolTip(QString::fromUtf8("<i></i>") +
                           Helper::escapeHtml(md.artist));

    lab_title->setText(md.title);
    lab_title->setToolTip(QString::fromUtf8("<i></i>") +
                          Helper::escapeHtml(md.title));
}

