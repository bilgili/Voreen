####################################################
# Project file for the Voreen-Core library
####################################################
TEMPLATE = lib
TARGET = voreen_core
VERSION = 1.0

CONFIG += static thread
CONFIG -= dll

# Include local configuration
!include(../../config.txt) {
  warning("config.txt not found! Using config-default.txt instead.")
  warning("For custom behavior, copy config-default.txt to config.txt and edit!")
  include(../../config-default.txt)
}

# Include common configuration
include(../../commonconf.txt)

unix: DESTDIR = ../..
win32: {
  CONFIG(debug, debug|release) {
    DESTDIR = ../../Debug
  }
  else {
    DESTDIR = ../../Release
  }
}

# Include tgt
include(tgt.pro)

# Include modules which are selected in local configuration. The entry
# 'foo' in VRN_MODULES must correspond to a subdir 'modules/foo' and a
# file 'foo_core.pri' there.
for(i, VRN_MODULES) : include(../modules/$${i}/$${i}_core.pri)

PRECOMPILED_HEADER = ../../pch.h

# please insert new files in alphabetically order!
SOURCES += \
    application.cpp \
    version.cpp
SOURCES += \
    cmdparser/command.cpp \
    cmdparser/command_loglevel.cpp \
    cmdparser/commandlineparser.cpp
SOURCES += \
    geometry/geometrycontainer.cpp
SOURCES += \
    io/brickedvolumereader.cpp \
    io/brickedvolumewriter.cpp \
    io/cache.cpp \
    io/cacheindex.cpp \
    io/datvolumereader.cpp \
    io/datvolumewriter.cpp \
    io/interfilevolumereader.cpp \
    io/ioprogress.cpp \
    io/multivolumereader.cpp \
    io/nrrdvolumereader.cpp \
    io/nrrdvolumewriter.cpp \
    io/quadhidacvolumereader.cpp \
    io/rawvolumereader.cpp \
    io/siemensreader.cpp \
    io/textfilereader.cpp \
    io/tuvvolumereader.cpp \
    io/vevovolumereader.cpp \
    io/visiblehumanreader.cpp \
    io/volumecache.cpp \
    io/volumeserializer.cpp \
    io/volumeserializerpopulator.cpp \
    io/volumereader.cpp \
    io/volumewriter.cpp
SOURCES += \
    opengl/texturecontainer.cpp \
    opengl/texunitmapper.cpp
SOURCES += \
    vis/processors/benchmark.cpp \
    vis/processors/networkevaluator.cpp \
	vis/processors/networkevaluatorg.cpp \
	vis/processors/networkgraph.cpp \
    vis/processors/networkserializer.cpp \
    vis/processors/port.cpp \
    vis/processors/portmapping.cpp \
    vis/processors/processor.cpp \
    vis/processors/renderprocessor.cpp \
    vis/processors/processorfactory.cpp \
    vis/processors/processornetwork.cpp \
    vis/processors/interactionhandler.cpp\
    vis/processors/volumeselectionprocessor.cpp \
    vis/processors/volumesetsourceprocessor.cpp \
    vis/processors/entryexitpoints/entryexitpoints.cpp \
    vis/processors/geometry/geometryprocessor.cpp \
    vis/processors/geometry/pointlistrenderer.cpp \
    vis/processors/image/background.cpp \
    vis/processors/image/coarsenessrenderer.cpp \
    vis/processors/image/canvasrenderer.cpp \
    vis/processors/image/labeling.cpp \
    vis/processors/image/labelingmath.cpp \
    vis/processors/image/blur.cpp \
    vis/processors/image/regionmodifier.cpp \
    vis/processors/image/cacherenderer.cpp \
    vis/processors/image/collect.cpp \
    vis/processors/image/colordepth.cpp \
    vis/processors/image/combine.cpp \
    vis/processors/image/compositer.cpp \
    vis/processors/image/depthmask.cpp \
    vis/processors/image/depthoffield.cpp \
    vis/processors/image/edgedetect.cpp \
    vis/processors/image/imageprocessor.cpp \
    vis/processors/image/imageprocessordepth.cpp \
    vis/processors/image/merge.cpp \
    vis/processors/image/nullrenderer.cpp \
    vis/processors/image/renderstore.cpp \
    vis/processors/image/threshold.cpp \
    vis/processors/image/crosshair.cpp \
    vis/processors/image/glow.cpp \
    vis/processors/proxygeometry/axialsliceproxygeometry.cpp \
    vis/processors/proxygeometry/cubecutproxygeometry.cpp \
    vis/processors/proxygeometry/cubeproxygeometry.cpp \
    vis/processors/proxygeometry/multipleaxialsliceproxygeometry.cpp \
    vis/processors/proxygeometry/proxygeometry.cpp \
    vis/processors/proxygeometry/sliceproxygeometry.cpp \
    vis/processors/proxygeometry/slicingproxygeometry.cpp \
    vis/processors/render/firsthitrenderer.cpp \
    vis/processors/render/idraycaster.cpp \
    vis/processors/render/volumeeditor.cpp \
    vis/processors/render/segmentationraycaster.cpp \
    vis/processors/render/simpleraycaster.cpp \
    vis/processors/render/singleslicerenderer.cpp \
    vis/processors/render/singlevolumeraycaster.cpp \
    vis/processors/render/sliceentrypoints.cpp \
    vis/processors/render/slicerendererbase.cpp \
    vis/processors/render/slicesequencerenderer.cpp \
    vis/processors/render/volumeraycaster.cpp \
    vis/processors/render/volumerenderer.cpp \
    vis/processors/volume/datasetcreator.cpp \
    vis/processors/volume/regiongrowing.cpp \
    vis/processors/volume/simplesegmentation.cpp \
    vis/processors/volume/volumeinversion.cpp
