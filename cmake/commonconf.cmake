
IF(NOT COMMONCONF_PROCESSED)

SET(VRN_HOME ${CMAKE_CURRENT_SOURCE_DIR})
MESSAGE(STATUS "Voreen Home: ${VRN_HOME}")

# set/create binary output path
IF(NOT VRN_BINARY_OUTPUT_DIR)
    SET(VRN_BINARY_OUTPUT_DIR ${VRN_HOME}/bin)
ENDIF()
IF(NOT EXISTS ${VRN_BINARY_OUTPUT_DIR})
    MESSAGE(STATUS "VRN_BINARY_OUTPUT_DIR does not exist: ${VRN_BINARY_OUTPUT_DIR}. Creating it ...")
    FILE(MAKE_DIRECTORY ${VRN_BINARY_OUTPUT_DIR})
    IF(NOT EXISTS ${VRN_BINARY_OUTPUT_DIR})
        MESSAGE(FATAL_ERROR "Failed to create VRN_BINARY_OUTPUT_DIR: ${VRN_BINARY_OUTPUT_DIR}")
    ENDIF()
ENDIF()
MESSAGE(STATUS "Output Path: ${VRN_BINARY_OUTPUT_DIR}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VRN_BINARY_OUTPUT_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${VRN_BINARY_OUTPUT_DIR})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${VRN_BINARY_OUTPUT_DIR})

# detect compiler and architecture
IF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008")
    SET(VRN_MSVC2008 TRUE)
    SET(VRN_MSVC TRUE)
    SET(VRN_WIN32 TRUE)
    MESSAGE(STATUS "Visual Studio 2008 build (32 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008 Win64")
    SET(VRN_MSVC2008 TRUE)
    SET(VRN_MSVC TRUE)
    SET(VRN_WIN64 TRUE)
    MESSAGE(STATUS "Visual Studio 2008 Build (64 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10")
    SET(VRN_MSVC2010 TRUE)
    SET(VRN_MSVC TRUE)
    SET(VRN_WIN32 TRUE)
    MESSAGE(STATUS "Visual Studio 2010 Build (32 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10 Win64")
    SET(VRN_MSVC2010 TRUE)
    SET(VRN_MSVC TRUE)
    SET(VRN_WIN64 TRUE)
    MESSAGE(STATUS "Visual Studio 2010 Build (64 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")
    SET(VRN_MSVC2012 TRUE)
    SET(VRN_MSVC TRUE)
    SET(VRN_WIN32 TRUE)
    MESSAGE("Visual Studio 2012 Build (32 Bit)")
ELSEIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 11 Win64")
    SET(VRN_MSVC2012 TRUE)
    SET(VRN_MSVC TRUE)
    SET(VRN_WIN64 TRUE)
    MESSAGE("Visual Studio 2012 Build (64 bit)")
ELSEIF(${CMAKE_GENERATOR} MATCHES "NMake") 
    SET(VRN_NMAKE TRUE)
    SET(VRN_MSVC TRUE)
    IF(CMAKE_CL_64)
        SET(VRN_WIN64 TRUE)
        MESSAGE(STATUS "NMake 64 Bit Build")
    ELSE(CMAKE_CL_64)
        SET(VRN_WIN32 TRUE)
        MESSAGE(STATUS "NMake 32 Bit Build")            
    ENDIF(CMAKE_CL_64)    
ELSEIF(${CMAKE_GENERATOR} MATCHES "MinGW")
    SET(VRN_MINGW TRUE)
    SET(VRN_WIN32 TRUE)
    MESSAGE("MinGW 32 Bit Build (not actively supported)")
ELSEIF(${CMAKE_GENERATOR} MATCHES "Unix")
    SET(VRN_UNIX TRUE)
    MESSAGE(STATUS "Unix Build")
ELSE()
    MESSAGE(WARNING "Unsupported or unknown generator: ${CMAKE_GENERATOR}")
ENDIF()

# include macros and config
INCLUDE(${VRN_HOME}/cmake/macros.cmake)
IF(EXISTS ${VRN_HOME}/config.cmake)
    MESSAGE(STATUS "Including custom configuration file 'config.cmake'")
    INCLUDE(${VRN_HOME}/config.cmake)
ELSE()
    INCLUDE(${VRN_HOME}/config-default.cmake)
ENDIF()

# common include directories
LIST(APPEND VRN_INCLUDE_DIRECTORIES "${VRN_HOME}" "${VRN_HOME}/include" "${VRN_HOME}/ext")
LIST(APPEND VRN_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}) 

# shared/static libs
IF(VRN_SHARED_LIBS)
    SET(BUILD_SHARED_LIBS TRUE)
    LIST(APPEND VRN_DEFINITIONS "-DVRN_SHARED_LIBS")
    MESSAGE(STATUS "Build Type: Shared Libraries")
ELSE()
    SET(BUILD_SHARED_LIBS FALSE)
    MESSAGE(STATUS "Build Type: Static Libraries")
ENDIF()

# print pch status
IF(VRN_PRECOMPILED_HEADER)
    MESSAGE(STATUS "Precompiled Headers: Enabled")
ELSE()
    MESSAGE(STATUS "Precompiled Headers: Disabled")
ENDIF()

# platform-dependent configuration
IF(VRN_MSVC)
    LIST(APPEND VRN_DEFINITIONS -DNOMINMAX -D_CRT_SECURE_NO_DEPRECATE)

    # Windows API dependencies
    LIST(APPEND VRN_EXTERNAL_LIBRARIES netapi32 version)

    # Disable warnings for Microsoft compiler:
    # C4305: 'identifier' : truncation from 'type1' to 'type2'
    # C4800: 'type' : forcing value to bool 'true' or 'false' (performance warning
    # C4290: C++ exception specification ignored except to indicate a function is
    #        not __declspec(nothrow)
    # C4068: unknown pragma
    # C4251  class needs to have dll interface (used for std classes)
    # C4355: 'this' : used in base member initializer list 
    #        occurs in processors' constructors when initializing event properties, 
    #        but is safe there, since the 'this' pointer is only stored and not accessed.
    # C4390: ';' : empty controlled statement found; is this the intent?
    #        occurs when OpenGL error logging macros are disabled
    LIST(APPEND VRN_DEFINITIONS /wd4305 /wd4800 /wd4290 /wd4068 /wd4251 /wd4355 /wd4390)
    
    # enable parallel builds in Visual Studio
    LIST(APPEND VRN_DEFINITIONS /MP)

    # prevent error: number of sections exceeded object file format limit
    LIST(APPEND VRN_DEFINITIONS /bigobj)
    
    # prevents rarely-used header files from being automatically included by windows.h (esp. winsock) 
    LIST(APPEND VRN_DEFINITIONS -DWIN32_LEAN_AND_MEAN)
    
    # allows 32 Bit builds to use more than 2GB RAM (VC++ only)
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE")
    SET(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")

    IF(VRN_SHARED_LIBS)
        # Linking against Windows DLLs requires explicit instantiation of templates
        LIST(APPEND VRN_DEFINITIONS -DDLL_TEMPLATE_INST)

        IF(NOT VRN_GENERATE_MANIFEST)
            # Do not embed manifest into binaries in debug mode (slows down incremental linking)
            SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /MANIFEST:NO")
            SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /MANIFEST:NO")
        ENDIF()
    ENDIF()

    # enable/disable incremental linking in debug builds
    If(VRN_INCREMENTAL_LINKING)
		IF(NOT VRN_PRECOMPILED_HEADER)
			MESSAGE("Incremental linking only available when using precompiled headers (VRN_PRECOMPILED_HEADER).")
			SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
			SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
    	ELSE()
			SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL")
			SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL")
		ENDIF()
	ELSE()
        SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
        SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
    ENDIF()
    
    # suppress ZERO_CHECK target
    IF(VRN_DISABLE_ZERO_CHECK)
        SET(CMAKE_SUPPRESS_REGENERATION TRUE)
    ENDIF()
    
    # Windows deployment   
    IF(VRN_DEPLOYMENT)
        LIST(APPEND VRN_DEFINITIONS "-DVRN_DEPLOYMENT") 
        MESSAGE("Windows deployment build:")

        MESSAGE("* Adding install target")
        SET(VRN_ADD_INSTALL_TARGET TRUE)
        MESSAGE("* Install directory (CMAKE_INSTALL_PREFIX): ${CMAKE_INSTALL_PREFIX}")
        
        IF(NOT ${VRN_BINARY_OUTPUT_DIR} STREQUAL ${VRN_HOME}/bin)
            MESSAGE("* Overriding binary output directory: ${VRN_HOME}/bin")
            SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VRN_HOME}/bin)
            SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${VRN_HOME}/bin)
            SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${VRN_HOME}/bin)
        ENDIF()

        MESSAGE("* Adding Visual Studio redist libraries to install target")
        IF(VRN_MSVC2010)
            GET_FILENAME_COMPONENT(VS_DIR [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\10.0\\Setup\\VS;ProductDir] REALPATH CACHE)
            IF(NOT EXISTS ${VS_DIR})
                MESSAGE(WARNING "Visual Studio directory not found: ${VS_DIR}")
            ELSE()
                IF(VRN_WIN32)
                    SET(machine_type "x86")
                ELSEIF(VRN_WIN64)
                    SET(machine_type "x64")
                ENDIF()
                MESSAGE("  - ${VS_DIR}/VC/redist/${machine_type}/Microsoft.VC100.CRT/msvcp100.dll")
                MESSAGE("  - ${VS_DIR}/VC/redist/${machine_type}/Microsoft.VC100.CRT/msvcr100.dll")
                MESSAGE("  - ${VS_DIR}/VC/redist/${machine_type}/Microsoft.VC100.OpenMP/vcomp100.dll")
                INSTALL(FILES "${VS_DIR}/VC/redist/${machine_type}/Microsoft.VC100.CRT/msvcp100.dll" DESTINATION .)
                INSTALL(FILES "${VS_DIR}/VC/redist/${machine_type}/Microsoft.VC100.CRT/msvcr100.dll" DESTINATION .)
                INSTALL(FILES "${VS_DIR}/VC/redist/${machine_type}/Microsoft.VC100.OpenMP/vcomp100.dll" DESTINATION .)
            ENDIF()
        ELSEIF(VRN_MSVC2012)
            GET_FILENAME_COMPONENT(VS_DIR [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\11.0\\Setup\\VS;ProductDir] REALPATH CACHE)
            IF(NOT EXISTS ${VS_DIR})
                MESSAGE(WARNING "Visual Studio directory not found: ${VS_DIR}")
            ELSE()
                IF(VRN_WIN32)
                    SET(machine_type "x86")
                ELSEIF(VRN_WIN64)
                    SET(machine_type "x64")
                ENDIF()
                MESSAGE("  - ${VS_DIR}/VC/redist/${machine_type}/Microsoft.VC110.CRT/msvcp110.dll")
                MESSAGE("  - ${VS_DIR}/VC/redist/${machine_type}/Microsoft.VC110.CRT/msvcr110.dll")
                MESSAGE("  - ${VS_DIR}/VC/redist/${machine_type}/Microsoft.VC110.OpenMP/vcomp110.dll")
                INSTALL(FILES "${VS_DIR}/VC/redist/${machine_type}/Microsoft.VC110.CRT/msvcp110.dll" DESTINATION .)
                INSTALL(FILES "${VS_DIR}/VC/redist/${machine_type}/Microsoft.VC110.CRT/msvcr110.dll" DESTINATION .)
                INSTALL(FILES "${VS_DIR}/VC/redist/${machine_type}/Microsoft.VC110.OpenMP/vcomp110.dll" DESTINATION .)
            ENDIF()
        ELSE()
            MESSAGE(WARNING "Deploying redist libraries only supported for Visual Studio 2010 or 2012")
        ENDIF()
    ELSE(VRN_DEPLOYMENT)
        # hardcode Voreen base path, if binary output dir has been modified and we are not in deployment mode
        IF(NOT ${VRN_BINARY_OUTPUT_DIR} STREQUAL "${VRN_HOME}/bin")
            LIST(APPEND VRN_DEFINITIONS "-DVRN_BASE_PATH=\"${VRN_HOME}\"")
        ENDIF()
    ENDIF(VRN_DEPLOYMENT)

