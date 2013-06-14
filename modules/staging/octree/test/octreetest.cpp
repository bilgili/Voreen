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

#include "voreen/core/voreenapplication.h"
#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/network/workspace.h"
#include "voreen/core/network/processornetwork.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

#include "modules/core/io/vvdvolumereader.h"
#include "modules/core/io/vvdvolumewriter.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorresizepoweroftwo.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorresize.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorhalfsample.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorequalize.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorsubset.h"

#include "tgt/init.h"
#include "tgt/logmanager.h"
#include "tgt/camera.h"
#include "tgt/shadermanager.h"
#include "tgt/stopwatch.h"

#include "modules/staging/octree/datastructures/volumeoctree.h"
#include "modules/staging/octree/datastructures/octreeutils.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE OctreeTests
#include <boost/test/unit_test.hpp>

using namespace voreen;
using tgt::svec3;
using tgt::vec3;
using tgt::ivec3;

const std::string loggerCat_("octreetest");

//-----------------------------------------------------------------------------
// helper functions

inline bool pointsNeighbored(const svec3& p1, const svec3& p2) {
    return (absDiff(p1.x, p2.x) <= 1 && absDiff(p1.y, p2.y) <= 1 && absDiff(p1.z, p2.z) <= 1);
}

bool closeToBoxCorner(const svec3& point, const svec3& llf, const svec3& urb) {
    tgtAssert(tgt::hand(tgt::greaterThan(urb, llf)), "invalid llf-urb combination");
    return (pointsNeighbored(point, svec3(llf.x, llf.y, llf.z)) ||
            pointsNeighbored(point, svec3(llf.x, llf.y, urb.z)) ||
            pointsNeighbored(point, svec3(llf.x, urb.y, llf.z)) ||
            pointsNeighbored(point, svec3(llf.x, urb.y, urb.z)) ||
            pointsNeighbored(point, svec3(urb.x, llf.y, llf.z)) ||
            pointsNeighbored(point, svec3(urb.x, llf.y, urb.z)) ||
            pointsNeighbored(point, svec3(urb.x, urb.y, llf.z)) ||
            pointsNeighbored(point, svec3(urb.x, urb.y, urb.z))   );
}

bool closeToBoxCorner(const vec3& point, const vec3& llf, const vec3& urb, float epsilonSq = 1e-6f) {
    tgtAssert(tgt::hand(tgt::greaterThan(urb, llf)), "invalid llf-urb combination");
    return ( (tgt::lengthSq(point - vec3(llf.x, llf.y, llf.z)) <= epsilonSq) ||
             (tgt::lengthSq(point - vec3(llf.x, llf.y, urb.z)) <= epsilonSq) ||
             (tgt::lengthSq(point - vec3(llf.x, urb.y, llf.z)) <= epsilonSq) ||
             (tgt::lengthSq(point - vec3(llf.x, urb.y, urb.z)) <= epsilonSq) ||
             (tgt::lengthSq(point - vec3(urb.x, llf.y, llf.z)) <= epsilonSq) ||
             (tgt::lengthSq(point - vec3(urb.x, llf.y, urb.z)) <= epsilonSq) ||
             (tgt::lengthSq(point - vec3(urb.x, urb.y, llf.z)) <= epsilonSq) ||
             (tgt::lengthSq(point - vec3(urb.x, urb.y, urb.z)) <= epsilonSq)   );
}

/// Returns the number of nodes of a complete octree of the specified depth.
size_t getNodeCount(size_t numLevels) {
    tgtAssert(numLevels > 0, "num levels must be greater zero");
    size_t result = 0;
    for (size_t l=0; l<numLevels; l++) {
        result += tgt::hmul(svec3(1<<l));
    }
    return result;
}

/// Extracts the data buffer (voxel array) from the passed UInt16 volume.
const uint16_t* getUInt16DataBuffer(const Volume* volume) {
    const VolumeRAM_UInt16* uint16Volume = dynamic_cast<const VolumeRAM_UInt16*>(volume->getRepresentation<VolumeRAM>());
    if (!uint16Volume) {
        LERROR("Passed volume is not uint16_t");
        return 0;
    }
    return reinterpret_cast<const uint16_t*>(uint16Volume->getData());
}

uint16_t* getUInt16DataBuffer(VolumeRAM* volume) {
    VolumeRAM_UInt16* uint16Volume = dynamic_cast<VolumeRAM_UInt16*>(volume);
    if (!uint16Volume) {
        LERROR("Passed volume is not uint16_t");
        return 0;
    }
    return reinterpret_cast<uint16_t*>(uint16Volume->getData());
}


//-----------------------------------------------------------------------------
// test data creation functions

/**
 * Simple random generator with reproducible results, only depending on two seeds.
 * http://www.codeproject.com/Articles/25172/Simple-Random-Number-Generation
 */
struct RandomNumberGenerator {
    uint32_t mz_, mw_;

    RandomNumberGenerator(uint32_t mz=3, uint32_t mw=17) {
        tgtAssert(mz != 0 && mw != 0, "seeds must be non-zero");
        mz_ = mz;
        mw_ = mw;
    }

    uint32_t getUInt32() {
        mz_ = 36969 * (mz_ & 65535) + (mz_ >> 16);
        mw_ = 18000 * (mw_ & 65535) + (mw_ >> 16);
        return (mz_ << 16) + mw_;
    }

    uint16_t getUInt16() {
        return static_cast<uint16_t>(getUInt32() >> 16);
    }
};

/**
 * Creates a uint16_t volume with random values.
 * For constant arguments the function is guaranteed to return the same result.
 *
 * @param dimensions dimensions of the volume to create.
 * @param mz,mw seed values for the random number generator, must be non-zero.
 */
Volume* createNoiseVolume(const svec3& dimensions, uint32_t mz=3, uint32_t mw=17) {
    RandomNumberGenerator rng(mz, mw);
    VolumeRAM_UInt16* volume = new VolumeRAM_UInt16(dimensions, true);
    for (size_t i=0; i<tgt::hmul(dimensions); i++)
        volume->voxel(i) = rng.getUInt16();

    return new Volume(volume, vec3(1.f), vec3(0.f));
}

/**
 * Creates a uint16_t volume consisting of homogeneous regions (bricks).
 * For constant arguments the function is guaranteed to return the same result.
 *
 * @param dimensions dimensions of the volume to create.
 * @param brickSize dimensions of the homogeneous regions. Must be a divisor of the volume size (dimension-wise).
 * @param mz,mw seed values for the random number generator, must be non-zero.
 * @param minNeighborDiff minimal value difference between neighboring bricks
 */
Volume* createBrickedVolume(const svec3& dimensions, const svec3& brickSize,
        uint32_t mz=3, uint32_t mw=17, uint16_t minNeibhborDiff = 1<<13)
{
    tgtAssert(tgt::hand(tgt::lessThanEqual(brickSize, dimensions)), "brickSize larger than volume dimensions");
    svec3 numBricks = dimensions / brickSize;
    tgtAssert(numBricks*brickSize == dimensions, "brickSize must be a divisor of the volume dimensions");
    RandomNumberGenerator rng(mz, mw);

    uint16_t* brickValues = new uint16_t[tgt::hmul(numBricks)];

    VolumeRAM_UInt16* volume = new VolumeRAM_UInt16(dimensions, true);
    VRN_FOR_EACH_VOXEL(brick, svec3::zero, numBricks) {

        // generate brick value with minimal difference to neighbor bricks
        uint16_t brickValue;
        bool different = false;
        while (!different) {
            brickValue = rng.getUInt16();
            if (brick.x > 0) {
                uint16_t nValue = brickValues[cubicCoordToLinear(brick-svec3(1, 0, 0), numBricks)];
                if (absDiff(brickValue, nValue) < minNeibhborDiff)
                    continue;
            }
            if (brick.y > 0) {
                uint16_t nValue = brickValues[cubicCoordToLinear(brick-svec3(0, 1, 0), numBricks)];
                if (absDiff(brickValue, nValue) < minNeibhborDiff)
                    continue;
            }
            if (brick.z > 0) {
                uint16_t nValue = brickValues[cubicCoordToLinear(brick-svec3(0, 0, 1), numBricks)];
                if (absDiff(brickValue, nValue) < minNeibhborDiff)
                    continue;
            }
            different = true;
        }
        brickValues[cubicCoordToLinear(brick, numBricks)] = brickValue;

        // assign generated value to brick voxels
        svec3 brickOffset = brick*brickSize;
        VRN_FOR_EACH_VOXEL(voxelInBrick, svec3::zero, brickSize) {
            svec3 voxel = brickOffset + voxelInBrick;
            volume->voxel(voxel) = brickValue;
        }
    }

    return new Volume(volume, vec3(1.f), vec3(0.f));
}

/**
 * Creates a volume by combining the passed 8 octant volumes,
 * which must have equal dimensions. The returned volume has dimensions: 2 * octant dimensions.
 */
Volume* mergeVolumes(const std::vector<const Volume*> octants) {
    tgtAssert(octants.size() == 8, "8 octant volumes expected");
    svec3 octantDim = octants.front()->getDimensions();
    for (size_t i=0; i<octants.size(); i++) {
        tgtAssert(octants.at(i)->getDimensions() == octantDim, "octants have varying dimensions");
    }

    svec3 volumeDim = octantDim*svec3::two;
    VolumeRAM_UInt16* mergedVolume = new VolumeRAM_UInt16(volumeDim, true);
    VRN_FOR_EACH_VOXEL(octant, svec3::zero, svec3::two) {
        svec3 octantOffset = octant*octantDim;
        const Volume* octantVolume = octants.at(cubicCoordToLinear(octant, svec3::two));
        const uint16_t* octantBuffer = getUInt16DataBuffer(octantVolume);
        VRN_FOR_EACH_VOXEL(octantVoxel, svec3::zero, octantDim) {
            mergedVolume->voxel(octantVoxel+octantOffset) = octantBuffer[cubicCoordToLinear(octantVoxel, octantDim)];
        }
    }

    return new Volume(mergedVolume, vec3(1.f), vec3(0.f));
}

/**
 * Adds regular noise to the passed volume
 * by adding/subtracting noiseRange/2.0 to/from each odd/even voxel value.
 */
void addRegularNoise(VolumeRAM* volume, uint16_t noiseRange = 20) {
    tgtAssert(volume, "null pointer passed");
    uint16_t* dataBuffer = getUInt16DataBuffer(volume);
    tgtAssert(dataBuffer, "failed to retrieve uint16 data buffer from volume");

    uint16_t halfNoiseRange = noiseRange / 2;
    for (size_t i=0; i<volume->getNumVoxels(); i++) {
        if (i % 2 == 0) { // even => add halfNoiseRange
            if (dataBuffer[i] >= halfNoiseRange)
                dataBuffer[i] -= halfNoiseRange;
            else
                dataBuffer[i] = 0;
        }
        else { // uneven => subtract halfNoiseRange
            if (dataBuffer[i] <= 65535-halfNoiseRange)
                dataBuffer[i] += halfNoiseRange;
            else
                dataBuffer[i] = 65535;
        }
    }
}

/**
 * Creates a mip map with the specified depth (numLevels) from the passed volume.
 * Level 0 of the returned mip map stores the full resolution volume itself (not a copy!).
 */
std::vector<const Volume*> createMipMap(const Volume* level0, size_t numLevels) {
    INST_SCALAR_TYPES(VolumeOperatorHalfsample, VolumeOperatorHalfsampleGeneric)

    std::vector<const Volume*> levels;
    levels.push_back(level0);
    for (size_t l=1; l<numLevels; l++) {
        Volume* nextLevel = dynamic_cast<Volume*>(VolumeOperatorHalfsample::APPLY_OP(levels.back()));
        tgtAssert(nextLevel && nextLevel->getDimensions() == levels.back()->getDimensions() / tgt::svec3::two, "invalid mip map level");
        levels.push_back(nextLevel);
    }

    return levels;
}

/**
 * Generates grid points of an regular 3D grid.
 * @param gridDist specifies the axis-aligned distance between two grid points
 * @param cellCentered if true, the grid points are centered within their grid cell (=> first grid point is vec3(gridDist/2) ),
 *  otherwise they are located at the lower-left-front the grid cell (=> first grid point is vec3(0) )
 */
std::vector<vec3> generateRegularGridPoints(float gridDist, bool cellCentered=true) {
    if (cellCentered) {
        tgtAssert(gridDist > 0.f && gridDist <= 0.5, "invalid grid dist");
    }
    else {
        tgtAssert(gridDist > 0.f && gridDist <= 1.0, "invalid grid dist");
    }

    svec3 gridPointsPerDim = svec3(tgt::ifloor(1.f/gridDist));
    if (!cellCentered)
        gridPointsPerDim += svec3(1);

    std::vector<vec3> result;
    VRN_FOR_EACH_VOXEL(cell, svec3::zero, gridPointsPerDim) {
        vec3 gridPos = vec3(cell)*vec3(gridDist);
        if (cellCentered)
            gridPos += gridDist/2.f;
        tgtAssert(inRange(gridPos, vec3::zero, vec3::one), "grid pos outside unit cube");
        result.push_back(gridPos);
    }
    tgtAssert(result.size() == tgt::hmul(gridPointsPerDim), "invalid number of grid points");

    return result;
}

Volume* loadVolume(std::string file) {
    VvdVolumeReader vvdReader;
    VolumeBase* volume = 0;
    try {
        VolumeList* coll = vvdReader.read(VoreenApplication::app()->getBasePath(file));
        volume = coll->first();
        delete coll;
    }
    catch (tgt::Exception& e) {
        LERROR(e.what());
    }

    return dynamic_cast<Volume*>(volume);
}

void saveOctreeMipMap(const VolumeOctree& octree, const std::string& basename) {
    for (size_t l=0; l<octree.getNumLevels(); l++) {
        VolumeRAM* levelVolumeRAM = octree.createVolume(l);
        Volume* levelVolume = new Volume(levelVolumeRAM, tgt::vec3::one, tgt::vec3::zero);
        try {
            VvdVolumeWriter().write(VoreenApplication::app()->getBasePath(basename + "-level" + itos(l) + ".vvd"), levelVolume);
        }
        catch (tgt::Exception& e) {
            LERROR(e.what());
        }
        delete levelVolume;
    }
}

//-----------------------------------------------------------------------------
// comparison functions

bool compareBuffers(const uint16_t* bufferA, const uint16_t* bufferB, svec3 bufferDim, uint16_t& maxError, size_t& errorCount,
    uint16_t errorThresh = 0)
{
    tgtAssert(bufferA && bufferB, "null pointer passed");

    maxError = 0;
    errorCount = 0;
    for (size_t i=0; i<tgt::hmul(bufferDim); i++) {
        if (bufferA[i] != bufferB[i]) {
            uint16_t diff = std::max(bufferA[i], bufferB[i]) - std::min(bufferA[i], bufferB[i]);
            if (diff > errorThresh) {
                errorCount++;
                maxError = std::max(maxError, diff);
            }
        }
    }

    return errorCount == 0;
}

