####################################################
# Project file for the Voreen Qt application
####################################################
TARGET = voreendev
TEMPLATE = app
LANGUAGE = C++

CONFIG += qt dll
QT += opengl network

# Include local configuration
!include(../../config.txt) {
  error("config.txt not found! copy config-default.txt to config.txt and edit!")
}

# Include common configuration
include(../../commonconf.txt)

# Include generic app configuration
include(../voreenapp.txt)

debug {
    DEFINES += tgtDEBUG
}

PRECOMPILED_HEADER = pch.h

FORMS = ui/aboutbox.ui

SOURCES +=  main.cpp \
            cmdlineparser.cpp \
            iosystem.cpp \
            mainframe.cpp

HEADERS +=  mainframe.h \
            cmdlineparser.h \
            iosystem.h

contains(DEFINES, VRN_WITH_DCMTK) {
   SOURCES +=  \
            ../../src/qt/dicomdialog.cpp \
            complexwizard.cpp
   HEADERS += \
            voreen/qt/dicomdialog.h \
            complexwizard.h
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

# this must come after all SOURCES, HEADERS and FORMS have been added
contains(DEFINES, VRN_WITH_SVNVERSION) : revtarget.depends = $$SOURCES $$FORMS #$$HEADERS
### Local Variables:
### mode:conf-unix
### End:
