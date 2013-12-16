
################################################################################
# External dependency: DevIL library
################################################################################

SET(MOD_DEFINITIONS -DTGT_HAS_DEVIL)

IF(WIN32)
    SET(MOD_INCLUDE_DIRECTORIES "${MOD_DIR}/ext/il/include")

    IF(VRN_WIN32)
        SET(MOD_LIBRARIES 
            "${MOD_DIR}/ext/il/lib/win32/DevIL.lib"
            "${MOD_DIR}/ext/il/lib/win32/ILU.lib"
            #"${MOD_DIR}/ext/il/lib/win32/ILUT.lib" #< currently not needed
        )
        
        SET(MOD_DEBUG_DLLS
            "${MOD_DIR}/ext/il/lib/win32/DevIL.dll"
            "${MOD_DIR}/ext/il/lib/win32/ILU.dll"
            #"${MOD_DIR}/ext/il/lib/win32/ILUT.dll" #< currently not needed
        )
        SET(MOD_RELEASE_DLLS ${MOD_DEBUG_DLLS})
    ELSEIF(VRN_WIN64)
        SET(MOD_LIBRARIES 
            "${MOD_DIR}/ext/il/lib/win64/DevIL.lib"
            "${MOD_DIR}/ext/il/lib/win64/ILU.lib"
            #"${MOD_DIR}/ext/il/lib/win64/ILUT.lib" #< currently not needed
        )
        
        SET(MOD_DEBUG_DLLS
            "${MOD_DIR}/ext/il/lib/win64/DevIL.dll"
            "${MOD_DIR}/ext/il/lib/win64/ILU.dll"
            #"${MOD_DIR}/ext/il/lib/win64/ILUT.dll" #< currently not needed
        )
        SET(MOD_RELEASE_DLLS ${MOD_DEBUG_DLLS})
    ENDIF()
    
    LIST(APPEND MOD_DEBUG_DLLS "${MOD_DIR}/ext/jpeg/jpeg62.dll")
    LIST(APPEND MOD_RELEASE_DLLS "${MOD_DIR}/ext/jpeg/jpeg62.dll")

    # deployment
    SET(MOD_INSTALL_FILES
        ${MOD_DIR}/ext/il/lgpl.txt
        ${MOD_DIR}/ext/il/libpng-LICENSE.txt
        ${MOD_DIR}/ext/jpeg/license.txt
    )
    
ELSEIF(UNIX)
    FIND_PACKAGE(DevIL REQUIRED)
    IF(IL_FOUND)
        MESSAGE(STATUS "  - Found DevIL library")
        SET(MOD_INCLUDE_DIRECTORIES ${IL_INCLUDE_DIR}/..)
        SET(MOD_LIBRARIES 
            ${IL_LIBRARIES}
            ${ILU_LIBRARIES}
            #${ILUT_LIBRARIES} #< currently not needed
        )
    ELSE()
        MESSAGE(FATAL_ERROR "DevIL library not found!")
    ENDIF()
ENDIF()


################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS DevILModule)

#SET(MOD_CORE_SOURCES )

#SET(MOD_CORE_HEADERS )
   
