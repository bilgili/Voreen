# core module class
SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/openclmodule.cpp
HEADERS += $${VRN_MODULE_INC_DIR}/opencl/openclmodule.h

SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/grayscale_cl.cpp
SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/raytracingentryexitpoints.cpp
SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/volumegradient_cl.cpp
SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/voreenblas.cpp

HEADERS += $${VRN_MODULE_INC_DIR}/opencl/grayscale_cl.h
HEADERS += $${VRN_MODULE_INC_DIR}/opencl/raytracingentryexitpoints.h
HEADERS += $${VRN_MODULE_INC_DIR}/opencl/volumegradient_cl.h
HEADERS += $${VRN_MODULE_INC_DIR}/opencl/voreenblas.h

visual_studio {
SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/grayscale.cl
SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/gradient.cl
SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/voreenblas.cl
}

### Local Variables:
### mode:conf-unix
### End:
