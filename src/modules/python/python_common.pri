DEFINES += VRN_MODULE_PYTHON

# module class  
VRN_MODULE_CLASSES += PythonModule
VRN_MODULE_CLASS_HEADERS += python/pythonmodule.h
VRN_MODULE_CLASS_SOURCES += python/pythonmodule.cpp

# python lib
win32 {
    !defined(PYTHON_DIR) : PYTHON_DIR = $${VRN_MODULE_INC_DIR}/python/ext/python26
    INCLUDEPATH += "$${PYTHON_DIR}"
}
unix {
    INCLUDEPATH += $${PYTHON_DIR}
    LIBS += $${PYTHON_LIBS}
}