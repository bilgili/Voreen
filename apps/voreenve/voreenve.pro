TARGET = voreenve
TEMPLATE = app
LANGUAGE = C++

CONFIG += console qt
QT += opengl

# Include local configuration
include(../../config.txt)

# Include common configuration
include(../../commonconf.txt)

# Include generic app configuration
include(../voreenapp.txt)

HEADERS += \
    voreencanvaswidget.h \
    voreenmainwindow.h \
    workspace.h 

    
contains(DEFINES, VRN_WITH_DCMTK) {
  HEADERS += voreen/qt/dicomdialog.h
}

SOURCES += \
    main.cpp \
    voreencanvaswidget.cpp \
    voreenmainwindow.cpp

contains(DEFINES, VRN_WITH_DCMTK) {
  SOURCES +=  ../../src/qt/dicomdialog.cpp
}

# for compiled-in files (see Qt resource system)
!contains(DEFINES, VRN_SNAPSHOT) {
   RESOURCES += voreenve.qrc 
}

win32 {
    # icon description file for windows-exe
    RC_FILE = "../../resource/vrn_share/icons/winicon.rc"
}

macx {
  # icon for the application bundle
  ICON = "$${VRN_HOME}/resource/vrn_share/icons/icon.icns"
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
