TARGET = voreenve
TEMPLATE = app
LANGUAGE = C++

CONFIG += qt
QT += opengl

# check qmake version
QMAKE_VERS = $$[QMAKE_VERSION]
QMAKE_VERSION_CHECK = $$find(QMAKE_VERS, "^[234]\\.")
isEmpty(QMAKE_VERSION_CHECK) {
   error("Your qmake version '$$QMAKE_VERS' is too old, qmake from Qt 4 is required!")
}

# include config
!exists(../../config.txt) {
  error("config.txt not found! copy config-default.txt to config.txt and edit!")
}
include(../../config.txt)

# Include common configuration
include(../../commonconf.pri)

# Include generic app configuration
include(../voreenapp.pri)

win32 {
  contains(DEFINES, VRN_DEBUG) {
    # link against console subsystem for debug builds (opens console)
    CONFIG += console
  }
}

HEADERS += \
    voreenmainwindow.h \
    voreenvisualization.h \
    networkeditor/aggregationgraphicsitem.h \
    networkeditor/arrowgraphicsitem.h \
	networkeditor/arrowheadselectiongraphicsitem.h \
    networkeditor/hastooltip.h \
    networkeditor/linkarrowgraphicsitem.h \
    networkeditor/linkarrowgraphicsitemstub.h \
    networkeditor/linkdialogarrowgraphicsitem.h \
    networkeditor/linkdialoggraphicsview.h \
    networkeditor/linkdialogprocessorgraphicsitem.h \
    networkeditor/linkdialogpropertygraphicsitem.h \
    networkeditor/networkeditor.h \
    networkeditor/networkeditor_common.h \
    networkeditor/openpropertylistbutton.h \
    networkeditor/portarrowgraphicsitem.h \
    networkeditor/portgraphicsitem.h \
    networkeditor/processorgraphicsitem.h \
	networkeditor/progressbargraphicsitem.h \
    networkeditor/propertygraphicsitem.h \
    networkeditor/propertylinkdialog.h \
    networkeditor/propertylistgraphicsitem.h \
    networkeditor/rootgraphicsitem.h \
    networkeditor/textgraphicsitem.h \
	networkeditor/widgetindicatorbutton.h

SOURCES += \
    main.cpp \
    voreenmainwindow.cpp \
    voreenvisualization.cpp \
    networkeditor/aggregationgraphicsitem.cpp \
    networkeditor/arrowgraphicsitem.cpp \
	networkeditor/arrowheadselectiongraphicsitem.cpp \
    networkeditor/linkarrowgraphicsitem.cpp \
    networkeditor/linkarrowgraphicsitemstub.cpp \
    networkeditor/linkdialogarrowgraphicsitem.cpp \
    networkeditor/linkdialoggraphicsview.cpp \
    networkeditor/linkdialogprocessorgraphicsitem.cpp \
    networkeditor/linkdialogpropertygraphicsitem.cpp \
    networkeditor/networkeditor.cpp \
    networkeditor/openpropertylistbutton.cpp \
    networkeditor/portarrowgraphicsitem.cpp \
    networkeditor/portgraphicsitem.cpp \
    networkeditor/processorgraphicsitem.cpp \
	networkeditor/progressbargraphicsitem.cpp \
    networkeditor/propertygraphicsitem.cpp \
    networkeditor/propertylinkdialog.cpp \
    networkeditor/propertylistgraphicsitem.cpp \
    networkeditor/rootgraphicsitem.cpp \
    networkeditor/textgraphicsitem.cpp \
	networkeditor/widgetindicatorbutton.cpp

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