bool checkOctreeAgainstMipMap(const VolumeOctree& octree, const std::vector<const Volume*>& mipmap, size_t channel = 0) {
    tgtAssert(!mipmap.empty(), "empty mipmap");
    tgtAssert(octree.getDimensions() == mipmap.front()->getDimensions(),
        "volume dimensions of octree and mipmap level 0 do not match");

    const svec3 volumeDim = mipmap.front()->getDimensions();
    const svec3 brickDim = octree.getBrickDim();

    bool success = true;
    for (size_t level=0; level<std::min(octree.getNumLevels(), mipmap.size()); level++) {
        svec3 octreeLevelTexDimExpected = volumeDim / tgt::svec3(1<<level);
        VolumeRAM* octreeLevelVolume = octree.createVolume(level, channel);
        if (!octreeLevelVolume) {
            success = false;
            LERROR("Octree level " << level << " volume is null");
            continue;
        }
        if (octreeLevelVolume->getDimensions() != octreeLevelTexDimExpected) {
            success = false;
            LERROR("Octree level " << level << " volume dimensions invalid: " << octreeLevelVolume->getDimensions()
                << " (expected: " << octreeLevelTexDimExpected << ")");
            delete octreeLevelVolume;
            octreeLevelVolume = 0;
            continue;
        }

        const uint16_t* levelVolumeBuffer = getUInt16DataBuffer(octreeLevelVolume);
        const uint16_t* mipMapBuffer = getUInt16DataBuffer(mipmap.at(level));

        uint16_t maxError;
        size_t errorCount;
        bool cmp = compareBuffers(levelVolumeBuffer, mipMapBuffer, octreeLevelVolume->getDimensions(), maxError, errorCount, 1);
        if (!cmp) {
            success = false;
            LERROR("Octree level " << level << " texture does not match reference mip map volume "
                << "(errors: " << errorCount << ", max error: " << maxError << ")");
            delete octreeLevelVolume;
            octreeLevelVolume = 0;
            continue;
        }

        delete octreeLevelVolume;
        octreeLevelVolume = 0;
    }

    return success;
}

bool checkNodeBrickAgainstVolume(const VolumeOctree& octree, const VolumeOctreeNode* node, const svec3& nodeLlf, const svec3& nodeDim,
    const Volume* volume, uint16_t& maxError, size_t& errorCount, uint16_t errorThresh = 0, size_t channel = 0)
{
    tgtAssert(node && volume, "null pointer passed");
    const svec3 volumeDim = volume->getDimensions();
    tgtAssert(tgt::hand(tgt::lessThanEqual(nodeLlf+nodeDim, volumeDim)), "nodeUrb larger than volume dimensions");

    const uint16_t* volumeBuffer = getUInt16DataBuffer(volume);
    tgtAssert(volumeBuffer, "failed to retrieve volume buffer");

    maxError = 0;
    errorCount = 0;
    const uint16_t* brickBuffer = octree.getNodeBrick(node);
    VRN_FOR_EACH_VOXEL(nodeVoxel, svec3::zero, nodeDim) {
        uint16_t nodeVoxelVal = brickBuffer ? brickBuffer[cubicCoordToLinear(nodeVoxel, nodeDim)] : node->getAvgValue();
        svec3 volumeVoxel = nodeVoxel + nodeLlf;
        uint16_t volumeVoxelVal = volumeBuffer[cubicCoordToLinear(volumeVoxel, volumeDim)];
        uint16_t diff = absDiff(nodeVoxelVal, volumeVoxelVal);
        if (diff > errorThresh) {
            errorCount++;
            maxError = std::max(diff, maxError);
        }
    }

    return (errorCount == 0);
}

bool checkNodeBrickAgainstVolume(const VolumeOctree& octree, const VolumeOctreeNode* node, const svec3& nodeLlf, const svec3& nodeDim,
    const Volume* volume, uint16_t errorThresh = 0, size_t channel = 0)
{
    uint16_t maxError;
    size_t errorCount;
    return checkNodeBrickAgainstVolume(octree, node, nodeLlf, nodeDim, volume, maxError, errorCount, errorThresh, channel);
}

bool checkSliceAgainstVolume(const VolumeRAM* slice, const Volume* volume, SliceAlignment alignment, size_t sliceIndex,
    uint16_t errorThresh = 0)
{
    tgtAssert(slice && volume, "null pointer passed");
    const svec3 sliceDim = slice->getDimensions();
    const svec3 volumeDim = volume->getDimensions();

    if (alignment == XY_PLANE)
        BOOST_REQUIRE(sliceDim.xy() == volumeDim.xy());
    else if (alignment == XZ_PLANE)
        BOOST_REQUIRE(tgt::svec2(sliceDim.x, sliceDim.z) == tgt::svec2(volumeDim.x, volumeDim.z));
    else if (alignment == YZ_PLANE)
        BOOST_REQUIRE(tgt::svec2(sliceDim.y, sliceDim.z) == tgt::svec2(volumeDim.y, volumeDim.z));
    tgtAssert(sliceIndex < volumeDim[alignment], "invalid slice index");

    const uint16_t* volumeBuffer = getUInt16DataBuffer(volume);
    tgtAssert(volumeBuffer, "failed to retrieve volume buffer");
    tgtAssert(dynamic_cast<const VolumeRAM_UInt16*>(slice), "slice volume is not uint16_t");
    const uint16_t* sliceBuffer = reinterpret_cast<const uint16_t*>(slice->getData());
    tgtAssert(sliceBuffer, "no slice buffer");

    uint16_t maxError = 0;
    size_t errorCount = 0;
    svec3 sliceOffset(0,0,0);
    sliceOffset[alignment] = sliceIndex;
    VRN_FOR_EACH_VOXEL(sliceVoxel, svec3::zero, sliceDim) {
        svec3 volumeVoxel = sliceVoxel + sliceOffset;
        uint16_t volumeVoxelVal = volumeBuffer[cubicCoordToLinear(volumeVoxel, volumeDim)];
        uint16_t sliceVoxelVal = sliceBuffer[cubicCoordToLinear(sliceVoxel, sliceDim)];
        uint16_t diff = absDiff(sliceVoxelVal, volumeVoxelVal);
        if (diff > errorThresh) {
            errorCount++;
            maxError = std::max(diff, maxError);
        }
    }

    if (errorCount > 0) {
        LERROR("Slice " << sliceIndex << " does not match reference volume "
            << "(errors: " << errorCount << ", max error: " << maxError << ")");
    }

    return (errorCount == 0);
}

void compareTreeNode(const VolumeOctreeNode* nodeA, const VolumeOctreeNode* nodeB,
    const VolumeOctree* octreeA, const VolumeOctree* octreeB)
{
    // nodes null?
    BOOST_REQUIRE((nodeA == 0) == (nodeB == 0));
    if (!nodeA)
        return;

    // check node values
    BOOST_REQUIRE_EQUAL(nodeA->getBrickAddress(), nodeB->getBrickAddress());
    BOOST_REQUIRE_EQUAL(nodeA->getNumChannels(), nodeB->getNumChannels());
    for (size_t i=0; i<nodeA->getNumChannels(); i++) {
        BOOST_REQUIRE_EQUAL(nodeA->getAvgValue(i), nodeB->getAvgValue(i));
        BOOST_REQUIRE_EQUAL(nodeA->getMinValue(i), nodeB->getMinValue(i));
        BOOST_REQUIRE_EQUAL(nodeA->getMaxValue(i), nodeB->getMaxValue(i));
    }

    // check node bricks
    const uint16_t* brickA = octreeA->getNodeBrick(nodeA);
    const uint16_t* brickB = octreeB->getNodeBrick(nodeB);
    BOOST_CHECK((brickA == 0) == (brickB == 0));
    if (brickA && brickB) {
        BOOST_CHECK(memcmp(brickA, brickB, octreeA->getBrickMemorySize()) == 0);
    }

    // check children
    compareTreeNode(nodeA->children_[0], nodeB->children_[0], octreeA, octreeB);
    compareTreeNode(nodeA->children_[1], nodeB->children_[1], octreeA, octreeB);
    compareTreeNode(nodeA->children_[2], nodeB->children_[2], octreeA, octreeB);
    compareTreeNode(nodeA->children_[3], nodeB->children_[3], octreeA, octreeB);
    compareTreeNode(nodeA->children_[4], nodeB->children_[4], octreeA, octreeB);
    compareTreeNode(nodeA->children_[5], nodeB->children_[5], octreeA, octreeB);
    compareTreeNode(nodeA->children_[6], nodeB->children_[6], octreeA, octreeB);
    compareTreeNode(nodeA->children_[7], nodeB->children_[7], octreeA, octreeB);
}

void compareOctrees(const VolumeOctree* octreeA, const VolumeOctree* octreeB) {
    BOOST_CHECK_EQUAL(octreeA->getDimensions(), octreeB->getDimensions());
    BOOST_CHECK_EQUAL(octreeA->getNumChannels(),octreeB->getNumChannels());
    BOOST_CHECK_EQUAL(octreeA->getOctreeDim(),  octreeB->getOctreeDim());
    BOOST_CHECK_EQUAL(octreeA->getBrickDim(),   octreeB->getBrickDim());

    BOOST_CHECK_EQUAL(octreeA->getNumLevels(),  octreeB->getNumLevels());
    BOOST_CHECK_EQUAL(octreeA->getNumNodes(),   octreeB->getNumNodes());
    BOOST_CHECK_EQUAL(octreeA->getBrickPoolMemoryUsed(), octreeB->getBrickPoolMemoryUsed());

    compareTreeNode(octreeA->getRootNode(), octreeB->getRootNode(), octreeA, octreeB);
}

//--------------------------------------------------------------------------------------------

const Volume* noiseVolume128_;

const Volume* brickVolume128_128_;
const Volume* brickVolume128_64_;
const Volume* brickVolume128_32_;
const Volume* brickVolume128_16_;
const Volume* brickVolume128_8_;
const Volume* brickVolume128_4_;
const Volume* brickVolume128_2_;

const Volume* brickVolume128_inhomogeneous_;

const Volume* brickVolume125_96_115_brick16_;
const Volume* brickVolume125_96_115_inhomogeneous_;
const Volume* brickVolume35_96_115_inhomogeneous_;

std::map<const Volume*, std::vector<const Volume*> > mipMaps_;

// Global setup & tear down
struct GlobalFixture {
    VoreenApplication* app;
    tgt::Stopwatch testTimeWatch;

    GlobalFixture() {
        app = new VoreenApplication("octreetest", "octreetest", "octreetest", 0, 0);
        app->initialize();

        LINFO("Creating test data...");
        tgt::Stopwatch watch; watch.start();

        // create noise volume
        noiseVolume128_ = createNoiseVolume(svec3(128));
        mipMaps_[noiseVolume128_] = createMipMap(noiseVolume128_, 7);

        // create regular bricked volumes
        brickVolume128_128_ = createBrickedVolume(svec3(128), svec3(128));
        mipMaps_[brickVolume128_128_] = createMipMap(brickVolume128_128_, 7);

        brickVolume128_64_ = createBrickedVolume(svec3(128), svec3(64));
        mipMaps_[brickVolume128_64_] = createMipMap(brickVolume128_64_, 7);

        brickVolume128_32_ = createBrickedVolume(svec3(128), svec3(32));
        mipMaps_[brickVolume128_32_] = createMipMap(brickVolume128_32_, 7);

        brickVolume128_16_ = createBrickedVolume(svec3(128), svec3(16));
        mipMaps_[brickVolume128_16_] = createMipMap(brickVolume128_16_, 7);

        brickVolume128_8_ = createBrickedVolume(svec3(128), svec3(8));
        mipMaps_[brickVolume128_8_] = createMipMap(brickVolume128_8_, 7);

        brickVolume128_4_ = createBrickedVolume(svec3(128), svec3(4));
        mipMaps_[brickVolume128_4_] = createMipMap(brickVolume128_4_, 7);

        brickVolume128_2_ = createBrickedVolume(svec3(128), svec3(2));
        mipMaps_[brickVolume128_2_] = createMipMap(brickVolume128_2_, 7);

        // create inhomogeneously bricked volume (i.e., octants have different brick sizes)
        std::vector<const Volume*> octants;
        octants.push_back(createBrickedVolume(svec3(64), svec3(32))); //< [0,0,0]
        octants.push_back(createNoiseVolume(  svec3(64)));            //< [1,0,0]
        octants.push_back(createBrickedVolume(svec3(64), svec3(64))); //< [0,1,0]
        octants.push_back(createBrickedVolume(svec3(64), svec3(32))); //< [1,1,0]
        octants.push_back(createNoiseVolume(  svec3(64)));            //< [0,0,1]
        octants.push_back(createBrickedVolume(svec3(64), svec3(4)));  //< [1,0,1]
        octants.push_back(createBrickedVolume(svec3(64), svec3(16))); //< [0,1,1]
        octants.push_back(createNoiseVolume(  svec3(64)));            //< [1,1,1]
        brickVolume128_inhomogeneous_ = mergeVolumes(octants);
        mipMaps_[brickVolume128_inhomogeneous_] = createMipMap(brickVolume128_inhomogeneous_, 7);
        // delete octants
        for (size_t i=0; i<octants.size(); i++)
            delete octants.at(i);

        // create NPOT volume from bricked/inhomogeneous volumes by cropping the lower-left-front part
        VolumeOperatorSubsetGeneric<uint16_t> opSubset;
        brickVolume125_96_115_brick16_ = opSubset.apply(static_cast<const VolumeBase*>(brickVolume128_16_),
            tgt::svec3::zero, tgt::svec3(125, 96, 115));
        tgtAssert(brickVolume125_96_115_brick16_ && brickVolume125_96_115_brick16_->getDimensions() == tgt::svec3(125,96,115),
            "invalid npot volume");
        mipMaps_[brickVolume125_96_115_brick16_] = createMipMap(brickVolume125_96_115_brick16_, 6);

        brickVolume125_96_115_inhomogeneous_ = opSubset.apply(static_cast<const VolumeBase*>(brickVolume128_inhomogeneous_),
            tgt::svec3::zero, tgt::svec3(125, 96, 115));
        tgtAssert(brickVolume125_96_115_inhomogeneous_ && brickVolume125_96_115_inhomogeneous_->getDimensions() == tgt::svec3(125,96,115),
            "invalid npot volume");
        mipMaps_[brickVolume125_96_115_inhomogeneous_] = createMipMap(brickVolume125_96_115_inhomogeneous_, 6);

        brickVolume35_96_115_inhomogeneous_ = opSubset.apply(static_cast<const VolumeBase*>(brickVolume128_inhomogeneous_),
            tgt::svec3::zero, tgt::svec3(35, 96, 115));
        tgtAssert(brickVolume35_96_115_inhomogeneous_ && brickVolume35_96_115_inhomogeneous_->getDimensions() == tgt::svec3(35,96,115),
            "invalid npot volume");
        mipMaps_[brickVolume35_96_115_inhomogeneous_] = createMipMap(brickVolume35_96_115_inhomogeneous_, 5);

        watch.stop();
        LINFO("Creation time: " << watch.getRuntime() << " ms");

         /*VvdVolumeWriter writer;
        for (size_t i=0; i<mipMaps_[brickVolume125_96_115_inhomogeneous_].size(); i++) {
            writer.write(VoreenApplication::app()->getBasePath("data/brickVolume125_96_115_inhomogeneous_level" + itos(i) + ".vvd"),
                mipMaps_[brickVolume125_96_115_inhomogeneous_].at(i));
        } */

        /*try{
            VvdVolumeWriter writer;
            writer.write(VoreenApplication::app()->getBasePath("data/bricked128_inhomogeneous.vvd"), brickVolume128_inhomogeneous_);
            writer.write(VoreenApplication::app()->getBasePath("data/noise128.vvd"),       noiseVolume128_);
            writer.write(VoreenApplication::app()->getBasePath("data/bricked128_128.vvd"), brickVolume128_128_);
            writer.write(VoreenApplication::app()->getBasePath("data/bricked128_64.vvd"),  brickVolume128_64_);
            writer.write(VoreenApplication::app()->getBasePath("data/bricked128_32.vvd"),  brickVolume128_32_);
            writer.write(VoreenApplication::app()->getBasePath("data/bricked128_16.vvd"),  brickVolume128_16_);
            writer.write(VoreenApplication::app()->getBasePath("data/bricked128_8.vvd"),   brickVolume128_8_);
            writer.write(VoreenApplication::app()->getBasePath("data/bricked128_4.vvd"),   brickVolume128_4_);
            writer.write(VoreenApplication::app()->getBasePath("data/bricked128_2.vvd"),   brickVolume128_2_);
        }
        catch(tgt::Exception& ex) {
            LERROR(ex.what());
        } */

        testTimeWatch.start();
    }
    ~GlobalFixture() {
        // print total testing time
        LINFO("Total testing time: " << testTimeWatch.getRuntime() << " ms");

        // clear mipmaps
        std::map<const Volume*, std::vector<const Volume*> >::iterator it;
        for (it = mipMaps_.begin(); it!=mipMaps_.end(); ++it) {
            std::vector<const Volume*> mipmap = it->second;
            for (size_t i=0; i<mipmap.size(); i++)
                delete mipmap.at(i);
        }
        mipMaps_.clear();

        app->deinitialize();
        delete app;
    }
};


BOOST_GLOBAL_FIXTURE(GlobalFixture);

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Octree_StaticFunctions);

BOOST_AUTO_TEST_CASE(Octree_NodeCount) {
    BOOST_CHECK_EQUAL(VolumeOctreeBase::getCompleteTreeNodeCount(1), getNodeCount(1));
    BOOST_CHECK_EQUAL(VolumeOctreeBase::getCompleteTreeNodeCount(2), getNodeCount(2));
    BOOST_CHECK_EQUAL(VolumeOctreeBase::getCompleteTreeNodeCount(3), getNodeCount(3));
    BOOST_CHECK_EQUAL(VolumeOctreeBase::getCompleteTreeNodeCount(4), getNodeCount(4));
    BOOST_CHECK_EQUAL(VolumeOctreeBase::getCompleteTreeNodeCount(5), getNodeCount(5));
    BOOST_CHECK_EQUAL(VolumeOctreeBase::getCompleteTreeNodeCount(6), getNodeCount(6));
    BOOST_CHECK_EQUAL(VolumeOctreeBase::getCompleteTreeNodeCount(7), getNodeCount(7));
    BOOST_CHECK_EQUAL(VolumeOctreeBase::getCompleteTreeNodeCount(8), getNodeCount(8));
    BOOST_CHECK_EQUAL(VolumeOctreeBase::getCompleteTreeNodeCount(9), getNodeCount(9));
    BOOST_CHECK_EQUAL(VolumeOctreeBase::getCompleteTreeNodeCount(10), getNodeCount(10));
}

BOOST_AUTO_TEST_SUITE_END()

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(OctreeConstruction_InvalidParameters);

BOOST_AUTO_TEST_CASE(OctreeConstruction_InvalidBrickSize) {
    // brick size non power-of-two
    BOOST_CHECK_THROW(VolumeOctree octree(noiseVolume128_, 0), std::invalid_argument);
    BOOST_CHECK_THROW(VolumeOctree octree(noiseVolume128_, 1), std::invalid_argument);
    BOOST_CHECK_THROW(VolumeOctree octree(noiseVolume128_, 7), std::invalid_argument);
    BOOST_CHECK_THROW(VolumeOctree octree(noiseVolume128_, 10), std::invalid_argument);
    BOOST_CHECK_THROW(VolumeOctree octree(noiseVolume128_, 100), std::invalid_argument);

    // brick size larger than volume dimensions
    BOOST_CHECK_THROW(VolumeOctree octree(noiseVolume128_, 256), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_InvalidBufferSize) {
    // buffer size smaller than brick size
    BOOST_CHECK_THROW(VolumeOctree octree(noiseVolume128_, 32, 0.f, new OctreeBrickPoolManagerRAM((32*32*32*2) / 2)), VoreenException);
}

BOOST_AUTO_TEST_SUITE_END();

//-----------------------------------------------------------------------------
// Construction tests of an complete (unoptimized) octree with single/multiple buffers:
// * volume dimensions: 128^3, varying brick dimensions
// * a noise volume and a bricked volume (contains homogeneous regions) are used
// * octree is checked by comparing its level textures against a reference mip map of the respective input volume
BOOST_AUTO_TEST_SUITE(OctreeConstruction_CompleteTree);

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_SingleBuffer_Dim128_Brick128) {
    // note: the default buffer size of the OctreeBrickPoolManagerRAM (64 MB) is by far larger than the volume mip map (~4.5MB)
    VolumeOctree octreeNoise(noiseVolume128_, 128, -1.f);
    BOOST_CHECK_EQUAL(octreeNoise.getVolumeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNoise.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNoise.getNumChannels(),     1);
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       1);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 1);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(),  getNodeCount(1));
    BOOST_CHECK_EQUAL(octreeNoise.getNumBricks(), getNodeCount(1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    VolumeOctree octreeBricked(brickVolume128_128_, 128, -1.f);
    BOOST_CHECK_EQUAL(octreeBricked.getDimensions(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeBricked.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeBricked.getNumChannels(),     1);
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       1);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 1);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(),  getNodeCount(1));
    BOOST_CHECK_EQUAL(octreeBricked.getNumBricks(), getNodeCount(1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_SingleBuffer_Dim128_Brick64) {
    VolumeOctree octreeNoise(noiseVolume128_, 64, -1.f);
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 2);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(),  getNodeCount(2));
    BOOST_CHECK_EQUAL(octreeNoise.getNumBricks(), getNodeCount(2));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    VolumeOctree octreeBricked(brickVolume128_128_, 64, -1.f);
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 2);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(),  getNodeCount(2));
    BOOST_CHECK_EQUAL(octreeBricked.getNumBricks(), getNodeCount(2));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_SingleBuffer_Dim128_Brick32) {
    VolumeOctree octreeNoise(noiseVolume128_, 32, -1.f);
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       3);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 3);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(),  getNodeCount(3));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    VolumeOctree octreeBricked(brickVolume128_64_, 32, -1.f);
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       3);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 3);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(), getNodeCount(3));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_64_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_SingleBuffer_Dim128_Brick16) {
    VolumeOctree octreeNoise(noiseVolume128_, 16, -1.f);
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 4);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(), getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    VolumeOctree octreeBricked(brickVolume128_32_, 16, -1.f);
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 4);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(),  getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_32_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_SingleBuffer_Dim128_Brick8) {
    VolumeOctree octreeNoise(noiseVolume128_, 8, -1.f);
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       5);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 5);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(), getNodeCount(5));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    VolumeOctree octreeBricked(brickVolume128_16_, 8, -1.f);
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       5);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 5);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(), getNodeCount(5));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_16_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_SingleBuffer_Dim128_Brick4) {
    VolumeOctree octreeNoise(noiseVolume128_, 4, -1.f);
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       6);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 6);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(), getNodeCount(6));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    VolumeOctree octreeBricked(brickVolume128_16_, 4, -1.f);
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       6);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 6);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(), getNodeCount(6));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_16_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_SingleBuffer_Dim128_Brick2) {
    VolumeOctree octreeNoise(noiseVolume128_, 2, -1.f);
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       7);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 7);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(), getNodeCount(7));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    VolumeOctree octreeBricked(brickVolume128_16_, 2, -1.f);
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       7);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 7);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(), getNodeCount(7));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_16_]));
}

// in the following multiple buffer tests 1.0 MB (1<<20 bytes) is used as buffer size
// complete brick buffer requires ~4.5 MB memory

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_MultipleBuffers_Dim128_Brick64) {
    VolumeOctree octree(noiseVolume128_, 64, -1.f, new OctreeBrickPoolManagerRAM(1<<20));
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 2);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(2));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[noiseVolume128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_MultipleBuffers_Dim128_Brick32) {
    VolumeOctree octree(noiseVolume128_, 32, -1.f, new OctreeBrickPoolManagerRAM(1<<20));
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       3);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 3);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(3));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[noiseVolume128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_MultipleBuffers_Dim128_Brick16) {
    VolumeOctree octree(noiseVolume128_, 16, -1.f, new OctreeBrickPoolManagerRAM(1<<20));
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 4);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[noiseVolume128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_MultipleBuffers_Dim128_Brick8) {
    VolumeOctree octree(noiseVolume128_, 8, -1.f, new OctreeBrickPoolManagerRAM(1<<20));
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       5);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 5);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(5));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[noiseVolume128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_MultipleBuffers_Dim128_Brick4) {
    VolumeOctree octree(noiseVolume128_, 4, -1.f, new OctreeBrickPoolManagerRAM(1<<20));
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       6);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 6);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(6));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[noiseVolume128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_MultipleBuffers_Dim128_Brick2) {
    VolumeOctree octree(noiseVolume128_, 2, -1.f, new OctreeBrickPoolManagerRAM(1<<20));
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       7);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 7);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(7));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[noiseVolume128_]));
}

// in the following tests, the buffer size equals the brick (memory) size

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_OneBufferPerBrick_Dim128_Brick64) {
    VolumeOctree octree(noiseVolume128_, 64, -1.f, new OctreeBrickPoolManagerRAM((64*64*64)*2));
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 2);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(2));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[noiseVolume128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_OneBufferPerBrick_Dim128_Brick32) {
    VolumeOctree octree(noiseVolume128_, 32, -1.f, new OctreeBrickPoolManagerRAM((32*32*32)*2));
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       3);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 3);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(3));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[noiseVolume128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_OneBufferPerBrick_Dim128_Brick16) {
    VolumeOctree octree(noiseVolume128_, 16, -1.f, new OctreeBrickPoolManagerRAM((16*16*16)*2));
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 4);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[noiseVolume128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_OneBufferPerBrick_Dim128_Brick8) {
    VolumeOctree octree(noiseVolume128_, 8, -1.f, new OctreeBrickPoolManagerRAM((8*8*8)*2));
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       5);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 5);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(5));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[noiseVolume128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_OneBufferPerBrick_Dim128_Brick4) {
    VolumeOctree octree(noiseVolume128_, 4, -1.f, new OctreeBrickPoolManagerRAM((4*4*4)*2));
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       6);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 6);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(6));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[noiseVolume128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_CompleteTree_OneBufferPerBrick_Dim128_Brick2) {
    VolumeOctree octree(noiseVolume128_, 2, -1.f, new OctreeBrickPoolManagerRAM((2*2*2)*2));
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       7);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 7);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(7));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[noiseVolume128_]));
}

BOOST_AUTO_TEST_SUITE_END() //< OctreeConstruction_CompleteTree

//-----------------------------------------------------------------------------
// Construction tests of an unoptimized octree with single/multiple buffers:
// * volume dimensions: 128^3, varying brick dimensions
// * a noise volume and a bricked volume (contains homogeneous regions) are used
// * octree is checked by comparing its level textures against a reference mip map of the respective input volume
BOOST_AUTO_TEST_SUITE(OctreeConstruction_OptimizedTree);

BOOST_AUTO_TEST_CASE(OctreeConstruction_OptimizedTree_SingleBuffer_Dim128_Brick64) {
    // noise volume => no homogeneous regions => complete tree expected
    VolumeOctree octreeNoise(noiseVolume128_, 64, 0.f);
    BOOST_CHECK_EQUAL(octreeNoise.getVolumeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNoise.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 2);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(), getNodeCount(2));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    // completely homogeneous volume => only root node expected
    VolumeOctree octreeBricked(brickVolume128_128_, 64, 0.f);
    BOOST_CHECK_EQUAL(octreeBricked.getDimensions(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeBricked.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 1);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(),  1);
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_OptimizedTree_SingleBuffer_Dim128_Brick16) {
    // noise volume => no homogeneous regions => complete tree expected
    VolumeOctree octreeNoise(noiseVolume128_, 16, 0.f);
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 4);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(), getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    // homogeneous regions 4 times larger than brick size => tree depth should be reduced by 2
    VolumeOctree octreeBricked(brickVolume128_64_, 16, 0.f);
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 2);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(), getNodeCount(2));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_64_]));

    //saveOctreeMipMap(octreeBricked, "data/export/optimized_128_64_16");
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_OptimizedTree_SingleBuffer_Dim128_Brick2) {
    // noise volume => no homogeneous regions => complete tree expected
    VolumeOctree octreeNoise(noiseVolume128_, 2, 0.f);
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       7);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 7);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(), getNodeCount(7));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    // homogeneous regions 16 times larger than brick size => tree depth should be reduced by 4
    VolumeOctree octreeBricked(brickVolume128_32_, 2, 0.f);
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       7);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 3);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(), getNodeCount(3));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_32_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_OptimizedTree_MultipleBuffers_Dim128_Brick64) {
    // noise volume => no homogeneous regions => complete tree expected
    VolumeOctree octreeNoise(noiseVolume128_, 64, 0.f, new OctreeBrickPoolManagerRAM(1<<19));
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 2);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(), getNodeCount(2));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    // completely homogeneous volume => only root node expected
    VolumeOctree octreeBricked(brickVolume128_128_, 64, 0.f, new OctreeBrickPoolManagerRAM(1<<19));
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 1);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(), 1);
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_128_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_OptimizedTree_MultipleBuffers_Dim128_Brick16) {
    // noise volume => no homogeneous regions => complete tree expected
    VolumeOctree octreeNoise(noiseVolume128_, 16, 0.f, new OctreeBrickPoolManagerRAM(1<<18));
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 4);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(), getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    // homogeneous regions 2 times larger than brick size => tree depth should be reduced by 1
    VolumeOctree octreeBricked(brickVolume128_32_, 16, 0.f, new OctreeBrickPoolManagerRAM(1<<18));
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 3);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(), getNodeCount(3));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_32_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_OptimizedTree_MultipleBuffers_Dim128_Brick2) {
    // noise volume => no homogeneous regions => complete tree expected
    VolumeOctree octreeNoise(noiseVolume128_, 2, 0.f, new OctreeBrickPoolManagerRAM(1<<6));
    BOOST_CHECK_EQUAL(octreeNoise.getNumLevels(),       7);
    BOOST_CHECK_EQUAL(octreeNoise.getActualTreeDepth(), 7);
    BOOST_CHECK_EQUAL(octreeNoise.getNumNodes(), getNodeCount(7));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNoise, mipMaps_[noiseVolume128_]));

    // homogeneous regions 16 times larger than brick size => tree depth should be reduced by 4
    VolumeOctree octreeBricked(brickVolume128_32_, 2, 0.f, new OctreeBrickPoolManagerRAM(1<<6));
    BOOST_CHECK_EQUAL(octreeBricked.getNumLevels(),       7);
    BOOST_CHECK_EQUAL(octreeBricked.getActualTreeDepth(), 3);
    BOOST_CHECK_EQUAL(octreeBricked.getNumNodes(),  getNodeCount(3));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBricked, mipMaps_[brickVolume128_32_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_OptimizedTree_NonZeroHomogeneityThreshold_Dim128) {
    // create 64-bricked volume as basis,
    // then add noise regular noise with noise range 20
    // => relative inhomogeneity: max-min=20/65535 ~= 0.0003
    Volume* brickedNoiseVolume = brickVolume128_64_->clone();
    addRegularNoise(brickedNoiseVolume->getWritableRepresentation<VolumeRAM_UInt16>(), 20);
    std::vector<const Volume*> mipMap = createMipMap(brickedNoiseVolume, 7);

    // set homogeneity threshold slightly *below* relative inhomogeneity => full tree expected
    VolumeOctree fullOctree(brickedNoiseVolume, 32, 0.00025f);
    BOOST_CHECK_EQUAL(fullOctree.getNumLevels(),       3);
    BOOST_CHECK_EQUAL(fullOctree.getActualTreeDepth(), 3);
    BOOST_CHECK_EQUAL(fullOctree.getNumNodes(), getNodeCount(3));
    BOOST_CHECK(checkOctreeAgainstMipMap(fullOctree, mipMap));

    // set homogeneity threshold slightly *above* relative inhomogeneity => incomplete tree expected (2 levels)
    VolumeOctree optimizedOctree(brickedNoiseVolume, 32, 0.00035f);
    BOOST_CHECK_EQUAL(optimizedOctree.getNumLevels(),       3);
    BOOST_CHECK_EQUAL(optimizedOctree.getActualTreeDepth(), 2);
    BOOST_CHECK_EQUAL(optimizedOctree.getNumNodes(), getNodeCount(2));
    BOOST_CHECK(checkOctreeAgainstMipMap(optimizedOctree, mipMaps_[brickVolume128_64_])); //< octree optimization discards noise

    // delete created data
    for (size_t i=0; i<mipMap.size(); i++)
        delete mipMap.at(i);
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_OptimizedTree_Dim128_InhomogeneousBrickSize) {

    // NOTE: see global fixture for structure of the inhomogeneously bricked volume

    // octree with brick size 64^3 => 2 levels, full tree expected
    VolumeOctree octreeBrick64(brickVolume128_inhomogeneous_, 64, 0.f);
    BOOST_CHECK_EQUAL(octreeBrick64.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octreeBrick64.getActualTreeDepth(), 2);
    BOOST_CHECK_EQUAL(octreeBrick64.getNumNodes(), getNodeCount(2));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBrick64, mipMaps_[brickVolume128_inhomogeneous_]));

    // octree with brick size 16^3 => 4 levels expected, 64^3 (1x), 32^3 (2x) octants incomplete
    VolumeOctree octreeBrick16(brickVolume128_inhomogeneous_, 16, 0.f);
    BOOST_CHECK_EQUAL(octreeBrick16.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeBrick16.getActualTreeDepth(), 4);

    // noise, 4^3, 16^3 octants are complete (level-3 octree) => 5*getNodeCount(3) nodes
    // 32^3 octants (2x) represented by two level-2 trees => 2*getNodeCount(2) nodes
    // 64^3 octant represented by a single node
    // + root node
    BOOST_CHECK_EQUAL(octreeBrick16.getNumNodes(), 5*getNodeCount(3) + 2*getNodeCount(2) + 2);
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeBrick16, mipMaps_[brickVolume128_inhomogeneous_]));
}

BOOST_AUTO_TEST_SUITE_END() //< OctreeConstruction_OptimizedTree


//-----------------------------------------------------------------------------
// Construction tests of from a non-power-of-two volume (both complete and optimized)
BOOST_AUTO_TEST_SUITE(OctreeConstruction_NPOT);

// complete tree

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_CompleteTree_Dim125_96_115_Brick128) {
    VolumeOctree octreeNpot(brickVolume125_96_115_brick16_, 128, -1.f);
    BOOST_CHECK_EQUAL(octreeNpot.getVolumeDim(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       1);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 1);
    BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume125_96_115_brick16_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_CompleteTree_Dim125_96_115_Brick16) {
    VolumeOctree octreeNpot(brickVolume125_96_115_brick16_, 16, -1.f);
    BOOST_CHECK_EQUAL(octreeNpot.getVolumeDim(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 4);
    BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume125_96_115_brick16_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_CompleteTree_Dim125_96_115_Brick4) {
    VolumeOctree octreeNpot(brickVolume125_96_115_brick16_, 4, -1.f);
    BOOST_CHECK_EQUAL(octreeNpot.getVolumeDim(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       6);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 6);
    BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(6));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume125_96_115_brick16_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_CompleteTree_Dim125_96_115_Brick128_InhomogeneousBrickSize) {
    VolumeOctree octreeNpot(brickVolume125_96_115_inhomogeneous_, 128, -1.f);
    BOOST_CHECK_EQUAL(octreeNpot.getVolumeDim(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       1);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 1);
    BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume125_96_115_inhomogeneous_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_CompleteTree_Dim125_96_115_Brick16_InhomogeneousBrickSize) {
    VolumeOctree octreeNpot(brickVolume125_96_115_inhomogeneous_, 16, -1.f);
    BOOST_CHECK_EQUAL(octreeNpot.getVolumeDim(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 4);
    BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume125_96_115_inhomogeneous_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_CompleteTree_Dim125_96_115_Brick4_InhomogeneousBrickSize) {
    VolumeOctree octreeNpot(brickVolume125_96_115_inhomogeneous_, 4, -1.f);
    BOOST_CHECK_EQUAL(octreeNpot.getDimensions(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       6);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 6);
    BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(6));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume125_96_115_inhomogeneous_]));
}

// optimized tree

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_OptimizedTree_Dim125_96_115_Brick128) {
    VolumeOctree octreeNpot(brickVolume125_96_115_brick16_, 128, 0.f);
    BOOST_CHECK_EQUAL(octreeNpot.getDimensions(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       1);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 1);
    BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume125_96_115_brick16_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_OptimizedTree_Dim125_96_115_Brick16) {
    VolumeOctree octreeNpot(brickVolume125_96_115_brick16_, 16, 0.f);
    BOOST_CHECK_EQUAL(octreeNpot.getDimensions(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 4);
    //BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume125_96_115_brick16_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_OptimizedTree_Dim125_96_115_Brick4) {
    VolumeOctree octreeNpot(brickVolume125_96_115_brick16_, 4, 0.f);
    BOOST_CHECK_EQUAL(octreeNpot.getDimensions(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       6);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 6);
    //BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume125_96_115_brick16_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_OptimizedTree_Dim125_96_115_Brick128_InhomogeneousBrickSize) {
    VolumeOctree octreeNpot(brickVolume125_96_115_inhomogeneous_, 128, 0.f);
    BOOST_CHECK_EQUAL(octreeNpot.getDimensions(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       1);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 1);
    BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume125_96_115_inhomogeneous_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_OptimizedTree_Dim125_96_115_Brick16_InhomogeneousBrickSize) {
    VolumeOctree octreeNpot(brickVolume125_96_115_inhomogeneous_, 16, 0.f);
    BOOST_CHECK_EQUAL(octreeNpot.getDimensions(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 4);
    //BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume125_96_115_inhomogeneous_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_OptimizedTree_Dim125_96_115_Brick4_InhomogeneousBrickSize) {
    VolumeOctree octreeNpot(brickVolume125_96_115_inhomogeneous_, 4, 0.f);
    BOOST_CHECK_EQUAL(octreeNpot.getDimensions(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       6);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 6);
    //BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(6));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume125_96_115_inhomogeneous_]));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_NPOT_OptimizedTree_Dim35_96_115_Brick16_InhomogeneousBrickSize) {
    VolumeOctree octreeNpot(brickVolume35_96_115_inhomogeneous_, 16, 0.f);
    BOOST_CHECK_EQUAL(octreeNpot.getDimensions(), tgt::svec3(35, 96, 115));
    BOOST_CHECK_EQUAL(octreeNpot.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octreeNpot.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeNpot.getActualTreeDepth(), 4);
    //BOOST_CHECK_EQUAL(octreeNpot.getNumNodes(),  getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octreeNpot, mipMaps_[brickVolume35_96_115_inhomogeneous_]));
}

BOOST_AUTO_TEST_SUITE_END()


//-----------------------------------------------------------------------------
// Tests disk serialization of constructed octrees.
BOOST_AUTO_TEST_SUITE(OctreeSerialization);

BOOST_AUTO_TEST_CASE(OctreeSerialization_CompleteTree_Dim128_Brick16) {
    VolumeOctree octreeBrick16(brickVolume128_inhomogeneous_, 16, -1.f);
    BOOST_CHECK_EQUAL(octreeBrick16.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeBrick16.getActualTreeDepth(), 4);

    // serialize octree
    std::string octreePath = tgt::FileSystem::cleanupPath(VoreenApplication::app()->getTemporaryPath("octree-test"));
    if (!tgt::FileSystem::dirExists(octreePath))
        tgt::FileSystem::createDirectoryRecursive(octreePath);
    else
        tgt::FileSystem::clearDirectory(octreePath);

    XmlSerializer serializer(octreePath);
    serializer.serialize("Octree", &octreeBrick16);
    std::stringstream serializationStream;
    serializer.write(serializationStream);

    // deserialize octree from stream
    VolumeOctree* deserializedOctree = 0;
    XmlDeserializer deserializer(octreePath);
    deserializer.read(serializationStream);
    deserializer.deserialize("Octree", deserializedOctree);

    // check deserialized octree
    BOOST_CHECK(deserializedOctree != 0);
    compareOctrees(&octreeBrick16, deserializedOctree);

    delete deserializedOctree;
}

BOOST_AUTO_TEST_CASE(OctreeSerialization_OptimizedTree_Dim128_Brick16) {
    VolumeOctree octreeBrick16(brickVolume128_inhomogeneous_, 16, 0.f);
    BOOST_CHECK_EQUAL(octreeBrick16.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octreeBrick16.getActualTreeDepth(), 4);

    // serialize octree
    std::string octreePath = tgt::FileSystem::cleanupPath(VoreenApplication::app()->getTemporaryPath("octree-test"));
    if (!tgt::FileSystem::dirExists(octreePath))
        tgt::FileSystem::createDirectoryRecursive(octreePath);
    else
        tgt::FileSystem::clearDirectory(octreePath);

    XmlSerializer serializer(octreePath);
    serializer.serialize("Octree", &octreeBrick16);
    std::stringstream serializationStream;
    serializer.write(serializationStream);

    // deserialize octree from stream
    VolumeOctree* deserializedOctree = 0;
    XmlDeserializer deserializer(octreePath);
    deserializer.read(serializationStream);
    deserializer.deserialize("Octree", deserializedOctree);

    // check deserialized octree
    BOOST_CHECK(deserializedOctree != 0);
    compareOctrees(&octreeBrick16, deserializedOctree);

    delete deserializedOctree;
}

BOOST_AUTO_TEST_SUITE_END(); //< OctreeSerialization

//-----------------------------------------------------------------------------
// Test for node retrieval from the octree
// * used data sets:
//   - the noise volume for the complete tree tests
//   - the inhomogeneously bricked volume for the optimized tree tests
// * nodes at all octree levels of the respective brick size are queried
// * node positions at a regular grid are queried:
//   - test are run separately for grid points located within the interior of nodes,
//     and grid points located at the node borders
BOOST_AUTO_TEST_SUITE(OctreeNodeRetrieval);

BOOST_AUTO_TEST_CASE(OctreeNodeRetrieval_CompleteTree_NodeInterior_Dim128_Brick128) {
    // create octree
    VolumeOctree octree(noiseVolume128_, 128, -1.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       1);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 1);

    // query nodes for level 0: root node expected at all positions
    std::vector<vec3> grid = generateRegularGridPoints(0.5f, true);
    const Volume* levelVolume = noiseVolume128_;
    for (size_t i=0; i<grid.size(); i++) {
        size_t level = 0;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(grid.at(i), level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 0);
        BOOST_CHECK_EQUAL(voxelLLF, svec3::zero);
        BOOST_CHECK_EQUAL(voxelURB, svec3(128));
        BOOST_CHECK_EQUAL(normLLF, vec3(0.f));
        BOOST_CHECK_EQUAL(normURB, vec3(1.f));
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3::zero, svec3(128), levelVolume));
    }
}

BOOST_AUTO_TEST_CASE(OctreeNodeRetrieval_CompleteTree_NodeBorders_Dim128_Brick128) {
    // create octree
    VolumeOctree octree(noiseVolume128_, 128, -1.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       1);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 1);

    // query nodes for level 0: root node expected at all positions
    std::vector<vec3> grid = generateRegularGridPoints(1.f, false); //< one grid point at each corner of the unit cube
    const Volume* levelVolume = noiseVolume128_;
    for (size_t i=0; i<grid.size(); i++) {
        size_t level = 0;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(grid.at(i), level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 0);
        BOOST_CHECK_EQUAL(voxelLLF, svec3::zero);
        BOOST_CHECK_EQUAL(voxelURB, svec3(128));
        BOOST_CHECK_EQUAL(normLLF, vec3(0.f));
        BOOST_CHECK_EQUAL(normURB, vec3(1.f));
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3::zero, svec3(128), levelVolume));
    }
}

BOOST_AUTO_TEST_CASE(OctreeNodeRetrieval_CompleteTree_NodeInterior_Dim128_Brick64) {
    // create octree
    VolumeOctree octree(noiseVolume128_, 64, -1.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 2);

    // grid with two interior grid points per dimension at the deepest tree level
    const std::vector<vec3> grid = generateRegularGridPoints(0.25f, true);

    // query nodes for level 1: root node expected at all positions
    for (size_t i=0; i<grid.size(); i++) {
        size_t level = 1;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(grid.at(i), level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 1);
        BOOST_CHECK_EQUAL(voxelLLF, svec3::zero);
        BOOST_CHECK_EQUAL(voxelURB, svec3(128));
        BOOST_CHECK_EQUAL(normLLF, vec3(0.f));
        BOOST_CHECK_EQUAL(normURB, vec3(1.f));
    }

    // query nodes for level 0: 64^3 nodes expected
    vec3 nodeDimNorm = vec3(0.5f);
    svec3 nodeDimVoxel = svec3(64);
    const Volume* levelVolume = noiseVolume128_; //< bricks have full resolution at this level
    for (size_t i=0; i<grid.size(); i++) {
        const vec3 gridPos = grid.at(i);
        const vec3 nodeID = tgt::floor(gridPos / nodeDimNorm);
        const svec3 snodeID = svec3(nodeID);
        size_t level = 0;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(gridPos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 0);
        BOOST_CHECK_EQUAL(voxelLLF, snodeID*nodeDimVoxel);
        BOOST_CHECK_EQUAL(voxelURB, (snodeID+svec3(1))*nodeDimVoxel);
        BOOST_CHECK_EQUAL(normLLF, nodeID*nodeDimNorm);
        BOOST_CHECK_EQUAL(normURB, (nodeID+vec3(1.f))*nodeDimNorm);
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF, svec3(64), levelVolume));
    }
}

BOOST_AUTO_TEST_CASE(OctreeNodeRetrieval_CompleteTree_NodeBorders_Dim128_Brick64) {
    // create octree
    VolumeOctree octree(noiseVolume128_, 64, -1.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 2);

    // query nodes for level 1: root node expected at all positions
    std::vector<vec3> grid = generateRegularGridPoints(1.f, false); //< one grid point at each corner of the unit cube
    for (size_t i=0; i<grid.size(); i++) {
        size_t level = 1;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(grid.at(i), level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 1);
        BOOST_CHECK_EQUAL(voxelLLF, svec3::zero);
        BOOST_CHECK_EQUAL(voxelURB, svec3(128));
        BOOST_CHECK_EQUAL(normLLF, vec3(0.f));
        BOOST_CHECK_EQUAL(normURB, vec3(1.f));
    }

    // query nodes for level 0: 64^3 nodes expected
    grid = generateRegularGridPoints(0.5f, false); //< one grid point at each corner of each node
    vec3 nodeDimNorm = vec3(0.5f);
    svec3 nodeDimVoxel = svec3(64);
    const Volume* levelVolume = noiseVolume128_; //< bricks have full resolution at this level
    for (size_t i=0; i<grid.size(); i++) {
        const vec3 gridPos = grid.at(i);
        const svec3 gridVoxel = samplePosToVoxel(gridPos, svec3(128));
        size_t level = 0;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(gridPos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 0);
        // multiple nodes possible => verify that gridPos/gridVoxel lies inside returned node + check dimensions
        BOOST_CHECK(inRange(gridVoxel, voxelLLF, voxelURB));
        BOOST_CHECK_EQUAL(voxelURB-voxelLLF, nodeDimVoxel);
        BOOST_CHECK(inRange(gridPos, normLLF, normURB));
        BOOST_CHECK_EQUAL(normURB-normLLF, nodeDimNorm);
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF, svec3(64), levelVolume));
    }
}

BOOST_AUTO_TEST_CASE(OctreeNodeRetrieval_CompleteTree_NodeInterior_Dim128_Brick32) {
    // create octree
    VolumeOctree octree(noiseVolume128_, 32, -1.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       3);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 3);

    // grid with two interior grid points per dimension at the deepest tree level
    const std::vector<vec3> grid = generateRegularGridPoints(0.125f, true);

    // query nodes for level 2: root node expected at all positions
    for (size_t i=0; i<grid.size(); i++) {
        size_t level = 2;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(grid.at(i), level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 2);
        BOOST_CHECK_EQUAL(voxelLLF, svec3::zero);
        BOOST_CHECK_EQUAL(voxelURB, svec3(128));
        BOOST_CHECK_EQUAL(normLLF, vec3(0.f));
        BOOST_CHECK_EQUAL(normURB, vec3(1.f));
    }

    // query nodes for level 1: 64^3 nodes expected
    vec3 nodeDimNorm = vec3(0.5f);
    svec3 nodeDimVoxel = svec3(64);
    const Volume* levelVolume = mipMaps_[noiseVolume128_].at(1); //< bricks have half resolution at this level
    svec3 levelResDivisor = svec3(2);
    for (size_t i=0; i<grid.size(); i++) {
        const vec3 gridPos = grid.at(i);
        const vec3 nodeID = tgt::floor(gridPos / nodeDimNorm);
        const svec3 snodeID = svec3(nodeID);
        size_t level = 1;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(gridPos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 1);
        BOOST_CHECK_EQUAL(voxelLLF, snodeID*nodeDimVoxel);
        BOOST_CHECK_EQUAL(voxelURB, (snodeID+svec3(1))*nodeDimVoxel);
        BOOST_CHECK_EQUAL(normLLF, nodeID*nodeDimNorm);
        BOOST_CHECK_EQUAL(normURB, (nodeID+vec3(1.f))*nodeDimNorm);
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF/levelResDivisor, svec3(32), levelVolume));
    }

    // query nodes for level 0: 32^3 nodes expected
    nodeDimNorm = vec3(0.25f);
    nodeDimVoxel = svec3(32);
    levelVolume = noiseVolume128_; //< bricks have full resolution at this level
    for (size_t i=0; i<grid.size(); i++) {
        const vec3 gridPos = grid.at(i);
        const vec3 nodeID = tgt::floor(gridPos / nodeDimNorm);
        const svec3 snodeID = svec3(nodeID);
        size_t level = 0;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(gridPos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 0);
        BOOST_CHECK_EQUAL(voxelLLF, snodeID*nodeDimVoxel);
        BOOST_CHECK_EQUAL(voxelURB, (snodeID+svec3(1))*nodeDimVoxel);
        BOOST_CHECK_EQUAL(normLLF, nodeID*nodeDimNorm);
        BOOST_CHECK_EQUAL(normURB, (nodeID+vec3(1.f))*nodeDimNorm);
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF, svec3(32), levelVolume));
    }
}

BOOST_AUTO_TEST_CASE(OctreeNodeRetrieval_CompleteTree_NodeBorders_Dim128_Brick32) {
    // create octree
    VolumeOctree octree(noiseVolume128_, 32, -1.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       3);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 3);

    // query nodes for level 2: root node expected at all positions
    std::vector<vec3> grid = generateRegularGridPoints(1.f, false); //< one grid point at each corner of the unit cube
    for (size_t i=0; i<grid.size(); i++) {
        size_t level = 2;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(grid.at(i), level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 2);
        BOOST_CHECK_EQUAL(voxelLLF, svec3::zero);
        BOOST_CHECK_EQUAL(voxelURB, svec3(128));
        BOOST_CHECK_EQUAL(normLLF, vec3(0.f));
        BOOST_CHECK_EQUAL(normURB, vec3(1.f));
    }

    // query nodes for level 1: 64^3 nodes expected
    grid = generateRegularGridPoints(0.5f, false); //< one grid point at each corner of each node
    vec3 nodeDimNorm = vec3(0.5f);
    svec3 nodeDimVoxel = svec3(64);
    const Volume* levelVolume = mipMaps_[noiseVolume128_].at(1); //< bricks have half resolution at this level
    svec3 levelResDivisor = svec3(2);
    for (size_t i=0; i<grid.size(); i++) {
        const vec3 gridPos = grid.at(i);
        const vec3 nodeID = tgt::floor(gridPos / nodeDimNorm);
        const svec3 gridVoxel = samplePosToVoxel(gridPos, svec3(128));
        size_t level = 1;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(gridPos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 1);
        // multiple nodes possible => verify that gridPos/gridVoxel lies inside returned node + check dimensions
        BOOST_CHECK(inRange(gridVoxel, voxelLLF, voxelURB));
        BOOST_CHECK_EQUAL(voxelURB-voxelLLF, nodeDimVoxel);
        BOOST_CHECK(inRange(gridPos, normLLF, normURB));
        BOOST_CHECK_EQUAL(normURB-normLLF, nodeDimNorm);
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF/levelResDivisor, svec3(32), levelVolume));
    }

    // query nodes for level 0: 32^3 nodes expected (full resolution)
    grid = generateRegularGridPoints(0.25f, false); //< one grid point at each corner of each node
    nodeDimNorm = vec3(0.25f);
    nodeDimVoxel = svec3(32);
    levelVolume = noiseVolume128_; //< bricks have full resolution at this level
    for (size_t i=0; i<grid.size(); i++) {
        const vec3 gridPos = grid.at(i);
        const vec3 nodeID = tgt::floor(gridPos / nodeDimNorm);
        const svec3 gridVoxel = samplePosToVoxel(gridPos, svec3(128));
        size_t level = 0;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(gridPos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 0);
        // multiple nodes possible => verify that gridPos/gridVoxel lies inside returned node + check dimensions
        BOOST_CHECK(inRange(gridVoxel, voxelLLF, voxelURB));
        BOOST_CHECK_EQUAL(voxelURB-voxelLLF, nodeDimVoxel);
        BOOST_CHECK(inRange(gridPos, normLLF, normURB));
        BOOST_CHECK_EQUAL(normURB-normLLF, nodeDimNorm);
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF, svec3(32), levelVolume));
    }
}

BOOST_AUTO_TEST_CASE(OctreeNodeRetrieval_CompleteTree_NodeInterior_Dim128_Brick16) {
    // create octree
    VolumeOctree octree(noiseVolume128_, 16, -1.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 4);

    // grid with two interior grid points per dimension at the deepest tree level
    const std::vector<vec3> grid = generateRegularGridPoints(0.0625f, true);

    // query nodes for level 3: root node expected at all positions
    for (size_t i=0; i<grid.size(); i++) {
        size_t level = 3;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(grid.at(i), level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 3);
        BOOST_CHECK_EQUAL(voxelLLF, svec3::zero);
        BOOST_CHECK_EQUAL(voxelURB, svec3(128));
        BOOST_CHECK_EQUAL(normLLF, vec3(0.f));
        BOOST_CHECK_EQUAL(normURB, vec3(1.f));
    }

    // query nodes for level 2: 64^3 nodes expected
    vec3 nodeDimNorm = vec3(0.5f);
    svec3 nodeDimVoxel = svec3(64);
    const Volume* levelVolume = mipMaps_[noiseVolume128_].at(2); //< bricks have 1/4 resolution at this level
    svec3 levelResDivisor = svec3(4);
    for (size_t i=0; i<grid.size(); i++) {
        const vec3 gridPos = grid.at(i);
        const vec3 nodeID = tgt::floor(gridPos / nodeDimNorm);
        const svec3 snodeID = svec3(nodeID);
        size_t level = 2;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(gridPos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 2);
        BOOST_CHECK_EQUAL(voxelLLF, snodeID*nodeDimVoxel);
        BOOST_CHECK_EQUAL(voxelURB, (snodeID+svec3(1))*nodeDimVoxel);
        BOOST_CHECK_EQUAL(normLLF, nodeID*nodeDimNorm);
        BOOST_CHECK_EQUAL(normURB, (nodeID+vec3(1.f))*nodeDimNorm);
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF/levelResDivisor, svec3(16), levelVolume));
    }

    // query nodes for level 1: 32^3 nodes expected
    nodeDimNorm = vec3(0.25f);
    nodeDimVoxel = svec3(32);
    levelVolume = mipMaps_[noiseVolume128_].at(1); //< bricks have half resolution at this level
    levelResDivisor = svec3(2);
    for (size_t i=0; i<grid.size(); i++) {
        const vec3 gridPos = grid.at(i);
        const vec3 nodeID = tgt::floor(gridPos / nodeDimNorm);
        const svec3 snodeID = svec3(nodeID);
        size_t level = 1;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(gridPos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 1);
        BOOST_CHECK_EQUAL(voxelLLF, snodeID*nodeDimVoxel);
        BOOST_CHECK_EQUAL(voxelURB, (snodeID+svec3(1))*nodeDimVoxel);
        BOOST_CHECK_EQUAL(normLLF, nodeID*nodeDimNorm);
        BOOST_CHECK_EQUAL(normURB, (nodeID+vec3(1.f))*nodeDimNorm);
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF/levelResDivisor, svec3(16), levelVolume));
    }

    // query nodes for level 2: 32^3 nodes expected (full resolution)
    nodeDimNorm = vec3(0.125f);
    nodeDimVoxel = svec3(16);
    levelVolume = noiseVolume128_; //< bricks have full resolution at this level
    for (size_t i=0; i<grid.size(); i++) {
        const vec3 gridPos = grid.at(i);
        const vec3 nodeID = tgt::floor(gridPos / nodeDimNorm);
        const svec3 snodeID = svec3(nodeID);
        size_t level = 0;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(gridPos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 0);
        BOOST_CHECK_EQUAL(voxelLLF, snodeID*nodeDimVoxel);
        BOOST_CHECK_EQUAL(voxelURB, (snodeID+svec3(1))*nodeDimVoxel);
        BOOST_CHECK_EQUAL(normLLF, nodeID*nodeDimNorm);
        BOOST_CHECK_EQUAL(normURB, (nodeID+vec3(1.f))*nodeDimNorm);
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF, svec3(16), levelVolume));
    }
}

BOOST_AUTO_TEST_CASE(OctreeNodeRetrieval_CompleteTree_NodeBorders_Dim128_Brick16) {
    // create octree
    VolumeOctree octree(noiseVolume128_, 16, -1.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 4);

    // query nodes for level 3: root node expected at all positions
    std::vector<vec3> grid = generateRegularGridPoints(1.f, false); //< one grid point at each corner of the unit cube
    for (size_t i=0; i<grid.size(); i++) {
        size_t level = 3;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(grid.at(i), level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 3);
        BOOST_CHECK_EQUAL(voxelLLF, svec3::zero);
        BOOST_CHECK_EQUAL(voxelURB, svec3(128));
        BOOST_CHECK_EQUAL(normLLF, vec3(0.f));
        BOOST_CHECK_EQUAL(normURB, vec3(1.f));
    }

    // query nodes for level 2: 64^3 nodes expected
    grid = generateRegularGridPoints(0.5f, false); //< one grid point at each corner of each node
    vec3 nodeDimNorm = vec3(0.5f);
    svec3 nodeDimVoxel = svec3(64);
    const Volume* levelVolume = mipMaps_[noiseVolume128_].at(2); //< bricks have 1/4 resolution at this level
    svec3 levelResDivisor = svec3(4);
    for (size_t i=0; i<grid.size(); i++) {
        const vec3 gridPos = grid.at(i);
        const vec3 nodeID = tgt::floor(gridPos / nodeDimNorm);
        const svec3 gridVoxel = samplePosToVoxel(gridPos, svec3(128));
        size_t level = 2;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(gridPos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 2);
        // multiple nodes possible => verify that gridPos/gridVoxel lies inside returned node + check dimensions
        BOOST_CHECK(inRange(gridVoxel, voxelLLF, voxelURB));
        BOOST_CHECK_EQUAL(voxelURB-voxelLLF, nodeDimVoxel);
        BOOST_CHECK(inRange(gridPos, normLLF, normURB));
        BOOST_CHECK_EQUAL(normURB-normLLF, nodeDimNorm);
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF/levelResDivisor, svec3(16), levelVolume));
    }

    // query nodes for level 1: 32^3 nodes expected
    grid = generateRegularGridPoints(0.25f, false); //< one grid point at each corner of each node
    nodeDimNorm = vec3(0.25f);
    nodeDimVoxel = svec3(32);
    levelVolume = mipMaps_[noiseVolume128_].at(1); //< bricks have half resolution at this level
    levelResDivisor = svec3(2);
    for (size_t i=0; i<grid.size(); i++) {
        const vec3 gridPos = grid.at(i);
        const vec3 nodeID = tgt::floor(gridPos / nodeDimNorm);
        const svec3 gridVoxel = samplePosToVoxel(gridPos, svec3(128));
        size_t level = 1;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(gridPos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 1);
        // multiple nodes possible => verify that gridPos/gridVoxel lies inside returned node + check dimensions
        BOOST_CHECK(inRange(gridVoxel, voxelLLF, voxelURB));
        BOOST_CHECK_EQUAL(voxelURB-voxelLLF, nodeDimVoxel);
        BOOST_CHECK(inRange(gridPos, normLLF, normURB));
        BOOST_CHECK_EQUAL(normURB-normLLF, nodeDimNorm);
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF/levelResDivisor, svec3(16), levelVolume));
    }

    // query nodes for level 0: 16^3 nodes expected (full resolution)
    grid = generateRegularGridPoints(0.125f, false); //< one grid point at each corner of each node
    nodeDimNorm = vec3(0.125f);
    nodeDimVoxel = svec3(16);
    levelVolume = noiseVolume128_; //< bricks have full resolution at this level
    for (size_t i=0; i<grid.size(); i++) {
        const vec3 gridPos = grid.at(i);
        const vec3 nodeID = tgt::floor(gridPos / nodeDimNorm);
        const svec3 gridVoxel = samplePosToVoxel(gridPos, svec3(128));
        size_t level = 0;
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        const VolumeOctreeNode* node = octree.getNode(gridPos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK_EQUAL(level, 0);
        // multiple nodes possible => verify that gridPos/gridVoxel lies inside returned node + check dimensions
        BOOST_CHECK(inRange(gridVoxel, voxelLLF, voxelURB));
        BOOST_CHECK_EQUAL(voxelURB-voxelLLF, nodeDimVoxel);
        BOOST_CHECK(inRange(gridPos, normLLF, normURB));
        BOOST_CHECK_EQUAL(normURB-normLLF, nodeDimNorm);
        BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF, svec3(16), levelVolume));
    }
}

BOOST_AUTO_TEST_CASE(OctreeNodeRetrieval_CompleteTree_RayTraversal_Dim128_Brick8) {
    VolumeOctree octree(noiseVolume128_, 8, -1.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       5);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 5);

    const vec3 rayEntry = vec3(1.f, 0.1f, 0.15f);
    const vec3 rayExit = vec3(0.f, 0.8f, 0.9f);
    const float stepSize = 0.0001f;

    // traverse ray and query nodes at sample positions
    const vec3 rayDir = rayExit-rayEntry;
    const float numSteps = (float)(tgt::ifloor(tgt::length(rayDir) / stepSize));
    const vec3 rayStep = tgt::normalize(rayDir) * stepSize;
    const vec3 nodeDimNorm = vec3(0.0625f);
    const svec3 nodeDimVoxel = svec3(8);
    for (float i=0.f; i<numSteps; i=i+1.f) {
        const vec3 samplePos = rayEntry + i*rayStep;
        const svec3 sampleVoxel = samplePosToVoxel(samplePos, svec3(128));
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        size_t level = 0;
        const VolumeOctreeNode* node = octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB);
        //LINFO("sample pos: " << samplePos << ", voxelLLF=" << normLLF << ", voxelURB=" << normURB);
        BOOST_CHECK(node != 0);
        BOOST_CHECK_EQUAL(level, 0);
        BOOST_CHECK(inRange(sampleVoxel, voxelLLF, voxelURB));
        BOOST_CHECK_EQUAL(voxelURB-voxelLLF, nodeDimVoxel);
        BOOST_CHECK(inRange(samplePos, normLLF, normURB));
        BOOST_CHECK_EQUAL(normURB-normLLF, nodeDimNorm);
        if ((int(i) % 50) == 0) {
            BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF, svec3(8), noiseVolume128_));
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeNodeRetrieval_OptimizedTree_Dim128_Brick64) {

    // NOTE: see global fixture for structure of the inhomogeneously bricked volume

    VolumeOctree octree(brickVolume128_inhomogeneous_, 64, 0.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 2);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(2)); //< full tree expected

    svec3 voxelLLF, voxelURB;
    vec3 normLLF, normURB;

    // level 1
    size_t level = 1;
    const VolumeOctreeNode* node = octree.getNode(vec3(0.5f), level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 1);
    BOOST_CHECK_EQUAL(voxelLLF, svec3::zero);
    BOOST_CHECK_EQUAL(voxelURB, svec3(128));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f));
    BOOST_CHECK_EQUAL(normURB, vec3(1.f));
    const Volume* levelVolume = mipMaps_[brickVolume128_inhomogeneous_].at(1);  //< brick has half resolution at root level
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3::zero, svec3(64), levelVolume));

    // level 0: octant [0, 1, 0] (homogeneous => node without brick expected)
    level = 0;
    node = octree.getNode(vec3(0.25f, 0.75f, 0.25f), level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 0);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 64, 0));
    BOOST_CHECK_EQUAL(voxelURB, svec3(64, 128, 64));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.5f, 0.f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.5f, 1.f, 0.5f));
    levelVolume = brickVolume128_inhomogeneous_;  //< bricks have full resolution at this level
    //BOOST_CHECK(octree.getNodeBrick(node) == 0);
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF, svec3(64), levelVolume));

    // level 0: octant [1, 0, 0] (inhomogeneous => node with brick expected)
    level = 0;
    node = octree.getNode(vec3(0.25f, 0.25f, 0.75f), level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 0);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 0, 64));
    BOOST_CHECK_EQUAL(voxelURB, svec3(64, 64, 128));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.f, 0.5f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.5f, 0.5f, 1.f));
    levelVolume = brickVolume128_inhomogeneous_;  //< bricks have full resolution at this level
    BOOST_CHECK(octree.getNodeBrick(node) != 0);
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF, svec3(64), levelVolume));

}

BOOST_AUTO_TEST_CASE(OctreeNodeRetrieval_OptimizedTree_Dim128_Brick8) {

    // NOTE: see global fixture for structure of the inhomogeneously bricked volume

    VolumeOctree octree(brickVolume128_inhomogeneous_, 8, 0.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       5);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 5);

    svec3 voxelLLF, voxelURB;
    vec3 normLLF, normURB;

    // level 4
    size_t level = 4;
    const VolumeOctreeNode* node = octree.getNode(vec3(0.5f), level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 4);
    BOOST_CHECK_EQUAL(voxelLLF, svec3::zero);
    BOOST_CHECK_EQUAL(voxelURB, svec3(128));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f));
    BOOST_CHECK_EQUAL(normURB, vec3(1.f));
    const Volume* levelVolume = mipMaps_[brickVolume128_inhomogeneous_].at(4);  //< brick has 1/16 resolution at root level
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3::zero, svec3(8), levelVolume));

    //
    // Octant [0, 1, 0]: completely homogeneous => leaf node without brick at level 1 expected
    //
    vec3 samplePos = vec3(0.25f, 0.75f, 0.25f); //< center of [0,1,0] octant
    level = 3;
    node = octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 3);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 64, 0));
    BOOST_CHECK_EQUAL(voxelURB, svec3(64, 128, 64));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.5f, 0.f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.5f, 1.f, 0.5f));
    levelVolume = mipMaps_[brickVolume128_inhomogeneous_].at(3);  //< bricks have 1/8 resolution at this level
    BOOST_CHECK(octree.getNodeBrick(node) == 0);
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3(0, 8, 0), svec3(8), levelVolume));
    // for all lower levels the level 3 node must be returned
    level = 2;
    BOOST_CHECK_EQUAL(octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB), node);
    BOOST_CHECK_EQUAL(level, 3);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 64, 0));
    BOOST_CHECK_EQUAL(voxelURB, svec3(64, 128, 64));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.5f, 0.f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.5f, 1.f, 0.5f));
    level = 1;
    BOOST_CHECK_EQUAL(octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB), node);
    BOOST_CHECK_EQUAL(level, 3);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 64, 0));
    BOOST_CHECK_EQUAL(voxelURB, svec3(64, 128, 64));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.5f, 0.f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.5f, 1.f, 0.5f));
    level = 0;
    BOOST_CHECK_EQUAL(octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB), node);
    BOOST_CHECK_EQUAL(level, 3);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 64, 0));
    BOOST_CHECK_EQUAL(voxelURB, svec3(64, 128, 64));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.5f, 0.f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.5f, 1.f, 0.5f));

    //
    // Octant [0, 0, 0]: 32^3 sub-octants are homogeneous => leaf node without brick at level 2 expected
    //
    samplePos = vec3(0.125f, 0.375f, 0.125f); //< center of node [0,1,0] out of [4,4,4] nodes at level 2
    level = 3;
    node = octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 3);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 0, 0));
    BOOST_CHECK_EQUAL(voxelURB, svec3(64, 64, 64));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.f, 0.f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.5f, 0.5f, 0.5f));
    levelVolume = mipMaps_[brickVolume128_inhomogeneous_].at(3);  //< bricks have 1/8 resolution at this level
    BOOST_CHECK(octree.getNodeBrick(node) != 0);
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3(0, 0, 0), svec3(8), levelVolume));
    // level 2
    level = 2;
    node = octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 2);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 32, 0));
    BOOST_CHECK_EQUAL(voxelURB, svec3(32, 64, 32));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.25f, 0.f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.25f, 0.5f, 0.25f));
    levelVolume = mipMaps_[brickVolume128_inhomogeneous_].at(2);  //< bricks have 1/4 resolution at this level
    BOOST_CHECK(octree.getNodeBrick(node) == 0);
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3(0, 8, 0), svec3(8), levelVolume));
    // for all lower levels the level 2 node must be returned
    level = 1;
    BOOST_CHECK_EQUAL(octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB), node);
    BOOST_CHECK_EQUAL(level, 2);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 32, 0));
    BOOST_CHECK_EQUAL(voxelURB, svec3(32, 64, 32));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.25f, 0.f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.25f, 0.5f, 0.25f));
    level = 0;
    BOOST_CHECK_EQUAL(octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB), node);
    BOOST_CHECK_EQUAL(level, 2);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 32, 0));
    BOOST_CHECK_EQUAL(voxelURB, svec3(32, 64, 32));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.25f, 0.f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.25f, 0.5f, 0.25f));

    //
    // Octant [0, 1, 1]: 16^3 sub-sub-octants are homogeneous => leaf node without brick at level 3 expected
    //
    samplePos = vec3(0.0625f, 0.5f+0.0625f, 1.f-0.0625f); //< center of node [0,4,7] out of [8,8,8] nodes at level 3
    level = 3;
    node = octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 3);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 64, 64));
    BOOST_CHECK_EQUAL(voxelURB, svec3(64, 128, 128));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.5f, 0.5f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.5f, 1.f, 1.f));
    levelVolume = mipMaps_[brickVolume128_inhomogeneous_].at(3);  //< bricks have 1/8 resolution at this level
    BOOST_CHECK(octree.getNodeBrick(node) != 0);
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3(0, 8, 8), svec3(8), levelVolume));
    // level 2
    level = 2;
    node = octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 2);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 64, 96));
    BOOST_CHECK_EQUAL(voxelURB, svec3(32, 96, 128));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.5f, 0.75f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.25f, 0.75f, 1.f));
    levelVolume = mipMaps_[brickVolume128_inhomogeneous_].at(2);  //< bricks have 1/4 resolution at this level
    BOOST_CHECK(octree.getNodeBrick(node) != 0);
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3(0, 16, 24), svec3(8), levelVolume));
    // level 1
    level = 1;
    node = octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 1);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 64, 112));
    BOOST_CHECK_EQUAL(voxelURB, svec3(16, 80, 128));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.5f, 0.875f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.125f, 0.625f, 1.f));
    levelVolume = mipMaps_[brickVolume128_inhomogeneous_].at(1);  //< bricks have 1/2 resolution at this level
    BOOST_CHECK(octree.getNodeBrick(node) == 0);
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3(0, 32, 56), svec3(8), levelVolume));
    // level 0: equals level 1 node
    level = 0;
    BOOST_CHECK_EQUAL(octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB), node);
    BOOST_CHECK_EQUAL(level, 1);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 64, 112));
    BOOST_CHECK_EQUAL(voxelURB, svec3(16, 80, 128));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.5f, 0.875f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.125f, 0.625f, 1.f));

    //
    // Octant [0, 0, 1]: completely random => leaf node with brick at level 0 expected
    //
    samplePos = vec3(0.03125f, 0.03125f, 1.f-0.03125f); //< center of node [0,0,15] out of [16,16,16] nodes at level
    level = 3;
    node = octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 3);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 0, 64));
    BOOST_CHECK_EQUAL(voxelURB, svec3(64, 64, 128));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.0f, 0.5f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.5f, 0.5f, 1.f));
    levelVolume = mipMaps_[brickVolume128_inhomogeneous_].at(3);  //< bricks have 1/8 resolution at this level
    BOOST_CHECK(octree.getNodeBrick(node) != 0);
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3(0, 0, 8), svec3(8), levelVolume));
    // level 2
    level = 2;
    node = octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 2);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 0, 96));
    BOOST_CHECK_EQUAL(voxelURB, svec3(32, 32, 128));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.f, 0.75f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.25f, 0.25f, 1.f));
    levelVolume = mipMaps_[brickVolume128_inhomogeneous_].at(2);  //< bricks have 1/4 resolution at this level
    BOOST_CHECK(octree.getNodeBrick(node) != 0);
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3(0, 0, 24), svec3(8), levelVolume));
    // level 1
    level = 1;
    node = octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 1);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 0, 112));
    BOOST_CHECK_EQUAL(voxelURB, svec3(16, 16, 128));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.f, 0.875f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.125f, 0.125f, 1.f));
    levelVolume = mipMaps_[brickVolume128_inhomogeneous_].at(1);  //< bricks have 1/2 resolution at this level
    BOOST_CHECK(octree.getNodeBrick(node) != 0);
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3(0, 0, 56), svec3(8), levelVolume));
    // level 0
    level = 0;
    node = octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB);
    BOOST_CHECK_EQUAL(level, 0);
    BOOST_CHECK_EQUAL(voxelLLF, svec3(0, 0, 120));
    BOOST_CHECK_EQUAL(voxelURB, svec3(8, 8, 128));
    BOOST_CHECK_EQUAL(normLLF, vec3(0.f, 0.f, 0.9375f));
    BOOST_CHECK_EQUAL(normURB, vec3(0.0625f, 0.0625f, 1.f));
    levelVolume = brickVolume128_inhomogeneous_;  //< bricks have full resolution at this level
    BOOST_CHECK(octree.getNodeBrick(node) != 0);
    BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, svec3(0, 0, 120), svec3(8), levelVolume));
}

BOOST_AUTO_TEST_CASE(OctreeNodeRetrieval_OptimizedTree_RayTraversal_Dim128_Brick8) {
    VolumeOctree octree(brickVolume128_inhomogeneous_, 8, 0.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       5);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 5); //< noise octants present

    const vec3 rayEntry = vec3(1.f, 0.35f, 0.15f);
    const vec3 rayExit = vec3(0.f, 0.8f, 0.9f);
    const float stepSize = 0.0001f;

    // traverse ray and query nodes at sample positions
    const vec3 rayDir = rayExit-rayEntry;
    const float numSteps = (float)(tgt::ifloor(tgt::length(rayDir) / stepSize));
    const vec3 rayStep = tgt::normalize(rayDir) * stepSize;
    for (float i=0.f; i<numSteps; i=i+1.f) {
        const vec3 samplePos = rayEntry + i*rayStep;
        const svec3 sampleVoxel = samplePosToVoxel(samplePos, svec3(128));
        svec3 voxelLLF, voxelURB;
        vec3 normLLF, normURB;
        size_t level = 0;
        const VolumeOctreeNode* node = octree.getNode(samplePos, level, voxelLLF, voxelURB, normLLF, normURB);
        BOOST_CHECK(node != 0);
        BOOST_CHECK(inRange(sampleVoxel, voxelLLF, voxelURB));
        BOOST_CHECK_EQUAL(voxelURB-voxelLLF, svec3(128>>(4-level)));
        BOOST_CHECK(inRange(samplePos, normLLF, normURB));
        //BOOST_CHECK_EQUAL(normURB-normLLF, nodeDimNorm);
        if ((int(i) % 50) == 0) {
            const Volume* levelVolume = mipMaps_[brickVolume128_inhomogeneous_].at(level);
            BOOST_CHECK(checkNodeBrickAgainstVolume(octree, node, voxelLLF / svec3(1<<(level)), svec3(8), levelVolume));
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

//-----------------------------------------------------------------------------
BOOST_AUTO_TEST_SUITE(OctreeSliceRetrieval);

//--XY--

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_CompleteTree_Dim128_Brick32_XY) {

    VolumeOctree octreeBricked(brickVolume128_32_, 32, -1.f);
    for (size_t level = 0; level < octreeBricked.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[brickVolume128_32_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
            VolumeRAM* slice = octreeBricked.createSlice(XY_PLANE, sliceIndex, level);
            const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, XY_PLANE, levelVolumeSliceIndex));
            // VvdVolumeWriter vvdWriter;
            // vvdWriter.write("d:/voreen-conv/voreen/data/tmp/octree-slice.vvd", new Volume(slice, brickVolume128_32_));
            delete slice;
        }
    }

    VolumeOctree octreeNoise(noiseVolume128_, 32, -1.f);
    for (size_t level = 0; level < octreeNoise.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[noiseVolume128_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
            VolumeRAM* slice = octreeNoise.createSlice(XY_PLANE, sliceIndex, level);
            const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, XY_PLANE, levelVolumeSliceIndex));
            delete slice;
        }
    }

}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_OptimizedTree_Dim128_Brick16_XY) {

    VolumeOctree octreeBricked(brickVolume128_32_, 16, 0.f);
    for (size_t level = 0; level < octreeBricked.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[brickVolume128_32_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
            VolumeRAM* slice = octreeBricked.createSlice(XY_PLANE, sliceIndex, level);
            const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, XY_PLANE, levelVolumeSliceIndex));
            // VvdVolumeWriter vvdWriter;
            // vvdWriter.write("d:/voreen-conv/voreen/data/tmp/octree-slice.vvd", new Volume(slice, brickVolume128_32_));
            delete slice;
        }
    }

}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_NPOT_CompleteTree_Dim125_96_115_Brick32_XY) {
    VolumeOctree octreeNpot(brickVolume125_96_115_brick16_, 32, -1.f);
    for (size_t level = 0; level < octreeNpot.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[brickVolume125_96_115_brick16_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < brickVolume125_96_115_brick16_->getDimensions().z; sliceIndex++) {
            VolumeRAM* slice = octreeNpot.createSlice(XY_PLANE, sliceIndex, level);
            size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            levelVolumeSliceIndex = std::min(levelVolumeSliceIndex, levelVolume->getDimensions().z - 1);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, XY_PLANE, levelVolumeSliceIndex));
            //if (level == 2 && sliceIndex == 65) {
            //    VvdVolumeWriter vvdWriter;
            //    vvdWriter.write("d:/voreen-conv/voreen/data/tmp/octree-slice.vvd", new Volume(slice, brickVolume125_96_115_brick16_));
            //}
            delete slice;
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_NPOT_OptimizedTree_Dim35_96_115_Brick8_XY) {
    VolumeOctree octreeNpot(brickVolume35_96_115_inhomogeneous_, 8, 0.f);
    for (size_t level = 0; level < octreeNpot.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[brickVolume35_96_115_inhomogeneous_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < brickVolume35_96_115_inhomogeneous_->getDimensions().z; sliceIndex++) {
            VolumeRAM* slice = octreeNpot.createSlice(XY_PLANE, sliceIndex, level);
            size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            levelVolumeSliceIndex = std::min(levelVolumeSliceIndex, levelVolume->getDimensions().z - 1);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, XY_PLANE, levelVolumeSliceIndex));
            delete slice;
        }
    }
}

//--XZ--

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_CompleteTree_Dim128_Brick32_XZ) {

    VolumeOctree octreeBricked(brickVolume128_32_, 32, -1.f);
    for (size_t level = 0; level < octreeBricked.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[brickVolume128_32_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
            VolumeRAM* slice = octreeBricked.createSlice(XZ_PLANE, sliceIndex, level);
            const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, XZ_PLANE, levelVolumeSliceIndex));
            // VvdVolumeWriter vvdWriter;
            // vvdWriter.write("d:/voreen-conv/voreen/data/tmp/octree-slice.vvd", new Volume(slice, brickVolume128_32_));
            delete slice;
        }
    }

    VolumeOctree octreeNoise(noiseVolume128_, 32, -1.f);
    for (size_t level = 0; level < octreeNoise.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[noiseVolume128_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
            VolumeRAM* slice = octreeNoise.createSlice(XZ_PLANE, sliceIndex, level);
            const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, XZ_PLANE, levelVolumeSliceIndex));
            delete slice;
        }
    }

}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_OptimizedTree_Dim128_Brick16_XZ) {

    VolumeOctree octreeBricked(brickVolume128_32_, 16, 0.f);
    for (size_t level = 0; level < octreeBricked.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[brickVolume128_32_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
            VolumeRAM* slice = octreeBricked.createSlice(XZ_PLANE, sliceIndex, level);
            const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, XZ_PLANE, levelVolumeSliceIndex));
            // VvdVolumeWriter vvdWriter;
            // vvdWriter.write("d:/voreen-conv/voreen/data/tmp/octree-slice.vvd", new Volume(slice, brickVolume128_32_));
            delete slice;
        }
    }

}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_NPOT_CompleteTree_Dim125_96_115_Brick32_XZ) {
    VolumeOctree octreeNpot(brickVolume125_96_115_brick16_, 32, -1.f);
    for (size_t level = 0; level < octreeNpot.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[brickVolume125_96_115_brick16_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < brickVolume125_96_115_brick16_->getDimensions().y; sliceIndex++) {
            VolumeRAM* slice = octreeNpot.createSlice(XZ_PLANE, sliceIndex, level);
            size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            levelVolumeSliceIndex = std::min(levelVolumeSliceIndex, levelVolume->getDimensions().y - 1);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, XZ_PLANE, levelVolumeSliceIndex));
            delete slice;
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_NPOT_OptimizedTree_Dim35_96_115_Brick8_XZ) {
    VolumeOctree octreeNpot(brickVolume35_96_115_inhomogeneous_, 8, 0.f);
    for (size_t level = 0; level < octreeNpot.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[brickVolume35_96_115_inhomogeneous_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < brickVolume35_96_115_inhomogeneous_->getDimensions().y; sliceIndex++) {
            VolumeRAM* slice = octreeNpot.createSlice(XZ_PLANE, sliceIndex, level);
            size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            levelVolumeSliceIndex = std::min(levelVolumeSliceIndex, levelVolume->getDimensions().y - 1);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, XZ_PLANE, levelVolumeSliceIndex));
            delete slice;
        }
    }
}

//--YZ--

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_CompleteTree_Dim128_Brick32_YZ) {

    VolumeOctree octreeBricked(brickVolume128_32_, 32, -1.f);
    for (size_t level = 0; level < octreeBricked.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[brickVolume128_32_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
            VolumeRAM* slice = octreeBricked.createSlice(YZ_PLANE, sliceIndex, level);
            const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, YZ_PLANE, levelVolumeSliceIndex));
            // VvdVolumeWriter vvdWriter;
            // vvdWriter.write("d:/voreen-conv/voreen/data/tmp/octree-slice.vvd", new Volume(slice, brickVolume128_32_));
            delete slice;
        }
    }

    VolumeOctree octreeNoise(noiseVolume128_, 32, -1.f);
    for (size_t level = 0; level < octreeNoise.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[noiseVolume128_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
            VolumeRAM* slice = octreeNoise.createSlice(YZ_PLANE, sliceIndex, level);
            const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, YZ_PLANE, levelVolumeSliceIndex));
            delete slice;
        }
    }

}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_OptimizedTree_Dim128_Brick16_YZ) {

    VolumeOctree octreeBricked(brickVolume128_32_, 16, 0.f);
    for (size_t level = 0; level < octreeBricked.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[brickVolume128_32_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
            VolumeRAM* slice = octreeBricked.createSlice(YZ_PLANE, sliceIndex, level);
            const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, YZ_PLANE, levelVolumeSliceIndex));
            // VvdVolumeWriter vvdWriter;
            // vvdWriter.write("d:/voreen-conv/voreen/data/tmp/octree-slice.vvd", new Volume(slice, brickVolume128_32_));
            delete slice;
        }
    }

}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_NPOT_CompleteTree_Dim125_96_115_Brick32_YZ) {
    VolumeOctree octreeNpot(brickVolume125_96_115_brick16_, 32, -1.f);
    for (size_t level = 0; level < octreeNpot.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[brickVolume125_96_115_brick16_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < brickVolume125_96_115_brick16_->getDimensions().x; sliceIndex++) {
            VolumeRAM* slice = octreeNpot.createSlice(YZ_PLANE, sliceIndex, level);
            size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            levelVolumeSliceIndex = std::min(levelVolumeSliceIndex, levelVolume->getDimensions().x - 1);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, YZ_PLANE, levelVolumeSliceIndex));
            delete slice;
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_NPOT_OptimizedTree_Dim35_96_115_Brick8_YZ) {
    VolumeOctree octreeNpot(brickVolume35_96_115_inhomogeneous_, 8, 0.f);
    for (size_t level = 0; level < octreeNpot.getNumLevels(); level++) {
        LINFO("Level: " << level);
        const Volume* levelVolume = mipMaps_[brickVolume35_96_115_inhomogeneous_].at(level);

        // iterate over all slices
        for (size_t sliceIndex = 0; sliceIndex < brickVolume35_96_115_inhomogeneous_->getDimensions().x; sliceIndex++) {
            VolumeRAM* slice = octreeNpot.createSlice(YZ_PLANE, sliceIndex, level);
            size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
            levelVolumeSliceIndex = std::min(levelVolumeSliceIndex, levelVolume->getDimensions().x - 1);
            BOOST_CHECK(checkSliceAgainstVolume(slice, levelVolume, YZ_PLANE, levelVolumeSliceIndex));
            delete slice;
        }
    }
}

BOOST_AUTO_TEST_SUITE_END();

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(OctreeConstruction_MultiChannel);

// complete tree/single buffer

BOOST_AUTO_TEST_CASE(OctreeConstruction_TripleChannel_CompleteTree_SingleBuffer_Dim128_Brick128) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(noiseVolume128_);
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);

    // note: the default buffer size of the OctreeBrickPoolManagerRAM (64 MB) is by far larger than the volume mip maps (~13.5MB)
    VolumeOctree octree(testVolumes, 128, -1.f);
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     3);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       1);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 1);
    BOOST_CHECK_EQUAL(octree.getNumNodes(),  getNodeCount(1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_TripleChannel_CompleteTree_SingleBuffer_Dim128_Brick64) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(noiseVolume128_);
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);

    // note: the default buffer size of the OctreeBrickPoolManagerRAM (64 MB) is by far larger than the volume mip maps (~13.5MB)
    VolumeOctree octree(testVolumes, 64, -1.f);
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     3);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 2);
    BOOST_CHECK_EQUAL(octree.getNumNodes(),  getNodeCount(2));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_TripleChannel_CompleteTree_SingleBuffer_Dim128_Brick16) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(noiseVolume128_);
    testVolumes.push_back(brickVolume128_32_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);

    // note: the default buffer size of the OctreeBrickPoolManagerRAM (64 MB) is by far larger than the volume mip maps (~13.5MB)
    VolumeOctree octree(testVolumes, 16, -1.f);
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     3);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 4);
    BOOST_CHECK_EQUAL(octree.getNumNodes(),  getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_TripleChannel_CompleteTree_SingleBuffer_Dim128_Brick2) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(noiseVolume128_);
    testVolumes.push_back(brickVolume128_16_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);

    // note: the default buffer size of the OctreeBrickPoolManagerRAM (64 MB) is by far larger than the volume mip maps (~13.5MB)
    VolumeOctree octree(testVolumes, 2, -1.f);
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     3);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       7);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 7);
    BOOST_CHECK_EQUAL(octree.getNumNodes(),  getNodeCount(7));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
}

// complete tree/multiple buffers

BOOST_AUTO_TEST_CASE(OctreeConstruction_TripleChannel_CompleteTree_MultipleBuffers_Dim128_Brick16) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(noiseVolume128_);
    testVolumes.push_back(brickVolume128_32_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);

    VolumeOctree octree(testVolumes, 16, -1.f, new OctreeBrickPoolManagerRAM(1<<20));
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     3);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 4);
    BOOST_CHECK_EQUAL(octree.getNumNodes(),  getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
}

// optimized tree/multiple buffers

BOOST_AUTO_TEST_CASE(OctreeConstruction_TripleChannel_OptimizedTree_MultipleBuffers_Dim128_Brick64) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_64_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);

    VolumeOctree octree(testVolumes, 64, 0.f, new OctreeBrickPoolManagerRAM(1<<21));
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     3);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       2);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 2);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(2));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_TripleChannel_OptimizedTree_MultipleBuffers_Dim128_Brick16) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_64_);
    testVolumes.push_back(brickVolume128_32_);

    VolumeOctree octree(testVolumes, 16, 0.f, new OctreeBrickPoolManagerRAM(1<<20));
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     3);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 3);
    BOOST_CHECK(octree.getNumNodes() == getNodeCount(3));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_TripleChannel_OptimizedTree_MultipleBuffers_Dim128_Brick2) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_16_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);

    VolumeOctree octree(testVolumes, 2, 0.f, new OctreeBrickPoolManagerRAM(1<<20));
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     3);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       7);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 7);
    BOOST_CHECK(octree.getNumNodes() < getNodeCount(7));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_DualChannel_OptimizedTree_MultipleBuffers_Dim128_Brick2) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_16_);

    VolumeOctree octree(testVolumes, 2, 0.f, new OctreeBrickPoolManagerRAM(1<<20));
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     2);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       7);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 4);
    BOOST_CHECK(octree.getNumNodes() == getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_QuadChannel_OptimizedTree_MultipleBuffers_Dim128_Brick2) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_16_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);
    testVolumes.push_back(brickVolume128_32_);

    VolumeOctree octree(testVolumes, 2, 0.f, new OctreeBrickPoolManagerRAM(1<<20));
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     4);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       7);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 7);
    BOOST_CHECK(octree.getNumNodes() < getNodeCount(7));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(3))], 3));
}

// NPOT

BOOST_AUTO_TEST_CASE(OctreeConstruction_TripleChannel_NPOT_CompleteTree_Dim125_96_115_Brick128) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume125_96_115_brick16_);
    testVolumes.push_back(brickVolume125_96_115_inhomogeneous_);
    testVolumes.push_back(brickVolume125_96_115_brick16_);

    // note: the default buffer size of the OctreeBrickPoolManagerRAM (64 MB) is by far larger than the volume mip maps (~13.5MB)
    VolumeOctree octree(testVolumes, 128, -1.f);
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     3);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       1);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 1);
    BOOST_CHECK_EQUAL(octree.getNumNodes(),  getNodeCount(1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_TripleChannel_NPOT_CompleteTree_Dim125_96_115_Brick16) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume125_96_115_brick16_);
    testVolumes.push_back(brickVolume125_96_115_inhomogeneous_);
    testVolumes.push_back(brickVolume125_96_115_brick16_);

    VolumeOctree octree(testVolumes, 16, -1.f);
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     3);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 4);
    BOOST_CHECK_EQUAL(octree.getNumNodes(), getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_TripleChannel_NPOT_OptimizedTree_Dim125_96_115_Brick128) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume125_96_115_brick16_);
    testVolumes.push_back(brickVolume125_96_115_inhomogeneous_);
    testVolumes.push_back(brickVolume125_96_115_brick16_);

    // note: the default buffer size of the OctreeBrickPoolManagerRAM (64 MB) is by far larger than the volume mip maps (~13.5MB)
    VolumeOctree octree(testVolumes, 128, 0.f);
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     3);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       1);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 1);
    BOOST_CHECK_EQUAL(octree.getNumNodes(),  getNodeCount(1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_TripleChannel_NPOT_OptimizedTree_Dim125_96_115_Brick16) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume125_96_115_brick16_);
    testVolumes.push_back(brickVolume125_96_115_inhomogeneous_);
    testVolumes.push_back(brickVolume125_96_115_brick16_);

    VolumeOctree octree(testVolumes, 16, 0.f);
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     3);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 4);
    BOOST_CHECK(octree.getNumNodes() < getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(2))], 2));
}

BOOST_AUTO_TEST_CASE(OctreeConstruction_DualChannel_NPOT_OptimizedTree_Dim125_96_115_Brick16) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume125_96_115_brick16_);
    testVolumes.push_back(brickVolume125_96_115_inhomogeneous_);

    VolumeOctree octree(testVolumes, 16, 0.f);
    BOOST_CHECK_EQUAL(octree.getVolumeDim(), tgt::svec3(125, 96, 115));
    BOOST_CHECK_EQUAL(octree.getOctreeDim(), tgt::svec3(128));
    BOOST_CHECK_EQUAL(octree.getNumChannels(),     2);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       4);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 4);
    BOOST_CHECK(octree.getNumNodes() < getNodeCount(4));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(0))], 0));
    BOOST_CHECK(checkOctreeAgainstMipMap(octree, mipMaps_[static_cast<const Volume*>(testVolumes.at(1))], 1));
}

// slice retrieval

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_TripleChannel_CompleteTree_Dim128_Brick32_XY) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_32_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);
    VolumeOctree octree(testVolumes, 32, -1.f);

    // iterate over all channels
    for (size_t channel = 0; channel<testVolumes.size(); channel++) {
        LINFO("Channel: " << channel);

        // iterate over all levels
        for (size_t level = 0; level < octree.getNumLevels(); level++) {
            const Volume* levelVolume = mipMaps_[static_cast<const Volume*>(testVolumes.at(channel))].at(level);

            // iterate over all slices
            for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
                VolumeRAM* slice = octree.createSlice(XY_PLANE, sliceIndex, level, channel);
                const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
                BOOST_REQUIRE(checkSliceAgainstVolume(slice, levelVolume, XY_PLANE, levelVolumeSliceIndex));
                delete slice;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_TripleChannel_OptimizedTree_Dim128_Brick32_XY) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_32_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);
    VolumeOctree octree(testVolumes, 32, 0.f);

    // iterate over all channels
    for (size_t channel = 0; channel<testVolumes.size(); channel++) {
        LINFO("Channel: " << channel);

        // iterate over all levels
        for (size_t level = 0; level < octree.getNumLevels(); level++) {
            const Volume* levelVolume = mipMaps_[static_cast<const Volume*>(testVolumes.at(channel))].at(level);

            // iterate over all slices
            for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
                VolumeRAM* slice = octree.createSlice(XY_PLANE, sliceIndex, level, channel);
                const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
                BOOST_REQUIRE(checkSliceAgainstVolume(slice, levelVolume, XY_PLANE, levelVolumeSliceIndex));
                delete slice;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_DualChannel_OptimizedTree_Dim128_Brick32_XY) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_64_);
    VolumeOctree octree(testVolumes, 32, 0.f);

    // iterate over all channels
    for (size_t channel = 0; channel<testVolumes.size(); channel++) {
        LINFO("Channel: " << channel);

        // iterate over all levels
        for (size_t level = 0; level < octree.getNumLevels(); level++) {
            const Volume* levelVolume = mipMaps_[static_cast<const Volume*>(testVolumes.at(channel))].at(level);

            // iterate over all slices
            for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
                VolumeRAM* slice = octree.createSlice(XY_PLANE, sliceIndex, level, channel);
                const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
                BOOST_REQUIRE(checkSliceAgainstVolume(slice, levelVolume, XY_PLANE, levelVolumeSliceIndex));
                delete slice;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_TripleChannel_CompleteTree_Dim128_Brick32_XZ) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_32_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);
    VolumeOctree octree(testVolumes, 32, -1.f);

    // iterate over all channels
    for (size_t channel = 0; channel<testVolumes.size(); channel++) {
        LINFO("Channel: " << channel);

        // iterate over all levels
        for (size_t level = 0; level < octree.getNumLevels(); level++) {
            const Volume* levelVolume = mipMaps_[static_cast<const Volume*>(testVolumes.at(channel))].at(level);

            // iterate over all slices
            for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
                VolumeRAM* slice = octree.createSlice(XZ_PLANE, sliceIndex, level, channel);
                const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
                BOOST_REQUIRE(checkSliceAgainstVolume(slice, levelVolume, XZ_PLANE, levelVolumeSliceIndex));
                delete slice;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_TripleChannel_OptimizedTree_Dim128_Brick32_XZ) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_32_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);
    VolumeOctree octree(testVolumes, 32, 0.f);

    // iterate over all channels
    for (size_t channel = 0; channel<testVolumes.size(); channel++) {
        LINFO("Channel: " << channel);

        // iterate over all levels
        for (size_t level = 0; level < octree.getNumLevels(); level++) {
            const Volume* levelVolume = mipMaps_[static_cast<const Volume*>(testVolumes.at(channel))].at(level);

            // iterate over all slices
            for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
                VolumeRAM* slice = octree.createSlice(XZ_PLANE, sliceIndex, level, channel);
                const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
                BOOST_REQUIRE(checkSliceAgainstVolume(slice, levelVolume, XZ_PLANE, levelVolumeSliceIndex));
                delete slice;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_TripleChannel_CompleteTree_Dim128_Brick32_YZ) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_32_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);
    VolumeOctree octree(testVolumes, 32, -1.f);

    // iterate over all channels
    for (size_t channel = 0; channel<testVolumes.size(); channel++) {
        LINFO("Channel: " << channel);

        // iterate over all levels
        for (size_t level = 0; level < octree.getNumLevels(); level++) {
            const Volume* levelVolume = mipMaps_[static_cast<const Volume*>(testVolumes.at(channel))].at(level);

            // iterate over all slices
            for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
                VolumeRAM* slice = octree.createSlice(YZ_PLANE, sliceIndex, level, channel);
                const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
                BOOST_REQUIRE(checkSliceAgainstVolume(slice, levelVolume, YZ_PLANE, levelVolumeSliceIndex));
                delete slice;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_TripleChannel_OptimizedTree_Dim128_Brick32_YZ) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume128_128_);
    testVolumes.push_back(brickVolume128_32_);
    testVolumes.push_back(brickVolume128_inhomogeneous_);
    VolumeOctree octree(testVolumes, 32, 0.f);

    // iterate over all channels
    for (size_t channel = 0; channel<testVolumes.size(); channel++) {
        LINFO("Channel: " << channel);

        // iterate over all levels
        for (size_t level = 0; level < octree.getNumLevels(); level++) {
            const Volume* levelVolume = mipMaps_[static_cast<const Volume*>(testVolumes.at(channel))].at(level);

            // iterate over all slices
            for (size_t sliceIndex = 0; sliceIndex < 128; sliceIndex++) {
                VolumeRAM* slice = octree.createSlice(YZ_PLANE, sliceIndex, level, channel);
                const size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
                BOOST_REQUIRE(checkSliceAgainstVolume(slice, levelVolume, YZ_PLANE, levelVolumeSliceIndex));
                delete slice;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_TripleChannel_NPOT_OptimizedTree_Dim125_96_115_Brick16_XY) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume125_96_115_brick16_);
    testVolumes.push_back(brickVolume125_96_115_inhomogeneous_);
    testVolumes.push_back(brickVolume125_96_115_brick16_);
    VolumeOctree octree(testVolumes, 16, 0.f);

    // iterate over all channels
    for (size_t channel = 0; channel<testVolumes.size(); channel++) {
        LINFO("Channel: " << channel);

        // iterate over all levels
        for (size_t level = 0; level < octree.getNumLevels(); level++) {
            const Volume* levelVolume = mipMaps_[static_cast<const Volume*>(testVolumes.at(channel))].at(level);

            // iterate over all slices
            for (size_t sliceIndex = 0; sliceIndex < testVolumes.at(channel)->getDimensions().z; sliceIndex++) {
                VolumeRAM* slice = octree.createSlice(XY_PLANE, sliceIndex, level, channel);
                size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
                levelVolumeSliceIndex = std::min(levelVolumeSliceIndex, levelVolume->getDimensions().z - 1);
                BOOST_REQUIRE(checkSliceAgainstVolume(slice, levelVolume, XY_PLANE, levelVolumeSliceIndex));
                delete slice;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_TripleChannel_NPOT_OptimizedTree_Dim125_96_115_Brick16_XZ) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume125_96_115_brick16_);
    testVolumes.push_back(brickVolume125_96_115_inhomogeneous_);
    testVolumes.push_back(brickVolume125_96_115_brick16_);
    VolumeOctree octree(testVolumes, 16, 0.f);

    // iterate over all channels
    for (size_t channel = 0; channel<testVolumes.size(); channel++) {
        LINFO("Channel: " << channel);

        // iterate over all levels
        for (size_t level = 0; level < octree.getNumLevels(); level++) {
            const Volume* levelVolume = mipMaps_[static_cast<const Volume*>(testVolumes.at(channel))].at(level);

            // iterate over all slices
            for (size_t sliceIndex = 0; sliceIndex < testVolumes.at(channel)->getDimensions().y; sliceIndex++) {
                VolumeRAM* slice = octree.createSlice(XZ_PLANE, sliceIndex, level, channel);
                size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
                levelVolumeSliceIndex = std::min(levelVolumeSliceIndex, levelVolume->getDimensions().y - 1);
                BOOST_REQUIRE(checkSliceAgainstVolume(slice, levelVolume, XZ_PLANE, levelVolumeSliceIndex));
                delete slice;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSliceRetrieval_TripleChannel_NPOT_OptimizedTree_Dim125_96_115_Brick16_YZ) {
    std::vector<const VolumeBase*> testVolumes;
    testVolumes.push_back(brickVolume125_96_115_brick16_);
    testVolumes.push_back(brickVolume125_96_115_inhomogeneous_);
    testVolumes.push_back(brickVolume125_96_115_brick16_);
    VolumeOctree octree(testVolumes, 16, 0.f);

    // iterate over all channels
    for (size_t channel = 0; channel<testVolumes.size(); channel++) {
        LINFO("Channel: " << channel);

        // iterate over all levels
        for (size_t level = 0; level < octree.getNumLevels(); level++) {
            const Volume* levelVolume = mipMaps_[static_cast<const Volume*>(testVolumes.at(channel))].at(level);

            // iterate over all slices
            for (size_t sliceIndex = 0; sliceIndex < testVolumes.at(channel)->getDimensions().x; sliceIndex++) {
                VolumeRAM* slice = octree.createSlice(YZ_PLANE, sliceIndex, level, channel);
                size_t levelVolumeSliceIndex = sliceIndex / (1LL << level);
                levelVolumeSliceIndex = std::min(levelVolumeSliceIndex, levelVolume->getDimensions().x - 1);
                BOOST_REQUIRE(checkSliceAgainstVolume(slice, levelVolume, YZ_PLANE, levelVolumeSliceIndex));
                delete slice;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(OctreeSerialization_TripleChannel_OptimizedTree_Dim128_Brick8) {
    std::vector<const VolumeBase*> channelVolumes;
    channelVolumes.push_back(brickVolume128_32_);
    channelVolumes.push_back(brickVolume128_16_);
    channelVolumes.push_back(brickVolume128_64_);

    VolumeOctree octree(channelVolumes, 8, 0.f);
    BOOST_CHECK_EQUAL(octree.getNumLevels(),       5);
    BOOST_CHECK_EQUAL(octree.getActualTreeDepth(), 4);

    // serialize octree
    std::string octreePath = tgt::FileSystem::cleanupPath(VoreenApplication::app()->getTemporaryPath("octree-test"));
    if (!tgt::FileSystem::dirExists(octreePath))
        tgt::FileSystem::createDirectoryRecursive(octreePath);
    else
        tgt::FileSystem::clearDirectory(octreePath);

    XmlSerializer serializer(octreePath);
    serializer.serialize("Octree", &octree);
    std::stringstream serializationStream;
    serializer.write(serializationStream);

    // deserialize octree from stream
    VolumeOctree* deserializedOctree = 0;
    XmlDeserializer deserializer(octreePath);
    deserializer.read(serializationStream);
    deserializer.deserialize("Octree", deserializedOctree);

    // check deserialized octree
    BOOST_CHECK(deserializedOctree != 0);
    compareOctrees(&octree, deserializedOctree);

    delete deserializedOctree;
}

BOOST_AUTO_TEST_SUITE_END()
