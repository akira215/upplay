#!/bin/sh
fatal()
{
    echo $*
    exit 1
}
Usage()
{
    fatal mkinstdir.sh targetdir
}

test $# -eq 1 || Usage

DESTDIR=$1

test -d $DESTDIR || mkdir $DESTDIR || fatal cant create $DESTDIR

# Script to make a prototype recoll install directory from locally compiled
# software. *** Needs msys or cygwin ***

################################
# Local values (to be adjusted)

# Upplay src tree
UPP=c:/users/bill/documents/upnp/upplay

ReleaseBuild=y

# Where to find libgcc_s_dw2-1.dll for progs which need it copied
gccpath=`which gcc`
MINGWBIN=`dirname $gccpath`

test -z "$MINGWBIN" && fatal cannot find gcc

# Where to copy the Qt Dlls from:
QTBIN=C:/Qt/5.5/mingw492_32/bin
PATH=$QTBIN:$PATH
export PATH

# Qt arch
QTA=Desktop_Qt_5_5_0_MinGW_32bit
if test X$ReleaseBuild = X'y'; then
    qtsdir=release
else
    qtsdir=debug
fi

# Release/debug needs to be changed in the left pane of qtcreator for
# this to work
GUIBIN=$UPP/../build-upplay-${QTA}-${qtsdir}/${qtsdir}/upplay.exe

# checkcopy. 
chkcp()
{
    cp $@ || fatal cp $@ failed
}

copyupplay()
{
    chkcp $UPP/dirbrowser/cdbrowser.css $DESTDIR//share/cdbrowser
    chkcp $UPP/dirbrowser/standard.css $DESTDIR/share/cdbrowser
    chkcp $UPP/dirbrowser/dark.css $DESTDIR/share/cdbrowser
    cp -rp $UPP/GUI/icons $DESTDIR/share
    chkcp $UPP/GUI/standard.css $DESTDIR/share
    chkcp $UPP/GUI/dark.css $DESTDIR/share
    chkcp $UPP/GUI/common.css $DESTDIR/share
    chkcp $UPP/GUI/icons/upplay.ico $DESTDIR
    chkcp -rp $UPP/GUI/icons $DESTDIR
    chkcp $GUIBIN $DESTDIR
}

copyqt()
{
    cd $DESTDIR
    $QTBIN/windeployqt upplay.exe
}


test -d $DESTDIR/share/cdbrowser || mkdir -p $DESTDIR/share/cdbrowser || exit 1
copyupplay
copyqt