ELSEIF(UNIX)

    LIST(APPEND VRN_DEFINITIONS "-DUNIX")  
    LIST(APPEND VRN_DEFINITIONS "-D__STDC_CONSTANT_MACROS")  
    
    IF(VRN_DEPLOYMENT)
        MESSAGE("Unix deployment build")        
        LIST(APPEND VRN_DEFINITIONS "-DVRN_DEPLOYMENT") 
    ENDIF()

    # hardcode Voreen base path, if binary output dir has been modified
    IF(NOT ${VRN_BINARY_OUTPUT_DIR} STREQUAL "${VRN_HOME}/bin")
        LIST(APPEND VRN_DEFINITIONS "-DVRN_BASE_PATH=\"${VRN_HOME}\"")
    ENDIF()

ELSEIF(VRN_MINGW)
    # hardcode Voreen base path, if binary output dir has been modified
    IF(NOT ${VRN_BINARY_OUTPUT_DIR} STREQUAL "${VRN_HOME}/bin")
        LIST(APPEND VRN_DEFINITIONS "-DVRN_BASE_PATH=\"${VRN_HOME}\"")
    ENDIF()
ENDIF()

# macosx
IF(APPLE)
    FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation )
    LIST(APPEND VRN_EXTERNAL_LIBRARIES ${COREFOUNDATION_LIBRARY})
