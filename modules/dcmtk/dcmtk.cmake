
################################################################################
# External dependency: DCMTK library
################################################################################

# DCMTK version selection
OPTION(VRN_DCMTK_VERSION_354    "Use DCMTK version 3.5.4"   OFF)
OPTION(VRN_DCMTK_VERSION_360    "Use DCMTK version 3.6.0"   ON )

IF((NOT VRN_DCMTK_VERSION_354 AND NOT VRN_DCMTK_VERSION_360) OR
        (VRN_DCMTK_VERSION_354 AND VRN_DCMTK_VERSION_360))
    MESSAGE(FATAL_ERROR "Please select either option VRN_DCMTK_VERSION_354 or VRN_DCMTK_VERSION_360")
ENDIF()

# Definition
IF(VRN_DCMTK_VERSION_354)
    SET(MOD_DEFINITIONS "-DVRN_DCMTK_VERSION_354")
ELSEIF(VRN_DCMTK_VERSION_360)
    SET(MOD_DEFINITIONS "-DVRN_DCMTK_VERSION_360")
ENDIF()

# Library
IF(WIN32)
    IF(VRN_WIN32)
        # DCMTK include path
        SET(MOD_INCLUDE_DIRECTORIES "${MOD_DIR}/ext/dcmtk/include")
        IF(NOT EXISTS "${MOD_DIR}/ext/dcmtk/include/dcmtk/config/osconfig.h")
            MESSAGE(FATAL_ERROR "DCMTK library not found (osconfig.h). "
                "Copy DCMTK headers to ${MOD_DIR}/ext/dcmtk/include/ (see http://voreen.uni-muenster.de)")
        ENDIF()
        
        # OpenSSL include path (only for v3.6.0)
        IF(VRN_DCMTK_VERSION_360)
            LIST(APPEND MOD_INCLUDE_DIRECTORIES "${MOD_DIR}/ext/openssl/include")
            IF(NOT EXISTS "${MOD_DIR}/ext/openssl/include/openssl/opensslconf.h")
                MESSAGE(FATAL_ERROR "OpenSSL library not found (opensslconf.h). "
                       "Copy OpenSSL headers to ${MOD_DIR}/ext/openssl/include/ (see http://voreen.uni-muenster.de).")
            ENDIF()
        ENDIF()

        # DCMTK library
        IF(NOT EXISTS "${MOD_DIR}/ext/dcmtk/lib/win32/Debug/dcmimage.lib")
            MESSAGE(FATAL_ERROR "DCMTK library not found (dcmimage.lib). "
                   "Copy DCMTK libs to ${MOD_DIR}/ext/dcmtk/lib/win32/[Debug|Release]/ (see http://voreen.uni-muenster.de)")
        ENDIF()
        SET(MOD_DEBUG_LIBRARIES 
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/dcmdata.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/dcmdsig.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/dcmimage.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/dcmimgle.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/dcmjpeg.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/dcmnet.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/dcmpstat.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/dcmqrdb.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/dcmsr.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/dcmtls.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/dcmwlm.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/ijg8.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/ijg12.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/ijg16.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/debug/ofstd.lib"
        )
        SET(MOD_RELEASE_LIBRARIES 
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/dcmdata.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/dcmdsig.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/dcmimage.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/dcmimgle.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/dcmjpeg.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/dcmnet.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/dcmpstat.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/dcmqrdb.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/dcmsr.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/dcmtls.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/dcmwlm.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/ijg8.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/ijg12.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/ijg16.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win32/release/ofstd.lib"
        )
        IF(VRN_DCMTK_VERSION_360)
            LIST(APPEND MOD_DEBUG_LIBRARIES "${MOD_DIR}/ext/dcmtk/lib/win32/debug/oflog.lib")
            LIST(APPEND MOD_RELEASE_LIBRARIES "${MOD_DIR}/ext/dcmtk/lib/win32/release/oflog.lib")
        ENDIF()
        
        # OpenSSL library (only for 3.6.0)
        IF(VRN_DCMTK_VERSION_360)
            LIST(APPEND MOD_LIBRARIES
                "${MOD_DIR}/ext/openssl/lib/win32/dcmtkssl_d.lib"  
                "${MOD_DIR}/ext/openssl/lib/win32/dcmtkeay_d.lib"  
            )

            LIST(APPEND MOD_DEBUG_DLLS
                "${MOD_DIR}/ext/openssl/lib/win32/dcmtkssl.dll"  
                "${MOD_DIR}/ext/openssl/lib/win32/dcmtkeay.dll"  
            )
            LIST(APPEND MOD_RELEASE_DLLS
                "${MOD_DIR}/ext/openssl/lib/win32/dcmtkssl.dll"  
                "${MOD_DIR}/ext/openssl/lib/win32/dcmtkeay.dll"  
            )
        ENDIF()
        
    ELSEIF(VRN_WIN64)
        # DCMTK include path
        SET(MOD_INCLUDE_DIRECTORIES "${MOD_DIR}/ext/dcmtk/include/win64")
        IF(NOT EXISTS "${MOD_DIR}/ext/dcmtk/include/win64/dcmtk/config/osconfig.h")
            MESSAGE(FATAL_ERROR "DCMTK library not found (osconfig.h). "
                "Copy DCMTK headers to ${MOD_DIR}/ext/dcmtk/include/win64 (see README.txt)")
        ENDIF()
        
        # DCMTK library
        IF(NOT EXISTS "${MOD_DIR}/ext/dcmtk/lib/win64/Debug/dcmimage.lib")
            MESSAGE(FATAL_ERROR "DCMTK library not found (dcmimage.lib). "
                   "Copy DCMTK libs to ${MOD_DIR}/ext/dcmtk/lib/win64/[Debug|Release]/ (see README.txt)")
        ENDIF()
        SET(MOD_DEBUG_LIBRARIES 
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/dcmdata.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/dcmdsig.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/dcmimage.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/dcmimgle.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/dcmjpeg.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/dcmnet.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/dcmpstat.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/dcmqrdb.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/dcmsr.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/dcmtls.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/dcmwlm.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/ijg8.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/ijg12.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/ijg16.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/debug/ofstd.lib"
        )
        SET(MOD_RELEASE_LIBRARIES 
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/dcmdata.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/dcmdsig.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/dcmimage.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/dcmimgle.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/dcmjpeg.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/dcmnet.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/dcmpstat.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/dcmqrdb.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/dcmsr.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/dcmtls.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/dcmwlm.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/ijg8.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/ijg12.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/ijg16.lib"
            "${MOD_DIR}/ext/dcmtk/lib/win64/release/ofstd.lib"
        )
        IF(VRN_DCMTK_VERSION_360)
            LIST(APPEND MOD_DEBUG_LIBRARIES   "${MOD_DIR}/ext/dcmtk/lib/win64/debug/oflog.lib")
            LIST(APPEND MOD_RELEASE_LIBRARIES "${MOD_DIR}/ext/dcmtk/lib/win64/release/oflog.lib")
        ENDIF()
        
    ENDIF(VRN_WIN32)
    
    # deployment
    SET(MOD_INSTALL_FILES
        ${MOD_DIR}/ext/dcmtk/COPYRIGHT
        ${MOD_DIR}/ext/openssl/LICENSE
    )
    
