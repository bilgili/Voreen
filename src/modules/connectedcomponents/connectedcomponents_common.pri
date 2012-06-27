DEFINES += VRN_MODULE_CONNECTEDCOMPONENTS

# module class  
VRN_MODULE_CLASSES += ConnectedComponentsModule
VRN_MODULE_CLASS_HEADERS += connectedcomponents/connectedcomponentsmodule.h
VRN_MODULE_CLASS_SOURCES += connectedcomponents/connectedcomponentsmodule.cpp

# external dependency: connexe library
INCLUDEPATH += $${VRN_MODULE_INC_DIR}/connectedcomponents/ext/connexe/include
SOURCES += "$${VRN_MODULE_SRC_DIR}/connectedcomponents/ext/connexe/connexe.c
