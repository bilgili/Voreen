
################################################################################
# External dependencies: Freetype and FTGL libraries
################################################################################

SET(MOD_DEFINITIONS 
    -DTGT_HAS_FREETYPE
    -DTGT_HAS_FTGL
)

IF(WIN32)
    SET(MOD_INCLUDE_DIRECTORIES 
        "${MOD_DIR}/ext/freetype/include"
        "${MOD_DIR}/ext/freetype/include/freetype"
        "${MOD_DIR}/ext/ftgl/include"
    )

    IF(VRN_WIN32)
        SET(MOD_DEBUG_LIBRARIES 
            "${MOD_DIR}/ext/freetype/lib/win32/debug/freetype.lib"
            "${MOD_DIR}/ext/ftgl/lib/win32/debug/ftgl.lib"
        )
        SET(MOD_RELEASE_LIBRARIES 
            "${MOD_DIR}/ext/freetype/lib/win32/release/freetype.lib"
            "${MOD_DIR}/ext/ftgl/lib/win32/release/ftgl.lib"
        )
        
        SET(MOD_DEBUG_DLLS
            "${MOD_DIR}/ext/freetype/lib/win32/debug/freetype.dll"
            "${MOD_DIR}/ext/ftgl/lib/win32/debug/ftgl.dll"
        )
        SET(MOD_RELEASE_DLLS
            "${MOD_DIR}/ext/freetype/lib/win32/release/freetype.dll"
            "${MOD_DIR}/ext/ftgl/lib/win32/release/ftgl.dll"
        )
    ELSEIF(VRN_WIN64)

        SET(MOD_DEBUG_LIBRARIES 
            "${MOD_DIR}/ext/freetype/lib/win64/debug/freetype.lib"
            "${MOD_DIR}/ext/ftgl/lib/win64/debug/ftgl.lib"
        )
        SET(MOD_RELEASE_LIBRARIES 
            "${MOD_DIR}/ext/freetype/lib/win64/release/freetype.lib"
            "${MOD_DIR}/ext/ftgl/lib/win64/release/ftgl.lib"
        )
        
        SET(MOD_DEBUG_DLLS
            "${MOD_DIR}/ext/freetype/lib/win64/debug/freetype.dll"
            "${MOD_DIR}/ext/ftgl/lib/win64/debug/ftgl.dll"
        )
        SET(MOD_RELEASE_DLLS
            "${MOD_DIR}/ext/freetype/lib/win64/release/freetype.dll"
            "${MOD_DIR}/ext/ftgl/lib/win64/release/ftgl.dll"
        )
    ENDIF()

    # deployment
    SET(MOD_INSTALL_FILES
        ${MOD_DIR}/ext/freetype/FTL.TXT
        ${MOD_DIR}/ext/ftgl/LICENSE
    )
    
ELSEIF(UNIX)
    FIND_PACKAGE(Freetype REQUIRED)
    IF(FREETYPE_FOUND)
        MESSAGE(STATUS "  - Found Freetype library")
        SET(MOD_INCLUDE_DIRECTORIES ${FREETYPE_INCLUDE_DIRS})
        SET(MOD_LIBRARIES ${FREETYPE_LIBRARIES})
    ELSE()
        MESSAGE(FATAL_ERROR "Freetype library not found!")
    ENDIF()
    
    FIND_PACKAGE(FTGLVRN REQUIRED)
    IF(FTGL_FOUND)
        MESSAGE(STATUS "  - Found FTGL library")
        LIST(APPEND MOD_INCLUDE_DIRECTORIES ${FTGL_INCLUDE_DIR})
        LIST(APPEND MOD_LIBRARIES ${FTGL_LIBRARIES})
    ELSE()
        MESSAGE(FATAL_ERROR "FTGL library not found!")
    ENDIF()
    
ENDIF()


################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS FontRenderingModule)

#SET(MOD_CORE_SOURCES )

#SET(MOD_CORE_HEADERS )
   
