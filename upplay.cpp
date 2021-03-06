/* Copyright (C) 2014 J.F.Dockes
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

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>
using namespace std;

#include <QApplication>
#include <QObject>
#include <QTimer>

#include <libupnpp/upnpplib.hxx>
#include <libupnpp/log.hxx>

#include "application.h"

using namespace UPnPClient;

static const char *thisprog;

static int    op_flags;
#define OPT_h     0x4 
#define OPT_c     0x20

static const char usage [] =
    "\n"
    ;
static void
Usage(void)
{
    FILE *fp = (op_flags & OPT_h) ? stdout : stderr;
    fprintf(fp, "%s: Usage: %s", thisprog, usage);
    exit((op_flags & OPT_h)==0);
}


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Upmpd.org");
    QCoreApplication::setApplicationName("upplay");

    string a_config;

    QStringList params;
    for(int i = 1; i < argc; i++){
        QString param(argv[i]);
        params.push_back(param);
    }

    thisprog = argv[0];
    argc--; argv++;

    while (argc > 0 && **argv == '-') {
        (*argv)++;
        if (!(**argv))
            Usage();
        while (**argv)
            switch (*(*argv)++) {
            case 'c':   op_flags |= OPT_c; if (argc < 2)  Usage();
                a_config = *(++argv);
                argc--; goto b1;
            default: Usage();
            }
    b1: argc--; argv++;
    }

    if (argc > 0)
        Usage();

    if (Logger::getTheLog("stderr") == 0) {
        cerr << "Can't initialize log" << endl;
        return 1;
    }
    const char *cp;
    if ((cp = getenv("UPPLAY_LOGLEVEL"))) {
        Logger::getTheLog("")->setLogLevel(Logger::LogLevel(atoi(cp)));
    }

    LibUPnP *mylib = LibUPnP::getLibUPnP();
    if (!mylib) {
        cerr << "Can't get LibUPnP" << endl;
        return 1;
    }
    if (!mylib->ok()) {
        cerr << "Lib init failed: " <<
            mylib->errAsString("main", mylib->getInitError()) << endl;
        return 1;
    }
    if ((cp = getenv("UPPLAY_UPNPLOGFILENAME"))) {
        char *cp1 = getenv("UPPLAY_UPNPLOGLEVEL");
        int loglevel = LibUPnP::LogLevelNone;
        if (cp1) {
            loglevel = atoi(cp1);
        }
        loglevel = loglevel < 0 ? 0: loglevel;
        loglevel = loglevel > int(LibUPnP::LogLevelDebug) ? 
            int(LibUPnP::LogLevelDebug) : loglevel;

        if (loglevel != LibUPnP::LogLevelNone) {
            mylib->setLogFileName(cp, LibUPnP::LogLevel(loglevel));
        }
    }

    Application application(&app, params.size(), 0);
    if(!application.is_initialized()) 
        return 1;

    return app.exec();
}
