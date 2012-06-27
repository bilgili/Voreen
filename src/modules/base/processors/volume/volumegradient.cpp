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

#include "voreen/modules/base/processors/volume/volumegradient.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/gradient.h"

namespace voreen {

const std::string VolumeGradient::loggerCat_("voreen.VolumeGradient");

VolumeGradient::VolumeGradient()
    : VolumeProcessor(),
    technique_("technique", "Technique"),
    copyIntensityChannel_("copyIntensityChannel", "Copy intensity channel", false),
    inport_(Port::INPORT, "volumehandle.input"),
    outport_(Port::OUTPORT, "volumehandle.output", 0)

{
    technique_.addOption("central-differences", "Central differences");
    technique_.addOption("sobel", "Sobel");
    technique_.addOption("linear-regression", "Linear regression");
    addProperty(technique_);
    addProperty(copyIntensityChannel_);

    addPort(inport_);
    addPort(outport_);
}

VolumeGradient::~VolumeGradient() {
}

Processor* VolumeGradient::create() const {
    return new VolumeGradient();
}

std::string VolumeGradient::getProcessorInfo() const {
    return std::string("Computes gradients of the intensity input volume and stores them in a RGB volume. \
                        The A-channel can optionally be filled with the input volume's intensity.");
}

void VolumeGradient::process() {

    Volume* inputVolume = inport_.getData()->getVolume();
    Volume* outputVolume = 0;

    // expecting a single-channel volume
    if (inputVolume->getNumChannels() == 1) {

        bool bit16 = inputVolume->getBitsAllocated() > 8;

        if (technique_.get() == "central-differences") {
            if (copyIntensityChannel_.get() && bit16)
                outputVolume = calcGradients<tgt::Vector4<uint16_t> >(inputVolume);
            else if (copyIntensityChannel_.get() && !bit16)
                outputVolume = calcGradients<tgt::col4>(inputVolume);
            else if (!copyIntensityChannel_.get() && bit16)
                outputVolume = calcGradients<tgt::Vector3<uint16_t> >(inputVolume);
            else if (!copyIntensityChannel_.get() && !bit16)
                outputVolume = calcGradients<tgt::col3>(inputVolume);
            else {
                tgtAssert(false, "Should not get here");
            }
        }
        else if (technique_.get() == "sobel") {
            if (copyIntensityChannel_.get() && bit16)
                outputVolume = calcGradientsSobel<tgt::Vector4<uint16_t> >(inputVolume);
            else if (copyIntensityChannel_.get() && !bit16)
                outputVolume = calcGradientsSobel<tgt::col4>(inputVolume);
            else if (!copyIntensityChannel_.get() && bit16)
                outputVolume = calcGradientsSobel<tgt::Vector3<uint16_t> >(inputVolume);
            else if (!copyIntensityChannel_.get() && !bit16)
                outputVolume = calcGradientsSobel<tgt::col3>(inputVolume);
            else {
                tgtAssert(false, "Should not get here");
            }
        }
        else if (technique_.get() == "linear-regression") {
            if (copyIntensityChannel_.get() && bit16)
                outputVolume = calcGradientsLinearRegression<tgt::Vector4<uint16_t> >(inputVolume);
            else if (copyIntensityChannel_.get() && !bit16)
                outputVolume = calcGradientsLinearRegression<tgt::col4>(inputVolume);
            else if (!copyIntensityChannel_.get() && bit16)
                outputVolume = calcGradientsLinearRegression<tgt::Vector3<uint16_t> >(inputVolume);
            else if (!copyIntensityChannel_.get() && !bit16)
                outputVolume = calcGradientsLinearRegression<tgt::col3>(inputVolume);
            else {
                tgtAssert(false, "Should not get here");
            }
        }
        else {
            LERROR("Unknown technique");
        }
    }
    else {
        LWARNING("Intensity volume expected, but passed volume consists of " << inputVolume->getNumChannels() << " channels.");
    }

    // assign computed volume to outport
    if (outputVolume)
        outport_.setData(new VolumeHandle(outputVolume), true);
    else
        outport_.deleteVolume();
}

void VolumeGradient::deinitialize() throw (VoreenException) {
    outport_.deleteVolume();

    VolumeProcessor::deinitialize();
}

}   // namespace
