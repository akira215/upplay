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

#include <QtNetwork/QNetworkReply>
#include <QIcon>
#include <QImageReader>
#include <QImage>
#include <QTemporaryFile>
#include <QDir>

void GUI_Player::sl_cover_fetch_done(QNetworkReply* reply)
{
    //qDebug() << "GUI_Player::sl_cover_fetch_done";

    if (reply != (QNetworkReply*)m_currentCoverReply) {
        // Ignore answers to older requests
        reply->deleteLater();
        return;
    }
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        sl_no_cover_available();
        return;
    }

    QString smime = 
        reply->header(QNetworkRequest::ContentTypeHeader).toString();
    int scolon;
    if ((scolon = smime.indexOf(";")) > 0) {
        smime = smime.left(scolon);
    }
    const char *imtype;
    const char *suffix;
    if (!smime.compare("image/png", Qt::CaseInsensitive)) {
        imtype = "PNG";
        suffix = ".png";
    } else if (!smime.compare("image/jpeg", Qt::CaseInsensitive)) {
        imtype = "JPG";
        suffix = ".jpg";
    } else if (!smime.compare("image/gif", Qt::CaseInsensitive)) {
        imtype = "GIF";
        suffix = ".gif";
    } else {
        qDebug() << "GUI_Player::sl_cover_fetch_done: unsupported mime type: "<<
            smime;
        reply->deleteLater();
        sl_no_cover_available();
        return;
    }
    QByteArray imdata = reply->readAll();

    QString tpath = 
        QDir(QDir::tempPath()).absoluteFilePath(QString::fromUtf8("XXXXXX")
                                                + suffix);

    if (m_covertempfile) {
        delete(m_covertempfile);
        m_covertempfile = 0;
    }
    m_covertempfile = new QTemporaryFile(tpath, this);
    if (m_covertempfile) {
        m_covertempfile->open();
        m_covertempfile->write(imdata);
        m_covertempfile->close();
    }

    QImage image;
    if (!image.loadFromData(imdata, imtype)) {
        qDebug() << "GUI_Player::sl_cover_fetch_done: image read failed ";
        reply->deleteLater();
        sl_no_cover_available();
        return;
    }

    QPixmap pixmap;
    pixmap.convertFromImage(image);
    ui->player_w->albumCover->setIcon(QIcon(pixmap));
    ui->player_w->albumCover->setIconSize(ui->player_w->albumCover->size());

    QString htmlfrag("<img src=\"");
    htmlfrag += m_covertempfile->fileName();
    htmlfrag += "\">";
    ui->player_w->albumCover->setToolTip(htmlfrag);

    reply->deleteLater();
}


void GUI_Player::sl_no_cover_available()
{
    QString coverpath = Helper::getIconPath("logo.png");
    ui->player_w->albumCover->setIcon(QIcon(coverpath));
    ui->player_w->albumCover->setToolTip("");
}
