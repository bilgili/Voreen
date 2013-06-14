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

#include "singleoctreeraycastercl.h"

#include "../datastructures/volumeoctree.h"
#include "../datastructures/octreeutils.h"

#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/geometry/meshgeometry.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorresizepoweroftwo.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorconvert.h"
#include "voreen/core/datastructures/volume/volumedecorator.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/core/datastructures/meta/realworldmappingmetadata.h"

#include "voreen/core/voreenapplication.h"

#include "modules/opencl/openclmodule.h"

#include "tgt/vector.h"
#include "tgt/tgt_math.h"
#include "tgt/stopwatch.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include <queue>
#include <stack>

using tgt::ivec2;
using tgt::ivec3;
using tgt::ivec4;
using tgt::vec2;
using tgt::vec3;
using tgt::vec4;
using tgt::svec3;

namespace {

// 64-bit masks for node buffer entries
const uint64_t MASK_INBRICKPOOL = 0x8000000000000000;  //< 10000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
const size_t   MASK_INBRICKPOOL_SHIFT  = 63;
const size_t   MASK_INBRICKPOOL_NUMBITS = 1;

const uint64_t MASK_HOMOGENEOUS = 0x4000000000000000;  //< 01000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
const size_t   MASK_HOMOGENEOUS_SHIFT  = 62;
const size_t   MASK_HOMOGENEOUS_NUMBITS = 1;

const uint64_t MASK_CHILD =       0x3FFFFFFF00000000;  //< 00111111 11111111 11111111 11111111 00000000 00000000 00000000 00000000
const size_t   MASK_CHILD_SHIFT  =  32;
const size_t   MASK_CHILD_NUMBITS = 30;

const uint64_t MASK_AVG =         0x00000000FFF00000;  //< 00000000 00000000 00000000 00000000 11111111 11110000 00000000 00000000
const size_t   MASK_AVG_SHIFT  =  20;
const size_t   MASK_AVG_NUMBITS = 12;

const uint64_t MASK_BRICK =       0x00000000000FFFFF;  //< 00000000 00000000 00000000 00000000 00000000 00001111 11111111 11111111
const size_t   MASK_BRICK_SHIFT  =  0;
const size_t   MASK_BRICK_NUMBITS = 20;

// 8-bit masks for node flag buffer entries
const uint8_t MASK_BRICK_USED =      1;  //< 00000001
const uint8_t MASK_BRICK_REQUESTED = 2;  //< 00000010
const uint8_t MASK_NODE_TRAVERSED  = 4;  //< 00000100 (meaning: node has been traversed by ray, but brick has not necessarily been used)

// node buffer access functions

inline uint64_t createNodeBufferEntry(bool inPool, bool homogeneous, size_t childPointer, uint16_t avgValue, size_t brickPointer) {
    uint64_t result = 0;
    result |= (inPool ? 1LL : 0LL)      << MASK_INBRICKPOOL_SHIFT;
    result |= (homogeneous ? 1LL : 0LL) << MASK_HOMOGENEOUS_SHIFT;
    result |= ((uint64_t)childPointer   << MASK_CHILD_SHIFT) & MASK_CHILD;
    result |= ((uint64_t)avgValue       << MASK_AVG_SHIFT  ) & MASK_AVG;
    result |= ((uint64_t)brickPointer   << MASK_BRICK_SHIFT) & MASK_BRICK_SHIFT;
    return result;
}

inline void extractNodeValues(const uint64_t nodeEntry, bool& inPool, bool& homogeneous, size_t& childPointer, uint16_t& avgValue, size_t& brickPointer) {
    inPool =      ((nodeEntry & MASK_INBRICKPOOL) >> MASK_INBRICKPOOL_SHIFT) == 1;
    homogeneous = ((nodeEntry & MASK_HOMOGENEOUS) >> MASK_HOMOGENEOUS_SHIFT) == 1;
    childPointer = (nodeEntry & MASK_CHILD) >> MASK_CHILD_SHIFT;
    avgValue =     (nodeEntry & MASK_AVG)   >> MASK_AVG_SHIFT;
    brickPointer = (nodeEntry & MASK_BRICK) >> MASK_BRICK_SHIFT;
}

inline void updateNodeChildPointer(uint64_t& nodeEntry, size_t childPointer) {
    // clear old value
    nodeEntry &= ~MASK_CHILD;
    // set new one
    nodeEntry |= (((uint64_t)childPointer) << MASK_CHILD_SHIFT) & MASK_CHILD;
}

inline void updateNodeBrickPointer(uint64_t& nodeEntry, bool hasBrick, size_t brickPointer) {
    // has brick
    nodeEntry &= ~MASK_INBRICKPOOL;
    nodeEntry |= (((uint64_t)(hasBrick ? 1 : 0)) << MASK_INBRICKPOOL_SHIFT) & MASK_INBRICKPOOL;

    // brick pointer
    nodeEntry &= ~MASK_BRICK;
    nodeEntry |= (((uint64_t)brickPointer) << MASK_BRICK_SHIFT) & MASK_BRICK;
}

inline size_t getNodeBrickPointer(uint64_t& nodeEntry) {
    return (nodeEntry & MASK_BRICK) >> MASK_BRICK_SHIFT;
}

inline bool hasNodeBrick(uint64_t& nodeEntry) {
    return (nodeEntry & MASK_INBRICKPOOL) > 0;
}

// flag buffer access functions

inline bool hasBrickBeenUsed(const uint8_t flagEntry) {
    return (flagEntry & MASK_BRICK_USED) > 0;
}
inline uint8_t setBrickUsed(const uint8_t flagEntry, const bool used) {
    return (flagEntry & ~MASK_BRICK_USED) | (used ? MASK_BRICK_USED : 0);
}

inline bool hasNodeBeenTraversed(const uint8_t flagEntry) {
    return (flagEntry & MASK_NODE_TRAVERSED) > 0;
}
inline uint8_t setNodeTraversed(const uint8_t flagEntry, const bool traversed) {
    return (flagEntry & ~MASK_NODE_TRAVERSED) | (traversed ? MASK_NODE_TRAVERSED : 0);
}

inline bool isBrickRequested(const uint8_t flagEntry) {
    return (flagEntry & MASK_BRICK_REQUESTED) > 0;
}
inline uint8_t setBrickRequested(const uint8_t flagEntry, const bool requested) {
    return (flagEntry & ~MASK_BRICK_REQUESTED) | (requested ? MASK_BRICK_REQUESTED : 0);
}

//------------------------------------

struct RealWorldMappingCL {
    RealWorldMappingCL(voreen::RealWorldMapping mapping)
        : scale_(mapping.getScale())
        , offset_(mapping.getOffset())
    {}
    float scale_;
    float offset_;
};

}

