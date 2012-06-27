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

#include "voreen/core/vis/clippingplanewidget.h"
#include "voreen/core/vis/processors/entryexitpoints/entryexitpoints.h"
#include "voreen/core/vis/processors/geometrytestprocessor.h"
#include "voreen/core/vis/processors/geometry/geometryprocessor.h"
#include "voreen/core/vis/processors/geometry/pointlistrenderer.h"
#include "voreen/core/vis/processors/image/background.h"
#include "voreen/core/vis/processors/image/blur.h"
#include "voreen/core/vis/processors/image/cacherenderer.h"
#include "voreen/core/vis/processors/image/canvasrenderer.h"
#include "voreen/core/vis/processors/image/coarsenessrenderer.h"
#include "voreen/core/vis/processors/image/colordepth.h"
#include "voreen/core/vis/processors/image/combine.h"
#include "voreen/core/vis/processors/image/compositer.h"
#include "voreen/core/vis/processors/image/crosshair.h"
#include "voreen/core/vis/processors/image/depthmask.h"
#include "voreen/core/vis/processors/image/depthoffield.h"
#include "voreen/core/vis/processors/image/edgedetect.h"
#include "voreen/core/vis/processors/image/glow.h"
#include "voreen/core/vis/processors/image/labeling.h"
#include "voreen/core/vis/processors/image/nullrenderer.h"
#include "voreen/core/vis/processors/image/regionmodifier.h"
#include "voreen/core/vis/processors/image/renderstore.h"
#include "voreen/core/vis/processors/image/threshold.h"
#include "voreen/core/vis/processors/proxygeometry/axialsliceproxygeometry.h"
#include "voreen/core/vis/processors/proxygeometry/cubecutproxygeometry.h"
#include "voreen/core/vis/processors/proxygeometry/cubeproxygeometry.h"
#include "voreen/core/vis/processors/proxygeometry/multipleaxialsliceproxygeometry.h"
#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"
#include "voreen/core/vis/processors/proxygeometry/sliceproxygeometry.h"
#include "voreen/core/vis/processors/proxygeometry/slicingproxygeometry.h"
#include "voreen/core/vis/processors/render/firsthitrenderer.h"
#include "voreen/core/vis/processors/render/idraycaster.h"
#include "voreen/core/vis/processors/render/segmentationraycaster.h"
#include "voreen/core/vis/processors/render/simpleraycaster.h"
#include "voreen/core/vis/processors/render/singleslicerenderer.h"
#include "voreen/core/vis/processors/render/singlevolumeraycaster.h"
#include "voreen/core/vis/processors/render/sliceentrypoints.h"
#include "voreen/core/vis/processors/render/slicesequencerenderer.h"
#include "voreen/core/vis/processors/render/volumeeditor.h"
#include "voreen/core/vis/processors/volume/regiongrowing.h"
#include "voreen/core/vis/processors/volume/volumeinversion.h"
#include "voreen/core/vis/processors/volumeselectionprocessor.h"
#include "voreen/core/vis/processors/volumesetsourceprocessor.h"

namespace voreen {

ProcessorFactory* ProcessorFactory::instance_ = 0;

ProcessorFactory::ProcessorFactory() {
    initializeClassList();
}

ProcessorFactory::~ProcessorFactory() {
    for (std::map <Identifier, Processor*>::iterator it = classList_.begin();
         it != classList_.end();
         ++it)
    {
        delete it->second;
    }
}

ProcessorFactory* ProcessorFactory::getInstance() {
    if (!instance_)
        instance_ = new ProcessorFactory();

    return instance_;
}

std::string ProcessorFactory::getProcessorInfo(Identifier name) {
    std::map<Identifier, Processor*>::iterator it = classList_.find(name);
    if (it != classList_.end() && it->second != 0)
        return it->second->getProcessorInfo();

    return "";
}

void ProcessorFactory::destroy() {
    delete instance_;
    instance_ = 0;
}

Processor* ProcessorFactory::create(Identifier name) {
    std::map<Identifier, Processor*>::iterator it = classList_.find(name);
    if (it != classList_.end() && it->second != 0)
        return it->second->create();

    return 0;
}

const std::vector<Identifier>& ProcessorFactory::getKnownClasses() {
    return knownClasses_;
}

void ProcessorFactory::registerClass(Processor* newClass) {
    Identifier id  = newClass->getClassName().getName();

    classList_.insert(std::make_pair(id.getSubString(1), newClass));
    knownClasses_.push_back(newClass->getClassName());

    // sort knownClasses_ in alphabetical order
    std::sort(knownClasses_.begin(), knownClasses_.end(), std::less<voreen::Identifier>());
}

void ProcessorFactory::initializeClassList() {

    registerClass(new AxialSliceProxyGeometry());
    registerClass(new CubeCutProxyGeometry());
    registerClass(new CubeProxyGeometry());
    registerClass(new MultipleAxialSliceProxyGeometry());
    registerClass(new SliceProxyGeometry());

    registerClass(new EntryExitPoints());
    registerClass(new SliceEntryPoints());

    registerClass(new Background());
    registerClass(new Blur());
    registerClass(new ColorDepth());
    registerClass(new Combine());
    registerClass(new Compositer());
    registerClass(new CrossHair());
    registerClass(new DepthMask());
    registerClass(new DepthOfField());
    registerClass(new EdgeDetect());
    registerClass(new FirstHitRenderer());
    registerClass(new Glow());
    registerClass(new IDRaycaster());
    registerClass(new Labeling());
    registerClass(new RegionModifier());
    registerClass(new SegmentationRaycaster());
    registerClass(new SimpleRaycaster());
    registerClass(new SingleVolumeRaycaster());
    registerClass(new Threshold());
    registerClass(new VolumeEditor());

    registerClass(new GeomBoundingBox());
    registerClass(new GeomLightWidget());
    registerClass(new GeometryProcessor());
    registerClass(new PointListRenderer());

    registerClass(new ClippingPlaneWidget());

    registerClass(new CoarsenessRenderer());
    registerClass(new CanvasRenderer());
    registerClass(new CacheRenderer());
    registerClass(new NullRenderer());
    registerClass(new RenderStore());

    registerClass(new VolumeSetSourceProcessor());
    registerClass(new VolumeSelectionProcessor());

    registerClass(new SingleSliceRenderer());
    registerClass(new SliceSequenceRenderer());

    registerClass(new RegionGrowingProcessor());
    registerClass(new VolumeInversion());
}

} // namespace voreen
