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

#include "voreen/core/volume/histogram.h"

using namespace voreen;
using namespace tgt;

using tgt::ivec2;

namespace voreen {

HistogramIntensity::HistogramIntensity(Volume* volume, int bucketCount) {
    for (int i=0; i<bucketCount; i++)
        hist_.push_back(0);
       
    maxValue_ = 0;
    significantRange_ = ivec2(bucketCount, -1);

    VolumeUInt8* sourceDataset8Bit = dynamic_cast<VolumeUInt8*>(volume);
    if (sourceDataset8Bit) {
        int bucket;
        float m = (bucketCount-1.0f) / 255.0f;
        for (int z=0; z<volume->getDimensions().z; z++) {
            for (int y=0; y<volume->getDimensions().y; y++) {
                for (int x=0; x<volume->getDimensions().x; x++) {
                    bucket = static_cast<int>(floor(sourceDataset8Bit->voxel(ivec3(x,y,z))*m));
                    hist_[bucket]++;

                    if ( hist_[bucket] > maxValue_ && bucket > 0)
                        maxValue_ = hist_[bucket];

                    if ( bucket < significantRange_.x )
                        significantRange_.x = bucket;
                    if ( bucket > significantRange_.y )
                        significantRange_.y = bucket;

                }
            }
        }
        return;
    }
    
    Volume4xUInt8* sourceDataset32Bit = dynamic_cast<Volume4xUInt8*>(volume);
    if (sourceDataset32Bit) {
        int bucket;
        float m = (bucketCount-1.0f) / 255.0f;
        for (int z=0; z<volume->getDimensions().z; z++) {
            for (int y=0; y<volume->getDimensions().y; y++) {
                for (int x=0; x<volume->getDimensions().x; x++) {
                    bucket = static_cast<int>(floor(sourceDataset32Bit->voxel(ivec3(x,y,z))[3]*m));
                    hist_[bucket]++;

                    if ( hist_[bucket] > maxValue_ && bucket > 0)
                        maxValue_ = hist_[bucket];

                    if ( bucket < significantRange_.x )
                        significantRange_.x = bucket;
                    if ( bucket > significantRange_.y )
                        significantRange_.y = bucket;

                }
            }
        }
        return;
    }
    
    VolumeUInt16* sourceDataset16Bit = dynamic_cast<VolumeUInt16*>(volume);
    if (sourceDataset16Bit) {
        int bucket;
        float maxValue = (sourceDataset16Bit->getBitsStored() == 12) ? 4095.f : 65535.f;
        float m = (bucketCount-1.0f) / maxValue;
        for (int z=0; z<volume->getDimensions().z; z++) {
            for (int y=0; y<volume->getDimensions().y; y++) {
                for (int x=0; x<volume->getDimensions().x; x++) {
                    bucket = static_cast<int>(floor(sourceDataset16Bit->voxel(ivec3(x,y,z))*m));
					if (bucket < static_cast<int>(hist_.size())) {
						hist_[bucket]++;

						if ( hist_[bucket] > maxValue_ && bucket > 0)
							maxValue_ = hist_[bucket];

						if ( bucket < significantRange_.x )
							significantRange_.x = bucket;
						if ( bucket > significantRange_.y )
							significantRange_.y = bucket;
					}

                }
            }
        }
        return;
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

HistogramIntensityGradient::HistogramIntensityGradient(voreen::Volume* volumeGrad, voreen::Volume* volumeIntensity,
                                                       int bucketCounti, int bucketCountg)
{
    std::string loggerCat_ = "voreen.HistogramIntensityGradient";
    if (dynamic_cast<Volume3xUInt8*>(volumeGrad)) {
        Volume3xUInt8* vol24 = (Volume3xUInt8*) volumeGrad;
        calcHG(vol24, volumeIntensity, bucketCounti, bucketCountg);
    }
    else if (dynamic_cast<Volume4xUInt8*>(volumeGrad)) {
        Volume4xUInt8* vol32 = (Volume4xUInt8*) volumeGrad;
        calcHG(vol32, volumeIntensity, bucketCounti, bucketCountg);
    }
    else {
        LERROR("HistogramIntensityGradient needs 24 or 32 bit DS as input!");
    }
}

size_t HistogramIntensityGradient::getBucketCountIntensity() {
    return hist_.size();
}

size_t HistogramIntensityGradient::getBucketCountGradient() {
    return hist_[0].size();
}

int HistogramIntensityGradient::getValue(int i, int g) {
    return hist_[i][g];
}

float HistogramIntensityGradient::getNormalized(int i, int g) {
    return (static_cast<float>(hist_[i][g]) / static_cast<float>(maxValue_));
}

float HistogramIntensityGradient::getLogNormalized(int i, int g) {
    return (logf(static_cast<float>(1+hist_[i][g]) ) / log( static_cast<float>(1+maxValue_)));
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

//-----------------------------------------------------------------------------

} // namespace voreen

