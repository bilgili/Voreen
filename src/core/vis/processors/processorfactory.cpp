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

#include "voreen/core/vis/processors/volumesetsourceprocessor.h"
#include "voreen/core/vis/processors/volumeselectionprocessor.h"
#include "voreen/core/vis/processors/geometrytestprocessor.h"
#include "voreen/core/vis/processors/render/slicingproxygeometry.h"
#include "voreen/core/vis/processors/render/slicerenderer.h"
#ifndef VRN_SNAPSHOT
#include "voreen/core/vis/processors/render/registrationraycaster.h"
#include "voreen/core/vis/processors/render/rgbaraycaster.h"
#include "voreen/core/vis/processors/render/rgbraycaster.h"
#include "voreen/core/vis/processors/render/fancyraycaster.h"
#include "voreen/core/vis/processors/image/glow.h"
#include "voreen/core/vis/processors/image/crosshair.h"
#include "voreen/core/vis/processors/image/normalestimation.h"
#include "voreen/core/vis/virtualclipping.h"
#include "voreen/core/vis/geomvirtualclippingwidget.h"
#endif
#include "voreen/core/vis/processors/render/simpleraycaster.h"
#include "voreen/core/vis/processors/render/idraycaster.h"
#include "voreen/core/vis/processors/render/proxygeometry.h"
#include "voreen/core/vis/processors/image/canvasrenderer.h"
#include "voreen/core/vis/processors/image/coarsenessrenderer.h"
#include "voreen/core/vis/processors/image/background.h"
#include "voreen/core/vis/processors/image/geometryprocessor.h"
#include "voreen/core/vis/processors/render/entryexitpoints.h"

#include "voreen/core/vis/geomclippingwidget.h"

#include "voreen/core/vis/processors/image/blur.h"
#include "voreen/core/vis/processors/image/regionmodifier.h"
#include "voreen/core/vis/processors/image/threshold.h"
#include "voreen/core/vis/processors/image/depthmask.h"
#include "voreen/core/vis/processors/image/colordepth.h"
#include "voreen/core/vis/processors/image/depthoffield.h"
#include "voreen/core/vis/processors/image/combine.h"
#include "voreen/core/vis/processors/image/edgedetect.h"
#include "voreen/core/vis/processors/image/labeling.h"

#include "voreen/core/vis/processors/volume/visiblehumandatasetcreator.h"


#include <algorithm> // for sorting vectors
#include <functional>

#ifdef VRN_MODULE_CLOSEUPS
#include "voreen/modules/closeups/closeuprenderer.h"
#include "voreen/modules/closeups/overviewraycaster.h"
#include "voreen/modules/closeups/overviewentryexitpoints.h"
#endif

#ifdef VRN_MODULE_CUDA
#include "voreen/modules/cuda/cudaraycaster.h"
#endif

#ifdef VRN_MODULE_DEFORMATION
#include "voreen/modules/deformation/eepdeformation.h"
#include "voreen/modules/deformation/volumeproxygeometry.h"
#endif

#ifdef VRN_MODULE_GLYPHS
#include "voreen/modules/glyphs/glyphrenderer.h"
#include "voreen/modules/glyphs/glyphsentryexitpoints.h"
#include "voreen/modules/glyphs/glyphs.h"
#include "voreen/modules/glyphs/glyphplacing.h"
#endif

#ifdef VRN_MODULE_GLYPHS_MESH
#include "voreen/modules/glyphs_mesh/meshrenderer.h"
#include "voreen/modules/glyphs_mesh/glyphplacingmesh.h"
#endif

#ifdef VRN_MODULE_SHADOWS
#include "voreen/modules/shadows/lightentryexitpoints.h"
#endif

#ifdef VRN_MODULE_SKETCHTF
#include "voreen/modules/sketchtf/sketchraycaster.h"
#include "voreen/modules/sketchtf/sketchrenderer.h"
#endif

#ifdef VRN_MODULE_SPEEDLINES
#include "voreen/modules/speedlines/speedlinerenderer.h"
#endif

namespace voreen {

ProcessorFactory* ProcessorFactory::instance_ = 0;
    
ProcessorFactory::ProcessorFactory() {
    tc_ = 0; 
    initializeClassList();
}


ProcessorFactory::~ProcessorFactory() {
    std::map <Identifier, Processor*>::iterator it;
    for (it = classList_.begin(); it != classList_.end(); ++it)
        delete it->second;
}

ProcessorFactory* ProcessorFactory::getInstance() {
    if (!instance_) {
        instance_ = new ProcessorFactory();
    }
    return instance_;
}

std::string ProcessorFactory::getProcessorInfo(Identifier name) {
	std::map <Identifier, Processor*>::iterator it = classList_.begin();
    while (it != classList_.end()) {
        if (it->first == name)
            return classList_[it->first]->getProcessorInfo();
        it++;  
    }
    return 0;

}

void ProcessorFactory::destroy() {
    delete instance_;
    instance_ = 0;
}

Processor* ProcessorFactory::create(Identifier name) {
    std::map <Identifier, Processor*>::iterator it = classList_.begin();
    while (it != classList_.end()) {
        if (it->first == name)
            return classList_[it->first]->create();
        it++;  
    }
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
    registerClass(new CubeCutProxyGeometry());
      
    registerClass(new CubeEntryExitPoints());

    registerClass(new SimpleRaycaster());
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
    registerClass(new CopyToScreenRenderer());     
    registerClass(new OutputProcessor());

    registerClass(new VolumeSetSourceProcessor());
    registerClass(new VolumeSelectionProcessor());

    registerClass(new SliceRenderer3D());
    registerClass(new SingleSliceRenderer());

#ifndef VRN_SNAPSHOT
    registerClass(new VirtualClipvolume());
    registerClass(new FancyRaycaster());
    registerClass(new RegistrationRaycaster());
	registerClass(new RGBARaycaster() );
	registerClass(new RGBRaycaster());    
    registerClass(new Glow());
    registerClass(new CrossHair());
    registerClass(new NormalEstimation());
    registerClass(new VirtualClippingWidget());
    registerClass(new SlicingProxyGeometry());
#endif

#ifdef VRN_MODULE_CLOSEUPS
    registerClass(new CloseupRenderer());
    registerClass(new OverviewRaycaster());
    registerClass(new OverviewEntryExitPoints());
#endif

#ifdef VRN_MODULE_DEFORMATION
    registerClass(new TexCoordEntryExitPoints());
    registerClass(new VertexEntryExitPoints());
    registerClass(new VolumeProxyGeometry());
#endif

#ifdef VRN_MODULE_GLYPHS
    registerClass(new GlyphsEntryExitPoints());
    registerClass(new GlyphRenderer());
    registerClass(new Glyphs());
    registerClass(new GlyphPlacingGrid());
    registerClass(new GlyphPlacingIsosurface());
#endif

#ifdef VRN_MODULE_GLYPHS_MESH
    registerClass(new MeshRenderer());
    registerClass(new GlyphPlacingMeshGrid());
    registerClass(new GlyphPlacingMeshSurface());
#endif

#ifdef VRN_MODULE_SHADOWS
    registerClass(new LightEntryExitPoints());
    registerClass(new ShadowGen());
    //registerClass(new ShadowMap());
#endif

#ifdef VRN_MODULE_SKETCHTF    
    registerClass(new SketchRaycaster());
    registerClass(new SketchRenderer());
#endif

#ifdef VRN_MODULE_SPEEDLINES
    registerClass(new VectorfieldRenderer());
    registerClass(new SpeedlineRenderer());
#endif

    // Add the processors that use the static-initialization mechanism using the
    // VRN_PROCESSOR_CLASS() and VRN_PROCESSOR_REGISTER() macros. This also takes care of
    // freeing the objects.
    std::vector<Processor*> l = Processor::getRegisteredProcessors();
    for (size_t i=0; i < l.size(); i++) {
        registerClass(l[i]);
    }   
}

void ProcessorFactory::setTextureContainer(voreen::TextureContainer* tc){
    tc_ = tc;
}

voreen::TextureContainer* ProcessorFactory::getTextureContainer(){
    return tc_;
}

} // namespace