namespace voreen {

const std::string SingleOctreeRaycasterCL::loggerCat_("voreen.SingleOctreeRaycasterCL");

const int INVALID_BUFFERS = 50;

SingleOctreeRaycasterCL::SingleOctreeRaycasterCL()
    : RenderProcessor()
    , volumeInport_(Port::INPORT, "volumeInput", "Octree Input")
    , entryPointsInport_(Port::INPORT, "entryPointsInport", "Entry-points")
    , exitPointsInport_(Port::INPORT, "exitPointsInport", "Exit-points")
    , renderOutport_(Port::OUTPORT, "renderOutport", "Image Output")
    , internalRenderPort_(Port::OUTPORT, "internalRenderPort", "Internal RenderPort")
    , internalRenderPortDepth_(Port::OUTPORT, "internalRenderPortDepth", "Internal RenderPort Depth"/*, false, INVALID_RESULT, RenderPort::RENDERSIZE_DEFAULT, GL_LUMINANCE32F_ARB*/)
    , transferFunc_("transferFunction", "Transfer function")
    , samplingRate_("samplingRate", "Sampling Rate", 2.f, 0.01f, 20.f)
    , adaptiveSampling_("adaptiveSampling", "Adaptive Sampling", true, Processor::INVALID_PROGRAM)
    , interactionCoarseness_("interactionCoarseness", "Interaction Coarseness", 2, 1, 8, Processor::VALID)
    , interactionNodeLevelReduction_("interactionNodeLevelReduction", "Node Level Reduction", 1, 0, 3)
    , useBricks_("useBricks", "Use Bricks", true, Processor::INVALID_PROGRAM)
    , textureFilterMode_("textureFilterMode", "Texture Filtering", Processor::INVALID_PROGRAM)
    , brickBufferSizeMB_("brickBufferSizeMB", "Brick Buffer Size (MB)", 128, 1, 1024, INVALID_BUFFERS)
    //, updateBrickBufferIncrementally_("updateBrickBufferIncrementally", "Incremental brick buffer update", true)
    , uploadBricksDuringInteraction_("uploadBricksDuringInteracion", "Upload bricks during interaction", true)
    , useAncestorNodes_("useAncestorNodes", "Use Ancestor Nodes", true, Processor::INVALID_PROGRAM)
    , openclKernel_("renderProgram", "OpenCL Kernel",
                    VoreenApplication::app()->getBasePath("modules/staging/octree/processors/cl/singleoctreeraycaster.cl"))
    , cameraProperty_("cameraProperty", "Camera")
    , nodeBuffer_(0)
    , brickBuffer_(0)
    , brickFlagBuffer_(0)
    , brickBufferSize_(0)
    , numBrickBufferSlots_(0)
    , nodeBufferCL_(0)
    , brickBufferCL_(0)
    , brickFlagBufferCL_(0)
    , currentOctree_(0)
    , regenerateBuffers_(false)
    , nextRenderPassNeeded_(false)
{
    addPort(volumeInport_);
    addPort(entryPointsInport_);
    addPort(exitPointsInport_);

    addPort(renderOutport_);
    addPrivateRenderPort(internalRenderPort_);
    addPrivateRenderPort(internalRenderPortDepth_);

    addProperty(transferFunc_);

    addProperty(samplingRate_);
    addProperty(adaptiveSampling_);
    samplingRate_.setGroupID("sampling");
    adaptiveSampling_.setGroupID("sampling");
    setPropertyGroupGuiName("sampling", "Sampling");

    addProperty(interactionCoarseness_);
    addProperty(interactionNodeLevelReduction_);
    interactionCoarseness_.setGroupID("interaction");
    interactionNodeLevelReduction_.setGroupID("interaction");
    setPropertyGroupGuiName("interaction", "Interaction");

    addProperty(useBricks_);
    textureFilterMode_.addOption("nearest", "Nearest");
    textureFilterMode_.addOption("linear",  "Linear");
    textureFilterMode_.selectByKey("linear");
    addProperty(textureFilterMode_);
    addProperty(brickBufferSizeMB_);
    brickBufferSizeMB_.setGroupID("bricking");
    brickBufferSizeMB_.onChange(CallMemberAction<SingleOctreeRaycasterCL>(this, &SingleOctreeRaycasterCL::regenerateBuffers));
    //addProperty(updateBrickBufferIncrementally_);
    addProperty(uploadBricksDuringInteraction_);
    addProperty(useAncestorNodes_);
    useBricks_.setGroupID("bricking");
    textureFilterMode_.setGroupID("bricking");
    brickBufferSizeMB_.setGroupID("bricking");
    //updateBrickBufferIncrementally_.setGroupID("bricking");
    uploadBricksDuringInteraction_.setGroupID("bricking");
    useAncestorNodes_.setGroupID("bricking");
    setPropertyGroupGuiName("bricking", "Brick Handling");

    addProperty(openclKernel_);
    addProperty(cameraProperty_);

    // check that node buffer bitmasks are complementary
    tgtAssert((MASK_INBRICKPOOL & MASK_HOMOGENEOUS) == 0, "bitmasks not complementary");
    tgtAssert((MASK_INBRICKPOOL & MASK_CHILD) == 0, "bitmasks not complementary");
    tgtAssert((MASK_INBRICKPOOL & MASK_AVG) == 0, "bitmasks not complementary");
    tgtAssert((MASK_INBRICKPOOL & MASK_BRICK) == 0, "bitmasks not complementary");

    tgtAssert((MASK_HOMOGENEOUS & MASK_CHILD) == 0, "bitmasks not complementary");
    tgtAssert((MASK_HOMOGENEOUS & MASK_AVG) == 0, "bitmasks not complementary");
    tgtAssert((MASK_HOMOGENEOUS & MASK_BRICK) == 0, "bitmasks not complementary");

    tgtAssert((MASK_CHILD & MASK_AVG) == 0, "bitmasks not complementary");
    tgtAssert((MASK_CHILD & MASK_BRICK) == 0, "bitmasks not complementary");

    tgtAssert((MASK_AVG & MASK_BRICK) == 0, "bitmasks not complementary");

    tgtAssert((MASK_INBRICKPOOL + MASK_HOMOGENEOUS + MASK_CHILD + MASK_AVG + MASK_BRICK) == 0xFFFFFFFFFFFFFFFF,
        "node buffer bitmasks do not use all bits");

    // check flag buffer masks
    tgtAssert((MASK_NODE_TRAVERSED & MASK_BRICK_USED) == 0, "bitmasks not complementary");
    tgtAssert((MASK_NODE_TRAVERSED & MASK_BRICK_REQUESTED) == 0, "bitmasks not complementary");
    tgtAssert((MASK_BRICK_REQUESTED & MASK_BRICK_USED) == 0, "bitmasks not complementary");
}

SingleOctreeRaycasterCL::~SingleOctreeRaycasterCL() {
    delete[] nodeBuffer_;
    nodeBuffer_ = 0;
    delete[] brickBuffer_;
    brickBuffer_ = 0;
    delete[] brickFlagBuffer_;
    brickFlagBuffer_ = 0;
}

Processor* SingleOctreeRaycasterCL::create() const {
    return new SingleOctreeRaycasterCL();
}

void SingleOctreeRaycasterCL::initialize() throw (VoreenException) {
    OpenCLModule::getInstance()->initCL();
    if (!OpenCLModule::getInstance()->getCLContext())
        throw VoreenException("No OpenCL context created");

    RenderProcessor::initialize();

    // load rescale shader program
    rescaleShader_ = ShdrMgr.loadSeparate("passthrough.vert", "copyimage_octreecoarseness.frag", generateHeader(), false);
}

void SingleOctreeRaycasterCL::deinitialize() throw (VoreenException) {
    delete nodeBufferCL_;
    nodeBufferCL_ = 0;
    delete brickBufferCL_;
    brickBufferCL_ = 0;
    delete brickFlagBufferCL_;
    brickFlagBufferCL_ = 0;

    ShdrMgr.dispose(rescaleShader_);
    rescaleShader_ = 0;

    RenderProcessor::deinitialize();
}

bool SingleOctreeRaycasterCL::isReady() const {
    return (volumeInport_.isReady());
}

void SingleOctreeRaycasterCL::beforeProcess() {
    RenderProcessor::beforeProcess();

    if (!volumeInport_.hasData() || !volumeInport_.getData()->hasRepresentation<VolumeOctreeBase>())
        return;

    // compile kernel if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM || !openclKernel_.getProgram()) {
        rebuildProgram();
    }

