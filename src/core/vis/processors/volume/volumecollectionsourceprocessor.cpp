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

#include "voreen/core/vis/processors/volume/volumecollectionsourceprocessor.h"

#include "voreen/core/vis/processors/processorwidget.h"
#include "voreen/core/vis/processors/processorwidgetfactory.h"
#include "voreen/core/volume/volumehandle.h"
#include "voreen/core/volume/volumecontainer.h"
#include "voreen/core/vis/processors/ports/allports.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/ioprogress.h"
#include "voreen/core/application.h"

namespace voreen {

const std::string VolumeCollectionSourceProcessor::loggerCat_("voreen.VolumeCollectionSourceProcessor");

VolumeCollectionSourceProcessor::VolumeCollectionSourceProcessor()
    : Processor(),
      volumeCollection_("volumeCollection", "Volume Collection", new VolumeCollection()),
      outport_(Port::OUTPORT, "volumecollection", 0)
{

    addPort(outport_);

    addProperty(volumeCollection_);
}

const std::string VolumeCollectionSourceProcessor::getProcessorInfo() const {
    return "Provides a collection of volumes.";
}

Processor* VolumeCollectionSourceProcessor::create() const {
    return new VolumeCollectionSourceProcessor();
}

void VolumeCollectionSourceProcessor::process() {

}

void VolumeCollectionSourceProcessor::initialize() throw (VoreenException) {

    Processor::initialize();

    outport_.setData(volumeCollection_.get());

    if (getProcessorWidget())
        getProcessorWidget()->updateFromProcessor();
}

void VolumeCollectionSourceProcessor::invalidate(InvalidationLevel /*inv = INVALID_RESULT*/) {

    if (outport_.getData() != volumeCollection_.get())
        outport_.setData(volumeCollection_.get());
    else
        outport_.invalidate();
}

void VolumeCollectionSourceProcessor::setVolumeCollection(VolumeCollection* collection) {
    volumeCollection_.set(collection);
}

VolumeCollection* VolumeCollectionSourceProcessor::getVolumeCollection() const {
    return volumeCollection_.get();
}

} // namespace
