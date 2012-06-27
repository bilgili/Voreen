debug {
    DEFINES += tgtDEBUG
}

unix : contains(DEFINES, tgtZLIB) {
  LIBS += -lz
}

contains(DEFINES, TGT_USE_PYTHON) {
    SOURCES += ../../ext/tgt/scriptmanager.cpp
    HEADERS += ../../ext/tgt/scriptmanager.h
}

contains(DEFINES, TGT_WITH_TRACKING) {
    SOURCES += ../../ext/tgt/modules/tracking/tracker.cpp \
               ../../ext/tgt/modules/tracking/eostracker.cpp

    HEADERS += ../../ext/tgt/modules/tracking/tracker.h \
               ../../ext/tgt/modules/tracking/eostracker.h \
               ../../ext/tgt/modules/tracking/trackingevent.h \
               ../../ext/tgt/modules/tracking/headtracking.h
}

win32: {
  # for CoInitializeSecurity() to be defined
  contains(DEFINES, TGT_WITH_WMI) {
    DEFINES += _WIN32_DCOM
  }
}

contains(DEFINES, VRN_WITH_DEVIL) {
    DEFINES += TGT_HAS_DEVIL
}

SOURCES += \
    ../../ext/tgt/assert.cpp \
    ../../ext/tgt/bounds.cpp \
    ../../ext/tgt/camera.cpp \
    ../../ext/tgt/curve.cpp \
    ../../ext/tgt/exception.cpp \
    ../../ext/tgt/filesystem.cpp \
    ../../ext/tgt/freetype.cpp \
    ../../ext/tgt/frustum.cpp \
    ../../ext/tgt/glcanvas.cpp \
    ../../ext/tgt/gpucapabilities.cpp \
    ../../ext/tgt/gpucapabilitieswindows.cpp \
    ../../ext/tgt/init.cpp \
    ../../ext/tgt/light.cpp \
    ../../ext/tgt/memorymanager.cpp \
    ../../ext/tgt/modelmanager.cpp \
    ../../ext/tgt/painter.cpp \
    ../../ext/tgt/shadermanager.cpp \
    ../../ext/tgt/spline.cpp \
    ../../ext/tgt/stopwatch.cpp \
    ../../ext/tgt/tesselator.cpp \
    ../../ext/tgt/texturemanager.cpp \
    ../../ext/tgt/texture.cpp \
    ../../ext/tgt/texturereader.cpp \
    ../../ext/tgt/texturereaderdevil.cpp \
    ../../ext/tgt/texturereadertga.cpp \
    ../../ext/tgt/tgt_gl.cpp \
    ../../ext/tgt/timer.cpp \
    ../../ext/tgt/event/eventhandler.cpp \
    ../../ext/tgt/event/eventlistener.cpp \
    ../../ext/tgt/event/keyevent.cpp \
    ../../ext/tgt/event/mouseevent.cpp \
    ../../ext/tgt/event/timeevent.cpp \
    ../../ext/tgt/logmanager.cpp \
    ../../ext/tgt/navigation/trackball.cpp \
    ../../ext/tgt/navigation/navigation.cpp

HEADERS += \
    ../../ext/tgt/assert.h \
    ../../ext/tgt/attribute.h \
    ../../ext/tgt/bounds.h \
    ../../ext/tgt/camera.h \
    ../../ext/tgt/curve.h \
    ../../ext/tgt/exception.h \
    ../../ext/tgt/filesystem.h \
    ../../ext/tgt/framecounter.h \
    ../../ext/tgt/freetype.h \
    ../../ext/tgt/frustum.h \
    ../../ext/tgt/glcanvas.h \
    ../../ext/tgt/glmath.h \
    ../../ext/tgt/gpucapabilities.h \
    ../../ext/tgt/gpucapabilitieswindows.h \
    ../../ext/tgt/init.h \
    ../../ext/tgt/light.h \
    ../../ext/tgt/manager.h \
    ../../ext/tgt/material.h \
    ../../ext/tgt/math.h \
    ../../ext/tgt/matrix.h \
    ../../ext/tgt/memorymanager.h \
    ../../ext/tgt/modelmanager.h \
    ../../ext/tgt/mouse.h \
    ../../ext/tgt/painter.h \
    ../../ext/tgt/plane.h \
    ../../ext/tgt/quadric.h \
    ../../ext/tgt/quadtree.h \
    ../../ext/tgt/quaternion.h \
    ../../ext/tgt/renderable.h \
    ../../ext/tgt/singleton.h \
    ../../ext/tgt/spline.h \
    ../../ext/tgt/stopwatch.h \
    ../../ext/tgt/timer.h \
    ../../ext/tgt/tesselator.h \
    ../../ext/tgt/tgt_gl.h \
    ../../ext/tgt/types.h \
    ../../ext/tgt/texturemanager.h \
    ../../ext/tgt/texturereader.h \
    ../../ext/tgt/texturereaderdevil.h \
    ../../ext/tgt/texture.h \
    ../../ext/tgt/shadermanager.h \
    ../../ext/tgt/timer.h \
    ../../ext/tgt/vector.h \
    ../../ext/tgt/vertex.h \
\
    ../../ext/tgt/event/event.h \
    ../../ext/tgt/event/eventhandler.h \
    ../../ext/tgt/event/keyevent.h \
    ../../ext/tgt/event/mouseevent.h \
    ../../ext/tgt/event/timeevent.h \
    ../../ext/tgt/event/exitevent.h \
    ../../ext/tgt/logmanager.h \
    ../../ext/tgt/navigation/trackball.h \
    ../../ext/tgt/navigation/navigation.h

