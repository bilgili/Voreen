# core module class
SOURCES += $${VRN_MODULE_SRC_DIR}/base/basemodule.cpp
HEADERS += $${VRN_MODULE_INC_DIR}/base/basemodule.h

#    
# Processor sources
#
SOURCES += \
    $${VRN_MODULE_SRC_DIR}/base/processors/datasource/geometrysource.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/datasource/imagesequencesource.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/datasource/imagesource.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/datasource/textsource.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/datasource/volumecollectionsource.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/datasource/volumesource.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/entryexitpoints/entryexitpoints.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/entryexitpoints/meshentryexitpoints.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/boundingboxrenderer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/camerapositionrenderer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/clippingplanewidget.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/depthpeelingprocessor.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/eepgeometryintegrator.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/geometryprocessor.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/geometryrenderer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/isosurfaceextractor.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/lightwidgetrenderer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/meshclipping.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/meshclippingwidget.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/meshslabclipping.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/pointlistrenderer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/pointsegmentlistrenderer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/geometry/slicepositionrenderer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/background.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/binaryimageprocessor.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/canny.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/colordepth.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/compositor.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/convolution.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/depthdarkening.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/dilation.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/edgedetect.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/erosion.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/explosioncompositor.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/fade.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/gabor.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/gaussian.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/grayscale.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/imageabstraction.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/imagemasking.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/imageoverlay.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/imagethreshold.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/labeling.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/labelingmath.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/loopcompositor.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/mean.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/median.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/nonminmaxsuppression.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/orientationoverlay.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/quadview.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/regionofinterest2d.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/image/unaryimageprocessor.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/proxygeometry/cubemeshproxygeometry.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/proxygeometry/cubeproxygeometry.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/proxygeometry/explosionproxygeometry.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/proxygeometry/multivolumeproxygeometry.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/proxygeometry/proxygeometry.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/cpuraycaster.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/curvatureraycaster.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/glslraycaster.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/halfangleslicer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/idraycaster.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/multiplanarslicerenderer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/multivolumeraycaster.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/rgbraycaster.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/segmentationraycaster.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/simpleraycaster.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/singlevolumeraycaster.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/singlevolumeslicer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/slicerendererbase.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/sliceviewer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/render/volumeslicer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/clockprocessor.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/coordinatetransformation.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/distancemeasure.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/imageselector.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/imagesequenceloopfinalizer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/imagesequenceloopinitiator.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/intensitymeasure.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/multiscale.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/renderloopfinalizer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/renderloopinitiator.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/renderstore.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/scale.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/segmentationvalidation.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/textoverlay.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/volumecollectionmodalityfilter.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/volumeinformation.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/utility/volumeselector.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/vectormagnitude.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumecombine.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumeconvert.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumecreate.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumecubify.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumecurvature.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumedecomposer.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumedistancetransform.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumefiltering.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumegradient.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumehalfsample.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumeinversion.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumemasking.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumemirrorz.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumemorphology.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumeprocessor.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumeresample.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumesave.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumesubset.cpp \
    $${VRN_MODULE_SRC_DIR}/base/processors/volume/volumetransformation.cpp
 
#
# Volume readers/writers sources
#
SOURCES += \
    $${VRN_MODULE_SRC_DIR}/base/io/analyzevolumereader.cpp \    
    $${VRN_MODULE_SRC_DIR}/base/io/interfilevolumereader.cpp \       
    $${VRN_MODULE_SRC_DIR}/base/io/multivolumereader.cpp \
    $${VRN_MODULE_SRC_DIR}/base/io/nrrdvolumereader.cpp \
    $${VRN_MODULE_SRC_DIR}/base/io/nrrdvolumewriter.cpp \
    $${VRN_MODULE_SRC_DIR}/base/io/philipsusvolumereader.cpp \
    $${VRN_MODULE_SRC_DIR}/base/io/quadhidacvolumereader.cpp \
    $${VRN_MODULE_SRC_DIR}/base/io/tuvvolumereader.cpp  

