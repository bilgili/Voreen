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

#include "singleoctreeraycastercpu.h"

#include "voreen/core/datastructures/octree/volumeoctree.h"
#include "voreen/core/datastructures/octree/octreeutils.h"

#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/geometry/meshgeometry.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorresizepoweroftwo.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorconvert.h"
#include "voreen/core/datastructures/volume/volumedecorator.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"

#include "tgt/vector.h"
#include "tgt/tgt_math.h"
#include "tgt/stopwatch.h"

using tgt::ivec2;
using tgt::ivec3;
using tgt::ivec4;
using tgt::vec2;
using tgt::vec3;
using tgt::vec4;
using tgt::svec3;

namespace {

inline tgt::vec3 computeNodeExitPoint(const tgt::vec3& nodeLLF, const tgt::vec3& nodeURB, const tgt::vec3& nodeEntry, const tgt::vec3& rayDir) {
    tgtAssert(tgt::hand(tgt::lessThan(nodeLLF, nodeURB)), "node LLF not less than node URB");
    tgtAssert(inRange(nodeEntry, nodeLLF, nodeURB), "node entry point outside node");

    vec3 exitPlanes = vec3(rayDir.x >= 0.f ? nodeURB.x : nodeLLF.x,
        rayDir.y >= 0.f ? nodeURB.y : nodeLLF.y,
        rayDir.z >= 0.f ? nodeURB.z : nodeLLF.z);
    //vec3 tNodeExit = (exitPlanes - nodeEntry) / rayDir;
    vec3 tNodeExit;
    tNodeExit.x = rayDir.x != 0.f ? ((exitPlanes.x - nodeEntry.x) / rayDir.x) : 1e6f;
    tNodeExit.y = rayDir.y != 0.f ? ((exitPlanes.y - nodeEntry.y) / rayDir.y) : 1e6f;
    tNodeExit.z = rayDir.z != 0.f ? ((exitPlanes.z - nodeEntry.z) / rayDir.z) : 1e6f;
    tgtAssert(tgt::hand(tgt::greaterThanEqual(tNodeExit, vec3::zero)), "at least one negative node exit parameter");

    float tNodeExitMin = tgt::min(tNodeExit);
    tgtAssert(inRange(tNodeExitMin, 0.f, 1.f), "minimum node exit parameter outside range [0.0;1.0]");

    vec3 nodeExit = nodeEntry + (tNodeExitMin - 1e-6f)*rayDir;
    tgtAssert(inRange(nodeExit, nodeLLF, nodeURB), "node exit point outside node");
    return nodeExit;
}

const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

}

