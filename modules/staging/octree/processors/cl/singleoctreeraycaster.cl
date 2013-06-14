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
const sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__constant float SAMPLING_BASE_INTERVAL_RCP = 200.0;

// preprocessor defines
__constant uint  OCTREE_DIMENSIONS = OCTREE_DIMENSIONS_DEF; //< voxel dimensions of the octree (cubic, power-of-two, >= volume dim)
__constant uint  OCTREE_BRICKDIM = OCTREE_BRICKDIM_DEF;     //< brick dimensions (cubic, power-of-two)
__constant uint  OCTREE_DEPTH = OCTREE_DEPTH_DEF;           //< number of levels of the octree

//-------------------------------------
// 64-bit masks for node buffer entries

__constant ulong MASK_INBRICKPOOL = 0x8000000000000000;  //< 10000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
__constant uint  MASK_INBRICKPOOL_SHIFT  = 63;
__constant uint  MASK_INBRICKPOOL_NUMBITS = 1;

__constant ulong MASK_HOMOGENEOUS = 0x4000000000000000;  //< 01000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
__constant uint  MASK_HOMOGENEOUS_SHIFT  = 62;
__constant uint  MASK_HOMOGENEOUS_NUMBITS = 1;

__constant ulong MASK_CHILD =       0x3FFFFFFF00000000;  //< 00111111 11111111 11111111 11111111 00000000 00000000 00000000 00000000
__constant uint  MASK_CHILD_SHIFT  =  32;
__constant uint  MASK_CHILD_NUMBITS = 30;

__constant ulong MASK_AVG =         0x00000000FFF00000;  //< 00000000 00000000 00000000 00000000 11111111 11110000 00000000 00000000
__constant uint  MASK_AVG_SHIFT  =  20;
__constant uint  MASK_AVG_NUMBITS = 12;

__constant ulong MASK_BRICK =       0x00000000000FFFFF;  //< 00000000 00000000 00000000 00000000 00000000 00001111 11111111 11111111
__constant uint  MASK_BRICK_SHIFT  =  0;
__constant uint  MASK_BRICK_NUMBITS = 20;

// 8-bit masks for node flag buffer entries
__constant uchar MASK_BRICK_USED =      1;  //< 00000001
__constant uchar MASK_BRICK_REQUESTED = 2;  //< 00000010
__constant uchar MASK_NODE_TRAVERSED =  4;  //< 00000100

// ---------------------
// node access functions

bool hasBrick(const ulong node) {
    return (node & MASK_INBRICKPOOL) > 0;
}

bool isHomogeneous(const ulong node) {
    return (node & MASK_HOMOGENEOUS) > 0;
}

