####################################################
# Project file for the Voreen glut example
####################################################
TARGET = glutexample
TEMPLATE	= app
LANGUAGE	= C++

CONFIG -= qt


# Include local configuration
include(../../config.txt)

# Include common configuration
include(../../commonconf.txt)

# Include generic app configuration
include(../voreenapp.txt)

win32 {
  INCLUDEPATH += \
    $${VRN_HOME}/ext/glut/ \
    $${VRN_HOME}
  QMAKE_LIBDIR += $${VRN_HOME}/ext/glut
  LIBS += -lglut32
}

unix {
  LIBS += -lglut
}

SOURCES += \
    glutexample.cpp \
    $${VRN_HOME}/ext/tgt/glut/glutcanvas.cpp \
    $${VRN_HOME}/ext/tgt/glut/glutmouse.cpp

HEADERS += \
    $${VRN_HOME}/ext/tgt/glut/glutcanvas.h \
    $${VRN_HOME}/ext/tgt/glut/glutmouse.h 
