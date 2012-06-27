####################################################
# Project file for the Voreen-Qt library
####################################################
#unix: TEMPLATE = lib
#win32: TEMPLATE = vclib
TEMPLATE = lib
TARGET = voreen_qt
VERSION = 1.0

CONFIG += qt
QT += opengl

QT += network
CONFIG += static thread
CONFIG -= dll

unix: DESTDIR = ../..
win32: {
  CONFIG(debug, debug|release) {
    DESTDIR = ../../Debug
  }
  else {
    DESTDIR = ../../Release
  }
}

# Include local configuration
!include(../../config.txt) {
  error("config.txt not found! copy config-default.txt to config.txt and edit!")
}

# Include common configuration
include(../../commonconf.txt)

# Include modules which are selected in local configuration. The entry
# 'foo' in VRN_MODULES must correspond to a subdir 'modules/foo' and a
# file 'foo_qt.pri' there.
for(i, VRN_MODULES) : include(../modules/$${i}/$${i}_qt.pri)

# include resource files
RESOURCES = $${VRN_HOME}/resource/qt/vrn_qt.qrc

# please insert new files in alphabetical order!
SOURCES += \
    cmdlineparser.cpp \
    datasetserver.cpp \
    helpbrowser.cpp \
    pyvoreenqt.cpp \
    qcolorluminancepicker.cpp \
    qcolorpicker.cpp \
    voreenapp.cpp \
    voreenmainframe.cpp
SOURCES += \
    widgets/adddialog.cpp \
    widgets/backgroundplugin.cpp \
    widgets/clipperwidget.cpp \
    widgets/clippingplugin.cpp \
    widgets/consoleplugin.cpp \
    widgets/dockbarhandler.cpp \
    widgets/doublesliderwidget.cpp \
    widgets/dynamicsplugin.cpp \
    widgets/flybyplugin.cpp \
    widgets/histogrampainterwidget.cpp \
    widgets/informationplugin.cpp \
    widgets/labelingwidgetqt.cpp \
    widgets/lightmaterialplugin.cpp \
    widgets/orientationplugin.cpp \
    widgets/pickingplugin.cpp\
    widgets/plugindialog.cpp \
    widgets/qlabelclickable.cpp \
    widgets/renderobjectsplugin.cpp \
    widgets/segmentationplugin.cpp \
    widgets/showtexcontainerwidget.cpp \
    widgets/sliderspinboxwidget.cpp \
    widgets/snapshotplugin.cpp \
    widgets/stereoplugin.cpp \
    widgets/thresholdwidget.cpp \
    widgets/volumesetwidget.cpp \
    widgets/widgetplugin.cpp \
    widgets/widgetgenerator.cpp \
    widgets/widgetgeneratorplugins.cpp \
    widgets/transfunc/transfuncplugin.cpp \
    widgets/transfunc/transfuncalphaplugin.cpp \
    widgets/transfunc/transfuncintensitypainter.cpp \
    widgets/transfunc/transfuncintensityplugin.cpp \
    widgets/transfunc/transfuncintensitypetplugin.cpp \
    widgets/transfunc/transfuncgradient.cpp \
    widgets/transfunc/transfuncmappingcanvas.cpp \
    widgets/transfunc/transfuncintensitygradientplugin.cpp \
    ../../ext/tgt/qt/qtcanvas.cpp \
    ../../ext/tgt/qt/qttimer.cpp

HEADERS += \
    ../../include/voreen/qt/cmdlineparser.h \
    ../../include/voreen/qt/datasetserver.h \
    ../../include/voreen/qt/helpbrowser.h \
    ../../include/voreen/qt/pyvoreenqt.h \
    ../../include/voreen/qt/qcolorluminancepicker.h \
    ../../include/voreen/qt/qcolorpicker.h \
    ../../include/voreen/qt/qdebug.h \
    ../../include/voreen/qt/voreenapp.h \
    ../../include/voreen/qt/voreenmainframe.h
HEADERS += \
    ../../include/voreen/qt/widgets/adddialog.h \
    ../../include/voreen/qt/widgets/backgroundplugin.h \
    ../../include/voreen/qt/widgets/clipperwidget.h \
    ../../include/voreen/qt/widgets/clippingplugin.h \
    ../../include/voreen/qt/widgets/consoleplugin.h \
    ../../include/voreen/qt/widgets/dockbarhandler.h \
    ../../include/voreen/qt/widgets/doublesliderwidget.h \
    ../../include/voreen/qt/widgets/dynamicsplugin.h \
    ../../include/voreen/qt/widgets/flybyplugin.h \
    ../../include/voreen/qt/widgets/histogrampainterwidget.h \
    ../../include/voreen/qt/widgets/informationplugin.h \
    ../../include/voreen/qt/widgets/labelingwidgetqt.h \
    ../../include/voreen/qt/widgets/lightmaterialplugin.h \
    ../../include/voreen/qt/widgets/orientationplugin.h \
    ../../include/voreen/qt/widgets/pickingplugin.h\
    ../../include/voreen/qt/widgets/plugindialog.h \
    ../../include/voreen/qt/widgets/qlabelclickable.h \
    ../../include/voreen/qt/widgets/renderobjectsplugin.h \
    ../../include/voreen/qt/widgets/segmentationplugin.h \
    ../../include/voreen/qt/widgets/showtexcontainerwidget.h \
    ../../include/voreen/qt/widgets/sliderspinboxwidget.h \
    ../../include/voreen/qt/widgets/snapshotplugin.h \
    ../../include/voreen/qt/widgets/stereoplugin.h \
    ../../include/voreen/qt/widgets/thresholdwidget.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncplugin.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncalphaplugin.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncintensitypainter.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncintensityplugin.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncintensitypetplugin.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncgradient.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncintensitygradientplugin.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncmappingcanvas.h \
    ../../include/voreen/qt/widgets/volumesetwidget.h \
    ../../include/voreen/qt/widgets/widgetgenerator.h \
    ../../include/voreen/qt/widgets/widgetgeneratorplugins.h \
    ../../include/voreen/qt/widgets/widgetplugin.h \
    ../../ext/tgt/qt/qtcanvas.h \
    ../../ext/tgt/qt/qttimer.h
MSVC_IDE: SOURCES += \
    ../core/vis/glsl/stc_showdepth.frag \
    ../core/vis/glsl/stc_showfloatcolor.frag \
    ../core/vis/glsl/stc_showtexture.frag

win32 {
    contains(DEFINES, VRN_WITH_QWT) {
        INCLUDEPATH += $${VRN_HOME}/ext/qwt/include
        DEFINES += QWT_DLL
    }
}

# add files, which are not available in the snapshot release
# these files will be added to the snapshot, when they are cleaned up
!contains(DEFINES, VRN_SNAPSHOT) {
SOURCES += \
    widgets/ultrasoundplugin.cpp \
    widgets/usframeoverlay.cpp \
    widgets/usschematicrenderarea.cpp \
    widgets/vectorfieldplugin.cpp \

HEADERS += \
    ../../include/voreen/qt/widgets/ultrasoundplugin.h \
    ../../include/voreen/qt/widgets/usframeoverlay.h \
    ../../include/voreen/qt/widgets/usschematicrenderarea.h \
    ../../include/voreen/qt/widgets/vectorfieldplugin.h \
}


# this must come after all SOURCES, HEADERS and FORMS have been added
contains(DEFINES, VRN_WITH_SVNVERSION) : revtarget.depends = $$SOURCES $$HEADERS $$FORMS

### Local Variables:
### mode:conf-unix
### End: