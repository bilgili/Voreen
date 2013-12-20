/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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
#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volumedisk.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorgradient.h"

#include "voreen/core/io/serialization/xmlserializer.h"
#include "voreen/core/io/serialization/xmldeserializer.h"

namespace voreen {

using tgt::vec3;
using tgt::ivec3;

Histogram1D createHistogram1DFromVolume(const VolumeBase* handle, size_t bucketCount, size_t channel /*= 0*/) {
    RealWorldMapping rwm = handle->getRealWorldMapping();

    const VolumeMinMax* volumeMinMax = handle->getDerivedData<VolumeMinMax>();
    tgtAssert(channel < volumeMinMax->getNumChannels(), "invalid channel");
    float min = volumeMinMax->getMinNormalized(channel);
    float max = volumeMinMax->getMaxNormalized(channel);
    min = rwm.normalizedToRealWorld(min);
    max = rwm.normalizedToRealWorld(max);

    return createHistogram1DFromVolume(handle, bucketCount, min, max, channel);
}

VRN_CORE_API Histogram1D createHistogram1DFromVolume(const VolumeBase* handle, size_t bucketCount,
    float realWorldMin, float realWorldMax, size_t channel /*= 0*/)
{
    tgtAssert(realWorldMin <= realWorldMax, "invalid real world range");

    RealWorldMapping rwm = handle->getRealWorldMapping();
    Histogram1D h(realWorldMin, realWorldMax, (int)bucketCount);

    // prefer RAM over disk representation, but only if RAM volume is already present
    const VolumeRAM* volumeRam = 0;
    const VolumeDisk* volumeDisk = 0;
    if (handle->hasRepresentation<VolumeRAM>())
        volumeRam = handle->getRepresentation<VolumeRAM>();
    else if (handle->hasRepresentation<VolumeDisk>())
        volumeDisk = handle->getRepresentation<VolumeDisk>();
    else {
        LWARNINGC("voreen.Histogram", "Unable to compute 1D histogram: neither disk nor RAM representation available");
        return h;
    }
    tgtAssert(volumeRam || volumeDisk, "no representation");

    // iterate over slices
    tgt::svec3 dims = handle->getDimensions();
    tgt::svec3 pos;
    for (pos.z = 0; pos.z < dims.z; ++pos.z) {
        try {
            boost::this_thread::sleep(boost::posix_time::seconds(0));
        }
        catch(boost::thread_interrupted&)
        {
            throw boost::thread_interrupted();
        }

        if (volumeRam) {
            // access volume data in RAM directly
            for (pos.y = 0; pos.y < dims.y; ++pos.y) {
                for (pos.x = 0; pos.x < dims.x; ++pos.x) {
                    float val = volumeRam->getVoxelNormalized(pos, channel);
                    val = rwm.normalizedToRealWorld(val);
                    h.addSample(val);
                }
            }
        }
        else if (volumeDisk) {
            try {
                // temporarily load current slice into RAM
                VolumeRAM* sliceVolume = volumeDisk->loadSlices(pos.z, pos.z);
                tgtAssert(sliceVolume, "null pointer returned (exception expected)");
                for (pos.y = 0; pos.y < dims.y; ++pos.y) {
                    for (pos.x = 0; pos.x < dims.x; ++pos.x) {
                        float val = sliceVolume->getVoxelNormalized(tgt::svec3(pos.x, pos.y, 0), channel);
                        val = rwm.normalizedToRealWorld(val);
                        h.addSample(val);
                    }
                }
                delete sliceVolume;
            }
            catch (tgt::Exception& e) {
                LWARNINGC("voreen.Histogram", "Unable to compute 1D histogram: failed to load slice from disk volume: " + std::string(e.what()));
                return h;
            }
        }
        else {
            tgtAssert(false, "should never get here");
        }
    }

    return h;
}

//-----------------------------------------------------------------------------

Histogram2D createHistogram2DFromVolume(const VolumeBase* handle, int bucketCountIntensity, int bucketCountGradient) {
    const VolumeRAM* vol = handle->getRepresentation<VolumeRAM>();
    RealWorldMapping rwm = handle->getRealWorldMapping();
    ivec3 dims = vol->getDimensions();
    vec3 sp = handle->getSpacing();

    float min = handle->getDerivedData<VolumeMinMax>()->getMinNormalized();
    float max = handle->getDerivedData<VolumeMinMax>()->getMaxNormalized();
    min = rwm.normalizedToRealWorld(min);
    max = rwm.normalizedToRealWorld(max);

    float minGradLength = 0.0f; // always 0
    float maxGradLength = 0.0f;

    //TODO: improve performance
    ivec3 pos;
    for (pos.z = 0; pos.z < dims.z; ++pos.z) {
        try {
            boost::this_thread::sleep(boost::posix_time::seconds(0));
        }
        catch(boost::thread_interrupted&)
        {
            throw boost::thread_interrupted();
        }
        for (pos.y = 0; pos.y < dims.y; ++pos.y) {
            for (pos.x = 0; pos.x < dims.x; ++pos.x) {
                //vec3 grad = VolumeOperatorGradient::calcGradientCentralDifferences(vol, sp, pos);
                vec3 grad = VolumeOperatorGradient::calcGradientSobel(vol, sp, pos);

                float nlength = tgt::length(grad) * rwm.getScale();

                if (nlength > maxGradLength)
                    maxGradLength = nlength;
            }
        }
    }

    Histogram2D h(min, max, bucketCountIntensity, minGradLength, maxGradLength, bucketCountGradient);
    for (pos.z = 0; pos.z < dims.z; ++pos.z) {
        try {
            boost::this_thread::sleep(boost::posix_time::seconds(0));
        }
        catch(boost::thread_interrupted&)
        {
            throw boost::thread_interrupted();
        }
        for (pos.y = 0; pos.y < dims.y; ++pos.y) {
            for (pos.x = 0; pos.x < dims.x; ++pos.x) {
                //vec3 grad = VolumeOperatorGradient::calcGradientCentralDifferences(vol, sp, pos);
                vec3 grad = VolumeOperatorGradient::calcGradientSobel(vol, sp, pos);

                float nlength = tgt::length(grad) * rwm.getScale();

                float v = vol->getVoxelNormalized(pos);
                v = rwm.normalizedToRealWorld(v);

                h.addSample(v, nlength);
            }
        }
    }

    return h;
}

//-----------------------------------------------------------------------------

VolumeHistogramIntensity::VolumeHistogramIntensity() :
    VolumeDerivedData(),
    histograms_()
{}

VolumeHistogramIntensity::VolumeHistogramIntensity(const VolumeHistogramIntensity& h)
    : VolumeDerivedData()
    , histograms_(h.histograms_)
{}

VolumeHistogramIntensity::VolumeHistogramIntensity(const Histogram1D& h) {
    histograms_.push_back(h);
}

VolumeHistogramIntensity::VolumeHistogramIntensity(const std::vector<Histogram1D>& histograms)
    : VolumeDerivedData()
    , histograms_(histograms)
{}

VolumeDerivedData* VolumeHistogramIntensity::createFrom(const VolumeBase* handle) const {
    tgtAssert(handle, "no volume");
    VolumeHistogramIntensity* h = new VolumeHistogramIntensity();
    for (size_t channel = 0; channel < handle->getNumChannels(); channel++) {
        Histogram1D hist = createHistogram1DFromVolume(handle, 256, channel);
        h->histograms_.push_back(hist);
    }
    return h;
}

size_t VolumeHistogramIntensity::getNumChannels() const {
    return histograms_.size();
}

size_t VolumeHistogramIntensity::getBucketCount(size_t channel) const {
    tgtAssert(channel < histograms_.size(), "invalid channel");
    return histograms_.at(channel).getNumBuckets();
}

uint64_t VolumeHistogramIntensity::getValue(int bucket, size_t channel) const {
    tgtAssert(channel < histograms_.size(), "invalid channel");
    return histograms_.at(channel).getBucket(bucket);
}

uint64_t VolumeHistogramIntensity::getValue(size_t bucket, size_t channel) const {
    tgtAssert(channel < histograms_.size(), "invalid channel");
    return getValue(static_cast<int>(bucket), channel);
}

uint64_t VolumeHistogramIntensity::getValue(float i, size_t channel) const {
    tgtAssert(channel < histograms_.size(), "invalid channel");
    size_t bucketCount = histograms_.at(channel).getNumBuckets();
    float m = (bucketCount - 1.f);
    int bucket = static_cast<int>(floor(i * m));
    return getValue(bucket, channel);
}

float VolumeHistogramIntensity::getNormalized(int i, size_t channel) const {
    tgtAssert(channel < histograms_.size(), "invalid channel");
    return histograms_.at(channel).getBucketNormalized(i);
}

float VolumeHistogramIntensity::getNormalized(float i, size_t channel) const {
    tgtAssert(channel < histograms_.size(), "invalid channel");
    size_t bucketCount = histograms_.at(channel).getNumBuckets();
    float m = (bucketCount - 1.f);
    int bucket = static_cast<int>(floor(i * m));
    return getNormalized(bucket, channel);
}

float VolumeHistogramIntensity::getLogNormalized(int i, size_t channel) const {
    tgtAssert(channel < histograms_.size(), "invalid channel");
    const Histogram1D& hist = histograms_.at(channel);
    return hist.getBucketLogNormalized(i);
}

float VolumeHistogramIntensity::getLogNormalized(float i, size_t channel) const {
    tgtAssert(channel < histograms_.size(), "invalid channel");
    size_t bucketCount = histograms_.at(channel).getNumBuckets();
    float m = (bucketCount - 1.f);
    int bucket = static_cast<int>(floor(i * m));
    return getLogNormalized(bucket, channel);
}

void VolumeHistogramIntensity::serialize(XmlSerializer& s) const  {
    s.serialize("histograms", histograms_, "channel");
}

void VolumeHistogramIntensity::deserialize(XmlDeserializer& s) {
    try {
        s.deserialize("histograms", histograms_, "channel");
    }
    catch (SerializationException& /*e*/) {
        // try to deserialize old format (single channel)
        Histogram1D hist;
        s.deserialize("histogram", hist);

        histograms_.clear();
        histograms_.push_back(hist);
    }
}

const Histogram1D& VolumeHistogramIntensity::getHistogram(size_t channel) const {
    tgtAssert(channel < histograms_.size(), "invalid channel");
    return histograms_.at(channel);
}

Histogram1D& VolumeHistogramIntensity::getHistogram(size_t channel) {
    tgtAssert(channel < histograms_.size(), "invalid channel");
    return histograms_.at(channel);
}

VolumeDerivedData* VolumeHistogramIntensity::create() const {
    return new VolumeHistogramIntensity();
}

//-----------------------------------------------------------------------------

VolumeHistogramIntensityGradient::VolumeHistogramIntensityGradient() :
    VolumeDerivedData(),
    hist_(0.0f, 1.0f, 1, 0.0f, 1.0f, 1)
{}

VolumeDerivedData* VolumeHistogramIntensityGradient::createFrom(const VolumeBase* handle) const {
    tgtAssert(handle, "no volume");
    VolumeHistogramIntensityGradient* h = new VolumeHistogramIntensityGradient();
    Histogram2D hist = createHistogram2DFromVolume(handle, 256, 256);
    h->hist_ = hist;
    h->maxBucket_ = hist.getMaxBucket();
    return h;
}

size_t VolumeHistogramIntensityGradient::getBucketCountIntensity() const {
    return hist_.getNumBuckets(0);
}

size_t VolumeHistogramIntensityGradient::getBucketCountGradient() const {
    return hist_.getNumBuckets(1);
}

int VolumeHistogramIntensityGradient::getValue(int i, int g) const {
    return static_cast<int>(hist_.getBucket(i, g));
}

float VolumeHistogramIntensityGradient::getNormalized(int i, int g) const {
    return (static_cast<float>(getValue(i, g)) / static_cast<float>(getMaxBucket()));
}

float VolumeHistogramIntensityGradient::getLogNormalized(int i, int g) const {
    return (logf(static_cast<float>(1+getValue(i, g)) ) / log(static_cast<float>(1+getMaxBucket())));
}

int VolumeHistogramIntensityGradient::getMaxBucket() const {
    return static_cast<int>(maxBucket_);
}

float VolumeHistogramIntensityGradient::getMinValue(int dim) const {
    return hist_.getMinValue(dim);
}

float VolumeHistogramIntensityGradient::getMaxValue(int dim) const {
    return hist_.getMaxValue(dim);
}

void VolumeHistogramIntensityGradient::serialize(XmlSerializer& s) const {
    s.serialize("histogram", hist_);
}

void VolumeHistogramIntensityGradient::deserialize(XmlDeserializer& s) {
    s.deserialize("histogram", hist_);
}

VolumeDerivedData* VolumeHistogramIntensityGradient::create() const {
    return new VolumeHistogramIntensityGradient();
}


} // namespace voreen
