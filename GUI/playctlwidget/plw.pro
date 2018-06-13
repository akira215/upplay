TEMPLATE        = app
LANGUAGE        = C++

QMAKE_CXXFLAGS += -std=c++0x 

CONFIG  += qt warn_on thread release 

HEADERS += playctlwidgetif.h playctlwidget.h \
        ../../HelperStructs/CSettingsStorage.h

SOURCES += plw.cpp playctlwidget.cpp \
        ../../HelperStructs/Helper.cpp \
        ../../HelperStructs/CSettingsStorage.cpp
        
FORMS   = playctlwidget.ui

#RESOURCES = ../upplay.qrc

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  INCLUDEPATH += ../../
}
