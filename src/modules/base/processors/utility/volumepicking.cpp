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

#include "voreen/modules/base/processors/utility/volumepicking.h"

namespace voreen {

const std::string VolumePicking::loggerCat_("voreen.VolumePicking");

VolumePicking::VolumePicking()
    : RenderProcessor(),
      inportFirstHit_(Port::INPORT, "inport.firsthit"),
      inportVolume_(Port::INPORT, "inport.volume"),
      outportPicked_(Port::OUTPORT, "outport.picked"),
      outCoordinateSystem_("outCoordinateSystem", "Output Coordinate System", INVALID_PARAMETERS),
      clearLast_("clearLast", "Clear Last"),
      clearAll_("clearAll", "Clear All"),
      numPicked_("numPicked", "Number of Points", 0, 0, 1000, Processor::VALID),
      lastPicked_("lastPicked", "Last Picked", tgt::vec3(0.f), tgt::vec3(-10000.f), tgt::vec3(10000.f),
                  Processor::VALID),
      pickEvent_("pickEvent", "Picking", this, &VolumePicking::mouseEvent, tgt::MouseEvent::MOUSE_BUTTON_LEFT,
                 tgt::MouseEvent::PRESSED, tgt::Event::CTRL),
      transformedPositions_(0),
      lastMousePosition_(tgt::vec2(-1))
{
    addPort(inportFirstHit_);
    addPort(inportVolume_);
    addPort(outportPicked_);

    outCoordinateSystem_.addOption("voxel",     "Voxel Coordinates");
    outCoordinateSystem_.addOption("volume",    "Volume Coordinates");
    outCoordinateSystem_.addOption("world",     "World Coordinates");
    outCoordinateSystem_.addOption("texture",   "Texture Coordinates");
    outCoordinateSystem_.select("voxel");

    clearLast_.onClick(CallMemberAction<VolumePicking>(this, &VolumePicking::clearLast));
    clearAll_.onClick(CallMemberAction<VolumePicking>(this, &VolumePicking::clearAll));
    numPicked_.setWidgetsEnabled(false);
    lastPicked_.setWidgetsEnabled(false);

    addProperty(outCoordinateSystem_);
    addProperty(numPicked_);
    addProperty(lastPicked_);
    addProperty(clearLast_);
    addProperty(clearAll_);
    addEventProperty(pickEvent_);
}

Processor* VolumePicking::create() const {
    return new VolumePicking();
}

std::string VolumePicking::getProcessorInfo() const {
    return "Converts 2D mouse coordinates into a selectable volume coordinate system. "
           "The picked 3D positions are put out as PointListGeometry. Additionally, the last picked "
           "position is provided through a property. The RenderPort needs to be connected to a "
           "first-hit rendering of the picked volume (currently expecting normalized texture coordinates).";
}

void VolumePicking::process() {

    if (lastMousePosition_ != tgt::ivec2(-1)) {
        tgt::vec3 pickedPos = getPickedPosition(lastMousePosition_);
        pickedPositions_.push_back(pickedPos);
        LINFO("Picked pos: " << pickedPos);
        lastMousePosition_ = tgt::ivec2(-1);
    }

    delete transformedPositions_;
    transformedPositions_ = getTransformedPositions();
    outportPicked_.setData(transformedPositions_);

    updateProperties();
}

void VolumePicking::initialize() throw (VoreenException) {
    RenderProcessor::initialize();
    transformedPositions_ = new PointListGeometryVec3();
}

void VolumePicking::deinitialize() throw (VoreenException) {
    outportPicked_.setData(0);
    delete transformedPositions_;
    transformedPositions_ = 0;

    RenderProcessor::deinitialize();
}

void VolumePicking::invalidate(int inv) {
    if (inv == INVALID_RESULT)
        return;

    RenderProcessor::invalidate(inv);
}

void VolumePicking::serialize(XmlSerializer& s) const  {
    RenderProcessor::serialize(s);

    s.serialize("pickedPositions", pickedPositions_);
}

void VolumePicking::deserialize(XmlDeserializer& d) {
    RenderProcessor::deserialize(d);

    try {
        d.deserialize("pickedPositions", pickedPositions_);
    }
    catch (XmlSerializationNoSuchDataException&) {
        d.removeLastError();
    }
}

tgt::vec3 VolumePicking::getPickedPosition(tgt::ivec2 mousePosition) {
    tgtAssert(inportVolume_.hasData() && inportFirstHit_.hasRenderTarget(), "inports not ready");

    tgt::ivec2 dims = inportFirstHit_.getRenderTarget()->getSize();
    if (tgt::hor(tgt::lessThan(mousePosition, tgt::ivec2(0))) || tgt::hor(tgt::greaterThanEqual(mousePosition, dims)) ) {
        LWARNING("Invalid mouse position" << mousePosition);
        return tgt::vec3(-1.f);
    }
    mousePosition.y = dims.y - mousePosition.y;

    // read first hit position from input texture
    tgt::vec3 pickedPos = inportFirstHit_.getRenderTarget()->getColorAtPos(mousePosition).xyz();

    return pickedPos;
}

PointListGeometryVec3* VolumePicking::getTransformedPositions() const {
    tgtAssert(inportVolume_.hasData() && inportFirstHit_.hasRenderTarget(), "inports not ready");

    std::vector<tgt::vec3> transformedPoints = pickedPositions_;

    // voxel coordinates
    if (outCoordinateSystem_.isSelected("voxel")) {
        tgt::vec3 volDims = tgt::vec3(inportVolume_.getData()->getVolume()->getDimensions()-1);
        for (size_t i=0; i<transformedPoints.size(); i++)
            transformedPoints[i] *= volDims;
    }
    // volume coordinates
    else if (outCoordinateSystem_.isSelected("volume")) {
        tgt::vec3 volDims = tgt::vec3(inportVolume_.getData()->getVolume()->getDimensions()-1);
        tgt::vec3 cubeSize = inportVolume_.getData()->getVolume()->getCubeSize();
        for (size_t i=0; i<transformedPoints.size(); i++)
            transformedPoints[i] = (transformedPoints[i] - 0.5f) * cubeSize;
    }
    // world coordinates
    else if (outCoordinateSystem_.isSelected("world")) {
        tgt::vec3 volDims = tgt::vec3(inportVolume_.getData()->getVolume()->getDimensions()-1);
        tgt::mat4 voxelToWorldTrafo = inportVolume_.getData()->getVolume()->getVoxelToWorldMatrix();
        for (size_t i=0; i<transformedPoints.size(); i++) {
            transformedPoints[i] = voxelToWorldTrafo * (transformedPoints[i] * volDims);
        }
    }
    // texture coordinates: [0:1.0]^3 (no transformation necessary)
    else if (outCoordinateSystem_.isSelected("texture")) {
    }

    PointListGeometryVec3* result = new PointListGeometryVec3();
    result->setData(transformedPoints);

    return result;
}

void VolumePicking::mouseEvent(tgt::MouseEvent* e) {
    lastMousePosition_ = e->coord();
    invalidate(INVALID_RESULT+1);
}

void VolumePicking::clearLast() {
    if (pickedPositions_.size() > 0) {
        tgtAssert(transformedPositions_, "no geometry");
        tgtAssert(pickedPositions_.size() == transformedPositions_->getNumPoints(), "point count mis-match");
        pickedPositions_.pop_back();
        transformedPositions_->removeLast();
        updateProperties();
        invalidate(INVALID_RESULT+1);
    }
}

void VolumePicking::clearAll() {
    if (pickedPositions_.size() > 0) {
        tgtAssert(transformedPositions_, "no geometry");
        pickedPositions_.clear();
        transformedPositions_->clear();
        updateProperties();
        invalidate(INVALID_RESULT+1);
    }
}

void VolumePicking::updateProperties() {
    numPicked_.set(pickedPositions_.size());
    if (transformedPositions_->getNumPoints() > 0)
        lastPicked_.set(transformedPositions_->getData().back());
    else
        lastPicked_.set(tgt::vec3(0.f));
}

} // voreen namespace