ELSEIF(UNIX)
    FIND_PACKAGE(DCMTK REQUIRED)
    IF(DCMTK_FOUND)
        MESSAGE(STATUS "  - Found DCMTK library")
        SET(MOD_INCLUDE_DIRECTORIES ${DCMTK_INCLUDE_DIR} ${DCMTK_INCLUDE_DIRS})
        #SET(MOD_LIBRARIES ${DCMTK_LIBRARIES})
        SET(MOD_LIBRARIES 
            -ldcmtls 
            -ldcmimage 
            -ldcmimgle 
            -ldcmdata 
            -ldcmnet 
            -ldcmjpeg 
            -lijg8 
            -lijg12 
            -lijg16 
            -lofstd 
         )
         IF(VRN_DCMTK_VERSION_360)
             LIST(APPEND MOD_LIBRARIES -loflog) 
         ENDIF()
         LIST(APPEND MOD_LIBRARIES -lz -lssl -lwrap) 
    ELSE()
        MESSAGE(FATAL_ERROR "DCMTK library not found!")
    ENDIF()
ENDIF()


################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS DcmtkModule)

SET(MOD_CORE_SOURCES 
    ${MOD_DIR}/io/dcmtkfindscu.cpp
    ${MOD_DIR}/io/dcmtkmovescu.cpp
    ${MOD_DIR}/io/dcmtkvolumereader.cpp
    ${MOD_DIR}/io/dcmtkvolumewriter.cpp
)

SET(MOD_CORE_HEADERS 
    ${MOD_DIR}/io/voreendcmtk.h
    ${MOD_DIR}/io/dcmtkvolumereader.h
    ${MOD_DIR}/io/dcmtkvolumewriter.h
    ${MOD_DIR}/io/dcmtkfindscu.h
    ${MOD_DIR}/io/dcmtkmovescu.h
)
   
