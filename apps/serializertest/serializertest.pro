TARGET = serializertest
TEMPLATE = app
LANGUAGE = C++

CONFIG -= qt
CONFIG += console

# Include local configuration
include(../../config.txt)

# Include common configuration
include(../../commonconf.txt)

include(../voreenapp.txt)

# HEADERS += ../../ext/tgt/qt/qtcanvas.h

SOURCES += serializertest.cpp

unix {
  DEFINES += LINUX

#  LIBS += -lGL -lGLU -lGLEW
#  LIBS += -lvoreen_core -lziparch
#  LIBS += -lIL -lILU -lILUT
}
