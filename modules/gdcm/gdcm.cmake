
################################################################################
# External dependency: GDCM library
################################################################################

IF(WIN32)
    # always use GDCM 2.2 on Windows (included in the distribution)
    SET(MOD_DEFINITIONS -DVRN_GDCM_VERSION_22)
     
    # include path
    SET(GDCM_INCLUDE_DIR ${MOD_DIR}/ext/gdcm-2.2/include)
    IF(NOT EXISTS ${GDCM_INCLUDE_DIR}/gdcm-2.2/gdcmReader.h)
        MESSAGE(FATAL_ERROR "GDCM 2.2 headers not found (${GDCM_INCLUDE_DIR}/gdcm-2.2/gdcmReader.h). "
            "Copy GDCM 2.2 library to modules/gdcm/ext/gdcm-2.2 (see http://voreen.uni-muenster.de)")
    ENDIF()
    SET(MOD_INCLUDE_DIRECTORIES ${GDCM_INCLUDE_DIR})
    
    # library
    IF(VRN_WIN32)
        SET(GDCM_LIB_DIR ${MOD_DIR}/ext/gdcm-2.2/lib/win32)
    ELSEIF(VRN_WIN64)
        SET(GDCM_LIB_DIR ${MOD_DIR}/ext/gdcm-2.2/lib/win64)
    ELSE(VRN_WIN32)
        MESSAGE(FATAL_ERROR "Neither VRN_WIN32 nor VRN_WIN64 defined!")
    ENDIF(VRN_WIN32)
        
    IF(NOT EXISTS ${GDCM_LIB_DIR}/debug/gdcmCommon.lib)
        MESSAGE(FATAL_ERROR "GDCM 2.2 library not found (${GDCM_LIB_DIR}/debug/gdcmCommon.lib). "
            "Copy GDCM 2.2 library to modules/gdcm/ext/gdcm-2.2 (see http://voreen.uni-muenster.de)")
    ENDIF()
        
    SET(MOD_DEBUG_LIBRARIES 
        ${GDCM_LIB_DIR}/debug/gdcmcharls.lib
        ${GDCM_LIB_DIR}/debug/gdcmCommon.lib
        ${GDCM_LIB_DIR}/debug/gdcmDICT.lib
        ${GDCM_LIB_DIR}/debug/gdcmDSED.lib
        ${GDCM_LIB_DIR}/debug/gdcmexpat.lib
        ${GDCM_LIB_DIR}/debug/gdcmgetopt.lib
        ${GDCM_LIB_DIR}/debug/gdcmIOD.lib
        ${GDCM_LIB_DIR}/debug/gdcmjpeg8.lib
        ${GDCM_LIB_DIR}/debug/gdcmjpeg12.lib
        ${GDCM_LIB_DIR}/debug/gdcmjpeg16.lib
        ${GDCM_LIB_DIR}/debug/gdcmMEXD.lib
        ${GDCM_LIB_DIR}/debug/gdcmMSFF.lib
        ${GDCM_LIB_DIR}/debug/gdcmopenjpeg.lib
        ${GDCM_LIB_DIR}/debug/gdcmzlib.lib
    )
    SET(MOD_RELEASE_LIBRARIES 
        ${GDCM_LIB_DIR}/release/gdcmcharls.lib
        ${GDCM_LIB_DIR}/release/gdcmCommon.lib
        ${GDCM_LIB_DIR}/release/gdcmDICT.lib
        ${GDCM_LIB_DIR}/release/gdcmDSED.lib
        ${GDCM_LIB_DIR}/release/gdcmexpat.lib
        ${GDCM_LIB_DIR}/release/gdcmgetopt.lib
        ${GDCM_LIB_DIR}/release/gdcmIOD.lib
        ${GDCM_LIB_DIR}/release/gdcmjpeg8.lib
        ${GDCM_LIB_DIR}/release/gdcmjpeg12.lib
        ${GDCM_LIB_DIR}/release/gdcmjpeg16.lib
        ${GDCM_LIB_DIR}/release/gdcmMEXD.lib
        ${GDCM_LIB_DIR}/release/gdcmMSFF.lib
        ${GDCM_LIB_DIR}/release/gdcmopenjpeg.lib
        ${GDCM_LIB_DIR}/release/gdcmzlib.lib
    )
    
    SET(MOD_DEBUG_DLLS 
        ${GDCM_LIB_DIR}/debug/gdcmcharls.dll
        ${GDCM_LIB_DIR}/debug/gdcmCommon.dll
        ${GDCM_LIB_DIR}/debug/gdcmDICT.dll
        ${GDCM_LIB_DIR}/debug/gdcmDSED.dll
        ${GDCM_LIB_DIR}/debug/gdcmexpat.dll
        ${GDCM_LIB_DIR}/debug/gdcmgetopt.dll
        ${GDCM_LIB_DIR}/debug/gdcmIOD.dll
        ${GDCM_LIB_DIR}/debug/gdcmjpeg8.dll
        ${GDCM_LIB_DIR}/debug/gdcmjpeg12.dll
        ${GDCM_LIB_DIR}/debug/gdcmjpeg16.dll
        ${GDCM_LIB_DIR}/debug/gdcmMEXD.dll
        ${GDCM_LIB_DIR}/debug/gdcmMSFF.dll
        ${GDCM_LIB_DIR}/debug/gdcmopenjpeg.dll
        ${GDCM_LIB_DIR}/debug/gdcmzlib.dll
        ${GDCM_LIB_DIR}/debug/socketxx.dll
    )
    SET(MOD_RELEASE_DLLS
        ${GDCM_LIB_DIR}/release/gdcmcharls.dll
        ${GDCM_LIB_DIR}/release/gdcmCommon.dll
        ${GDCM_LIB_DIR}/release/gdcmDICT.dll
        ${GDCM_LIB_DIR}/release/gdcmDSED.dll
        ${GDCM_LIB_DIR}/release/gdcmexpat.dll
        ${GDCM_LIB_DIR}/release/gdcmgetopt.dll
        ${GDCM_LIB_DIR}/release/gdcmIOD.dll
        ${GDCM_LIB_DIR}/release/gdcmjpeg8.dll
        ${GDCM_LIB_DIR}/release/gdcmjpeg12.dll
        ${GDCM_LIB_DIR}/release/gdcmjpeg16.dll
        ${GDCM_LIB_DIR}/release/gdcmMEXD.dll
        ${GDCM_LIB_DIR}/release/gdcmMSFF.dll
        ${GDCM_LIB_DIR}/release/gdcmopenjpeg.dll
        ${GDCM_LIB_DIR}/release/gdcmzlib.dll
        ${GDCM_LIB_DIR}/release/socketxx.dll
    )        
    
    # deployment
    SET(MOD_INSTALL_FILES
        ${MOD_DIR}/ext/gdcm-2.2/Copyright.txt
    )
    
