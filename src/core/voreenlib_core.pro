####################################################
# Project file for the Voreen-Core library
####################################################
TEMPLATE = lib
TARGET = voreen_core
VERSION = 1.0

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

# Include tgt
#define TGT_COMPILED_FOR_VOREEN
include(../../tgt.pro)

# Include modules which are selected in local configuration. The entry
# 'foo' in VRN_MODULES must correspond to a subdir 'modules/foo' and a
# file 'foo_core.pri' there.
for(i, VRN_MODULES) : include(../modules/$${i}/$${i}_core.pri)

PRECOMPILED_HEADER = ../../pch.h

# please insert new files in alphabetically order!
SOURCES += \
	geometry/geometrycontainer.cpp
SOURCES += \
    io/datvolumereader.cpp \
    io/datvolumewriter.cpp \
    io/interfilevolumereader.cpp \
    io/ioprogress.cpp \
    io/nrrdvolumereader.cpp \
    io/nrrdvolumewriter.cpp \
    io/quadhidacvolumereader.cpp \
    io/rawvolumereader.cpp \
    io/siemensreader.cpp \
    io/textfilereader.cpp \
    io/tuvvolumereader.cpp \
    io/vevovolumereader.cpp \
    io/visiblehumanreader.cpp \
    io/volumeserializer.cpp \
    io/volumeserializerpopulator.cpp \
    io/volumereader.cpp \
    io/volumewriter.cpp
SOURCES += \
    opengl/texturecontainer.cpp \
    opengl/texunitmapper.cpp
SOURCES += \
    vis/processors/networkevaluator.cpp \
    vis/processors/networkserializer.cpp \
    vis/processors/port.cpp \
    vis/processors/portmapping.cpp \
    vis/processors/processor.cpp \
    vis/processors/processorfactory.cpp \
    vis/processors/propertyset.cpp \
    vis/processors/volumeselectionprocessor.cpp \
    vis/processors/volumesetsourceprocessor.cpp \
    vis/processors/image/background.cpp \
    vis/processors/image/coarsenessrenderer.cpp \
    vis/processors/image/canvasrenderer.cpp \
    vis/processors/image/copytoscreenrenderer.cpp \
    vis/processors/image/geometryprocessor.cpp \
    vis/processors/image/labeling.cpp \
    vis/processors/image/labelingmath.cpp \
    vis/processors/image/postprocessor.cpp \
    vis/processors/image/blur.cpp \
    vis/processors/image/regionmodifier.cpp \
    vis/processors/image/collect.cpp \
    vis/processors/image/colordepth.cpp \
    vis/processors/image/combine.cpp \
    vis/processors/image/depthmask.cpp \
    vis/processors/image/depthoffield.cpp \
    vis/processors/image/edgedetect.cpp \
    vis/processors/image/genericfragment.cpp \
    vis/processors/image/merge.cpp \
    vis/processors/image/threshold.cpp \
    vis/processors/render/entryexitpoints.cpp \
    vis/processors/render/idraycaster.cpp \
    vis/processors/render/proxygeometry.cpp \
    vis/processors/render/simpleraycaster.cpp \
    vis/processors/render/slicerenderer.cpp \
    vis/processors/render/slicingproxygeometry.cpp \
    vis/processors/render/volumeraycaster.cpp \
    vis/processors/render/volumerenderer.cpp \
    vis/processors/volume/datasetcreator.cpp \
    vis/processors/volume/visiblehumandatasetcreator.cpp
SOURCES += \
    vis/transfunc/transfunc.cpp \
	vis/transfunc/transfunceditor.cpp \
    vis/transfunc/transfuncintensitygradient.cpp \
    vis/transfunc/transfuncintensitykeys.cpp \
    vis/transfunc/transfuncmappingkey.cpp \
    vis/transfunc/transfuncpainter.cpp
SOURCES += \
    vis/geomclippingwidget.cpp \
    vis/identifier.cpp \
    vis/idmanager.cpp \
    vis/lightmaterial.cpp \
    vis/message.cpp \
    vis/messagedistributor.cpp \
    vis/networkanalyzer.cpp \
    vis/property.cpp \
    vis/trackballnavigation.cpp \
    vis/voreenpainter.cpp
SOURCES += \
	vis/slicing/edge3d.cpp \
	vis/slicing/edgevertex3d.cpp \
	vis/slicing/polygon3d.cpp \
	vis/slicing/polygonface3d.cpp