/// Returns the node's avg value, converted to a normalized float.
float getNodeAvgValue(const ulong node) {
    ulong avgInt = (node & MASK_AVG) >> MASK_AVG_SHIFT;
    float avgNorm = clamp((float)(avgInt) * 1.f/(float)(1 << MASK_AVG_NUMBITS), 0.f, 1.f);
    //float avgNorm = clamp((float)(avgInt) * (1.f/4096.f), 0.f, 1.f); //< for 12 bit precision
    return avgNorm;
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
ulong getNodeBrickOffset(const ulong node) {
    return ((node & MASK_BRICK) >> MASK_BRICK_SHIFT);
}

__global const ushort* getNodeBrick(const ulong node, __global const ushort* brickBuffer) {
    ulong brickOffset = getNodeBrickOffset(node);
    return &brickBuffer[brickOffset*(OCTREE_BRICKDIM*OCTREE_BRICKDIM*OCTREE_BRICKDIM)];
}

// ----------------------------
// flag buffer access functions

bool hasBrickBeenUsed(const uchar flagEntry) {
    return (flagEntry & MASK_BRICK_USED) > 0;
}
void setBrickUsed(global uchar* flagEntry, const bool used) {
    *flagEntry = (*flagEntry & ~MASK_BRICK_USED) | (used ? MASK_BRICK_USED : 0);
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
    float4 res4 = matVecMult4x4(mat, (float4)(vec, 1.0));
    return (res4 / res4.w).xyz;
}

//-------------------------------------
// brick filtering

float filterBrickNearest(float3 samplePosInBrick, __global const ushort* brick) {
    uint3 sampleCoordsInBrick;
    sampleCoordsInBrick.x = clamp((uint)round(samplePosInBrick.x*(float)(OCTREE_BRICKDIM)-0.5), (uint)0, (uint)(OCTREE_BRICKDIM-1));
    sampleCoordsInBrick.y = clamp((uint)round(samplePosInBrick.y*(float)(OCTREE_BRICKDIM)-0.5), (uint)0, (uint)(OCTREE_BRICKDIM-1));
    sampleCoordsInBrick.z = clamp((uint)round(samplePosInBrick.z*(float)(OCTREE_BRICKDIM)-0.5), (uint)0, (uint)(OCTREE_BRICKDIM-1));
    uint inBrickOffset = cubicCoordToLinear(sampleCoordsInBrick, (uint3)(OCTREE_BRICKDIM));
    return brick[inBrickOffset] * (1.0/(float)(1 << 16)); // 65535.f
}

float filterBrickLinear(float3 samplePosInBrick, __global const ushort* brick) {
    const float3 sampleCoordsInBrick = samplePosInBrick * ((float3)(OCTREE_BRICKDIM)-1.0);

    // cubic coordinates of llf/urb corner voxels surrounding the sample pos
    uint3 llf = (uint3)(floor(sampleCoordsInBrick.x),
                        floor(sampleCoordsInBrick.y),
                        floor(sampleCoordsInBrick.z));
    uint3 urb = llf + (uint3)(1);
    llf = clamp(llf, (uint3)(0), (uint3)(OCTREE_BRICKDIM-1));
    urb = min(urb, (uint3)(OCTREE_BRICKDIM-1));

    // linear coodinates of the eight corner voxels
    uint llfOffset = cubicCoordToLinear((uint3)(llf.x, llf.y, llf.z), (uint3)(OCTREE_BRICKDIM));
    uint llbOffset = cubicCoordToLinear((uint3)(llf.x, llf.y, urb.z), (uint3)(OCTREE_BRICKDIM));
    uint lrfOffset = cubicCoordToLinear((uint3)(llf.x, urb.y, llf.z), (uint3)(OCTREE_BRICKDIM));
    uint lrbOffset = cubicCoordToLinear((uint3)(llf.x, urb.y, urb.z), (uint3)(OCTREE_BRICKDIM));
    uint ulfOffset = cubicCoordToLinear((uint3)(urb.x, llf.y, llf.z), (uint3)(OCTREE_BRICKDIM));
    uint ulbOffset = cubicCoordToLinear((uint3)(urb.x, llf.y, urb.z), (uint3)(OCTREE_BRICKDIM));
    uint urfOffset = cubicCoordToLinear((uint3)(urb.x, urb.y, llf.z), (uint3)(OCTREE_BRICKDIM));
    uint urbOffset = cubicCoordToLinear((uint3)(urb.x, urb.y, urb.z), (uint3)(OCTREE_BRICKDIM));

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
    float sample = llfVoxel * (1.0-p.x)*(1.0-p.y)*(1.0-p.z) +
                   llbVoxel * (1.0-p.x)*(1.0-p.y)*(    p.z) +
                   lrfVoxel * (1.0-p.x)*(    p.y)*(1.0-p.z) +
                   lrbVoxel * (1.0-p.x)*(    p.y)*(    p.z) +
                   ulfVoxel * (    p.x)*(1.0-p.y)*(1.0-p.z) +
                   ulbVoxel * (    p.x)*(1.0-p.y)*(    p.z) +
                   urfVoxel * (    p.x)*(    p.y)*(1.0-p.z) +
                   urbVoxel * (    p.x)*(    p.y)*(    p.z);

    sample *= (1.0/(float)(1 << 16)); // 65535.f;

    return sample;
}

//-------------------------------------
// transfer function

typedef struct {
    float scale_;
    float offset_;
} RealWorldMapping;

const sampler_t transFuncSampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

float4 applyTransFunc(const float normIntensity, read_only image2d_t transFuncTex,
    const float2 transFuncDomain, const RealWorldMapping realWorldMapping)
{
    // transform intensity value into TF domain
    float realWorldIntensity = (normIntensity * realWorldMapping.scale_) + realWorldMapping.offset_;
    float tfIntensity = (realWorldIntensity - transFuncDomain.x) / (transFuncDomain.y - transFuncDomain.x);

    // apply transfer function
    float4 color = read_imagef(transFuncTex, transFuncSampler, (float2)(tfIntensity, 0.0));

    return color;
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

typedef struct {
    ulong value_;
    ulong offset_;
    float3 llf_;
    float3 urb_;
} OctreeNode;

OctreeNode getNodeAtSamplePos(const float3 samplePos, const uint requestedLevel, __global const ulong* nodeBuffer,
    uint* returnedLevel, OctreeNode* parentNode, OctreeNode* grandParentNode)
{

    OctreeNode currentNode;
    currentNode.value_ = nodeBuffer[0];
    currentNode.offset_ = 0;
    currentNode.llf_ = (float3)(0.0, 0.0, 0.0);
    currentNode.urb_ = (float3)(1.0, 1.0, 1.0);

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
            float3 nodeHalfDim = nodeDim * (float3)(0.5);
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


typedef struct {
    float4 color;
    float depth;
} RayResult;

RayResult traverseRay(const float3 entry, const float3 exit, const uint2 viewportSize,
                      __global const ulong* nodeBuffer,
                      __global const ushort* brickBuffer,
                      __global uchar* brickFlagBuffer,
                      const int nodeLevelOfDetail,
                      RealWorldMapping realWorldMapping,
                      read_only image2d_t transFunc,
                      const float2 transFuncDomain,
                      const float samplingStepSize)
{
    // calculate ray parameters
    float3 direction = exit - entry;
    float tEnd = length(direction);
    if (tEnd > 0.f)
        direction = normalize(direction);

    // ray-casting loop
    float t = 0.f;
    RayResult result;
    result.color = (float4)(0.0f);  ///< resulting color
    float tFirstHit = 1.0f;         ///< t parameter of first hit point (used for depth calculation)
    result.depth = -1.0f;           ///< resulting depth
    bool finished = false;          ///< loop end

    OctreeNode currentNode;         ///< current octree node
    uint currentNodeLevel = 0;      ///< level of current node
    float currentNodeAvg;           ///< average value of current node
    float3 currentNodeDimRec;       ///< reciproke dimension of current node in texture coordinates

    bool currentNodeHasBrick = false;          ///< does current node have a brick?
    __global const ushort* currentBrick = 0;   ///< brick of current node (uint16_t)

    float samplingStepSizeNode = samplingStepSize; ///< adapted sampling step size for current node
    float tEndCurrentNode = -1.f;                  ///< end ray parameter for current node, i.e. t value of last sample within the current node

    // ray casting loop
    for (int loop=0; !finished && loop<65535; ++loop) {

        float3 sample = entry + t*direction;
        float currentStepSize = samplingStepSizeNode;

        // retrieve next node, if ray has passed last sample within current node
        if (t > (tEndCurrentNode+1.e-6f)) {

            // retrieve node
            uint origNodeLevel;
            OctreeNode parentNode, grandParentNode;
            OctreeNode origNode = getNodeAtSamplePos(sample, nodeLevelOfDetail, nodeBuffer, &origNodeLevel, &parentNode, &grandParentNode);

            // compute node exit point and ray end parameter for original node
            if (tEnd > 0.f) {
                float3 nodeExit = computeNodeExitPoint(origNode.llf_, origNode.urb_, sample, direction);
                // determine ray parameter of last sample before node exit point
                float tOffset = minFloat3((nodeExit - sample) / direction);
                tOffset = floor(tOffset / samplingStepSize) * samplingStepSize;
                tEndCurrentNode = t + tOffset;
            }

               currentNode = origNode;
               currentNodeLevel = origNodeLevel;
            currentNodeHasBrick = false;
            currentBrick = 0;

#ifdef USE_BRICKS
            if (!isHomogeneous(origNode.value_)) { // node not homogeneous => fetch brick
                currentNodeHasBrick = hasBrick(currentNode.value_);
                if (currentNodeHasBrick) { //< brick is in GPU buffer => use it
                    currentBrick = getNodeBrick(currentNode.value_, brickBuffer);
                }
                else { //< brick is not in GPU buffer => request it
                    setBrickRequested(brickFlagBuffer + origNode.offset_, true);

                    // use parent or grandparent node, if they have a brick
                    #ifdef USE_ANCESTOR_NODES
                        if (hasBrick(parentNode.value_)) {
                            currentNode = parentNode;
                            currentNodeLevel -= 1;
                            currentBrick = getNodeBrick(currentNode.value_, brickBuffer);
                            currentNodeHasBrick = true;
                        }
                        else if (hasBrick(grandParentNode.value_)) {
                            setBrickRequested(brickFlagBuffer + parentNode.offset_, true);
                            currentNode = grandParentNode;
                            currentNodeLevel -= 2;
                            currentBrick = getNodeBrick(currentNode.value_, brickBuffer);
                                currentNodeHasBrick = true;
                        }
                    #endif

                }
            }

            setBrickUsed(brickFlagBuffer + currentNode.offset_, currentNodeHasBrick);
            //setNodeTraversed(brickFlagBuffer + currentNode.offset_, true);
#endif

            currentNodeDimRec = 1.0 / (currentNode.urb_-currentNode.llf_);
            currentNodeAvg = getNodeAvgValue(currentNode.value_);

#ifdef ADAPTIVE_SAMPLING
            // adapt sampling step size to current node level/resolution
            samplingStepSizeNode = samplingStepSize * (float)(1<<(OCTREE_DEPTH-1-min(currentNodeLevel, OCTREE_DEPTH-1)));
#else
            samplingStepSizeNode = samplingStepSize;
#endif

        } // node retrieval


        // retrieve sample intensity
        float intensity;
        if (currentNodeHasBrick) { // sample brick
            float3 samplePosInBrick = (sample - currentNode.llf_) * currentNodeDimRec;
            #ifdef TEXTURE_FILTER_LINEAR
               intensity = filterBrickLinear(samplePosInBrick, currentBrick);
            #else
               intensity = filterBrickNearest(samplePosInBrick, currentBrick);
            #endif
        }
        else { // node has no brick => use average value
            intensity = currentNodeAvg;
        }

        // apply transfer function
        float4 sampleColor = applyTransFunc(intensity, transFunc, transFuncDomain, realWorldMapping);

        // perform compositing
        if (sampleColor.w > 0.0f) {
            // apply opacity correction to accomodate for variable sampling intervals
            sampleColor.w = 1.f - pow(1.f - sampleColor.w, currentStepSize * SAMPLING_BASE_INTERVAL_RCP);

            // actual compositing
            result.color.xyz = result.color.xyz + (1.0f - result.color.w) * sampleColor.w * sampleColor.xyz;
            result.color.w = result.color.w + (1.0f - result.color.w) * sampleColor.w;

            // save first-hit point
            tFirstHit = min(t, tFirstHit);
        }

        // switch back to base sampling step size,
        // if node step size would yield a sampling point beyond node exit point
        if (t+samplingStepSizeNode > (tEndCurrentNode+1.e-6f))
            samplingStepSizeNode = samplingStepSize;

        // advance along ray
        t += samplingStepSizeNode;

        // early ray termination
        if (result.color.w >= 0.95f) {
            result.color.w = 1.0f;
            finished = true;
        }

        finished = finished || (t > tEnd);

    } // ray-casting loop

    result.depth = tFirstHit;

    return result;
}

__kernel void render(read_only image2d_t entryTex,
                     read_only image2d_t exitTex,
                     const uint2 viewportSize,
                     const uint coarsenessFactor,

                     const float samplingStepSize,
                     const uint4 volumeDimensions,

                     __global const ulong* nodeBuffer,
                     __global const ushort* brickBuffer,
                     __global uchar* brickFlagBuffer,
                     const uint nodeLevelOfDetail,

                     RealWorldMapping realWorldMapping,
                     read_only image2d_t transFunc,
                     const float2 transFuncDomain,

                     write_only image2d_t output,
                     write_only image2d_t outputDepth
                )
{
    if (get_global_id(0) >= viewportSize.x || get_global_id(1) >= viewportSize.y)
        return;

    // each workgroup is assigned to one tile of the image
    const uint2 localSize = (uint2)(get_local_size(0), get_local_size(1));
    const uint2 groupID = (uint2)(get_group_id(0), get_group_id(1));
    const uint2 localID = (uint2)(get_local_id(0), get_local_id(1));
    const int2 fragPos = (int2)(groupID.x*localSize.x + localID.x, groupID.y*localSize.y + localID.y);

    // read entry/exit points
    float4 entry = read_imagef(entryTex, imageSampler, fragPos*(int2)(coarsenessFactor));
    float4 exit = read_imagef(exitTex, imageSampler, fragPos*(int2)(coarsenessFactor));

    // transform entry/exit points from NPOT to POT: (1.0,1.0,1.0) refers to the URB voxel of the volume
    // (which differs from the URB of the octree for NPOT volumes)
    const float4 coordinateCorrection =
        (float4)((float)volumeDimensions.x / (float)OCTREE_DIMENSIONS,
                 (float)volumeDimensions.y / (float)OCTREE_DIMENSIONS,
                 (float)volumeDimensions.z / (float)OCTREE_DIMENSIONS,
                 1);
    entry = entry*coordinateCorrection;
    exit = exit*coordinateCorrection;

    // traverse ray
    RayResult rayResult;
    if (entry.w > 0.0 && (entry.x != exit.x || entry.y != exit.y || entry.z != exit.z)) {
        rayResult = traverseRay(entry.xyz, exit.xyz, viewportSize, nodeBuffer, brickBuffer, brickFlagBuffer, nodeLevelOfDetail,
                    realWorldMapping, transFunc, transFuncDomain, samplingStepSize);
    }
    else {
        rayResult.color = (float4)(0.0);
        rayResult.depth = 1.0;
    }

    // write fragment
    write_imagef(output, fragPos, rayResult.color);
    write_imagef(outputDepth, fragPos, rayResult.depth);

}

//-------------------------------------------------------------------------------------------
// helper kernels

__kernel void updateBrickBuffer(
                    __global const ushort* brickUpdateBuffer,
                    __global const uint* brickUpdateAddressBuffer,
                    const uint numUpdateBricks,
                    __global ushort* brickBuffer
                    )
{
    const uint brickID = get_global_id(0);
    if (brickID >= numUpdateBricks)
        return;

    const uint BRICK_NUMVOXELS = OCTREE_BRICKDIM*OCTREE_BRICKDIM*OCTREE_BRICKDIM;
    const uint srcOffset = brickID*BRICK_NUMVOXELS;
    const uint destOffset =  brickUpdateAddressBuffer[brickID]*BRICK_NUMVOXELS;
    for (int i=0; i<BRICK_NUMVOXELS; i++)
        brickBuffer[destOffset+i] = brickUpdateBuffer[srcOffset+i];
}
