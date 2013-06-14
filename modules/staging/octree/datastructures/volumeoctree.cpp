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

#include "volumeoctree.h"

#include "octreeutils.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/utils/stringutils.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/tgt_math.h"
#include "tgt/stopwatch.h"
#include "tgt/filesystem.h"

#include <sstream>

using tgt::svec3;
using tgt::vec3;

namespace {
    const size_t MAX_CHANNELS = 4; //< maximum number of channels that are supported
}

namespace voreen {

const std::string VolumeOctree::loggerCat_("voreen.VolumeOctree");

VolumeOctree::VolumeOctree(const std::vector<const VolumeBase*>& channelVolumes, size_t brickDim, float homogeneityThreshold /*= 0.001f*/,
                           OctreeBrickPoolManagerBase* brickPoolManager, ProgressBar* progessBar) throw (std::exception)
    : VolumeOctreeBase(tgt::svec3(brickDim), !channelVolumes.empty() ? channelVolumes.front()->getDimensions() : svec3(brickDim), channelVolumes.size())
    , rootNode_(0)
{
    if (channelVolumes.empty())
        throw std::invalid_argument("No input volumes passed");
    if (channelVolumes.size() > MAX_CHANNELS)
        throw std::invalid_argument("More than " + itos(MAX_CHANNELS) + " not supported");

    if (!brickPoolManager)
        throw std::invalid_argument("No brick pool manager passed");
    brickPoolManager_ = brickPoolManager;

    // check that input channel volumes have equal properties
    svec3 volumeDimensions = channelVolumes.front()->getDimensions();
    std::string format = channelVolumes.front()->getFormat();
    for (size_t i=1; i<channelVolumes.size(); i++) {
        if (channelVolumes.at(i)->getDimensions() != volumeDimensions)
            throw std::invalid_argument("Dimensions of input volumes do not match [" +
                genericToString(channelVolumes.at(i)->getDimensions()) + "!=" + genericToString(volumeDimensions) + "]");
        if (channelVolumes.at(i)->getFormat() != format)
            throw std::invalid_argument("Formats of input volumes do not match [" + channelVolumes.at(i)->getFormat() + "!=" + format + "]");
    }

    tgtAssert(getNumLevels() > 0 && getNumLevels() < tgt::max(channelVolumes.front()->getDimensions()), "invalid level count");

    // convert normalized homogeneity threshold to uint16_t
    bool octreeOptimization = (homogeneityThreshold >= 0.f);
    uint16_t homogeneityThresholdUInt16 = tgt::clamp(tgt::iround(homogeneityThreshold * 65535), 0, 65535);

#ifdef VRN_VOLUMEOCTREE_DEBUG
    tgt::Stopwatch watch;
    watch.start();
    buildOctree(channelVolumes, octreeOptimization, homogeneityThresholdUInt16, progessBar);
    watch.stop();
    logDescription();
    LINFO("Construction time: \t" << watch.getRuntime() << " ms\n");
#else
    buildOctree(channelVolumes, octreeOptimization, homogeneityThresholdUInt16, progessBar);
#endif
    tgtAssert(rootNode_, "no root node after octree construction");
}

VolumeOctree::VolumeOctree(const VolumeBase* volume, size_t brickDim, float homogeneityThreshold /*= 0.001f*/,
    OctreeBrickPoolManagerBase* brickPoolManager, ProgressBar* progessBar) throw (std::exception)
    : VolumeOctreeBase(tgt::svec3(brickDim), volume->getDimensions(), 1)
    , rootNode_(0)
{
    tgtAssert(getNumLevels() > 0 && getNumLevels() < tgt::max(volume->getDimensions()), "invalid level count");
    tgtAssert(volume, "no volume passed");
    tgtAssert(brickPoolManager, "no brick pool manager passed");
    brickPoolManager_ = brickPoolManager;

    // convert normalized homogeneity threshold to uint16_t
    bool octreeOptimization = (homogeneityThreshold >= 0.f);
    uint16_t homogeneityThresholdUInt16 = tgt::clamp(tgt::iround(homogeneityThreshold * 65535), 0, 65535);

    std::vector<const VolumeBase*> channelVolumes;
    channelVolumes.push_back(volume);
#ifdef VRN_VOLUMEOCTREE_DEBUG
    tgt::Stopwatch watch;
    watch.start();
    buildOctree(channelVolumes, octreeOptimization, homogeneityThresholdUInt16, progessBar);
    watch.stop();
    logDescription();
    LINFO("Construction time: \t" << watch.getRuntime() << " ms\n");
#else
    buildOctree(channelVolumes, octreeOptimization, homogeneityThresholdUInt16, progessBar);
#endif
    tgtAssert(rootNode_, "no root node after octree construction");
}

// default constructor for serialization (private)
VolumeOctree::VolumeOctree()
    : rootNode_(0)
    , brickPoolManager_(0)
{}

VolumeOctree::~VolumeOctree() {
    if (rootNode_)
        deleteSubTree(rootNode_);
    rootNode_ = 0;

    // deinitialize brick pool manager
    //tgtAssert(brickPoolManager_, "no brick pool manager");
    if (brickPoolManager_)
        brickPoolManager_->deinitialize();
    delete brickPoolManager_;
    brickPoolManager_ = 0;
}

VolumeOctree* VolumeOctree::create() const {
    return new VolumeOctree();
}

// TODO: cache node count
size_t VolumeOctree::getNumNodes() const {
    tgtAssert(rootNode_, "no root node");
    return rootNode_->getNodeCount();
}

size_t VolumeOctree::getNumBricks() const {
    tgtAssert(rootNode_, "no root node");
    return rootNode_->getNumBricks();
}

size_t VolumeOctree::getActualTreeDepth() const {
    tgtAssert(rootNode_, "no root node");
    return rootNode_->getDepth();
}

uint64_t VolumeOctree::getBrickPoolMemoryAllocated() const {
    return brickPoolManager_->getBrickPoolMemoryAllocated();
}

uint64_t VolumeOctree::getBrickPoolMemoryUsed() const {
    tgtAssert(rootNode_, "no root node");
    return rootNode_->getNumBricks()*getBrickMemorySize();
}

std::string VolumeOctree::getDescription() const {
    std::ostringstream desc;

    desc << "Volume dim: " << getVolumeDim() << std::endl;
    desc << "Octree dim: " << getOctreeDim() << std::endl;
    desc << "Brick size: " << getBrickDim() << std::endl;
    desc << "Num levels: " << getNumLevels() << std::endl;
    desc << "Tree depth: " << getActualTreeDepth() << std::endl;
    desc << "Num nodes/bricks: \t" << getNumNodes() << "/" << rootNode_->getNumBricks() << std::endl;

    size_t brickMemSize = getBrickMemorySize();
    float brickMemSizeKB = getBrickMemorySize() / 1024.f;
    desc << "Brick memory size: \t" << ftos(brickMemSizeKB, 0) << " KB    \t(" << brickMemSize << " Bytes)" << std::endl;

    uint64_t mimMapMemUsed = getBrickPoolMemoryUsed();
    float mipMapMemUsedMB = getBrickPoolMemoryUsed() / (1024.f*1024.f);
    uint64_t mimMapMemAllocated = getBrickPoolMemoryAllocated();
    float mipMapMemAllocatedMB = getBrickPoolMemoryAllocated() / (1024.f*1024.f);
    desc << "Brick pool mem used:\t" << ftos(mipMapMemUsedMB, 2) << " MB \t(" << mimMapMemUsed << " Bytes)" << std::endl;
    desc << "Brick pool mem alloc:\t" << ftos(mipMapMemAllocatedMB, 2) << " MB \t(" << mimMapMemAllocated << " Bytes)" << std::endl;

    size_t volumeMemSize = tgt::hmul(getVolumeDim())*getBytesPerVoxel();
    float volumeMemSizeMB = volumeMemSize / (1024.f*1024.f);
    desc << "Volume memory size: \t" << ftos(volumeMemSizeMB, 2) << " MB \t(" << volumeMemSize << " Bytes)" /*<< std::endl()*/;

    return desc.str();
}

const VolumeOctreeNode* VolumeOctree::getRootNode() const {
    return rootNode_;
}

const VolumeOctreeNode* VolumeOctree::getNode(const tgt::vec3& point, size_t& level,
    tgt::svec3& voxelLLF, tgt::svec3& voxelURB, tgt::vec3& normLLF, tgt::vec3& normURB) const
{
    tgtAssert(rootNode_, "no root node");
    if (!inRange(point, vec3(0.f), vec3(1.f)))
        throw std::invalid_argument("Point coordinates outside unit cube: " + genericToString(point));
    if (level >= getNumLevels())
        throw std::invalid_argument("Passed level larger than octree depth: " + itos(level) + " (octree depth: " + itos(getNumLevels()) + ")");

    svec3 voxel = samplePosToVoxel(point, getVolumeDim());
    tgtAssert(tgt::hand(tgt::lessThan(voxel, getVolumeDim())), "invalid voxel");

    size_t resultLevel;
    const VolumeOctreeNode* node = getNodeAtVoxel(voxel, getNumLevels()-1, level, rootNode_, svec3::zero, getOctreeDim(), resultLevel, voxelLLF, voxelURB);
    tgtAssert(resultLevel >= level, "invalid return level");
    tgtAssert(tgt::hand(tgt::lessThan(voxelLLF, voxelURB)) && tgt::hand(tgt::lessThanEqual(voxelURB, getOctreeDim())),
        "invalid llf/urb");

    level = resultLevel;
    normLLF = vec3(voxelLLF) / vec3(getVolumeDim());
    normURB = vec3(voxelURB) / vec3(getVolumeDim());
    //voxelURB -= svec3(1); //< urb is the next voxel outside the actual node => subtract 1

    return node;
}

const uint16_t* VolumeOctree::getNodeBrick(const VolumeOctreeNode* node) const
    throw (VoreenException)
{
    tgtAssert(brickPoolManager_, "no brick pool manager");
    tgtAssert(node, "null pointer");
    tgtAssert((node->getBrickAddress() == 0) || (node->getBrickAddress() == std::numeric_limits<uint64_t>::max()) || isMultipleOf(node->getBrickAddress(), (uint64_t)getBrickMemorySize()),
        "node brick offset is not a multiple of brick memory size");

    return brickPoolManager_->getBrick(node->getBrickAddress());
}

void VolumeOctree::releaseNodeBrick(const VolumeOctreeNode* node) const {
    tgtAssert(brickPoolManager_, "no brick pool manager");
    tgtAssert(node, "null pointer");
    tgtAssert((node->getBrickAddress() == 0) || (node->getBrickAddress() == std::numeric_limits<uint64_t>::max()) || isMultipleOf(node->getBrickAddress(), (uint64_t)getBrickMemorySize()),
        "node brick offset is not a multiple of brick memory size");

    return brickPoolManager_->setBrickUnused(node->getBrickAddress());

}

VolumeRAM* VolumeOctree::createVolume(size_t level /*= 0*/, size_t channel /*= 0*/) const
    throw (VoreenException)
{
    if (level >= getNumLevels())
        throw std::invalid_argument("Passed level larger than octree depth: " + itos(level) + " (octree depth: " + itos(getNumLevels()) + ")");
    if (channel >= getNumChannels())
        throw std::invalid_argument("Selected channel larger than number of channels (" + itos(getNumChannels()) + "):" + itos(channel));

    svec3 levelVolumeDim = getVolumeDim() / svec3(1 << (level));
    VolumeRAM_UInt16* levelVolumeRam = 0;
    try {
        levelVolumeRam = new VolumeRAM_UInt16(levelVolumeDim, true);
    }
    catch (std::bad_alloc&) {
        LERROR("Failed to allocate texture buffer");
        return 0;
    }
    tgtAssert(levelVolumeRam, "output volume not created");
    uint16_t* levelVolumeBuffer = reinterpret_cast<uint16_t*>(levelVolumeRam->getData());

    composeNodeTexture(rootNode_, svec3(0,0,0), getNumLevels()-1, level, channel, levelVolumeBuffer, levelVolumeDim);

    return levelVolumeRam;
}

VolumeRAM* VolumeOctree::createSlice(SliceAlignment sliceAlignment, size_t sliceIndex, size_t level /*= 0*/, size_t channel /*= 0*/) const
    throw (VoreenException)
{
    if (level >= getNumLevels())
        throw std::invalid_argument("Passed level larger than octree depth: " + itos(level) + " (octree depth: " + itos(getNumLevels()) + ")");
    if (channel >= getNumChannels())
        throw std::invalid_argument("Selected channel larger than number of channels (" + itos(getNumChannels()) + "):" + itos(channel));
    if (sliceIndex >= getVolumeDim()[sliceAlignment])
        throw std::invalid_argument("Invalid slice index:" + itos(sliceIndex));

    // determine dimension of output slice with regard to selected level
    const svec3 levelVolumeDim = getVolumeDim() / svec3(1 << level);
    svec3 sliceVolumeDim = levelVolumeDim;
    sliceVolumeDim[sliceAlignment] = 1;

    // transform requested slice index to selected level
    const size_t sliceIndexAtLevel = std::min(sliceIndex / static_cast<size_t>(1 << level), levelVolumeDim[sliceAlignment]-1);

    // allocate output slice volume
    VolumeRAM_UInt16* sliceVolumeRam = 0;
    try {
        sliceVolumeRam = new VolumeRAM_UInt16(sliceVolumeDim, true);
    }
    catch (std::bad_alloc&) {
        LERROR("Failed to allocate texture buffer");
        return 0;
    }
    tgtAssert(sliceVolumeRam, "output volume not created");

    // recursively copy voxel data from bricks to output slice
    uint16_t* sliceVolumeBuffer = reinterpret_cast<uint16_t*>(sliceVolumeRam->getData());
    composeNodeSliceTexture(sliceAlignment, rootNode_, svec3(0,0,0), sliceIndexAtLevel, getNumLevels()-1, level,
        channel, sliceVolumeBuffer, sliceVolumeDim);

    return sliceVolumeRam;
}

void VolumeOctree::deleteSubTree(VolumeOctreeNode* root) const {
    if (!root)
        return;

    deleteSubTree(root->children_[0]);
    root->children_[0] = 0;

    deleteSubTree(root->children_[1]);
    root->children_[1] = 0;

    deleteSubTree(root->children_[2]);
    root->children_[2] = 0;

    deleteSubTree(root->children_[3]);
    root->children_[3] = 0;

    deleteSubTree(root->children_[4]);
    root->children_[4] = 0;

    deleteSubTree(root->children_[5]);
    root->children_[5] = 0;

    deleteSubTree(root->children_[6]);
    root->children_[6] = 0;

    deleteSubTree(root->children_[7]);
    root->children_[7] = 0;

    delete root;
}

void VolumeOctree::buildOctree(const std::vector<const VolumeBase*>& volumes, bool octreeOptimization, uint16_t homogeneityThreshold,
    ProgressBar* progessBar /*= 0*/) throw (VoreenException)
{
    tgtAssert(volumes.size() == getNumChannels(), "number of channel volumes does not match channel count");
    tgtAssert(volumes.front(), "no volume");

    // brick count from tree depth
    size_t numBricks = 0;
    for (size_t l=0; l<getNumLevels(); l++)
        numBricks += tgt::iround(pow(8.0, (double)l));

    // initialize brick pool manager
    tgtAssert(brickPoolManager_, "no brick pool manager");
    brickPoolManager_->initialize(getBrickMemorySize());

    // retrieve data buffers from input channel volumes
    std::vector<const uint16_t*> volumeBuffers;
    for (size_t i=0; i<getNumChannels(); i++) {
        tgtAssert(volumes.at(i), "null pointer passed as volume");
        const VolumeRAM_UInt16* volumeUInt16 = dynamic_cast<const VolumeRAM_UInt16*>(volumes.at(i)->getRepresentation<VolumeRAM>());
        if (!volumeUInt16)
            throw VoreenException("Failed to retrieve VolumeRAM_UInt16 from volume: " + itos(i));
        const uint16_t* volumeBuffer = reinterpret_cast<const uint16_t*>(volumeUInt16->getData());
        volumeBuffers.push_back(volumeBuffer);
    }
    tgtAssert(volumeBuffers.size() == getNumChannels(), "invalid number of volume buffers");

    if (progessBar)
        progessBar->setProgress(0.f);

    // create octree
    tgtAssert(getNumChannels() <= MAX_CHANNELS, "more than max channels");
    uint16_t avgValues[MAX_CHANNELS], minValues[MAX_CHANNELS], maxValues[MAX_CHANNELS];
    rootNode_ = createTreeNode(svec3::zero, getOctreeDim(), volumeBuffers, getVolumeDim(),
        octreeOptimization, homogeneityThreshold, avgValues, minValues, maxValues, progessBar);
    tgtAssert(minValues[0] <= avgValues[0] && avgValues[0] <= maxValues[0], "invalid avg/min/max values");
    tgtAssert(rootNode_->getAvgValue() == avgValues[0], "avg value of root node differs from returned avg value");
    tgtAssert(rootNode_->getNumBricks() <= rootNode_->getNodeCount(), "number of bricks larger than number of nodes");

    if (progessBar)
        progessBar->setProgress(1.f);
}

VolumeOctreeNode* VolumeOctree::createTreeNode(const svec3& llf, const svec3& urb,
    const std::vector<const uint16_t*>& textureBuffers, const tgt::svec3& textureDim,
    bool octreeOptimization, uint16_t homogeneityThreshold,
    uint16_t* avgValues, uint16_t* minValues, uint16_t* maxValues,
    ProgressBar* progessBar /*= 0*/)
{
    tgtAssert(textureBuffers.size() == getNumChannels(), "number of texture buffers does not match channel count");
    tgtAssert(getNumChannels() <= MAX_CHANNELS, "more than max channels");
    tgtAssert(textureBuffers.front(), "null pointer passed");
    tgtAssert(brickPoolManager_, "brick pool manager");
    tgtAssert(avgValues && minValues && maxValues, "null pointer passed as avg/min/max value array");

    tgtAssert(tgt::hand(tgt::lessThan(llf, getOctreeDim())), "invalid llf");
    tgtAssert(tgt::hand(tgt::lessThanEqual(urb, getOctreeDim())), "invalid urb");
    tgtAssert(tgt::hand(tgt::lessThan(llf, urb)), "llf larger than or equal urb");

    // determine dimensions of node region
    const svec3 nodeDim = urb-llf;
    tgtAssert(isMultipleOf(nodeDim, getBrickDim()), "node dim is not a multiple of brick dimensions");

    // create node
    VolumeOctreeNode* node = 0;
    if (octreeOptimization && tgt::hor(tgt::greaterThanEqual(llf, textureDim))) {
        // brick completely outside volume texture => return homogeneous node, if tree optimization is enabled
        node = VolumeOctreeBase::createNode(getNumChannels());
        for (size_t i=0; i<getNumChannels(); i++) {
            minValues[i] = 0;
            maxValues[i] = 0;
            avgValues[i] = 0;
        }
    }
    else if (nodeDim == getBrickDim()) {
        // highest level (full resolution) has been reached => create brick from volume texture and terminate recursion
        uint16_t* tempBrickBuffer = new uint16_t[getNumVoxelsPerBrick()*getNumChannels()];
        extractBrickFromTexture(textureBuffers, textureDim, tempBrickBuffer, getBrickDim(), llf, avgValues, minValues, maxValues);

        // determine whether node is homogeneous (in all channels)
        bool homogeneous = true;
        for (size_t i=0; i<getNumChannels(); i++) {
            tgtAssert(minValues[i] <= avgValues[i] && avgValues[i] <= maxValues[i], "invalid avg/min/max values");
            homogeneous &= (maxValues[i] - minValues[i] <= homogeneityThreshold);
        }

        // node not homogeneous => create leaf node with brick and shift virtual memory offset
        if (!homogeneous || !octreeOptimization) {
            const uint64_t virtualBrickAddress = brickPoolManager_->allocateBrick();
            uint16_t* brickBuffer = brickPoolManager_->getWritableBrick(virtualBrickAddress);
            memcpy(brickBuffer, tempBrickBuffer, getBrickMemorySize());
            brickPoolManager_->setBrickUnused(virtualBrickAddress);

            node = VolumeOctreeBase::createNode(getNumChannels(), avgValues, minValues, maxValues, virtualBrickAddress);
        }
        else { // node homogeneous => store only avg value (without brick)
            node = VolumeOctreeBase::createNode(getNumChannels(), avgValues, minValues, maxValues);
        }
        delete[] tempBrickBuffer;
    }
    else { // recursively create child nodes
        VolumeOctreeNode* children[8];
        const svec3 childNodeDim = nodeDim / svec3(2);

        uint32_t avgValuesUInt32[MAX_CHANNELS];
        for (size_t i=0; i<getNumChannels(); i++) {
            minValues[i] = 65535;
            maxValues[i] = 0;
            avgValuesUInt32[i] = 0;
        }
        VRN_FOR_EACH_VOXEL(child, svec3::zero, svec3::two) {
            size_t childIndex = cubicCoordToLinear(child, svec3(2));
            svec3 childLlf = llf + child*childNodeDim;
            svec3 childUrb = childLlf + childNodeDim;
            uint16_t childAvg[MAX_CHANNELS], childMin[MAX_CHANNELS], childMax[MAX_CHANNELS];
            children[childIndex] = createTreeNode(childLlf, childUrb, textureBuffers, textureDim,
                octreeOptimization, homogeneityThreshold,
                childAvg, childMin, childMax, progessBar);

            for (size_t i=0; i<getNumChannels(); i++) {
                tgtAssert(childMin[i] <= childAvg[i] && childAvg[i] <= childMax[i], "invalid child avg/min/max values");
                avgValuesUInt32[i] += childAvg[i];
                minValues[i] = std::min(minValues[i], childMin[i]);
                maxValues[i] = std::max(maxValues[i], childMax[i]);
            }
        }

        // determine whether node is homogeneous (in all channels)
        bool homogeneous = true;
        for (size_t i=0; i<getNumChannels(); i++) {
            avgValues[i] = avgValuesUInt32[i] / 8;
            tgtAssert(minValues[i] <= avgValues[i] && avgValues[i] <= maxValues[i], "invalid avg/min/max values");
            homogeneous &= (maxValues[i] - minValues[i] <= homogeneityThreshold);
        }

        // node is not homogeneous => create inner node with brick by merging child nodes
        if (!homogeneous || !octreeOptimization) {
            node = createParentNode(children, brickPoolManager_->allocateBrick());
        }
        else { // node is homogeneous => create leaf node without brick
            node = VolumeOctreeBase::createNode(getNumChannels(), avgValues, minValues, maxValues);

            // delete child nodes
            for (size_t i=0; i<8; i++)
                deleteSubTree(children[i]);
        }
    }
    tgtAssert(node, "no node created");
    tgtAssert(minValues[0] <= avgValues[0] && avgValues[0] <= maxValues[0], "invalid avg/min/max values");
    tgtAssert(node->getAvgValue() == avgValues[0], "avg value mis-match");

    // update progress bar (only on second level)
    if (progessBar && nodeDim.x == (getOctreeDim().x / 4)) {
        size_t curLevel = tgt::iround(logf((float)getOctreeDim().x / (float)(nodeDim.x)) / logf(2.f));
        size_t treeDepth = getNumLevels();
        float relativeSubtreeSize = (float)getCompleteTreeNodeCount(treeDepth - curLevel) / (float)getCompleteTreeNodeCount(treeDepth);
        //float npotCorrection = (float)tgt::hmul(getOctreeDim()) / (float)tgt::hmul(getVolumeDim());
        progessBar->setProgress(progessBar->getProgress() + relativeSubtreeSize);
    }

    return node;
}

//------------------
// private functions

void VolumeOctree::copyBrickToTexture(const uint16_t* brick, const svec3& brickDim, size_t channel,
    uint16_t* texture, const svec3& textureDim,
    const svec3& brickOffsetInTexture) const
{
    tgtAssert(brick, "null pointer passed");
    tgtAssert(channel < getNumChannels(), "invalid channel");
    tgtAssert(texture, "null pointer passed");
    tgtAssert(tgt::hand(tgt::lessThanEqual(brickDim, getOctreeDim())), "brick dimensions greater than texture dimensions");
    tgtAssert(tgt::hand(tgt::lessThanEqual(brickOffsetInTexture+brickDim, getOctreeDim())), "brick (partially) outside texture");

    const size_t brickChannelSize = tgt::hmul(brickDim);
    const size_t brickBufferSize = brickChannelSize*getNumChannels();
    const size_t textureBufferSize = tgt::hmul(textureDim);

    const size_t channelOffset = channel*brickChannelSize;
    VRN_FOR_EACH_VOXEL(brickVoxel, svec3(0,0,0), brickDim) {
        size_t brickLinearCoord = channelOffset + cubicCoordToLinear(brickVoxel, brickDim);
        tgtAssert(brickLinearCoord < brickBufferSize, "invalid brick linear coord");
        const tgt::svec3 textureCoord = brickVoxel+brickOffsetInTexture;
        if (tgt::hand(tgt::lessThan(textureCoord, textureDim))) {
            size_t textureLinearCoord = cubicCoordToLinear(textureCoord, textureDim);
            tgtAssert(textureLinearCoord < textureBufferSize, "invalid texture linear coord");
            texture[textureLinearCoord] = brick[brickLinearCoord];
        }
    }
}

void VolumeOctree::extractBrickFromTexture(const std::vector<const uint16_t*>& textures, const svec3& textureDim,
    uint16_t* brickBuffer, const svec3& brickDim, const svec3& brickOffsetInTexture,
    uint16_t* avgValues, uint16_t* minValues, uint16_t* maxValues) const
{
    tgtAssert(textures.size() == getNumChannels(), "number of channel textures does not match channel count");
    tgtAssert(textures.front(), "null pointer passed");
    tgtAssert(brickBuffer, "null pointer passed");
    tgtAssert(avgValues && minValues && maxValues, "null pointer passed as avg/min/max value array");

    tgtAssert(tgt::hand(tgt::lessThanEqual(brickDim, getOctreeDim())), "brick dimensions greater than octree dimensions");
    tgtAssert(tgt::hand(tgt::lessThanEqual(brickOffsetInTexture+brickDim, getOctreeDim())), "brick (partially) outside octree dimensions");

    tgtAssert(getNumChannels() <= MAX_CHANNELS, "more than max channels");
    uint64_t avgValues64[MAX_CHANNELS];
    for (size_t channel=0; channel<getNumChannels(); channel++) {
        avgValues64[channel] = 0;
        minValues[channel] = 65535;
        maxValues[channel] = 0;
    }

    size_t brickChannelSize = tgt::hmul(brickDim);
    size_t brickBufferSize = brickChannelSize*getNumChannels();
    size_t textureBufferSize = tgt::hmul(textureDim);

    // copy voxels from channel textures to brick
    for (size_t channel=0; channel<getNumChannels(); channel++) {
        size_t channelOffset = channel*brickChannelSize;
        const uint16_t* channelTexture = textures.at(channel);

        uint64_t numSignificantBrickVoxels = 0; //< number of brick voxels lying inside texture (NPOT)
        VRN_FOR_EACH_VOXEL(brickVoxel, svec3(0,0,0), brickDim) {
            size_t brickLinearCoord = channelOffset + cubicCoordToLinear(brickVoxel, brickDim);
            tgtAssert(brickLinearCoord < brickBufferSize, "invalid brick linear coord");
            const tgt::svec3 textureCoord = brickVoxel+brickOffsetInTexture;
            if (tgt::hand(tgt::lessThan(textureCoord, textureDim))) { //< octree voxel inside volume
                size_t textureLinearCoord = cubicCoordToLinear(brickVoxel+brickOffsetInTexture, textureDim);
                tgtAssert(textureLinearCoord < textureBufferSize, "invalid texture linear coord");
                uint16_t value = channelTexture[textureLinearCoord];

                brickBuffer[brickLinearCoord] = value;
                avgValues64[channel] += value;
                minValues[channel] = std::min(minValues[channel], value);
                maxValues[channel] = std::max(maxValues[channel], value);

                numSignificantBrickVoxels++;
            }
            else {
                brickBuffer[brickLinearCoord] = 0;
            }
        }

        if (numSignificantBrickVoxels > 0) {
            avgValues[channel] = static_cast<uint16_t>(avgValues64[channel] / numSignificantBrickVoxels);
            tgtAssert(minValues[channel] <= maxValues[channel], "min value is larger than max value");
            tgtAssert(minValues[channel] <= avgValues[channel] && avgValues[channel] <= maxValues[channel], "avg value outside min-max value range");
        }
        else {
            avgValues[channel] = 0;
            minValues[channel] = 0;
            maxValues[channel] = 0;
        }
    }
}

void VolumeOctree::halfSampleBrick(const uint16_t* brick, const svec3& brickDim, uint16_t* halfSampledBrick) const {
    tgtAssert(brick && halfSampledBrick, "null pointer passed");
    tgtAssert(isCubicAndPot(brickDim), "invalid brick dimensions");

    const svec3 halfDim = brickDim / svec3(2);

    const size_t brickChannelSize = tgt::hmul(brickDim);
    const size_t halfBrickChannelSize = tgt::hmul(halfDim);

    for (size_t ch=0; ch < getNumChannels(); ch++) {
        const size_t channelOffset =     ch*brickChannelSize;
        const size_t halfChannelOffset = ch*halfBrickChannelSize;
        VRN_FOR_EACH_VOXEL(halfPos, svec3::zero, halfDim) {
            svec3 pos = halfPos*svec3(2);
            uint64_t halfValue =
                (uint64_t)brick[channelOffset + cubicCoordToLinear(pos, brickDim)]                              +  // LLF
                (uint64_t)brick[channelOffset + cubicCoordToLinear(svec3(pos.x,   pos.y,   pos.z+1), brickDim)] +  // LLB
                (uint64_t)brick[channelOffset + cubicCoordToLinear(svec3(pos.x,   pos.y+1, pos.z),   brickDim)] +  // ULF
                (uint64_t)brick[channelOffset + cubicCoordToLinear(svec3(pos.x,   pos.y+1, pos.z+1), brickDim)] +  // ULB
                (uint64_t)brick[channelOffset + cubicCoordToLinear(svec3(pos.x+1, pos.y,   pos.z),   brickDim)] +  // LRF
                (uint64_t)brick[channelOffset + cubicCoordToLinear(svec3(pos.x+1, pos.y,   pos.z+1), brickDim)] +  // LRB
                (uint64_t)brick[channelOffset + cubicCoordToLinear(svec3(pos.x+1, pos.y+1, pos.z),   brickDim)] +  // URF
                (uint64_t)brick[channelOffset + cubicCoordToLinear(svec3(pos.x+1, pos.y+1, pos.z+1), brickDim)] ;  // URB
            halfValue /= 8;

            halfSampledBrick[halfChannelOffset + cubicCoordToLinear(halfPos, halfDim)] = static_cast<uint16_t>(halfValue);
        }
    }
}

VolumeOctreeNode* VolumeOctree::createParentNode(VolumeOctreeNode* children[8], uint64_t brickVirtualMemoryAddress) {
    tgtAssert(brickPoolManager_, "no brick pool manager");

    // OPTIMIZATION: half-sample directly into target brick buffer

    // half sample child bricks and compute avg value
    svec3 halfSampleBrickDim = getBrickDim() / svec3(2);
    size_t halfSampleBufferSize = tgt::hmul(halfSampleBrickDim)*getNumChannels();
    size_t brickBufferSize = halfSampleBufferSize*8;

    // store all half sampled bricks in one temporary buffer
    uint16_t* halfSampledBrickBuffer = new uint16_t[brickBufferSize];

    tgtAssert(getNumChannels() <= MAX_CHANNELS, "more than max channels");
    uint64_t avgValuesUInt64[MAX_CHANNELS];
    uint16_t minValues[MAX_CHANNELS], maxValues[MAX_CHANNELS];
    for (size_t ch=0; ch<getNumChannels(); ch++) {
        avgValuesUInt64[ch] = 0;
        minValues[ch] = (uint16_t)(-1); //< max uint16_t
        maxValues[ch] = 0;
    }

    for (size_t childID=0; childID<8; childID++) {
        VolumeOctreeNode* child = children[childID];
        tgtAssert(child, "null pointer");
        if (child->hasBrick()) { // node brick present => halfsample into buffer
            halfSampleBrick(brickPoolManager_->getBrick(child->getBrickAddress()), getBrickDim(), &halfSampledBrickBuffer[childID*halfSampleBufferSize]);
            brickPoolManager_->setBrickUnused(child->getBrickAddress());
        }
        else { // no brick present => use node's avg values
            for (size_t channel=0; channel<getNumChannels(); channel++) {
                size_t childAndChannelOffset = childID*halfSampleBufferSize + channel*tgt::hmul(halfSampleBrickDim);
                for (size_t voxel=0; voxel<tgt::hmul(halfSampleBrickDim); voxel++) {
                    size_t bufferIndex = childAndChannelOffset + voxel;
                    tgtAssert(bufferIndex < brickBufferSize, "invalid buffer index");
                    halfSampledBrickBuffer[bufferIndex] = child->getAvgValue(channel);
                }
            }
        }

        for (size_t ch=0; ch<getNumChannels(); ch++) {
            avgValuesUInt64[ch] += child->getAvgValue(ch);
            minValues[ch] = std::min(minValues[ch], child->getMinValue(ch));
            maxValues[ch] = std::max(maxValues[ch], child->getMaxValue(ch));
        }
    }

    uint16_t avgValues[MAX_CHANNELS];
    for (size_t ch=0; ch<getNumChannels(); ch++) {
        avgValues[ch] = static_cast<uint16_t>(avgValuesUInt64[ch] / 8);
    }

    // copy halfsampled bricks to dest buffer (child nodes/halfsampled bricks are expected to be zyx ordered)
    uint16_t* brickBuffer = brickPoolManager_->getWritableBrick(brickVirtualMemoryAddress);
    VRN_FOR_EACH_VOXEL(child, svec3::zero, svec3::two) {
        const size_t childOffset = cubicCoordToLinear(child, svec3::two)*halfSampleBufferSize;
        svec3 halfSampleOffset = child * halfSampleBrickDim;
        for (size_t channel=0; channel<getNumChannels(); channel++) {
            size_t channelOffsetInBrick = channel*tgt::hmul(getBrickDim());
            tgtAssert(channelOffsetInBrick < brickBufferSize, "invalid offset");

            uint16_t* halfSampledBrick = halfSampledBrickBuffer + childOffset;
            uint16_t* channelBrickBuffer = brickBuffer + channelOffsetInBrick;
            tgtAssert(brickBuffer, "no brick buffer allocated");
            copyBrickToTexture(halfSampledBrick, halfSampleBrickDim, channel, channelBrickBuffer, getBrickDim(), halfSampleOffset);
        }
    }
    delete halfSampledBrickBuffer;

    // create parent node
    VolumeOctreeNode* parent = VolumeOctreeBase::createNode(getNumChannels(), avgValues, minValues, maxValues,
        brickVirtualMemoryAddress, children);

    brickPoolManager_->setBrickUnused(brickVirtualMemoryAddress);

    return parent;
}

void VolumeOctree::composeNodeTexture(const VolumeOctreeNode* node, const svec3& nodeOffset, size_t curLevel, size_t targetLevel,
    size_t channel, uint16_t* textureBuffer, const svec3& textureDim) const
{
    tgtAssert(brickPoolManager_, "no brick pool manager");
    tgtAssert(node, "null pointer passed");
    tgtAssert(curLevel >= targetLevel && curLevel < getNumLevels(), "invalid current level");
    tgtAssert(textureBuffer, "null pointer passed");
    tgtAssert(tgt::hand(tgt::lessThanEqual(nodeOffset, getOctreeDim())), "node offset larger than texture dimensions");

    if (curLevel == targetLevel) { // final level => copy brick texture to target buffer (or use avg value)
        if (node->hasBrick()) {
            copyBrickToTexture(brickPoolManager_->getBrick(node->getBrickAddress()), getBrickDim(), channel, textureBuffer, textureDim, nodeOffset);
        }
        else { // no brick texture stored => use avg value
            VRN_FOR_EACH_VOXEL(voxel, nodeOffset, nodeOffset+getBrickDim()) {
                if (tgt::hand(tgt::lessThan(voxel, textureDim))) {
                    textureBuffer[cubicCoordToLinear(voxel, textureDim)] = node->getAvgValue(channel);
                }
            }
        }
    }
    else { // higher level => let child nodes copy their sub-node textures to target texture (or use avg value)
        svec3 nodeDim = getBrickDim() * svec3(1<<(curLevel - targetLevel));
        if (node->children_[0]) {
            // note: child nodes are zyx ordered
            svec3 subNodeDim = nodeDim / svec3(2);
            VRN_FOR_EACH_VOXEL(childCoord, svec3::zero, svec3::two) {
                const VolumeOctreeNode* child = node->children_[cubicCoordToLinear(childCoord, svec3::two)];
                tgtAssert(child, "no child node");
                composeNodeTexture(child, nodeOffset + childCoord*subNodeDim, curLevel-1, targetLevel, channel, textureBuffer, textureDim);
            }
        }
        else { // no child node => use avg value
            VRN_FOR_EACH_VOXEL(voxel, nodeOffset, nodeOffset+nodeDim) {
                if (tgt::hand(tgt::lessThan(voxel, textureDim))) {
                    textureBuffer[cubicCoordToLinear(voxel, textureDim)] = node->getAvgValue(channel);
                }
            }
        }
    }
}

void VolumeOctree::composeNodeSliceTexture(SliceAlignment sliceAlignment, const VolumeOctreeNode* node,
    const tgt::svec3& nodeOffsetInTexture, size_t sliceIndexInNode, size_t curLevel, size_t targetLevel, size_t channel,
    uint16_t* textureBuffer, const tgt::svec3& textureDim) const
{
    tgtAssert(brickPoolManager_, "no brick pool manager");
    tgtAssert(node, "null pointer passed");
    tgtAssert(curLevel >= targetLevel && curLevel < getNumLevels(), "invalid current level");
    tgtAssert(textureBuffer, "null pointer passed");
    tgtAssert(tgt::hand(tgt::lessThan(nodeOffsetInTexture, getOctreeDim())), "node offset larger than octree dimensions");
    tgtAssert(tgt::hor(tgt::lessThan(nodeOffsetInTexture, getVolumeDim())), "node offset outside volume dimensions");

    const svec3 brickDim = getBrickDim();
    const svec3 nodeDimInTexture = brickDim * svec3(1<<(curLevel-targetLevel));
    tgtAssert(sliceIndexInNode < nodeDimInTexture[sliceAlignment], "invalid slice index");

    if (node->isHomogeneous()) { // homogeneous => no brick and no children => use avg value
        svec3 textureVoxelStart = nodeOffsetInTexture;
        svec3 textureVoxelEnd = textureVoxelStart + nodeDimInTexture;
        textureVoxelStart[sliceAlignment] = 0;
        textureVoxelEnd[sliceAlignment] = 1;
        textureVoxelEnd = tgt::min(textureVoxelEnd, textureDim); //< node might lie partially outside target texture (NPOT)
        VRN_FOR_EACH_VOXEL(textureVoxel, textureVoxelStart, textureVoxelEnd) {
            textureBuffer[cubicCoordToLinear(textureVoxel, textureDim)] = node->getAvgValue(channel);
        }
    }
    else if (curLevel == targetLevel) { // final level => copy brick slice to target texture
        tgtAssert(sliceIndexInNode < getBrickDim().z, "slice index outside brick");
        tgtAssert(node->hasBrick(), "node has no brick (should not get here)");
        const uint16_t* brick = brickPoolManager_->getBrick(node->getBrickAddress());

        const size_t channelOffset = tgt::hmul(brickDim)*channel;
        svec3 textureVoxelStart = nodeOffsetInTexture;
        svec3 textureVoxelEnd = textureVoxelStart + brickDim;
        textureVoxelStart[sliceAlignment] = 0;
        textureVoxelEnd[sliceAlignment] = 1;
        textureVoxelEnd = tgt::min(textureVoxelEnd, textureDim); //< node might lie partially outside target texture (NPOT)
        VRN_FOR_EACH_VOXEL(textureVoxel, textureVoxelStart, textureVoxelEnd) {
            size_t textureLinearCoord = cubicCoordToLinear(textureVoxel, textureDim);
            svec3 brickVoxel = textureVoxel - textureVoxelStart;
            brickVoxel[sliceAlignment] += sliceIndexInNode;
            tgtAssert(tgt::hand(tgt::lessThan(brickVoxel, brickDim)), "invalid brick voxel");
            size_t brickLinearCoord = channelOffset + cubicCoordToLinear(brickVoxel, brickDim);
            textureBuffer[textureLinearCoord] = brick[brickLinearCoord];
        }
    }
    else { // inner node => let child nodes recursively copy their sub-node slice textures to target texture
        // note: child nodes are zyx ordered
        tgtAssert(!node->isLeaf() && node->children_[0], "node has no children (should not get here)");

        svec3 childNodeDim = nodeDimInTexture / svec3(2);
        size_t childLayer;
        if (sliceIndexInNode < childNodeDim[sliceAlignment]) { //< slice lies in lower four child nodes
            childLayer = 0;
        }
        else { //< slice lies in upper four child nodes
            childLayer = 1;
            sliceIndexInNode -= childNodeDim[sliceAlignment];
        }
        svec3 childStart = svec3(0, 0, 0);
        svec3 childEnd = svec3(2, 2, 2);
        childStart[sliceAlignment] = childLayer;
        childEnd[sliceAlignment] = childLayer+1;
        VRN_FOR_EACH_VOXEL(childCoord, childStart, childEnd) {
            const VolumeOctreeNode* child = node->children_[cubicCoordToLinear(childCoord, svec3::two)];
            tgtAssert(child, "no child node");
            svec3 childNodeOffset = nodeOffsetInTexture + childCoord*childNodeDim;
            composeNodeSliceTexture(sliceAlignment, child, childNodeOffset, sliceIndexInNode, curLevel-1, targetLevel,
                channel, textureBuffer, textureDim);
        }
    }
}

const VolumeOctreeNode* VolumeOctree::getNodeAtVoxel(const svec3& voxel, const size_t curLevel, const size_t targetLevel,
    const VolumeOctreeNode* node, const svec3& nodeLlf, const svec3& nodeUrb,
    size_t& resultLevel, svec3& resultLlf, svec3& resultUrb) const
{
    tgtAssert(node, "null pointer passed");
    tgtAssert(curLevel >= targetLevel && curLevel < getNumLevels(), "invalid current level");
    tgtAssert(inRange(voxel, nodeLlf, nodeUrb-svec3(1)), "point coords outside node dimensions");

    if (curLevel == targetLevel || !node->children_[0]) { // current node level requested, or current node is leaf => stop descent
        resultLevel = curLevel;
        resultLlf = nodeLlf;
        resultUrb = nodeUrb;
        return node;
    }
    else {
        svec3 nodeDim = nodeUrb - nodeLlf;
        svec3 nodeHalfDim = nodeDim/svec3(2);
        svec3 voxelOffset = voxel - nodeLlf;
        tgtAssert(inRange(voxelOffset, svec3::zero, nodeDim-svec3(1)), "invalid voxel offset");
        svec3 childNodeID = voxelOffset / nodeHalfDim;
        tgtAssert(inRange(childNodeID, svec3::zero, svec3::one), "invalid child node id");

        const VolumeOctreeNode* childNode = node->children_[cubicCoordToLinear(childNodeID, svec3::two)];
        tgtAssert(childNode, "child node is null");
        svec3 childLlf = nodeLlf + childNodeID*nodeHalfDim;
        svec3 childUrb = childLlf + nodeHalfDim;
        const VolumeOctreeNode* resultNode = getNodeAtVoxel(voxel, curLevel-1, targetLevel,
            childNode, childLlf, childUrb, resultLevel, resultLlf, resultUrb);
        tgtAssert(resultNode, "null pointer returned as node");
        return resultNode;
    }
}

void VolumeOctree::serialize(XmlSerializer& s) const {
    if (!brickPoolManager_)
        throw SerializationException("Unable to serialize octree: no brick pool manager assigned");

    // determine output path for node buffer
    const std::string octreeFile = s.getDocumentPath();
    const std::string octreePath = tgt::FileSystem::dirName(octreeFile);
    if (octreeFile.empty() || !tgt::FileSystem::dirExists(octreePath))
        throw SerializationException("Octree path does not exist: " + octreePath);

    // serialize base octree
    VolumeOctreeBase::serialize(s);

    // serialize tree to binary buffer
    char* nodeBuffer = 0;
    size_t bufferSize = 0;
    serializeNodeBuffer(nodeBuffer, bufferSize);
    tgtAssert(nodeBuffer, "null pointer returned");
    tgtAssert(bufferSize, "invalid buffer size returned");

    // write node buffer to file
    const std::string bufferFile = tgt::FileSystem::cleanupPath(octreePath + "/nodebuffer.raw");
    std::fstream fileStream(bufferFile.c_str(), std::ios_base::out | std::ios_base::binary);
    if (fileStream.fail()) {
        delete[] nodeBuffer;
        throw SerializationException("Failed to open file '" + bufferFile + "' for writing");
    }
    try {
        fileStream.write(nodeBuffer, bufferSize);
    }
    catch (std::exception& e) {
        delete[] nodeBuffer;
        fileStream.close();
        throw SerializationException("Failed to write node buffer to file '" + bufferFile + "': " + std::string(e.what()));
    }
    fileStream.close();
    delete[] nodeBuffer;
    nodeBuffer = 0;

    // serialize node buffer meta information
    s.serialize("nodeCount", rootNode_->getNodeCount());
    s.serialize("nodeBufferSize", bufferSize);

    // serialize brick pool manager
    s.serialize("brickPoolManager", brickPoolManager_);
}

void VolumeOctree::deserialize(XmlDeserializer& s) {
    // determine output path for node buffer
    const std::string octreeFile = s.getDocumentPath();
    const std::string octreePath = tgt::FileSystem::dirName(octreeFile);
    if (octreeFile.empty() || !tgt::FileSystem::dirExists(octreePath))
        throw SerializationException("Octree path does not exist: " + octreePath);

    // deserialize base octree properties
    VolumeOctreeBase::deserialize(s);

    // deserialize node buffer meta information
    size_t nodeCount, nodeBufferSize;
    s.deserialize("nodeCount", nodeCount);
    if (nodeCount == 0)
        throw SerializationException("Invalid node count: " + itos(nodeCount));
    s.deserialize("nodeBufferSize", nodeBufferSize);
    if (nodeBufferSize == 0 || nodeBufferSize < nodeCount)
        throw SerializationException("Invalid node buffer size: " + itos(nodeBufferSize));

    // load binary node buffer from file
    const std::string bufferFile = tgt::FileSystem::cleanupPath(octreePath + "/nodebuffer.raw");
    std::fstream fileStream(bufferFile.c_str(), std::ios_base::in | std::ios_base::binary);
    if (fileStream.fail())
        throw SerializationException("Failed to open node buffer file '" + bufferFile + "' for reading");
    char* nodeBuffer = new char[nodeBufferSize];
    try {
        fileStream.read(nodeBuffer, nodeBufferSize);
    }
    catch (std::exception& e) {
        delete[] nodeBuffer;
        fileStream.close();
        throw SerializationException("Failed to read node buffer from file '" + bufferFile + "': " + std::string(e.what()));
    }
    fileStream.close();

    // construct tree nodes from node buffer
    if (rootNode_)
        deleteSubTree(rootNode_);
    rootNode_ = 0;
    try {
        tgt::Stopwatch watch;
        watch.start();
        rootNode_ = deserializeNodeBuffer(nodeBuffer, nodeCount, nodeBufferSize);
        LDEBUG("Node buffer deserialization time: " << watch.getRuntime() << " msec");
    }
    catch (std::exception& e) {
        delete[] nodeBuffer;
        throw SerializationException("Failed to deserialize binary node buffer '" + bufferFile + "': " + std::string(e.what()));
    }
    tgtAssert(rootNode_, "no root node"); //< exception expected from deserializeNodeBuffer
    delete[] nodeBuffer;
    nodeBuffer = 0;

    // deserialize brick pool manager
    delete brickPoolManager_;
    brickPoolManager_ = 0;
    tgt::Stopwatch watch;
    watch.start();
    s.deserialize("brickPoolManager", brickPoolManager_);
    LDEBUG("Brick pool manager deserialization time: " << watch.getRuntime() << " msec");
    if (!brickPoolManager_) {
        deleteSubTree(rootNode_);
        rootNode_ = 0;
        throw SerializationException("Brick pool manager not deserialized");
    }

}

void VolumeOctree::serializeNodeBuffer(char*& binaryBuffer, size_t& bufferSize) const
    throw (SerializationException)
{
    tgtAssert(rootNode_, "no root node");

    const size_t NODE_CONTENT_SIZE = rootNode_->getContentSize();
    const size_t NODE_SIZE = NODE_CONTENT_SIZE + sizeof(uint64_t); // content size + child group offset
    const size_t NODE_COUNT = rootNode_->getNodeCount();

    bufferSize = NODE_COUNT*NODE_SIZE;
    binaryBuffer = new char[bufferSize];

    // pair consisting of a octree node whose children still have to be added to the buffer, and the node's buffer offset
    typedef std::pair<const VolumeOctreeNode*, size_t> QuededNode;
    std::stack<QuededNode> workQueue;

    // start with root node, put encountered nodes into fifo queue, iterate until queue is empty
    rootNode_->serializeContentToBinaryBuffer(binaryBuffer);
    workQueue.push(QuededNode(rootNode_, 0));
    size_t curBufferOffset = 1; //< next after root node
    while (!workQueue.empty()) {
        // retrieve next node to process
        const VolumeOctreeNode* curNode = workQueue.top().first;
        size_t nodeOffset = workQueue.top().second;
        workQueue.pop();

        // no children => nothing to do
        if (!curNode->children_[0])
            continue;

        tgtAssert(nodeOffset < curBufferOffset,  "node offset not less than current offset");
        tgtAssert(curBufferOffset+8 <= NODE_COUNT, "invalid current buffer offset");

        // set curNode's child group pointer to current offset
        size_t nodeByteOffset = nodeOffset*NODE_SIZE;
        tgtAssert(nodeByteOffset < bufferSize, "invalid byte offset");
        *reinterpret_cast<uint64_t*>(binaryBuffer + nodeByteOffset + NODE_CONTENT_SIZE) = curBufferOffset;

        // create eight adjacent buffer entries at curOffset for children, and add them to work queue
        for (size_t childID = 0; childID < 8; childID++) {
            const VolumeOctreeNode* child = curNode->children_[childID];
            tgtAssert(child, "missing child");
            size_t childOffset = curBufferOffset+childID;

            size_t childByteOffset = childOffset*NODE_SIZE;
            tgtAssert(childByteOffset < bufferSize, "invalid child byte offset");
            child->serializeContentToBinaryBuffer(binaryBuffer + childByteOffset);
            // assign max uint64_t as child group offset to nodes without children
            *reinterpret_cast<uint64_t*>(binaryBuffer + childByteOffset + NODE_CONTENT_SIZE) = std::numeric_limits<uint64_t>::max();

            workQueue.push(QuededNode(child, childOffset));
        }
        curBufferOffset += 8;
    }
    tgtAssert(curBufferOffset == NODE_COUNT, "buffer offset does not equal number of tree nodes");

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

VolumeOctreeNode* VolumeOctree::deserializeNodeBuffer(const char* binaryBuffer, const size_t nodeCount, const size_t bufferSize)
    throw (SerializationException)
{
    tgtAssert(binaryBuffer, "null pointer passed");
    tgtAssert(nodeCount > 0, "invalid node count");
    tgtAssert(bufferSize > 0, "invalid buffer size");

    VolumeOctreeNode* rootNode = VolumeOctreeBase::createNode(getNumChannels());
    const size_t NODE_CONTENT_SIZE = rootNode->getContentSize();
    const size_t NODE_SIZE = NODE_CONTENT_SIZE + sizeof(uint64_t); // content size + child group offset

    // check buffer size against node count
    if (bufferSize != nodeCount*NODE_SIZE) {
        delete rootNode;
        throw SerializationException("Node buffer byte size does not match nodeCount*numBytesPerNode [" +
                                      itos(bufferSize) + " != " + itos(nodeCount) + "*" + itos(NODE_SIZE) + "]");
    }

    // pair consisting of a octree node whose children still have to be read from the buffer, and the children group's buffer offset
    typedef std::pair<VolumeOctreeNode*, uint64_t> QuededNode;
    std::stack<QuededNode> workQueue;

    // create root node from first buffer entry
    rootNode->deserializeContentFromBinaryBuffer(binaryBuffer);
    uint64_t childGroupOffset = *reinterpret_cast<const uint64_t*>(binaryBuffer + NODE_CONTENT_SIZE);
    if (childGroupOffset < std::numeric_limits<uint64_t>::max()) {
        if (childGroupOffset >= bufferSize-8*NODE_SIZE) {
            delete rootNode;
            throw SerializationException("Invalid child group offset: " + itos((size_t)childGroupOffset));
        }
        workQueue.push(std::pair<VolumeOctreeNode*, uint64_t>(rootNode, childGroupOffset));
    }

    // process work queue
    while (!workQueue.empty()) {
        // retrieve next node/child group offset to process
        VolumeOctreeNode* curNode = workQueue.top().first;
        uint64_t childGroupOffset = workQueue.top().second;
        tgtAssert(childGroupOffset < bufferSize-8*NODE_SIZE, "invalid child group offset");
        workQueue.pop();

        // iterate over child group and create nodes from buffer entries
        for (size_t i=0; i<8; i++) {
            // create child node
            VolumeOctreeNode* childNode = VolumeOctreeBase::createNode(getNumChannels());
            const char* childBuffer = binaryBuffer + (childGroupOffset+i)*NODE_SIZE;
            childNode->deserializeContentFromBinaryBuffer(childBuffer);

            // retrieve child group offset and add to work queue, if children present
            uint64_t grandChildGroupOffset = *reinterpret_cast<const uint64_t*>(childBuffer + NODE_CONTENT_SIZE);
            if (grandChildGroupOffset < std::numeric_limits<uint64_t>::max()) {
                if (grandChildGroupOffset >= bufferSize-8*NODE_SIZE) {
                    deleteSubTree(rootNode);
                    throw SerializationException("Invalid child group offset: " + itos((size_t)grandChildGroupOffset));
                }
                workQueue.push(std::pair<VolumeOctreeNode*, uint64_t>(childNode, grandChildGroupOffset));
            }

            curNode->children_[i] = childNode;
        }
    }
    tgtAssert(rootNode, "no root node");
    if (rootNode->getNodeCount() != nodeCount) {
        deleteSubTree(rootNode);
        throw SerializationException("Node count of deserialized octree does not match specified node count [" +
                                      itos(rootNode->getNodeCount()) + " != " + itos(nodeCount) + "]");
    }

    return rootNode;
}

} // namespace
