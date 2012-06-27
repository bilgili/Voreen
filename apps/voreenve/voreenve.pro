TARGET = voreenve
TEMPLATE = app
LANGUAGE = C++

CONFIG += console qt
QT += opengl

# Include local configuration
include(../../config.txt)

# Include common configuration
include(../../commonconf.txt)

# Include generic app configuration
include(../voreenapp.txt)

HEADERS += \
    voreencanvaswidget.h \
    voreenmainwindow.h \
    workspace.h 

    
contains(DEFINES, VRN_WITH_DCMTK) {
  HEADERS += voreen/qt/dicomdialog.h
}

SOURCES += \
    main.cpp \
    voreencanvaswidget.cpp \
    voreenmainwindow.cpp

contains(DEFINES, VRN_WITH_DCMTK) {
  SOURCES +=  ../../src/qt/dicomdialog.cpp
}

# for compiled-in files (see Qt resource system)
!contains(DEFINES, VRN_SNAPSHOT) {
   RESOURCES += voreenve.qrc 
}

win32 {
    # icon description file for windows-exe
    RC_FILE = "../../resource/vrn_share/icons/winicon.rc"
}

macx {
  # icon for the application bundle
  ICON = "$${VRN_HOME}/resource/vrn_share/icons/icon.icns"

  # installer target for creating distribution DMG
  contains(DEFINES, VRN_DISTRIBUTION) {
    deploy.path = ./
    deploy.extra = ../../tools/macdeployment/macdeployment.sh
    INSTALLS += deploy
  }  
}

unix {
    # Prevent a namespace clash
   DEFINES += QT_CLEAN_NAMESPACE

   UI_DIR = .ui

   # Build new tar archive with shader file with "make shaders.tar"
   shaders_tar.target = shaders.tar
   shaders_tar.commands = cd ../../src/core/vis/glsl/ && \
                          tar -cf $$PWD/shaders.tar * \
                              --owner root --group 0 --exclude \*.svn
   QMAKE_EXTRA_TARGETS += shaders_tar
}

contains(DEFINES, VRN_WITH_CLOSEUP_RENDERER) {
   LIBS += -lBox2D
}

win32-g++ {
  EXTDIR=..\..\ext
  DLLDEST=$$DESTDIR

  contains(DEFINES, VRN_WITH_DEVIL) {
    devil_dll.target = $$DLLDEST\DevIL.dll
    devil_dll.commands = copy $$EXTDIR\il\lib\DevIL.dll $$DLLDEST
    jpeg_dll.target = $$DLLDEST\jpeg62.dll
    jpeg_dll.commands = copy $$EXTDIR\jpeg\jpeg62.dll $$DLLDEST

    QMAKE_EXTRA_TARGETS += devil_dll jpeg_dll
    POST_TARGETDEPS += $$DLLDEST\DevIL.dll $$DLLDEST\jpeg62.dll
  }

  contains(DEFINES, VRN_WITH_TIFF) {
    tiff_dll.target = $$DLLDEST\libtiff3.dll
    tiff_dll.commands = copy $$EXTDIR\tiff\lib\libtiff3.dll $$DLLDEST
    zlib_dll.target = $$DLLDEST\zlib1.dll
    zlib_dll.commands = copy $$EXTDIR\zlib\zlib1.dll $$DLLDEST
    
    QMAKE_EXTRA_TARGETS += tiff_dll zlib_dll
    POST_TARGETDEPS += $$DLLDEST\libtiff3.dll $$DLLDEST\zlib1.dll
  }
  
  contains(DEFINES, VRN_WITH_ZIP) {
    ziparchive_dll.target = $$DLLDEST\libziparchive.dll
    ziparchive_dll.commands = copy $$EXTDIR\ziparchive\lib\mingw\libziparchive.dll $$DLLDEST
    
    QMAKE_EXTRA_TARGETS += ziparchive_dll
    POST_TARGETDEPS += $$DLLDEST\libziparchive.dll
  }
  
  contains(DEFINES, VRN_WITH_FONTRENDERING) {
    freetype_dll.target = $$DLLDEST\libfreetype-6.dll
    freetype_dll.commands = copy $$EXTDIR\freetype\lib\mingw\libfreetype-6.dll $$DLLDEST
    ftgl_dll.target = $$DLLDEST\libftgl-2.dll
    ftgl_dll.commands = copy $$EXTDIR\ftgl\lib\mingw\libftgl-2.dll $$DLLDEST

    QMAKE_EXTRA_TARGETS += freetype_dll ftgl_dll
    POST_TARGETDEPS += $$DLLDEST\libfreetype-6.dll $$DLLDEST\libftgl-2.dll
  }
}

### Local Variables:
### mode:conf-unix
### End:
