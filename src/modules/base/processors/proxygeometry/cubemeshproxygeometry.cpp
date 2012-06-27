/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/modules/base/processors/proxygeometry/cubemeshproxygeometry.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"

namespace voreen {

const std::string CubeMeshProxyGeometry::loggerCat_("voreen.CubeMeshProxyGeometry");

CubeMeshProxyGeometry::CubeMeshProxyGeometry()
    : Processor()
    , inport_(Port::INPORT, "volumehandle.volumehandle")
    , outport_(Port::OUTPORT, "proxygeometry.geometry")
    , enableClipping_("useClipping", "Enable Clipping", true)
    , clipRight_("rightClippingPlane", "Right clipping plane (x)", 0.0f, 0.0f, 100000.0f)
    , clipLeft_("leftClippingPlane", "Left clipping plane (x)", 0.0f, 0.0f, 100000.0f)
    , clipFront_("frontClippingPlane", "Front clipping plane (y)", 0.0f, 0.0f, 100000.0f)
    , clipBack_("backClippingPlane", "Back clipping plane (y)", 0.0f, 0.0f, 100000.0f)
    , clipBottom_("bottomClippingPlane", "Bottom clipping plane (z)", 0.0f, 0.0f, 100000.0f)
    , clipTop_("topClippingPlane", "Top clipping plane (z)", 0.0f, 0.0f, 100000.0f)
    , resetClipPlanes_("resetClipPlanes", "Reset Planes")
    , geometry_(new MeshListGeometry())
{

    addPort(inport_);
    addPort(outport_);

    clipRight_.onChange(CallMemberAction<CubeMeshProxyGeometry>(this, &CubeMeshProxyGeometry::onClipRightChange));
    clipLeft_.onChange(CallMemberAction<CubeMeshProxyGeometry>(this, &CubeMeshProxyGeometry::onClipLeftChange));
    clipFront_.onChange(CallMemberAction<CubeMeshProxyGeometry>(this, &CubeMeshProxyGeometry::onClipFrontChange));
    clipBack_.onChange(CallMemberAction<CubeMeshProxyGeometry>(this, &CubeMeshProxyGeometry::onClipBackChange));
    clipBottom_.onChange(CallMemberAction<CubeMeshProxyGeometry>(this, &CubeMeshProxyGeometry::onClipBottomChange));
    clipTop_.onChange(CallMemberAction<CubeMeshProxyGeometry>(this, &CubeMeshProxyGeometry::onClipTopChange));
    enableClipping_.onChange(CallMemberAction<CubeMeshProxyGeometry>(this, &CubeMeshProxyGeometry::adjustClipPropertiesVisibility));
    resetClipPlanes_.onChange(CallMemberAction<CubeMeshProxyGeometry>(this, &CubeMeshProxyGeometry::resetClipPlanes));

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

CubeMeshProxyGeometry::~CubeMeshProxyGeometry() {
    delete geometry_;
    geometry_ = 0;
}

std::string CubeMeshProxyGeometry::getProcessorInfo() const {
    return "Provides a mesh representing a cubic proxy geometry that can be passed to a MeshEntryExitPoints processor. "
           "The proxy geometry can be manipulated by axis-aligned clipping. "
           "Clipping against an arbitrarily oriented plane is provided by the MeshClipping processor.";
}

Processor* CubeMeshProxyGeometry::create() const {
    return new CubeMeshProxyGeometry();
}

void CubeMeshProxyGeometry::process() {
    tgtAssert(inport_.getData()->getVolume(), "no input volume");

    Volume* inputVolume = inport_.getData()->getVolume();
    tgt::vec3 volumeSize = inputVolume->getCubeSize();
    tgt::ivec3 numSlices = inputVolume->getDimensions();
    if (oldVolumeDimensions_ == tgt::ivec3(0,0,0))
        oldVolumeDimensions_ = inputVolume->getDimensions();

    // adapt clipping plane properties on volume change
    if (inport_.hasChanged()) {
        adjustClipPropertiesRanges();
    }

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
    geometry_->clear();
    geometry_->addMesh(MeshGeometry::createCube(coordLlf, coordUrb, texLlf, texUrb, texLlf, texUrb));
    geometry_->transform(inputVolume->getTransformation());

    outport_.setData(geometry_);
}

void CubeMeshProxyGeometry::onClipRightChange() {
    if (clipRight_.get() > clipLeft_.get())
        clipLeft_.set(clipRight_.get());
}

void CubeMeshProxyGeometry::onClipLeftChange() {
    if (clipRight_.get() > clipLeft_.get())
        clipRight_.set(clipLeft_.get());
}

void CubeMeshProxyGeometry::onClipFrontChange() {
    if (clipFront_.get() > clipBack_.get())
        clipBack_.set(clipFront_.get());
}

void CubeMeshProxyGeometry::onClipBackChange() {
    if (clipFront_.get() > clipBack_.get())
        clipFront_.set(clipBack_.get());
}

void CubeMeshProxyGeometry::onClipBottomChange() {
    if (clipBottom_.get() > clipTop_.get())
        clipTop_.set(clipBottom_.get());
}

void CubeMeshProxyGeometry::onClipTopChange() {
    if (clipBottom_.get() > clipTop_.get())
        clipBottom_.set(clipTop_.get());
}

void CubeMeshProxyGeometry::resetClipPlanes() {

    clipRight_.set(0.0f);
    clipLeft_.set(clipLeft_.getMaxValue());

    clipFront_.set(0.0f);
    clipBack_.set(clipBack_.getMaxValue());

    clipBottom_.set(0.0f);
    clipTop_.set(clipTop_.getMaxValue());
}

void CubeMeshProxyGeometry::adjustClipPropertiesRanges() {
    tgtAssert(inport_.getData() && inport_.getData()->getVolume(), "No input volume");
    tgt::ivec3 numSlices = inport_.getData()->getVolume()->getDimensions();

    // adapt clipping plane properties to volume dimensions
    clipRight_.setMaxValue(numSlices.x-1.0f);
    clipLeft_.setMaxValue(numSlices.x-1.0f);

    clipFront_.setMaxValue(numSlices.y-1.0f);
    clipBack_.setMaxValue(numSlices.y-1.0f);

    clipBottom_.setMaxValue(numSlices.z-1.0f);
    clipTop_.setMaxValue(numSlices.z-1.0f);

    // assign new clipping values while taking care that the right>left validation
    // does not alter the assigned values
    float rightVal = clipRight_.get()/static_cast<float>(oldVolumeDimensions_.x-1) * (numSlices.x-1);
    float leftVal = clipLeft_.get()/static_cast<float>(oldVolumeDimensions_.x-1) * (numSlices.x-1);
    clipLeft_.set(clipLeft_.getMaxValue());
    clipRight_.set(rightVal);
    clipLeft_.set(leftVal);

    float frontVal = clipFront_.get()/static_cast<float>(oldVolumeDimensions_.y-1) * (numSlices.y-1);
    float backVal = clipBack_.get()/static_cast<float>(oldVolumeDimensions_.y-1) * (numSlices.y-1);
    clipBack_.set(clipBack_.getMaxValue());
    clipFront_.set(frontVal);
    clipBack_.set(backVal);

    float bottomVal = clipBottom_.get()/static_cast<float>(oldVolumeDimensions_.z-1) * (numSlices.z-1);
    float topVal = clipTop_.get()/static_cast<float>(oldVolumeDimensions_.z-1) * (numSlices.z-1);
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

void CubeMeshProxyGeometry::adjustClipPropertiesVisibility() {
    bool clipEnabled = enableClipping_.get();
    /*clipRight_.setVisible(clipEnabled);
    clipLeft_.setVisible(clipEnabled);
    clipFront_.setVisible(clipEnabled);
    clipBack_.setVisible(clipEnabled);
    clipBottom_.setVisible(clipEnabled);
    clipTop_.setVisible(clipEnabled);
    resetClipPlanes_.setVisible(clipEnabled);*/
    setPropertyGroupVisible("clipping", clipEnabled);
}


} // namespace
