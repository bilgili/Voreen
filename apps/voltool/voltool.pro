####################################################
# Project file for the Voreen voltool app
####################################################
TARGET = voltool
TEMPLATE	= app
LANGUAGE	= C++

# Include local configuration
include(../../config.txt)

# Include common configuration
include(../../commonconf.txt)

include(../voreenapp.txt)

CONFIG += console

SOURCES	+= voltool.cpp \
           command.cpp \   
           commands_grad.cpp \
           commands_convert.cpp \
		   #commands_histo.cpp \
           commands_create.cpp \
           commands_modify.cpp

HEADERS +=  command.h \
            commands_grad.h \
            commands_convert.h \
            #commands_histo.h \
            commands_create.h \
            commands_modify.h

# add files, which are not available in the snapshot release
# these files will be added to the snapshot, when they are cleaned up
!contains(DEFINES, VRN_SNAPSHOT) {
SOURCES	+= vqhelper.cpp \
	       aopreprocess.cpp \
           commands_motion.cpp \
           commands_dao.cpp

HEADERS +=  vqhelper.h \
	        aopreprocess.h  \
            commands_motion.h \
            commands_dao.h
}