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

 // sampler configuration for 2D texture lookup
__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__constant float SAMPLING_BASE_INTERVAL_RCP = 200.f;

// structs
typedef struct {
    float scale_;
    float offset_;
} RealWorldMapping;

typedef struct {
    ulong value_;
    ulong offset_;
    float3 llf_;
    float3 urb_;
} OctreeNode;

typedef struct {
    float4 color;
    float param;
    float firsthit;
    float channelIntensities[4];
} RayInfo;
__constant uint RAYINFO_NUM_ELEMENTS = 10; //< number of single floats stored in an RayInfo object

// octree properties (set by CPU)
__constant uint  OCTREE_DIMENSIONS  = OCTREE_DIMENSIONS_DEF;    //< voxel dimensions of the octree (cubic, power-of-two, >= volume dim)
__constant uint  OCTREE_BRICKDIM    = OCTREE_BRICKDIM_DEF;      //< brick dimensions (cubic, power-of-two)
__constant uint  OCTREE_DEPTH       = OCTREE_DEPTH_DEF;         //< number of levels of the octree
__constant uint  OCTREE_NUMCHANNELS = OCTREE_NUMCHANNELS_DEF;   //< number of channels of the octree [1..4]

//--------------------------------------------------
// 64-bit masks for node buffer entries (set by CPU)

// flag indicating whether a node is homogeneous
__constant ulong MASK_HOMOGENEOUS           = MASK_HOMOGENEOUS_DEF;
__constant uint  MASK_HOMOGENEOUS_SHIFT     = MASK_HOMOGENEOUS_SHIFT_DEF;
__constant uint  MASK_HOMOGENEOUS_NUMBITS   = MASK_HOMOGENEOUS_NUMBITS_DEF;

// flag indicating whether a node's brick is in the GPU buffer
__constant ulong MASK_INBRICKPOOL           = MASK_INBRICKPOOL_DEF;
__constant uint  MASK_INBRICKPOOL_SHIFT     = MASK_INBRICKPOOL_SHIFT_DEF;
__constant uint  MASK_INBRICKPOOL_NUMBITS   = MASK_INBRICKPOOL_NUMBITS_DEF;

// offset of a node's child group in the node buffer
__constant ulong MASK_CHILD                 = MASK_CHILD_DEF;
__constant uint  MASK_CHILD_SHIFT           = MASK_CHILD_SHIFT_DEF;
__constant uint  MASK_CHILD_NUMBITS         = MASK_CHILD_NUMBITS_DEF;

// address of a node's brick in the GPU buffer
__constant ulong MASK_BRICK                 = MASK_BRICK_DEF;
__constant uint  MASK_BRICK_SHIFT           = MASK_BRICK_SHIFT_DEF;
__constant uint  MASK_BRICK_NUMBITS         = MASK_BRICK_NUMBITS_DEF;

// node's avg values for all four channels
__constant ulong MASK_AVG_0                 = MASK_AVG_0_DEF;
__constant uint  MASK_AVG_0_SHIFT           = MASK_AVG_0_SHIFT_DEF;
__constant uint  MASK_AVG_0_NUMBITS         = MASK_AVG_0_NUMBITS_DEF;
__constant ulong MASK_AVG_1                 = MASK_AVG_1_DEF;
__constant uint  MASK_AVG_1_SHIFT           = MASK_AVG_1_SHIFT_DEF;
__constant uint  MASK_AVG_1_NUMBITS         = MASK_AVG_1_NUMBITS_DEF;
__constant ulong MASK_AVG_2                 = MASK_AVG_2_DEF;
__constant uint  MASK_AVG_2_SHIFT           = MASK_AVG_2_SHIFT_DEF;
__constant uint  MASK_AVG_2_NUMBITS         = MASK_AVG_2_NUMBITS_DEF;
__constant ulong MASK_AVG_3                 = MASK_AVG_3_DEF;
__constant uint  MASK_AVG_3_SHIFT           = MASK_AVG_3_SHIFT_DEF;
__constant uint  MASK_AVG_3_NUMBITS         = MASK_AVG_3_NUMBITS_DEF;

// 8-bit masks for node flag buffer entries
__constant uchar MASK_BRICK_INUSE     = MASK_BRICK_INUSE_DEF;
__constant uchar MASK_BRICK_REQUESTED = MASK_BRICK_REQUESTED_DEF;
__constant uchar MASK_NODE_TRAVERSED  = MASK_NODE_TRAVERSED_DEF;  //< (node has been traversed by ray, but brick has not necessarily been used)

// ---------------------
// node access functions

bool isHomogeneous(const ulong node) {
    return (node & MASK_HOMOGENEOUS) > 0;
}

bool hasBrick(const ulong node) {
    return (node & MASK_INBRICKPOOL) > 0;
}

/// Returns the offset of the node's child group in the node buffer, i.e. the offset of its first child.
ulong getNodeChildGroupOffset(const ulong node) {
    return ((node & MASK_CHILD) >> MASK_CHILD_SHIFT);
}

/// Returns true, if the node has no children, i.e. if its child group pointer is 0.
bool isLeafNode(const ulong node) {
    return (getNodeChildGroupOffset(node) == 0);
}

/// Returns the address of the node's brick in the brick buffer.
/// If the node has no brick, its brick offset is undefined.
ulong getNodeBrickAddress(const ulong node) {
    return ((node & MASK_BRICK) >> MASK_BRICK_SHIFT);
}

__global const ushort* getNodeBrick(const ulong node, __global const ushort* brickBuffer) {
    ulong brickOffset = getNodeBrickAddress(node);
    return &brickBuffer[brickOffset * (OCTREE_BRICKDIM*OCTREE_BRICKDIM*OCTREE_BRICKDIM * OCTREE_NUMCHANNELS)];
}

/// Returns the node's avg value for a specific channel, converted to a normalized float.
float getNodeAvgValue0(const ulong node) {
    ulong avgInt = (node & MASK_AVG_0) >> MASK_AVG_0_SHIFT;
    float avgNorm = clamp((float)(avgInt) * 1.f/(float)((1 << MASK_AVG_0_NUMBITS)-1), 0.f, 1.f);
    //float avgNorm = clamp((float)(avgInt) * (1.f/4095.f), 0.f, 1.f); //< for 12 bit precision
    return avgNorm;
}
float getNodeAvgValue1(const ulong node) {
    ulong avgInt = (node & MASK_AVG_1) >> MASK_AVG_1_SHIFT;
    float avgNorm = clamp((float)(avgInt) * 1.f/(float)((1 << MASK_AVG_1_NUMBITS)-1), 0.f, 1.f);
    return avgNorm;
}
float getNodeAvgValue2(const ulong node) {
    ulong avgInt = (node & MASK_AVG_2) >> MASK_AVG_2_SHIFT;
    float avgNorm = clamp((float)(avgInt) * 1.f/(float)((1 << MASK_AVG_2_NUMBITS)-1), 0.f, 1.f);
    return avgNorm;
}
float getNodeAvgValue3(const ulong node) {
    ulong avgInt = (node & MASK_AVG_3) >> MASK_AVG_3_SHIFT;
    float avgNorm = clamp((float)(avgInt) * 1.f/(float)((1 << MASK_AVG_3_NUMBITS)-1), 0.f, 1.f);
    return avgNorm;
}

/// Returns the node's avg values for all channels, converted to a normalized float.
void getNodeAvgValues(const ulong node, float* channelAvgValues) {
    channelAvgValues[0] = getNodeAvgValue0(node);
#if OCTREE_NUMCHANNELS_DEF > 1
    channelAvgValues[1] = getNodeAvgValue1(node);
#endif
#if OCTREE_NUMCHANNELS_DEF > 2
    channelAvgValues[2] = getNodeAvgValue2(node);
#endif
#if OCTREE_NUMCHANNELS_DEF > 3
    channelAvgValues[3] = getNodeAvgValue3(node);
#endif
}

// ----------------------------
// flag buffer access functions

bool isBrickInUse(const uchar flagEntry) {
    return (flagEntry & MASK_BRICK_INUSE) > 0;
}
void setBrickUsed(global uchar* flagEntry, const bool used) {
    *flagEntry = (*flagEntry & ~MASK_BRICK_INUSE) | (used ? MASK_BRICK_INUSE : 0);
}

bool hasNodeBeenTraversed(const uchar flagEntry) {
    return (flagEntry & MASK_NODE_TRAVERSED) > 0;
}
void setNodeTraversed(global uchar* flagEntry, const bool traversed) {
    *flagEntry = (*flagEntry & ~MASK_NODE_TRAVERSED) | (traversed ? MASK_NODE_TRAVERSED : 0);
}

bool isBrickRequested(const uchar flagEntry) {
    return (flagEntry & MASK_BRICK_REQUESTED) > 0;
}
void setBrickRequested(global uchar* flagEntry, const bool requested) {
    *flagEntry = (*flagEntry & ~MASK_BRICK_REQUESTED) | (requested ? MASK_BRICK_REQUESTED : 0);
}

//--------------------------------------
// RayInfo storage

void fetchRayFromBuffer(const global float* buffer, const int2 pos, const uint2 bufferDim, RayInfo* ray) {
    const uint bufferIndex = (pos.y*bufferDim.x + pos.x) * RAYINFO_NUM_ELEMENTS;
    uint offset = 0;

    ray->param    = buffer[bufferIndex + offset++];
    ray->firsthit = buffer[bufferIndex + offset++];

    ray->color.x  = buffer[bufferIndex + offset++];
    ray->color.y  = buffer[bufferIndex + offset++];
    ray->color.z  = buffer[bufferIndex + offset++];
    ray->color.w  = buffer[bufferIndex + offset++];

    ray->channelIntensities[0] = buffer[bufferIndex + offset++];
    ray->channelIntensities[1] = buffer[bufferIndex + offset++];
    ray->channelIntensities[2] = buffer[bufferIndex + offset++];
    ray->channelIntensities[3] = buffer[bufferIndex + offset++];
}

void writeRayToBuffer(global float* buffer, const int2 pos, const uint2 bufferDim, const RayInfo* ray) {
    const uint bufferIndex = (pos.y*bufferDim.x + pos.x) * RAYINFO_NUM_ELEMENTS;
    uint offset = 0;

    buffer[bufferIndex + offset++] = ray->param;
    buffer[bufferIndex + offset++] = ray->firsthit;

    buffer[bufferIndex + offset++] = ray->color.x;
    buffer[bufferIndex + offset++] = ray->color.y;
    buffer[bufferIndex + offset++] = ray->color.z;
    buffer[bufferIndex + offset++] = ray->color.w;

    buffer[bufferIndex + offset++] = ray->channelIntensities[0];
    buffer[bufferIndex + offset++] = ray->channelIntensities[1];
    buffer[bufferIndex + offset++] = ray->channelIntensities[2];
    buffer[bufferIndex + offset++] = ray->channelIntensities[3];
}

//--------------------------------------
// low-level helper functions

uint cubicCoordToLinear(uint3 coord, const uint3 dim) {
    //coord = clamp(coord, (uint3)0, dim-(uint3)(1));
    return coord.z*dim.x*dim.y + coord.y*dim.x + coord.x;
}

uint3 linearCoordToCubic(uint coord, const uint3 dim) {
    uint3 result;
    result.z = coord / (dim.x*dim.y);
    coord = coord % (dim.x*dim.y);
    result.y = coord / dim.x;
    result.x = coord % dim.x;
    return result;
}

float minFloat3(float3 vec) {
    return min(vec.x, min(vec.y, vec.z));
}

float minFloat4(float4 vec) {
    return min(vec.x, min(vec.y, min(vec.z, vec.w)));
}

/// multiplication of a 4x4 row-matrix by a 4-component vector
float4 matVecMult4x4(const float16 mat, const float4 vec) {
    return (float4)(dot(mat.s0123, vec),
                    dot(mat.s4567, vec),
                    dot(mat.s89AB, vec),
                    dot(mat.sCDEF, vec));
}

/// multiplication of a 4x4 row-matrix by a 3-component vector (1.0 is added as w-component)
float3 matVecMult4x3(const float16 mat, const float3 vec) {
    float4 res4 = matVecMult4x4(mat, (float4)(vec, 1.f));
    return (res4 / res4.w).xyz;
}

//-------------------------------------
// brick filtering

float filterBrickNearest(float3 samplePosInBrick, const uint channel, __global const ushort* brick) {
    uint3 sampleCoordsInBrick;
    sampleCoordsInBrick.x = clamp((uint)round(samplePosInBrick.x*(float)(OCTREE_BRICKDIM)-0.5f), (uint)0, (uint)(OCTREE_BRICKDIM-1));
    sampleCoordsInBrick.y = clamp((uint)round(samplePosInBrick.y*(float)(OCTREE_BRICKDIM)-0.5f), (uint)0, (uint)(OCTREE_BRICKDIM-1));
    sampleCoordsInBrick.z = clamp((uint)round(samplePosInBrick.z*(float)(OCTREE_BRICKDIM)-0.5f), (uint)0, (uint)(OCTREE_BRICKDIM-1));

    uint inBrickOffset = cubicCoordToLinear(sampleCoordsInBrick, (uint3)(OCTREE_BRICKDIM))*OCTREE_NUMCHANNELS + channel;
    return brick[inBrickOffset] * (1.f/(float)(1 << 16)); // 65535.f
}

