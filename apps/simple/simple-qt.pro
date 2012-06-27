TARGET = simple-qt
TEMPLATE = app
LANGUAGE = C++

CONFIG += qt console
QT += opengl

# Include local configuration
include(../../config.txt)

# Include common configuration
include(../../commonconf.pri)

include(../voreenapp.pri)

HEADERS += ../../ext/tgt/qt/qtcanvas.h

SOURCES += simple-qt.cpp 

unix {
  DEFINES += LINUX

#  LIBS += -lGL -lGLU -lGLEW
#  LIBS += -lvoreen_core -lziparch
#  LIBS += -lIL -lILU -lILUT
}
