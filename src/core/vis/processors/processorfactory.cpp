/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/processorfactory.h"

#include <algorithm> // for sorting vectors
#include <functional>

#include "voreen/core/vis/processors/clockprocessor.h"
#include "voreen/core/vis/processors/entryexitpoints/entryexitpoints.h"
#include "voreen/core/vis/processors/entryexitpoints/depthpeelingentryexitpoints.h"
#include "voreen/core/vis/processors/geometry/clippingplanewidget.h"
#include "voreen/core/vis/processors/geometry/coordinatetransformation.h"
#include "voreen/core/vis/processors/geometry/geometryprocessor.h"
#include "voreen/core/vis/processors/geometry/geometrysource.h"
#include "voreen/core/vis/processors/geometry/pointlistrenderer.h"
#include "voreen/core/vis/processors/geometry/pointsegmentlistrenderer.h"
#include "voreen/core/vis/processors/image/background.h"
#include "voreen/core/vis/processors/image/binaryimageprocessor.h"
#include "voreen/core/vis/processors/image/unaryimageprocessor.h"
#include "voreen/core/vis/processors/image/gaussian.h"
#include "voreen/core/vis/processors/image/canvasrenderer.h"
#include "voreen/core/vis/processors/image/colordepth.h"
#include "voreen/core/vis/processors/image/compositer.h"
#include "voreen/core/vis/processors/image/crosshair.h"
#include "voreen/core/vis/processors/image/depthdarkening.h"
#include "voreen/core/vis/processors/image/depthoffield.h"
#include "voreen/core/vis/processors/image/edgedetect.h"
#include "voreen/core/vis/processors/image/fade.h"
#include "voreen/core/vis/processors/image/glow.h"
#include "voreen/core/vis/processors/image/grayscale.h"
#include "voreen/core/vis/processors/image/imageoverlay.h"
#include "voreen/core/vis/processors/image/labeling.h"
#include "voreen/core/vis/processors/image/regionmodifier.h"
#include "voreen/core/vis/processors/image/renderstore.h"
#include "voreen/core/vis/processors/image/scale.h"
#include "voreen/core/vis/processors/image/multiscale.h"
#include "voreen/core/vis/processors/image/quadview.h"
#include "voreen/core/vis/processors/image/threshold.h"
#include "voreen/core/vis/processors/proxygeometry/axialsliceproxygeometry.h"
#include "voreen/core/vis/processors/proxygeometry/cubecutproxygeometry.h"
#include "voreen/core/vis/processors/proxygeometry/cubeproxygeometry.h"
#include "voreen/core/vis/processors/proxygeometry/multipleaxialsliceproxygeometry.h"
#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"
#include "voreen/core/vis/processors/proxygeometry/sliceproxygeometry.h"
#include "voreen/core/vis/processors/proxygeometry/slicingproxygeometry.h"
#include "voreen/core/vis/processors/render/glslraycaster.h"
#include "voreen/core/vis/processors/render/idraycaster.h"
#include "voreen/core/vis/processors/render/multivolumeraycaster.h"
#include "voreen/core/vis/processors/render/orthogonalslicerenderer.h"
#include "voreen/core/vis/processors/render/segmentationraycaster.h"
#include "voreen/core/vis/processors/render/simpleraycaster.h"
#include "voreen/core/vis/processors/render/singleslicerenderer.h"
#include "voreen/core/vis/processors/render/singlevolumeraycaster.h"
#include "voreen/core/vis/processors/render/sliceentrypoints.h"
#include "voreen/core/vis/processors/render/slicesequencerenderer.h"
#include "voreen/core/vis/processors/render/volumeeditor.h"
#include "voreen/core/vis/processors/volume/regiongrowing.h"
#include "voreen/core/vis/processors/volume/vectormagnitude.h"
#include "voreen/core/vis/processors/volume/volumecollectionmodalityfilter.h"
#include "voreen/core/vis/processors/volume/volumeinversion.h"
#include "voreen/core/vis/processors/volume/volumegradient.h"
#include "voreen/core/vis/processors/volume/volumesave.h"
#include "voreen/core/vis/processors/volume/volumeselector.h"
#include "voreen/core/vis/processors/volume/volumesourceprocessor.h"
#include "voreen/core/vis/processors/volume/volumecollectionsourceprocessor.h"

#ifdef VRN_MODULE_FLOWREEN
#include "voreen/modules/flowreen/flowmagnitudes3d.h"
#include "voreen/modules/flowreen/flowslicerenderer2d.h"
#include "voreen/modules/flowreen/flowslicerenderer3d.h"
#include "voreen/modules/flowreen/flowstreamlinestexture3d.h"
#include "voreen/modules/flowreen/pathlinerenderer3d.h"
#include "voreen/modules/flowreen/streamlinerenderer3d.h"
#endif

namespace voreen {

ProcessorFactory* ProcessorFactory::instance_ = 0;

ProcessorFactory::ProcessorFactory() {
    initializeClassList();
}

ProcessorFactory::~ProcessorFactory() {
    for (std::map<std::string, Processor*>::iterator it = classList_.begin();
        it != classList_.end(); ++it)
    {
        delete it->second;
    }
}

ProcessorFactory* ProcessorFactory::getInstance() {
    if (!instance_)
        instance_ = new ProcessorFactory();

    return instance_;
}

std::string ProcessorFactory::getProcessorInfo(const std::string& name) {
    std::map<std::string, Processor*>::iterator it = classList_.find(name);
    if (it != classList_.end() && it->second != 0)
        return it->second->getProcessorInfo();

    return "";
}

std::string ProcessorFactory::getProcessorCategory(const std::string& name) {
    std::map<std::string, Processor*>::iterator it = classList_.find(name);
    if (it != classList_.end() && it->second != 0)
        return it->second->getCategory();

    return "";
}

std::string ProcessorFactory::getProcessorModuleName(const std::string& name) {
    std::map<std::string, Processor*>::iterator it = classList_.find(name);
    if (it != classList_.end() && it->second != 0)
        return it->second->getModuleName();

    return "";
}

    // Returns processor codestate
Processor::CodeState ProcessorFactory::getProcessorCodeState(const std::string& name) {
    std::map<std::string, Processor*>::iterator it = classList_.find(name);
    if (it != classList_.end() && it->second != 0)
        return it->second->getCodeState();

    return Processor::CODE_STATE_BROKEN;
}

void ProcessorFactory::destroy() {
    delete instance_;
    instance_ = 0;
}

Processor* ProcessorFactory::create(const std::string& name) {
    std::map<std::string, Processor*>::iterator it = classList_.find(name);
    if (it != classList_.end() && it->second != 0)
        return it->second->create();

    return 0;
}

const std::string ProcessorFactory::getTypeString(const std::type_info& type) const {
    for (std::map<std::string, Processor*>::const_iterator it = classList_.begin();
        it != classList_.end(); ++it)
    {
        if (type == typeid(*(it->second)))
            return it->first;
    }

    return "";
}

Serializable* ProcessorFactory::createType(const std::string& typeString) {
    return create(typeString);
}

void ProcessorFactory::registerClass(Processor* const newClass) {
    classList_.insert(std::make_pair(newClass->getClassName(), newClass));

    // In the KnownClassesVector, the elements are pairs of string, with
    // the first part of the pair identifying the category name of the
    // processor and the second one the class name.
    //
    knownClasses_.push_back(
        std::make_pair(newClass->getCategory(), newClass->getClassName()) );
}

void ProcessorFactory::initializeClassList() {
    registerClass(new AxialSliceProxyGeometry());
    registerClass(new CubeProxyGeometry());
    registerClass(new MultipleAxialSliceProxyGeometry());
    registerClass(new SliceProxyGeometry());

    registerClass(new EntryExitPoints());
    registerClass(new SliceEntryPoints());

	registerClass(new Background());
	registerClass(new UnaryImageProcessor());
	registerClass(new BinaryImageProcessor());
    registerClass(new Gaussian());
    registerClass(new ColorDepth());
    registerClass(new Compositer());
    registerClass(new DepthDarkening());
    registerClass(new EdgeDetect());
    registerClass(new Fade());
    registerClass(new GLSLRaycaster());
    registerClass(new Grayscale());
    registerClass(new IDRaycaster());
    registerClass(new ImageOverlay());
    registerClass(new Labeling());
    registerClass(new SingleScale());
    registerClass(new MultiScale());
    registerClass(new QuadView());
    registerClass(new SegmentationRaycaster());
    registerClass(new SimpleRaycaster());
    registerClass(new SingleVolumeRaycaster());
    registerClass(new Threshold());

    registerClass(new CoordinateTransformation());
    registerClass(new GeomBoundingBox());
    registerClass(new SlicePositionRenderer());
    registerClass(new GeomLightWidget());
    registerClass(new GeometryProcessor());
    registerClass(new GeometrySource());
    registerClass(new PointListRenderer());
    registerClass(new PointSegmentListRenderer());

	registerClass(new ClippingPlaneWidget());

    registerClass(new CanvasRenderer());

    registerClass(new ClockProcessor());
    registerClass(new VolumeCollectionModalityFilter());
    registerClass(new VolumeSelector());
    registerClass(new VolumeSourceProcessor());
    registerClass(new VolumeCollectionSourceProcessor());

    registerClass(new OrthogonalSliceRenderer());
    registerClass(new SingleSliceRenderer());
    registerClass(new SliceSequenceRenderer());

    registerClass(new VectorMagnitude());
    registerClass(new VolumeInversion());
    registerClass(new VolumeGradient());
    registerClass(new VolumeSave());

#ifdef VRN_MODULE_FLOWREEN
    registerClass(new FlowMagnitudes3D());
    registerClass(new FlowSliceRenderer2D());
    registerClass(new FlowSliceRenderer3D());
    registerClass(new FlowStreamlinesTexture3D());
    registerClass(new PathlineRenderer3D());
    registerClass(new StreamlineRenderer3D());
#endif

    // Sort knownClasses_ in alphabetical order. For the vector contains pairs
    // of strings, the binary predicate has to be a user defined one, because
    // pairs usually posses no weak strict order.
    //
    std::sort(knownClasses_.begin(), knownClasses_.end(), KnownClassesOrder());
}

} // namespace voreen