float filterBrickLinear(float3 samplePosInBrick, const uint channel, __global const ushort* brick) {
    const float3 sampleCoordsInBrick = samplePosInBrick * ((float3)(OCTREE_BRICKDIM)-1.f);

    // cubic coordinates of llf/urb corner voxels surrounding the sample pos
    uint3 llf = (uint3)(floor(sampleCoordsInBrick.x),
                        floor(sampleCoordsInBrick.y),
                        floor(sampleCoordsInBrick.z));
    uint3 urb = llf + (uint3)(1);
    llf = clamp(llf, (uint3)(0), (uint3)(OCTREE_BRICKDIM-1));
    urb = min(urb, (uint3)(OCTREE_BRICKDIM-1));

    // linear coodinates of the eight corner voxels
    uint llfOffset = cubicCoordToLinear((uint3)(llf.x, llf.y, llf.z), (uint3)(OCTREE_BRICKDIM))*OCTREE_NUMCHANNELS + channel;
    uint llbOffset = cubicCoordToLinear((uint3)(llf.x, llf.y, urb.z), (uint3)(OCTREE_BRICKDIM))*OCTREE_NUMCHANNELS + channel;
    uint lrfOffset = cubicCoordToLinear((uint3)(llf.x, urb.y, llf.z), (uint3)(OCTREE_BRICKDIM))*OCTREE_NUMCHANNELS + channel;
    uint lrbOffset = cubicCoordToLinear((uint3)(llf.x, urb.y, urb.z), (uint3)(OCTREE_BRICKDIM))*OCTREE_NUMCHANNELS + channel;
    uint ulfOffset = cubicCoordToLinear((uint3)(urb.x, llf.y, llf.z), (uint3)(OCTREE_BRICKDIM))*OCTREE_NUMCHANNELS + channel;
    uint ulbOffset = cubicCoordToLinear((uint3)(urb.x, llf.y, urb.z), (uint3)(OCTREE_BRICKDIM))*OCTREE_NUMCHANNELS + channel;
    uint urfOffset = cubicCoordToLinear((uint3)(urb.x, urb.y, llf.z), (uint3)(OCTREE_BRICKDIM))*OCTREE_NUMCHANNELS + channel;
    uint urbOffset = cubicCoordToLinear((uint3)(urb.x, urb.y, urb.z), (uint3)(OCTREE_BRICKDIM))*OCTREE_NUMCHANNELS + channel;

    // voxel intensities of the eight corner voxels
    float llfVoxel = brick[llfOffset];
    float llbVoxel = brick[llbOffset];
    float lrfVoxel = brick[lrfOffset];
    float lrbVoxel = brick[lrbOffset];
    float ulfVoxel = brick[ulfOffset];
    float ulbVoxel = brick[ulbOffset];
    float urfVoxel = brick[urfOffset];
    float urbVoxel = brick[urbOffset];

    // interpolate between corner voxels
    float3 p = sampleCoordsInBrick - (float3)(llf.x,llf.y,llf.z);
    float sample = llfVoxel * (1.f-p.x)*(1.f-p.y)*(1.f-p.z) +
                   llbVoxel * (1.f-p.x)*(1.f-p.y)*(    p.z) +
                   lrfVoxel * (1.f-p.x)*(    p.y)*(1.f-p.z) +
                   lrbVoxel * (1.f-p.x)*(    p.y)*(    p.z) +
                   ulfVoxel * (    p.x)*(1.f-p.y)*(1.f-p.z) +
                   ulbVoxel * (    p.x)*(1.f-p.y)*(    p.z) +
                   urfVoxel * (    p.x)*(    p.y)*(1.f-p.z) +
                   urbVoxel * (    p.x)*(    p.y)*(    p.z);

    sample *= (1.f/(float)(1 << 16)); // 65535.f;

    return sample;
}

void filterBrick(float3 samplePosInBrick, __global const ushort* const brick, float* channelIntensities) {
#ifdef TEXTURE_FILTER_LINEAR
    channelIntensities[0] = filterBrickLinear(samplePosInBrick, 0, brick);
    #if OCTREE_NUMCHANNELS_DEF > 1
        channelIntensities[1] = filterBrickLinear(samplePosInBrick, 1, brick);
    #endif
    #if OCTREE_NUMCHANNELS_DEF > 2
        channelIntensities[2] = filterBrickLinear(samplePosInBrick, 2, brick);
    #endif
    #if OCTREE_NUMCHANNELS_DEF > 3
        channelIntensities[3] = filterBrickLinear(samplePosInBrick, 3, brick);
    #endif
#else
    channelIntensities[0] = filterBrickNearest(samplePosInBrick, 0, brick);
    #if OCTREE_NUMCHANNELS_DEF > 1
        channelIntensities[1] = filterBrickNearest(samplePosInBrick, 1, brick);
    #endif
    #if OCTREE_NUMCHANNELS_DEF > 2
        channelIntensities[2] = filterBrickNearest(samplePosInBrick, 2, brick);
    #endif
    #if OCTREE_NUMCHANNELS_DEF > 3
        channelIntensities[3] = filterBrickNearest(samplePosInBrick, 3, brick);
    #endif
#endif
}


//-------------------------------------
// transfer function

__constant sampler_t transFuncSampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

float4 applyChannelTransFunc(const float normIntensity, read_only image2d_t transFuncTex,
    const float2 transFuncDomain, const RealWorldMapping realWorldMapping, const uint channel)
{
    // transform intensity value into TF domain
    float realWorldIntensity = (normIntensity * realWorldMapping.scale_) + realWorldMapping.offset_;
    float tfIntensity = (realWorldIntensity - transFuncDomain.x) / (transFuncDomain.y - transFuncDomain.x);
    float yCoord = (float)(channel*3 + 1) / (float)(3*OCTREE_NUMCHANNELS);

    // apply transfer function
    float4 color = read_imagef(transFuncTex, transFuncSampler, (float2)(tfIntensity, yCoord));
    return color;
}

void applyTransFuncs(const float* channelIntensities, read_only image2d_t transFuncTex,
    const float8 transFuncDomains, const RealWorldMapping realWorldMapping, float4* channelColors)
{
    channelColors[0] = applyChannelTransFunc(channelIntensities[0], transFuncTex,
        (float2)(transFuncDomains.s0, transFuncDomains.s1), realWorldMapping, 0);
#if OCTREE_NUMCHANNELS_DEF > 1
    channelColors[1] = applyChannelTransFunc(channelIntensities[1], transFuncTex,
        (float2)(transFuncDomains.s2, transFuncDomains.s3), realWorldMapping, 1);
#endif
#if OCTREE_NUMCHANNELS_DEF > 2
    channelColors[2] = applyChannelTransFunc(channelIntensities[2], transFuncTex,
        (float2)(transFuncDomains.s4, transFuncDomains.s5), realWorldMapping, 2);
#endif
#if OCTREE_NUMCHANNELS_DEF > 3
    channelColors[3] = applyChannelTransFunc(channelIntensities[3], transFuncTex,
        (float2)(transFuncDomains.s6, transFuncDomains.s7), realWorldMapping, 3);
#endif

}

//-------------------------------------

float3 computeNodeExitPoint(const float3 nodeLLF, const float3 nodeURB, const float3 nodeEntry, const float3 rayDir) {
    //tgtAssert(inRange(nodeEntry, nodeLLF, nodeURB), "node entry point outside node");

    float3 exitPlanes = (float3)(rayDir.x >= 0.f ? nodeURB.x : nodeLLF.x,
                                    rayDir.y >= 0.f ? nodeURB.y : nodeLLF.y,
                                    rayDir.z >= 0.f ? nodeURB.z : nodeLLF.z);
    float3 tNodeExit;
    tNodeExit.x = rayDir.x != 0.f ? ((exitPlanes.x - nodeEntry.x) / rayDir.x) : 1e6f;
    tNodeExit.y = rayDir.y != 0.f ? ((exitPlanes.y - nodeEntry.y) / rayDir.y) : 1e6f;
    tNodeExit.z = rayDir.z != 0.f ? ((exitPlanes.z - nodeEntry.z) / rayDir.z) : 1e6f;
    //tgtAssert(tgt::hand(tgt::greaterThanEqual(tNodeExit, vec3::zero)), "at least one negative node exit parameter");

    float tNodeExitMin = minFloat3(tNodeExit);
    //tgtAssert(inRange(tNodeExitMin, 0.f, 1.f), "minimum node exit parameter outside range [0.0;1.0]");

    float3 nodeExit = nodeEntry + (tNodeExitMin - 1e-6f)*rayDir;
    //tgtAssert(inRange(nodeExit, nodeLLF, nodeURB), "node exit point outside node");
    return nodeExit;
}

//-------------------------------------

OctreeNode getNodeAtSamplePos(const float3 samplePos, const uint requestedLevel, __global const ulong* const nodeBuffer,
    uint* returnedLevel, OctreeNode* parentNode, OctreeNode* grandParentNode)
{

    OctreeNode currentNode;
    currentNode.value_ = nodeBuffer[0];
    currentNode.offset_ = 0;
    currentNode.llf_ = (float3)(0.f, 0.f, 0.f);
    currentNode.urb_ = (float3)(1.f, 1.f, 1.f);

    uint currentLevel = 0;

    *parentNode = currentNode;
    *grandParentNode = currentNode;

    // iteratively descent to children
    while (true) {
        const ulong childGroupOffset = getNodeChildGroupOffset(currentNode.value_);
        if (currentLevel == requestedLevel || childGroupOffset == 0) { // current node level requested, or current node is leaf => stop descent
            *returnedLevel = currentLevel;
            return currentNode;
        }
        else { // descent to next level
            *grandParentNode = *parentNode;
            *parentNode = currentNode;

            float3 nodeDim = currentNode.urb_ - currentNode.llf_;
            float3 nodeHalfDim = nodeDim * (float3)(0.5f);
            float3 nodeCenter = currentNode.llf_ + nodeHalfDim;

            // select child node
            uint3 childNodeGroupCoord; //< "coordinates" of the child node within its child group
            childNodeGroupCoord.x = (samplePos.x >= nodeCenter.x) ? 1 : 0;
            childNodeGroupCoord.y = (samplePos.y >= nodeCenter.y) ? 1 : 0;
            childNodeGroupCoord.z = (samplePos.z >= nodeCenter.z) ? 1 : 0;
            currentNode.offset_ = childGroupOffset + cubicCoordToLinear(childNodeGroupCoord, (uint3)(2));
            currentNode.value_ = nodeBuffer[currentNode.offset_];
            currentLevel++;

            // update LLF, URB
            currentNode.llf_.x += nodeHalfDim.x*(float)childNodeGroupCoord.x;
            currentNode.llf_.y += nodeHalfDim.y*(float)childNodeGroupCoord.y;
            currentNode.llf_.z += nodeHalfDim.z*(float)childNodeGroupCoord.z;
            currentNode.urb_ = currentNode.llf_ + nodeHalfDim;
        }
    }

    // should not get here
    OctreeNode dummy;
    return dummy;
}

