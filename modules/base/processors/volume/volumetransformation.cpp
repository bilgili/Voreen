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

#include "volumetransformation.h"

#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumedecorator.h"
#include "voreen/core/datastructures/meta/primitivemetadata.h"

#include "tgt/logmanager.h"

namespace voreen {

const std::string VolumeTransformation::loggerCat_("voreen.base.VolumeTransformation");

VolumeTransformation::VolumeTransformation()
    : VolumeProcessor()
    , inport_(Port::INPORT, "volumehandle.input", "Volume Input")
    , outport_(Port::OUTPORT, "volumehandle.output", "Volume Output", false)
    , enableProcessing_("enableProcessing", "Enable")
    , transformationMode_("transformationMode", "Transformation Mode")
    , sourceCoordinateSystem_("sourceCoordinateSystem", "Source Coordinate System")
    , transformMatrix_("transformMatrix", "Transformation Matrix", tgt::mat4::identity, tgt::mat4(-1e10), tgt::mat4(1e10))
{
    addPort(inport_);
    addPort(outport_);

    addProperty(enableProcessing_);

    transformationMode_.addOption("concatenate", "Concatenate");
    transformationMode_.addOption("replace",     "Replace");
    transformationMode_.onChange(CallMemberAction<VolumeTransformation>(this, &VolumeTransformation::adjustPropertyVisibility));
    addProperty(transformationMode_);

    sourceCoordinateSystem_.addOption("voxel-coordinates",   "Voxel Coordinates");
    sourceCoordinateSystem_.addOption("texture-coordinates", "Texture Coordinates");
    sourceCoordinateSystem_.addOption("volume-coordinates",  "Volume/Physical Coordinates");
    sourceCoordinateSystem_.addOption("world-coordinates",   "World Coordinates");
    sourceCoordinateSystem_.select("volume-coordinates");
    addProperty(sourceCoordinateSystem_);

    addProperty(transformMatrix_);
}

VolumeTransformation::~VolumeTransformation() {}

Processor* VolumeTransformation::create() const {
    return new VolumeTransformation();
}

void VolumeTransformation::initialize() throw (tgt::Exception) {
    VolumeProcessor::initialize();
    adjustPropertyVisibility();
}

void VolumeTransformation::process() {
    const VolumeBase* inputVolume = inport_.getData();
    if (!enableProcessing_.get()) {
        outport_.setData(inputVolume, false);
        return;
    }

    tgt::mat4 outputTrafo = tgt::mat4::identity;
    if (transformationMode_.isSelected("concatenate")) {
        // transform volume into world-coords using current transformation, then apply specified transformation
        outputTrafo = transformMatrix_.get() * inputVolume->getPhysicalToWorldMatrix();
    }
    else if (transformationMode_.isSelected("replace")) {
        // transform from selected source coords into physical coords, then apply specified transformation
        tgt::mat4 toPhysical = tgt::mat4::identity;
        if (sourceCoordinateSystem_.isSelected("voxel-coordinates"))
            toPhysical = inputVolume->getPhysicalToVoxelMatrix();
        else if (sourceCoordinateSystem_.isSelected("volume-coordinates"))
            toPhysical = tgt::mat4::identity; // already in physical coordinates
        else if (sourceCoordinateSystem_.isSelected("texture-coordinates"))
            toPhysical = inputVolume->getPhysicalToTextureMatrix();
        else if (sourceCoordinateSystem_.isSelected("world-coordinates"))
            toPhysical = inputVolume->getPhysicalToWorldMatrix();
        else {
            LERROR("unknown source coordinates system selected: " << sourceCoordinateSystem_.get());
        }

        outputTrafo = transformMatrix_.get() * toPhysical;
    }
    else {
        LERROR("unknown transformationMode selected: " << transformationMode_.get());
    }

    VolumeBase* outputVolume =
        new VolumeDecoratorReplaceTransformation(inport_.getData(), outputTrafo);
    outport_.setData(outputVolume);
}

void VolumeTransformation::adjustPropertyVisibility() {
    sourceCoordinateSystem_.setVisible(transformationMode_.isSelected("replace"));
}

}   // namespace
