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

#include "octreeproxygeometry.h"

#include "modules/staging/octree/datastructures/volumeoctreebase.h"
#include "modules/staging/octree/datastructures/octreeutils.h"

#include "voreen/core/datastructures/geometry/meshlistgeometry.h"
#include "voreen/core/datastructures/geometry/trianglemeshgeometry.h"
#include "voreen/core/datastructures/transfunc/preintegrationtable.h"

namespace voreen {

const std::string OctreeProxyGeometry::loggerCat_("voreen.OctreeProxyGeometry");

using tgt::vec3;

OctreeProxyGeometry::OctreeProxyGeometry()
    : Processor()
    , inport_(Port::INPORT, "volume", "Volume Input")
    , outport_(Port::OUTPORT, "proxygeometry", "Proxy Geometry Output")
    , visibilityOptimization_("visibilityOptimization", "Visibility Optimization", false)
    , enableClipping_("useClipping", "Enable Clipping", true)
    , clipRight_("rightClippingPlane", "Right Clip Plane (x)", 0.f, 0.f, 1e5f)
    , clipLeft_("leftClippingPlane", "Left Clip Plane (x)", 1e5f, 0.f, 1e5f)
    , clipFront_("frontClippingPlane", "Front Clip Plane (y)", 0.f, 0.f, 1e5f)
    , clipBack_("backClippingPlane", "Back Clip Plane (y)", 1e5f, 0.f, 1e5f)
    , clipBottom_("bottomClippingPlane", "Bottom Clip Plane (z)", 0.f, 0.f, 1e5f)
    , clipTop_("topClippingPlane", "Top Clip Plane (z)", 1e5f, 0.f, 1e5f)
    , resetClipPlanes_("resetClipPlanes", "Reset Planes")
    , transfunc_("transfunc", "Transfer Function")
    , oldVolumeDimensions_(0, 0, 0)
{
    addPort(inport_);
    addPort(outport_);

    clipRight_.onChange(CallMemberAction<OctreeProxyGeometry>(this, &OctreeProxyGeometry::onClipRightChange));
    clipLeft_.onChange(CallMemberAction<OctreeProxyGeometry>(this, &OctreeProxyGeometry::onClipLeftChange));
    clipFront_.onChange(CallMemberAction<OctreeProxyGeometry>(this, &OctreeProxyGeometry::onClipFrontChange));
    clipBack_.onChange(CallMemberAction<OctreeProxyGeometry>(this, &OctreeProxyGeometry::onClipBackChange));
    clipBottom_.onChange(CallMemberAction<OctreeProxyGeometry>(this, &OctreeProxyGeometry::onClipBottomChange));
    clipTop_.onChange(CallMemberAction<OctreeProxyGeometry>(this, &OctreeProxyGeometry::onClipTopChange));
    enableClipping_.onChange(CallMemberAction<OctreeProxyGeometry>(this, &OctreeProxyGeometry::adjustClipPropertiesVisibility));
    resetClipPlanes_.onChange(CallMemberAction<OctreeProxyGeometry>(this, &OctreeProxyGeometry::resetClipPlanes));

    addProperty(visibilityOptimization_);

    addProperty(enableClipping_);
    addProperty(clipRight_);
    addProperty(clipLeft_);
    addProperty(clipFront_);
    addProperty(clipBack_);
    addProperty(clipBottom_);
    addProperty(clipTop_);
    addProperty(resetClipPlanes_);

    clipRight_.setGroupID("clipping");
    clipLeft_.setGroupID("clipping");
    clipFront_.setGroupID("clipping");
    clipBack_.setGroupID("clipping");
    clipBottom_.setGroupID("clipping");
    clipTop_.setGroupID("clipping");
    resetClipPlanes_.setGroupID("clipping");
    setPropertyGroupGuiName("clipping", "Clipping Planes");
    adjustClipPropertiesVisibility();

    addProperty(transfunc_);
}

OctreeProxyGeometry::~OctreeProxyGeometry() {
}

Processor* OctreeProxyGeometry::create() const {
    return new OctreeProxyGeometry();
}

void OctreeProxyGeometry::process() {
    tgtAssert(inport_.getData(), "no input octree");

    // adapt clipping plane properties on volume change
    if (inport_.hasChanged()) {
        adjustClipPropertiesRanges();
        adjustClippingToVolumeROI();
    }

    // retrieve octree from input volume
    tgtAssert(inport_.hasData(), "inport is empty");
    const VolumeBase* inputVolume = inport_.getData();
    tgtAssert(inputVolume, "no input volume");

    if (!inputVolume->hasRepresentation<VolumeOctreeBase>()) {
        LWARNING("Input volume has no octree representation. Use OctreeCreator processor!");
        outport_.clear();
        return;
    }
    const VolumeOctreeBase* inputOctree = inputVolume->getRepresentation<VolumeOctreeBase>();
    tgtAssert(inputOctree, "no input octree");
    const tgt::svec3 volumeDim = inputOctree->getDimensions();

    // determine normalized clipping planes (texture space)
    tgt::vec3 clipLlf(0.f);
    tgt::vec3 clipUrb(1.f);
    if (enableClipping_.get()) {
        clipLlf = tgt::vec3(clipRight_.get()  / static_cast<float>(volumeDim.x),
                            clipFront_.get()  / static_cast<float>(volumeDim.y),
                            clipBottom_.get() / static_cast<float>(volumeDim.z));
        clipUrb = tgt::vec3((clipLeft_.get()+1.0f) / static_cast<float>(volumeDim.x),
                            (clipBack_.get()+1.0f) / static_cast<float>(volumeDim.y),
                            (clipTop_.get()+1.0f)  / static_cast<float>(volumeDim.z));
    }

    // create proxy geometry in texture space
    Geometry* proxyGeometry = 0;
    if (visibilityOptimization_.get()) {
        TransFunc1DKeys* transfuncKeys = dynamic_cast<TransFunc1DKeys*>(transfunc_.get());
        if (transfuncKeys) {
            proxyGeometry = computeOptimizedProxyGeometry(inputOctree, clipLlf, clipUrb, inputVolume->getRealWorldMapping(), transfuncKeys);
        }
        else {
            LWARNING("Transfer function type not supported for optimization");
            proxyGeometry = TriangleMeshGeometryVec3::createCube(VertexVec3(clipLlf, clipLlf), VertexVec3(clipUrb, clipUrb));
        }
    }
    else {
        proxyGeometry = TriangleMeshGeometryVec3::createCube(VertexVec3(clipLlf, clipLlf), VertexVec3(clipUrb, clipUrb));
    }
    tgtAssert(proxyGeometry, "no proxy geometry created");

    // transform proxy geometry to world space
    proxyGeometry->setTransformationMatrix(inputVolume->getTextureToWorldMatrix());

    outport_.setData(proxyGeometry);
}

Geometry* OctreeProxyGeometry::computeOptimizedProxyGeometry(const VolumeOctreeBase* octree,
    const tgt::vec3& clipLlfNorm, const tgt::vec3& clipUrbNorm,
    const RealWorldMapping& rwm, TransFunc1DKeys* transfunc) const
{
    tgtAssert(octree, "null pointer passed");
    tgtAssert(transfunc, "null pointer passed");
    tgtAssert(tgt::hand(tgt::lessThanEqual(clipLlfNorm, clipUrbNorm)), "invalid clipping range");

    tgt::Stopwatch watch;
    watch.start();
    const PreIntegrationTable* preintegrationTable = transfunc->getPreIntegrationTable(1.f, 1024, true, false);
    tgtAssert(preintegrationTable, "no preintegration table returned");
    LDEBUG("Preintegration table time: " << watch.getRuntime());

    tgt::vec3 npotCorrection = static_cast<tgt::vec3>(octree->getOctreeDim()) / static_cast<tgt::vec3>(octree->getDimensions());

    TriangleMeshGeometryVec3* optimizedGeometry = new TriangleMeshGeometryVec3();
    addNodeGeometry(optimizedGeometry, octree->getRootNode(),
        tgt::vec3(0.f), tgt::vec3(1.f)*npotCorrection, clipLlfNorm, clipUrbNorm,
        rwm, preintegrationTable, transfunc->getDomain());

    return optimizedGeometry;
}

void OctreeProxyGeometry::addNodeGeometry(TriangleMeshGeometryVec3* proxyGeometry,
    const VolumeOctreeNode* node, const tgt::vec3& nodeLlf, const tgt::vec3& nodeUrb,
    const tgt::vec3& clipLlf, const tgt::vec3& clipUrb,
    const RealWorldMapping& rwm, const PreIntegrationTable* preintegrationTable, const tgt::vec2& transFuncDomain) const
{
    tgtAssert(proxyGeometry, "null pointer passed");
    tgtAssert(node, "null pointer passed");
    tgtAssert(preintegrationTable, "null pointer passed");

    // transform normalized voxel intensities to real-world
    float minRealWorld = rwm.normalizedToRealWorld(node->getMinValue() / 65535.f);
    float maxRealWorld = rwm.normalizedToRealWorld(node->getMaxValue() / 65535.f);

    // normalize real-world intensities with regard to tf domain
    float minTF = TransFunc::realWorldToNormalized(minRealWorld, transFuncDomain);
    float maxTF = TransFunc::realWorldToNormalized(maxRealWorld, transFuncDomain);
    tgtAssert(minTF <= maxTF, "minTF > maxTF");

    if (tgt::hor(tgt::greaterThanEqual(nodeLlf, clipUrb)) || tgt::hor(tgt::lessThanEqual(nodeUrb, clipLlf))) {
        // node is completely outside clipping range => discard it
    }
    else if (preintegrationTable->classify(minTF, maxTF).a == 0.f) { //< node is completely invisible => discard it
        // nothing
    }
    else if (node->isLeaf()) { //< leaf node is visible => return node proxy geometry
        tgt::vec3 nodeLlfClipped = tgt::max(nodeLlf, clipLlf);
        tgt::vec3 nodeUrbClipped = tgt::min(nodeUrb, clipUrb);
        proxyGeometry->addCube(VertexVec3(nodeLlfClipped, nodeLlfClipped), VertexVec3(nodeUrbClipped, nodeUrbClipped));
    }
    else { // construct node geometry from visible child nodes
        tgt::vec3 nodeHalfDim = (nodeUrb-nodeLlf) / 2.f;
        for (size_t childID=0; childID<8; childID++) {
            const VolumeOctreeNode* childNode = node->children_[childID];
            if (childNode) {
                tgt::svec3 childCoord = linearCoordToCubic(childID, tgt::svec3::two);
                tgt::vec3 childLlf = nodeLlf + static_cast<tgt::vec3>(childCoord)*nodeHalfDim;
                tgt::vec3 childUrb = childLlf + nodeHalfDim;
                addNodeGeometry(proxyGeometry, childNode, childLlf, childUrb,
                    clipLlf, clipUrb,
                    rwm, preintegrationTable, transFuncDomain);
            }
        }
    }
}

void OctreeProxyGeometry::onClipRightChange() {
    if (clipRight_.get() > clipLeft_.get())
        clipLeft_.set(clipRight_.get());
}

void OctreeProxyGeometry::onClipLeftChange() {
    if (clipRight_.get() > clipLeft_.get())
        clipRight_.set(clipLeft_.get());
}

void OctreeProxyGeometry::onClipFrontChange() {
    if (clipFront_.get() > clipBack_.get())
        clipBack_.set(clipFront_.get());
}

void OctreeProxyGeometry::onClipBackChange() {
    if (clipFront_.get() > clipBack_.get())
        clipFront_.set(clipBack_.get());
}

void OctreeProxyGeometry::onClipBottomChange() {
    if (clipBottom_.get() > clipTop_.get())
        clipTop_.set(clipBottom_.get());
}

void OctreeProxyGeometry::onClipTopChange() {
    if (clipBottom_.get() > clipTop_.get())
        clipBottom_.set(clipTop_.get());
}

void OctreeProxyGeometry::resetClipPlanes() {

    clipRight_.set(0.0f);
    clipLeft_.set(clipLeft_.getMaxValue());

    clipFront_.set(0.0f);
    clipBack_.set(clipBack_.getMaxValue());

    clipBottom_.set(0.0f);
    clipTop_.set(clipTop_.getMaxValue());
}

void OctreeProxyGeometry::adjustClipPropertiesRanges() {
    tgtAssert(inport_.getData(), "No input octree");

    if (oldVolumeDimensions_ == tgt::ivec3(0,0,0))
        oldVolumeDimensions_ = inport_.getData()->getDimensions();

    tgt::ivec3 numSlices = inport_.getData()->getDimensions();

    // assign new clipping values while taking care that the right>left validation
    // does not alter the assigned values
    float scaleRight = tgt::clamp(clipRight_.get()/static_cast<float>(oldVolumeDimensions_.x-1), 0.f, 1.f);
    float scaleLeft =  tgt::clamp(clipLeft_.get()/static_cast<float>(oldVolumeDimensions_.x-1), 0.f, 1.f);
    float rightVal = scaleRight * (numSlices.x-1);
    float leftVal = scaleLeft * (numSlices.x-1);
    // set new max values now (we cannot set them earlier as they might clip the previous values needed for the relative re-positioning to the new range)
    clipRight_.setMaxValue(numSlices.x-1.0f);
    clipLeft_.setMaxValue(numSlices.x-1.0f);
    clipLeft_.set(clipLeft_.getMaxValue());
    clipRight_.set(rightVal);
    clipLeft_.set(leftVal);

    float scaleFront = tgt::clamp(clipFront_.get()/static_cast<float>(oldVolumeDimensions_.y-1), 0.f, 1.f);
    float scaleBack =  tgt::clamp(clipBack_.get()/static_cast<float>(oldVolumeDimensions_.y-1), 0.f, 1.f);
    float frontVal = scaleFront * (numSlices.y-1);
    float backVal = scaleBack * (numSlices.y-1);
    clipFront_.setMaxValue(numSlices.y-1.0f);
    clipBack_.setMaxValue(numSlices.y-1.0f);
    clipBack_.set(clipBack_.getMaxValue());
    clipFront_.set(frontVal);
    clipBack_.set(backVal);

    float scaleBottom = tgt::clamp(clipBottom_.get()/static_cast<float>(oldVolumeDimensions_.z-1), 0.f, 1.f);
    float scaleTop =  tgt::clamp(clipTop_.get()/static_cast<float>(oldVolumeDimensions_.z-1), 0.f, 1.f);
    float bottomVal = scaleBottom * (numSlices.z-1);
    float topVal = scaleTop * (numSlices.z-1);
    clipBottom_.setMaxValue(numSlices.z-1.0f);
    clipTop_.setMaxValue(numSlices.z-1.0f);
    clipTop_.set(clipTop_.getMaxValue());
    clipBottom_.set(bottomVal);
    clipTop_.set(topVal);

    if (clipRight_.get() > clipRight_.getMaxValue())
        clipRight_.set(clipRight_.getMaxValue());

    if (clipLeft_.get() > clipLeft_.getMaxValue())
        clipLeft_.set(clipLeft_.getMaxValue());

    if (clipFront_.get() > clipFront_.getMaxValue())
        clipFront_.set(clipFront_.getMaxValue());

    if (clipBack_.get() > clipBack_.getMaxValue())
        clipBack_.set(clipBack_.getMaxValue());

    if (clipBottom_.get() > clipBottom_.getMaxValue())
        clipBottom_.set(clipBottom_.getMaxValue());

    if (clipTop_.get() > clipTop_.getMaxValue())
        clipTop_.set(clipTop_.getMaxValue());

    oldVolumeDimensions_ = numSlices;
}

void OctreeProxyGeometry::adjustClipPropertiesVisibility() {
    bool clipEnabled = enableClipping_.get();
    setPropertyGroupVisible("clipping", clipEnabled);
}

void OctreeProxyGeometry::adjustClippingToVolumeROI() {
    // adjust clipping sliders to volume ROI, if set
    if (inport_.getData()->hasMetaData("RoiPixelOffset") && inport_.getData()->hasMetaData("RoiPixelLength")) {
        const tgt::ivec3 volumeDim = static_cast<tgt::ivec3>(inport_.getData()->getDimensions());
        tgt::ivec3 roiLlf = inport_.getData()->getMetaDataValue<IVec3MetaData>("RoiPixelOffset", tgt::ivec3(0));
        tgt::ivec3 roiLength = inport_.getData()->getMetaDataValue<IVec3MetaData>("RoiPixelLength", volumeDim);
        if (tgt::hor(tgt::lessThan(roiLlf, tgt::ivec3::zero)) || tgt::hor(tgt::greaterThanEqual(roiLlf, volumeDim))) {
            LWARNING("Invalid ROI offset: " << roiLlf);
        }
        else if (tgt::hor(tgt::lessThanEqual(roiLength, tgt::ivec3::zero))) {
            LWARNING("Invalid ROI length: " << roiLength);
        }
        else {
            tgt::ivec3 roiUrb = tgt::min(roiLlf + roiLength - 1, volumeDim - tgt::ivec3::one);

            LINFO("Applying volume ROI: llf=" << roiLlf << ", urb=" << roiUrb);

            clipRight_.set(static_cast<float>(roiLlf.x));
            clipFront_.set(static_cast<float>(roiLlf.y));
            clipBottom_.set(static_cast<float>(roiLlf.z));

            clipLeft_.set(static_cast<float>(roiUrb.x));
            clipBack_.set(static_cast<float>(roiUrb.y));
            clipTop_.set(static_cast<float>(roiUrb.z));
        }
    }
}

} // namespace
