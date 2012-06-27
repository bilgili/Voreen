# module availability macro
DEFINES += VRN_MODULE_DICOM

# module class  
VRN_MODULE_CLASSES += DicomModule
VRN_MODULE_CLASS_HEADERS += dicom/dicommodule.h
VRN_MODULE_CLASS_SOURCES += dicom/dicommodule.cpp

# include module configuration
include(dicom_config.txt)

# location library headers on windows
win32 {
  
  # dcmtk
  INCLUDEPATH += "$${DCMTK_INC_DIR}"
  !exists("$${DCMTK_INC_DIR}/dcmtk/config/osconfig.h") {
      error("DICOM module: DCMTK library not found (osconfig.h). \
             Copy DCMTK headers to $${DCMTK_INC_DIR}/dcmtk (see README).")
  }

}

### Local Variables:
### mode:conf-unix
### End: