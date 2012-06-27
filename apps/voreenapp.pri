####################################################
# Generic project file for all Voreen applications
####################################################

# common Qt resource files
qt : RESOURCES = "$${VRN_HOME}/resource/vrn_share/vrn_app.qrc"

contains(DEFINES, VRN_PRECOMPILE_HEADER) { 
  qt {
    PRECOMPILED_HEADER = ../pch_qtapp.h
  }
  else {
    PRECOMPILED_HEADER = ../../pch.h
  }  CONFIG += precompile_header
}
else {
  CONFIG -= precompile_header
}

####################################################
# Platform-dependant configuration
####################################################

win32 {
  CONFIG(debug, debug|release) {
    MSC_CONFIG = Debug
  } else {
    MSC_CONFIG = Release
  }

  visual_studio {
    # Contains "Release" or "Debug" as selected in the IDE
    MSC_CONFIG = $(ConfigurationName)
  }

  QMAKE_LIBDIR += "$${VRN_HOME}/ext/glew/lib"

  win32-msvc {
    # these libs are not automatically added when building with nmake
    nmake: LIBS *= advapi32.lib shell32.lib

    qt: LIBS += "$${VRN_HOME}/$${MSC_CONFIG}/voreen_qt.lib" -lqtmain

    LIBS += "$${VRN_HOME}/$${MSC_CONFIG}/voreen_core.lib"
  }

  win32-g++ {
    qt: LIBS += "$${VRN_HOME}/$${MSC_CONFIG}/libvoreen_qt.a"
    LIBS += "$${VRN_HOME}/$${MSC_CONFIG}/libvoreen_core.a" \
  }

  LIBS += "$${VRN_HOME}/ext/glew/lib/$${MSC_CONFIG}/glew32s.lib"

  # no libc for vc++ since we build a multithreaded executable
  win32-msvc {
    LIBS += /NODEFAULTLIB:libc.lib
  }

  contains(DEFINES, VRN_WITH_DEVIL) {
    LIBS += "$${DEVIL_DIR}/lib/DevIL.lib"
  }

  contains(DEFINES, VRN_WITH_ZLIB) {
    LIBS += "$${ZLIB_DIR}/lib/zdll.lib"
  }	

  contains(DEFINES, VRN_WITH_FONTRENDERING) {
    win32-msvc: LIBS += "$${FREETYPE_DIR}/lib/freetype.lib"
    win32-g++:  LIBS += "$${FREETYPE_DIR}/lib/mingw/freetype.lib"
    win32-msvc: LIBS += "$${FTGL_DIR}/lib/ftgl.lib"
    win32-g++:  LIBS += "$${FTGL_DIR}/lib/mingw/ftgl.lib"
    INCLUDEPATH += "$${FTGL_DIR}/include"
  }

  contains(DEFINES, VRN_WITH_FFMPEG) {
    LIBS += "$${FFMPEG_DIR}/win32/avcodec.lib"
    LIBS += "$${FFMPEG_DIR}/win32/avdevice.lib"
    LIBS += "$${FFMPEG_DIR}/win32/avformat.lib"
    LIBS += "$${FFMPEG_DIR}/win32/avutil.lib"
    LIBS += "$${FFMPEG_DIR}/win32/swscale.lib"
  }
  
  LIBS += -lnetapi32 -lopengl32 -lglu32

  # For reading file version, file date and making registry calls
  # via Windows API
  LIBS += -lVersion

  # Windows Management Instrumentation (WMI) for hardware detection
  contains(DEFINES, TGT_WITH_WMI) {
    LIBS += -lWbemUuid
  }

}

unix {
  DEFINES += LINUX

  !macx: LIBS += -lGL -lGLU
  LIBS += -lGLEW
  qt : LIBS += -lvoreen_qt
  LIBS += -lvoreen_core

  contains(DEFINES, VRN_WITH_DEVIL) {
    LIBS += -lIL
  }

  contains(DEFINES, VRN_WITH_ZLIB) {
    LIBS += -lz
  }

  contains(DEFINES, VRN_WITH_FONTRENDERING) {
    LIBS += -lfreetype -lftgl
  }

  contains(DEFINES, VRN_WITH_FFMPEG) {
    # It is important that this comes after linking the voreen_* libs.
    LIBS += -lbz2 -lavformat -lavcodec -lavutil -lswscale
  }

  contains(DEFINES, VRN_WITH_LZO) {
    LIBS += -llzo2
  }

  !isEmpty(INSTALL_PREFIX) {
    target.path = $$INSTALLPATH_BIN
    INSTALLS += target
  }
}

macx {
  LIBS += -framework OpenGL
  LIBS += -framework ApplicationServices
}


# Include modules which are selected in local configuration. The entry
# 'foo' in VRN_MODULES must correspond to a subdir 'modules/foo' and a
# file 'foo_app.pri' there.
for(i, VRN_MODULES) : exists($${VRN_HOME}/src/modules/$${i}/$${i}_app.pri) {
  include($${VRN_HOME}/src/modules/$${i}/$${i}_app.pri)
}

### Local Variables:
### mode:conf-unix
### End:
