/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/volume/volumemetadata.h"

#include "tgt/filesystem.h"

namespace voreen {

/*
 * constructor
 */

VolumeMetaData::VolumeMetaData(const std::string& str)
  : string_(str)
  , imagePositionZ_(0.f)
  , brickSize_(0)
{}

/*
 * getters and setters
 */

const std::string& VolumeMetaData::getString() const {
    return string_;
}

void VolumeMetaData::setString(const std::string& str) {
    string_ = str;
}

float VolumeMetaData::getImagePositionZ() const {
    return imagePositionZ_;
}

void VolumeMetaData::setImagePositionZ(float pos) {
    imagePositionZ_ = pos;
}

void VolumeMetaData::setBrickSize(size_t bricksize) {
    brickSize_ = bricksize;
}

size_t VolumeMetaData::getBrickSize() {
    return brickSize_;
}

void VolumeMetaData::setUnit(const std::string& unit) {
    unit_ = unit;
}

const std::string& VolumeMetaData::getUnit() const {
    return unit_;
}

void VolumeMetaData::setParentVolumeDimensions(tgt::ivec3 dimensions) {
    parentVolumeDimensions_ = dimensions;
}

tgt::ivec3 VolumeMetaData::getParentVolumeDimensions() {
    return parentVolumeDimensions_;
}

} // namespace voreen
