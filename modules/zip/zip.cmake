
################################################################################
# External dependency: ZLIB
################################################################################

IF(WIN32)
    SET(MOD_INCLUDE_DIRECTORIES "${MOD_DIR}/ext/zlib/include")

    IF(VRN_WIN32)
        SET(MOD_LIBRARIES    "${MOD_DIR}/ext/zlib/lib/win32/zdll.lib")
        SET(MOD_DEBUG_DLLS   "${MOD_DIR}/ext/zlib/lib/win32/zlib1.dll")
        SET(MOD_RELEASE_DLLS "${MOD_DIR}/ext/zlib/lib/win32/zlib1.dll")
    ELSEIF(VRN_WIN64)
        SET(MOD_LIBRARIES    "${MOD_DIR}/ext/zlib/lib/win64/zlibwapi.lib")
        SET(MOD_DEBUG_DLLS   "${MOD_DIR}/ext/zlib/lib/win64/zlibwapi.dll")
        SET(MOD_RELEASE_DLLS "${MOD_DIR}/ext/zlib/lib/win64/zlibwapi.dll")
        
        # 64 bit lib has been built with WINAPI calling conventions
        # (see zlib-1.2.5 source distribution: contrib/vstudio/readme.txt)
        LIST(APPEND MOD_DEFINITIONS "-DZLIB_WINAPI")
    ENDIF()
    
    # deployment
    SET(MOD_INSTALL_FILES
        ${MOD_DIR}/ext/zlib/license.txt
    )

ELSEIF(UNIX)
    FIND_PACKAGE(ZLIB REQUIRED)
    IF(ZLIB_FOUND)
        MESSAGE(STATUS "  - Found ZLIB library")
        SET(MOD_INCLUDE_DIRECTORIES ${ZLIB_INCLUDE_DIRS})
        SET(MOD_LIBRARIES ${ZLIB_LIBRARIES})
    ELSE()
        MESSAGE(FATAL_ERROR "ZLIB not found!")
    ENDIF()
ENDIF()


################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS ZIPModule)

#SET(MOD_CORE_SOURCES )

#SET(MOD_CORE_HEADERS )
   