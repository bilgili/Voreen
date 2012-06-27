DEFINES += VRN_MODULE_HPMC

# module class  
VRN_MODULE_CLASSES += HPMCModule
VRN_MODULE_CLASS_HEADERS += hpmc/hpmcmodule.h
VRN_MODULE_CLASS_SOURCES += hpmc/hpmcmodule.cpp

INCLUDEPATH += "$${VRN_MODULE_INC_DIR}/hpmc/ext/hpmc/include"
