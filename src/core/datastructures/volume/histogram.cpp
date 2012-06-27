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

#include "voreen/core/datastructures/volume/histogram.h"
#include "voreen/core/datastructures/volume/bricking/brickedvolume.h"

namespace voreen {

HistogramIntensity::HistogramIntensity(const Volume* volume, int bucketCount) {
    tgtAssert(volume, "HistogramIntensity: No volume");
    tgtAssert(bucketCount > 0, "HistogramIntensity: Invalid bucket count");

    // Limit to 16 bit
    if (bucketCount > 65536)
        bucketCount = 65536;

    // set all buckets to zero
    hist_.assign(bucketCount, 0);

    maxValue_ = 0;
    significantRange_ = tgt::ivec2(bucketCount, -1);

    const Volume* currentVolume = volume;

    const BrickedVolume* brickedVolume = dynamic_cast<const BrickedVolume*>(volume);
    if (brickedVolume) {
        currentVolume = brickedVolume->getPackedVolume();
    }

    const VolumeUInt8* sourceDataset8Bit = dynamic_cast<const VolumeUInt8*>(currentVolume);
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

    const Volume4xUInt8* sourceDataset32Bit = dynamic_cast<const Volume4xUInt8*>(currentVolume);
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

    const VolumeUInt16* sourceDataset16Bit = dynamic_cast<const VolumeUInt16*>(currentVolume);
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

    const VolumeFloat* sourceDatasetFloat = dynamic_cast<const VolumeFloat*>(currentVolume);
    if (sourceDatasetFloat) {
        float m = (bucketCount - 1.f);
        int count = static_cast<int>(hist_.size());

        for (size_t i=0; i < sourceDatasetFloat->getNumVoxels(); ++i) {
            int bucket = static_cast<int>(floor(sourceDatasetFloat->voxel(i) * m));
            if (bucket < count && bucket >= 0) {
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

size_t HistogramIntensity::getBucketCount() const {
    return hist_.size();
}

int HistogramIntensity::getValue(int i) const {
    return hist_[i];
}

int HistogramIntensity::getValue(size_t i) const {
    return getValue(static_cast<int>(i));
}

int HistogramIntensity::getValue(float i) const {
    size_t bucketCount = hist_.size();
    float m = (bucketCount - 1.f);
    int bucket = static_cast<int>(floor(i * m));
    return getValue(bucket);
}

float HistogramIntensity::getNormalized(int i) const {
    return (static_cast<float>(hist_[i]) / static_cast<float>(maxValue_));
}

float HistogramIntensity::getNormalized(float i) const {
    size_t bucketCount = hist_.size();
    float m = (bucketCount - 1.f);
    int bucket = static_cast<int>(floor(i * m));
    return getNormalized(bucket);
}

float HistogramIntensity::getLogNormalized(int i) const {
     return (logf(static_cast<float>(1+hist_[i]) ) / log( static_cast<float>(1+maxValue_)));
}

float HistogramIntensity::getLogNormalized(float i) const {
    size_t bucketCount = hist_.size();
    float m = (bucketCount - 1.f);
    int bucket = static_cast<int>(floor(i * m));
    return getLogNormalized(bucket);
}

tgt::ivec2 HistogramIntensity::getSignificantRange() const {
    return significantRange_;
}


//-----------------------------------------------------------------------------

HistogramIntensityGradient::HistogramIntensityGradient(const Volume* volumeGrad, const Volume* volumeIntensity,
                                                       int bucketCounti, int bucketCountg, bool scale)
    : scaleFactor_(1.f)
{
    if (dynamic_cast<const Volume3xUInt8*>(volumeGrad)) {
        calcHG(static_cast<const Volume3xUInt8*>(volumeGrad), volumeIntensity, bucketCounti, bucketCountg, scale);
    }
    else if (dynamic_cast<const Volume4xUInt8*>(volumeGrad)) {
        calcHG(static_cast<const Volume4xUInt8*>(volumeGrad), volumeIntensity, bucketCounti, bucketCountg, scale);
    }
    else if (dynamic_cast<const Volume3xUInt16*>(volumeGrad)) {
        calcHG(static_cast<const Volume3xUInt16*>(volumeGrad), volumeIntensity, bucketCounti, bucketCountg, scale);
    }
    else if (dynamic_cast<const Volume4xUInt16*>(volumeGrad)) {
        calcHG(static_cast<const Volume4xUInt16*>(volumeGrad), volumeIntensity, bucketCounti, bucketCountg, scale);
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

template<class U>
void HistogramIntensityGradient::calcHG(const VolumeAtomic<U>* volumeGrad, const Volume* volumeIntensity, int bucketCounti, int bucketCountg, bool scale) {
    // bits used for gradients
    int bitsG = volumeGrad->getBitsStored() / volumeGrad->getNumChannels();
    float halfMax = (pow(2.f, bitsG) - 1.f) / 2.f;
    const tgt::ivec3 gradDim = volumeGrad->getDimensions();

    std::vector<float> gradientLengths;
    float maxGradientLength = 0.f;

    // calculate length of all gradients
    for (int z = 0; z < gradDim.z; ++z) {
        for (int y = 0; y < gradDim.y; ++y) {
            for (int x = 0; x < gradDim.x; ++x) {
                float gx = volumeGrad->voxel(x,y,z).r - halfMax;
                float gy = volumeGrad->voxel(x,y,z).g - halfMax;
                float gz = volumeGrad->voxel(x,y,z).b - halfMax;

                float nlength = tgt::length(tgt::vec3(gx, gy, gz));

                if (nlength > maxGradientLength)
                    maxGradientLength = nlength;

                gradientLengths.push_back(nlength);
            }
        }
    }

    // maximum length of a gradient
    float maxLength;
    if (scale) {
        maxLength = maxGradientLength;
        scaleFactor_ = maxLength / (halfMax * sqrt(3.f));
    }
    else
        maxLength = halfMax * sqrt(3.f);

    // init histogram with 0 values
    for (int j = 0; j < bucketCounti; j++)
        hist_.push_back(std::vector<int>(bucketCountg));


    int bucketg, bucketi;
    maxValue_ = 0;
    significantRangeIntensity_ = tgt::ivec2(bucketCounti, -1);
    significantRangeGradient_ = tgt::ivec2(bucketCountg, -1);

    for (int z = 0; z < gradDim.z; ++z) {
        for (int y = 0; y < gradDim.y; ++y) {
            for (int x = 0; x < gradDim.x; ++x) {
                float intensity;
                if (volumeIntensity)
                    intensity = volumeIntensity->getVoxelFloat(x,y,z);
                else
                    intensity = volumeGrad->getVoxelFloat(x,y,z,volumeGrad->getNumChannels()-1);

                if (intensity > 1.f)
                    intensity = 1.f;

                int pos = z * gradDim.y * gradDim.x + y * gradDim.x + x;

                bucketi = tgt::ifloor(intensity * (bucketCounti - 1));
                bucketg = tgt::ifloor((gradientLengths[pos] / maxLength) * (bucketCountg - 1));

                hist_[bucketi][bucketg]++;
                if ((hist_[bucketi][bucketg]) > maxValue_)
                    maxValue_ = hist_[bucketi][bucketg];

                if (bucketi < significantRangeIntensity_.x)
                    significantRangeIntensity_.x = bucketi;
                if (bucketi > significantRangeIntensity_.y)
                    significantRangeIntensity_.y = bucketi;
                if (bucketg < significantRangeGradient_.x)
                    significantRangeGradient_.x = bucketg;
                if (bucketg > significantRangeGradient_.y)
                    significantRangeGradient_.y = bucketg;
            }
        }
    }
}

} // namespace voreen