ELSEIF(UNIX)
    find_package(GDCM)
    IF(GDCM_FOUND)
        SET(GDCM_USE_VTK 0)
        INCLUDE(${GDCM_USE_FILE})

        IF(GDCM_MAJOR_VERSION EQUAL 2)

            IF(GDCM_MINOR_VERSION EQUAL 0)
                IF(GDCM_BUILD_VERSION GREATER 17)
                    SET(MOD_LIBRARIES 
                        -lgdcmMSFF  -lgdcmDICT -lgdcmIOD -lgdcmDSED -lgdcmCommon
                        -lgdcmjpeg8 -lgdcmjpeg12 -lgdcmjpeg16 -lCharLS
                        )
                    MESSAGE( STATUS, "GDCM 2.0.(>18) detected." )
                ELSE()
                    SET(MOD_LIBRARIES 
                        -lgdcmMSFF  -lgdcmDICT -lgdcmIOD -lgdcmDSED -lgdcmCommon
                        -lgdcmjpeg8 -lgdcmjpeg12 -lgdcmjpeg16 -lgdcmcharls
                        )
                    MESSAGE( STATUS, "GDCM 2.0.(<18) detected." )
                ENDIF()

                MESSAGE( STATUS, "GDCM 2.0.x is missing network support!" )
                SET(MOD_DEFINITIONS -DVRN_GDCM_VERSION_20)
            ENDIF()

            IF(GDCM_MINOR_VERSION EQUAL 2)
                MESSAGE( STATUS, "GDCM 2.2.x detected." )
                SET(MOD_DEFINITIONS -DVRN_GDCM_VERSION_22)
                #LIST(APPEND MOD_LIBRARIES -lgdcmMEXD -lgdcmopenjpeg -lgdcmuuid -lgdcmzlib)
                LIST(APPEND MOD_LIBRARIES -lgdcmCommon -lgdcmDICT -lgdcmDSED -lgdcmIOD -lgdcmMEXD -lgdcmMSFF -lgdcmjpeg12 -lgdcmjpeg16 -lgdcmjpeg8)
            ENDIF()

        ELSE()
            MESSAGE(FATAL_ERROR "Unknown GDCM major version.")
        ENDIF()
    ELSE()
        MESSAGE(FATAL_ERROR "GDCM library not found.")
    ENDIF()

ENDIF()


################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS GdcmModule)

SET(MOD_CORE_SOURCES 
    ${MOD_DIR}/customdicomdict.cpp
    ${MOD_DIR}/dicominfo.cpp
    ${MOD_DIR}/dicomdictentry.cpp
    ${MOD_DIR}/dicomdict.cpp
    ${MOD_DIR}/io/dicomdirparser.cpp
    ${MOD_DIR}/io/dicomnetworkconnector.cpp
    ${MOD_DIR}/io/gdcmvolumereader.cpp
    ${MOD_DIR}/io/volumediskdicom.cpp
)

SET(MOD_CORE_HEADERS 
    ${MOD_DIR}/customdicomdict.h
    ${MOD_DIR}/dicominfo.h
    ${MOD_DIR}/dicomdictentry.h
    ${MOD_DIR}/dicomdict.h
    ${MOD_DIR}/io/dicomdirparser.h
    ${MOD_DIR}/io/dicomnetworkconnector.h
    ${MOD_DIR}/io/gdcmvolumereader.h
    ${MOD_DIR}/io/volumediskdicom.h
)

# deployment
LIST(APPEND MOD_INSTALL_DIRECTORIES ${MOD_DIR}/dicts)


################################################################################
# Qt module resources 
################################################################################
#SET(MOD_CQT_MODULECLASS GdcmModule)

SET(MOD_QT_SOURCES 
    ${MOD_DIR}/qt/dicomconnectiondialog.cpp
    ${MOD_DIR}/qt/dicomhierarchymodel.cpp
)

SET(MOD_QT_HEADERS 
    ${MOD_DIR}/qt/dicomconnectiondialog.h
    ${MOD_DIR}/qt/dicomhierarchymodel.h
)
   
SET(MOD_QT_HEADERS_NONMOC
)

