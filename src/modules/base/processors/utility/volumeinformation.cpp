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

#include "voreen/modules/base/processors/utility/volumeinformation.h"

#include "voreen/core/datastructures/volume/histogram.h"
#include "voreen/core/datastructures/volume/volumeoperator.h"

namespace voreen {

VolumeInformation::VolumeInformation()
    : VolumeProcessor()
    , volume_(Port::INPORT, "volume.inport", false, Processor::INVALID_RESULT)
    , computeButton_("compute", "Compute information")
    , numVoxels_("numVoxels", "Num voxels", 0, 0, 1<<30)
    , numSignificant_("numSignificant", "Num significant voxels", 0, 0, 1<<30)
    , minValue_("min", "Min intensity", 0.f, -1e16, 1e16)
    , maxValue_("max", "Max intensity", 0.f, -1e16, 1e16)
    , meanValue_("mean", "Mean intensity", 0.f, -1e16, 1e16)
    , standardDeviation_("dev", "Standard deviation", 0.f, 0.f, 1e16)
    , entropy_("entropy", "Entropy", 0.f, 0.f, 1e16)
{
    addPort(volume_);

    numVoxels_.setWidgetsEnabled(false);
    numSignificant_.setWidgetsEnabled(false);
    minValue_.setWidgetsEnabled(false);
    maxValue_.setWidgetsEnabled(false);
    entropy_.setWidgetsEnabled(false);
    meanValue_.setWidgetsEnabled(false);
    standardDeviation_.setWidgetsEnabled(false);
    computeButton_.onChange(CallMemberAction<VolumeInformation>(this, &VolumeInformation::computeInformation));

    addProperty(computeButton_);
    addProperty(numVoxels_);
    addProperty(numSignificant_);
    addProperty(minValue_);
    addProperty(maxValue_);
    addProperty(meanValue_);
    addProperty(standardDeviation_);
    addProperty(entropy_);
}

Processor* VolumeInformation::create() const {
    return new VolumeInformation;
}

std::string VolumeInformation::getProcessorInfo() const {
    return "Computes some properties such as the min, max, average voxel intensity "
           "of the input volume and displays them by read-only properties.";
}

void VolumeInformation::process() {
    // nothing
}

void VolumeInformation::computeInformation() {

    if (!volume_.hasData()) {
        numVoxels_.set(0);
        numSignificant_.set(0);
        minValue_.set(0.f);
        maxValue_.set(0.f);
        meanValue_.set(0.f);
        standardDeviation_.set(0.f);
        entropy_.set(0.f);
        return;
    }

    Volume* volume = volume_.getData()->getVolume();
    tgtAssert(volume, "No input volume");
    size_t numVoxels = volume->getNumVoxels();
    tgt::vec2 intensityRange = volume->elementRange();

    numVoxels_.setMaxValue(numVoxels);
    numVoxels_.set(numVoxels);

    VolumeOperatorNumSignificant volOpSignificant;
    numSignificant_.setMaxValue(numVoxels);
    numSignificant_.set(static_cast<int>(volOpSignificant.apply<size_t>(volume)));

    // compute the entropy of the volume
    // E(H) = -\sum_{i=0}^{K-1}{H_p(i) \log_2(H_p(i)}
    // with H_p(i): normalized histrogram with intensity value i
    int histMax = tgt::iround(intensityRange.y);
    // handle float data as if it was 16 bit to prevent overflow
    histMax = tgt::clamp(histMax, 0, 1<<16);

    HistogramIntensity histogram(volume, histMax);
    double entropy = 0.0;
    for (size_t i = 0; i < static_cast<size_t>(histMax); ++i) {
        int value = histogram.getValue(static_cast<int>(i));

        if (value == 0)
            continue;

        double hi = static_cast<double>(value) / static_cast<double>(numVoxels);
        double loghi = log(hi);

        entropy += hi * loghi;
    }
    entropy *= -1;
    entropy_.setMaxValue(intensityRange.y);
    entropy_.set(static_cast<float>(entropy));

    // min value
    minValue_.setMinValue(intensityRange.x);
    minValue_.setMaxValue(intensityRange.y);
    minValue_.set(static_cast<float>(histogram.getSignificantRange().x));

    //max value
    maxValue_.setMinValue(intensityRange.x);
    maxValue_.setMaxValue(intensityRange.y);
    maxValue_.set(static_cast<float>(histogram.getSignificantRange().y));

    // mean value
    double mean = 0.0;
    for (size_t i = 0; i < numVoxels; ++i) {
        float value = volume->getVoxelFloat(i);
        mean += value;
    }
    mean /= numVoxels;
    meanValue_.setMinValue(intensityRange.x);
    meanValue_.setMaxValue(intensityRange.y);
    meanValue_.set(static_cast<float>(mean));

    // variance
    double variance = 0.0;
    for (size_t i = 0; i < numVoxels; ++i) {
        float value = volume->getVoxelFloat(i);
        variance += pow(static_cast<double>(value) - mean, 2.0);
    }
    variance /= numVoxels;
    standardDeviation_.setMaxValue(intensityRange.y);
    standardDeviation_.set(static_cast<float>(sqrt(variance)));
}

} // namespace