SOURCES += \
    vis/properties/action.cpp \
    vis/properties/boolproperty.cpp \
    vis/properties/colorproperty.cpp \
    vis/properties/condition.cpp \
    vis/properties/enumproperty.cpp \
    vis/properties/eventproperty.cpp \
    vis/properties/filedialogproperty.cpp \
    vis/properties/floatproperty.cpp \
    vis/properties/intproperty.cpp \
    vis/properties/numericproperty.cpp \
    vis/properties/optionproperty.cpp \
    vis/properties/property.cpp \
    vis/properties/propertyvector.cpp \
    vis/properties/stringproperty.cpp \
    vis/properties/stringselectionproperty.cpp \
    vis/properties/transferfuncproperty.cpp \
    vis/properties/vectorproperty.cpp
SOURCES += \
    vis/transfunc/transfunc.cpp \
    vis/transfunc/transfuncintensity.cpp \
    vis/transfunc/transfuncintensitygradient.cpp \
    vis/transfunc/transfuncmappingkey.cpp \
    vis/transfunc/transfuncprimitive.cpp
SOURCES += \
    vis/clippingplanewidget.cpp \
    vis/identifier.cpp \
    vis/idmanager.cpp \
    vis/lightmaterial.cpp \
    vis/flythroughnavigation.cpp \
    vis/message.cpp \
    vis/messagedistributor.cpp \
    vis/pyvoreen.cpp \
	vis/rendertarget.cpp \
    vis/trackballnavigation.cpp \
    vis/voreenpainter.cpp
SOURCES += \
    vis/slicing/edge3d.cpp \
    vis/slicing/edgevertex3d.cpp \
    vis/slicing/polygon3d.cpp \
    vis/slicing/polygonface3d.cpp
SOURCES += \
    volume/modality.cpp \
    volume/noisevolume.cpp \
    volume/volume.cpp \
    volume/volumegl.cpp \
    volume/volumehandle.cpp \
    volume/volumehandlevalidator.cpp \
    volume/volumemetadata.cpp \
    volume/volumeoperator.cpp \
    volume/volumeseries.cpp \
    volume/volumeset.cpp \
    volume/volumesetcontainer.cpp \
    volume/volumetexture.cpp \
    volume/gradient.cpp \
    volume/histogram.cpp \
    volume/observer.cpp
