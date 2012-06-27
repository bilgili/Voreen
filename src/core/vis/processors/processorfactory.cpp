/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/geomclippingwidget.h"
#include "voreen/core/vis/processors/geometrytestprocessor.h"
#include "voreen/core/vis/processors/volumesetsourceprocessor.h"
#include "voreen/core/vis/processors/volumeselectionprocessor.h"
#include "voreen/core/vis/processors/image/background.h"
#include "voreen/core/vis/processors/image/blur.h"
#include "voreen/core/vis/processors/image/canvasrenderer.h"
#include "voreen/core/vis/processors/image/coarsenessrenderer.h"
#include "voreen/core/vis/processors/image/colordepth.h"
#include "voreen/core/vis/processors/image/combine.h"
#include "voreen/core/vis/processors/image/depthmask.h"
#include "voreen/core/vis/processors/image/depthoffield.h"
#include "voreen/core/vis/processors/image/edgedetect.h"
#include "voreen/core/vis/processors/image/geometryprocessor.h"
#include "voreen/core/vis/processors/image/labeling.h"
#include "voreen/core/vis/processors/image/regionmodifier.h"
#include "voreen/core/vis/processors/image/threshold.h"
#include "voreen/core/vis/processors/render/entryexitpoints.h"
#include "voreen/core/vis/processors/render/sliceentrypoints.h"
#include "voreen/core/vis/processors/render/idraycaster.h"
#include "voreen/core/vis/processors/render/proxygeometry.h"
#include "voreen/core/vis/processors/render/sliceproxygeometry.h"
#include "voreen/core/vis/processors/render/simpleraycaster.h"
#include "voreen/core/vis/processors/render/singlevolumeraycaster.h"
#include "voreen/core/vis/processors/render/firsthitrenderer.h"
#include "voreen/core/vis/processors/render/slicingproxygeometry.h"
#include "voreen/core/vis/processors/render/slicerenderer.h"
#include "voreen/core/vis/processors/volume/visiblehumandatasetcreator.h"

namespace voreen {

ProcessorFactory* ProcessorFactory::instance_ = 0;
    
ProcessorFactory::ProcessorFactory() {
    tc_ = 0; 
    initializeClassList();
}


ProcessorFactory::~ProcessorFactory() {
    for (std::map <Identifier, Processor*>::iterator it = classList_.begin();
        it != classList_.end();
        ++it)
    {
        delete it->second;
    }
    classList_.clear();
}

ProcessorFactory* ProcessorFactory::getInstance() {
    if (!instance_)
        instance_ = new ProcessorFactory();
    
    return instance_;
}

std::string ProcessorFactory::getProcessorInfo(Identifier name) {
    std::map<Identifier, Processor*>::iterator it = classList_.find(name);
    if ( (it != classList_.end()) && (it->second != 0) )
        return it->second->getProcessorInfo();
    
    return "";
}

void ProcessorFactory::destroy() {
    delete instance_;
    instance_ = 0;
}

Processor* ProcessorFactory::create(Identifier name) {
    std::map<Identifier, Processor*>::iterator it = classList_.find(name);
    if ( (it != classList_.end()) && (it->second != 0) )
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
    //
    // This way is maybe the most suitable: inserting the processor at the
    // correct position would only take O(n) whereas sort() requires
    // O(n * log(n)) time. But insertion into a vector in the middle
    // also requires O(n).... Besides, after initialization, insertion will
    // be probably rare and therefore resorting the entire vector would be
    // fast enough. rfc (Dirk)
    //
    std::sort( knownClasses_.begin(), knownClasses_.end(), std::less<voreen::Identifier>() );
}

void ProcessorFactory::initializeClassList() {
	registerClass(new VisibleHumanDatasetCreator() );

    registerClass(new CubeProxyGeometry());
    registerClass(new SliceProxyGeometry());
    registerClass(new CubeCutProxyGeometry());
      
    registerClass(new CubeEntryExitPoints());
    registerClass(new SliceEntryPoints());

    registerClass(new SimpleRaycaster());
    registerClass(new SingleVolumeRaycaster());
    registerClass(new FirstHitRenderer());
    registerClass(new IDRaycaster());
    registerClass(new RegionModifier());
    registerClass(new Threshold());
    registerClass(new DepthMask());
    registerClass(new ColorDepth());
    registerClass(new DepthOfField());
    registerClass(new Combine());
    registerClass(new Blur());
    registerClass(new EdgeDetect());
    registerClass(new Labeling());
    registerClass(new Background());

    registerClass(new GeomBoundingBox());      
    registerClass(new GeomLightWidget());  
    registerClass(new GeometryProcessor());
    registerClass(new GeomRegistrationMarkers());

    registerClass(new ClippingWidget());

    registerClass(new CoarsenessRenderer());
    registerClass(new CanvasRenderer());
    registerClass(new CacheRenderer());
    registerClass(new NullRenderer());
    registerClass(new CopyToScreenRenderer());     
    registerClass(new OutputProcessor());

    registerClass(new VolumeSetSourceProcessor());
    registerClass(new VolumeSelectionProcessor());

    registerClass(new SliceRenderer3D());
    registerClass(new SingleSliceRenderer());

    // Add the processors that use the static-initialization mechanism using the
    // VRN_PROCESSOR_CLASS() and VRN_PROCESSOR_REGISTER() macros.
    std::vector<Processor::CreateProcessorFunctionPointer> l = Processor::getRegisteredProcessors();
    for (size_t i=0; i < l.size(); ++i) {
        // The method specified by the function pointer returns a new instance of the
        // respective subclass of Processor.
        Processor::CreateProcessorFunctionPointer createStatic = l[i]; 
        registerClass(createStatic());
    }   
}

void ProcessorFactory::setTextureContainer(voreen::TextureContainer* tc){
    tc_ = tc;
}

voreen::TextureContainer* ProcessorFactory::getTextureContainer(){
    return tc_;
}

} // namespace
