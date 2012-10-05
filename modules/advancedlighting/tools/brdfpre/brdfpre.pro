####################################################
# Project file for the Voreen voltool app
####################################################
TARGET = brdfpre
TEMPLATE	= app
LANGUAGE	= C++

CONFIG += console
CONFIG -= qt

# check qmake version
QMAKE_VERS = $$[QMAKE_VERSION]
QMAKE_VERSION_CHECK = $$find(QMAKE_VERS, "^[234]\\.")
isEmpty(QMAKE_VERSION_CHECK) {
   error("Your qmake version '$$QMAKE_VERS' is too old, qmake from Qt 4 is required!")
}

# include config
!exists(../../../../config.txt) {
  error("config.txt not found! copy config-default.txt to config.txt and edit!")
}
include(../../../../config.txt)

# Include common configuration
include(../../../../commonconf.pri)

include(../../../../apps/voreenapp.pri)

contains(DEFINES, VRN_PRECOMPILE_HEADER) {
  qt {
    PRECOMPILED_HEADER = ../../../../apps/pch_qtapp.h
  }
  else {
    PRECOMPILED_HEADER = ../../../../pch.h
  }  CONFIG += precompile_header
}

win32 {
 contains(DEFINES, VRN_WITH_DEVIL) {
     LIBS += "$${DEVIL_DIR}/lib/ILU.lib"
  }
}

unix {
  contains(DEFINES, VRN_WITH_DEVIL) {
    LIBS += -lILU
  }
}

SOURCES	+= brdfpre.cpp \
           commands_shgen.cpp

HEADERS +=  commands_shgen.h
