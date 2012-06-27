debug {
    DEFINES += tgtDEBUG
}

unix : contains(DEFINES, tgtZLIB) {
  LIBS += -lz
}

contains(DEFINES, TGT_USE_PYTHON) {
    SOURCES += $${VRN_HOME}/ext/tgt/scriptmanager.cpp
    HEADERS += $${VRN_HOME}/ext/tgt/scriptmanager.h
}

contains(DEFINES, TGT_WITH_TRACKING) {
    SOURCES += $${VRN_HOME}/ext/tgt/modules/tracking/tracker.cpp \
               $${VRN_HOME}/ext/tgt/modules/tracking/eostracker.cpp

    HEADERS += $${VRN_HOME}/ext/tgt/modules/tracking/tracker.h \
               $${VRN_HOME}/ext/tgt/modules/tracking/eostracker.h \
               $${VRN_HOME}/ext/tgt/modules/tracking/trackingevent.h \
               $${VRN_HOME}/ext/tgt/modules/tracking/headtracking.h
}

contains(DEFINES, VRN_WITH_DEVIL) {
    DEFINES += TGT_HAS_DEVIL
}

SOURCES += \
    $${VRN_HOME}/ext/tgt/assert.cpp \
    $${VRN_HOME}/ext/tgt/bounds.cpp \
    $${VRN_HOME}/ext/tgt/camera.cpp \
    $${VRN_HOME}/ext/tgt/curve.cpp \
    $${VRN_HOME}/ext/tgt/exception.cpp \
    $${VRN_HOME}/ext/tgt/filesystem.cpp \
    $${VRN_HOME}/ext/tgt/frustum.cpp \
    $${VRN_HOME}/ext/tgt/glcanvas.cpp \
    $${VRN_HOME}/ext/tgt/gpucapabilities.cpp \
    $${VRN_HOME}/ext/tgt/init.cpp \
    $${VRN_HOME}/ext/tgt/light.cpp \
    $${VRN_HOME}/ext/tgt/memorymanager.cpp \
    $${VRN_HOME}/ext/tgt/modelmanager.cpp \
    $${VRN_HOME}/ext/tgt/painter.cpp \
    $${VRN_HOME}/ext/tgt/spline.cpp \
    $${VRN_HOME}/ext/tgt/stopwatch.cpp \
    $${VRN_HOME}/ext/tgt/tesselator.cpp \
    $${VRN_HOME}/ext/tgt/texturemanager.cpp \
    $${VRN_HOME}/ext/tgt/texture.cpp \
    $${VRN_HOME}/ext/tgt/texturereader.cpp \
    $${VRN_HOME}/ext/tgt/texturereaderdevil.cpp \
    $${VRN_HOME}/ext/tgt/texturereadertga.cpp \
    $${VRN_HOME}/ext/tgt/tgt_gl.cpp \
    $${VRN_HOME}/ext/tgt/timer.cpp \
    $${VRN_HOME}/ext/tgt/shadermanager.cpp \
    $${VRN_HOME}/ext/tgt/event/eventhandler.cpp \
    $${VRN_HOME}/ext/tgt/event/eventlistener.cpp \
    $${VRN_HOME}/ext/tgt/event/keyevent.cpp \
    $${VRN_HOME}/ext/tgt/event/mouseevent.cpp \
    $${VRN_HOME}/ext/tgt/event/timeevent.cpp \
    $${VRN_HOME}/ext/tgt/logmanager.cpp \
    $${VRN_HOME}/ext/tgt/navigation/trackball.cpp \
    $${VRN_HOME}/ext/tgt/navigation/navigation.cpp

HEADERS += \
    $${VRN_HOME}/ext/tgt/assert.h \
    $${VRN_HOME}/ext/tgt/attribute.h \
    $${VRN_HOME}/ext/tgt/bounds.h \
    $${VRN_HOME}/ext/tgt/camera.h \
    $${VRN_HOME}/ext/tgt/curve.h \
    $${VRN_HOME}/ext/tgt/exception.h \
    $${VRN_HOME}/ext/tgt/filesystem.h \
    $${VRN_HOME}/ext/tgt/framecounter.h \
    $${VRN_HOME}/ext/tgt/frustum.h \
    $${VRN_HOME}/ext/tgt/glcanvas.h \
    $${VRN_HOME}/ext/tgt/glmath.h \
    $${VRN_HOME}/ext/tgt/gpucapabilities.h \
    $${VRN_HOME}/ext/tgt/init.h \
    $${VRN_HOME}/ext/tgt/light.h \
    $${VRN_HOME}/ext/tgt/manager.h \
    $${VRN_HOME}/ext/tgt/material.h \
    $${VRN_HOME}/ext/tgt/math.h \
    $${VRN_HOME}/ext/tgt/matrix.h \
    $${VRN_HOME}/ext/tgt/memorymanager.h \
    $${VRN_HOME}/ext/tgt/modelmanager.h \
    $${VRN_HOME}/ext/tgt/mouse.h \
    $${VRN_HOME}/ext/tgt/painter.h \
    $${VRN_HOME}/ext/tgt/plane.h \
    $${VRN_HOME}/ext/tgt/quadric.h \
    $${VRN_HOME}/ext/tgt/quadtree.h \
    $${VRN_HOME}/ext/tgt/quaternion.h \
    $${VRN_HOME}/ext/tgt/renderable.h \
    $${VRN_HOME}/ext/tgt/singleton.h \
    $${VRN_HOME}/ext/tgt/spline.h \
    $${VRN_HOME}/ext/tgt/stopwatch.h \
    $${VRN_HOME}/ext/tgt/timer.h \
    $${VRN_HOME}/ext/tgt/tesselator.h \
    $${VRN_HOME}/ext/tgt/tgt_gl.h \
    $${VRN_HOME}/ext/tgt/types.h \
    $${VRN_HOME}/ext/tgt/texturemanager.h \
    $${VRN_HOME}/ext/tgt/texturereader.h \
    $${VRN_HOME}/ext/tgt/texturereaderdevil.h \
    $${VRN_HOME}/ext/tgt/texture.h \
    $${VRN_HOME}/ext/tgt/shadermanager.h \
    $${VRN_HOME}/ext/tgt/timer.h \
    $${VRN_HOME}/ext/tgt/vector.h \
    $${VRN_HOME}/ext/tgt/vertex.h \
\
    $${VRN_HOME}/ext/tgt/event/event.h \
    $${VRN_HOME}/ext/tgt/event/eventhandler.h \
    $${VRN_HOME}/ext/tgt/event/keyevent.h \
    $${VRN_HOME}/ext/tgt/event/mouseevent.h \
    $${VRN_HOME}/ext/tgt/event/timeevent.h \
    $${VRN_HOME}/ext/tgt/event/exitevent.h \
    $${VRN_HOME}/ext/tgt/logmanager.h \
    $${VRN_HOME}/ext/tgt/navigation/trackball.h \
    $${VRN_HOME}/ext/tgt/navigation/navigation.h