ENDIF(APPLE)

# use STL in tinyXML
LIST(APPEND VRN_DEFINITIONS "-DTIXML_USE_STL") 

# tgt configuration
LIST(APPEND VRN_DEFINITIONS "-DTGT_WITHOUT_DEFINES") # don't use tgt's build system
IF(VRN_MSVC)
    SET(TGT_WITH_WMI TRUE)  #< enable Windows Management Instrumentation for hardware detection
ENDIF()

# set Voreen debug flags for debug builds
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTGT_DEBUG -DVRN_DEBUG")
 
# minimum Qt version
SET(VRN_REQUIRED_QT_VERSION "4.6")


# detect libraries
MESSAGE(STATUS "--------------------------------------------------------------------------------")
MESSAGE(STATUS "Detecting Common Mandatory Libraries:")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${VRN_HOME}/cmake")

# OpenGL
FIND_PACKAGE(OpenGL REQUIRED)
IF(OPENGL_FOUND)
    MESSAGE(STATUS "* Found OpenGL")
    LIST(APPEND VRN_INCLUDE_DIRECTORIES ${OPENGL_INCLUDE_DIR})
    LIST(APPEND VRN_EXTERNAL_LIBRARIES ${OPENGL_LIBRARIES})
ELSE(OPENGL_FOUND)
    MESSAGE(FATAL_ERROR "OpenGL not found!")
ENDIF(OPENGL_FOUND)
    
# GLEW
FIND_PACKAGE(GlewVRN REQUIRED)
IF(GLEW_FOUND)
    MESSAGE(STATUS "* Found GLEW")
    LIST(APPEND VRN_DEFINITIONS ${GLEW_DEFINITIONS})
    LIST(APPEND VRN_INCLUDE_DIRECTORIES ${GLEW_INCLUDE_DIR})
    LIST(APPEND VRN_EXTERNAL_LIBRARIES ${GLEW_LIBRARY})
    LIST(APPEND VRN_EXTERNAL_DEBUG_DLLS ${GLEW_DLL_DEBUG})
    LIST(APPEND VRN_EXTERNAL_RELEASE_DLLS ${GLEW_DLL_RELEASE})
    LIST(APPEND VRN_EXTERNAL_LICENSE_FILES ${GLEW_LICENSE_FILE})
ELSE(GLEW_FOUND)
    MESSAGE(FATAL_ERROR "GLEW not found!")
ENDIF(GLEW_FOUND)

