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

#include "voreen/core/vis/processors/volume/volumegradient.h"
#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/volumehandle.h"
#include "voreen/core/volume/gradient.h"

namespace voreen {

VolumeGradient::VolumeGradient()
    : VolumeProcessor(),
    processedVolumeHandle_(0),
    technique_("technique", "Technique"),
    inport_(Port::INPORT, "volumehandle.input"),
    outport_(Port::OUTPORT, "volumehandle.output", 0)

{
    technique_.addOption("central-differences", "Central differences");
    technique_.addOption("sobel", "Sobel");
    technique_.addOption("linear-regression", "Linear regression");
    addProperty(technique_);

    addPort(inport_);
    addPort(outport_);
}

VolumeGradient::~VolumeGradient() {
    delete processedVolumeHandle_;
}

const std::string VolumeGradient::getProcessorInfo() const {
    return std::string("");
}

void VolumeGradient::process() {

    Volume* v = 0;

    if (technique_.get() == "central-differences")
        v = calcGradients<tgt::col3>(inport_.getData()->getVolume());
    else if (technique_.get() == "sobel")
        v = calcGradientsSobel<tgt::col3>(inport_.getData()->getVolume());
    else if (technique_.get() == "linear-regression")
        v = calcGradientsLinearRegression<tgt::col3>(inport_.getData()->getVolume());
    else 
        LERROR("Unknown technique!");

    if (processedVolumeHandle_) {
        delete processedVolumeHandle_;
        processedVolumeHandle_ = 0;
    }

    if (v)
        processedVolumeHandle_ = new VolumeHandle(v, 0.0f);
    
    outport_.setData(processedVolumeHandle_);
}

}   // namespace
