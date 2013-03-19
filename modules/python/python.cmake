
################################################################################
# External dependency: Python library
################################################################################

IF(WIN32)
    SET(MOD_INCLUDE_DIRECTORIES "${MOD_DIR}/ext/python27/include")

    IF(VRN_WIN32)
        SET(MOD_DEBUG_LIBRARIES 
            "${MOD_DIR}/ext/python27/lib/win32/debug/python27_d.lib"
        )
        SET(MOD_RELEASE_LIBRARIES 
            "${MOD_DIR}/ext/python27/lib/win32/release/python27.lib"
        )
        
        SET(MOD_DEBUG_DLLS
            "${MOD_DIR}/ext/python27/lib/win32/debug/python27_d.dll"
        )
        SET(MOD_RELEASE_DLLS 
            "${MOD_DIR}/ext/python27/lib/win32/release/python27.dll"
        )
    ELSEIF(VRN_WIN64)
        SET(MOD_DEBUG_LIBRARIES 
            "${MOD_DIR}/ext/python27/lib/win64/debug/python27_d.lib"
        )
        SET(MOD_RELEASE_LIBRARIES 
            "${MOD_DIR}/ext/python27/lib/win64/release/python27.lib"
        )
        
        SET(MOD_DEBUG_DLLS
            "${MOD_DIR}/ext/python27/lib/win64/debug/python27_d.dll"
        )
        SET(MOD_RELEASE_DLLS 
            "${MOD_DIR}/ext/python27/lib/win64/release/python27.dll"
        )
    ENDIF()
    
    # deployment
    SET(MOD_INSTALL_DIRECTORIES
        ${MOD_DIR}/scripts
    )
    SET(MOD_INSTALL_FILES
        ${MOD_DIR}/ext/python27/LICENSE
    )

ELSEIF(UNIX)
    FIND_PACKAGE(PythonLibs REQUIRED)
    IF(PYTHONLIBS_FOUND)
        MESSAGE(STATUS "  - Found Python library")
        SET(MOD_INCLUDE_DIRECTORIES ${PYTHON_INCLUDE_DIRS})
        SET(MOD_LIBRARIES ${PYTHON_LIBRARIES})
    ELSE()
        MESSAGE(FATAL_ERROR "Python library not found!")
    ENDIF()
ENDIF()


################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS PythonModule)

SET(MOD_CORE_SOURCES 
    ${MOD_DIR}/core/pythonscript.cpp
    ${MOD_DIR}/core/pyvoreen.cpp
)

SET(MOD_CORE_HEADERS
    ${MOD_DIR}/core/pythonscript.h
    ${MOD_DIR}/core/pyvoreen.h
)


################################################################################
# Qt module resources 
################################################################################
SET(MOD_QT_MODULECLASS PythonModuleQt)

SET(MOD_QT_SOURCES 
    ${MOD_DIR}/qt/pyvoreenqt.cpp
    ${MOD_DIR}/qt/pythonhighlighter.cpp
)

SET(MOD_QT_HEADERS
)

SET(MOD_QT_HEADERS_NONMOC
    ${MOD_DIR}/qt/pyvoreenqt.h
    ${MOD_DIR}/qt/pythonhighlighter.h
)


################################################################################
# VoreenVE module resources 
################################################################################
SET(MOD_VE_MODULECLASS PythonModuleVE)

SET(MOD_VE_SOURCES 
    ${MOD_DIR}/veplugin/pythoneditor.cpp
)

SET(MOD_VE_HEADERS
    ${MOD_DIR}/veplugin/pythoneditor.h
)

SET(MOD_VE_HEADERS_NONMOC
)

SET(MOD_VE_RESOURCES
    ${MOD_DIR}/veplugin/python.qrc
)
