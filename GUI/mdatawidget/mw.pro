TEMPLATE        = app
LANGUAGE        = C++

QMAKE_CXXFLAGS += -std=c++0x 

CONFIG  += qt warn_on thread release 

HEADERS += mdatawidgetif.h mdatawidget.h \
        ../../HelperStructs/CSettingsStorage.h

SOURCES += mw.cpp mdatawidget.cpp \
        ../../HelperStructs/Helper.cpp \
        ../../HelperStructs/CSettingsStorage.cpp
        
#FORMS   = mdatawidget.ui

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  INCLUDEPATH += ../../
}
