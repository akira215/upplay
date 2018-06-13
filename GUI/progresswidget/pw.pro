TEMPLATE        = app
LANGUAGE        = C++

QMAKE_CXXFLAGS += -std=c++0x 

CONFIG  += qt warn_on thread release 

HEADERS += progresswidgetif.h progresswidget.h ../widgets/DirectSlider.h \
        ../../HelperStructs/CSettingsStorage.h

SOURCES += pw.cpp progresswidget.cpp \
        ../widgets/DirectSlider.cpp \
        ../../HelperStructs/Helper.cpp \
        ../../HelperStructs/CSettingsStorage.cpp
        
FORMS   = progresswidget.ui

RESOURCES = ../upplay.qrc

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  INCLUDEPATH += ../../
}
