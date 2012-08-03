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

#include "voreen/core/datastructures/volume/histogram.h"
#include "voreen/core/datastructures/volume/volume.h"

#include "voreen/core/io/serialization/xmlserializer.h"
#include "voreen/core/io/serialization/xmldeserializer.h"

namespace voreen {

Histogram1D createHistogram1DFromVolume(const VolumeBase* handle, int bucketCount) {
    const VolumeRAM* vol = handle->getRepresentation<VolumeRAM>();
    RealWorldMapping rwm = handle->getRealWorldMapping();

    float min = handle->getDerivedData<VolumeMinMax>()->getMinNormalized();
    float max = handle->getDerivedData<VolumeMinMax>()->getMaxNormalized();
    min = rwm.normalizedToRealWorld(min);
    max = rwm.normalizedToRealWorld(max);

    Histogram1D h(min, max, bucketCount);

    for(size_t i=0; i<vol->getNumVoxels(); i++) {
        float v = vol->getVoxelNormalized(i);
        v = rwm.normalizedToRealWorld(v);

        h.addSample(v);
    }

    return h;
}

//-----------------------------------------------------------------------------

VolumeHistogramIntensity::VolumeHistogramIntensity() :
    VolumeDerivedData(),
    hist_(0.0f, 1.0f, 1)
{}

VolumeHistogramIntensity::VolumeHistogramIntensity(const VolumeHistogramIntensity& h) : hist_(h.hist_) {
}

VolumeHistogramIntensity::VolumeHistogramIntensity(const Histogram1D& h) : hist_(h) {
}

VolumeHistogramIntensity::VolumeHistogramIntensity(const VolumeBase* vol, int bucketCount) : hist_(createHistogram1DFromVolume(vol, bucketCount)) {
}

VolumeDerivedData* VolumeHistogramIntensity::createFrom(const VolumeBase* handle) const {
    tgtAssert(handle, "no volume handle");
    VolumeHistogramIntensity* h = new VolumeHistogramIntensity();
    Histogram1D hist = createHistogram1DFromVolume(handle, 256);
    h->hist_ = hist;
    return h;
}

size_t VolumeHistogramIntensity::getBucketCount() const {
    return hist_.getNumBuckets();
}

uint64_t VolumeHistogramIntensity::getValue(int bucket) const {
    return hist_.getBucket(bucket);
}

uint64_t VolumeHistogramIntensity::getValue(size_t bucket) const {
    return getValue(static_cast<int>(bucket));
}

uint64_t VolumeHistogramIntensity::getValue(float i) const {
    size_t bucketCount = hist_.getNumBuckets();
    float m = (bucketCount - 1.f);
    int bucket = static_cast<int>(floor(i * m));
    return getValue(bucket);
}

float VolumeHistogramIntensity::getNormalized(int i) const {
    return hist_.getBucketNormalized(i);
}

float VolumeHistogramIntensity::getNormalized(float i) const {
    size_t bucketCount = hist_.getNumBuckets();
    float m = (bucketCount - 1.f);
    int bucket = static_cast<int>(floor(i * m));
    return getNormalized(bucket);
}

float VolumeHistogramIntensity::getLogNormalized(int i) const {
     return (logf(static_cast<float>(1+hist_.getBucket(i)) ) / logf( static_cast<float>(1+hist_.getMaxBucket())));
}

float VolumeHistogramIntensity::getLogNormalized(float i) const {
    size_t bucketCount = hist_.getNumBuckets();
    float m = (bucketCount - 1.f);
    int bucket = static_cast<int>(floor(i * m));
    return getLogNormalized(bucket);
}

void VolumeHistogramIntensity::serialize(XmlSerializer& s) const  {
    s.serialize("histogram", hist_);
}

void VolumeHistogramIntensity::deserialize(XmlDeserializer& s) {
    s.deserialize("histogram", hist_);
}

const Histogram1D& VolumeHistogramIntensity::getHistogram() const {
    return hist_;
}

Histogram1D& VolumeHistogramIntensity::getHistogram() {
    return hist_;
}

//-----------------------------------------------------------------------------

VolumeHistogramIntensityGradient::VolumeHistogramIntensityGradient(const VolumeBase* volumeHandleGrad, const VolumeBase* volumeHandleIntensity,
                                                       int bucketCounti, int bucketCountg, bool scale)
    : scaleFactor_(1.f)
{
    const VolumeRAM* volumeGrad = volumeHandleGrad->getRepresentation<VolumeRAM>();
    const VolumeRAM* volumeIntensity = volumeHandleIntensity->getRepresentation<VolumeRAM>();

    if (dynamic_cast<const VolumeRAM_3xUInt8*>(volumeGrad)) {
        calcHG(static_cast<const VolumeRAM_3xUInt8*>(volumeGrad), volumeIntensity, bucketCounti, bucketCountg, scale);
    }
    else if (dynamic_cast<const VolumeRAM_4xUInt8*>(volumeGrad)) {
        calcHG(static_cast<const VolumeRAM_4xUInt8*>(volumeGrad), volumeIntensity, bucketCounti, bucketCountg, scale);
    }
    else if (dynamic_cast<const VolumeRAM_3xUInt16*>(volumeGrad)) {
        calcHG(static_cast<const VolumeRAM_3xUInt16*>(volumeGrad), volumeIntensity, bucketCounti, bucketCountg, scale);
    }
    else if (dynamic_cast<const VolumeRAM_4xUInt16*>(volumeGrad)) {
        calcHG(static_cast<const VolumeRAM_4xUInt16*>(volumeGrad), volumeIntensity, bucketCounti, bucketCountg, scale);
    }
    else {
        LERRORC("voreen.VolumeVolumeHistogramIntensityGradient",
                "VolumeVolumeHistogramIntensityGradient needs 24 or 32 bit DS as input!");
    }
}

VolumeHistogramIntensityGradient::VolumeHistogramIntensityGradient() :
    VolumeDerivedData(),
    maxValue_(-1),
    significantRangeIntensity_(-1),
    significantRangeGradient_(-1),
    scaleFactor_(-1.f)
{}

VolumeDerivedData* VolumeHistogramIntensityGradient::createFrom(const VolumeBase* /*handle*/) const {
    // unable to create 2D histogram without gradient volume
    return 0;
}

size_t VolumeHistogramIntensityGradient::getBucketCountIntensity() const {
    return histValues_.size();
}

size_t VolumeHistogramIntensityGradient::getBucketCountGradient() const {
    return histValues_[0].size();
}

int VolumeHistogramIntensityGradient::getValue(int i, int g) const {
    return histValues_[i][g];
}

float VolumeHistogramIntensityGradient::getNormalized(int i, int g) const {
    return (static_cast<float>(histValues_[i][g]) / static_cast<float>(maxValue_));
}

float VolumeHistogramIntensityGradient::getLogNormalized(int i, int g) const {
    return (logf(static_cast<float>(1+histValues_[i][g]) ) / log(static_cast<float>(1+maxValue_)));
}

int VolumeHistogramIntensityGradient::getMaxValue() const {
    return maxValue_;
}

tgt::ivec2 VolumeHistogramIntensityGradient::getSignificantRangeIntensity() const {
    return significantRangeIntensity_;
}

tgt::ivec2 VolumeHistogramIntensityGradient::getSignificantRangeGradient() const {
    return significantRangeGradient_;
}

float VolumeHistogramIntensityGradient::getScaleFactor() const {
    return scaleFactor_;
}

template<class U>
void VolumeHistogramIntensityGradient::calcHG(const VolumeAtomic<U>* volumeGrad, const VolumeRAM* volumeIntensity, int bucketCounti, int bucketCountg, bool scale) {
    // bits used for gradients
    int bitsG = volumeGrad->getBitsAllocated() / volumeGrad->getNumChannels();
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
        histValues_.push_back(std::vector<int>(bucketCountg));


    int bucketg, bucketi;
    maxValue_ = 0;
    significantRangeIntensity_ = tgt::ivec2(bucketCounti, -1);
    significantRangeGradient_ = tgt::ivec2(bucketCountg, -1);

    for (int z = 0; z < gradDim.z; ++z) {
        for (int y = 0; y < gradDim.y; ++y) {
            for (int x = 0; x < gradDim.x; ++x) {
                float intensity;
                if (volumeIntensity)
                    intensity = volumeIntensity->getVoxelNormalized(x,y,z);
                else
                    intensity = volumeGrad->getVoxelNormalized(x,y,z,volumeGrad->getNumChannels()-1);

                if (intensity > 1.f)
                    intensity = 1.f;

                int pos = z * gradDim.y * gradDim.x + y * gradDim.x + x;

                bucketi = tgt::ifloor(intensity * (bucketCounti - 1));
                bucketg = tgt::ifloor((gradientLengths[pos] / maxLength) * (bucketCountg - 1));

                histValues_[bucketi][bucketg]++;
                if ((histValues_[bucketi][bucketg]) > maxValue_)
                    maxValue_ = histValues_[bucketi][bucketg];

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

void VolumeHistogramIntensityGradient::serialize(XmlSerializer& s) const {
    s.serialize("values", histValues_);
    s.serialize("maxValue", maxValue_);
    s.serialize("significantRangeIntensity", significantRangeIntensity_);
    s.serialize("significantRangeGradient", significantRangeGradient_);
    s.serialize("scaleFactor", scaleFactor_);
}

void VolumeHistogramIntensityGradient::deserialize(XmlDeserializer& s) {
    s.deserialize("values", histValues_);
    s.deserialize("maxValue", maxValue_);
    s.deserialize("significantRangeIntensity", significantRangeIntensity_);
    s.deserialize("significantRangeGradient", significantRangeGradient_);
    s.deserialize("scaleFactor", scaleFactor_);
}

} // namespace voreen
