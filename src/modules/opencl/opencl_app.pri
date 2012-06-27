# libraries to link

win32 {
    isEmpty(NVIDIA_GPU_COMPUTING_SDK) {
        warning("OpenCL module: NVIDIA_GPU_COMPUTING_SDK not set")
    }
    LIBS += -L$${NVIDIA_GPU_COMPUTING_SDK}/OpenCL/common/lib/Win32
    LIBS += -L$${NVIDIA_GPU_COMPUTING_SDK}/lib
    LIBS += -L$${NVIDIA_GPU_COMPUTING_SDK}
    LIBS += -lOpenCL
}

unix {
    QMAKE_LFLAGS += -lOpenCL
}

### Local Variables:
### mode:conf-unix
### End:
