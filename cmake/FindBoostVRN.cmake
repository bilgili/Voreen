# Try to find Boost library and include path. Once done this will define:
# Boost_FOUND
# Boost_DEFINITIONS
# Boost_INCLUDE_DIRS
# Boost_LIBRARIES (containing both debug and release libraries on win32)
# win32: Boost_DEBUG_LIBRARIES, Boost_RELEASE_LIBRARIES, Boost_DEBUG_DLLS, Boost_RELEASE_DLLS

IF (WIN32)
    SET(Boost_DIR "${VRN_HOME}/ext/boost" CACHE PATH "If boost is not found, set this path")
    
    SET(Boost_DEFINITIONS "-DBOOST_ALL_NO_LIB")

    SET(Boost_INCLUDE_DIRS "${Boost_DIR}/include")

    # set debug and release libraries
    IF(VRN_WIN32)
        IF(VRN_MSVC2008)
            SET(Boost_LIB_DIR "${Boost_DIR}/lib/vs2008")
        ELSEIF(VRN_MSVC2010)
            SET(Boost_LIB_DIR "${Boost_DIR}/lib/vs2010/win32")
        ELSEIF(VRN_MSVC2012)
            SET(Boost_LIB_DIR "${Boost_DIR}/lib/vs2012/win32")
        ELSE()
            MESSAGE(FATAL_ERROR "Unknown 32Bit Windows compiler!")
        ENDIF()
    ELSEIF(VRN_WIN64)
        IF(VRN_MSVC2010)
            SET(Boost_LIB_DIR "${Boost_DIR}/lib/vs2010/win64")
        ELSEIF(VRN_MSVC2012)
            SET(Boost_LIB_DIR "${Boost_DIR}/lib/vs2012/win64")
        ELSE()
            MESSAGE(FATAL_ERROR "Unknown 64Bit Windows compiler!")
        ENDIF()
    ELSE()
        MESSAGE(FATAL_ERROR "Neither VRN_WIN32 nor VRN_WIN64 defined!")
    ENDIF()

    LIST(APPEND Boost_DEBUG_LIBRARIES   "${Boost_LIB_DIR}/debug/boost_date_time.lib")
    LIST(APPEND Boost_DEBUG_DLLS        "${Boost_LIB_DIR}/debug/boost_date_time.dll")
    LIST(APPEND Boost_RELEASE_LIBRARIES "${Boost_LIB_DIR}/release/boost_date_time.lib")
    LIST(APPEND Boost_RELEASE_DLLS      "${Boost_LIB_DIR}/release/boost_date_time.dll")
    
    LIST(APPEND Boost_DEBUG_LIBRARIES   "${Boost_LIB_DIR}/debug/boost_prg_exec_monitor.lib")
    LIST(APPEND Boost_DEBUG_DLLS        "${Boost_LIB_DIR}/debug/boost_prg_exec_monitor.dll")
    LIST(APPEND Boost_RELEASE_LIBRARIES "${Boost_LIB_DIR}/release/boost_prg_exec_monitor.lib")
    LIST(APPEND Boost_RELEASE_DLLS      "${Boost_LIB_DIR}/release/boost_prg_exec_monitor.dll")

    LIST(APPEND Boost_DEBUG_LIBRARIES   "${Boost_LIB_DIR}/debug/boost_program_options.lib")
    LIST(APPEND Boost_DEBUG_DLLS        "${Boost_LIB_DIR}/debug/boost_program_options.dll")
    LIST(APPEND Boost_RELEASE_LIBRARIES "${Boost_LIB_DIR}/release/boost_program_options.lib")
    LIST(APPEND Boost_RELEASE_DLLS      "${Boost_LIB_DIR}/release/boost_program_options.dll")
    
    LIST(APPEND Boost_DEBUG_LIBRARIES   "${Boost_LIB_DIR}/debug/boost_regex.lib")
    LIST(APPEND Boost_DEBUG_DLLS        "${Boost_LIB_DIR}/debug/boost_regex.dll")
    LIST(APPEND Boost_RELEASE_LIBRARIES "${Boost_LIB_DIR}/release/boost_regex.lib")
    LIST(APPEND Boost_RELEASE_DLLS      "${Boost_LIB_DIR}/release/boost_regex.dll")

    LIST(APPEND Boost_DEBUG_LIBRARIES   "${Boost_LIB_DIR}/debug/boost_serialization.lib")
    LIST(APPEND Boost_DEBUG_DLLS        "${Boost_LIB_DIR}/debug/boost_serialization.dll")
    LIST(APPEND Boost_RELEASE_LIBRARIES "${Boost_LIB_DIR}/release/boost_serialization.lib")
    LIST(APPEND Boost_RELEASE_DLLS      "${Boost_LIB_DIR}/release/boost_serialization.dll")
    
    LIST(APPEND Boost_DEBUG_LIBRARIES   "${Boost_LIB_DIR}/debug/boost_system.lib")
    LIST(APPEND Boost_DEBUG_DLLS        "${Boost_LIB_DIR}/debug/boost_system.dll")
    LIST(APPEND Boost_RELEASE_LIBRARIES "${Boost_LIB_DIR}/release/boost_system.lib") 
    LIST(APPEND Boost_RELEASE_DLLS      "${Boost_LIB_DIR}/release/boost_system.dll") 

    LIST(APPEND Boost_DEBUG_LIBRARIES   "${Boost_LIB_DIR}/debug/boost_thread.lib")
    LIST(APPEND Boost_DEBUG_DLLS        "${Boost_LIB_DIR}/debug/boost_thread.dll")
    LIST(APPEND Boost_RELEASE_LIBRARIES "${Boost_LIB_DIR}/release/boost_thread.lib")    
    LIST(APPEND Boost_RELEASE_DLLS      "${Boost_LIB_DIR}/release/boost_thread.dll")
    
    LIST(APPEND Boost_DEBUG_LIBRARIES   "${Boost_LIB_DIR}/debug/boost_unit_test_framework.lib")
    LIST(APPEND Boost_DEBUG_DLLS        "${Boost_LIB_DIR}/debug/boost_unit_test_framework.dll")
    LIST(APPEND Boost_RELEASE_LIBRARIES "${Boost_LIB_DIR}/release/boost_unit_test_framework.lib")
    LIST(APPEND Boost_RELEASE_DLLS      "${Boost_LIB_DIR}/release/boost_unit_test_framework.dll")

    LIST(APPEND Boost_DEBUG_LIBRARIES   "${Boost_LIB_DIR}/debug/boost_wserialization.lib")
    LIST(APPEND Boost_DEBUG_DLLS        "${Boost_LIB_DIR}/debug/boost_wserialization.dll")
    LIST(APPEND Boost_RELEASE_LIBRARIES "${Boost_LIB_DIR}/release/boost_wserialization.lib")
    LIST(APPEND Boost_RELEASE_DLLS      "${Boost_LIB_DIR}/release/boost_wserialization.dll")
    
    LIST(APPEND Boost_DEBUG_LIBRARIES   "${Boost_LIB_DIR}/debug/libboost_exception.lib")
    LIST(APPEND Boost_RELEASE_LIBRARIES "${Boost_LIB_DIR}/release/libboost_exception.lib") 
    
    FOREACH(lib ${Boost_DEBUG_LIBRARIES})
        LIST(APPEND Boost_LIBRARIES debug ${lib})
    ENDFOREACH()
    FOREACH(lib ${Boost_RELEASE_LIBRARIES})
        LIST(APPEND Boost_LIBRARIES optimized ${lib})
    ENDFOREACH()
   
    IF(Boost_INCLUDE_DIRS AND Boost_LIBRARIES)
        SET(Boost_FOUND TRUE)
    ELSE()
        SET(Boost_FOUND FALSE)
    ENDIF()

ELSE(WIN32)
    FIND_PACKAGE(Boost 1.40.0 REQUIRED date_time prg_exec_monitor program_options regex thread unit_test_framework system)
ENDIF(WIN32)

MARK_AS_ADVANCED(Boost_DIR Boost_INCLUDE_DIRS)