# 
# Processor headers
#
HEADERS += \
    $${VRN_MODULE_INC_DIR}/base/processors/datasource/geometrysource.h \
    $${VRN_MODULE_INC_DIR}/base/processors/datasource/imagesequencesource.h \
    $${VRN_MODULE_INC_DIR}/base/processors/datasource/imagesource.h \
    $${VRN_MODULE_INC_DIR}/base/processors/datasource/textsource.h \
    $${VRN_MODULE_INC_DIR}/base/processors/datasource/volumecollectionsource.h \
    $${VRN_MODULE_INC_DIR}/base/processors/datasource/volumesource.h \
    $${VRN_MODULE_INC_DIR}/base/processors/entryexitpoints/entryexitpoints.h \
    $${VRN_MODULE_INC_DIR}/base/processors/entryexitpoints/meshentryexitpoints.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/boundingboxrenderer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/camerapositionrenderer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/clippingplanewidget.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/depthpeelingprocessor.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/geometryprocessor.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/geometryrenderer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/isosurfaceextractor.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/lightwidgetrenderer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/meshclipping.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/meshclippingwidget.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/meshslabclipping.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/pointlistrenderer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/pointsegmentlistrenderer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/geometry/slicepositionrenderer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/background.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/binaryimageprocessor.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/canny.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/colordepth.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/compositor.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/convolution.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/depthdarkening.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/dilation.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/edgedetect.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/erosion.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/explosioncompositor.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/fade.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/gabor.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/gaussian.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/grayscale.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/imageabstraction.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/imagemasking.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/imageoverlay.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/imagethreshold.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/labeling.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/labelingmath.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/loopcompositor.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/mean.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/median.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/nonminmaxsuppression.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/orientationoverlay.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/quadview.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/regionofinterest2d.h \
    $${VRN_MODULE_INC_DIR}/base/processors/image/unaryimageprocessor.h \
    $${VRN_MODULE_INC_DIR}/base/processors/proxygeometry/cubemeshproxygeometry.h \
    $${VRN_MODULE_INC_DIR}/base/processors/proxygeometry/cubeproxygeometry.h \
    $${VRN_MODULE_INC_DIR}/base/processors/proxygeometry/explosionproxygeometry.h \
    $${VRN_MODULE_INC_DIR}/base/processors/proxygeometry/multivolumeproxygeometry.h \
    $${VRN_MODULE_INC_DIR}/base/processors/proxygeometry/proxygeometry.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/cpuraycaster.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/curvatureraycaster.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/glslraycaster.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/halfangleslicer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/idraycaster.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/multiplanarslicerenderer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/multivolumeraycaster.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/rgbraycaster.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/segmentationraycaster.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/simpleraycaster.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/singlevolumeraycaster.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/singlevolumeslicer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/slicerendererbase.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/sliceviewer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/render/volumeslicer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/clockprocessor.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/coordinatetransformation.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/distancemeasure.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/imageselector.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/imagesequenceloopfinalizer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/imagesequenceloopinitiator.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/intensitymeasure.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/multiscale.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/renderloopfinalizer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/renderloopinitiator.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/renderstore.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/scale.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/segmentationvalidation.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/textoverlay.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/volumecollectionmodalityfilter.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/volumeinformation.h \
    $${VRN_MODULE_INC_DIR}/base/processors/utility/volumeselector.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/vectormagnitude.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumecombine.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumeconvert.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumecreate.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumecubify.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumecurvature.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumedecomposer.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumedistancetransform.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumefiltering.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumegradient.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumehalfsample.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumeinversion.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumemasking.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumemirrorz.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumemorphology.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumeprocessor.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumeresample.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumesave.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumesubset.h \
    $${VRN_MODULE_INC_DIR}/base/processors/volume/volumetransformation.h
    
#
# Volume readers/writers headers
#
HEADERS += \
    $${VRN_MODULE_INC_DIR}/base/io/analyzevolumereader.h \
    $${VRN_MODULE_INC_DIR}/base/io/interfilevolumereader.h \
    $${VRN_MODULE_INC_DIR}/base/io/multivolumereader.h \
    $${VRN_MODULE_INC_DIR}/base/io/nrrdvolumereader.h \
    $${VRN_MODULE_INC_DIR}/base/io/nrrdvolumewriter.h \
    $${VRN_MODULE_INC_DIR}/base/io/philipsusvolumereader.h \
    $${VRN_MODULE_INC_DIR}/base/io/quadhidacvolumereader.h \
    $${VRN_MODULE_INC_DIR}/base/io/tuvvolumereader.h \
    
#
# Processor shaders (only necessary for making them visible in Visual Studio)
#
SHADER_SOURCES += \
    $${VRN_MODULE_SRC_DIR}/base/glsl/eep_clipping.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/eep_depth.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/eep_depth.vert \
    $${VRN_MODULE_SRC_DIR}/base/glsl/eep_geometry.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/eep_inside_volume.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/eep_inside_volume.vert \
    $${VRN_MODULE_SRC_DIR}/base/glsl/eep_jitter.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/eep_simple.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/eep_simple.vert \
    $${VRN_MODULE_SRC_DIR}/base/glsl/eep_texcoord.vert \
    $${VRN_MODULE_SRC_DIR}/base/glsl/eep_vertex.vert \
    $${VRN_MODULE_SRC_DIR}/base/glsl/gabor.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_background.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_binary.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_canny.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_colordepth.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_compositor.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_convolution.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_depthdarkening.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_depthpeeling.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_depthpeeling.vert \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_dilation.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_distance.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_edgedetect.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_erosion.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_fade.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_gaussian.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_grayscale.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_imageabstraction.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_imagethreshold.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_labeling.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_mask.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_mean.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_median.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_nonminmax.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_orientationoverlay.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_scale.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/pp_unary.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/rc_curvature.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/rc_firsthit.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/rc_hitpoints.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/rc_id.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/rc_multivolume.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/rc_rgb.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/rc_segmentation.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/rc_simple.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/rc_singlevolume.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/sl_base.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/sl_base.vert \
    $${VRN_MODULE_SRC_DIR}/base/glsl/sl_frontcompositing.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/sl_halfslicing.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/sl_singlevolume.frag \
    $${VRN_MODULE_SRC_DIR}/base/glsl/sl_singlevolume.vert \
    $${VRN_MODULE_SRC_DIR}/base/glsl/textoverlay.frag

### Local Variables:
### mode:conf-unix
### End:
