
win32 {
    win32-msvc2005: LIBS += "$${VRN_MODULE_SRC_DIR}/python/ext/python26/$${MSC_CONFIG}/VS2005/python26.lib"
    win32-msvc2008: LIBS += "$${VRN_MODULE_SRC_DIR}/python/ext/python26/$${MSC_CONFIG}/python26.lib"
    win32-g++:      LIBS += "$${VRN_MODULE_SRC_DIR}/python/ext/python26/$${MSC_CONFIG}/VS2005/python26.lib"
}