    if (volumeInport_.hasChanged()) {
        const VolumeBase* sourceVolume = volumeInport_.getData();
        if (sourceVolume) {
            // assign octree source volume to transfer function for extraction of derived data
            transferFunc_.setVolumeHandle(sourceVolume);

            // adapt scene to octree/volume bounding box
            cameraProperty_.adaptInteractionToScene(sourceVolume->getBoundingBox().getBoundingBox(),
                tgt::min(sourceVolume->getSpacing()));
        }
    }

}

void SingleOctreeRaycasterCL::process() {
    if (!volumeInport_.getData()->hasRepresentation<VolumeOctreeBase>()) {
        LWARNING("Input volume has no octree representation. Use OctreeCreator processor!");
        return;
    }
    const VolumeOctreeBase* inputOctree = volumeInport_.getData()->getRepresentation<VolumeOctreeBase>();
    tgtAssert(inputOctree, "no input octree");

    if (volumeInport_.hasChanged()) {
        currentOctree_ = inputOctree;

        initializeNodeBuffer();
        initializeBrickBuffer();
        regenerateBuffers_ = false;

        rebuildProgram();   //< defines depend on octree configuration
    }
    else if (regenerateBuffers_) {
        currentOctree_ = inputOctree;
        initializeNodeBuffer();
        initializeBrickBuffer();
        regenerateBuffers_ = false;
    }

    // perform actual volume rendering
    if (renderOutport_.isConnected() && entryPointsInport_.isReady() && exitPointsInport_.isReady()) {

        // resize outport to inport dimensions
        tgt::svec2 outputRenderSize = entryPointsInport_.getSize();
        if (outputRenderSize != tgt::svec2(renderOutport_.getSize()))
            renderOutport_.resize(outputRenderSize);
        LGL_ERROR;

        // resize internal render port according to coarseness and clear it
        const bool renderCoarse = interactionMode() && interactionCoarseness_.get() > 1;
        const tgt::svec2 internalRenderSize = (renderCoarse ? (renderOutport_.getSize() / interactionCoarseness_.get()) : renderOutport_.getSize());
        if (internalRenderSize != tgt::svec2(internalRenderPort_.getSize()))
            internalRenderPort_.resize(internalRenderSize);
        if (internalRenderSize != tgt::svec2(internalRenderPortDepth_.getSize()))
            internalRenderPortDepth_.resize(internalRenderSize);
        LGL_ERROR;
        internalRenderPort_.activateTarget();
        internalRenderPort_.clearTarget();
        internalRenderPort_.deactivateTarget();
        internalRenderPortDepth_.activateTarget();
        internalRenderPortDepth_.clearTarget();
        internalRenderPortDepth_.deactivateTarget();
        LGL_ERROR;

        renderVolume();

        // copy rendering from internal color and depth buffers to outport (thereby resize it)
        rescaleRendering(internalRenderPort_, internalRenderPortDepth_, entryPointsInport_, exitPointsInport_, renderOutport_);
    }
    else {
        nextRenderPassNeeded_ = false;
    }
}

void SingleOctreeRaycasterCL::afterProcess() {
    RenderProcessor::afterProcess();

    // request further rendering pass, if bricks have been requested during last pass and there are free (unused) brick slots
    if (volumeInport_.hasData() && volumeInport_.getData()->hasRepresentation<VolumeOctreeBase>() && useBricks_.get()) {
        size_t numTraversedNodes, numUsedNodes, numRequestedNodes;
        analyzeBrickFlagBuffer(numTraversedNodes, numUsedNodes, numRequestedNodes);
        LDEBUG("Num traversed/used/requested: " << numTraversedNodes << "/" << numUsedNodes << "/" << numRequestedNodes);
        if (numRequestedNodes > 0 && numUsedNodes < numBrickBufferSlots_)
            invalidate();
    }
}

void SingleOctreeRaycasterCL::initializeNodeBuffer() {
    tgtAssert(volumeInport_.hasData() && volumeInport_.getData()->hasRepresentation<VolumeOctreeBase>(), "no input octree");
    const VolumeOctreeBase* octree = volumeInport_.getData()->getRepresentation<VolumeOctreeBase>();

    const size_t nodeCount = octree->getNumNodes();

    delete nodeBufferCL_;
    nodeBufferCL_ = 0;

    delete[] nodeBuffer_;
    nodeBuffer_ = new uint64_t[nodeCount];
    nodeBufferToNodeMap_.clear();

    tgt::Stopwatch watch;
    watch.start();

    // pair consisting of a octree node whose children still have to be added to the buffer,
    // and the node's buffer offset
    typedef std::pair<const VolumeOctreeNode*, size_t> QuededNode;
    //std::queue<QuededNode> workQueue;
    std::stack<QuededNode> workQueue;

    // start with root node, put encountered nodes into fifo queue, iterate until queue is empty
    const VolumeOctreeNode* rootNode = octree->getRootNode();
    tgtAssert(rootNode, "no root node");
    nodeBuffer_[0] = createNodeBufferEntry(false, !rootNode->hasBrick(), 0, (rootNode->getAvgValue() >> (16-MASK_AVG_NUMBITS)), 0);
    nodeBufferToNodeMap_.insert(std::pair<size_t, const VolumeOctreeNode*>(0, octree->getRootNode()));
    workQueue.push(QuededNode(octree->getRootNode(), 0));
    size_t curOffset = 1; //< next after root node
    while (!workQueue.empty()) {
        // retrieve next node to process
        /*const VolumeOctreeNode* curNode = workQueue.front().first;
        size_t nodeOffset = workQueue.front().second; */
        const VolumeOctreeNode* curNode = workQueue.top().first;
        size_t nodeOffset = workQueue.top().second;
        workQueue.pop();

        // no children => nothing to do
        if (!curNode->children_[0])
            continue;

        tgtAssert(nodeOffset < curOffset,  "node offset not less than current offset");
        tgtAssert(curOffset+8 <= nodeCount, "invalid current buffer offset");

        // set curNode's child pointer to current offset
        updateNodeChildPointer(nodeBuffer_[nodeOffset], curOffset);
        tgtAssert(((nodeBuffer_[nodeOffset] & MASK_CHILD) >> MASK_CHILD_SHIFT) == curOffset, "child pointer not properly set");

        // create eight adjacent buffer entries at curOffset for children, and add them to work queue
        for (size_t childID = 0; childID < 8; childID++) {
            const VolumeOctreeNode* child = curNode->children_[childID];
            tgtAssert(child, "missing child");
            size_t childOffset = curOffset+childID;

            uint16_t childAvgValue = child->getAvgValue() >> (16-MASK_AVG_NUMBITS); //< less than 16 bit available for storing avg value
            //uint16_t childAvgValue = ((child->maxValue_+child->minValue_)/2) >> (16-MASK_AVG_NUMBITS); //< less than 16 bit available for storing avg value
            uint64_t childEntry = createNodeBufferEntry(false, !child->hasBrick(), 0, childAvgValue, 0);
            tgtAssert(((childEntry & MASK_INBRICKPOOL) >> MASK_INBRICKPOOL_SHIFT) == 0, "in-brick-pool value not properly set");
            tgtAssert(((childEntry & MASK_HOMOGENEOUS) >> MASK_HOMOGENEOUS_SHIFT) == (!child->hasBrick()), "homogeneous value not properly set");
            tgtAssert(((childEntry & MASK_CHILD) >> MASK_CHILD_SHIFT) == 0, "child pointer not properly set");
            tgtAssert(((childEntry & MASK_AVG) >> MASK_AVG_SHIFT) == childAvgValue, "node avg value not properly set");
            tgtAssert(((childEntry & MASK_BRICK) >> MASK_BRICK_SHIFT) == 0, "node brick pointer not properly set");
            nodeBuffer_[childOffset] = childEntry;
            nodeBufferToNodeMap_.insert(std::pair<size_t, const VolumeOctreeNode*>(childOffset, child));

            workQueue.push(QuededNode(child, childOffset));
        }
        curOffset += 8;
    }
    tgtAssert(curOffset == nodeCount, "buffer offset does not equal number of tree nodes");
    tgtAssert(nodeBufferToNodeMap_.size() == nodeCount, "nodeBufferToNodeMap size does not equal number of tree nodes");

    LINFO("Node buffer size: " << nodeCount*sizeof(uint64_t) << " bytes");
    LINFO("Node buffer generation time: " << watch.getRuntime() << " ms");

    // cl buffers
    cl::Context* context = OpenCLModule::getInstance()->getCLContext();
    if (!context) {
        LERROR("No CL context");
        return;
    }
    nodeBufferCL_ = new cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, nodeCount*8, nodeBuffer_);


    // validate result
    /*LINFO("Validating node buffer against octree...");
    watch.reset();
    watch.start();
    try {
        compareNodeToBuffer(rootNode, 0);
    }
    catch (VoreenException& e) {
        LERROR(e.what());
    }
    LINFO("Validation time: " << watch.getRuntime() << " ms");*/
}