OctreeNode fetchNextRayNode(const float3 samplePos, const float rayParam, const float3 rayDir, const float samplingStepSize,
    const uint nodeLevel, __global const ulong* const nodeBuffer, __global const ushort* const brickBuffer, __global uchar* const brickFlagBuffer,
    float* exitParam, float* samplingStepSizeNode, bool* nodeHasBrick, __global const ushort** brick)
{

    // retrieve node
    uint origNodeLevel;
    OctreeNode parentNode, grandParentNode;
    OctreeNode origNode = getNodeAtSamplePos(samplePos, nodeLevel, nodeBuffer, &origNodeLevel, &parentNode, &grandParentNode);

    // compute node exit point and ray end parameter for original node
    if (length(rayDir) > 0.f) {
        float3 nodeExit = computeNodeExitPoint(origNode.llf_, origNode.urb_, samplePos, rayDir);
        // determine ray parameter of last sample before node exit point
        float tOffset = minFloat3((nodeExit - samplePos) / rayDir);
        tOffset = floor(tOffset / samplingStepSize) * samplingStepSize;
        *exitParam = rayParam + tOffset;
    }
    else {
        *exitParam = rayParam;
    }

    OctreeNode resultNode = origNode;
    uint resultNodeLevel = origNodeLevel;
    *nodeHasBrick = false;
    *brick = 0;

#ifdef USE_BRICKS
    if (!isHomogeneous(origNode.value_)) { // node not homogeneous => fetch brick
        *nodeHasBrick = hasBrick(origNode.value_);
        if (*nodeHasBrick) { //< brick is in GPU buffer => use it
            *brick = getNodeBrick(origNode.value_, brickBuffer);
        }
        else { //< brick is not in GPU buffer => try ancestor nodes
            setBrickRequested(brickFlagBuffer + origNode.offset_, true);

            // use parent or grandparent node, if they have a brick
            #ifdef USE_ANCESTOR_NODES
            if (hasBrick(parentNode.value_)) {
                resultNode = parentNode;
                resultNodeLevel -= 1;
                *brick = getNodeBrick(parentNode.value_, brickBuffer);
                *nodeHasBrick = true;
            }
            else if (hasBrick(grandParentNode.value_)) {
                if (!isHomogeneous(parentNode.value_))
                    setBrickRequested(brickFlagBuffer + parentNode.offset_, true);
                resultNode = grandParentNode;
                resultNodeLevel -= 2;
                *brick = getNodeBrick(grandParentNode.value_, brickBuffer);
                *nodeHasBrick = true;
            }
            else {
                if (!isHomogeneous(parentNode.value_))
                    setBrickRequested(brickFlagBuffer + parentNode.offset_, true);
                if (!isHomogeneous(grandParentNode.value_))
                    setBrickRequested(brickFlagBuffer + grandParentNode.offset_, true);
            }
            #endif

        }
    }
#endif

#ifdef ADAPTIVE_SAMPLING
    // adapt sampling step size to current node level/resolution
    *samplingStepSizeNode = samplingStepSize * (float)(1<<(OCTREE_DEPTH-1-min(resultNodeLevel, OCTREE_DEPTH-1)));
#else
    *samplingStepSizeNode = samplingStepSize;
#endif

    return resultNode;
}

//-------------------------------------
// ray traversal functions

void traverseRayDVR(const float3 entry, const float3 exit, const uint2 viewportSize,
                      __global const ulong* nodeBuffer,
                      __global const ushort* brickBuffer,
                      __global uchar* brickFlagBuffer,
                      const int nodeLevelOfDetail,

                      const RealWorldMapping realWorldMapping,
                      read_only image2d_t transFunc,
                      const float8 transFuncDomains,

                      const float samplingStepSize,

                      RayInfo* ray)
{

    // calculate ray parameters
    float3 direction = exit - entry;
    float tEnd = length(direction);
    if (tEnd > 0.f)
        direction = normalize(direction);

    // current node information
    OctreeNode currentNode;         ///< current octree node
    float3 currentNodeDimRec;       ///< reciproke dimension of current node in texture coordinates
    bool currentNodeHasBrick = false;               ///< does current node have a brick?
    __global const ushort* currentBrick = 0;        ///< brick of current node (uint16_t)

    // ray casting loop
    float tEndCurrentNode = -1.f;                   ///< end ray parameter for current node, i.e. t value of last sample within the current node
    float samplingStepSizeNode = samplingStepSize;  ///< adapted sampling step size for current node
    while (ray->param <= tEnd) {

        float3 sample = entry + ray->param*direction;
        float currentStepSize = samplingStepSizeNode;

        // retrieve next node, if ray has passed last sample within current node
        if (ray->param > (tEndCurrentNode+1.e-6f)) {

            currentNode = fetchNextRayNode(sample, ray->param, direction, samplingStepSize,
                                    nodeLevelOfDetail, nodeBuffer, brickBuffer, brickFlagBuffer,
                                    &tEndCurrentNode, &samplingStepSizeNode, &currentNodeHasBrick, &currentBrick);

#if defined(DISPLAY_MODE_REFINEMENT)
            if (!currentNodeHasBrick && !isHomogeneous(currentNode.value_)) { //< break ray traversal on missing brick
                return;
            }
            else {
                // traversal flag not used in DVR
                //setNodeTraversed(brickFlagBuffer + currentNode.offset_, true);
            }
#else // full frame mode
            // mark available brick as used (=> should be kept on GPU)
            if (currentNodeHasBrick)
                setBrickUsed(brickFlagBuffer + currentNode.offset_, true);
#endif

            currentNodeDimRec = 1.f / (currentNode.urb_-currentNode.llf_);

        } // node retrieval

        // retrieve sample intensity (for each channel)
        float channelIntensities[OCTREE_NUMCHANNELS_DEF];
        if (currentNodeHasBrick) { // sample brick
            float3 samplePosInBrick = (sample - currentNode.llf_) * currentNodeDimRec;
            filterBrick(samplePosInBrick, currentBrick, channelIntensities);
       }
        else { // node has no brick => use average value for DVR, 0 for MIP, resp.
            getNodeAvgValues(currentNode.value_, channelIntensities);
        }

        // apply channel transfer functions
        float4 channelColors[OCTREE_NUMCHANNELS_DEF];
        applyTransFuncs(channelIntensities, transFunc, transFuncDomains, realWorldMapping, channelColors);

        // average channel colors
        float4 sampleColor;
        #if OCTREE_NUMCHANNELS_DEF == 1
            sampleColor = channelColors[0];
        #else
            // add operator
            sampleColor = (float4)(0.f);
            for (int i=0; i<OCTREE_NUMCHANNELS; i++) {
                if (channelColors[i].w > 0.f) {
                    sampleColor.xyz += channelColors[i].xyz;
                    sampleColor.w = max(channelColors[i].w, sampleColor.w);
                }
            }
            sampleColor.xyz = min(sampleColor.xyz, (float3)(1.f));

            // alpha-blending
            /*sampleColor = (float4)(0.f);
            float alphaSum = 0.f;
            for (int i=0; i<OCTREE_NUMCHANNELS; i++) {
                sampleColor.xyz += channelColors[i].w * channelColors[i].xyz;
                alphaSum += channelColors[i].w;
            }
            sampleColor.xyz /= alphaSum;
            sampleColor.w = min(alphaSum, 1.f); */

        #endif

        // perform compositing
        if (sampleColor.w > 0.f) {
            // apply opacity correction to accomodate for variable sampling intervals
            sampleColor.w = 1.f - pow(1.f - sampleColor.w, currentStepSize * SAMPLING_BASE_INTERVAL_RCP);

            // actual compositing
            ray->color.xyz = ray->color.xyz + (1.f - ray->color.w) * sampleColor.w * sampleColor.xyz;
            ray->color.w   = ray->color.w +   (1.f - ray->color.w) * sampleColor.w;

            // save first-hit point
            ray->firsthit = min(ray->param, ray->firsthit);
        }

        // switch back to base sampling step size,
        // if node step size would yield a sampling point beyond node exit point
        if (ray->param+samplingStepSizeNode > (tEndCurrentNode+1.e-6f))
            samplingStepSizeNode = samplingStepSize;

        // advance along ray
        ray->param += samplingStepSizeNode;

        //finished = finished || (t > tEnd);

        // early ray termination
        if (ray->color.w >= 0.95f) {
            ray->color.w = 1.f;
            ray->param = tEnd + samplingStepSize;
        }

    } // ray-casting loop

    // mark ray as finished
    ray->param = 1.f;

}

void traverseRayMIP(const float3 entry, const float3 exit, const uint2 viewportSize,
                      __global const ulong* nodeBuffer,
                      __global const ushort* brickBuffer,
                      __global uchar* brickFlagBuffer,
                      const int nodeLevelOfDetail,

                      const RealWorldMapping realWorldMapping,
                      read_only image2d_t transFunc,
                      const float8 transFuncDomains,

                      const float samplingStepSize,

                      RayInfo* ray)
{

    // calculate ray parameters
    float3 direction = exit - entry;
    float tEnd = length(direction);
    if (tEnd > 0.f)
        direction = normalize(direction);

    // current node information
    OctreeNode currentNode;         ///< current octree node
    float3 currentNodeDimRec;       ///< reciproke dimension of current node in texture coordinates
    bool currentNodeHasBrick = false;          ///< does current node have a brick?
    __global const ushort* currentBrick = 0;   ///< brick of current node (uint16_t)

    // ray casting loop
    float tEndCurrentNode = -1.f;                   ///< end ray parameter for current node, i.e. t value of last sample within the current node
    float samplingStepSizeNode = samplingStepSize;  ///< adapted sampling step size for current node

#ifdef DISPLAY_MODE_REFINEMENT
    ray->param = 0.f;               ///< traverse entire ray in refinement mode
    bool rayFinished = true;        ///< set to false during traversal, if a missing brick is encountered
    bool skipNode = false;          ///< true, if current node should be skipped (either already traversed or brick is missing)
#endif

    while (ray->param <= tEnd) {

        float3 sample = entry + ray->param*direction;

        // retrieve next node, if ray has passed last sample within current node
        if (ray->param > (tEndCurrentNode+1.e-6f)) {

            currentNode = fetchNextRayNode(sample, ray->param, direction, samplingStepSize,
                                    nodeLevelOfDetail, nodeBuffer, brickBuffer, brickFlagBuffer,
                                    &tEndCurrentNode, &samplingStepSizeNode, &currentNodeHasBrick, &currentBrick);

#if defined(DISPLAY_MODE_REFINEMENT)
            if (!currentNodeHasBrick && !isHomogeneous(currentNode.value_)) { //< skip node, if brick is missing (inhomogeneous node)
                skipNode = true;
                if (hasNodeBeenTraversed(brickFlagBuffer[currentNode.offset_]))
                    setBrickRequested(brickFlagBuffer + currentNode.offset_, false); //< un-request traversed brick
                else
                    rayFinished = false;
            }
            else { //< mark available brick (or homogeneous node) as traversed
                skipNode = false;
                setNodeTraversed(brickFlagBuffer + currentNode.offset_, true);
            }
#else // full frame mode
            // mark available brick as used (=> should be kept on GPU)
            if (currentNodeHasBrick)
                setBrickUsed(brickFlagBuffer + currentNode.offset_, true);
#endif

            currentNodeDimRec = 1.f / (currentNode.urb_-currentNode.llf_);

        } // node retrieval

#ifdef DISPLAY_MODE_REFINEMENT
        if (!skipNode) {
#else
        {
#endif
            // retrieve sample intensity (for each channel)
            float channelIntensities[OCTREE_NUMCHANNELS_DEF];
            if (currentNodeHasBrick) { // sample brick
                float3 samplePosInBrick = (sample - currentNode.llf_) * currentNodeDimRec;
                filterBrick(samplePosInBrick, currentBrick, channelIntensities);
            }
            else { // node has no brick => use average value for DVR, 0 for MIP, resp.
                getNodeAvgValues(currentNode.value_, channelIntensities);
            }

            // update max intensities and depth values for each channel
            if (channelIntensities[0] > ray->channelIntensities[0]) {
                ray->channelIntensities[0] = channelIntensities[0];
                ray->firsthit = min(ray->firsthit, ray->param);
            }
            #if OCTREE_NUMCHANNELS_DEF > 1
            if (channelIntensities[1] > ray->channelIntensities[1]) {
                ray->channelIntensities[1] = channelIntensities[1];
                ray->firsthit = min(ray->firsthit, ray->param);
            }
            #endif
            #if OCTREE_NUMCHANNELS_DEF > 2
            if (channelIntensities[2] > ray->channelIntensities[2]) {
                ray->channelIntensities[2] = channelIntensities[2];
                ray->firsthit = min(ray->firsthit, ray->param);
            }
            #endif
            #if OCTREE_NUMCHANNELS_DEF > 3
            if (channelIntensities[3] > ray->channelIntensities[3]) {
                ray->channelIntensities[3] = channelIntensities[3];
                ray->firsthit = min(ray->firsthit, ray->param);
            }
            #endif

        }

        // switch back to base sampling step size,
        // if node step size would yield a sampling point beyond node exit point
        if (ray->param+samplingStepSizeNode > (tEndCurrentNode+1.e-6f))
            samplingStepSizeNode = samplingStepSize;

        // advance along ray
        ray->param += samplingStepSizeNode;

    } // ray-casting loop


    // retrieve channel color values
    float4 maxIntensityColor[OCTREE_NUMCHANNELS_DEF];
    applyTransFuncs(ray->channelIntensities, transFunc, transFuncDomains, realWorldMapping, maxIntensityColor);

    ray->color = (float4)(0.f);

    // blending mode: add
    for (int i=0; i<OCTREE_NUMCHANNELS; i++) {
        float4 channelColor = maxIntensityColor[i];
        if (channelColor.w > 0.f) {
            ray->color += channelColor;
        }
    }
    ray->color = min(ray->color, (float4)(1.f));

    // blending mode: alpha-blending
    /*float alphaSum = 0.f;
    for (int i=0; i<OCTREE_NUMCHANNELS; i++) {
        float channelDepth = maxIntensityDepth[i];
        float4 channelColor = maxIntensityColor[i];
        if (channelColor.w > 0.f) {
            result.depth = min(channelDepth, result.depth);
            result.color.xyz += channelColor.w * channelColor.xyz;
            alphaSum += channelColor.w;
        }
    }
    result.color.xyz /= alphaSum;
    result.color.w = min(alphaSum, 1.f); */

#ifdef DISPLAY_MODE_REFINEMENT
    ray->param = rayFinished ? 1.f : 0.f;  ///< mark ray as finished/unfinished
#endif

}

void traverseRayMOP(const float3 entry, const float3 exit, const uint2 viewportSize,
                      __global const ulong* nodeBuffer,
                      __global const ushort* brickBuffer,
                      __global uchar* brickFlagBuffer,
                      const int nodeLevelOfDetail,

                      const RealWorldMapping realWorldMapping,
                      read_only image2d_t transFunc,
                      const float8 transFuncDomains,

                      const float samplingStepSize,

                      RayInfo* ray)
{

    // calculate ray parameters
    float3 direction = exit - entry;
    float tEnd = length(direction);
    if (tEnd > 0.f)
        direction = normalize(direction);

    // current node information
    OctreeNode currentNode;         ///< current octree node
    float3 currentNodeDimRec;       ///< reciproke dimension of current node in texture coordinates
    bool currentNodeHasBrick = false;          ///< does current node have a brick?
    __global const ushort* currentBrick = 0;   ///< brick of current node (uint16_t)

    // current ray information
    float4 channelColors[OCTREE_NUMCHANNELS_DEF];
    applyTransFuncs(ray->channelIntensities, transFunc, transFuncDomains, realWorldMapping, channelColors);

    // ray casting loop
    float tEndCurrentNode = -1.f;                   ///< end ray parameter for current node, i.e. t value of last sample within the current node
    float samplingStepSizeNode = samplingStepSize;  ///< adapted sampling step size for current node

#ifdef DISPLAY_MODE_REFINEMENT
    ray->param = 0.f;               ///< traverse entire ray in refinement mode
    bool rayFinished = true;        ///< set to false during traversal on a missing brick
    bool skipNode = false;          ///< true, if current node should be skipped (either already traversed or brick is missing)
#endif

    while (ray->param <= tEnd) {

        float3 sample = entry + ray->param*direction;

        // retrieve next node, if ray has passed last sample within current node
        if (ray->param > (tEndCurrentNode+1.e-6f)) {

            currentNode = fetchNextRayNode(sample, ray->param, direction, samplingStepSize,
                                    nodeLevelOfDetail, nodeBuffer, brickBuffer, brickFlagBuffer,
                                    &tEndCurrentNode, &samplingStepSizeNode, &currentNodeHasBrick, &currentBrick);

#if defined(DISPLAY_MODE_REFINEMENT)
            if (!currentNodeHasBrick && !isHomogeneous(currentNode.value_)) { //< skip node, if brick is missing (inhomogeneous node)
                skipNode = true;
                if (hasNodeBeenTraversed(brickFlagBuffer[currentNode.offset_]))
                    setBrickRequested(brickFlagBuffer + currentNode.offset_, false); //< un-request traversed brick
                else
                    rayFinished = false;
            }
            else { //< mark available brick (or homogeneous node) as traversed
                skipNode = false;
                setNodeTraversed(brickFlagBuffer + currentNode.offset_, true);
            }
#else // full frame mode
            // mark available brick as used (=> should be kept on GPU)
            if (currentNodeHasBrick)
                setBrickUsed(brickFlagBuffer + currentNode.offset_, true);
#endif

            currentNodeDimRec = 1.f / (currentNode.urb_-currentNode.llf_);

        } // node retrieval

#ifdef DISPLAY_MODE_REFINEMENT
        if (!skipNode) {
#else
        {
#endif
            // retrieve sample intensity (for each channel)
            float channelIntensities[OCTREE_NUMCHANNELS_DEF];
            if (currentNodeHasBrick) { // sample brick
                float3 samplePosInBrick = (sample - currentNode.llf_) * currentNodeDimRec;
                filterBrick(samplePosInBrick, currentBrick, channelIntensities);
            }
            else { // node has no brick => use average value for DVR, 0 for MIP, resp.
                getNodeAvgValues(currentNode.value_, channelIntensities);
            }

            // retrieve channel color values
            float4 sampleIntensityColor[OCTREE_NUMCHANNELS_DEF];
            applyTransFuncs(channelIntensities, transFunc, transFuncDomains, realWorldMapping, sampleIntensityColor);

            // update channel color and depth values
            if (sampleIntensityColor[0].w > channelColors[0].w) {
                channelColors[0] = sampleIntensityColor[0];
                ray->channelIntensities[0] = channelIntensities[0];
                ray->firsthit = min(ray->firsthit, ray->param);
            }
            #if OCTREE_NUMCHANNELS_DEF > 1
            if (sampleIntensityColor[1].w > channelColors[1].w) {
                channelColors[1] = sampleIntensityColor[1];
                ray->channelIntensities[1] = channelIntensities[1];
                ray->firsthit = min(ray->firsthit, ray->param);
            }
            #endif
            #if OCTREE_NUMCHANNELS_DEF > 2
            if (sampleIntensityColor[2].w > channelColors[2].w) {
                channelColors[2] = sampleIntensityColor[2];
                ray->channelIntensities[2] = channelIntensities[2];
                ray->firsthit = min(ray->firsthit, ray->param);
            }
            #endif
            #if OCTREE_NUMCHANNELS_DEF > 3
            if (sampleIntensityColor[3].w > channelColors[3].w) {
                channelColors[3] = sampleIntensityColor[3];
                ray->channelIntensities[3] = channelIntensities[3];
                ray->firsthit = min(ray->firsthit, ray->param);
            }
            #endif
        }

        // switch back to base sampling step size,
        // if node step size would yield a sampling point beyond node exit point
        if (ray->param+samplingStepSizeNode > (tEndCurrentNode+1.e-6f))
            samplingStepSizeNode = samplingStepSize;

        // advance along ray
        ray->param += samplingStepSizeNode + 1e-6f;

    } // ray-casting loop

    ray->color = (float4)(0.f);

    // blending mode: add
    for (int i=0; i<OCTREE_NUMCHANNELS; i++) {
        float4 channelColor = channelColors[i];
        if (channelColor.w > 0.f) {
            ray->color += channelColor;
        }
    }
    ray->color = min(ray->color, (float4)(1.f));

#ifdef DISPLAY_MODE_REFINEMENT
    ray->param = rayFinished ? 1.f : 0.f;  ///< mark ray as finished/unfinished
#endif

}

//-------------------------------------------------------------------------------------------
// main kernel

__kernel void render( read_only image2d_t entryTex
                    , read_only image2d_t exitTex
                    , const uint2 viewportSize
                    , const uint coarsenessFactor

                    , const float samplingStepSize
                    , const uint4 volumeDimensions

                    , __global const ulong* nodeBuffer
                    , __global const ushort* brickBuffer
                    , __global uchar* brickFlagBuffer
                    , const uint nodeLevelOfDetail

                    , const RealWorldMapping realWorldMapping
                    , read_only image2d_t transFunc
                    , const float8 transFuncDomains

                    , write_only image2d_t output
                    , write_only image2d_t outputDepth

                    , __global float* rayBuffer
                    , const uint firstRefinementFrame   //< bool flag
                    )
{

    // determine fragment position: each workgroup is assigned to one tile of the image
    const uint2 localSize = (uint2)(get_local_size(0), get_local_size(1));
    const uint2 groupID = (uint2)(get_group_id(0), get_group_id(1));
    const uint2 localID = (uint2)(get_local_id(0), get_local_id(1));
    int2 fragPos = (int2)(groupID.x*localSize.x + localID.x, groupID.y*localSize.y + localID.y);

    if (fragPos.x >= viewportSize.x || fragPos.y >= viewportSize.y)
        return;

    // read entry/exit points
    float4 entry = read_imagef(entryTex, imageSampler, fragPos*(int2)(coarsenessFactor));
    float4 exit = read_imagef(exitTex, imageSampler, fragPos*(int2)(coarsenessFactor));

    // transform entry/exit points from NPOT to POT: (1.0,1.0,1.0) refers to the URB voxel of the volume
    // (which differs from the URB of the octree for NPOT volumes)
    const float4 coordinateCorrection =
        (float4)((float)volumeDimensions.x / (float)OCTREE_DIMENSIONS,
                 (float)volumeDimensions.y / (float)OCTREE_DIMENSIONS,
                 (float)volumeDimensions.z / (float)OCTREE_DIMENSIONS,
                 1.f);
    entry = entry*coordinateCorrection;
    exit = exit*coordinateCorrection;

    // initialize ray
    RayInfo ray;
    ray.param = 0.f;             ///< ray parameter
    ray.color = (float4)(0.f);   ///< resulting color
    ray.firsthit = 1.f;          ///< first hit point
    ray.channelIntensities[0] = 0.f;
    ray.channelIntensities[1] = 0.f;
    ray.channelIntensities[2] = 0.f;
    ray.channelIntensities[3] = 0.f;

#ifdef DISPLAY_MODE_REFINEMENT
    // fetch intermediate ray result from refinement buffer
    fetchRayFromBuffer(rayBuffer, fragPos, viewportSize, &ray);

    if (firstRefinementFrame) {
        ray.param = 0.f;
        #ifdef COMPOSITING_MODE_DVR
        ray.color = (float4)(0.f);
        ray.firsthit = 0.f;
        #endif
    }

    if (ray.param >= 1.f)
        return;
#endif

    // traverse ray
    if (entry.w > 0.f && (entry.x != exit.x || entry.y != exit.y || entry.z != exit.z)) {
#ifdef COMPOSITING_MODE_DVR
        traverseRayDVR(entry.xyz, exit.xyz, viewportSize, nodeBuffer, brickBuffer, brickFlagBuffer,
                                nodeLevelOfDetail, realWorldMapping,
                                transFunc, transFuncDomains, samplingStepSize,
                                &ray);
#elif COMPOSITING_MODE_MIP
        traverseRayMIP(entry.xyz, exit.xyz, viewportSize, nodeBuffer, brickBuffer, brickFlagBuffer,
                                nodeLevelOfDetail, realWorldMapping,
                                transFunc, transFuncDomains, samplingStepSize,
                                &ray);
#elif COMPOSITING_MODE_MOP
        traverseRayMOP(entry.xyz, exit.xyz, viewportSize, nodeBuffer, brickBuffer, brickFlagBuffer,
                                nodeLevelOfDetail, realWorldMapping,
                                transFunc, transFuncDomains, samplingStepSize,
                                &ray);
#else
        ERROR: unknown compositing mode
#endif
    }

// store ray result in ray buffer
writeRayToBuffer(rayBuffer, fragPos, viewportSize, &ray);

// write fragment
#ifdef DISPLAY_MODE_REFINEMENT
    // DVR:     output finished rays only
    // MIP/MOP: always output ray result
    bool outputRay = true;
    #ifdef COMPOSITING_MODE_DVR
    if (ray.param < 1.f)
        outputRay = false;
    #endif

    if (outputRay) {
        write_imagef(output, fragPos, ray.color);
        write_imagef(outputDepth, fragPos, ray.firsthit);
    }

#else // full frame mode => always output ray
    write_imagef(output, fragPos, ray.color);
    write_imagef(outputDepth, fragPos, ray.firsthit);
#endif

}

//-------------------------------------------------------------------------------------------
// helper kernels

__kernel void updateBrickBuffer(__global const ushort* brickUpdateBuffer,
                                __global const uint* brickUpdateAddressBuffer,
                                const uint numUpdateBricks,
                                __global ushort* brickBuffer
                                )
{
    // each brick is divided among 16 work units
    const uint BRICK_NUM_BLOCKS = 16;

    const uint brickID =          get_global_id(0) / BRICK_NUM_BLOCKS;
    const uint brickBlockOffset = get_global_id(0) % BRICK_NUM_BLOCKS;
    if (brickID >= numUpdateBricks)
        return;

    const uint BRICK_NUMVOXELS = OCTREE_BRICKDIM*OCTREE_BRICKDIM*OCTREE_BRICKDIM*OCTREE_NUMCHANNELS;
    const uint srcOffset =   brickID*BRICK_NUMVOXELS + brickBlockOffset*BRICK_NUMVOXELS/BRICK_NUM_BLOCKS;
    const uint destOffset =  brickUpdateAddressBuffer[brickID]*BRICK_NUMVOXELS +
                             brickBlockOffset*BRICK_NUMVOXELS/BRICK_NUM_BLOCKS;
    for (int i=0; i<BRICK_NUMVOXELS/BRICK_NUM_BLOCKS; i++)
        brickBuffer[destOffset+i] = brickUpdateBuffer[srcOffset+i];
}

