TEMPLATE        = app
LANGUAGE        = C++

QT += script xml network

# WEBPLATFORM = webengine
contains(WEBPLATFORM, webengine) {
    QT += widgets webenginewidgets
    QMAKE_CXXFLAGS += -DUSING_WEBENGINE
} else {
    # Webkit (qt4 or 5)
    QT += webkit
    greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webkitwidgets
}

# VERSION is ^egrepped and must stay in the first column
VERSION = 1.2.8
COPYRDATES = 2011-2017

QMAKE_CXXFLAGS += -DUPPLAY_VERSION=\\\"$$VERSION\\\" 
QMAKE_CXXFLAGS += -DUPPLAY_COPYRDATES=\\\"$$COPYRDATES\\\" 
QMAKE_CXXFLAGS += -std=c++0x

buildtype = release
CONFIG  += qt warn_on thread $$buildtype

# DEFINES += UPPLAY_HORIZONTAL_LAYOUT

# This is only used for the dir browser tab close button, which is set in
# the style sheet, did not find any other reasonably simple way to do it.
RESOURCES += GUI/upplay.qrc

HEADERS += \
        GUI/mainw/mainw.h \
        GUI/mainw/trayicon.h \
        GUI/mdatawidget/mdatawidget.h \
        GUI/mdatawidget/mdatawidgetif.h \
        GUI/playctlwidget/playctlwidget.h \
        GUI/playctlwidget/playctlwidgetif.h \
        GUI/playerwidget/playerhwidget.h \
        GUI/playerwidget/playervwidget.h \
        GUI/playlist/GUI_Playlist.h \
        GUI/playlist/delegate/PlaylistItemDelegate.h \
        GUI/playlist/entry/GUI_PlaylistEntry.h \
        GUI/playlist/entry/GUI_PlaylistEntryBig.h \
        GUI/playlist/entry/GUI_PlaylistEntrySmall.h \
        GUI/playlist/model/PlaylistItemModel.h \
        GUI/playlist/view/ContextMenu.h \
        GUI/playlist/view/PlaylistView.h \
        GUI/prefs/prefs.h \
        GUI/prefs/sortprefs.h \
        GUI/progresswidget/progresswidget.h \
        GUI/progresswidget/progresswidgetif.h \
        GUI/renderchoose/renderchoose.h \
        GUI/songcast/songcastdlg.h \
        GUI/sourcechoose/sourcechoose.h \
        GUI/volumewidget/soundslider.h  \
        GUI/volumewidget/volumewidget.h \
        GUI/volumewidget/volumewidgetif.h \
        GUI/widgets/directslider.h  \
        HelperStructs/CSettingsStorage.h \
        HelperStructs/Helper.h \
        HelperStructs/PlaylistMode.h \
        HelperStructs/Style.h \
        application.h \
        dirbrowser/cdbrowser.h \
        dirbrowser/dirbrowser.h \
        dirbrowser/randplayer.h \
        dirbrowser/rreaper.h \
        notifications/audioscrobbler.h \
        notifications/notifications.h \
        playlist/playlist.h \
        playlist/playlistavt.h \
        playlist/playlistlocrd.h \
        playlist/playlistnull.h \
        playlist/playlistohpl.h \
        playlist/playlistohrcv.h \
        playlist/playlistohrd.h \
        upadapt/avtadapt.h \
        upadapt/ohifadapt.h \
        upadapt/ohpladapt.h \
        upadapt/ohrdadapt.h \
        upadapt/songcast.h \
        upqo/avtransport_qo.h \
        upqo/cdirectory_qo.h \
        upqo/ohinfo_qo.h \
        upqo/ohplaylist_qo.h \
        upqo/ohproduct_qo.h \
        upqo/ohradio_qo.h \
        upqo/ohreceiver_qo.h \
        upqo/ohtime_qo.h \
        upqo/ohvolume_qo.h \
        upqo/renderingcontrol_qo.h \
        utils/confgui.h \
        utils/smallut.h
                        

SOURCES += \
        GUI/mainw/mainw.cpp \
        GUI/mainw/mw_connections.cpp  \
        GUI/mainw/mw_controls.cpp  \
        GUI/mainw/mw_cover.cpp  \
        GUI/mainw/mw_menubar.cpp  \
        GUI/mainw/trayicon.cpp \
        GUI/mdatawidget/mdatawidget.cpp \
        GUI/playctlwidget/playctlwidget.cpp \
        GUI/playerwidget/playerwidget.cpp \
        GUI/playlist/GUI_Playlist.cpp \
        GUI/playlist/delegate/PlaylistItemDelegate.cpp \
        GUI/playlist/entry/GUI_PlaylistEntryBig.cpp \
        GUI/playlist/entry/GUI_PlaylistEntrySmall.cpp \
        GUI/playlist/model/PlaylistItemModel.cpp \
        GUI/playlist/view/ContextMenu.cpp \
        GUI/playlist/view/PlaylistView.cpp \
        GUI/prefs/prefs.cpp \
        GUI/prefs/sortprefs.cpp \
        GUI/progresswidget/progresswidget.cpp \
        GUI/songcast/songcastdlg.cpp \
        GUI/volumewidget/soundslider.cpp  \
        GUI/volumewidget/volumewidget.cpp \
        GUI/widgets/directslider.cpp \
        HelperStructs/CSettingsStorage.cpp \
        HelperStructs/Helper.cpp \
        HelperStructs/Style.cpp \
        application.cpp \
        dirbrowser/cdbrowser.cpp \
        dirbrowser/dirbrowser.cpp \
        dirbrowser/randplayer.cpp \
        dirbrowser/rreaper.cpp \
        notifications/audioscrobbler.cpp \
        notifications/notifications.cpp \
        playlist/playlist.cpp \
        playlist/playlistavt.cpp \
        playlist/playlistlocrd.cpp \
        playlist/playlistohpl.cpp \
        playlist/playlistohrd.cpp \
        upadapt/songcast.cpp \
        upadapt/upputils.cpp \
        upplay.cpp \
        upqo/ohpool.cpp \
        utils/confgui.cpp \
        utils/smallut.cpp

FORMS   = \
        GUI/mainw/mainhw.ui \
        GUI/mainw/mainw.ui \
        GUI/playctlwidget/playctlwidget.ui \
        GUI/playerwidget/playerhwidget.ui \
        GUI/playerwidget/playervwidget.ui \
        GUI/playlist/GUI_Playlist.ui \
        GUI/playlist/entry/GUI_PlaylistEntryBig.ui \
        GUI/playlist/entry/GUI_PlaylistEntrySmall.ui \
        GUI/prefs/sortprefs.ui \
        GUI/progresswidget/progresswidget.ui \
        GUI/renderchoose/renderchoose.ui \
        GUI/songcast/songcastdlg.ui \
        GUI/sourcechoose/sourcechoose.ui \
        dirbrowser/dirbrowser.ui

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  LIBS += -lupnpp
  isEmpty(PREFIX) {
    PREFIX = /usr
  }
  message("Prefix is $$PREFIX")
  DEFINES += PREFIX=\\\"$$PREFIX\\\"

 # Installation stuff
  target.path = "$$PREFIX/bin"

  bdata.files = dirbrowser/cdbrowser.css dirbrowser/dark.css \
              dirbrowser/standard.css dirbrowser/containerscript.js
  bdata.path = $$PREFIX/share/upplay/cdbrowser   
  gdata.files = GUI/common.css GUI/standard.css GUI/dark.css
  gdata.path = $$PREFIX/share/upplay/
  desktop.files += upplay.desktop
  desktop.path = $$PREFIX/share/applications/
  icona.files = GUI/icons/upplay.png
  icona.path = $$PREFIX/share/icons/hicolor/48x48/apps/
  iconb.files = GUI/icons/upplay.png
  iconb.path = $$PREFIX/share/pixmaps/
  iconc.files = GUI/icons/*.png
  iconc.path = $$PREFIX/share/upplay/icons
  icond.files = GUI/icons/*.png GUI/icons/dark/*.png
  icond.path = $$PREFIX/share/upplay/icons/dark

  INSTALLS += target bdata desktop gdata icona iconb iconc icond
}

windows {
    contains(QMAKE_CC, gcc){
        # MingW
        QMAKE_CXXFLAGS += -std=c++11 -Wno-unused-parameter
    }
    contains(QMAKE_CC, cl){
        # Visual Studio
    }
  DEFINES += WIN32
  DEFINES -= UNICODE
  DEFINES -= _UNICODE
  DEFINES += _MBCS
  DEFINES += PSAPI_VERSION=1
  INCLUDEPATH += c:/users/bill/documents/upnp/libupnpp
  INCLUDEPATH += c:/users/bill/documents/upnp/pupnp/include
  LIBS += -L../libupnpp/windows/build-libupnpp-Desktop_Qt_5_5_0_MinGW_32bit-$$buildtype/$$buildtype -lupnpp
  LIBS += c:/users/bill/documents/upnp/expat-2.1.0/.libs/libexpat.a
  LIBS += c:/users/bill/documents/upnp/curl-7.43.0/lib/libcurl.a
  LIBS += c:/users/bill/documents/upnp/pupnp/upnp/.libs/libupnp.a
  LIBS += c:/users/bill/documents/upnp/pupnp/ixml/.libs/libixml.a
  LIBS += c:/users/bill/documents/upnp/pupnp/threadutil/.libs/libthreadutil.a
  LIBS += -liphlpapi
  LIBS += -lwldap32
  LIBS += -lws2_32
  LIBS += -lshlwapi
}
