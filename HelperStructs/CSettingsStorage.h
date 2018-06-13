/* CSettingsStorage.h */

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
#ifndef CSettingsStorage_H
#define CSettingsStorage_H

#include <QString>
#include <QSettings>
#include <QApplication>
#include <QPair>
#include <QSize>
#include <QPoint>

#include <vector>

#include <HelperStructs/PlaylistMode.h>

using namespace std;

#define GENDEC_VARIABLE(NM, TP, CTP)       \
    void set##NM(TP);                      \
    TP get##NM ();


class CSettingsStorage : public QSettings
{
    Q_OBJECT

public:
    static CSettingsStorage* getInstance();

    virtual ~CSettingsStorage ();

    // First time run check
    bool isRunFirstTime ();

private:
    CSettingsStorage(QObject *parent = 0) 
        : QSettings (QCoreApplication::organizationName(),
                     QCoreApplication::applicationName(), parent)
        {
        }

    CSettingsStorage(const CSettingsStorage&);
    CSettingsStorage& operator=(const CSettingsStorage&);

public:
    GENDEC_VARIABLE(Version, QString, String);
    GENDEC_VARIABLE(Volume, int, Int);
    GENDEC_VARIABLE(PlayerSize, QSize, Size);
    GENDEC_VARIABLE(PlayerPos, QPoint, Point);
    GENDEC_VARIABLE(PlayerFullscreen, bool, Bool);
    GENDEC_VARIABLE(PlayerStyle, int, Int);
    GENDEC_VARIABLE(ShowNotifications, bool, Bool);
    GENDEC_VARIABLE(Notification, QString, String);
    GENDEC_VARIABLE(NotificationTimeout, int, Int);
    GENDEC_VARIABLE(NotificationScale, int, Int);
    GENDEC_VARIABLE(NoShowLibrary, bool, Bool);
    GENDEC_VARIABLE(MinimizeToTray, bool, Bool);
    GENDEC_VARIABLE(ShowSmallPlaylist, bool, Bool);
    GENDEC_VARIABLE(PlaylistNumbers, bool, Bool);
    GENDEC_VARIABLE(NotifyNewVersion, bool,  Bool);
    GENDEC_VARIABLE(PlayerUID, QString, String);
    GENDEC_VARIABLE(SplitterState, QByteArray, ByteArray);
    enum SortKinds {SK_NOSORT=0, SK_MINIMFNORDER=1, SK_CUSTOM=2};
    GENDEC_VARIABLE(SortKind, int, Int);
    GENDEC_VARIABLE(SortCrits, QStringList, StringList);
    // Special cases
    void setPlaylistMode(const Playlist_Mode&);
    Playlist_Mode getPlaylistMode();

};

#undef GENDEC_VARIABLE

#endif // CSettingsStorage_H
