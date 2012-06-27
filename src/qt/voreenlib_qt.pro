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

# Check Qt version:
# Use a regex that matches all invalid version numbers, i.e, X.*.*
# with X <= 3 and 4.Y.* with Y <= 2.
VERSION_CHECK = $$find(QT_VERSION, "^([123]|4\.[12])\..*$")
!isEmpty(VERSION_CHECK) {
   error("Your Qt version is $$QT_VERSION but at least 4.3.0 is required!")
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
##############
# Sources
##############
SOURCES += \
    aboutbox.cpp \
    applicationqt.cpp \
    datasetserver.cpp \
    helpbrowser.cpp \
    ioprogressdialog.cpp \
    opennetworkfiledialog.cpp \
    pyvoreenqt.cpp \
    versionqt.cpp \
    voreenapp.cpp \
    voreenmainframe.cpp
SOURCES += \
    widgets/animationplugin.cpp \
    widgets/backgroundplugin.cpp \
    widgets/canvasmodifier.cpp \
    widgets/choicelistcombobox.cpp \
    widgets/clipperwidget.cpp \
    widgets/clippingplugin.cpp \
    widgets/consoleplugin.cpp \
    widgets/dockbarhandler.cpp \
    widgets/dynamicsplugin.cpp \
    widgets/expandableheaderbutton.cpp \
    widgets/informationplugin.cpp \
    widgets/labelingwidgetqt.cpp \
    widgets/lightmaterialplugin.cpp \
    widgets/orientationplugin.cpp \
    widgets/pickingplugin.cpp\
    widgets/plugindialog.cpp \
    widgets/processorpropertieswidget.cpp \
    widgets/propertylistwidget.cpp \
    widgets/qlabelclickable.cpp \
    widgets/qpropertywidget.cpp \
    widgets/renderobjectsplugin.cpp \
    widgets/segmentationplugin.cpp \
    widgets/showtexcontainerwidget.cpp \
    widgets/sliderspinboxwidget.cpp \
    widgets/snapshotplugin.cpp \
    widgets/stereoplugin.cpp \
    widgets/thresholdwidget.cpp \
    widgets/volumesetwidget.cpp \
    widgets/voreenbutton.cpp \
    widgets/voreentoolwindow.cpp \
    widgets/voreentoolbar.cpp \
    widgets/widgetplugin.cpp \
    widgets/widgetgenerator.cpp \
    widgets/widgetgeneratorplugins.cpp
SOURCES += \
    widgets/compactproperty/compactboolpropertywidget.cpp \
    widgets/compactproperty/compactcolorpropertywidget.cpp \
    widgets/compactproperty/compactenumpropertywidget.cpp \
    widgets/compactproperty/compactfiledialogpropertywidget.cpp \
    widgets/compactproperty/compactfloatpropertywidget.cpp \
    widgets/compactproperty/compactfloatvec2propertywidget.cpp \
    widgets/compactproperty/compactfloatvec3propertywidget.cpp \
    widgets/compactproperty/compactfloatvec4propertywidget.cpp \
    widgets/compactproperty/compactintpropertywidget.cpp \
    widgets/compactproperty/compactintvec2propertywidget.cpp \
    widgets/compactproperty/compactintvec3propertywidget.cpp \
    widgets/compactproperty/compactintvec4propertywidget.cpp \
    widgets/compactproperty/compactoptionpropertywidget.cpp \
    widgets/compactproperty/compactpropertywidget.cpp \
    widgets/compactproperty/compactpropertywidgetfactory.cpp \
    widgets/compactproperty/compactstringpropertywidget.cpp \
    widgets/compactproperty/compactstringselectionpropertywidget.cpp \
    widgets/compactproperty/compactstringvectorpropertywidget.cpp \
    widgets/compactproperty/compacttransfuncpropertywidget.cpp    
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
    ../../include/voreen/qt/applicationqt.h \
    ../../include/voreen/qt/datasetserver.h \
    ../../include/voreen/qt/helpbrowser.h \
    ../../include/voreen/qt/ioprogressdialog.h \
    ../../include/voreen/qt/opennetworkfiledialog.h \
    ../../include/voreen/qt/pyvoreenqt.h \
    ../../include/voreen/qt/qdebug.h \
    ../../include/voreen/qt/versionqt.h \
    ../../include/voreen/qt/voreenapp.h \
    ../../include/voreen/qt/voreenmainframe.h
HEADERS += \
    ../../include/voreen/qt/widgets/animationplugin.h \
    ../../include/voreen/qt/widgets/backgroundplugin.h \
    ../../include/voreen/qt/widgets/canvasmodifier.h \
    ../../include/voreen/qt/widgets/choicelistcombobox.h \
    ../../include/voreen/qt/widgets/clipperwidget.h \
    ../../include/voreen/qt/widgets/clippingplugin.h \
    ../../include/voreen/qt/widgets/consoleplugin.h \
    ../../include/voreen/qt/widgets/dockbarhandler.h \
    ../../include/voreen/qt/widgets/dynamicsplugin.h \
    ../../include/voreen/qt/widgets/expandableheaderbutton.h \
    ../../include/voreen/qt/widgets/informationplugin.h \
    ../../include/voreen/qt/widgets/labelingwidgetqt.h \
    ../../include/voreen/qt/widgets/lightmaterialplugin.h \
    ../../include/voreen/qt/widgets/orientationplugin.h \
    ../../include/voreen/qt/widgets/pickingplugin.h\
    ../../include/voreen/qt/widgets/plugindialog.h \
    ../../include/voreen/qt/widgets/processorpropertieswidget.h \
    ../../include/voreen/qt/widgets/propertylistwidget.h \
    ../../include/voreen/qt/widgets/qlabelclickable.h \
    ../../include/voreen/qt/widgets/qpropertywidget.h \
    ../../include/voreen/qt/widgets/renderobjectsplugin.h \
    ../../include/voreen/qt/widgets/segmentationplugin.h \
    ../../include/voreen/qt/widgets/showtexcontainerwidget.h \
    ../../include/voreen/qt/widgets/sliderspinboxwidget.h \
    ../../include/voreen/qt/widgets/snapshotplugin.h \
    ../../include/voreen/qt/widgets/stereoplugin.h \
    ../../include/voreen/qt/widgets/thresholdwidget.h \
    ../../include/voreen/qt/widgets/volumesetwidget.h \
    ../../include/voreen/qt/widgets/voreenbutton.h \
    ../../include/voreen/qt/widgets/voreentoolwindow.h \
    ../../include/voreen/qt/widgets/voreentoolbar.h \
    ../../include/voreen/qt/widgets/widgetgenerator.h \
    ../../include/voreen/qt/widgets/widgetgeneratorplugins.h \
    ../../include/voreen/qt/widgets/widgetplugin.h
HEADERS += \
    ../../include/voreen/qt/widgets/compactproperty/compactboolpropertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactcolorpropertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactenumpropertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactfiledialogpropertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactfloatpropertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactfloatvec2propertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactfloatvec3propertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactfloatvec4propertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactintpropertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactintvec2propertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactintvec3propertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactintvec4propertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactoptionpropertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactpropertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactpropertywidgetfactory.h \
    ../../include/voreen/qt/widgets/compactproperty/compactstringpropertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactstringselectionpropertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactstringvectorpropertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compacttransfuncpropertywidget.h \
    ../../include/voreen/qt/widgets/compactproperty/compactvectorpropertywidget.h
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

win32 {
    contains(DEFINES, VRN_WITH_QWT) {
        INCLUDEPATH += $${VRN_HOME}/ext/qwt/include
        DEFINES += QWT_DLL
    }
}

FORMS = aboutbox.ui


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
