####################################################
# Project file for the Voreen-Qt library
####################################################
TEMPLATE = lib
TARGET = voreen_qt
VERSION = 1.0

CONFIG += qt
QT += opengl

QT += network
CONFIG += static thread
CONFIG -= dll

# Check Qt version:
# Use a regex that matches all invalid version numbers, i.e, X.*.*
# with X <= 3 and 4.Y.* with Y <= 2.
VERSION_CHECK = $$find(QT_VERSION, "^([123]|4\.[12])\..*$")
!isEmpty(VERSION_CHECK) {
   error("Your Qt version is $$QT_VERSION but at least 4.3.0 is required!")
}

# Include local configuration
!include(../../config.txt) {
  warning("config.txt not found! Using config-default.txt instead.")
  warning("For custom behavior, copy config-default.txt to config.txt and edit!")
  include(../../config-default.txt)
}

# Include common configuration
include(../../commonconf.txt)

macx: DESTDIR = ../..
unix: DESTDIR = ../..
win32: {
  CONFIG(debug, debug|release) {
    DESTDIR = ../../Debug
  }
  else {
    DESTDIR = ../../Release
  }
}

# Include modules which are selected in local configuration. The entry
# 'foo' in VRN_MODULES must correspond to a subdir 'modules/foo' and a
# file 'foo_qt.pri' there.
for(i, VRN_MODULES) : exists($${VRN_HOME}/src/modules/$${i}/$${i}_qt.pri) {
  include($${VRN_HOME}/src/modules/$${i}/$${i}_qt.pri)
}

# include resource files
RESOURCES = $${VRN_HOME}/resource/qt/vrn_qt.qrc

# please insert new files in alphabetical order!
##############
# Sources
##############
SOURCES += \
    aboutbox.cpp \
    helpbrowser.cpp \
    ioprogressdialog.cpp \
    pyvoreenqt.cpp \
    versionqt.cpp \
    voreenapplicationqt.cpp
SOURCES += \
    widgets/animationplugin.cpp \
    widgets/choicelistcombobox.cpp \
    widgets/consoleplugin.cpp \
    widgets/eventpropertywidget.cpp \
    widgets/expandableheaderbutton.cpp \
    widgets/glslhighlighter.cpp \
    widgets/keydetectorwidget.cpp \
    widgets/labelingwidgetqt.cpp \
    widgets/plugindialog.cpp \
    widgets/qlabelclickable.cpp \
    widgets/lineeditresetwidget.cpp \
    widgets/rawvolumewidget.cpp \
    widgets/segmentationplugin.cpp \
    widgets/shaderplugin.cpp \
    widgets/shortcutpreferenceswidget.cpp \
    widgets/rendertargetdebugwidget.cpp \
    widgets/sliderspinboxwidget.cpp \
    widgets/snapshotplugin.cpp \
    widgets/tablesstextedit.cpp \
    widgets/volumecontainerwidget.cpp \
    widgets/volumeviewhelper.cpp \
    widgets/voreentoolwindow.cpp \
    widgets/widgetplugin.cpp
SOURCES += \
    widgets/property/boolpropertywidget.cpp \
    widgets/property/buttonpropertywidget.cpp \
    widgets/property/camerapropertywidget.cpp \
    widgets/property/colorpropertywidget.cpp \
    widgets/property/filedialogpropertywidget.cpp \
    widgets/property/floatpropertywidget.cpp \
    widgets/property/floatvec2propertywidget.cpp \
    widgets/property/floatvec3propertywidget.cpp \
    widgets/property/floatvec4propertywidget.cpp \
    widgets/property/intpropertywidget.cpp \
    widgets/property/intvec2propertywidget.cpp \
    widgets/property/intvec3propertywidget.cpp \
    widgets/property/intvec4propertywidget.cpp \
    widgets/property/optionpropertywidget.cpp \
    widgets/property/propertyvectorwidget.cpp \
    widgets/property/processorpropertieswidget.cpp \
    widgets/property/qpropertywidget.cpp \
    widgets/property/qpropertywidgetfactory.cpp \
    widgets/property/shaderpropertywidget.cpp \
    widgets/property/stringpropertywidget.cpp \
    widgets/property/transfuncpropertywidget.cpp \
	widgets/property/vecpropertywidget.cpp \
    widgets/property/volumecollectionpropertywidget.cpp \
    widgets/property/volumehandlepropertywidget.cpp
SOURCES += \
    widgets/network/processorlistwidget.cpp \
    widgets/network/propertylistwidget.cpp \
    widgets/network/editor/arrowgraphicsitem.cpp \
    widgets/network/editor/linkarrowgraphicsitem.cpp \
    widgets/network/editor/portgraphicsitem.cpp \
    widgets/network/editor/networkeditor.cpp \
    widgets/network/editor/openpropertylistbutton.cpp \
    widgets/network/editor/processorgraphicsitem.cpp \
    widgets/network/editor/propertylistgraphicsitem.cpp \
    widgets/network/editor/tctooltip.cpp \
    widgets/network/editor/propertygraphicsitem.cpp \
    widgets/network/editor/propertylinkdialog.cpp \
    widgets/network/editor/scripteditor.cpp \
    widgets/network/editor/textgraphicsitem.cpp \
    widgets/network/editor/tooltiptimer.cpp
