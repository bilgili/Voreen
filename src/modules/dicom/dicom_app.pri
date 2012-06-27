# specify libraries to link

# include module configuration
include(dicom_config.txt)

win32 {
  
  CONFIG(debug, debug|release) {
    MSC_CONFIG = Debug
  } else {
    MSC_CONFIG = Release
  }
  
  visual_studio {
    # Contains "Release" or "Debug" as selected in the IDE
    MSC_CONFIG = $(ConfigurationName)
  }

  
  # DCMTK
  exists("$${DCMTK_LIB_DIR}/Debug/dcmimage.lib") {
    LIBS += "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/dcmdata.lib"  \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/dcmdsig.lib"  \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/dcmimage.lib" \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/dcmimgle.lib" \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/dcmjpeg.lib"  \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/dcmnet.lib"   \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/dcmpstat.lib" \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/dcmqrdb.lib"  \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/dcmsr.lib"    \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/dcmtls.lib"   \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/dcmwlm.lib"   \            
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/ijg8.lib"     \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/ijg12.lib"    \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/ijg16.lib"    \
            "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/ofstd.lib"    
    
    contains(DEFINES, VRN_DCMTK_VERSION_355) {
      LIBS += "$${DCMTK_LIB_DIR}/$${MSC_CONFIG}/oflog.lib" 
    }
  } 
  else { 
    error("DICOM module: DCMTK library not found (dcmimage.lib). \ 
           Copy DCMTK libs to $${DCMTK_LIB_DIR} or adapt dicom_config.txt.")      
  }  
        
}

unix {

  LIBS += -lz -lssl
  !without_libwrap: LIBS += -lwrap

  LIBS += -ldcmimage -ldcmimgle -ldcmnet -ldcmdata \
          -ldcmjpeg -lijg8 -lijg12 -lijg16 -lofstd

  LIBS += -ldcmtls

  macx: LIBS += -lcrypto

}

### Local Variables:
### mode:conf-unix
### End: