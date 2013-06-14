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

#ifndef VRN_VOLUMEOCTREE_H
#define VRN_VOLUMEOCTREE_H

#include "volumeoctreebase.h"
#include "octreebrickpoolmanager.h"

#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/io/progressbar.h"
#include "voreen/core/utils/exception.h"

#include <vector>
#include <string>

namespace voreen {

/**
 * Basic multi-channel octree implementation that creates the octree from one or multiple channel volumes.
 * The input volumes must be RAM-resident, i.e., VolumeRAM representations must be available during
 * octree construction.
 *
 * The octree can be optimized: Bricks or subtrees that represent a volume region whose
 * max-min difference does not exceed a specified threshold are discarded.
 *
 * @note Currently, only uint16_t volumes are supported as input.
 */
class VRN_CORE_API VolumeOctree : public VolumeOctreeBase {

public:
    /**
     * Standard constructor for the construction of a multi-channel octree.
     *
     * @param channelVolumes Input volumes the octree is constructed from. Up to four channels are supported.
     *        All channel volumes must have the same dimensions and format.  Only uint16_t is supported as voxel type.
     * @param brickDim dimensions of one brick (number of voxels per dimension). Must be power-of-two.
     * @param homogeneityThreshold All regions within the volume with a value range (i.e., max-min)
     *        less or equal this threshold are considered homogeneous and will be discarded in the octree.
     *        The threshold is normalized, i.e., 1.0 represents the data type's full value range.
     *        A negative threshold disables octree optimization, resulting in a complete tree.
     * @param brickPoolManager Mandatory helper class that organizes the bricks in RAM/disk memory.
     *        The octree takes ownership of the passed manager and deletes it on its own destruction.
     * @param progressBar Optional progress bar that is updated during tree construction.
     *
     * @throws std::exception If the octree construction fails.
     */
    VolumeOctree(const std::vector<const VolumeBase*>& channelVolumes, size_t brickDim, float homogeneityThreshold = 0.001f,
        OctreeBrickPoolManagerBase* brickPoolManager = new OctreeBrickPoolManagerRAM(),
        ProgressBar* progessBar = 0)
        throw (std::exception);

    /**
     * Convenience constructor for a single-channel octree.
     */
    VolumeOctree(const VolumeBase* volume, size_t brickDim, float homogeneityThreshold = 0.001f,
        OctreeBrickPoolManagerBase* brickPoolManager = new OctreeBrickPoolManagerRAM(),
        ProgressBar* progessBar = 0)
        throw (std::exception);

    /// Default constructor for serialization only.
    VolumeOctree();
    virtual ~VolumeOctree();
    virtual VolumeOctree* create() const;

    virtual std::string getClassName() const { return "VolumeOctree"; }

    virtual size_t getActualTreeDepth() const;

    virtual size_t getNumNodes() const;

    virtual size_t getNumBricks() const;

    virtual uint64_t getBrickPoolMemoryAllocated() const;

    virtual uint64_t getBrickPoolMemoryUsed() const;

    virtual std::string getDescription() const;


    virtual const VolumeOctreeNode* getRootNode() const;

    virtual const VolumeOctreeNode* getNode(const tgt::vec3& point, size_t& level,
        tgt::svec3& voxelLLF, tgt::svec3& voxelURB, tgt::vec3& normLLF, tgt::vec3& normURB) const;

    virtual const uint16_t* getNodeBrick(const VolumeOctreeNode* node) const
        throw (VoreenException);

    virtual void releaseNodeBrick(const VolumeOctreeNode* node) const;

    virtual VolumeRAM* createVolume(size_t level = 0, size_t channel = 0) const
        throw (VoreenException);

    virtual VolumeRAM* createSlice(SliceAlignment sliceAlignment, size_t sliceIndex, size_t level = 0, size_t channel = 0) const
        throw (VoreenException);


    virtual void serialize(XmlSerializer& s) const;

    virtual void deserialize(XmlDeserializer& s);

protected:
    static const std::string loggerCat_;

private:
    void buildOctree(const std::vector<const VolumeBase*>& volumes, bool octreeOptimization, uint16_t homogeneityThreshold,
        ProgressBar* progessBar)
        throw (VoreenException);

    VolumeOctreeNode* createTreeNode(const tgt::svec3& llf, const tgt::svec3& urb,
        const std::vector<const uint16_t*>& textureBuffers, const tgt::svec3& textureDim,
        bool octreeOptimization, uint16_t homogeneityThreshold,
        uint16_t* avgValues, uint16_t* minValues, uint16_t* maxValues,
        ProgressBar* progessBar);

    VolumeOctreeNode* createParentNode(VolumeOctreeNode* children[8], uint64_t brickVirtualMemoryAddress);

    const VolumeOctreeNode* getNodeAtVoxel(const tgt::svec3& voxel, const size_t curLevel, const size_t targetLevel,
        const VolumeOctreeNode* node, const tgt::svec3& nodeLlf, const tgt::svec3& nodeUrb,
        size_t& resultLevel, tgt::svec3& resultLlf, tgt::svec3& resultUrb) const;

    void composeNodeTexture(const VolumeOctreeNode* node, const tgt::svec3& nodeOffset, size_t curLevel, size_t targetLevel,
        size_t channel, uint16_t* textureBuffer, const tgt::svec3& textureDim) const;

    void composeNodeSliceTexture(SliceAlignment sliceAlignment, const VolumeOctreeNode* node,
        const tgt::svec3& nodeOffsetInTexture, size_t sliceIndexInNode, size_t curLevel, size_t targetLevel, size_t channel,
        uint16_t* textureBuffer, const tgt::svec3& textureDim) const;

    void serializeNodeBuffer(char*& binaryBuffer, size_t& bufferSize) const
        throw (SerializationException);

    VolumeOctreeNode* deserializeNodeBuffer(const char* binaryBuffer, const size_t nodeCount, const size_t bufferSize)
        throw (SerializationException);

    // low-level helper functions
    void deleteSubTree(VolumeOctreeNode* root) const;

    void extractBrickFromTexture(const std::vector<const uint16_t*>& textures, const tgt::svec3& textureDim,
        uint16_t* brickBuffer, const tgt::svec3& brickDim, const tgt::svec3& brickOffsetInTexture,
        uint16_t* avgValues, uint16_t* minValues, uint16_t* maxValues) const;

    void copyBrickToTexture(const uint16_t* brick, const tgt::svec3& brickDim, size_t channel,
        uint16_t* texture, const tgt::svec3& textureDim,
        const tgt::svec3& brickOffsetInTexture) const;

    void halfSampleBrick(const uint16_t* brick, const tgt::svec3& brickDim, uint16_t* halfSampledBrick) const;

    VolumeOctreeNode* rootNode_;

    OctreeBrickPoolManagerBase* brickPoolManager_;
};

} // namespace

#endif