void SingleOctreeRaycasterCL::initializeBrickBuffer() {
    tgtAssert(volumeInport_.hasData() && volumeInport_.getData()->hasRepresentation<VolumeOctreeBase>(), "no input octree");
    tgtAssert(nodeBuffer_, "no node buffer");

    const VolumeOctreeBase* octree = volumeInport_.getData()->getRepresentation<VolumeOctreeBase>();
    const size_t nodeCount = octree->getNumNodes();
    tgtAssert(nodeBufferToNodeMap_.size() == nodeCount, "nodeBufferToNodeMap has invalid size");

    delete brickBufferCL_;
    brickBufferCL_ = 0;
    delete brickFlagBufferCL_;
    brickFlagBufferCL_ = 0;

    delete[] brickBuffer_;

    numBrickBufferSlots_ = tgt::ifloor((float)(brickBufferSizeMB_.get() << 20) / (float)octree->getBrickMemorySize());
    numBrickBufferSlots_ = std::min(numBrickBufferSlots_, nodeCount);
    brickBufferSize_ = numBrickBufferSlots_ * octree->getNumVoxelsPerBrick(); // brick buffer size in uint16_t
    brickBuffer_ = new uint16_t[brickBufferSize_];


    /*tgt::Stopwatch watch;
    watch.start();

    size_t currentBrickID = 0;
    for (size_t nodeID=0; nodeID<nodeCount; nodeID++) {
        uint64_t nodeEntry = nodeBuffer_[nodeID];
        tgtAssert(nodeBufferToNodeMap_.find(nodeID) != nodeBufferToNodeMap_.end(), "missing nodeBufferToNodeMap entry");
        const uint16_t* brick = octree_->getNodeBrick(nodeBufferToNodeMap_[nodeID]);
        if (brick) {
            size_t brickBufferOffset = currentBrickID*octree_->getNumVoxelsPerBrick();
            memcpy(brickBuffer_+brickBufferOffset, brick, octree_->getBrickMemorySize());
            updateNodeBrickPointer(nodeBuffer_[nodeID], true, currentBrickID);
            tgtAssert(getNodeBrickPointer(nodeBuffer_[nodeID]) == currentBrickID, "node brick pointer not properly stored");
            currentBrickID += 1;
        }
        else {
            updateNodeBrickPointer(nodeBuffer_[nodeID], false, 0);
        }
    }
    // TODO: check currentBrickID == Octree-Bricks
    tgtAssert(currentBrickID <= nodeCount, "brick buffer offset larger than brick buffer size");

    LINFO("Brick buffer generation time: " << watch.getRuntime() << " ms"); */

    // cl buffers
    cl::Context* context = OpenCLModule::getInstance()->getCLContext();
    if (!context) {
        LERROR("No CL context");
        return;
    }
    brickBufferCL_ = new cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, brickBufferSize_*2, brickBuffer_);

    delete[] brickFlagBuffer_;
    brickFlagBuffer_ = new uint8_t[nodeCount];
    memset(brickFlagBuffer_, 0, nodeCount);
    brickFlagBufferCL_ = new cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, nodeCount, brickFlagBuffer_);

    leastRecentlyUsedBricks_.clear();

    LINFO("Brick buffer size: " << brickBufferSize_*2 << " bytes / " << numBrickBufferSlots_ << " brick slots");
    LINFO("Brick flag buffer size: " << nodeCount << " bytes");
}

void SingleOctreeRaycasterCL::updateBrickBufferComplete() {
    tgtAssert(volumeInport_.hasData() && volumeInport_.getData()->hasRepresentation<VolumeOctreeBase>(), "no input octree");
    tgtAssert(nodeBuffer_, "no node buffer");
    tgtAssert(brickBuffer_, "no brick buffer");
    tgtAssert(brickFlagBuffer_, "no flag buffer");

    const VolumeOctreeBase* octree = volumeInport_.getData()->getRepresentation<VolumeOctreeBase>();
    const size_t numNodes = octree->getNumNodes();
    tgtAssert(nodeBufferToNodeMap_.size() == numNodes, "nodeBufferToNodeMap has invalid size");

    // determine number of required brick slots
    size_t numBricksToUpload = 0;
    for (size_t nodeID=0; nodeID<numNodes; nodeID++) {
        uint8_t brickFlag = brickFlagBuffer_[nodeID];
        if (hasBrickBeenUsed(brickFlag) || isBrickRequested(brickFlag))
            numBricksToUpload++;
    }
    if (numBricksToUpload > numBrickBufferSlots_) {
        LWARNING("Number of used/requested bricks exceeds number of brick slots [" << numBricksToUpload << " > " << numBrickBufferSlots_ << "]");
        numBricksToUpload = numBrickBufferSlots_;
    }

    // determine nodes whose bricks are to be uploaded to the GPU
    //
    // Use map from brick address to the corresponding node+its id.
    // This implicitly sorts the bricks to be uploaded by their virtual memory address!
    std::map<uint64_t, std::pair<const VolumeOctreeNode*, size_t> > brickAddressToNodeIDMap;
    size_t numInserted = 0;
    for (size_t nodeID=0; nodeID<numNodes; nodeID++) {
        const uint8_t brickFlag = brickFlagBuffer_[nodeID];
        if ((hasBrickBeenUsed(brickFlag) || isBrickRequested(brickFlag)) && (numInserted<numBricksToUpload)) {
            tgtAssert(nodeBufferToNodeMap_.find(nodeID) != nodeBufferToNodeMap_.end(), "missing nodeBufferToNodeMap entry");
            const VolumeOctreeNode* node = nodeBufferToNodeMap_[nodeID];
            if (node->hasBrick()) {
                brickAddressToNodeIDMap.insert(std::make_pair(node->getBrickAddress(), std::make_pair(node, nodeID)));
                numInserted++;
            }
            else {
                LWARNING("Non existing brick has been requested by GPU: nodeID=" << nodeID);
                updateNodeBrickPointer(nodeBuffer_[nodeID], false, 0);
            }
        }
        else {
            updateNodeBrickPointer(nodeBuffer_[nodeID], false, 0);
        }
    }
    tgtAssert(brickAddressToNodeIDMap.size() == numBricksToUpload, "brickAddressToNodeIDMap has invalid size");

    // update CPU brick buffer
    size_t currentBrickID = 0;
    for (std::map<uint64_t, std::pair<const VolumeOctreeNode*, size_t> >::iterator it = brickAddressToNodeIDMap.begin();
        it != brickAddressToNodeIDMap.end(); ++it)
    {
        const uint64_t brickAddress = it->first;
        const VolumeOctreeNode* node = it->second.first;
        const size_t nodeID = it->second.second;
        tgtAssert(brickAddress < (size_t)-1, "invalid brick address");
        tgtAssert(node, "invalid node");
        tgtAssert(nodeID < numNodes, "invalid node id");

        // retrieve brick
        const uint16_t* brick = 0;
        try {
            brick = octree->getNodeBrick(node);
        }
        catch (VoreenException& e) {
            LWARNING("Failed to retrieve brick: " + std::string(e.what()));
            continue;
        }
        tgtAssert(brick, "no brick returned (exception expected)");

        // copy brick to brick buffer
        size_t brickBufferOffset = currentBrickID*octree->getNumVoxelsPerBrick();
        memcpy(brickBuffer_+brickBufferOffset, brick, octree->getBrickMemorySize());
        updateNodeBrickPointer(nodeBuffer_[nodeID], true, currentBrickID);
        tgtAssert(getNodeBrickPointer(nodeBuffer_[nodeID]) == currentBrickID, "node brick pointer not properly stored");

        currentBrickID += 1;

        octree->releaseNodeBrick(node);
    }
    tgtAssert(currentBrickID == numBricksToUpload, "invalid current brick id")
    float uploadedMB = (float)numBricksToUpload*octree->getNumVoxelsPerBrick()*2 / (1024.f*1024.f);
    LINFO("GPU bricks uploaded: " << numBricksToUpload << " (" << ftos(uploadedMB, 2) << " MB)");

    // upload CPU brick buffer to GPU
    cl::Context* context = OpenCLModule::getInstance()->getCLContext();
    cl::CommandQueue* commandQueue = OpenCLModule::getInstance()->getCLCommandQueue();
    tgtAssert(context, "No OpenCL context");
    tgtAssert(commandQueue, "No OpenCL command queue");
    commandQueue->enqueueWriteBuffer(brickBufferCL_, brickBuffer_, true);
}

