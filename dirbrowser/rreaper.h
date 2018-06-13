/* Copyright (C) 2014 J.F.Dockes
 *	 This program is free software; you can redistribute it and/or modify
 *	 it under the terms of the GNU General Public License as published by
 *	 the Free Software Foundation; either version 2 of the License, or
 *	 (at your option) any later version.
 *
 *	 This program is distributed in the hope that it will be useful,
 *	 but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	 GNU General Public License for more details.
 *
 *	 You should have received a copy of the GNU General Public License
 *	 along with this program; if not, write to the
 *	 Free Software Foundation, Inc.,
 *	 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _RREAPER_H_INCLUDED_
#define _RREAPER_H_INCLUDED_

#include <string>

#include <QThread>

#include <libupnpp/config.h>
#include <libupnpp/control/cdirectory.hxx>

struct CtDesc;
class RecursiveReaper : public QThread {
    Q_OBJECT;

public: 
    RecursiveReaper(UPnPClient::CDSH server, std::string objid, 
                   QObject *parent = 0);
    virtual ~RecursiveReaper();

    virtual void run();
    void setCancel();
    static std::string ttpathPrintable(const std::string in);

signals:
    void sliceAvailable(UPnPClient::UPnPDirContent*);
    void done(int);

private:
    bool scanContainer(const CtDesc* ctdesc);
    class Internal;
    Internal *m;
};

#endif /* _RREAPER_H_INCLUDED_ */
