
################################################################################
# External dependency: TIFF library
################################################################################

IF(WIN32)
    SET(MOD_INCLUDE_DIRECTORIES "${MOD_DIR}/ext/libtiff/include")

    IF(VRN_WIN32)
        SET(MOD_LIBRARIES
            "${MOD_DIR}/ext/libtiff/lib/win32/libtiff.lib"
        )
        
        SET(MOD_DEBUG_DLLS
            "${MOD_DIR}/ext/libtiff/lib/win32/libtiff3.dll"
        )
        SET(MOD_RELEASE_DLLS
            "${MOD_DIR}/ext/libtiff/lib/win32/libtiff3.dll"
        )
    ELSEIF(VRN_WIN64)
        SET(MOD_DEBUG_LIBRARIES 
            "${MOD_DIR}/ext/libtiff/lib/win64/debug/libtiff.lib"
        )
        SET(MOD_RELEASE_LIBRARIES 
            "${MOD_DIR}/ext/libtiff/lib/win64/release/libtiff.lib"
        )
        
        SET(MOD_DEBUG_DLLS
            "${MOD_DIR}/ext/libtiff/lib/win64/debug/libtiff.dll"
        )
        SET(MOD_RELEASE_DLLS
            "${MOD_DIR}/ext/libtiff/lib/win64/release/libtiff.dll"
        )
    ENDIF()
    
    # deployment
    SET(MOD_INSTALL_FILES
        ${MOD_DIR}/ext/libtiff/COPYRIGHT
    )

ELSEIF(UNIX)
    FIND_PACKAGE(TIFF REQUIRED)
    IF(TIFF_FOUND)
        MESSAGE(STATUS "  - Found TIFF library")
        SET(MOD_INCLUDE_DIRECTORIES ${TIFF_INCLUDE_DIR})
        SET(MOD_LIBRARIES ${TIFF_LIBRARIES})
    ELSE()
        MESSAGE(FATAL_ERROR "Tiff library not found!")
    ENDIF()
ENDIF()


################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS TiffModule)

SET(MOD_CORE_SOURCES 
    ${MOD_DIR}/io/tiffvolumereader.cpp
    ${MOD_DIR}/io/ometiffvolumereader.cpp
    ${MOD_DIR}/io/volumediskometiff.cpp
)

SET(MOD_CORE_HEADERS 
    ${MOD_DIR}/io/tiffvolumereader.h
    ${MOD_DIR}/io/ometiffvolumereader.h
    ${MOD_DIR}/io/volumediskometiff.h
)
   