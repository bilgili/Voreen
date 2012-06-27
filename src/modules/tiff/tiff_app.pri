# link against libtiff

win32 {
  LIBS += "$${VRN_MODULE_SRC_DIR}/tiff/ext/libtiff/libtiff.lib"
}

unix {
  LIBS += -ltiff
}