void SingleOctreeRaycasterCL::updateBrickBufferIncremental() {
    tgtAssert(volumeInport_.hasData() && volumeInport_.getData()->hasRepresentation<VolumeOctreeBase>(), "no input octree");
    tgtAssert(nodeBuffer_, "no node buffer");
    tgtAssert(brickBuffer_, "no brick buffer");
    tgtAssert(brickBufferSize_ > 0, "brick buffer size is 0");
    tgtAssert(numBrickBufferSlots_ > 0, "num brick buffer slots is 0");
    tgtAssert(brickFlagBuffer_, "no flag buffer");

    const VolumeOctreeBase* octree = volumeInport_.getData()->getRepresentation<VolumeOctreeBase>();
    const size_t numNodes = octree->getNumNodes();
    const size_t numVoxelsPerBrick = octree->getNumVoxelsPerBrick();
    const size_t brickMemorySize = octree->getBrickMemorySize();
    tgtAssert(nodeBufferToNodeMap_.size() == numNodes, "nodeBufferToNodeMap has invalid size");

    tgt::Stopwatch watch;
    watch.start();

    //
    // 1. Update LRU list:
    // - move used bricks that have been used during last rendering pass to front
    // - determine number of bricks used in last pass
    tgt::Stopwatch lruWatch;
    lruWatch.start();
    size_t numUsedInLastPass = 0;
    for (std::list<size_t>::iterator it = leastRecentlyUsedBricks_.begin(); it!=leastRecentlyUsedBricks_.end(); ) {
        const size_t nodeID = *it;
        tgtAssert(nodeID < numNodes, "invalid brick id");
        if (hasBrickBeenUsed(brickFlagBuffer_[nodeID])) {
            numUsedInLastPass++;
            it = leastRecentlyUsedBricks_.erase(it);
            leastRecentlyUsedBricks_.push_front(nodeID);
        }
        else {
            ++it;
        }
    }
    lruWatch.stop();

    // determine number of requested bricks
    size_t numRequested = 0;
    for (size_t nodeID = 0; nodeID < numNodes; nodeID++) {
        if (isBrickRequested(brickFlagBuffer_[nodeID]))
            numRequested++;
    }

    // no bricks requested => keep brick buffer as it is
    if (numRequested == 0) {
        #ifdef VRN_VOLUMEOCTREE_DEBUG
        LINFO("Update brick buffer time: " << watch.getRuntime() << " ms " << "(LRU update: " << lruWatch.getRuntime() << " ms)");
        #endif
        return;
    }


    //
    // 2. Determine brick slots:
    // - that are in use (have a GPU node pointing to them)
    //
    bool* brickSlotUsed = new bool[numBrickBufferSlots_];
    memset(brickSlotUsed, 0, numBrickBufferSlots_);  //< initialize with false
    size_t numSlotsOccupied = 0;
    for (size_t nodeID=0; nodeID<numNodes; nodeID++) {
        if (hasNodeBrick(nodeBuffer_[nodeID])) {
            size_t brickID = getNodeBrickPointer(nodeBuffer_[nodeID]);
            tgtAssert(brickID < numBrickBufferSlots_, "invalid brick id");
            tgtAssert(!brickSlotUsed[brickID], "brick slot already marked as used");
            brickSlotUsed[brickID] = true;
            numSlotsOccupied++;
        }
    }
    tgtAssert(numSlotsOccupied == leastRecentlyUsedBricks_.size(), "number of occupied slots does not match size of LRU list");

    //
    // 3. If not enough free capacity, free brick slots that have not been used during last rendering pass
    //
    tgtAssert(numSlotsOccupied >= numUsedInLastPass, "invalid number of occupied slots");
    size_t numFreeBrickSlots = numBrickBufferSlots_ - numSlotsOccupied;
    size_t numRemoved = 0;
    if (numRequested > numFreeBrickSlots) {
        size_t numUnusedInLastPass = numSlotsOccupied - numUsedInLastPass;
        size_t numToRemove = numRequested - numFreeBrickSlots;
        if (numToRemove > numUnusedInLastPass) {
            numToRemove = numUnusedInLastPass;
            LWARNING("Number of requested bricks exceeds number of available brick slots [" <<
                numRequested << " > " << (numFreeBrickSlots+numUnusedInLastPass) << "]" << "");
        }
        tgtAssert(numToRemove <= leastRecentlyUsedBricks_.size(), "more bricks to removed than LRU buffer contains elements");

        // remove numToRemove elements from tail of LRU list
        for (size_t i=0; i<numToRemove; i++) {
            size_t nodeID = leastRecentlyUsedBricks_.back();
            leastRecentlyUsedBricks_.pop_back();

            size_t brickID = getNodeBrickPointer(nodeBuffer_[nodeID]);
            tgtAssert(brickID < numBrickBufferSlots_, "invalid brick id");
            tgtAssert(brickSlotUsed[brickID], "brick slot not marked as used");
            updateNodeBrickPointer(nodeBuffer_[nodeID], false, 0);
            brickSlotUsed[brickID] = false;
            numFreeBrickSlots++;
            numRemoved++;
            numSlotsOccupied--;
        }
        /*for (size_t nodeID=0; nodeID<numNodes && numRemoved < numToRemove; nodeID++) {
            if (hasNodeBrick(nodeBuffer_[nodeID])) {
                if (!hasBrickBeenUsed(brickFlagBuffer_[nodeID])) {
                    size_t brickID = getNodeBrickPointer(nodeBuffer_[nodeID]);
                    tgtAssert(brickID < numBrickBufferSlots_, "invalid brick id");
                    tgtAssert(brickSlotUsed[brickID], "brick slot not marked as used");
                    updateNodeBrickPointer(nodeBuffer_[nodeID], false, 0);
                    brickSlotUsed[brickID] = false;
                    numFreeBrickSlots++;
                    numRemoved++;
                    numSlotsOccupied--;
                }
            }
        } */

        tgtAssert(numRemoved == numToRemove, "removed too few bricks from buffer");
    }
    // still no free brick slots => cannot upload any requested bricks
    if (numFreeBrickSlots == 0) {
        delete[] brickSlotUsed;
        #ifdef VRN_VOLUMEOCTREE_DEBUG
        LINFO("Update brick buffer time: " << watch.getRuntime() << " ms " << "(LRU update: " << lruWatch.getRuntime() << " ms)");
        #endif
        return;
    }


    //
    // 4. Determine nodes whose bricks are to be uploaded to the GPU
    //
    // Use map from brick address to the corresponding node+its id.
    // This implicitly sorts the bricks to be uploaded by their virtual memory address!
    //
    const size_t numBricksToUpload = std::min(numRequested, numFreeBrickSlots);
    std::map<uint64_t, std::pair<const VolumeOctreeNode*, size_t> > brickAddressToNodeIDMap;
    size_t numInserted = 0;
    for (size_t nodeID=0; nodeID<numNodes && numInserted<numBricksToUpload; nodeID++) {
        const uint8_t brickFlag = brickFlagBuffer_[nodeID];
        if (isBrickRequested(brickFlag)) {
            tgtAssert(nodeBufferToNodeMap_.find(nodeID) != nodeBufferToNodeMap_.end(), "missing nodeBufferToNodeMap entry");
            const VolumeOctreeNode* node = nodeBufferToNodeMap_[nodeID];
            if (node->hasBrick()) {
                brickAddressToNodeIDMap.insert(std::make_pair(node->getBrickAddress(), std::make_pair(node, nodeID)));
                leastRecentlyUsedBricks_.push_front(nodeID);
                numInserted++;
            }
            else {
                LWARNING("Non existing brick has been requested by GPU: nodeID=" << nodeID);
                updateNodeBrickPointer(nodeBuffer_[nodeID], false, 0);
            }
        }
    }
    tgtAssert(brickAddressToNodeIDMap.size() == numBricksToUpload, "brickAddressToNodeIDMap has invalid size");


    //
    // 5. Write requested bricks to an temporary update buffer
    //
    uint16_t* brickUpdateBuffer = new uint16_t[numBricksToUpload * numVoxelsPerBrick]; //< stores the bricks to be uploaded
    uint32_t* brickUpdateAddressBuffer = new uint32_t[numBricksToUpload]; //< stores the bricks' main buffer addresses

    // copy each requested brick to update buffer and assign next free brick slot as brick address
    size_t updateBufferPos = 0; //< current brick position in the temporary update buffer
    size_t nextFreeSlot = 0;    //< next free slot in the main brick buffer
    for (std::map<uint64_t, std::pair<const VolumeOctreeNode*, size_t> >::iterator it = brickAddressToNodeIDMap.begin();
         it != brickAddressToNodeIDMap.end(); ++it)
    {
        const uint64_t brickAddress = it->first;
        const VolumeOctreeNode* node = it->second.first;
        const size_t nodeID = it->second.second;
        tgtAssert(brickAddress < (size_t)-1, "invalid brick address");
        tgtAssert(node, "invalid node");
        tgtAssert(nodeID < numNodes, "invalid node id");

        // find next free slot in main buffer
        while (brickSlotUsed[nextFreeSlot])
            nextFreeSlot++;
        tgtAssert(updateBufferPos < numBricksToUpload, "invalid update buffer pos");
        tgtAssert(nextFreeSlot < numBrickBufferSlots_, "invalid next free slot");

        // retrieve brick
        const uint16_t* brick = 0;
        try {
            brick = octree->getNodeBrick(node);
        }
        catch (VoreenException& e) {
            LWARNING("Failed to retrieve node brick: " + std::string(e.what()));
            continue;
        }
        tgtAssert(brick, "no brick returned (exception expected)");

        // copy brick to update buffer and store correspond brick address
        memcpy(brickUpdateBuffer + updateBufferPos*numVoxelsPerBrick, brick, brickMemorySize);
        brickUpdateAddressBuffer[updateBufferPos] = static_cast<uint32_t>(nextFreeSlot);

        // update node brick pointer
        updateNodeBrickPointer(nodeBuffer_[nodeID], true, nextFreeSlot);
        tgtAssert(getNodeBrickPointer(nodeBuffer_[nodeID]) == nextFreeSlot, "node brick pointer not properly stored");

        updateBufferPos++;
        brickSlotUsed[nextFreeSlot] = true;
        numSlotsOccupied++;

        octree->releaseNodeBrick(node);
    }
    tgtAssert(numSlotsOccupied <= numBrickBufferSlots_, "more bricks occupied than slots available");


    //
    // 5. Use OpenCL kernel to copy bricks from update buffer to main brick buffer on the GPU
    //
    if (numBricksToUpload > 0) {
        cl::Context* context = OpenCLModule::getInstance()->getCLContext();
        cl::CommandQueue* commandQueue = OpenCLModule::getInstance()->getCLCommandQueue();
        tgtAssert(context, "No OpenCL context");
        tgtAssert(commandQueue, "No OpenCL command queue");
        cl::Kernel* updateBrickKernel = openclKernel_.getProgram()->getKernel("updateBrickBuffer");
        if (!updateBrickKernel) {
            LERROR("Found no kernel 'updateBrickBuffer' in CL program");
            delete[] brickUpdateBuffer;
            delete[] brickUpdateAddressBuffer;
            delete[] brickSlotUsed;
            return;
        }

        // create CL buffers for temporary update buffers (reducing their size to the actually used size!)
        cl::Buffer brickUpdateBufferCL(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            numBricksToUpload*numVoxelsPerBrick*2, brickUpdateBuffer);
        cl::Buffer brickUpdateAddressBufferCL(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            numBricksToUpload*4, brickUpdateAddressBuffer);

        updateBrickKernel->setArg(0, brickUpdateBufferCL);
        updateBrickKernel->setArg(1, brickUpdateAddressBufferCL);
        updateBrickKernel->setArg(2, static_cast<uint32_t>(numBricksToUpload));
        updateBrickKernel->setArg(3, brickBufferCL_);
        commandQueue->enqueue(updateBrickKernel, numBricksToUpload);
        commandQueue->finish();

        float occupiedMB = (float)numSlotsOccupied*numVoxelsPerBrick*2 / (1024.f*1024.f);
        float usedMB = (float)numUsedInLastPass*numVoxelsPerBrick*2 / (1024.f*1024.f);
        float uploadedMB = (float)numBricksToUpload*numVoxelsPerBrick*2 / (1024.f*1024.f);
        float removedMB = (float)numRemoved*numVoxelsPerBrick*2 / (1024.f*1024.f);
        #ifdef VRN_VOLUMEOCTREE_DEBUG
        LINFO("GPU bricks occupied/used/uploaded/removed: " << numSlotsOccupied << " (" << ftos(occupiedMB, 2) << " MB) / " <<
            numUsedInLastPass << " (" << ftos(usedMB, 2) << " MB) / " <<
            numBricksToUpload << " (" << ftos(uploadedMB, 2) << " MB) / " <<
            numRemoved << " (" << ftos(removedMB, 2) << " MB)");
        #endif

        // copy update buffer into main buffer (CPU version, for debugging only)
        /*for (size_t bufferPos=0; bufferPos<updateBufferPos; bufferPos++) {
            size_t destBufferOffset = brickUpdateAddressBuffer[bufferPos]*numVoxelsPerBrick;
            size_t srcBufferOffset = bufferPos*numVoxelsPerBrick;
            tgtAssert(destBufferOffset < brickBufferSize_, "invalid dest buffer size");
            tgtAssert(srcBufferOffset < numRequested*numVoxelsPerBrick, "invalid src buffer size");
            memcpy(brickBuffer_+destBufferOffset, brickUpdateBuffer+srcBufferOffset, brickMemorySize);
        } */
    }

    // free temporary CPU buffers
    delete[] brickUpdateBuffer;
    delete[] brickUpdateAddressBuffer;
    delete[] brickSlotUsed;

    #ifdef VRN_VOLUMEOCTREE_DEBUG
    LINFO("Update brick buffer time: " << watch.getRuntime() << " ms " << "(LRU update: " << lruWatch.getRuntime() << " ms)");
    #endif
}