SOURCES += \
    volume/bricking/balancedbrickresolutioncalculator.cpp \
    volume/bricking/boxbrickingregion.cpp \
    volume/bricking/brick.cpp \
    volume/bricking/brickedvolume.cpp \
    volume/bricking/brickedvolumegl.cpp \
    volume/bricking/brickingregion.cpp \
    volume/bricking/brickingregionmanager.cpp \
    volume/bricking/bricklodselector.cpp \
    volume/bricking/brickresolutioncalculator.cpp \
    volume/bricking/cameralodselector.cpp \
    volume/bricking/errorlodselector.cpp \ 
    volume/bricking/largevolumemanager.cpp \
    volume/bricking/maximumbrickresolutioncalculator.cpp 
SOURCES += \
    xml/xmlserializable.cpp \
    xml/serializable.cpp
SOURCES += \
    ../../ext/tinyxml/tinyxml.cpp \
    ../../ext/tinyxml/tinystr.cpp \
    ../../ext/tinyxml/tinyxmlerror.cpp \
    ../../ext/tinyxml/tinyxmlparser.cpp

SHADER_SOURCES += \
    vis/glsl/eep_clipping.frag \
    vis/glsl/eep_jitter.frag \
    vis/glsl/eep_simple.frag \
    vis/glsl/eep_simple.vert \
    vis/glsl/eep_texcoord.vert \
    vis/glsl/eep_vertex.vert \
    vis/glsl/lightvolumegenerator.frag \
    vis/glsl/lightvolumegenerator.vert \
    vis/glsl/pp_blur.frag \
    vis/glsl/pp_colordepth.frag \
    vis/glsl/pp_combine.frag \
    vis/glsl/pp_compositer.frag \
    vis/glsl/pp_depthmask.frag \
    vis/glsl/pp_depthoffield.frag \
    vis/glsl/pp_edgedetect.frag \
    vis/glsl/pp_identity.vert \
    vis/glsl/pp_labeling.frag \
    vis/glsl/pp_labeling.vert \
    vis/glsl/pp_merge.frag \
    vis/glsl/pp_regionmodifier.frag \
    vis/glsl/pp_threshold.frag \
    vis/glsl/pp_widget.frag \
    vis/glsl/rc_firsthit.frag \
    vis/glsl/rc_hitpoints.frag \
    vis/glsl/rc_id.frag \
    vis/glsl/rc_segmentation.frag \
    vis/glsl/rc_simple.frag \
    vis/glsl/rc_singlevolume.frag \
    vis/glsl/rc_volumeeditor.frag \
    vis/glsl/sr_transfunc.frag \
    vis/glsl/sr_transfunc.vert \
    vis/glsl/stc_showdepth.frag \
    vis/glsl/stc_showfloatcolor.frag \
    vis/glsl/stc_showtexture.frag \
    vis/glsl/vrn_interactionmode.frag \
    vis/glsl/vrn_rendertexture.frag

SHADER_SOURCES_MODS += \
    vis/glsl/modules/mod_compositing.frag \
    vis/glsl/modules/mod_depth.frag \
    vis/glsl/modules/mod_firsthit.frag \
    vis/glsl/modules/mod_gradients.frag \
    vis/glsl/modules/mod_masking.frag \
    vis/glsl/modules/mod_normdepth.frag \
    vis/glsl/modules/mod_raysetup.frag \
    vis/glsl/modules/mod_sampler2d.frag \
    vis/glsl/modules/mod_sampler3d.frag \
    vis/glsl/modules/mod_segmentation.frag \
    vis/glsl/modules/mod_shading.frag \
    vis/glsl/modules/mod_sketch.frag \
    vis/glsl/modules/mod_transfunc.frag \
    vis/glsl/modules/vrn_shaderincludes.frag

SHADER_SOURCES_MODS_BRICK += \
    vis/glsl/modules/bricking/mod_adaptive_sampling.frag \
    vis/glsl/modules/bricking/mod_basics.frag \
    vis/glsl/modules/bricking/mod_bricking.frag \
    vis/glsl/modules/bricking/mod_global_variables.frag \
    vis/glsl/modules/bricking/mod_interpolation.frag \
    vis/glsl/modules/bricking/mod_lookups.frag \
    vis/glsl/modules/bricking/mod_math.frag \
    vis/glsl/modules/bricking/mod_uniforms.frag

visual_studio: SOURCES += $$SHADER_SOURCES $$SHADER_SOURCES_MODS $$SHADER_SOURCES_MODS_BRICK

