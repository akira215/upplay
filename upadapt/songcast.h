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

#ifndef _SONGCAST_H_INCLUDED_
#define _SONGCAST_H_INCLUDED_

#include <string>

#include <QObject>
#include <QString>

class SongcastDLG;

class SongcastTool : public QObject {
    Q_OBJECT;
public:
    SongcastTool(SongcastDLG *dlg, QObject *parent = 0);

    SongcastDLG *dlg();

    void initControls();
                       
public slots:
    void onSongcastApply();

private slots:
    void enableOnButtons();
    void syncReceivers();

private:
    std::string senderNameFromUri(const std::string& uri);
    void songcastPopulate();
    QString receiverText(int, bool);
    
    class Internal;
    Internal *m;
};

#endif /* _SONGCAST_H_INCLUDED_ */
