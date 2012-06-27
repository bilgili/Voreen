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

#include "voreen/core/vis/meshcolordata.h"

namespace voreen {

MeshColorData::MeshColorData(const std::string caption, tgt::ivec2 dimension)
    : caption_(caption)
    , dimension_(dimension)
{
}

MeshColorData::~MeshColorData() {
    data_.clear();
}

tgt::ivec2 MeshColorData::getDimension() {
    return dimension_;
}

void MeshColorData::setData(std::vector<float> data) {
    data_.clear();
    data_ = data;
}

void MeshColorData::setData(int position, float value) {
    if ((position < 0) || (position > (int)data_.size()))
        return;
    else
        data_[position] = value;
}

const std::vector<float>& MeshColorData::getData() {
    return data_;
}

const std::vector<float> MeshColorData::getNormalizedData() {
    std::vector<float> startvalues = data_;
    //find min and max
    float max = startvalues[0];
    float min = startvalues[0];
    for (size_t i = 0; i < startvalues.size(); ++i) {
        if (startvalues[i] < min)
            min = startvalues[i];
        if (startvalues[i] > max)
            max = startvalues[i];
    }
    //normalize so that values lie between 0 and 1
    std::vector<float> normalized(startvalues.size());
    for (size_t i = 0; i < startvalues.size(); ++i) {
        float intensity = (startvalues[i]-min) / (max-min);
        normalized[i] = intensity;
    }
    return normalized;
}

const std::string MeshColorData::getCaption() {
    return caption_;
}

} //namespace voreen