void SingleOctreeRaycasterCL::analyzeBrickFlagBuffer(size_t& numTraversed, size_t& numUsed, size_t& numRequested) const {
    tgtAssert(volumeInport_.hasData() && volumeInport_.getData()->hasRepresentation<VolumeOctreeBase>(), "no input octree");
    tgtAssert(brickFlagBuffer_, "no flag buffer");

    const VolumeOctreeBase* octree = volumeInport_.getData()->getRepresentation<VolumeOctreeBase>();
    const size_t numNodes = octree->getNumNodes();

    numTraversed = 0;
    numUsed = 0;
    numRequested = 0;
    for (size_t nodeID = 0; nodeID < numNodes; nodeID++) {
        uint8_t nodeFlag = brickFlagBuffer_[nodeID];
        if (hasNodeBeenTraversed(nodeFlag))
            numTraversed++;
        if (hasBrickBeenUsed(nodeFlag))
            numUsed++;
        if (isBrickRequested(nodeFlag))
            numRequested++;
    }
}

void SingleOctreeRaycasterCL::compareNodeToBuffer(const VolumeOctreeNode* node, size_t nodeOffset) const
    throw (VoreenException)
{
    tgtAssert(volumeInport_.hasData() && volumeInport_.getData()->hasRepresentation<VolumeOctreeBase>(), "no input octree");
    tgtAssert(node, "null pointer passed");
    tgtAssert(nodeBuffer_, "no node buffer");

    const VolumeOctreeBase* octree = volumeInport_.getData()->getRepresentation<VolumeOctreeBase>();
    if (nodeOffset >= octree->getNumNodes())
        throw VoreenException("Invalid node offset: " + itos(nodeOffset));

    bool inPool, homogeneous;
    size_t childPointer, brickPointer;
    uint16_t avgValue;
    extractNodeValues(nodeBuffer_[nodeOffset], inPool, homogeneous, childPointer, avgValue, brickPointer);

    // compare node values
    if (inPool != 0)
        throw VoreenException("node at " + itos(nodeOffset) + ": mismatching inPool value (0 expected)");
    if (homogeneous == (!node->hasBrick()))
        throw VoreenException("node at " + itos(nodeOffset) + ": mismatching homogeneous value");
    uint16_t avgExpected = node->getAvgValue() >> (16-MASK_AVG_NUMBITS);
    if (avgValue != avgExpected)
        throw VoreenException("node at " + itos(nodeOffset) + ": mismatching avg value [" + itos(avgValue) + "!=" + itos(avgExpected) + "]");
    if (brickPointer != 0)
        throw VoreenException("node at " + itos(nodeOffset) + ": mismatching brick pointer value [" + itos(avgValue) + "!= 0]");

    // recursively compare children
    if (childPointer == 0 && node->children_[0] != 0)
        throw VoreenException("node at " + itos(nodeOffset) + ": missing children (child pointer == 0)");
    else if (childPointer != 0 && node->children_[0] == 0)
        throw VoreenException("node at " + itos(nodeOffset) + ": unexpected children (child pointer != 0)");
    else if (node->children_[0] != 0) {
        tgtAssert(childPointer > 0, "invalid child pointer");
        for (int childID = 0; childID<8; childID++) {
            const VolumeOctreeNode* childNode = node->children_[childID];
            tgtAssert(childNode, "child is 0");
            size_t childOffset = childPointer + childID;
            compareNodeToBuffer(childNode, childOffset);
        }
    }
}


