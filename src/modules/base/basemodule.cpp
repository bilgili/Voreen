/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/modules/base/basemodule.h"

///
/// Processors
///

// data sources
#include "voreen/modules/base/processors/datasource/geometrysource.h"
#include "voreen/modules/base/processors/datasource/imagesequencesource.h"
#include "voreen/modules/base/processors/datasource/imagesource.h"
#include "voreen/modules/base/processors/datasource/plotdatasource.h"
#include "voreen/modules/base/processors/datasource/textsource.h"
#include "voreen/modules/base/processors/datasource/volumecollectionsource.h"
#include "voreen/modules/base/processors/datasource/volumeseriessource.h"
#include "voreen/modules/base/processors/datasource/volumesource.h"

// entry-exit points
#include "voreen/modules/base/processors/entryexitpoints/entryexitpoints.h"
#include "voreen/modules/base/processors/entryexitpoints/meshentryexitpoints.h"

// geometry
#include "voreen/modules/base/processors/geometry/boundingboxrenderer.h"
#include "voreen/modules/base/processors/geometry/camerapositionrenderer.h"
#include "voreen/modules/base/processors/geometry/depthpeelingprocessor.h"
#include "voreen/modules/base/processors/geometry/eepgeometryintegrator.h"
#include "voreen/modules/base/processors/geometry/geometryprocessor.h"
#include "voreen/modules/base/processors/geometry/geometryrenderer.h"
#include "voreen/modules/base/processors/geometry/isosurfaceextractor.h"
#include "voreen/modules/base/processors/geometry/lightwidgetrenderer.h"
#include "voreen/modules/base/processors/geometry/meshclipping.h"
#include "voreen/modules/base/processors/geometry/meshclippingwidget.h"
#include "voreen/modules/base/processors/geometry/meshslabclipping.h"
#include "voreen/modules/base/processors/geometry/planewidgetprocessor.h"
#include "voreen/modules/base/processors/geometry/pointlistrenderer.h"
#include "voreen/modules/base/processors/geometry/pointsegmentlistrenderer.h"
#include "voreen/modules/base/processors/geometry/quadricrenderer.h"
#include "voreen/modules/base/processors/geometry/slicepositionrenderer.h"

// image
#include "voreen/modules/base/processors/image/background.h"
#include "voreen/modules/base/processors/image/binaryimageprocessor.h"
#include "voreen/modules/base/processors/image/unaryimageprocessor.h"
#include "voreen/modules/base/processors/image/gaussian.h"
#include "voreen/modules/base/processors/image/dilation.h"
#include "voreen/modules/base/processors/image/erosion.h"
#include "voreen/modules/base/processors/image/median.h"
#include "voreen/modules/base/processors/image/mean.h"
#include "voreen/modules/base/processors/image/nonminmaxsuppression.h"
#include "voreen/modules/base/processors/image/canny.h"
#include "voreen/modules/base/processors/image/colordepth.h"
#include "voreen/modules/base/processors/image/compositor.h"
#include "voreen/modules/base/processors/image/convolution.h"
#include "voreen/modules/base/processors/image/depthdarkening.h"
#include "voreen/modules/base/processors/image/edgedetect.h"
#include "voreen/modules/base/processors/image/explosioncompositor.h"
#include "voreen/modules/base/processors/image/fade.h"
#include "voreen/modules/base/processors/image/gabor.h"
#include "voreen/modules/base/processors/image/grayscale.h"
#include "voreen/modules/base/processors/image/imagemasking.h"
#include "voreen/modules/base/processors/image/imageoverlay.h"
#include "voreen/modules/base/processors/image/imagethreshold.h"
#include "voreen/modules/base/processors/image/labeling.h"
#include "voreen/modules/base/processors/image/loopcompositor.h"
#include "voreen/modules/base/processors/image/orientationoverlay.h"
#include "voreen/modules/base/processors/image/regionofinterest2d.h"
#include "voreen/modules/base/processors/image/quadview.h"

// plotting
#include "voreen/modules/base/processors/plotting/lineplot.h"

// proxy geometry
#include "voreen/modules/base/processors/proxygeometry/cubeproxygeometry.h"
#include "voreen/modules/base/processors/proxygeometry/cubemeshproxygeometry.h"
#include "voreen/modules/base/processors/proxygeometry/explosionproxygeometry.h"
#include "voreen/modules/base/processors/proxygeometry/multivolumeproxygeometry.h"
#include "voreen/modules/base/processors/proxygeometry/proxygeometry.h"

// render
#include "voreen/modules/base/processors/render/cpuraycaster.h"
#include "voreen/modules/base/processors/render/curvatureraycaster.h"
#include "voreen/modules/base/processors/render/glslraycaster.h"
#include "voreen/modules/base/processors/render/halfangleslicer.h"
#include "voreen/modules/base/processors/render/idraycaster.h"
#include "voreen/modules/base/processors/render/multiplanarslicerenderer.h"
#include "voreen/modules/base/processors/render/rgbraycaster.h"
#include "voreen/modules/base/processors/render/segmentationraycaster.h"
#include "voreen/modules/base/processors/render/simpleraycaster.h"
#include "voreen/modules/base/processors/render/singlevolumeraycaster.h"
#include "voreen/modules/base/processors/render/multivolumeraycaster.h"
#include "voreen/modules/base/processors/render/singlevolumeslicer.h"
#include "voreen/modules/base/processors/render/sliceviewer.h"

// utility
#include "voreen/modules/base/processors/utility/clockprocessor.h"
#include "voreen/modules/base/processors/utility/coordinatetransformation.h"
#include "voreen/modules/base/processors/utility/distancemeasure.h"
#include "voreen/modules/base/processors/utility/intensitymeasure.h"
#include "voreen/modules/base/processors/utility/imageanalyzer.h"
#include "voreen/modules/base/processors/utility/imageselector.h"
#include "voreen/modules/base/processors/utility/imagesequenceloopinitiator.h"
#include "voreen/modules/base/processors/utility/imagesequenceloopfinalizer.h"
#include "voreen/modules/base/processors/utility/multiscale.h"
#include "voreen/modules/base/processors/utility/renderloopinitiator.h"
#include "voreen/modules/base/processors/utility/renderloopfinalizer.h"
#include "voreen/modules/base/processors/utility/renderstore.h"
#include "voreen/modules/base/processors/utility/scale.h"
#include "voreen/modules/base/processors/utility/propertycontainer.h"
#include "voreen/modules/base/processors/utility/segmentationvalidation.h"
#include "voreen/modules/base/processors/utility/textoverlay.h"
#include "voreen/modules/base/processors/utility/volumecollectionmodalityfilter.h"
#include "voreen/modules/base/processors/utility/volumeinformation.h"
#include "voreen/modules/base/processors/utility/volumepicking.h"
#include "voreen/modules/base/processors/utility/volumeregistration.h"
#include "voreen/modules/base/processors/utility/volumeselector.h"

// volume
#include "voreen/modules/base/processors/volume/vectormagnitude.h"
#include "voreen/modules/base/processors/volume/volumebitscale.h"
#include "voreen/modules/base/processors/volume/volumeinversion.h"
#include "voreen/modules/base/processors/volume/volumecombine.h"
#include "voreen/modules/base/processors/volume/volumecurvature.h"
#include "voreen/modules/base/processors/volume/volumedecomposer.h"
#include "voreen/modules/base/processors/volume/volumefiltering.h"
#include "voreen/modules/base/processors/volume/volumegradient.h"
#include "voreen/modules/base/processors/volume/volumetransformation.h"
#include "voreen/modules/base/processors/volume/volumemasking.h"
#include "voreen/modules/base/processors/volume/volumemorphology.h"
#include "voreen/modules/base/processors/volume/volumedistancetransform.h"
#include "voreen/modules/base/processors/volume/volumeresample.h"
#include "voreen/modules/base/processors/volume/volumesave.h"
#include "voreen/modules/base/processors/volume/volumecreate.h"
#include "voreen/modules/base/processors/volume/volumecubify.h"
#include "voreen/modules/base/processors/volume/volumehalfsample.h"
#include "voreen/modules/base/processors/volume/volumemirror.h"
#include "voreen/modules/base/processors/volume/volumesubset.h"


///
/// Volume readers/writers
///
#include "voreen/modules/base/io/analyzevolumereader.h"
#include "voreen/modules/base/io/interfilevolumereader.h"
#include "voreen/modules/base/io/multivolumereader.h"
#include "voreen/modules/base/io/nrrdvolumereader.h"
#include "voreen/modules/base/io/nrrdvolumewriter.h"
#include "voreen/modules/base/io/philipsusvolumereader.h"
#include "voreen/modules/base/io/quadhidacvolumereader.h"
#include "voreen/modules/base/io/tuvvolumereader.h"


namespace voreen {

const std::string BaseModule::loggerCat_("voreen.BaseModule");

BaseModule::BaseModule()
    : VoreenModule()
{
    setName("Base");

    // data sources
    addProcessor(new GeometrySource());
    addProcessor(new ImageSequenceSource());
    addProcessor(new ImageSource());
    addProcessor(new PlotDataSource());
    addProcessor(new TextSource());
    addProcessor(new VolumeCollectionSource());
    addProcessor(new VolumeSeriesSource());
    addProcessor(new VolumeSource());

    // entry-exit points
    addProcessor(new EntryExitPoints());
    addProcessor(new MeshEntryExitPoints());

    // geometry
    addProcessor(new BoundingBoxRenderer());
    addProcessor(new CameraPositionRenderer());
    addProcessor(new DepthPeelingProcessor());
    addProcessor(new EEPGeometryIntegrator());
    addProcessor(new GeometryProcessor());
    addProcessor(new GeometryRenderer());
    addProcessor(new IsosurfaceExtractor());
    addProcessor(new LightWidgetRenderer());
    addProcessor(new MeshClipping());
    addProcessor(new MeshClippingWidget());
    addProcessor(new MeshSlabClipping());
    addProcessor(new PlaneWidgetProcessor());
    addProcessor(new PointListRenderer());
    addProcessor(new PointSegmentListRenderer());
    addProcessor(new QuadricRenderer());
    addProcessor(new SlicePositionRenderer());


    // image
    addProcessor(new Background());
    addProcessor(new UnaryImageProcessor());
    addProcessor(new BinaryImageProcessor());
    addProcessor(new Gaussian());
    addProcessor(new Erosion());
    addProcessor(new ExplosionCompositor());
    addProcessor(new Dilation());
    addProcessor(new Median());
    addProcessor(new Mean());
    addProcessor(new NonMinMaxSuppression());
    addProcessor(new Canny());
    addProcessor(new ColorDepth());
    addProcessor(new Compositor());
    addProcessor(new Convolution());
    addProcessor(new DepthDarkening());
    addProcessor(new EdgeDetect());
    addProcessor(new Fade());
    addProcessor(new PropertyContainer());
    addProcessor(new Gabor());
    addProcessor(new Grayscale());
    addProcessor(new ImageMasking());
    addProcessor(new ImageOverlay());
    addProcessor(new ImageThreshold());
    addProcessor(new Labeling());
    addProcessor(new QuadView());
    addProcessor(new RegionOfInterest2D());
    addProcessor(new OrientationOverlay());

    // plotting
    addProcessor(new LinePlot());

    // proxy geometry
    addProcessor(new CubeProxyGeometry());
    addProcessor(new CubeMeshProxyGeometry());
    addProcessor(new ExplosionProxyGeometry());
    addProcessor(new MultiVolumeProxyGeometry());

    // render
    addProcessor(new CPURaycaster());
    addProcessor(new CurvatureRaycaster());
    addProcessor(new GLSLRaycaster());
    addProcessor(new HalfAngleSlicer());
    addProcessor(new SingleVolumeSlicer());
    addProcessor(new RGBRaycaster());
    addProcessor(new SegmentationRaycaster());
    addProcessor(new SimpleRaycaster());
    addProcessor(new SingleVolumeRaycaster());
    addProcessor(new MultiVolumeRaycaster());
    addProcessor(new IDRaycaster());
    addProcessor(new SliceViewer());
    addProcessor(new MultiplanarSliceRenderer());

    // utility
    addProcessor(new ClockProcessor());
    addProcessor(new CoordinateTransformation());
    addProcessor(new DistanceMeasure());
    addProcessor(new IntensityMeasure());
    addProcessor(new ImageAnalyzer());
    addProcessor(new ImageSequenceLoopInitiator());
    addProcessor(new ImageSequenceLoopFinalizer());
    addProcessor(new ImageSelector());
    addProcessor(new MultiScale());
    addProcessor(new RenderLoopInitiator());
    addProcessor(new RenderLoopFinalizer());
    addProcessor(new RenderStore());
    addProcessor(new SegmentationValidation());
    addProcessor(new SingleScale());
    addProcessor(new TextOverlay());
    addProcessor(new VolumeCollectionModalityFilter());
    addProcessor(new VolumeInformation());
    addProcessor(new VolumePicking());
    addProcessor(new VolumeRegistration());
    addProcessor(new VolumeSelector());

    // volume
    addProcessor(new VolumeDecomposer());
    addProcessor(new VectorMagnitude());
    addProcessor(new VolumeCombine());
    addProcessor(new VolumeInversion());
    addProcessor(new VolumeMasking());
    addProcessor(new VolumeDistanceTransform());
    addProcessor(new VolumeCurvature());
    addProcessor(new VolumeGradient());
    addProcessor(new VolumeResample());
    addProcessor(new VolumeSave());
    addProcessor(new VolumeTransformation());
    addProcessor(new VolumeBitScale());
    addProcessor(new VolumeCreate());
    addProcessor(new VolumeCubify());
    addProcessor(new VolumeFiltering());
    addProcessor(new VolumeHalfsample());
    addProcessor(new VolumeMirror());
    addProcessor(new VolumeMorphology());
    addProcessor(new VolumeSubSet());

    // volume readers/writers
    addVolumeReader(new AnalyzeVolumeReader());
    addVolumeReader(new InterfileVolumeReader());
    addVolumeReader(new MultiVolumeReader(0));
    addVolumeReader(new NrrdVolumeReader());
    addVolumeWriter(new NrrdVolumeWriter());
    addVolumeReader(new PhilipsUSVolumeReader());
    addVolumeReader(new QuadHidacVolumeReader());
    addVolumeReader(new TUVVolumeReader());

    // shader paths
    addShaderPath(getModulesPath("base/glsl"));
}

void BaseModule::initialize() throw (VoreenException) {
    VoreenModule::initialize();
}

void BaseModule::deinitialize() throw (VoreenException) {
    VoreenModule::deinitialize();
}

} // namespace
