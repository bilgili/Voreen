/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

namespace voreen {

/*
 * constructor
 */

VolumeMetaData::VolumeMetaData(const std::string& str)
  : string_(str)
  , fileName_(std::string())
  , imagePositionZ_(0.f)
  , transformation_(tgt::mat4::identity)
  , size_(tgt::ivec2(0))
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

void VolumeMetaData::setFileName(const std::string& fileName) {
    fileName_ = fileName;
}

const std::string& VolumeMetaData::getFileName() const {
    return fileName_;
}

float VolumeMetaData::getImagePositionZ() const {
    return imagePositionZ_;
}

void VolumeMetaData::setImagePositionZ(float pos) {
    imagePositionZ_ = pos;
}

void VolumeMetaData::setTransformation(const tgt::mat4& trans) {
    transformation_ = trans;
}

const tgt::mat4& VolumeMetaData::getTransformation() const {
    return transformation_;
}

void VolumeMetaData::setSize(const tgt::ivec2& size) {
    size_ = size;
}

const tgt::ivec2& VolumeMetaData::getSize() const {
    return size_;
}

/*
 * further methods
 */

std::string VolumeMetaData::getFileNameWithoutPath(const std::string& fullpath) {
    std::string filename = fullpath;
    size_t pos = fullpath.find_last_of('/');

    if (pos != std::string::npos)
        filename = fullpath.substr(pos + 1);
    else {
        pos = fullpath.find_last_of('\\');
        if (pos != std::string::npos)
            filename = fullpath.substr(pos + 1);
    }

    return filename;
}

} // namespace voreen
