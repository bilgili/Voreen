/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "modules/base/basemodule.h"

///
/// Processors
///

// entry-exit points
#include "processors/entryexitpoints/eepgeometryintegrator.h"
#include "processors/entryexitpoints/meshentryexitpoints.h"

// geometry
#include "processors/geometry/boundingboxrenderer.h"
#include "processors/geometry/camerapositionrenderer.h"
#include "processors/geometry/geometryprocessor.h"
#include "processors/geometry/geometryrenderer.h"
#include "processors/geometry/geometryclipping.h"
#include "processors/geometry/geometryclippingwidget.h"
#include "processors/geometry/geometryslabclipping.h"
#include "processors/geometry/geometrytransformation.h"
#include "processors/geometry/geometrytransformationvolume.h"
#include "processors/geometry/lightwidgetrenderer.h"
#include "processors/geometry/planewidgetprocessor.h"
#include "processors/geometry/pointlistrenderer.h"
#include "processors/geometry/pointsegmentlistrenderer.h"
#include "processors/geometry/quadricrenderer.h"
#include "processors/geometry/slicepositionrenderer.h"

// image
#include "processors/image/background.h"
#include "processors/image/binaryimageprocessor.h"
#include "processors/image/unaryimageprocessor.h"
#include "processors/image/gaussian.h"
#include "processors/image/median.h"
#include "processors/image/mean.h"
#include "processors/image/nonminmaxsuppression.h"
#include "processors/image/colordepth.h"
#include "processors/image/compositor.h"
#include "processors/image/convolution.h"
#include "processors/image/depthdarkening.h"
#include "processors/image/edgedetect.h"
#include "processors/image/explosioncompositor.h"
#include "processors/image/fade.h"
#include "processors/image/grayscale.h"
#include "processors/image/imagemasking.h"
#include "processors/image/imagemorphology.h"
#include "processors/image/imageoverlay.h"
#include "processors/image/imagethreshold.h"
#include "processors/image/loopcompositor.h"
#include "processors/image/multiview.h"
#include "processors/image/orientationoverlay.h"
#include "processors/image/quadview.h"
#include "processors/image/regionofinterest2d.h"
#include "processors/image/splitter.h"
#include "processors/image/textoverlay.h"
#include "processors/image/tripleview.h"
#include "processors/image/unsharpmasking.h"

// proxy geometry
#include "processors/proxygeometry/cubeproxygeometry.h"
#include "processors/proxygeometry/explosionproxygeometry.h"
#include "processors/proxygeometry/multiplanarproxygeometry.h"
#include "processors/proxygeometry/multivolumeproxygeometry.h"

// render
#include "processors/render/cpuraycaster.h"
#include "processors/render/multiplanarslicerenderer.h"
#include "processors/render/rgbraycaster.h"
#include "processors/render/segmentationraycaster.h"
#include "processors/render/simpleraycaster.h"
#include "processors/render/singlevolumeraycaster.h"
#include "processors/render/multivolumeraycaster.h"
#include "processors/render/singlevolumeslicer.h"
#include "processors/render/sliceviewer.h"

// utility
#include "processors/utility/clockprocessor.h"
#include "processors/utility/distancemeasure.h"
#include "processors/utility/intensitymeasure.h"
#include "processors/utility/imagesequenceloopinitiator.h"
#include "processors/utility/imagesequenceloopfinalizer.h"
#include "processors/utility/metadataextractor.h"
#include "processors/utility/multiscale.h"
#include "processors/utility/renderloopinitiator.h"
#include "processors/utility/renderloopfinalizer.h"
#include "processors/utility/renderstore.h"
#include "processors/utility/scale.h"
#include "processors/utility/segmentationvalidation.h"
#include "processors/utility/volumecollectionmodalityfilter.h"
#include "processors/utility/volumeinformation.h"
#include "processors/utility/volumepicking.h"

// volume
#include "processors/volume/vectormagnitude.h"
#include "processors/volume/volumecrop.h"
#include "processors/volume/volumedistancetransform.h"
#include "processors/volume/volumeinversion.h"
#include "processors/volume/volumecombine.h"
#include "processors/volume/volumecomposer.h"
#include "processors/volume/volumedecomposer.h"
#include "processors/volume/volumefiltering.h"
#include "processors/volume/volumeformatconversion.h"
#include "processors/volume/volumegradient.h"
#include "processors/volume/volumelandmarkregistration.h"
#include "processors/volume/volumespacing.h"
#include "processors/volume/volumeoffset.h"
#include "processors/volume/volumetransformation.h"
#include "processors/volume/volumemasking.h"
#include "processors/volume/volumemorphology.h"
#include "processors/volume/volumeresample.h"
#include "processors/volume/volumecreate.h"
#include "processors/volume/volumecubify.h"
#include "processors/volume/volumehalfsample.h"
#include "processors/volume/volumemirror.h"


///
/// volume readers/writers
///
#include "io/analyzevolumereader.h"
#include "io/brukervolumereader.h"
#include "io/ecat7volumereader.h"
#include "io/multivolumereader.h"
#include "io/mhdvolumereader.h"
#include "io/mhdvolumewriter.h"
#include "io/nrrdvolumereader.h"
#include "io/nrrdvolumewriter.h"
#include "io/quadhidacvolumereader.h"
#include "io/synth2dreader.h"
#include "io/rawvoxvolumereader.h"
#include "io/tuvvolumereader.h"


