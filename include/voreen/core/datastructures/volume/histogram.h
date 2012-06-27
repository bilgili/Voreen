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

#ifndef VRN_HISTOGRAM_H
#define VRN_HISTOGRAM_H

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volume.h"

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
    HistogramIntensity(const Volume* volume, int bucketCount);

    size_t getBucketCount() const;

    /// get value in bucket i
    int getValue(int i) const;

    /// get value in bucket i
    int getValue(size_t i) const;

    /// get value in bucket nearest to i
    int getValue(float i) const;

    /// Returns normalized (with max.) histogram value at bucket i
    float getNormalized(int i) const;

    /// Returns normalized (with max.) histogram value at bucket nearest to i
    float getNormalized(float i) const;

    /// Returns normalized logarithmic histogram value at bucket i
    float getLogNormalized(int i) const;

    /// Returns normalized logarithmic histogram value at bucket nearest to i
    float getLogNormalized(float i) const;

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
    HistogramIntensityGradient(const Volume* volumeGrad, const Volume* volumeIntensity,
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
    void calcHG(const VolumeAtomic<U>* volumeGrad, const Volume* volumeIntensity, int bucketCounti, int bucketCountg, bool scale);
};

} // namespace voreen

#endif // VRN_HISTOGRAM_H
