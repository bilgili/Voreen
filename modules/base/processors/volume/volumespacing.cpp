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

#include "volumespacing.h"

#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volumedecorator.h"
#include "voreen/core/datastructures/meta/primitivemetadata.h"

namespace voreen {

const std::string VolumeSpacing::loggerCat_("voreen.base.VolumeSpacing");

VolumeSpacing::VolumeSpacing()
    : VolumeProcessor(),
    inport_(Port::INPORT, "volumehandle.input", "Volume Input"),
    outport_(Port::OUTPORT, "volumehandle.output", "Volume Output", false),
    enableProcessing_("enabled", "Enable", false),
    mode_("mode", "Mode"),
    uniformSpacing_("uniformSpacing", "Uniform Spacing", false),
    spacingX_("spacingX", "Spacing X", 1.0f, 0.0001f, 1000.f),
    spacingY_("spacingY", "Spacing Y", 1.0f, 0.0001f, 1000.f),
    spacingZ_("spacingZ", "Spacing Z", 1.0f, 0.0001f, 1000.f),
    spacingDisplay_("spacingDisplay", "Resulting Spacing", tgt::vec3(1.0f), tgt::vec3(0.0f), tgt::vec3(1000.f))
{
    addPort(inport_);
    addPort(outport_);

    addProperty(enableProcessing_);

    mode_.addOption("replace", "Replace");
    mode_.addOption("scale", "Scale");
    addProperty(mode_);

    addProperty(uniformSpacing_);

    spacingX_.setNumDecimals(5);
    spacingY_.setNumDecimals(5);
    spacingZ_.setNumDecimals(5);
    addProperty(spacingX_);
    addProperty(spacingY_);
    addProperty(spacingZ_);

    spacingDisplay_.setWidgetsEnabled(false);
    addProperty(spacingDisplay_);

    spacingX_.onChange(
        Call1ParMemberAction<VolumeSpacing, int>(this, &VolumeSpacing::spacingChanged, 0));
    spacingY_.onChange(
        Call1ParMemberAction<VolumeSpacing, int>(this, &VolumeSpacing::spacingChanged, 1));
    spacingZ_.onChange(
        Call1ParMemberAction<VolumeSpacing, int>(this, &VolumeSpacing::spacingChanged, 2));

    uniformSpacing_.onChange(
        CallMemberAction<VolumeSpacing>(this, &VolumeSpacing::uniformScalingChanged));
}

Processor* VolumeSpacing::create() const {
    return new VolumeSpacing();
}

void VolumeSpacing::process() {
    const VolumeBase* inputVolume = inport_.getData();

    if (!enableProcessing_.get()) {
        outport_.setData(inputVolume, false);
        spacingDisplay_.set(inputVolume->getSpacing());
        return;
    }

    tgt::vec3 spacing(spacingX_.get(), spacingY_.get(), spacingZ_.get());
    if (mode_.isSelected("scale"))
        spacing *= inputVolume->getSpacing();
    VolumeBase* outputVolume =
        new VolumeDecoratorReplaceSpacing(inputVolume, spacing);
    outport_.setData(outputVolume);
    spacingDisplay_.set(spacing);
}

void VolumeSpacing::spacingChanged(int dim) {

    if (!uniformSpacing_.get())
        return;

    if (dim == 0) {
        float xScale = spacingX_.get();
        spacingY_.set(xScale);
        spacingZ_.set(xScale);
    }
    else if (dim == 1) {
        float yScale = spacingY_.get();
        spacingX_.set(yScale);
        spacingZ_.set(yScale);
    }
    else if (dim == 2) {
        float zScale = spacingZ_.get();
        spacingX_.set(zScale);
        spacingY_.set(zScale);
    }
}

void VolumeSpacing::uniformScalingChanged() {
    if (uniformSpacing_.get())
        spacingChanged(0);
}

}   // namespace