HEADERS += \
    ../../include/voreen/core/application.h \
    ../../include/voreen/core/version.h
HEADERS += \
    ../../include/voreen/core/cmdparser/command.h \
    ../../include/voreen/core/cmdparser/command_loglevel.h \
    ../../include/voreen/core/cmdparser/commandlineparser.h \
    ../../include/voreen/core/cmdparser/multiplecommand.h \
    ../../include/voreen/core/cmdparser/predefinedcommands.h \
    ../../include/voreen/core/cmdparser/singlecommand.h
HEADERS += \
    ../../include/voreen/core/geometry/geometry.h \
    ../../include/voreen/core/geometry/geometrycontainer.h \
    ../../include/voreen/core/geometry/pointgeometry.h \
    ../../include/voreen/core/geometry/pointlistgeometry.h \
    ../../include/voreen/core/geometry/tgtvec3pointlistgeometry.h \
    ../../include/voreen/core/geometry/pointsegmentlistgeometry.h
HEADERS += \
    ../../include/voreen/core/io/brickedvolumereader.h \
    ../../include/voreen/core/io/brickedvolumewriter.h \
    ../../include/voreen/core/io/cache.h \
    ../../include/voreen/core/io/cacheindex.h \
    ../../include/voreen/core/io/datvolumereader.h \
    ../../include/voreen/core/io/datvolumewriter.h \
    ../../include/voreen/core/io/interfilevolumereader.h \
    ../../include/voreen/core/io/ioprogress.h \
    ../../include/voreen/core/io/multivolumereader.h \
    ../../include/voreen/core/io/nrrdvolumereader.h \
    ../../include/voreen/core/io/nrrdvolumewriter.h \
    ../../include/voreen/core/io/quadhidacvolumereader.h \
    ../../include/voreen/core/io/rawvolumereader.h \
    ../../include/voreen/core/io/siemensreader.h \
    ../../include/voreen/core/io/textfilereader.h \
    ../../include/voreen/core/io/tuvvolumereader.h \
    ../../include/voreen/core/io/vevovolumereader.h \
    ../../include/voreen/core/io/visiblehumanreader.h \
    ../../include/voreen/core/io/volumecache.h \
    ../../include/voreen/core/io/volumeserializer.h \
    ../../include/voreen/core/io/volumeserializerpopulator.h \
    ../../include/voreen/core/io/volumereader.h \
    ../../include/voreen/core/io/volumewriter.h
HEADERS += \
    ../../include/voreen/core/opengl/texturecontainer.h \
    ../../include/voreen/core/opengl/texunitmapper.h
