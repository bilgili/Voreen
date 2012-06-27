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

#include "voreen/core/vis/processors/volume/volumeselector.h"

#include "voreen/core/volume/volumehandle.h"
#include "voreen/core/volume/volumecollection.h"
#include "voreen/core/vis/processors/processorwidgetfactory.h"
#include "voreen/core/vis/processors/ports/allports.h"

namespace voreen {

const std::string VolumeSelector::loggerCat_("voreen.VolumeSelector");

VolumeSelector::VolumeSelector()
    : Processor(),
      volumeID_("volumeID", "Selected volume", 0, 0, 100),
      inport_(Port::INPORT, "volumecollection", 0),
      outport_(Port::OUTPORT, "volumehandle.volumehandle", 0)
{

    addPort(inport_);
    addPort(outport_);

    addProperty(volumeID_);
}

const std::string VolumeSelector::getProcessorInfo() const {
    return "Selects a single volume from the input collection.";
}

Processor* VolumeSelector::create() const {
    return new VolumeSelector();
}

void VolumeSelector::process() {

}

void VolumeSelector::initialize() throw (VoreenException) {

    Processor::initialize();

    adjustToVolumeCollection();

}

void VolumeSelector::invalidate(InvalidationLevel /*inv = INVALID_RESULT*/) {

    adjustToVolumeCollection();

}

void VolumeSelector::adjustToVolumeCollection() {
    VolumeCollection* collection = inport_.getData();
    int max = (collection != 0) ? static_cast<int>(collection->size()) : 0;

    if (collection && !collection->empty() && (volumeID_.get() < max)) {
        // adjust max id to size of collection
        if (volumeID_.getMaxValue() != max - 1) {
            volumeID_.setMaxValue(max - 1);
            if (volumeID_.get() > volumeID_.getMaxValue())
                volumeID_.set(volumeID_.getMaxValue());
            volumeID_.updateWidgets();
        }

        tgtAssert((volumeID_.get() >= 0) && (volumeID_.get() < max), "Invalid volume index");

        // update output handle
        if (collection->at(volumeID_.get()) != outport_.getData())
            outport_.setData(collection->at(volumeID_.get()));
    } else {
        if (max > 0)
            max -= 1;

        // If the collection is smaller than the previous one, the maximum value
        // must be adjusted and the new value should be set.
        // The collection is 0 when deserializing the workspace, so that we must
        // not set value in that case, because it is the just deserialized one!
        //
        volumeID_.setMaxValue(max);
        if (collection != 0)
            volumeID_.set(max);
        volumeID_.updateWidgets();
        outport_.setData(0);
    }

}

} // namespace
