
# OpenCL wrapper
SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/clwrapper.cpp
HEADERS += $${VRN_MODULE_INC_DIR}/opencl/clwrapper.h

# processors
SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/grayscale_cl.cpp
SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/raytracingentryexitpoints.cpp
SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/volumegradient_cl.cpp
SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/voreenblas.cpp

HEADERS += $${VRN_MODULE_INC_DIR}/opencl/grayscale_cl.h
HEADERS += $${VRN_MODULE_INC_DIR}/opencl/raytracingentryexitpoints.h
HEADERS += $${VRN_MODULE_INC_DIR}/opencl/volumegradient_cl.h
HEADERS += $${VRN_MODULE_INC_DIR}/opencl/voreenblas.h

SHADER_SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/grayscale.cl
SHADER_SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/gradient.cl
SHADER_SOURCES += $${VRN_MODULE_SRC_DIR}/opencl/voreenblas.cl

### Local Variables:
### mode:conf-unix
### End:
