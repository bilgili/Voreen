################################################################################
# Project file for the VoreenVE Application
################################################################################
PROJECT(voreenve)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring VoreenVE Application")

# sources
SET(VRN_VE_SOURCES
    main.cpp
    voreenmainwindow.cpp
    voreenmoduleve.cpp
    voreenveapplication.cpp    
    voreenveplugin.cpp
    voreenvisualization.cpp    
)

# headers
SET(VRN_VE_HEADERS
    voreenmainwindow.h
    voreenveapplication.h
    voreenvisualization.h
)

# headers that do not need to be run through moc
SET(VRN_VE_HEADERS_NONMOC
    voreenmoduleve.h
    voreenveplugin.h
)

# add module sources and headers
LIST(APPEND VRN_VE_SOURCES ${VRN_MODULE_VE_SOURCES})
LIST(APPEND VRN_VE_HEADERS ${VRN_MODULE_VE_HEADERS})
LIST(APPEND VRN_VE_HEADERS_NONMOC ${VRN_MODULE_VE_HEADERS_NONMOC})

# find Qt library
FIND_PACKAGE(Qt4 ${VRN_REQUIRED_QT_VERSION} REQUIRED QtCore QtGui QtOpenGL)


################################################################################
# generate module registration header
################################################################################
SET(REGISTRATION_FILE "${CMAKE_BINARY_DIR}/gen_moduleregistration_ve.h" )
MESSAGE(STATUS "* Generating module registration header ${REGISTRATION_FILE}")

SET(REGISTRATION_SOURCE "// WARNING: This file is generated and will be overwritten on next cmake run!\n" )
LIST(APPEND REGISTRATION_SOURCE "#include \"apps/voreenve/voreenveapplication.h\"\n\n" )
LIST(APPEND REGISTRATION_SOURCE "// include VoreenVE module class headers\n" )
FOREACH(inc ${VRN_MODULE_VE_MODULECLASSES_INCLUDES})
    LIST(APPEND REGISTRATION_SOURCE "#include \"${inc}\"\n" )
ENDFOREACH()
LIST(APPEND REGISTRATION_SOURCE "\nnamespace voreen {\n" )
LIST(APPEND REGISTRATION_SOURCE "\n// instantiate VoreenVE module classes\n" )
LIST(APPEND REGISTRATION_SOURCE "void registerAllVEModules(VoreenVEApplication* vappVE) {\n" )
IF(VRN_MODULE_VE_MODULECLASSES)
    LIST(APPEND REGISTRATION_SOURCE "    VoreenModuleVE* module\;\n\n")
    LIST(LENGTH VRN_MODULE_VE_MODULECLASSES num_modules)
    MATH(EXPR max_index "${num_modules} - 1")
    FOREACH(i RANGE ${max_index})
        LIST(GET VRN_MODULE_VE_MODULECLASSES ${i} class)
        LIST(GET VRN_MODULE_VE_MODULECLASSES_INCLUDES ${i} inc)
        FILE(RELATIVE_PATH inc_rel ${VRN_HOME} ${inc})
        GET_FILENAME_COMPONENT(mod_path_rel ${inc_rel} PATH)
        LIST(APPEND REGISTRATION_SOURCE "    module = new ${class}(\"${mod_path_rel}\")\;\n")
        LIST(APPEND REGISTRATION_SOURCE "    vappVE->registerVEModule(module)\;\n\n")
    ENDFOREACH()
ENDIF()
LIST(APPEND REGISTRATION_SOURCE "}\n\n" )
LIST(APPEND REGISTRATION_SOURCE "} // namespace\n" )

FILE(WRITE ${REGISTRATION_FILE} ${REGISTRATION_SOURCE})
LIST(APPEND VRN_VE_HEADERS_NONMOC ${REGISTRATION_FILE})


################################################################################
# define executable target
################################################################################

# Qt moc
QT4_WRAP_CPP(VRN_VE_MOC_SOURCES ${VRN_VE_HEADERS})
LIST(APPEND VRN_VE_SOURCES ${VRN_VE_MOC_SOURCES})

# Qt resources
QT4_ADD_RESOURCES(VRN_VE_RESOURCES_RCC "${VRN_HOME}/apps/voreenve/resource/voreenve.qrc" ${VRN_MODULE_VE_RESOURCES})

# add msvc pch
IF (VRN_PRECOMPILED_HEADER AND MSVC)
    ADD_MSVC_PRECOMPILED_HEADER("pch_ve.h" "pch_ve.cpp" VRN_VE_SOURCES)
ENDIF()

# define target
IF(VRN_DEPLOYMENT)
    MESSAGE("* Linking against Windows subsystem (deployment build)")
    
    # WIN32 symbol causes the executable to link against the Windows subsystem (thus not showing console window)
    ADD_EXECUTABLE(voreenve WIN32
        ${VRN_VE_SOURCES} ${VRN_VE_HEADERS} ${VRN_VE_HEADERS_NONMOC} 
        ${VRN_VE_RESOURCES_RCC} 
        "${VRN_HOME}/resource/icons/winicon.rc") 

    # explicitly define application entry point to prevent linker error when using Windows subsystem 
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup") 
ELSE()
    ADD_EXECUTABLE(voreenve
        ${VRN_VE_SOURCES} ${VRN_VE_HEADERS} ${VRN_VE_HEADERS_NONMOC} 
        ${VRN_VE_RESOURCES_RCC} 
        "${VRN_HOME}/resource/icons/winicon.rc") 
ENDIF()
ADD_DEFINITIONS(${VRN_DEFINITIONS} ${VRN_MODULE_DEFINITIONS})
INCLUDE_DIRECTORIES(${VRN_INCLUDE_DIRECTORIES} ${VRN_MODULE_INCLUDE_DIRECTORIES})
TARGET_LINK_LIBRARIES(voreenve tgt voreen_core voreen_qt ${VRN_EXTERNAL_LIBRARIES})

# group source files by subdir
IF(VRN_GROUP_SOURCE_FILES)
    LIST(APPEND RemovePathPrefixes "")
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(VRN_VE_SOURCES ${VRN_HOME} RemovePathPrefixes)
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(VRN_VE_HEADERS ${VRN_HOME} RemovePathPrefixes)
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(VRN_VE_HEADERS_NONMOC ${VRN_HOME} RemovePathPrefixes)
ENDIF()

# add Qt dependency
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}) #< location of generated ui headers in out-of-source builds
ADD_DEFINITIONS(${QT_DEFINITIONS})
TARGET_LINK_LIBRARIES(voreenve ${QT_LIBRARIES})

# add gcc pch
IF (VRN_PRECOMPILED_HEADER AND CMAKE_COMPILER_IS_GNUCXX)
    ADD_GCC_PRECOMPILED_HEADER(voreenve pch_ve.h)
ENDIF()


################################################################################
# deployment
################################################################################
IF(VRN_ADD_INSTALL_TARGET)
    INSTALL(TARGETS voreenve
        RUNTIME DESTINATION .
    )
ENDIF()    