SOURCES += \
    volume/modality.cpp \
    volume/volume.cpp \
    volume/volumecontainer.cpp \
    volume/volumegl.cpp \
    volume/volumehandle.cpp \
    volume/volumemetadata.cpp \
    volume/volumeseries.cpp \
    volume/volumeset.cpp \
    volume/volumesetcontainer.cpp \
    volume/volumetexture.cpp \
    volume/gradient.cpp \
    volume/histogram.cpp
SOURCES += \
    xml/serializable.cpp
SOURCES += \
    ../../ext/tinyxml/tinyxml.cpp \
    ../../ext/tinyxml/tinystr.cpp \
    ../../ext/tinyxml/tinyxmlerror.cpp \
    ../../ext/tinyxml/tinyxmlparser.cpp

win32 {
    SOURCES += opengl/gpucapabilitieswindows.cpp
    MSC_VER = $$find( QMAKE_COMPILER_DEFINES, "_MSC_VER")
    !isEmpty(MSC_VER) {
        SOURCES += \
            vis/glsl/eep_clipping.frag \
            vis/glsl/eep_jitter.frag \
            vis/glsl/eep_simple.frag \
            vis/glsl/eep_simple.vert \
            vis/glsl/eep_texcoord.vert \
            vis/glsl/eep_vertex.vert \
            vis/glsl/rc_hitpoints.frag \
            vis/glsl/rc_id.frag \
            vis/glsl/rc_simple.frag \
            vis/glsl/pp_blur.frag \
            vis/glsl/pp_regionmodifier.frag \
            vis/glsl/pp_colordepth.frag \
            vis/glsl/pp_combine.frag \
            vis/glsl/pp_depthmask.frag \
            vis/glsl/pp_depthoffield.frag \
            vis/glsl/pp_edgedetect.frag \
            vis/glsl/pp_identity.vert \
            vis/glsl/pp_labeling.frag \
            vis/glsl/pp_labeling.vert \
            vis/glsl/pp_merge.frag \
            vis/glsl/pp_threshold.frag \
            vis/glsl/pp_widget.frag \
            vis/glsl/stc_showdepth.frag \
            vis/glsl/stc_showfloatcolor.frag \
            vis/glsl/stc_showtexture.frag \
            vis/glsl/sr_transfunc.frag \
            vis/glsl/sr_transfunc.vert \
            vis/glsl/vrn_rendertexture.frag \
            vis/glsl/vrn_interactionmode.frag \
            vis/glsl/modules/mod_depth.frag \
            vis/glsl/modules/mod_gradients.frag \
            vis/glsl/modules/mod_raysetup.frag \
            vis/glsl/modules/mod_firsthit.frag \
            vis/glsl/modules/mod_segmentation.frag \
            vis/glsl/modules/mod_shading.frag \
            vis/glsl/modules/mod_shadowing.frag \
            vis/glsl/modules/mod_sketch.frag \
            vis/glsl/modules/mod_transfunc.frag \
            vis/glsl/modules/mod_sampler3D.frag \
            vis/glsl/modules/mod_sampler2D.frag
    }
}

HEADERS += \
    ../../include/voreen/core/geometry/geometry.h \
    ../../include/voreen/core/geometry/geometrycontainer.h \
    ../../include/voreen/core/geometry/pointlistgeometry.h \
    ../../include/voreen/core/geometry/tgtvec3pointlistgeometry.h
HEADERS += \
    ../../include/voreen/core/io/datvolumereader.h \
    ../../include/voreen/core/io/datvolumewriter.h \
    ../../include/voreen/core/io/interfilevolumereader.h \
    ../../include/voreen/core/io/ioprogress.h \
    ../../include/voreen/core/io/nrrdvolumereader.h \
    ../../include/voreen/core/io/nrrdvolumewriter.h \
    ../../include/voreen/core/io/quadhidacvolumereader.h \
    ../../include/voreen/core/io/rawvolumereader.h \
    ../../include/voreen/core/io/siemensreader.h \
    ../../include/voreen/core/io/textfilereader.h \
    ../../include/voreen/core/io/tuvvolumereader.h \
    ../../include/voreen/core/io/vevovolumereader.h \
    ../../include/voreen/core/io/visiblehumanreader.h \
    ../../include/voreen/core/io/volumeserializer.h \
    ../../include/voreen/core/io/volumeserializerpopulator.h \
    ../../include/voreen/core/io/volumereader.h \
    ../../include/voreen/core/io/volumewriter.h
HEADERS += \
    ../../include/voreen/core/opengl/texturecontainer.h \
    ../../include/voreen/core/opengl/texunitmapper.h
HEADERS += \
    ../../include/voreen/core/vis/processors/networkevaluator.h\
    ../../include/voreen/core/vis/processors/networkserializer.h\
    ../../include/voreen/core/vis/processors/port.h \
    ../../include/voreen/core/vis/processors/portmapping.h \
    ../../include/voreen/core/vis/processors/processor.h \
    ../../include/voreen/core/vis/processors/processorfactory.h \
    ../../include/voreen/core/vis/processors/propertyset.h \
    ../../include/voreen/core/vis/processors/volumeselectionprocessor.h \
    ../../include/voreen/core/vis/processors/volumesetsourceprocessor.h \
    ../../include/voreen/core/vis/processors/image/background.h \
    ../../include/voreen/core/vis/processors/image/blur.h \
    ../../include/voreen/core/vis/processors/image/canvasrenderer.h \
    ../../include/voreen/core/vis/processors/image/coarsenessrenderer.h \
    ../../include/voreen/core/vis/processors/image/collect.h \
    ../../include/voreen/core/vis/processors/image/colordepth.h \
    ../../include/voreen/core/vis/processors/image/combine.h \
    ../../include/voreen/core/vis/processors/image/copytoscreenrenderer.h \
    ../../include/voreen/core/vis/processors/image/depthmask.h \
    ../../include/voreen/core/vis/processors/image/depthoffield.h \
    ../../include/voreen/core/vis/processors/image/edgedetect.h \
    ../../include/voreen/core/vis/processors/image/genericfragment.h \
    ../../include/voreen/core/vis/processors/image/geometryprocessor.h \
    ../../include/voreen/core/vis/processors/image/labeling.h \
    ../../include/voreen/core/vis/processors/image/labelingmath.h \
    ../../include/voreen/core/vis/processors/image/merge.h \
    ../../include/voreen/core/vis/processors/image/postprocessor.h \
    ../../include/voreen/core/vis/processors/image/postprocessor.h \
    ../../include/voreen/core/vis/processors/image/regionmodifier.h \
    ../../include/voreen/core/vis/processors/image/threshold.h \
    ../../include/voreen/core/vis/processors/render/entryexitpoints.h \
    ../../include/voreen/core/vis/processors/render/idraycaster.h \
    ../../include/voreen/core/vis/processors/render/proxygeometry.h \
    ../../include/voreen/core/vis/processors/render/simpleraycaster.h \
    ../../include/voreen/core/vis/processors/render/slicerenderer.h \
    ../../include/voreen/core/vis/processors/render/slicingproxygeometry.h \
    ../../include/voreen/core/vis/processors/render/volumeraycaster.h \
    ../../include/voreen/core/vis/processors/render/volumerenderer.h \
    ../../include/voreen/core/vis/processors/volume/datasetcreator.h \
    ../../include/voreen/core/vis/processors/volume/visiblehumandatasetcreator.h
HEADERS += \
    ../../include/voreen/core/vis/transfunc/transfunc.h \
	../../include/voreen/core/vis/transfunc/transfunceditor.h \
    ../../include/voreen/core/vis/transfunc/transfuncmappingkey.h \
    ../../include/voreen/core/vis/transfunc/transfuncintensitygradient.h \
    ../../include/voreen/core/vis/transfunc/transfuncintensitykeys.h \
    ../../include/voreen/core/vis/transfunc/transfuncpainter.h
HEADERS += \
    ../../include/voreen/core/vis/exception.h \
    ../../include/voreen/core/vis/geomclippingwidget.h \
    ../../include/voreen/core/vis/identifier.h \
    ../../include/voreen/core/vis/idmanager.h \
    ../../include/voreen/core/vis/lightmaterial.h \
    ../../include/voreen/core/vis/message.h \
    ../../include/voreen/core/vis/messagedistributor.h \
    ../../include/voreen/core/vis/networkanalyzer.h \
    ../../include/voreen/core/vis/property.h \
    ../../include/voreen/core/vis/trackballnavigation.h \
    ../../include/voreen/core/vis/voreenpainter.h
HEADERS += \
	../../include/voreen/core/vis/slicing/polygon3d.h
HEADERS += \
    ../../include/voreen/core/volume/modality.h \
    ../../include/voreen/core/volume/volume.h \
    ../../include/voreen/core/volume/volumeatomic.h \
    ../../include/voreen/core/volume/volumecontainer.h \
    ../../include/voreen/core/volume/volumeelement.h \
    ../../include/voreen/core/volume/volumefusion.h \
    ../../include/voreen/core/volume/volumegl.h \
    ../../include/voreen/core/volume/volumehandle.h \
    ../../include/voreen/core/volume/volumemetadata.h \
    ../../include/voreen/core/volume/volumeseries.h \
    ../../include/voreen/core/volume/volumeset.h \
    ../../include/voreen/core/volume/volumesetcontainer.h \
    ../../include/voreen/core/volume/volumetexture.h \
    ../../include/voreen/core/volume/gradient.h \
    ../../include/voreen/core/volume/histogram.h
HEADERS += \
    ../../include/voreen/core/xml/serializable.h
HEADERS += \
    ../../ext/tinyxml/tinyxml.h \
    ../../ext/tinyxml/tinystr.h \
    ../../ext/il/include/IL/il.h

win32 : HEADERS += ../../include/voreen/core/opengl/gpucapabilitieswindows.h

# add files, which are not available in the snapshot release
# these files will be added to the snapshot, when they are cleaned up
!contains(DEFINES, VRN_SNAPSHOT) {
SOURCES += \
    vis/borderoverlay.cpp \
    vis/fpsoverlay.cpp \
    vis/geomplane.cpp \
    vis/geomvirtualclippingwidget.cpp \
    vis/pyvoreen.cpp \
    vis/virtualclipping.cpp \
    vis/processors/ultrasoundprocessor.cpp \
    vis/processors/image/crosshair.cpp \
    vis/processors/image/glow.cpp \
    vis/processors/image/magicmirror.cpp \
    vis/processors/image/measuring.cpp \
    vis/processors/image/normalestimation.cpp \
    vis/processors/image/texturerenderer.cpp \
    vis/processors/render/aoraycaster.cpp \
    vis/processors/render/fancyraycaster.cpp \
    vis/processors/render/optimizedentryexitpoints.cpp \
    vis/processors/render/multilayerraycaster.cpp \
    vis/processors/render/petctraycaster.cpp \
    vis/processors/render/registrationraycaster.cpp \
    vis/processors/render/rgbaraycaster.cpp \
    vis/processors/render/rgbraycaster.cpp \
    vis/processors/volume/motionestimation.cpp \

HEADERS += \
    ../../include/voreen/core/vis/borderoverlay.h \
    ../../include/voreen/core/vis/fpsoverlay.h \
    ../../include/voreen/core/vis/geomplane.h \
    ../../include/voreen/core/vis/geomvirtualclippingwidget.h \
    ../../include/voreen/core/vis/pyvoreen.h \
    ../../include/voreen/core/vis/virtualclipping.h \
    ../../include/voreen/core/vis/processors/ultrasoundprocessor.h \
    ../../include/voreen/core/vis/processors/image/crosshair.h \
    ../../include/voreen/core/vis/processors/image/glow.h \
    ../../include/voreen/core/vis/processors/image/magicmirror.h \
    ../../include/voreen/core/vis/processors/image/measuring.h \
    ../../include/voreen/core/vis/processors/image/normalestimation.h \
    ../../include/voreen/core/vis/processors/image/texturerenderer.h \
    ../../include/voreen/core/vis/processors/render/aoraycaster.h \
    ../../include/voreen/core/vis/processors/render/fancyraycaster.h \
    ../../include/voreen/core/vis/processors/render/multilayerraycaster.h \
    ../../include/voreen/core/vis/processors/render/optimizedentryexitpoints.h \
    ../../include/voreen/core/vis/processors/render/petctraycaster.h \
    ../../include/voreen/core/vis/processors/render/registrationraycaster.h \
    ../../include/voreen/core/vis/processors/render/rgbaraycaster.h \
    ../../include/voreen/core/vis/processors/render/rgbraycaster.h \
    ../../include/voreen/core/vis/processors/volume/motionestimation.h \
    ../../include/voreen/core/volume/vq.h \
    
	win32 {
		MSC_VER = $$find( QMAKE_COMPILER_DEFINES, "_MSC_VER")
	    !isEmpty(MSC_VER) {
		    SOURCES += \
            vis/glsl/mipmap_minmax.frag \
            vis/glsl/rc_ambientocclusion.frag \
            vis/glsl/rc_fancy.frag \
            vis/glsl/rc_fusion.frag \
            vis/glsl/rc_mapto3dtexture.frag \
            vis/glsl/rc_multilayer.frag \
            vis/glsl/rc_rgb.frag \
            vis/glsl/rc_rgba.frag \
            vis/glsl/pp_autostereo.frag \
            vis/glsl/pp_crosshair.frag \
            vis/glsl/pp_glow.frag \
            vis/glsl/pp_normalestimation.frag \
            vis/glsl/pp_rtblender.frag \
            vis/glsl/modules/mod_normalestimation.frag \
            vis/glsl/modules/mod_lightprobe.frag
	}
    }
}


####################################################
# Modules which can be enabled/disabled by defines
####################################################

# Closeup / Overview Rendering stuff
contains(DEFINES, VRN_WITH_CLOSEUP_RENDERER) {
  SOURCES += \
    vis/closeup.cpp \
    vis/processors/render/closeuprenderer.cpp \
    vis/processors/render/overviewentryexitpoints.cpp \
    vis/processors/render/overviewraycaster.cpp \
    ../../ext/freetype_nehe/FreeType.cpp
  HEADERS += \
    ../../include/voreen/core/vis/closeup.h \
    ../../include/voreen/core/vis/processors/render/closeuprenderer.h \
    ../../include/voreen/core/vis/processors/render/overviewentryexitpoints.h \
    ../../include/voreen/core/vis/processors/render/overviewraycaster.h \
    ../../ext/freetype_nehe/FreeType.h
}

contains(DEFINES, VRN_WITH_FBO_CLASS) {
  SOURCES += \
      ../../ext/fboClass/framebufferObject.cpp \
      ../../ext/fboClass/renderbuffer.cpp
  HEADERS += \
      ../../ext/fboClass/framebufferObject.h \
      ../../ext/fboClass/renderbuffer.h
}

contains(DEFINES, VRN_WITH_RENDER_TO_TEXTURE) {
  SOURCES += \
      ../../ext/rendertotexture/rendertexture.cpp
  HEADERS += \
      ../../ext/rendertotexture/rendertexture.h
}

# DICOM reader
contains(DEFINES, VRN_WITH_DCMTK) {
  SOURCES += \
      io/dicomfindscu.cpp \
      io/dicommovescu.cpp \
      io/dicomvolumereader.cpp \
      io/dicomvolumewriter.cpp
  HEADERS += \
      ../../include/voreen/core/io/dicomfindscu.h \
      ../../include/voreen/core/io/dicommovescu.h \
      ../../include/voreen/core/io/dicomvolumereader.h \
      ../../include/voreen/core/io/dicomvolumewriter.h
}

# Matlab Reader
contains(DEFINES, VRN_WITH_MATLAB) {
  SOURCES += io/matvolumereader.cpp

  HEADERS += ../../include/voreen/core/io/matvolumereader.h
}

# PVM Reader
contains(DEFINES, VRN_WITH_PVM) {
  SOURCES += io/pvmvolumereader.cpp \
             io/ddsbase.cpp
  HEADERS += ../../include/voreen/core/io/pvmvolumereader.h
}

# TIFF Reader
contains(DEFINES, VRN_WITH_TIFF) {
  SOURCES += io/tiffvolumereader.cpp
  HEADERS += ../../include/voreen/core/io/tiffvolumereader.h
}

# ZIP reader
contains(DEFINES, VRN_WITH_ZIP) {
  SOURCES += io/zipvolumereader.cpp
  HEADERS += ../../include/voreen/core/io/zipvolumereader.h
}

unix : contains(DEFINES, VRN_WITH_ZIP) {
  LIBS += -lziparch
}

unix : contains(DEFINES, VRN_WITH_CLIBPDF) {
   LIBS += -lcpdf
}

unix : contains(DEFINES, VRN_WITH_FREETYPE) {
    LIBS += -lfreetype
}

unix : contains(DEFINES, VRN_WITH_FTGL) {
    LIBS += -lftgl
}

unix : contains(DEFINES, VRN_FFMPEG) {
  LIBS += -lavcodec
}

# uncomment this for building a stripped library
# unix:QMAKE_POST_LINK=strip $(TARGET)

# this must come after alle SOURCES, HEADERS and FORMS have been added
contains(DEFINES, VRN_WITH_SVNVERSION) : revtarget.depends = $$SOURCES $$HEADERS $$FORMS

### Local Variables:
### mode:conf-unix
### End: