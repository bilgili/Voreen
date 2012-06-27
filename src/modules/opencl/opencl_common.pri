DEFINES += VRN_MODULE_OPENCL

# module class  
VRN_MODULE_CLASSES += OpenCLModule
VRN_MODULE_CLASS_HEADERS += opencl/openclmodule.h
VRN_MODULE_CLASS_SOURCES += opencl/openclmodule.cpp

win32 {
    isEmpty(NVIDIA_GPU_COMPUTING_SDK) {
        error("OpenCL module: NVIDIA_GPU_COMPUTING_SDK not set. See src/modules/opencl/README.txt")
    }
    INCLUDEPATH += "$${NVIDIA_GPU_COMPUTING_SDK}/OpenCL/common/inc"
    INCLUDEPATH += "$${NVIDIA_GPU_COMPUTING_SDK}/include
}