HEADERS += \
    ../../include/voreen/core/vis/processors/benchmark.h \
	../../include/voreen/core/vis/processors/graphvisitor.h \
    ../../include/voreen/core/vis/processors/networkevaluator.h \
	../../include/voreen/core/vis/processors/networkevaluatorg.h \
	../../include/voreen/core/vis/processors/networkgraph.h \
    ../../include/voreen/core/vis/processors/networkserializer.h \
    ../../include/voreen/core/vis/processors/port.h \
    ../../include/voreen/core/vis/processors/portmapping.h \
    ../../include/voreen/core/vis/processors/processor.h \
    ../../include/voreen/core/vis/processors/renderprocessor.h \
    ../../include/voreen/core/vis/processors/processorfactory.h \
    ../../include/voreen/core/vis/processors/processornetwork.h\
    ../../include/voreen/core/vis/processors/interactionhandler.h\
    ../../include/voreen/core/vis/processors/volumeselectionprocessor.h \
    ../../include/voreen/core/vis/processors/volumesetsourceprocessor.h \
    ../../include/voreen/core/vis/processors/entryexitpoints/entryexitpoints.h \
    ../../include/voreen/core/vis/processors/geometry/geometryprocessor.h \
    ../../include/voreen/core/vis/processors/geometry/pointlistrenderer.h \
    ../../include/voreen/core/vis/processors/image/background.h \
    ../../include/voreen/core/vis/processors/image/blur.h \
    ../../include/voreen/core/vis/processors/image/cacherenderer.h \
    ../../include/voreen/core/vis/processors/image/canvasrenderer.h \
    ../../include/voreen/core/vis/processors/image/coarsenessrenderer.h \
    ../../include/voreen/core/vis/processors/image/collect.h \
    ../../include/voreen/core/vis/processors/image/colordepth.h \
    ../../include/voreen/core/vis/processors/image/combine.h \
    ../../include/voreen/core/vis/processors/image/compositer.h \
    ../../include/voreen/core/vis/processors/image/depthmask.h \
    ../../include/voreen/core/vis/processors/image/depthoffield.h \
    ../../include/voreen/core/vis/processors/image/edgedetect.h \
    ../../include/voreen/core/vis/processors/image/imageprocessor.h \
    ../../include/voreen/core/vis/processors/image/imageprocessordepth.h \
    ../../include/voreen/core/vis/processors/image/labeling.h \
    ../../include/voreen/core/vis/processors/image/labelingmath.h \
    ../../include/voreen/core/vis/processors/image/merge.h \
    ../../include/voreen/core/vis/processors/image/nullrenderer.h \
    ../../include/voreen/core/vis/processors/image/regionmodifier.h \
    ../../include/voreen/core/vis/processors/image/renderstore.h \
    ../../include/voreen/core/vis/processors/image/threshold.h \
    ../../include/voreen/core/vis/processors/image/crosshair.h \
    ../../include/voreen/core/vis/processors/image/glow.h \
    ../../include/voreen/core/vis/processors/proxygeometry/axialsliceproxygeometry.h \
    ../../include/voreen/core/vis/processors/proxygeometry/cubecutproxygeometry.h \
    ../../include/voreen/core/vis/processors/proxygeometry/cubeproxygeometry.h \
    ../../include/voreen/core/vis/processors/proxygeometry/multipleaxialsliceproxygeometry.h \
    ../../include/voreen/core/vis/processors/proxygeometry/proxygeometry.h \
    ../../include/voreen/core/vis/processors/proxygeometry/sliceproxygeometry.h \
    ../../include/voreen/core/vis/processors/proxygeometry/slicingproxygeometry.h \
    ../../include/voreen/core/vis/processors/render/firsthitrenderer.h \
    ../../include/voreen/core/vis/processors/render/idraycaster.h \
    ../../include/voreen/core/vis/processors/render/volumeeditor.h \
    ../../include/voreen/core/vis/processors/render/segmentationraycaster.h \
    ../../include/voreen/core/vis/processors/render/simpleraycaster.h \
    ../../include/voreen/core/vis/processors/render/singleslicerenderer.h \
    ../../include/voreen/core/vis/processors/render/singlevolumeraycaster.h \
    ../../include/voreen/core/vis/processors/render/sliceentrypoints.h \
    ../../include/voreen/core/vis/processors/render/slicerendererbase.h \
    ../../include/voreen/core/vis/processors/render/slicesequencerenderer.h \
    ../../include/voreen/core/vis/processors/render/volumeraycaster.h \
    ../../include/voreen/core/vis/processors/render/volumerenderer.h \
    ../../include/voreen/core/vis/processors/volume/datasetcreator.h \
    ../../include/voreen/core/vis/processors/volume/regiongrowing.h \
    ../../include/voreen/core/vis/processors/volume/simplesegmentation.h \
    ../../include/voreen/core/vis/processors/volume/volumeinversion.h
HEADERS += \
	../../include/voreen/core/vis/properties/action.h \
	../../include/voreen/core/vis/properties/allproperties.h \
	../../include/voreen/core/vis/properties/boolproperty.h \
	../../include/voreen/core/vis/properties/colorproperty.h \
	../../include/voreen/core/vis/properties/condition.h \
	../../include/voreen/core/vis/properties/enumproperty.h \
    ../../include/voreen/core/vis/properties/eventproperty.h \
	../../include/voreen/core/vis/properties/filedialogproperty.h \
	../../include/voreen/core/vis/properties/floatproperty.h \
	../../include/voreen/core/vis/properties/intproperty.h \
	../../include/voreen/core/vis/properties/numericproperty.h \
	../../include/voreen/core/vis/properties/optionproperty.h \
	../../include/voreen/core/vis/properties/properties_decl.h \
	../../include/voreen/core/vis/properties/property.h \
	../../include/voreen/core/vis/properties/propertyvector.h \
	../../include/voreen/core/vis/properties/stringproperty.h \
	../../include/voreen/core/vis/properties/stringselectionproperty.h \
	../../include/voreen/core/vis/properties/templateproperty.h \
	../../include/voreen/core/vis/properties/templatepropertycondition.h \
	../../include/voreen/core/vis/properties/transferfuncproperty.h \
	../../include/voreen/core/vis/properties/vectorproperty.h