void SingleOctreeRaycasterCL::renderVolume() {
    tgtAssert(volumeInport_.hasData() && volumeInport_.getData()->hasRepresentation<VolumeOctreeBase>(), "no input octree");
    tgtAssert(nodeBuffer_ && nodeBufferCL_, "no node buffer");
    tgtAssert(brickBuffer_ && brickBufferCL_, "no brick buffer");
    tgtAssert(brickFlagBuffer_ && brickFlagBufferCL_, "no brick flag buffer");

    glFinish();

    const VolumeBase* volume = volumeInport_.getData();
    tgtAssert(volume, "no input volume");
    const VolumeOctreeBase* octree = volume->getRepresentation<VolumeOctreeBase>();
    const RealWorldMapping rwm = volume->getRealWorldMapping();
    TransFunc1DKeys* transfunc = dynamic_cast<TransFunc1DKeys*>(transferFunc_.get());
    tgtAssert(transfunc, "no 1D TF");
    // use dimension with the highest resolution for calculating the sampling step size
    float samplingStepSize = 1.f / (tgt::max(octree->getDimensions()) * samplingRate_.get());
    const uint32_t coarsenessFactor = tgt::iround((float)renderOutport_.getSize().x / (float)internalRenderPort_.getSize().x);

    // compute desired node level of detail
    size_t nodeLevel = computeNodeLevel(tgt::vec3(0.5f), octree->getDimensions(),
        volumeInport_.getData()->getTextureToWorldMatrix(), octree->getNumLevels(),
        cameraProperty_.get().getPosition(), cameraProperty_.get().getProjectionMatrix(renderOutport_.getSize()), renderOutport_.getSize());
    if (interactionMode())
        nodeLevel -= std::min((size_t)interactionNodeLevelReduction_.get(), nodeLevel);

    // retrieve CL kernel
    if (!openclKernel_.getProgram()) {
        LERROR("No CL program");
        return;
    }
    cl::Kernel* renderKernel = openclKernel_.getProgram()->getKernel("render");
    if (!renderKernel) {
        LERROR("Found no kernel 'render' in CL program");
        return;
    }
    tgtAssert(renderKernel, "no render kernel");
    cl::Context* context = OpenCLModule::getInstance()->getCLContext();
    cl::CommandQueue* commandQueue = OpenCLModule::getInstance()->getCLCommandQueue();
    tgtAssert(context, "No OpenCL context");
    tgtAssert(commandQueue, "No OpenCL command queue");

    // update brick buffer on GPU (also updates node buffer on CPU!)
    if (useBricks_.get()) {
        if (uploadBricksDuringInteraction_.get() || !interactionMode()) {
            //if (updateBrickBufferIncrementally_.get())
                updateBrickBufferIncremental();
            /*else
                updateBrickBufferComplete(); */
        }

        // upload node and flag buffers
        commandQueue->enqueueWriteBuffer(nodeBufferCL_, nodeBuffer_, true);
        memset(brickFlagBuffer_, 0, octree->getNumNodes()); //< clear flag buffer for next pass
        commandQueue->enqueueWriteBuffer(brickFlagBufferCL_, brickFlagBuffer_, true);
    }

    // entry/exit points
    cl::SharedTexture entryTex(context, CL_MEM_READ_ONLY, entryPointsInport_.getColorTexture());
    cl::SharedTexture exitTex(context, CL_MEM_READ_ONLY, exitPointsInport_.getColorTexture());

    // transfunc
    tgt::Texture* tfTex = transferFunc_.get()->getTexture();
    cl::ImageFormat imgf(CL_RGBA, CL_UNORM_INT8);
    cl::ImageObject2D transFuncTex(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, imgf, tfTex->getWidth(), 1, tfTex->getWidth() * tfTex->getBpp(), transferFunc_.get()->getPixelData());

    // output texture
    cl::SharedTexture outTex(context, CL_MEM_WRITE_ONLY, internalRenderPort_.getColorTexture());
    cl::SharedTexture outTexDepth(context, CL_MEM_WRITE_ONLY, internalRenderPortDepth_.getColorTexture());

    // configure kernel
    cl_uint paramID = 0;
    renderKernel->setArg(paramID++, entryTex);
    renderKernel->setArg(paramID++, exitTex);
    renderKernel->setArg(paramID++, internalRenderPort_.getSize());
    renderKernel->setArg(paramID++, coarsenessFactor);

    renderKernel->setArg(paramID++, samplingStepSize);
    tgt::ivec4 volumeDim((int)volume->getDimensions().x, (int)volume->getDimensions().y, (int)volume->getDimensions().z, 1);
    renderKernel->setArg(paramID++, volumeDim);

    renderKernel->setArg(paramID++, nodeBufferCL_);
    renderKernel->setArg(paramID++, brickBufferCL_);
    renderKernel->setArg(paramID++, brickFlagBufferCL_);
    renderKernel->setArg(paramID++, static_cast<const uint32_t>(nodeLevel));

    renderKernel->setArgTemplate<RealWorldMappingCL>(paramID++, RealWorldMappingCL(rwm));
    renderKernel->setArg(paramID++, transFuncTex);
    renderKernel->setArg(paramID++, transferFunc_.get()->getDomain());

    //renderKernel->setArg(12, cameraProperty_.get().getPosition());
    //renderKernel->setArg(13, cameraProperty_.get().getProjectionMatrix(renderOutport_.getSize()));

    renderKernel->setArg(paramID++, outTex);
    renderKernel->setArg(paramID++, outTexDepth);

    /*tgt::vec4 matrixOutBuffer(-1.f);
    cl::Buffer matrixOutBufferCL(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, 4*sizeof(float), matrixOutBuffer.elem);
    renderKernel->setArg(14, matrixOutBufferCL); */

    tgt::svec2 globalWorkSize(tgt::nextLargerPowerOfTwo(internalRenderPort_.getSize().x), tgt::nextLargerPowerOfTwo(internalRenderPort_.getSize().y));
    tgt::svec2 localWorkSize(16);

    // execute kernel
    commandQueue->enqueueAcquireGLObject(&entryTex);
    commandQueue->enqueueAcquireGLObject(&exitTex);
    commandQueue->enqueueAcquireGLObject(&outTex);
    commandQueue->enqueueAcquireGLObject(&outTexDepth);
    commandQueue->enqueue(renderKernel, globalWorkSize, localWorkSize);
    commandQueue->enqueueReleaseGLObject(&entryTex);
    commandQueue->enqueueReleaseGLObject(&exitTex);
    commandQueue->enqueueReleaseGLObject(&outTex);
    commandQueue->enqueueReleaseGLObject(&outTexDepth);

    if (useBricks_.get()) {
        commandQueue->enqueueReadBuffer(brickFlagBufferCL_, brickFlagBuffer_, true);
    }

    commandQueue->finish();


    // compare matrix mult result
    /*tgt::vec4 camPosProjectedCPU = octree->getTextureToWorldMatrix() * tgt::vec4(cameraProperty_.get().getPosition(), 1.f);
    camPosProjectedCPU /= camPosProjectedCPU.w;
    commandQueue->enqueueReadBuffer(&matrixOutBufferCL, matrixOutBuffer.elem, true);
    LINFO("CPU matrix mult result:" << camPosProjectedCPU);
    LINFO("GPU matrix mult result:" << matrixOutBuffer); */

    renderOutport_.validateResult();
}