namespace voreen {

const std::string SingleOctreeRaycasterCPU::loggerCat_("voreen.SingleOctreeRaycasterCPU");

SingleOctreeRaycasterCPU::SingleOctreeRaycasterCPU()
    : RenderProcessor()
    , octreeInport_(Port::INPORT, "octreeInput", "VolumeOctree Input")
    , entryPointsInport_(Port::INPORT, "entryPointsInport", "Entry-points", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , exitPointsInport_(Port::INPORT, "exitPointsInport", "Exit-points", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , renderOutport_(Port::OUTPORT, "renderOutport", "Image Output", true, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_RECEIVER)
    , octreeGeometryOutport_(Port::OUTPORT, "octreeGeometryOutport", "Octree Geometry")
    , levelVolumeOutport_(Port::OUTPORT, "levelVolumeOutport", "Level Volume")
    , rayNodeGeometryPort_(Port::OUTPORT, "rayNodeGeometryOutport", "Ray Node Geometry")
    , testVolumeOutport_(Port::OUTPORT, "testVolumeOutport", "Test Volume")
    , cameraProperty_("cameraProperty", "Camera")
    , transferFunc_("transferFunction", "Transfer function")
    , samplingRate_("samplingRate", "Sampling Rate", 2.f, 0.01f, 20.f)
    , rayPixelCoordsRel_("rayPixelCoordsRel", "Ray Pixel Coords Rel", tgt::vec2(-1.f), tgt::vec2(-1.f), tgt::vec2(1.f))
    , outputVolumeLevel_("outputVolumeLevel", "Output Volume Level", 0, 0, 10)
{
    addPort(octreeInport_);
    addPort(entryPointsInport_);
    addPort(exitPointsInport_);

    addPort(renderOutport_);

    addPort(octreeGeometryOutport_);
    addPort(levelVolumeOutport_);
    addPort(rayNodeGeometryPort_);
    addPort(testVolumeOutport_);

    addProperty(outputVolumeLevel_);
    ON_PROPERTY_CHANGE(outputVolumeLevel_, SingleOctreeRaycasterCPU, clearLevelVolume);

    addProperty(cameraProperty_);
    addProperty(transferFunc_);
    addProperty(samplingRate_);

    addProperty(rayPixelCoordsRel_);
}

SingleOctreeRaycasterCPU::~SingleOctreeRaycasterCPU() {
}

Processor* SingleOctreeRaycasterCPU::create() const {
    return new SingleOctreeRaycasterCPU();
}

bool SingleOctreeRaycasterCPU::isReady() const {
    return (octreeInport_.isReady() /*&& (octreeGeometryOutport_.isConnected() || levelVolumeOutport_.isConnected())*/);
}

void SingleOctreeRaycasterCPU::process() {
    if (!octreeInport_.getData()->hasRepresentation<VolumeOctreeBase>()) {
        LWARNING("Input volume has no octree representation. Use OctreeCreator processor!");
        return;
    }
    const VolumeOctreeBase* inputOctree = octreeInport_.getData()->getRepresentation<VolumeOctreeBase>();
    tgtAssert(inputOctree, "no input octree");

    if (octreeInport_.hasChanged()) {
        octreeGeometryOutport_.clear();
        levelVolumeOutport_.clear();
        rayNodeGeometryPort_.clear();
    }

    // generate octree geometry (entire tree), if respective outport needs data
    /*if (!octreeGeometryOutport_.hasData() && octreeGeometryOutport_.isConnected()) {
        tgt::Stopwatch watch; watch.start();
        MeshListGeometry* treeGeometry = inputOctree->createOctreeGeometry(true);
        watch.stop();
        octreeGeometryOutport_.setData(treeGeometry, true);
        LINFO("Geometry meshes: " << treeGeometry->getMeshCount() << " (" << watch.getRuntime() << " ms)");
    } */

    // generate level volume, if respective outport needs data
    if (levelVolumeOutport_.isConnected() && !levelVolumeOutport_.hasData()) {
        try {
            /*tgt::Stopwatch watch; watch.start();
            VolumeRAM* levelVolumeRAM = inputOctree->createLevelVolume(static_cast<size_t>(outputVolumeLevel_.get()));
            watch.stop();
            Volume* levelVolume = new Volume(levelVolumeRAM, inputVolume->)
            levelVolumeOutport_.setData(levelVolume, true);
            LINFO("Level volume dims: " << levelVolume->getDimensions() << " (" << watch.getRuntime() << " ms)"); */
        }
        catch (std::exception& e) {
            LERROR("Failed to create level volume: " << e.what());
            levelVolumeOutport_.clear();
        }
    }

    // perform actual volume rendering
    if (renderOutport_.isConnected() && entryPointsInport_.isReady() && exitPointsInport_.isReady())
        renderVolume();

}

MeshListGeometry* SingleOctreeRaycasterCPU::generateRayNodePathGeometry(const std::vector<RayNode>& rayNodes) {
    const VolumeOctreeBase* octree = octreeInport_.getData()->getRepresentation<VolumeOctreeBase>();
    tgtAssert(octree, "no octree");

    std::vector<tgt::vec3> levelColors;
    levelColors.push_back(vec3(0.f, 0.f, 0.f));
    levelColors.push_back(vec3(1.f, 0.f, 0.f));
    levelColors.push_back(vec3(0.f, 1.f, 0.f));
    levelColors.push_back(vec3(0.f, 0.f, 1.f));
    levelColors.push_back(vec3(1.f, 0.7f, 0.f));
    levelColors.push_back(vec3(0.f, 1.f, 1.f));
    levelColors.push_back(vec3(1.f, 0.f, 1.f));

    MeshListGeometry* rayGeom = new MeshListGeometry();

    for(size_t i = 0; i < rayNodes.size();++i) {
        tgt::vec3 nodeColor = levelColors.at(octree->getNumLevels() - rayNodes[i].depth_ % levelColors.size());
        MeshGeometry sampleMesh = MeshGeometry::createCube(rayNodes[i].llf_, rayNodes[i].urb_,
                vec3(0.f), vec3(0.f),
                //vec3(0.f,0.f,1.f), vec3(0.f,0.f,1.f)
                nodeColor, nodeColor);
        rayGeom->addMesh(sampleMesh);
    }

    //rayGeom->transform(octree_->getVolume()->getTextureToWorldMatrix());
    //rayGeom->setTransformationMatrix(octree->getTextureToWorldMatrix()); //< TODO

    return rayGeom;
}

void SingleOctreeRaycasterCPU::clearLevelVolume() {
    levelVolumeOutport_.clear();
}

void SingleOctreeRaycasterCPU::renderVolume() {
    const VolumeOctreeBase* octree = octreeInport_.getData()->getRepresentation<VolumeOctreeBase>();
    tgtAssert(octree, "no octree");

    // resize outport to inport dimensions
    tgt::svec2 renderSize = entryPointsInport_.getSize();
    if (renderSize != tgt::svec2(renderOutport_.getSize()))
        renderOutport_.resize(renderSize);

    const RealWorldMapping rwm; // = octree->getRealWorldMapping(); << TODO

    TransFunc1DKeys* transfunc = dynamic_cast<TransFunc1DKeys*>(transferFunc_.get());
    tgtAssert(transfunc, "no 1D TF");
    // use dimension with the highest resolution for calculating the sampling step size
    float samplingStepSize = 1.f / (tgt::max(octree->getDimensions()) * samplingRate_.get());

    renderOutport_.activateTarget();
    renderOutport_.clearTarget();

    if (entryPointsInport_.hasData() && exitPointsInport_.hasData()) {
        tgtAssert(renderOutport_.getSize() == entryPointsInport_.getSize(), "output size does not match eep size");

        // download entry/exit point textures
        tgt::vec4* entryBuffer = reinterpret_cast<tgt::vec4*>(
            entryPointsInport_.getColorTexture()->downloadTextureToBuffer(GL_RGBA, GL_FLOAT));
        tgt::vec4* exitBuffer = reinterpret_cast<tgt::vec4*>(
            exitPointsInport_.getColorTexture()->downloadTextureToBuffer(GL_RGBA, GL_FLOAT));
        LGL_ERROR;

        // create output buffer
        tgt::vec4* renderBuffer = new tgt::vec4[tgt::hmul(renderSize)*4];

        // debug ray node path
        ivec2 rayPixelCoordsAbs = tgt::iround(rayPixelCoordsRel_.get() * vec2(entryPointsInport_.getSize()));
        rayNodeGeometryPort_.clear();

        tgt::vec3 camPos = cameraProperty_.get().getPosition();
        tgt::mat4 projectionMatrix = cameraProperty_.get().getProjectionMatrix(renderOutport_.getSize());
        tgt::ivec2 viewport = renderOutport_.getSize();

        // render
        for (size_t y=0; y<renderSize.y; y++) {
            for (size_t x=0; x<renderSize.x; x++) {
                //renderBuffer[y*renderSize.x + x] = entryBuffer[y*renderSize.x + x];
                tgt::vec4 entryPoint = entryBuffer[y*renderSize.x + x];
                tgt::vec4 exitPoint = exitBuffer[y*renderSize.x + x];

                tgt::vec4 fragColor;
                if ((entryPoint == vec4(0.0)) && (exitPoint == vec4(0.0))) { //< background
                    fragColor = vec4(0.f);
                }
                else{ // inside volume
                    if (x == rayPixelCoordsAbs.x && y == rayPixelCoordsAbs.y) { //< track nodes passed by ray
                        std::vector<RayNode> rayNodes;
                        fragColor = traverseRay(entryPoint.xyz(), exitPoint.xyz(),
                            camPos, projectionMatrix, viewport,
                            rwm, transfunc, samplingStepSize, &rayNodes);
                        rayNodeGeometryPort_.setData(generateRayNodePathGeometry(rayNodes));
                    }
                    else
                        fragColor = traverseRay(entryPoint.xyz(), exitPoint.xyz(),
                            camPos, projectionMatrix, viewport,
                            rwm, transfunc, samplingStepSize);
                }

                //renderBuffer[y*renderSize.x + x] = entryBuffer[y*renderSize.x + x];
                renderBuffer[y*renderSize.x + x] = fragColor;
            }
        }

        // copy renderBuffer to framebuffer
        glWindowPos2i(0, 0);
        glDrawPixels((GLsizei)renderSize.x, (GLsizei)renderSize.y, GL_RGBA, GL_FLOAT, renderBuffer);
        LGL_ERROR;

        delete[] entryBuffer;
        delete[] exitBuffer;
        delete[] renderBuffer;
    }

    renderOutport_.deactivateTarget();
}

tgt::vec4 SingleOctreeRaycasterCPU::traverseRay(const tgt::vec3& entry, const tgt::vec3& exit,
    const tgt::vec3& camPos, const tgt::mat4& projectionMatrix, const tgt::ivec2& viewportSize,
    const RealWorldMapping& realWorldMapping, TransFunc1DKeys* transFunc, const float samplingStepSize,
    std::vector<RayNode>* passedNodes)
{
    const VolumeOctreeBase* octree = octreeInport_.getData()->getRepresentation<VolumeOctreeBase>();
    tgtAssert(octree, "no octree");

    // get dimensions
    svec3 volDim = octree->getDimensions();
    svec3 brickDim = octree->getBrickDim();

    //tgt::mat4 textureToWorldMatrix = octree->getTextureToWorldMatrix();
    tgt::mat4 textureToWorldMatrix = tgt::mat4::identity; //< TODO

    // calculate ray parameters
    vec3 direction = exit - entry;
    float tEnd = length(direction);
    if (tEnd > 0.f)
        direction = normalize(direction);

    // ray-casting loop
    float t = 0.f;
    vec4 result = vec4(0.0f);       ///< resulting color
    //float depthT = -1.0f;         ///< resulting depth
    bool finished = false;          ///< loop end

    const VolumeOctreeNode* currentNode = 0;     ///< current octree node
    const uint16_t* currentBrick = 0;               ///< brick of current node. Is null, if node has no brick.
    tgt::vec3 currentNodeLLF;                       ///< lower left front coordinate of currentNode
    tgt::vec3 currentNodeURB;                       ///< upper right back coordinate ot currentNode
    float samplingStepSizeNode = samplingStepSize;  ///< adapted sampling step for current node
    float tEndCurrentNode = -1.f;   ///< end ray parameter for current node, i.e. t value of last sample within the current node

    // handling LOD
    size_t numPassedNodes = 0;

    // ray casting loop
    for (int loop=0; !finished && loop<65535; ++loop) {

        vec3 sample = entry + t*direction;
        float currentStepSize = samplingStepSizeNode;

        // retrieve next node, if ray has passed last sample within current node
        if (!currentNode || t > (tEndCurrentNode+1.e-6f) /* !(tgt::hand(tgt::lessThanEqual(currentNodeLLF, sample)) && tgt::hand(tgt::lessThanEqual(sample, currentNodeURB))*/) {
            // determine node level for current sampling position
            const size_t requestedNodeLevel = computeNodeLevel(
                sample, volDim, textureToWorldMatrix,
                octree->getNumLevels(), camPos, projectionMatrix, viewportSize);
            tgtAssert(requestedNodeLevel < octree->getNumLevels(), "invalid node LOD");

            // retrieve node and brick
            size_t nodeLevel = requestedNodeLevel;
            tgt::svec3 voxelLLF, voxelURB;
            currentNode = octree->getNode(sample, nodeLevel, voxelLLF, voxelURB, currentNodeLLF, currentNodeURB);
            tgtAssert(currentNode, "null pointer returned");
            tgtAssert(tgt::hand(tgt::lessThanEqual(currentNodeLLF, sample)) && tgt::hand(tgt::lessThanEqual(sample, currentNodeURB)), "invalid node  bounding box");
            currentBrick = octree->getNodeBrick(currentNode);

            // adapt sampling step size to current node level/resolution
            samplingStepSizeNode = samplingStepSize * (float)(1<<(octree->getNumLevels()-1-nodeLevel));

            // compute node exit point and ray end parameter for current node
            if (tEnd > 0.f) {
                vec3 nodeExit = computeNodeExitPoint(currentNodeLLF, currentNodeURB, sample, direction);
                tgtAssert(inRange(nodeExit, currentNodeLLF, currentNodeURB), "node exit point outside node");
                // determine ray parameter of last sample before node exit point
                float tOffset = tgt::min((nodeExit - sample) / direction);
                tOffset = std::floor(tOffset / samplingStepSize) * samplingStepSize;
                tEndCurrentNode = t + tOffset;
            }

            // debug ray node path
            if (passedNodes){
                RayNode node;
                node.depth_ = currentNode->getDepth();
                node.llf_ = currentNodeLLF;
                node.urb_ = currentNodeURB;
                passedNodes->push_back(node);

                //LINFO("requested/returned node level: " << requestedNodeLevel << "/" << nodeLevel);
            }
        }
        tgtAssert(currentNode, "node expected at this point");
        tgtAssert(currentStepSize >= samplingStepSize, "current sampling step size for smaller than base step size");
        tgtAssert(samplingStepSizeNode >= samplingStepSize, "sampling step size for current node smaller than base step size");

        // retrieve sample intensity
        float intensity;
        if (currentBrick) { // sample brick
            vec3 relSamplePosInBrick = (sample - currentNodeLLF) / (currentNodeURB-currentNodeLLF);
            svec3 sampleCoordsInBrick = tgt::iround(relSamplePosInBrick*vec3(brickDim)-tgt::vec3(0.5f));
            sampleCoordsInBrick = tgt::clamp(sampleCoordsInBrick, svec3::zero, brickDim-svec3(1));
            intensity = currentBrick[sampleCoordsInBrick.z*brickDim.y*brickDim.x + sampleCoordsInBrick.y*brickDim.x + sampleCoordsInBrick.x] / 65535.f;
        }
        else { // node has no brick => use average value
            intensity = currentNode->getAvgValue() / 65535.f;
        }

        // transform intensity value into TF domain
        float realWorldIntensity = realWorldMapping.normalizedToRealWorld(intensity);
        float tfIntensity = transFunc->realWorldToNormalized(realWorldIntensity);

        // apply transfer function
        vec4 color = applyTransFunc(transFunc->getTexture(), tfIntensity);

        // perform compositing
        if (color.a > 0.0f) {
            // apply opacity correction to accomodate for variable sampling intervals
            color.a = 1.f - pow(1.f - color.a, currentStepSize * SAMPLING_BASE_INTERVAL_RCP);

            // actual compositing
            result.xyz() = result.xyz() + (1.0f - result.a) * color.a * vec3(color.elem);
            result.a = result.a + (1.0f - result.a) * color.a;
        }

        // switch back to base sampling step size,
        // if node step size would yield a sampling point beyond node exit point
        if (t+samplingStepSizeNode > (tEndCurrentNode+1.e-6f))
            samplingStepSizeNode = samplingStepSize;

        // advance along ray
        t += samplingStepSizeNode;

        // save first hit ray parameter for depth value calculation
        //if (depthT < 0.0f && result.a > 0.0f)
        //    depthT = t;

        // early ray termination
        if (result.a >= 0.95f) {
            result.a = 1.0f;
            finished = true;
        }

        finished = finished || (t > tEnd);

    } // ray-casting loop

    return result;
}

tgt::vec4 SingleOctreeRaycasterCPU::applyTransFunc(tgt::Texture* tfTexture, float intensity) {
    size_t textureBin = static_cast<size_t>(tgt::clamp(tgt::iround(intensity * (tfTexture->getWidth()-1)), 0, tfTexture->getWidth()-1));
    tgt::vec4 value = tgt::vec4(tfTexture->texel<tgt::col4>(textureBin)) / 255.f;
    return value;
}

/*MeshListGeometry* SingleOctreeRaycasterCPU::generateRayNodeGeometry(const vec3& volumeEntry, const vec3& volumeExit) {
    tgtAssert(octree_, "no octree");
    tgtAssert(inRange(volumeEntry, tgt::vec3(0.f), tgt::vec3(1.f)), "invalid volume entry point");
    tgtAssert(inRange(volumeExit, tgt::vec3(0.f), tgt::vec3(1.f)), "invalid volume exit point");

    const size_t level = octree_->getNumLevels()-1;

    std::vector<tgt::vec3> levelColors;
    levelColors.push_back(vec3(0.f, 0.f, 0.f));
    levelColors.push_back(vec3(1.f, 0.f, 0.f));
    levelColors.push_back(vec3(0.f, 1.f, 0.f));
    levelColors.push_back(vec3(0.f, 0.f, 1.f));
    levelColors.push_back(vec3(1.f, 0.7f, 0.f));
    levelColors.push_back(vec3(0.f, 1.f, 1.f));
    levelColors.push_back(vec3(1.f, 0.f, 1.f));

    float stepSize = 0.01f;
    MeshListGeometry* rayGeom = new MeshListGeometry();
    vec3 rayDir = volumeExit-volumeEntry;
    float numSteps = tgt::length(rayDir)/stepSize;
    const VolumeOctreeNode* curNode = 0;
    for (float step=0; step<numSteps; step+=1.f) {
        vec3 samplePos = volumeEntry + (step/numSteps)*rayDir;
        vec3 normLlf, normUrb;
        svec3 llf, urb;
        size_t actualLevel = level;
        const VolumeOctreeNode* node = octree_->getOctreeNode(samplePos, actualLevel, llf, urb, normLlf, normUrb);
        tgtAssert(inRange(samplePos, normLlf, normUrb), "node does not contain sample pos");
        if (node != curNode) {
            curNode = node;
            tgt::vec3 nodeColor = levelColors.at(actualLevel % levelColors.size());
            MeshGeometry sampleMesh = MeshGeometry::createCube(normLlf, normUrb,
                vec3(0.f), vec3(0.f),
                //vec3(0.f,0.f,1.f), vec3(0.f,0.f,1.f)
                nodeColor, nodeColor);
            rayGeom->addMesh(sampleMesh);
        }
    }

    rayGeom->transform(volumeInport_.getData()->getTextureToWorldMatrix());

    return rayGeom;
} */

}   // namespace