HEADERS += \
    ../../include/voreen/core/vis/transfunc/transfunc.h \
    ../../include/voreen/core/vis/transfunc/transfuncintensity.h \
    ../../include/voreen/core/vis/transfunc/transfuncintensitygradient.h \
    ../../include/voreen/core/vis/transfunc/transfuncmappingkey.h \
    ../../include/voreen/core/vis/transfunc/transfuncprimitive.h
HEADERS += \
    ../../include/voreen/core/vis/exception.h \
    ../../include/voreen/core/vis/clippingplanewidget.h \
    ../../include/voreen/core/vis/identifier.h \
    ../../include/voreen/core/vis/idmanager.h \
    ../../include/voreen/core/vis/flythroughnavigation.h \
    ../../include/voreen/core/vis/lightmaterial.h \
    ../../include/voreen/core/vis/message.h \
    ../../include/voreen/core/vis/messagedistributor.h \
    ../../include/voreen/core/vis/propertywidget.h \
    ../../include/voreen/core/vis/propertywidgetfactory.h \
    ../../include/voreen/core/vis/pyvoreen.h \
	../../include/voreen/core/vis/rendertarget.h \
    ../../include/voreen/core/vis/trackballnavigation.h \
    ../../include/voreen/core/vis/voreenpainter.h
HEADERS += \
    ../../include/voreen/core/vis/slicing/polygon3d.h
HEADERS += \
    ../../include/voreen/core/volume/modality.h \
    ../../include/voreen/core/volume/noisevolume.h \
    ../../include/voreen/core/volume/volume.h \
    ../../include/voreen/core/volume/volumeatomic.h \
    ../../include/voreen/core/volume/volumeelement.h \
    ../../include/voreen/core/volume/volumefusion.h \
    ../../include/voreen/core/volume/volumegl.h \
    ../../include/voreen/core/volume/volumehandle.h \
    ../../include/voreen/core/volume/volumehandlevalidator.h \
    ../../include/voreen/core/volume/volumemetadata.h \
    ../../include/voreen/core/volume/volumeoperator.h \
    ../../include/voreen/core/volume/volumeseries.h \
    ../../include/voreen/core/volume/volumeset.h \
    ../../include/voreen/core/volume/volumesetcontainer.h \
    ../../include/voreen/core/volume/volumetexture.h \
    ../../include/voreen/core/volume/gradient.h \
    ../../include/voreen/core/volume/histogram.h \
    ../../include/voreen/core/volume/observer.h
HEADERS += \
    ../../include/voreen/core/volume/bricking/balancedbrickresolutioncalculator.h \
    ../../include/voreen/core/volume/bricking/boxbrickingregion.h \
    ../../include/voreen/core/volume/bricking/brick.h \
    ../../include/voreen/core/volume/bricking/brickedvolume.h \
    ../../include/voreen/core/volume/bricking/brickedvolumegl.h \
    ../../include/voreen/core/volume/bricking/brickinginformation.h \
    ../../include/voreen/core/volume/bricking/brickingmanager.h \
    ../../include/voreen/core/volume/bricking/brickingregion.h \
    ../../include/voreen/core/volume/bricking/brickingregionmanager.h \
    ../../include/voreen/core/volume/bricking/bricklodselector.h \
    ../../include/voreen/core/volume/bricking/brickresolutioncalculator.h \
    ../../include/voreen/core/volume/bricking/cameralodselector.h \
    ../../include/voreen/core/volume/bricking/errorlodselector.h \
    ../../include/voreen/core/volume/bricking/largevolumemanager.h \
    ../../include/voreen/core/volume/bricking/maximumbrickresolutioncalculator.h \
    ../../include/voreen/core/volume/bricking/packingbrickassigner.h \
    ../../include/voreen/core/volume/bricking/rammanager.h \
    ../../include/voreen/core/volume/bricking/volumebrickcreator.h
