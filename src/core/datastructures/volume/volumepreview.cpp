/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/core/datastructures/volume/volumepreview.h"

namespace voreen {

VolumePreview::VolumePreview()
    : VolumeDerivedData()
    , height_(0)
{}

VolumePreview::VolumePreview(int height, const std::vector<unsigned char>& data)
    : VolumeDerivedData()
    , height_(height)
    , prevData_(data)
{}

VolumeDerivedData* VolumePreview::createFrom(const VolumeBase* handle) const {
    tgtAssert(handle, "no volume handle");

    const VolumeRAM* volume = handle->getRepresentation<VolumeRAM>();
    tgtAssert(volume, "no volume");

    int internHeight = 64;

    // gamma correction factor (amplifies low gray values)
    const float GAMMA = 1.8f;

    float xSpacing = handle->getSpacing()[0];
    float ySpacing = handle->getSpacing()[1];
    float xDimension = static_cast<float>(volume->getDimensions()[0]);
    float yDimension = static_cast<float>(volume->getDimensions()[1]);

    // determine offsets and scale factors for non-uniform aspect ratios
    float aspectRatio = (yDimension * ySpacing) / (xDimension * xSpacing);
    float xOffset, yOffset, xScale, yScale;
    if (aspectRatio <= 1.f) {
        xOffset = 0.f;
        yOffset = ((internHeight - 1.f) * (1.f - aspectRatio)) / 2.f;
        xScale = 1.f;
        yScale = 1.f / aspectRatio;
    }
    else {
        xOffset = ((internHeight -1.f) * (1.f - 1.f/aspectRatio)) / 2.f;
        yOffset = 0.f;
        xScale = aspectRatio;
        yScale = 1.f;
    }

    float maxVal, minVal;
    int numTries = 1;
    int maxTries = 1 << static_cast<int>(std::log(static_cast<float>(volume->getDimensions().z)) / std::log(2.f));
    std::vector<float> prevData = std::vector<float>(internHeight * internHeight);

    do {
        prevData = std::vector<float>(internHeight * internHeight);
        tgt::vec3 position;
        int exp2 = 1 << static_cast<int>(std::log(static_cast<float>(numTries)) / std::log(2.f));
        int offset = static_cast<int>(volume->getDimensions().z) / exp2;
        position.z = static_cast<float>(std::min(offset * (numTries - exp2) + offset / 2, static_cast<int>(volume->getDimensions().z - 1)));

        // generate preview in float buffer
        minVal = volume->elementRange().y;
        maxVal = volume->elementRange().x;
        for (int y = 0; y < internHeight; y++){
            for (int x = 0; x < internHeight; x++){
                position.x = ((x-xOffset) / (internHeight -1)) * xScale * (xDimension-1.f);
                position.y = ((y-yOffset) / (internHeight -1)) * yScale * (yDimension-1.f);
                int previewIndex = y*internHeight + x;
                float val = 0.f;
                if (position.x >= 0 && position.y >= 0 && position.x < xDimension && position.y < yDimension)
                    val = volume->getVoxelNormalizedLinear(position);
                prevData[previewIndex] = val;
                minVal = std::min(minVal, val);
                maxVal = std::max(maxVal, val);
            }
        }
        numTries++;
    } while (maxVal < 0.1f && numTries <= maxTries);

    float valOffset = minVal;
    float valScale = maxVal - minVal;
    if (valScale > 0.f) {
        for (int y=0; y<internHeight; y++) {
            for (int x=0; x<internHeight; x++) {
                int previewIndex = y*internHeight+ x;
                prevData[previewIndex] = pow((prevData[previewIndex] - valOffset)  / valScale, 1.f / GAMMA);
            }
        }
    }

    std::vector<unsigned char> prevDataUInt8 = std::vector<unsigned char>(internHeight * internHeight);
    for (size_t i = 0; i < prevDataUInt8.size(); i++)
        prevDataUInt8[i] = tgt::iround(prevData[i] * 255.f);

    return new VolumePreview(internHeight, prevDataUInt8);
}

void VolumePreview::serialize(XmlSerializer& s) const  {
    s.serialize("height", height_);
    s.serializeBinaryBlob("prevData", prevData_);
}

void VolumePreview::deserialize(XmlDeserializer& s) {
    s.deserialize("height", height_);
    s.deserializeBinaryBlob("prevData", prevData_);
}

} // namespace voreen