SOURCES += \
    widgets/processor/qprocessorwidget.cpp \
    widgets/processor/qprocessorwidgetfactory.cpp \
    widgets/processor/canvasrendererwidget.cpp
SOURCES += \
    widgets/property/camerawidget.cpp 
SOURCES += \
    widgets/transfunc/colorluminancepicker.cpp \
    widgets/transfunc/colorpicker.cpp \
    widgets/transfunc/doubleslider.cpp \
    widgets/transfunc/histogrampainter.cpp \
    widgets/transfunc/transfunceditor.cpp \
    widgets/transfunc/transfunceditorintensity.cpp \
    widgets/transfunc/transfunceditorintensitygradient.cpp \
    widgets/transfunc/transfunceditorintensitypet.cpp \
    widgets/transfunc/transfunceditorintensityramp.cpp \
    widgets/transfunc/transfuncintensitygradientpainter.cpp \
    widgets/transfunc/transfuncmappingcanvas.cpp \
    widgets/transfunc/transfuncmappingcanvasramp.cpp \
    widgets/transfunc/transfuncplugin.cpp \
    widgets/transfunc/transfunctexturepainter.cpp
    
SOURCES += \
    ../../ext/tgt/qt/qtcanvas.cpp \
    ../../ext/tgt/qt/qttimer.cpp

##############
# Headers
##############
    
HEADERS += \
    ../../include/voreen/qt/aboutbox.h \
    ../../include/voreen/qt/helpbrowser.h \
    ../../include/voreen/qt/ioprogressdialog.h \
    ../../include/voreen/qt/pyvoreenqt.h \
    ../../include/voreen/qt/qdebug.h \
    ../../include/voreen/qt/versionqt.h \
    ../../include/voreen/qt/voreenapplicationqt.h \
    ../../include/voreen/qt/voreenqtglobal.h
HEADERS += \
    ../../include/voreen/qt/widgets/animationplugin.h \
    ../../include/voreen/qt/widgets/choicelistcombobox.h \
    ../../include/voreen/qt/widgets/consoleplugin.h \
    ../../include/voreen/qt/widgets/eventpropertywidget.h \
    ../../include/voreen/qt/widgets/expandableheaderbutton.h \
    ../../include/voreen/qt/widgets/glslhighlighter.h \
    ../../include/voreen/qt/widgets/keydetectorwidget.h \
    ../../include/voreen/qt/widgets/labelingwidgetqt.h \
    ../../include/voreen/qt/widgets/plugindialog.h \
    ../../include/voreen/qt/widgets/qlabelclickable.h \
    ../../include/voreen/qt/widgets/lineeditresetwidget.h \
    ../../include/voreen/qt/widgets/rawvolumewidget.h \
    ../../include/voreen/qt/widgets/segmentationplugin.h \
    ../../include/voreen/qt/widgets/shaderplugin.h \
    ../../include/voreen/qt/widgets/shortcutpreferenceswidget.h \
    ../../include/voreen/qt/widgets/rendertargetdebugwidget.h \
    ../../include/voreen/qt/widgets/sliderspinboxwidget.h \
    ../../include/voreen/qt/widgets/snapshotplugin.h \
    ../../include/voreen/qt/widgets/tablesstextedit.h \
    ../../include/voreen/qt/widgets/volumecontainerwidget.h \
    ../../include/voreen/qt/widgets/volumeviewhelper.h \
    ../../include/voreen/qt/widgets/voreentoolwindow.h \
    ../../include/voreen/qt/widgets/widgetplugin.h