HEADERS += \
    ../../include/voreen/core/xml/xmlserializable.h \
    ../../include/voreen/core/xml/serializable.h
HEADERS += \
    ../../ext/tinyxml/tinyxml.h \
    ../../ext/tinyxml/tinystr.h

####################################################
# Modules which can be enabled/disabled by defines
####################################################

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

# uncomment this for building a stripped library
# unix:QMAKE_POST_LINK=strip $(TARGET)

# this must come after alle SOURCES, HEADERS and FORMS have been added
contains(DEFINES, VRN_WITH_SVNVERSION) : revtarget.depends = $$SOURCES $$HEADERS $$FORMS

# installation
unix {
  !isEmpty(INSTALL_PREFIX) {
    NETWORKS = clippingcombine clippingplanes fancy glutexample labeling minimal multimodal \
               multiplanar regiongrowing simple singleslice slicesequence \
               standard volumeinversion
    WORKSPACES = standard walnut
    SCRIPTS = fps.py
    VOLUMES = nucleon.dat nucleon.raw walnut.dat walnut.raw
    TFS = nucleon.tfi walnut.tfi
    TEXTURES = anterior2.png axial_b.png axial_t.png coronal_b.png coronal_f.png \
               inferior2.png lateral2.png sagittal_l.png sagittal_r.png septal2.png

    FONTS = Vera.ttf
    DOCS = CREDITS.txt Changelog.txt LICENSE-academic.txt LICENSE.txt readme.txt \
           doc/gettingstarted/

    target.path = $$INSTALLPATH_LIB

    shaders.path = $$INSTALLPATH_SHARE/shaders
    shaders.files += $$SHADER_SOURCES
    shader_modules.path = $$INSTALLPATH_SHARE/shaders/modules
    shader_modules.files += $$SHADER_SOURCES_MODS
    shader_modules_brick.path = $$INSTALLPATH_SHARE/shaders/modules/bricking
    shader_modules_brick.files += $$SHADER_SOURCES_MODS_BRICK

    for(i, NETWORKS): NETWORKS_FULL += $$join(i, " ", "../../data/networks/", ".vnw")
    networks.path = $$INSTALLPATH_SHARE/networks
    networks.files += $$NETWORKS_FULL

    for(i, WORKSPACES): WORKSPACES_FULL += $$join(i, " ", "../../data/workspaces/", ".vws")
    workspaces.path = $$INSTALLPATH_SHARE/workspaces
    workspaces.files += $$WORKSPACES_FULL

    for(i, SCRIPTS): SCRIPTS_FULL += $$join(i, " ", "../../data/scripts/")
    scripts.path = $$INSTALLPATH_SHARE/scripts
    scripts.files += $$SCRIPTS_FULL

    for(i, VOLUMES): VOLUMES_FULL += $$join(i, " ", "../../data/", "")
    volumes.path = $$INSTALLPATH_SHARE
    volumes.files += $$VOLUMES_FULL

    for(i, TFS): TFS_FULL += $$join(i, " ", "../../data/transferfuncs/", "")
    tfs.path = $$INSTALLPATH_SHARE/transferfuncs
    tfs.files += $$TFS_FULL

    for(i, TEXTURES): TEXTURES_FULL += $$join(i, " ", "../../data/textures/", "")
    textures.path = $$INSTALLPATH_SHARE/textures
    textures.files += $$TEXTURES_FULL

    for(i, FONTS): FONTS_FULL += $$join(i, " ", "../../data/fonts/", "")
    fonts.path = $$INSTALLPATH_SHARE/fonts
    fonts.files += $$FONTS_FULL

    for(i, DOCS): DOCS_FULL += $$join(i, " ", "../../", "")
    documentation.path = $$INSTALLPATH_DOC
    documentation.files += $$DOCS_FULL

    icons.path = $$INSTALLPATH_SHARE
    icons.files += ../../resource/vrn_share/icons/icon-64.png

    INSTALLS += target shaders shader_modules shader_modules_brick networks workspaces scripts volumes \
                tfs textures fonts documentation icons
  }
}


### Local Variables:
### mode:conf-unix
### End:
