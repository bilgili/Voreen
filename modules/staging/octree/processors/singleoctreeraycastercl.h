/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#ifndef VRN_SINGLEOCTREERAYCASTERCL_H
#define VRN_SINGLEOCTREERAYCASTERCL_H

#include "voreen/core/processors/renderprocessor.h"

#include "modules/opencl/utils/clwrapper.h"
#include "modules/opencl/properties/openclproperty.h"

#include "voreen/core/ports/volumeport.h"
#include "voreen/core/ports/geometryport.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/transfuncproperty.h"

#include "voreen/core/datastructures/meta/realworldmappingmetadata.h"

namespace voreen {

class VolumeOctreeBase;
class VolumeOctreeNode;
class MeshListGeometry;
class MeshGeometry;
class TransFunc1DKeys;

class SingleOctreeRaycasterCL : public RenderProcessor {
public:
    SingleOctreeRaycasterCL();
    virtual ~SingleOctreeRaycasterCL();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "SingleOctreeRaycasterCL"; }
    virtual std::string getCategory() const   { return "Octree/Rendering";        }
    virtual CodeState getCodeState() const    { return CODE_STATE_EXPERIMENTAL;   }

    virtual bool isReady() const;

protected:
    virtual void setDescriptions() {
        setDescription("Performs an out-of-core octree-based volume rendering using OpenCL. "
                       "The implementation is based on the GigaVoxel technique [Crassin et al., SIGGRAPH 2009]. "
                       "<p>See also: OctreeCreator</p>");
    }

    virtual void process();
    virtual void beforeProcess();
    virtual void afterProcess();

    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

private:
    void rebuildProgram();

    /**
     * Copies over the content of the srcPort to the destPort,
     * thereby implicitly rescaling the image to the dest dimensions.
     * To be used by subclasses for implementing coarseness (i.e., rendering with reduced dimensions in interaction mode).
     *
     * The entry/exit point ports are used for transforming the srcPortDepth values that are in ray coordinates (linear [0;1]).
     */
    void rescaleRendering(RenderPort& srcPort, RenderPort& srcPortDepth,
        RenderPort& entryPoints, RenderPort& exitPoints, RenderPort& destPort);

    void initializeNodeBuffer();
    void initializeBrickBuffer();

    void updateBrickBufferIncremental();

    void updateBrickBufferComplete();

    void analyzeBrickFlagBuffer(size_t& numTraversed, size_t& numUsed, size_t& numRequested) const;

    void compareNodeToBuffer(const VolumeOctreeNode* node, size_t nodeOffset) const throw (VoreenException);

    void renderVolume();

    void regenerateBuffers();

    VolumePort volumeInport_;
    RenderPort entryPointsInport_;
    RenderPort exitPointsInport_;

    RenderPort renderOutport_;
    RenderPort internalRenderPort_;      ///< used for coarseness (rendering with reduced resolution in interaction mode).
    RenderPort internalRenderPortDepth_; ///< necessary because OpenGL-OpenCL sharing does not work with depth textures

    TransFuncProperty transferFunc_;

    FloatProperty samplingRate_;
    BoolProperty adaptiveSampling_;

    IntProperty interactionCoarseness_;
    IntProperty interactionNodeLevelReduction_;

    BoolProperty useBricks_;
    StringOptionProperty textureFilterMode_;
    IntProperty brickBufferSizeMB_;
    // BoolProperty updateBrickBufferIncrementally_;
    BoolProperty uploadBricksDuringInteraction_;
    BoolProperty useAncestorNodes_;

    OpenCLProperty openclKernel_;
    CameraProperty cameraProperty_;

    uint64_t* nodeBuffer_;
    uint16_t* brickBuffer_;
    uint8_t* brickFlagBuffer_;
    size_t brickBufferSize_;        //< size of brick buffer in bytes
    size_t numBrickBufferSlots_;    //< number of bricks that can be stored in brick buffer

    cl::Buffer* nodeBufferCL_;
    cl::Buffer* brickBufferCL_;
    cl::Buffer* brickFlagBufferCL_;
    bool regenerateBuffers_;

    bool nextRenderPassNeeded_;

    // maps from addresses within the node buffer to the corresponding node
    std::map<size_t, const VolumeOctreeNode*> nodeBufferToNodeMap_;

    // LRU list of used bricks: storing the least recently brick at the tail
    std::list<size_t> leastRecentlyUsedBricks_;

    const VolumeOctreeBase* currentOctree_;

    tgt::Shader* rescaleShader_; ///< Shader used by the rescaleRendering() method

    static const std::string loggerCat_;
};

} //namespace

#endif
