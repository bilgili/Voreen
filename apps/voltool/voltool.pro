####################################################
# Project file for the Voreen voltool app
####################################################
TARGET = voltool
TEMPLATE	= app
LANGUAGE	= C++

CONFIG += console
CONFIG -= qt

# Include local configuration
include(../../config.txt)

# Include common configuration
include(../../commonconf.pri)

include(../voreenapp.pri)

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

SOURCES	+= voltool.cpp \
           commands_grad.cpp \
           commands_convert.cpp \
           commands_create.cpp \
           commands_modify.cpp \
           commands_registration.cpp

HEADERS +=  commands_grad.h \
            commands_convert.h \
            commands_create.h \
            commands_modify.h \
            commands_registration.h

