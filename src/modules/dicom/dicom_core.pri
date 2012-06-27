# volume reader/writer
SOURCES += $${VRN_MODULE_SRC_DIR}/dicom/dicomvolumereader.cpp
SOURCES += $${VRN_MODULE_SRC_DIR}/dicom/dicomvolumewriter.cpp

HEADERS += $${VRN_MODULE_INC_DIR}/dicom/dicomvolumereader.h
HEADERS += $${VRN_MODULE_INC_DIR}/dicom/dicomvolumewriter.h

# helper classes
SOURCES += $${VRN_MODULE_SRC_DIR}/dicom/dicomfindscu.cpp
SOURCES += $${VRN_MODULE_SRC_DIR}/dicom/dicommovescu.cpp

HEADERS += $${VRN_MODULE_INC_DIR}/dicom/voreendcmtk.h
HEADERS += $${VRN_MODULE_INC_DIR}/dicom/dicomfindscu.h
HEADERS += $${VRN_MODULE_INC_DIR}/dicom/dicommovescu.h

### Local Variables:
### mode:conf-unix
### End:
