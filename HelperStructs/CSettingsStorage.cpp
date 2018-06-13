/* CSettingsStorage.cpp */

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

#include "HelperStructs/Helper.h"
#include "HelperStructs/PlaylistMode.h"
#include "HelperStructs/CSettingsStorage.h"

#include <QFile>
#include <QDir>
#include <QDebug>

CSettingsStorage* CSettingsStorage::getInstance() {
    static CSettingsStorage* inst;
    if (inst == 0) {
        inst = new CSettingsStorage();
    }
    return inst;
}

CSettingsStorage::~CSettingsStorage () 
{
    //is called
}

#define GENCODE_VARIABLE(NM, TP, CTP)      \
    static const QString NM##Key(#NM);     \
    void CSettingsStorage::set##NM(TP val) \
    {                                      \
        setValue(NM##Key, val);            \
    }                                      \
    TP CSettingsStorage::get##NM ()        \
    {                                      \
        return value(NM##Key).to##CTP ();  \
    }


GENCODE_VARIABLE(Version, QString, String)
GENCODE_VARIABLE(Volume, int, Int)
GENCODE_VARIABLE(PlayerSize, QSize, Size)
GENCODE_VARIABLE(PlayerPos, QPoint, Point)
GENCODE_VARIABLE(PlayerFullscreen, bool, Bool)
GENCODE_VARIABLE(PlayerStyle, int, Int)
GENCODE_VARIABLE(ShowNotifications, bool, Bool)
GENCODE_VARIABLE(Notification, QString, String)
GENCODE_VARIABLE(NotificationTimeout, int, Int)
GENCODE_VARIABLE(NotificationScale, int, Int)
GENCODE_VARIABLE(NoShowLibrary, bool, Bool)
GENCODE_VARIABLE(MinimizeToTray, bool, Bool)
GENCODE_VARIABLE(ShowSmallPlaylist, bool, Bool)
GENCODE_VARIABLE(PlaylistNumbers, bool, Bool)
GENCODE_VARIABLE(NotifyNewVersion, bool,  Bool)
GENCODE_VARIABLE(PlayerUID, QString, String)
GENCODE_VARIABLE(SplitterState, QByteArray, ByteArray)
GENCODE_VARIABLE(SortKind, int, Int);
GENCODE_VARIABLE(SortCrits, QStringList, StringList);

bool CSettingsStorage::isRunFirstTime () 
{
    return value(VersionKey).isNull();
}

// Special cases

static const QString PlaylistModeKey("PlaylistMode");
void CSettingsStorage::setPlaylistMode(const Playlist_Mode& mode)
{
    //qDebug() << "setPlaylistMode: " << mode.toInt();
    setValue(PlaylistModeKey, mode.toInt());
}
Playlist_Mode CSettingsStorage::getPlaylistMode()
{
    //qDebug() << "getPlaylistMode: " << value(PlaylistModeKey).toInt();
    return Playlist_Mode(value(PlaylistModeKey).toInt());
}

// End special cases 