HEADERS += \
    ../../include/voreen/qt/widgets/property/boolpropertywidget.h \
    ../../include/voreen/qt/widgets/property/buttonpropertywidget.h \
    ../../include/voreen/qt/widgets/property/camerapropertywidget.h \
    ../../include/voreen/qt/widgets/property/colorpropertywidget.h \
    ../../include/voreen/qt/widgets/property/filedialogpropertywidget.h \
    ../../include/voreen/qt/widgets/property/floatpropertywidget.h \
    ../../include/voreen/qt/widgets/property/floatvec2propertywidget.h \
    ../../include/voreen/qt/widgets/property/floatvec3propertywidget.h \
    ../../include/voreen/qt/widgets/property/floatvec4propertywidget.h \
    ../../include/voreen/qt/widgets/property/intpropertywidget.h \
    ../../include/voreen/qt/widgets/property/intvec2propertywidget.h \
    ../../include/voreen/qt/widgets/property/intvec3propertywidget.h \
    ../../include/voreen/qt/widgets/property/intvec4propertywidget.h \
    ../../include/voreen/qt/widgets/property/optionpropertywidget.h \
    ../../include/voreen/qt/widgets/property/propertyvectorwidget.h \
    ../../include/voreen/qt/widgets/property/processorpropertieswidget.h \
    ../../include/voreen/qt/widgets/property/qpropertywidget.h \
    ../../include/voreen/qt/widgets/property/qpropertywidgetfactory.h \
    ../../include/voreen/qt/widgets/property/shaderpropertywidget.h \
    ../../include/voreen/qt/widgets/property/stringpropertywidget.h \
    ../../include/voreen/qt/widgets/property/transfuncpropertywidget.h \
	../../include/voreen/qt/widgets/property/vecpropertywidget.h \
    ../../include/voreen/qt/widgets/property/volumecollectionpropertywidget.h \
    ../../include/voreen/qt/widgets/property/volumehandlepropertywidget.h 
HEADERS += \
    ../../include/voreen/qt/widgets/network/processorlistwidget.h \
    ../../include/voreen/qt/widgets/network/propertylistwidget.h \
    ../../include/voreen/qt/widgets/network/editor/arrowgraphicsitem.h \
    ../../include/voreen/qt/widgets/network/editor/linkarrowgraphicsitem.h \
    ../../include/voreen/qt/widgets/network/editor/portgraphicsitem.h \
    ../../include/voreen/qt/widgets/network/editor/networkeditor.h \
    ../../include/voreen/qt/widgets/network/editor/openpropertylistbutton.h \
    ../../include/voreen/qt/widgets/network/editor/processorgraphicsitem.h \
    ../../include/voreen/qt/widgets/network/editor/propertylistgraphicsitem.h \
    ../../include/voreen/qt/widgets/network/editor/tctooltip.h \
    ../../include/voreen/qt/widgets/network/editor/textgraphicsitem.h \
    ../../include/voreen/qt/widgets/network/editor/tooltiptimer.h \
    ../../include/voreen/qt/widgets/network/editor/propertylinkdialog.h \
    ../../include/voreen/qt/widgets/network/editor/scripteditor.h \
    ../../include/voreen/qt/widgets/network/editor/itooltip.h \
    ../../include/voreen/qt/widgets/network/editor/propertygraphicsitem.h
HEADERS += \
    ../../include/voreen/qt/widgets/processor/qprocessorwidget.h \
    ../../include/voreen/qt/widgets/processor/qprocessorwidgetfactory.h \       
    ../../include/voreen/qt/widgets/processor/canvasrendererwidget.h
HEADERS += \
    ../../include/voreen/qt/widgets/property/camerawidget.h
HEADERS += \
    ../../include/voreen/qt/widgets/transfunc/colorluminancepicker.h \
    ../../include/voreen/qt/widgets/transfunc/colorpicker.h \
    ../../include/voreen/qt/widgets/transfunc/doubleslider.h \
    ../../include/voreen/qt/widgets/transfunc/histogrampainter.h \
    ../../include/voreen/qt/widgets/transfunc/transfunceditor.h \
    ../../include/voreen/qt/widgets/transfunc/transfunceditorintensity.h \
    ../../include/voreen/qt/widgets/transfunc/transfunceditorintensitygradient.h \
    ../../include/voreen/qt/widgets/transfunc/transfunceditorintensitypet.h \
    ../../include/voreen/qt/widgets/transfunc/transfunceditorintensityramp.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncintensitygradientpainter.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncmappingcanvas.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncmappingcanvasramp.h \
    ../../include/voreen/qt/widgets/transfunc/transfuncplugin.h \
    ../../include/voreen/qt/widgets/transfunc/transfunctexturepainter.h

HEADERS += \
    ../../ext/tgt/qt/qtcanvas.h \
    ../../ext/tgt/qt/qttimer.h

MSVC_IDE: SOURCES += \
    ../core/vis/glsl/stc_showdepth.frag \
    ../core/vis/glsl/stc_showfloatcolor.frag \
    ../core/vis/glsl/stc_showtexture.frag

contains(DEFINES, VRN_WITH_DCMTK) {
  HEADERS += ../../include/voreen/qt/dicomdialog.h
  SOURCES += dicomdialog.cpp
}



FORMS += aboutbox.ui scripteditor.ui


# this must come after all SOURCES, HEADERS and FORMS have been added
contains(DEFINES, VRN_WITH_SVNVERSION) : revtarget.depends = $$SOURCES $$HEADERS $$FORMS

# installation
unix {
  !isEmpty(INSTALL_PREFIX) {
    target.path = $$INSTALLPATH_LIB
    INSTALLS += target
  }
}

### Local Variables:
### mode:conf-unix
### End:
