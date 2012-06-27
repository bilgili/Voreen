# Try to find Win32 Qt DLLs to be copied to the bin directory. Once done this will define:
# QT_DLLS_FOUND
# QT_DEBUG_DLLS
# QT_RELEASE_DLLS
#
# Note: FIND_PACKAGE(Qt4 ...) has to be called before in order for ${QT_LIBRARY_DIR} 
# and ${QT_BINARY_DIR} to be defined!

# DLLs are either located in Qt library or binary directory
IF(EXISTS ${QT_LIBRARY_DIR}/QtCored4.dll)
    SET(DLL_DIR ${QT_LIBRARY_DIR})
ELSEIF(EXISTS ${QT_BINARY_DIR}/QtCored4.dll)
    SET(DLL_DIR ${QT_BINARY_DIR})
ELSEIF(EXISTS ${QT_LIBRARY_DIR}/QtCore4.dll)
    SET(DLL_DIR ${QT_LIBRARY_DIR})
ELSEIF(EXISTS ${QT_BINARY_DIR}/QtCore4.dll)
    SET(DLL_DIR ${QT_BINARY_DIR})
ENDIF()

IF(DLL_DIR)
    SET(QT_DLLS_FOUND TRUE)
    
    FOREACH(component ${Qt4DLLsVRN_FIND_COMPONENTS})
        IF(EXISTS ${DLL_DIR}/${component}d4.dll)
            LIST(APPEND QT_DEBUG_DLLS ${DLL_DIR}/${component}d4.dll)
        ELSE()
            MESSAGE("Failed to find Qt Debug DLL: ${component}d4.dll")
            SET(QT_DLLS_FOUND FALSE)
        ENDIF()
        
        IF(EXISTS ${DLL_DIR}/${component}4.dll)
            LIST(APPEND QT_RELEASE_DLLS ${DLL_DIR}/${component}4.dll)
        ELSE()
            MESSAGE(WARNING "Failed to find Qt Release DLL: ${component}4.dll")
            SET(QT_DLLS_FOUND FALSE)
        ENDIF()
    ENDFOREACH()
ELSE()
    SET(QT_DLLS_FOUND FALSE)
ENDIF()

UNSET(Qt4DLLsVRN_DIR)
MARK_AS_ADVANCED(Qt4DLLsVRN_DIR)
