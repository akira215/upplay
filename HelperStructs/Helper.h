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
#ifndef _HELPER_H
#define _HELPER_H
#include <time.h>

#include <string>
#include <vector>

#include <QString>
#include <QList>
#include <QWidget>
#include <QByteArray>

#include "HelperStructs/MetaData.h"


namespace Helper {
QByteArray readFileToByteArray(const QString& fn);

QString cvtMsecs2TitleLengthString(long int msec, bool colon = true,
                                   bool show_days = true);
QString cvtQString2FirstUpper(QString str);
QString calc_filesize_str(qint64 filesize);

void setStyleSubDir(const QString& subd);
QString getIconDir();
QString getIconPath(const QString& nm);
QString getSharePath();
QString getHomeDataPath();
QString createLink(QString name, QString target = "", bool underline = true);

QString get_cover_path(QString artist, QString album,
                       QString extension = "jpg");
QString get_cover_path(int album_id);
QString calc_cover_token(QString artist, QString album);

QStringList get_soundfile_extensions();

QString get_album_w_disc(const MetaData& md);

template <typename T> QList<T> randomize_list(const QList<T>& list)
{
    srand(time(NULL));

    QList<T> list_copy = list;
    for (int i = 0; i < list.size(); i++) {

        list_copy.swap(i, rand() % list.size());
    }

    return list_copy;
}

bool read_file_into_str(QString filename, QString* content);
// Escape things that would look like HTML markup
std::string escapeHtml(const string &in);
QString escapeHtml(const QString& in);

};


template <class T> void stringsToString(const T &tokens, std::string &s);
template <class T> std::string stringsToString(const T &tokens);

extern std::string ivtos(const std::vector<int>& nids);

#endif