void SingleOctreeRaycasterCL::rebuildProgram() {
    tgtAssert(volumeInport_.hasData() && volumeInport_.getData()->hasRepresentation<VolumeOctreeBase>(), "no input octree");
    const VolumeOctreeBase* octree = volumeInport_.getData()->getRepresentation<VolumeOctreeBase>();

    std::ostringstream clDefines;
    clDefines << " -cl-fast-relaxed-math -cl-mad-enable";
    // set include path for modules
    clDefines << " -I" << tgt::FileSystem::cleanupPath(VoreenApplication::app()->getBasePath() + "/modules/opencl/cl/");
    clDefines << " -DOCTREE_DIMENSIONS_DEF=" << (octree ? octree->getOctreeDim().x : 1);
    clDefines << " -DOCTREE_BRICKDIM_DEF="   << (octree ? octree->getBrickDim().x  : 1);
    clDefines << " -DOCTREE_DEPTH_DEF="      << (octree ? octree->getNumLevels()   : 1);

    /*clDefines << " -DVOLUME_DIMENSIONS_DEF=" << toOpenCLLiteral(octree ? octree->getDimensions() : svec3(0,0,0)) << "";
    clDefines << " -DVOLUME_TEXTURE_TO_WORLD_MATRIX_DEF=" <<
                    toOpenCLLiteral(octree ? volumeInport_.getData()->getTextureToWorldMatrix() : tgt::mat4(1.f)) << ""; */

    if (adaptiveSampling_.get())
        clDefines << " -DADAPTIVE_SAMPLING";

    if (textureFilterMode_.isSelected("linear"))
        clDefines << " -DTEXTURE_FILTER_LINEAR";

    if (useBricks_.get())
        clDefines << " -DUSE_BRICKS";
    if (useAncestorNodes_.get())
        clDefines << " -DUSE_ANCESTOR_NODES";

    openclKernel_.setDefines(clDefines.str());

    try {
        openclKernel_.rebuild();
    }
    catch (VoreenException& e) {
        LERROR(e.what());
        openclKernel_.clearProgram();
    }

    LGL_ERROR;
}

void SingleOctreeRaycasterCL::rescaleRendering(RenderPort& srcPort, RenderPort& srcPortDepth,
    RenderPort& entryPoints, RenderPort& exitPoints, RenderPort& destPort)
{
    // activate and clear output render target
    destPort.activateTarget();
    destPort.clearTarget();

    // activate shader and set uniforms
    tgtAssert(rescaleShader_, "bypass shader not loaded");
    rescaleShader_->activate();
    setGlobalShaderParameters(rescaleShader_, 0, destPort.getSize());

    // bind input rendering to texture units
    tgt::TextureUnit colorUnit, depthUnit;
    srcPort.bindColorTexture(colorUnit.getEnum(), GL_LINEAR);
    srcPortDepth.bindColorTexture(depthUnit.getEnum(), GL_LINEAR);
    srcPort.setTextureParameters(rescaleShader_, "texParams_");
    rescaleShader_->setUniform("colorTex_", colorUnit.getUnitNumber());
    rescaleShader_->setUniform("depthTex_", depthUnit.getUnitNumber());

    tgt::TextureUnit depthUnitEntry, depthUnitExit;
    entryPoints.bindDepthTexture(depthUnitEntry, GL_LINEAR);
    exitPoints.bindDepthTexture(depthUnitExit, GL_LINEAR);
    entryPoints.setTextureParameters(rescaleShader_, "texParamsEEP_");
    rescaleShader_->setUniform("depthTexEntry_", depthUnitEntry.getUnitNumber());
    rescaleShader_->setUniform("depthTexExit_", depthUnitExit.getUnitNumber());

    // render screen aligned quad
    renderQuad();

    // cleanup
    rescaleShader_->deactivate();
    destPort.deactivateTarget();
    tgt::TextureUnit::setZeroUnit();

    // check for OpenGL errors
    LGL_ERROR;
}

void SingleOctreeRaycasterCL::regenerateBuffers() {
    regenerateBuffers_ = true;
}

}   // namespace
