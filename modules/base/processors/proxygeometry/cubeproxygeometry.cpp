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

#include "cubeproxygeometry.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"
#include "voreen/core/datastructures/geometry/trianglemeshgeometry.h"

namespace voreen {

const std::string CubeProxyGeometry::loggerCat_("voreen.base.CubeProxyGeometry");

CubeProxyGeometry::CubeProxyGeometry()
    : Processor()
    , inport_(Port::INPORT, "volumehandle.volumehandle", "Volume Input")
    , outport_(Port::OUTPORT, "proxygeometry.geometry", "Proxy Geometry Output")
    , enableClipping_("useClipping", "Enable Clipping", true)
    , clipRight_("rightClippingPlane", "Right Clip Plane (x)", 0.f, 0.f, 1e5f)
    , clipLeft_("leftClippingPlane", "Left Clip Plane (x)", 1e5f, 0.f, 1e5f)
    , clipFront_("frontClippingPlane", "Front Clip Plane (y)", 0.f, 0.f, 1e5f)
    , clipBack_("backClippingPlane", "Back Clip Plane (y)", 1e5f, 0.f, 1e5f)
    , clipBottom_("bottomClippingPlane", "Bottom Clip Plane (z)", 0.f, 0.f, 1e5f)
    , clipTop_("topClippingPlane", "Top Clip Plane (z)", 1e5f, 0.f, 1e5f)
    , resetClipPlanes_("resetClipPlanes", "Reset Planes")
{

    addPort(inport_);
    addPort(outport_);

    clipRight_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::onClipRightChange));
    clipLeft_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::onClipLeftChange));
    clipFront_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::onClipFrontChange));
    clipBack_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::onClipBackChange));
    clipBottom_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::onClipBottomChange));
    clipTop_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::onClipTopChange));
    enableClipping_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::adjustClipPropertiesVisibility));
    resetClipPlanes_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::resetClipPlanes));

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

    oldVolumeDimensions_ = tgt::ivec3(0,0,0);
}

CubeProxyGeometry::~CubeProxyGeometry() {
}

Processor* CubeProxyGeometry::create() const {
    return new CubeProxyGeometry();
}

void CubeProxyGeometry::process() {
    tgtAssert(inport_.getData(), "no input volume");

    // adapt clipping plane properties on volume change
    if (inport_.hasChanged()) {
        adjustClipPropertiesRanges();
        adjustClippingToVolumeROI();
    }

    const VolumeBase* inputVolume = inport_.getData();
    tgt::vec3 volumeSize = inputVolume->getCubeSize();
    tgt::ivec3 numSlices = inputVolume->getDimensions();

    // vertex and tex coords of bounding box without clipping
    tgt::vec3 coordLlf = inputVolume->getLLF();
    tgt::vec3 coordUrb = inputVolume->getURB();
    const tgt::vec3 noClippingTexLlf(0, 0, 0);
    const tgt::vec3 noClippingTexUrb(1, 1, 1);

    tgt::vec3 texLlf;
    tgt::vec3 texUrb;
    if (enableClipping_.get()) {
        // adjust vertex and tex coords to clipping
        texLlf = tgt::vec3(
            clipRight_.get()  / static_cast<float>(numSlices.x),
            clipFront_.get()  / static_cast<float>(numSlices.y),
            clipBottom_.get() / static_cast<float>(numSlices.z));
        texUrb = tgt::vec3(
            (clipLeft_.get()+1.0f) / static_cast<float>(numSlices.x),
            (clipBack_.get()+1.0f) / static_cast<float>(numSlices.y),
            (clipTop_.get()+1.0f)  / static_cast<float>(numSlices.z));

        coordLlf -= volumeSize * (noClippingTexLlf - texLlf);
        coordUrb -= volumeSize * (noClippingTexUrb - texUrb);
    }
    else {
        texLlf = noClippingTexLlf;
        texUrb = noClippingTexUrb;
    }

    // create output mesh
    //TriangleMeshGeometryVec3* mesh = TriangleMeshGeometryVec3::createCube(VertexVec3(texLlf, texLlf), VertexVec3(texUrb, texUrb));
    TriangleMeshGeometryVec4Vec3* mesh = TriangleMeshGeometryVec4Vec3::createCube(texLlf, texUrb, texLlf, texUrb, 1.0f);
    mesh->transform(inputVolume->getTextureToWorldMatrix());

    outport_.setData(mesh);
}

void CubeProxyGeometry::onClipRightChange() {
    if (clipRight_.get() > clipLeft_.get())
        clipLeft_.set(clipRight_.get());
}

void CubeProxyGeometry::onClipLeftChange() {
    if (clipRight_.get() > clipLeft_.get())
        clipRight_.set(clipLeft_.get());
}

void CubeProxyGeometry::onClipFrontChange() {
    if (clipFront_.get() > clipBack_.get())
        clipBack_.set(clipFront_.get());
}

void CubeProxyGeometry::onClipBackChange() {
    if (clipFront_.get() > clipBack_.get())
        clipFront_.set(clipBack_.get());
}

void CubeProxyGeometry::onClipBottomChange() {
    if (clipBottom_.get() > clipTop_.get())
        clipTop_.set(clipBottom_.get());
}

void CubeProxyGeometry::onClipTopChange() {
    if (clipBottom_.get() > clipTop_.get())
        clipBottom_.set(clipTop_.get());
}

void CubeProxyGeometry::resetClipPlanes() {

    clipRight_.set(0.0f);
    clipLeft_.set(clipLeft_.getMaxValue());

    clipFront_.set(0.0f);
    clipBack_.set(clipBack_.getMaxValue());

    clipBottom_.set(0.0f);
    clipTop_.set(clipTop_.getMaxValue());
}

void CubeProxyGeometry::adjustClipPropertiesRanges() {
    tgtAssert(inport_.getData(), "No input volume");

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

void CubeProxyGeometry::adjustClipPropertiesVisibility() {
    bool clipEnabled = enableClipping_.get();
    setPropertyGroupVisible("clipping", clipEnabled);
}

void CubeProxyGeometry::adjustClippingToVolumeROI() {
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
