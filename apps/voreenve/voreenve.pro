TARGET = voreenve
TEMPLATE = app
LANGUAGE = C++

CONFIG += qt console
QT += opengl

# Include local configuration
include(../../config.txt)

# Include common configuration
include(../../commonconf.txt)

# Include generic app configuration
include(../voreenapp.txt)

FORMS = ui/aboutbox.ui

HEADERS += \
    rptprocessoritem.h \
    rptpropertysetitem.h \
    rptpropertybutton.h \
    rptmainwindow.h \
    rptgraphwidget.h \
    rptprocessorlistwidget.h \
    rptaggregationlistwidget.h \
    rptarrow.h \
    rpttcttooltip.h \
    rpttooltiptimer.h \
    rptpropertylistwidget.h \
    rptpainterwidget.h \
    rptnetworkserializergui.h \
    rptaggregationitem.h \
    rptguiitem.h \
    iosystem.h

contains(DEFINES, VRN_WITH_DCMTK) {
  HEADERS += voreen/qt/dicomdialog.h
}		
          
           
SOURCES += \
    rptprocessoritem.cpp \
    rptpropertysetitem.cpp \
    rptpropertybutton.cpp \
    rptmainwindow.cpp \
    rptgraphwidget.cpp \
    rptprocessorlistwidget.cpp \
    rptaggregationlistwidget.cpp \
    rptarrow.cpp \
    rpttcttooltip.cpp \
    rpttooltiptimer.cpp \
    rptnetworkserializergui.cpp \
    rptpropertylistwidget.cpp \						
    rptpainterwidget.cpp \
    rptaggregationitem.cpp \
    rptguiitem.cpp \
    main.cpp \
    iosystem.cpp
contains(DEFINES, VRN_WITH_DCMTK) {
  SOURCES +=  ../../src/qt/dicomdialog.cpp
}

# for compiled-in files (see Qt resource system)
!contains(DEFINES, VRN_SNAPSHOT) {
   RESOURCES += voreenve.qrc 
}

win32 {
    # icon description file for windows-exe
    RC_FILE = icons/winicon.rc

}

unix {
    # Prevent a namespace clash
   DEFINES += QT_CLEAN_NAMESPACE

   UI_DIR = .ui
}

contains(DEFINES, VRN_WITH_CLOSEUP_RENDERER) {
   LIBS += -lBox2D
}

### Local Variables:
### mode:conf-unix
### End:
