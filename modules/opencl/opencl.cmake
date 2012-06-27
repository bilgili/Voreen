
################################################################################
# External dependency: OpenCL SDK
################################################################################

IF(WIN32)
    FIND_PATH( 
        NVIDIA_GPU_COMPUTING_SDK 
        NAMES CL/cl.h OpenCL/common/inc/CL/cl.h
        DOC "Path to the Nvidia GPU Computing SDK"
    )
    IF(NOT EXISTS ${NVIDIA_GPU_COMPUTING_SDK})
        MESSAGE(FATAL_ERROR "Nvidia GPU Computing SDK not found! Please set Option 'NVIDIA_GPU_COMPUTING_SDK'!")
    ENDIF()
    
    FIND_PATH(
        OPENCL_INCLUDE_DIR
        NAMES CL/cl.h
        PATHS ${NVIDIA_GPU_COMPUTING_SDK}
        PATH_SUFFIXES include OpenCL/common/inc
    )
    
    IF(VRN_WIN32)
        FIND_LIBRARY(
            OPENCL_LIBRARY
            NAMES OpenCL.lib
            PATHS ${NVIDIA_GPU_COMPUTING_SDK}
            PATH_SUFFIXES lib/Win32 OpenCL/common/lib/Win32
        )
    ELSEIF(VRN_WIN64)
        FIND_LIBRARY(
            OPENCL_LIBRARY
            NAMES OpenCL.lib
            PATHS ${NVIDIA_GPU_COMPUTING_SDK}
            PATH_SUFFIXES lib/x64 OpenCL/common/lib/x64
        )
    ENDIF()

ELSEIF(UNIX)

    FIND_PATH(
        OPENCL_INCLUDE_DIR
        NAMES CL/cl.h
        PATHS ${OPENCL_INCLUDE_DIR}
              ${OPENCL_DIR}/include
              ${OPENCL_DIR}/OpenCL/common/inc
              $ENV{OPENCL_INCLUDE_DIR}
              $ENV{OPENCL_DIR}/include
              $ENV{OPENCL_DIR}/OpenCL/common/inc
              /usr/local/cuda/include
              /usr/local/include
              /usr/include
    )

    FIND_LIBRARY(
        OPENCL_LIBRARY
        NAMES OpenCL
        PATHS ${OPENCL_LIBRARY_DIR}
              ${OPENCL_DIR}/lib
              ${OPENCL_DIR}/lib/x86
              $ENV{OPENCL_LIBRARY_DIR}
              $ENV{OPENCL_DIR}/lib
              $ENV{OPENCL_DIR}/lib/x86
              /usr/local/lib64
              /usr/local/lib
              /usr/lib64
              /usr/lib
    )

ENDIF()

IF(OPENCL_INCLUDE_DIR AND OPENCL_LIBRARY)
    MESSAGE(STATUS "  - Found OpenCL library")
    SET(MOD_INCLUDE_DIRECTORIES "${OPENCL_INCLUDE_DIR}")
    SET(MOD_LIBRARIES "${OPENCL_LIBRARY}")
    MARK_AS_ADVANCED(OPENCL_INCLUDE_DIR OPENCL_LIBRARY)
ELSE()
    MESSAGE(FATAL_ERROR "OpenCL library not found (OPENCL_INCLUDE_DIR and/or OPENCL_LIBRARY missing)!")
    MARK_AS_ADVANCED(CLEAR OPENCL_INCLUDE_DIR OPENCL_LIBRARY)
ENDIF()


################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS OpenCLModule)

SET(MOD_CORE_SOURCES 
    ${MOD_DIR}/utils/clwrapper.cpp
    ${MOD_DIR}/processors/dynamicclprocessor.cpp
    ${MOD_DIR}/processors/grayscale_cl.cpp
    ${MOD_DIR}/processors/raycaster_cl.cpp
    ${MOD_DIR}/processors/raytracingentryexitpoints.cpp
    ${MOD_DIR}/processors/volumegradient_cl.cpp
    ${MOD_DIR}/utils/voreenblascl.cpp
    ${MOD_DIR}/properties/openclproperty.cpp
)

SET(MOD_CORE_HEADERS
    ${MOD_DIR}/utils/clwrapper.h
    ${MOD_DIR}/processors/dynamicclprocessor.h
    ${MOD_DIR}/processors/grayscale_cl.h
    ${MOD_DIR}/processors/raycaster_cl.h
    ${MOD_DIR}/processors/raytracingentryexitpoints.h
    ${MOD_DIR}/processors/volumegradient_cl.h
    ${MOD_DIR}/utils/voreenblascl.h
    ${MOD_DIR}/properties/openclproperty.h
)

# deployment
SET(MOD_INSTALL_DIRECTORIES
    ${MOD_DIR}/cl
)

################################################################################
# Qt module resources 
################################################################################
SET(MOD_QT_MODULECLASS OpenCLModuleQt)

SET(MOD_QT_SOURCES 
    ${MOD_DIR}/qt/openclpropertywidgetfactory.cpp
    ${MOD_DIR}/qt/openclhighlighter.cpp
    ${MOD_DIR}/qt/openclpropertywidget.cpp
    ${MOD_DIR}/qt/openclplugin.cpp
    ${MOD_DIR}/qt/openclprocessorwidgetfactory.cpp
    ${MOD_DIR}/qt/dynamicopenclwidget.cpp
)

SET(MOD_QT_HEADERS
    ${MOD_DIR}/qt/openclpropertywidget.h
    ${MOD_DIR}/qt/openclplugin.h
    ${MOD_DIR}/qt/dynamicopenclwidget.h
)

SET(MOD_QT_HEADERS_NONMOC
    ${MOD_DIR}/qt/openclpropertywidgetfactory.h
    ${MOD_DIR}/qt/openclprocessorwidgetfactory.h
    ${MOD_DIR}/qt/openclhighlighter.h
)

