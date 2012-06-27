TARGET = simple-memcheck
TEMPLATE	= app
LANGUAGE	= C++

CONFIG += console

CONFIG -= qt

# Include local configuration
include(../../config.txt)

# Include common configuration
include(../../commonconf.pri)

# Include generic app configuration
include(../voreenapp.pri)

SOURCES += \
    simple-memcheck.cpp \