namespace voreen {

const std::string BaseModule::loggerCat_("voreen.BaseModule");

BaseModule::BaseModule(const std::string& modulePath) : VoreenModule(modulePath) {
    setName("Base");

    // entry-exit points
    registerProcessor(new EEPGeometryIntegrator());
    registerProcessor(new MeshEntryExitPoints());

    // geometry
    registerProcessor(new BoundingBoxRenderer());
    registerProcessor(new CameraPositionRenderer());
    registerProcessor(new GeometryProcessor());
    registerProcessor(new GeometryRenderer());
    registerProcessor(new GeometryClipping());
    registerProcessor(new GeometryClippingWidget());
    registerProcessor(new GeometrySlabClipping());
    registerProcessor(new GeometryTransformation());
    registerProcessor(new GeometryTransformationVolume());
    registerProcessor(new LightWidgetRenderer());
    registerProcessor(new PlaneWidgetProcessor());
    registerProcessor(new PointListRenderer());
    registerProcessor(new PointSegmentListRenderer());
    registerProcessor(new QuadricRenderer());
    registerProcessor(new SlicePositionRenderer());

    // image
    registerProcessor(new Background());
    registerProcessor(new BinaryImageProcessor());
    registerProcessor(new ColorDepth());
    registerProcessor(new Compositor());
    registerProcessor(new Convolution());
    registerProcessor(new DepthDarkening());
    registerProcessor(new EdgeDetect());
    registerProcessor(new ExplosionCompositor());
    registerProcessor(new Fade());
    registerProcessor(new Gaussian());
    registerProcessor(new Grayscale());
    registerProcessor(new ImageMasking());
    registerProcessor(new ImageMorphology());
    registerProcessor(new ImageOverlay());
    registerProcessor(new ImageThreshold());
    registerProcessor(new Mean());
    registerProcessor(new Median());
    registerProcessor(new MultiView());
    registerProcessor(new NonMinMaxSuppression());
    registerProcessor(new OrientationOverlay());
    registerProcessor(new QuadView());
    registerProcessor(new RegionOfInterest2D());
    registerProcessor(new Splitter());
    registerProcessor(new TextOverlay());
    registerProcessor(new TripleView());
    registerProcessor(new UnaryImageProcessor());
    registerProcessor(new UnsharpMasking());

    // proxy geometry
    registerProcessor(new CubeProxyGeometry());
    registerProcessor(new ExplosionProxyGeometry());
    registerProcessor(new MultiPlanarProxyGeometry());
    registerProcessor(new MultiVolumeProxyGeometry());

    // render
    registerProcessor(new CPURaycaster());
    registerProcessor(new MultiplanarSliceRenderer());
    registerProcessor(new MultiVolumeRaycaster());
    registerProcessor(new SegmentationRaycaster());
    registerProcessor(new SimpleRaycaster());
    registerProcessor(new SingleVolumeRaycaster());
    registerProcessor(new SingleVolumeSlicer());
    registerProcessor(new SliceViewer());
    registerProcessor(new RGBRaycaster());

    // utility
    registerProcessor(new ClockProcessor());
    registerProcessor(new DistanceMeasure());
    registerProcessor(new IntensityMeasure());
    registerProcessor(new ImageSequenceLoopInitiator());
    registerProcessor(new ImageSequenceLoopFinalizer());
    registerProcessor(new MetaDataExtractor());
    registerProcessor(new MultiScale());
    registerProcessor(new RenderLoopInitiator());
    registerProcessor(new RenderLoopFinalizer());
    registerProcessor(new RenderStore());
    registerProcessor(new SegmentationValidation());
    registerProcessor(new SingleScale());
    registerProcessor(new VolumeCollectionModalityFilter());
    registerProcessor(new VolumeInformation());
    registerProcessor(new VolumePicking());

    // volume
    registerProcessor(new VectorMagnitude());
    registerProcessor(new VolumeCombine());
    registerProcessor(new VolumeComposer());
    registerProcessor(new VolumeCreate());
    registerProcessor(new VolumeCrop());
    registerProcessor(new VolumeCubify());
    registerProcessor(new VolumeDecomposer());
    registerProcessor(new VolumeDistanceTransform());
    registerProcessor(new VolumeFiltering());
    registerProcessor(new VolumeFormatConversion());
    registerProcessor(new VolumeGradient());
    registerProcessor(new VolumeHalfsample());
    registerProcessor(new VolumeInversion());
    registerProcessor(new VolumeLandmarkRegistration());
    registerProcessor(new VolumeMasking());
    registerProcessor(new VolumeMirror());
    registerProcessor(new VolumeMorphology());
    registerProcessor(new VolumeResample());
    registerProcessor(new VolumeSpacing());
    registerProcessor(new VolumeTransformation());
    registerProcessor(new VolumeOffset());

    // volume readers/writers
    registerVolumeReader(new AnalyzeVolumeReader());
    registerVolumeReader(new BrukerVolumeReader());
    registerVolumeReader(new ECAT7VolumeReader());
    registerVolumeReader(new MultiVolumeReader(0));
    registerVolumeReader(new MhdVolumeReader());
    registerVolumeWriter(new MhdVolumeWriter());
    registerVolumeReader(new NrrdVolumeReader());
    registerVolumeWriter(new NrrdVolumeWriter());
    registerVolumeReader(new QuadHidacVolumeReader());
    registerVolumeReader(new Synth2DReader());
    registerVolumeReader(new RawVoxVolumeReader());
    registerVolumeReader(new TUVVolumeReader());

    // shader paths
    addShaderPath(getModulePath("glsl"));

    // serialization factories
    registerSerializerFactory(new RegionOfInterest2D::RegionOfInterestGeometryFactory());
}

void BaseModule::initialize() throw (tgt::Exception) {
    VoreenModule::initialize();
}

void BaseModule::deinitialize() throw (tgt::Exception) {
    VoreenModule::deinitialize();
}

} // namespace
