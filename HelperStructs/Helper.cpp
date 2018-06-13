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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <ctime>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#include <list>

#include <Qt>
#include <QDir>
#include <QUrl>
#include <QString>
#include <QTextDocument>
#include <QCryptographicHash>
#include <QDebug>
#include <QList>
#include <QFile>
#include <QFontMetrics>
#include <QFileInfo>
#include <QMap>
#include <QString>

#include "HelperStructs/Helper.h"
#include "HelperStructs/MetaData.h"
#include "HelperStructs/globals.h"
#include "HelperStructs/CSettingsStorage.h"

using namespace std;

QByteArray Helper::readFileToByteArray(const QString& fn)
{
    QFile qf(fn);
    QByteArray ret;
    if (qf.open(QIODevice::ReadOnly)) {
        ret = qf.read(1000 * 1000);
    }
    return ret;
}

QString Helper::cvtQString2FirstUpper(QString str)
{

    QString ret_str = "";
    QStringList lst = str.split(" ");

    foreach(QString word, lst) {
        QString first = word.left(1);
        word.remove(0, 1);
        word = first.toUpper() + word + " ";

        ret_str += word;
    }

    return ret_str.left(ret_str.size() - 1);
}

QString cvtNum2String(int num, int digits)
{
    QString str = QString::number(num);
    while (str.size() < digits) {
        str.prepend("0");
    }

    return str;
}

QString Helper::cvtMsecs2TitleLengthString(long int msec, bool colon,
        bool show_days)
{
    QString sign;
    if (msec < 0) {
        msec = -msec;
        sign = "-";
    }
        
    bool show_hrs = false;

    int sec = msec / 1000;
    int min = sec / 60;

    int secs = sec % 60;
    int hrs = min / 60;
    int days = hrs / 24;

    QString final_str;

    if (days > 0 && show_days) {
        final_str += QString::number(days) + "d ";
        hrs = hrs % 24;
        show_hrs = true;
    }

    if (!show_days) {
        hrs += (days * 24);
    }

    if (hrs > 0 || show_hrs) {
        final_str += QString::number(hrs) + "h ";
        min = min % 60;
    }

    if (colon) {
        final_str +=  cvtNum2String(min, 2) + ":" + cvtNum2String(secs, 2);
    } else {
        final_str +=  cvtNum2String(min, 2) + "m " + cvtNum2String(secs, 2);
    }

    return sign + final_str;

}

#ifdef Q_OS_WIN
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

QString GetExecPath()
{
    TCHAR dest[MAX_PATH];
    DWORD length = GetModuleFileName(NULL, dest, MAX_PATH);
#ifdef NTDDI_WIN8_future
    PathCchRemoveFileSpec(dest, MAX_PATH);
#else
    PathRemoveFileSpec(dest);
#endif
    if (sizeof(TCHAR) == 2)
        return QString::fromUtf16((const ushort*)dest);
    else
        return QString::fromUtf8((const char*)dest);
}
#endif

QString Helper::getSharePath()
{
    QString path;
#ifndef Q_OS_WIN
    if (QFile::exists(PREFIX "/share/upplay")) {
        path = PREFIX "/share/upplay/";
    } else {
        path = "";
    }
#else
    QDir execdir(GetExecPath());
    QString rpath("Share");
    execdir.mkpath(rpath);
    path = execdir.absoluteFilePath(rpath);
#endif
    return path;
}

static QString styleSubDir;
void Helper::setStyleSubDir(const QString& subd)
{
    //qDebug() << "Helper::setStyleSubDir: " << subd;
    styleSubDir = subd;
}

QString Helper::getIconDir()
{
    //return ":/icons/";
    return QDir(getSharePath()).filePath("icons");
}

QString Helper::getIconPath(const QString& icnm)
{
    if (!styleSubDir.isEmpty()) {
        QDir styledir(QDir(getIconDir()).filePath(styleSubDir));
        //qDebug() << "getIconPath(): testing " << styledir.filePath(icnm);
        if (QFile::exists(styledir.filePath(icnm))) {
            return styledir.filePath(icnm);
        }
    }
    return QDir(getIconDir()).filePath(icnm);
}

QString Helper::getHomeDataPath()
{
    QDir homedir(QDir::home());

#ifndef Q_OS_WIN
    QString rpath = QString::fromUtf8(".local/share/upplay");
#else
    QString rpath = QString::fromUtf8("AppData/Local/upplay/");
#endif

    homedir.mkpath(rpath);
    return homedir.absoluteFilePath(rpath);
}

QString Helper::createLink(QString name, QString target, bool underline)
{

    int dark = CSettingsStorage::getInstance()->getPlayerStyle();
    if (target.size() == 0) {
        target = name;
    }

    QString content;
    QString style = "";

    if (!underline) {
        style = " style: \"text-decoration=none;\" ";
    }

    if (dark) {
        content = LIGHT_BLUE(name);
    } else {
        content = DARK_BLUE(name);
    }

    return QString("<a href=\"") + target + "\"" + style + ">" +
           content + "</a>";
}

QString Helper::calc_filesize_str(qint64 filesize)
{
    qint64 kb = 1024;
    qint64 mb = kb * 1024;
    qint64 gb = mb * 1024;

    QString size;
    if (filesize > gb) {
        size = QString::number(filesize / gb) + "." +
               QString::number((filesize / mb) % gb).left(2)  + " GB";
    } else if (filesize > mb) {
        size = QString::number(filesize / mb) + "." +
               QString::number((filesize / kb) % mb).left(2)  + " MB";
    }  else {
        size = QString::number(filesize / kb) + " KB";
    }

    return size;
}

QString Helper::calc_cover_token(QString artist, QString album)
{
    QString ret =
        QCryptographicHash::hash(artist.trimmed().toLower().toUtf8() +
                                 album.trimmed().toLower().toUtf8(),
                                 QCryptographicHash::Md5).toHex();
    return ret;
}

QString Helper::get_album_w_disc(const MetaData& md)
{
    return md.album.trimmed();
}

QStringList Helper::get_soundfile_extensions()
{
    QStringList filters;
    filters << "*.mp3"
            << "*.ogg"
            << "*.oga"
            << "*.m4a"
            << "*.wav"
            << "*.flac"
            << "*.aac"
            << "*.wma";

    QString bla;
    foreach(QString filter, filters) {
        filters.push_back(filter.toUpper());
    }

    return filters;
}

bool Helper::read_file_into_str(QString filename, QString* content)
{
    QFile file(filename);
    content->clear();
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    while (!file.atEnd()) {
        QByteArray arr = file.readLine();
        QString str = QString::fromLocal8Bit(arr);

        content->append(str);
    }

    file.close();

    if (content->size() > 0) {
        return true;
    }

    return false;
}

// Escape things that would look like HTML markup
string Helper::escapeHtml(const string &in)
{
    string out;
    for (string::size_type pos = 0; pos < in.length(); pos++) {
	switch(in.at(pos)) {
	case '<': out += "&lt;"; break;
	case '>': out += "&gt;"; break;
	case '&': out += "&amp;"; break;
	case '"': out += "&quot;"; break;
	default: out += in.at(pos); break;
	}
    }
    return out;
}

QString Helper::escapeHtml(const QString& in)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    return in.toHtmlEscaped();
#else
    return Qt::escape(in);
#endif
}

template <class T> void stringsToString(const T &tokens, string &s) 
{
    for (typename T::const_iterator it = tokens.begin();
	 it != tokens.end(); it++) {
	bool hasblanks = false;
	if (it->find_first_of(" \t\n") != string::npos)
	    hasblanks = true;
	if (it != tokens.begin())
	    s.append(1, ' ');
	if (hasblanks)
	    s.append(1, '"');
	for (unsigned int i = 0; i < it->length(); i++) {
	    char car = it->at(i);
	    if (car == '"') {
		s.append(1, '\\');
		s.append(1, car);
	    } else {
		s.append(1, car);
	    }
	}
	if (hasblanks)
	    s.append(1, '"');
    }
}
template void stringsToString<list<string> >(const list<string> &, string &);
template void stringsToString<vector<string> >(const vector<string> &,string &);
template void stringsToString<set<string> >(const set<string> &, string &);
template <class T> string stringsToString(const T &tokens)
{
    string out;
    stringsToString<T>(tokens, out);
    return out;
}
template string stringsToString<list<string> >(const list<string> &);
template string stringsToString<vector<string> >(const vector<string> &);
template string stringsToString<set<string> >(const set<string> &);

string ivtos(const vector<int>& nids)
{
    string sids;
    for (unsigned int i = 0; i < nids.size(); i++) {
        char cbuf[30];
        sprintf(cbuf, "%d", nids[i]);
        sids += string(cbuf) + " ";
    }
    return sids;
}