# Boost	
FIND_PACKAGE(BoostVRN REQUIRED)
IF(Boost_FOUND)
    MESSAGE(STATUS "* Found Boost")
    LIST(APPEND VRN_DEFINITIONS ${Boost_DEFINITIONS})
    LIST(APPEND VRN_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
    LIST(APPEND VRN_EXTERNAL_LIBRARIES ${Boost_LIBRARIES})
    LIST(APPEND VRN_EXTERNAL_DEBUG_DLLS ${Boost_DEBUG_DLLS})
    LIST(APPEND VRN_EXTERNAL_RELEASE_DLLS ${Boost_RELEASE_DLLS})
ELSE()
    MESSAGE(FATAL_ERROR "Boost not found!")
ENDIF()

# tinyxml
LIST(APPEND VRN_EXTERNAL_LICENSE_FILES "${VRN_HOME}/ext/tinyxml/license.txt")
    
# eigen
MESSAGE(STATUS "* Found Eigen")
LIST(APPEND VRN_INCLUDE_DIRECTORIES "${VRN_HOME}/ext/eigen")
LIST(APPEND VRN_DEFINITIONS -DEIGEN_PERMANENTLY_DISABLE_STUPID_WARNINGS)


#######################
# modules
#######################
MESSAGE(STATUS "--------------------------------------------------------------------------------")
    
# collect module dirs
SET(MODULE_BASEDIR_LIST ${VRN_HOME}/modules) #< framework modules
IF(VRN_CUSTOM_MODULEDIR)
    IF(EXISTS ${VRN_CUSTOM_MODULEDIR})
        LIST(APPEND MODULE_BASEDIR_LIST ${VRN_CUSTOM_MODULEDIR})
    ELSE()
        MESSAGE(WARNING "Custom module dir ${VRN_CUSTOM_MODULEDIR} does not exist!")
    ENDIF()
ENDIF()
FOREACH(num RANGE 0 10)
    IF(VRN_CUSTOM_MODULEDIR_${num})
        IF(EXISTS ${VRN_CUSTOM_MODULEDIR_${num}})
            LIST(APPEND MODULE_BASEDIR_LIST ${VRN_CUSTOM_MODULEDIR_${num}})
        ELSE()
            MESSAGE(WARNING "Custom module dir does not exist: ${VRN_CUSTOM_MODULEDIR_${num}}")
        ENDIF()
    ENDIF()
ENDFOREACH()
LIST(REMOVE_DUPLICATES MODULE_BASEDIR_LIST)

# include modules
SET(VRN_MODULE_CORE ON CACHE BOOL "Core module is always included" FORCE)
MARK_AS_ADVANCED(VRN_MODULE_CORE)
FOREACH(module_basedir ${MODULE_BASEDIR_LIST})

    MESSAGE(STATUS "Including Voreen Modules from ${module_basedir}:")

    IF(EXISTS ${module_basedir}/modulelist.cmake)
        INCLUDE(${module_basedir}/modulelist.cmake)
    ENDIF()
    
    # iterate over subdirectories of module dir and include each enabled module
    LIST_SUBDIRECTORIES(module_dir_list ${module_basedir} false)
    FOREACH(module_dir ${module_dir_list})
        SET(module_file ${module_basedir}/${module_dir}/${module_dir}.cmake)
        STRING(TOLOWER ${module_dir} module_lower)
        STRING(TOUPPER ${module_dir} module_upper)
        IF(EXISTS ${module_file})
            IF(VRN_MODULE_${module_upper})
                FILE(RELATIVE_PATH module_file_rel ${module_basedir} ${module_file})
                MESSAGE(STATUS "* ${module_file_rel}")
                
                # add module availability macro
                LIST(APPEND VRN_MODULE_DEFINITIONS "-DVRN_MODULE_${module_upper}")
                
                # include module .cmake file
                SET(MOD_DIR ${module_basedir}/${module_dir})
                INCLUDE(${module_file})
                
                # add module definitions
                LIST(APPEND VRN_MODULE_DEFINITIONS          ${MOD_DEFINITIONS})
                
                # add external dependencies
                LIST(APPEND VRN_MODULE_INCLUDE_DIRECTORIES  ${MOD_INCLUDE_DIRECTORIES})
                LIST(APPEND VRN_EXTERNAL_LIBRARIES          ${MOD_LIBRARIES})
                FOREACH(lib ${MOD_DEBUG_LIBRARIES})
                    LIST(APPEND VRN_EXTERNAL_LIBRARIES      debug ${lib})
                ENDFOREACH()
                FOREACH(lib ${MOD_RELEASE_LIBRARIES})
                    LIST(APPEND VRN_EXTERNAL_LIBRARIES      optimized ${lib})
                ENDFOREACH()
                LIST(APPEND VRN_EXTERNAL_DEBUG_DLLS         ${MOD_DEBUG_DLLS})
                LIST(APPEND VRN_EXTERNAL_RELEASE_DLLS       ${MOD_RELEASE_DLLS})
                
                # add install resources
                LIST(APPEND VRN_MODULE_INSTALL_DIRECTORIES  ${MOD_INSTALL_DIRECTORIES})
                LIST(APPEND VRN_MODULE_INSTALL_FILES        ${MOD_INSTALL_FILES})
                
                # add core resources
                IF(MOD_CORE_MODULECLASS)
                    LIST(APPEND VRN_MODULE_CORE_MODULECLASSES ${MOD_CORE_MODULECLASS})
                    STRING(TOLOWER ${MOD_CORE_MODULECLASS} moduleclass_lower)
                    LIST(APPEND VRN_MODULE_CORE_MODULECLASSES_INCLUDES "${module_basedir}/${module_dir}/${moduleclass_lower}.h")
                    LIST(APPEND VRN_MODULE_CORE_SOURCES "${module_basedir}/${module_dir}/${moduleclass_lower}.cpp")
                    LIST(APPEND VRN_MODULE_CORE_HEADERS "${module_basedir}/${module_dir}/${moduleclass_lower}.h")
                ENDIF()
                LIST(APPEND VRN_MODULE_CORE_SOURCES         ${MOD_CORE_SOURCES})
                LIST(APPEND VRN_MODULE_CORE_HEADERS         ${MOD_CORE_HEADERS})
                LIST(APPEND VRN_MODULE_CORE_APPLICATIONS    ${MOD_CORE_APPLICATIONS})

                # add qt resources
                IF(MOD_QT_MODULECLASS)
                    LIST(APPEND VRN_MODULE_QT_MODULECLASSES ${MOD_QT_MODULECLASS})
                    STRING(TOLOWER ${MOD_QT_MODULECLASS} moduleclass_lower)
                    LIST(APPEND VRN_MODULE_QT_MODULECLASSES_INCLUDES "${module_basedir}/${module_dir}/${moduleclass_lower}.h")
                    LIST(APPEND VRN_MODULE_QT_SOURCES        "${module_basedir}/${module_dir}/${moduleclass_lower}.cpp")
                    LIST(APPEND VRN_MODULE_QT_HEADERS_NONMOC "${module_basedir}/${module_dir}/${moduleclass_lower}.h")
                ENDIF()
                LIST(APPEND VRN_MODULE_QT_SOURCES           ${MOD_QT_SOURCES})
                LIST(APPEND VRN_MODULE_QT_HEADERS           ${MOD_QT_HEADERS})
                LIST(APPEND VRN_MODULE_QT_HEADERS_NONMOC    ${MOD_QT_HEADERS_NONMOC})
                LIST(APPEND VRN_MODULE_QT_FORMS_HEADERS     ${MOD_QT_FORMS_HEADERS})
                LIST(APPEND VRN_MODULE_QT_APPLICATIONS      ${MOD_QT_APPLICATIONS})
                        
                # add ve resources
                IF(MOD_VE_MODULECLASS)
                    LIST(APPEND VRN_MODULE_VE_MODULECLASSES ${MOD_VE_MODULECLASS})
                    STRING(TOLOWER ${MOD_VE_MODULECLASS} moduleclass_lower)
                    LIST(APPEND VRN_MODULE_VE_MODULECLASSES_INCLUDES "${module_basedir}/${module_dir}/${moduleclass_lower}.h")
                    LIST(APPEND VRN_MODULE_VE_SOURCES        "${module_basedir}/${module_dir}/${moduleclass_lower}.cpp")
                    LIST(APPEND VRN_MODULE_VE_HEADERS_NONMOC "${module_basedir}/${module_dir}/${moduleclass_lower}.h")
                ENDIF()
                LIST(APPEND VRN_MODULE_VE_SOURCES           ${MOD_VE_SOURCES})
                LIST(APPEND VRN_MODULE_VE_HEADERS           ${MOD_VE_HEADERS})
                LIST(APPEND VRN_MODULE_VE_HEADERS_NONMOC    ${MOD_VE_HEADERS_NONMOC})
                LIST(APPEND VRN_MODULE_VE_RESOURCES         ${MOD_VE_RESOURCES})
                
                UNSET(MOD_DIR)
                UNSET(MOD_DEFINITIONS)
                UNSET(MOD_INCLUDE_DIRECTORIES)
                UNSET(MOD_LIBRARIES)
                UNSET(MOD_DEBUG_LIBRARIES)
                UNSET(MOD_RELEASE_LIBRARIES)
                UNSET(MOD_DEBUG_DLLS)
                UNSET(MOD_RELEASE_DLLS)
                UNSET(MOD_INSTALL_DIRECTORIES)
                UNSET(MOD_INSTALL_FILES)
                
                UNSET(MOD_CORE_MODULECLASS)
                UNSET(MOD_CORE_SOURCES)
                UNSET(MOD_CORE_HEADERS)
                UNSET(MOD_CORE_APPLICATIONS)
                
                UNSET(MOD_QT_MODULECLASS)
                UNSET(MOD_QT_SOURCES)
                UNSET(MOD_QT_HEADERS)
                UNSET(MOD_QT_HEADERS_NONMOC)
                UNSET(MOD_QT_FORMS_HEADERS)
                UNSET(MOD_QT_APPLICATIONS)
                
                UNSET(MOD_VE_MODULECLASS)
                UNSET(MOD_VE_SOURCES)
                UNSET(MOD_VE_HEADERS)
                UNSET(MOD_VE_HEADERS_NONMOC)
                UNSET(MOD_VE_RESOURCES)

            ELSEIF(NOT DEFINED VRN_MODULE_${module_upper})
                # add missing module include option
                SET(VRN_MODULE_${module_upper} OFF CACHE BOOL "Include module \"${module_lower}\"?")
            ENDIF(VRN_MODULE_${module_upper})
        ENDIF(EXISTS ${module_file})
    ENDFOREACH(module_dir ${module_dir_list})
    
ENDFOREACH(module_basedir ${MODULE_BASEDIR_LIST})

MESSAGE(STATUS "--------------------------------------------------------------------------------")

# define framework and module install files (note: DLLs are installed by CMakeLists.txt in root dir)
IF (VRN_ADD_INSTALL_TARGET)
    # framework install files
    INCLUDE(${VRN_HOME}/cmake/install.cmake)
    
    # module install directories
    FOREACH(install_dir ${VRN_MODULE_INSTALL_DIRECTORIES})
        FILE(RELATIVE_PATH install_dir_rel ${VRN_HOME} ${install_dir})
        INSTALL(DIRECTORY ${install_dir}/ DESTINATION ${install_dir_rel})
    ENDFOREACH()
    
    # module install files
    FOREACH(install_file ${VRN_MODULE_INSTALL_FILES})
        FILE(RELATIVE_PATH install_file_rel ${VRN_HOME} ${install_file})
        GET_FILENAME_COMPONENT(install_path_rel ${install_file_rel} PATH)
        INSTALL(FILES ${install_file} DESTINATION ${install_path_rel})
    ENDFOREACH()
        
    # license files
    FOREACH(install_file ${VRN_EXTERNAL_LICENSE_FILES})
        FILE(RELATIVE_PATH install_file_rel ${VRN_HOME} ${install_file})
        GET_FILENAME_COMPONENT(install_path_rel ${install_file_rel} PATH)
        INSTALL(FILES ${install_file} DESTINATION ${install_path_rel})
    ENDFOREACH()
ENDIF()

SET(COMMONCONF_PROCESSED TRUE)
ENDIF(NOT COMMONCONF_PROCESSED)
