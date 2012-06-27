# qmake project file for FBO example

TARGET = helloGPGPU_GLSL_FBO
TEMPLATE = app
INCLUDEPATH += .

HEADERS += framebufferObject.h glErrorUtil.h renderbuffer.h

SOURCES += framebufferObject.cpp \
           glErrorUtil.cpp \
           helloGPGPU_GLSL_FBO.cpp \
           renderbuffer.cpp

include(../../config.txt)

LIBS += $${LIBDIR}
LIBS += -lGL -lGLU -lGLEW -lglut
