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

#ifndef VRN_HISTOGRAM_H
#define VRN_HISTOGRAM_H

#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/volume/volume.h"

#include <string>
#include <iostream>
#include <fstream>

namespace voreen {

/// Histogram base class
class Histogram {

};

// ----------------------------------------------------------------------------

/// 1D Intensity Histogram.
class HistogramIntensity : public Histogram {
public:
    /// Create new histogram with bucketCount buckets from volume
    HistogramIntensity(Volume* volume, int bucketCount);

    size_t getBucketCount();

    /// get value in bucket i
    int getValue(int i);

    /// Returns normalized (with max.) histogram value at bucket i
    float getNormalized(int i);

    /// Returns normalized logarithmic histogram value at bucket i
    float getLogNormalized(int i);

    /// Returns the significant range of the histogram, i.e.
    /// the minimal / maximal non-zero bucket
    tgt::ivec2 getSignificantRange() const;

protected:
    std::vector<int> hist_;
    int maxValue_;
    tgt::ivec2 significantRange_;
};

// ----------------------------------------------------------------------------

/// 2D histogram using intensity and gradient length.
class HistogramIntensityGradient : public Histogram {
public:
    /**
     * Calculate 2D Histogram.
     *
     * @param volumeGrad Holds the gradients
     * @param volumeIntensity Holds the intensity
     * @param bucketCounti Intensity bucket count
     * @param bucketCountg Gradient strength bucket count
     * @param scale should the histogram scaled to maximum gradient length in the dataset?
     */
    HistogramIntensityGradient(Volume* volumeGrad, Volume* volumeIntensity,
                               int bucketCounti, int bucketCountg, bool scale = false);

    /// Returns voxels in bucket.
    int getValue(int i, int g) const;

    /// Returns normalized (with max.) histogram value
    float getNormalized(int i, int g) const;

    /// Returns normalized logarithmic histogram value
    float getLogNormalized(int i, int g) const;

    /// Returns the maximal bucket value in the histogram
    int getMaxValue() const;

    /// Returns the significant intensity range of the histogram, i.e.
    /// the minimal / maximal bucket row
    tgt::ivec2 getSignificantRangeIntensity() const;

    /// Returns the significant gradient range of the histogram, i.e.
    /// the minimal / maximal bucket column
    tgt::ivec2 getSignificantRangeGradient() const;

    size_t getBucketCountIntensity() const;
    size_t getBucketCountGradient() const;

    float getScaleFactor() const;

protected:
    std::vector<std::vector<int> > hist_;  ///< 2D array representing the histogram (first index = intensity)
    int maxValue_;
    tgt::ivec2 significantRangeIntensity_;
    tgt::ivec2 significantRangeGradient_;

    float scaleFactor_;

    // calculate the histogram
    template<class U>
    void calcHG(VolumeAtomic<U>* volumeGrad, Volume* volumeIntensity, int bucketCounti, int bucketCountg, bool scale) {
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
                        intensity = volumeGrad->voxel(x,y,z).elem[2] * 2.f/ halfMax;

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
};

} // namespace voreen

#endif // VRN_HISTOGRAM_H
