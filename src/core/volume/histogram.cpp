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

#include "voreen/core/volume/histogram.h"
#include "voreen/core/volume/bricking/brickedvolume.h"

using namespace tgt;

using tgt::ivec2;

namespace voreen {

HistogramIntensity::HistogramIntensity(Volume* volume, int bucketCount) {

    tgtAssert(volume, "HistogramIntensity: No volume");
    tgtAssert(bucketCount > 0, "HistogramIntensity: Invalid bucket count");

    for (int i=0; i < bucketCount; ++i)
        hist_.push_back(0);

    maxValue_ = 0;
    significantRange_ = ivec2(bucketCount, -1);

    Volume* currentVolume = volume;


    BrickedVolume* brickedVolume = dynamic_cast<BrickedVolume*>(volume);
    if (brickedVolume) {
        currentVolume = brickedVolume->getPackedVolume();
    }

    VolumeUInt8* sourceDataset8Bit = dynamic_cast<VolumeUInt8*>(currentVolume);
    if (sourceDataset8Bit) {
        int bucket;
        float m = (bucketCount - 1.f) / 255.f;

        for (size_t i=0; i < sourceDataset8Bit->getNumVoxels(); ++i) {
            bucket = static_cast<int>(floor(sourceDataset8Bit->voxel(i) * m));
            int value = hist_[bucket] + 1;
            hist_[bucket] = value;

            if (value > maxValue_)
                maxValue_ = value;

            if (bucket < significantRange_.x)
                significantRange_.x = bucket;
            if (bucket > significantRange_.y)
                significantRange_.y = bucket;
        }
    }

    Volume4xUInt8* sourceDataset32Bit = dynamic_cast<Volume4xUInt8*>(currentVolume);
    if (sourceDataset32Bit) {
        int bucket;
        float m = (bucketCount - 1.f) / 255.f;

        for (size_t i=0; i < sourceDataset32Bit->getNumVoxels(); ++i) {
            bucket = static_cast<int>(floor(sourceDataset32Bit->voxel(i)[3] * m));
            int value = hist_[bucket] + 1;
            hist_[bucket] = value;

            if (value > maxValue_)
                maxValue_ = value;

            if (bucket < significantRange_.x)
                significantRange_.x = bucket;
            if (bucket > significantRange_.y)
                significantRange_.y = bucket;
        }
    }

    VolumeUInt16* sourceDataset16Bit = dynamic_cast<VolumeUInt16*>(currentVolume);
    if (sourceDataset16Bit) {
        int bucket;
        float maxValue = (sourceDataset16Bit->getBitsStored() == 12) ? 4095.f : 65535.f;
        float m = (bucketCount - 1.f) / maxValue;

        for (size_t i=0; i < sourceDataset16Bit->getNumVoxels(); ++i) {
            bucket = static_cast<int>(floor(sourceDataset16Bit->voxel(i) * m));
            if (bucket < static_cast<int>(hist_.size())) {
                int value = hist_[bucket] + 1;
                hist_[bucket] = value;

                if (value > maxValue_)
                    maxValue_ = value;

                if (bucket < significantRange_.x)
                    significantRange_.x = bucket;
                if (bucket > significantRange_.y)
                    significantRange_.y = bucket;
            }
        }
    }
}

size_t HistogramIntensity::getBucketCount() {
    return hist_.size();
}

int HistogramIntensity::getValue(int i) {
    return hist_[i];
}

float HistogramIntensity::getNormalized(int i) {
    return (static_cast<float>(hist_[i]) / static_cast<float>(maxValue_));
}

float HistogramIntensity::getLogNormalized(int i) {
     return (logf(static_cast<float>(1+hist_[i]) ) / log( static_cast<float>(1+maxValue_)));
}

tgt::ivec2 HistogramIntensity::getSignificantRange() const {
    return significantRange_;
}


//-----------------------------------------------------------------------------

HistogramIntensityGradient::HistogramIntensityGradient(Volume* volumeGrad, Volume* volumeIntensity,
                                                       int bucketCounti, int bucketCountg, bool scale)
    : scaleFactor_(1.f)
{
    if (dynamic_cast<Volume3xUInt8*>(volumeGrad)) {
        calcHG(static_cast<Volume3xUInt8*>(volumeGrad), volumeIntensity, bucketCounti, bucketCountg, scale);
    }
    else if (dynamic_cast<Volume4xUInt8*>(volumeGrad)) {
        calcHG(static_cast<Volume4xUInt8*>(volumeGrad), volumeIntensity, bucketCounti, bucketCountg, scale);
    }
    else if (dynamic_cast<Volume3xUInt16*>(volumeGrad)) {
        calcHG(static_cast<Volume3xUInt16*>(volumeGrad), volumeIntensity, bucketCounti, bucketCountg, scale);
    }
    else if (dynamic_cast<Volume4xUInt16*>(volumeGrad)) {
        calcHG(static_cast<Volume4xUInt16*>(volumeGrad), volumeIntensity, bucketCounti, bucketCountg, scale);
    }
    else {
        LERRORC("voreen.HistogramIntensityGradient",
                "HistogramIntensityGradient needs 24 or 32 bit DS as input!");
    }
}

size_t HistogramIntensityGradient::getBucketCountIntensity() const {
    return hist_.size();
}

size_t HistogramIntensityGradient::getBucketCountGradient() const {
    return hist_[0].size();
}

int HistogramIntensityGradient::getValue(int i, int g) const {
    return hist_[i][g];
}

float HistogramIntensityGradient::getNormalized(int i, int g) const {
    return (static_cast<float>(hist_[i][g]) / static_cast<float>(maxValue_));
}

float HistogramIntensityGradient::getLogNormalized(int i, int g) const {
    return (logf(static_cast<float>(1+hist_[i][g]) ) / log(static_cast<float>(1+maxValue_)));
}

int HistogramIntensityGradient::getMaxValue() const {
    return maxValue_;
}

tgt::ivec2 HistogramIntensityGradient::getSignificantRangeIntensity() const {
    return significantRangeIntensity_;
}

tgt::ivec2 HistogramIntensityGradient::getSignificantRangeGradient() const {
    return significantRangeGradient_;
}

float HistogramIntensityGradient::getScaleFactor() const {
    return scaleFactor_;
}

} // namespace voreen
