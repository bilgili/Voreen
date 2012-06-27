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

#include "voreen/core/vis/processors/volume/volumesourceprocessor.h"

#include "voreen/core/volume/volumehandle.h"
#include "voreen/core/vis/processors/processorwidgetfactory.h"
#include "voreen/core/vis/processors/ports/allports.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/ioprogress.h"
#include "voreen/core/application.h"

namespace voreen {

const std::string VolumeSourceProcessor::loggerCat_("voreen.VolumeSourceProcessor");

VolumeSourceProcessor::VolumeSourceProcessor()
    : Processor(),
      volumeHandle_("volumeHandle", "Volume", 0),
      outport_(Port::OUTPORT, "volumehandle.volumehandle", 0)
{

    addPort(outport_);

    addProperty(volumeHandle_);
}

const std::string VolumeSourceProcessor::getProcessorInfo() const {
    return "Provides a single volume.";
}

Processor* VolumeSourceProcessor::create() const {
    return new VolumeSourceProcessor();
}

void VolumeSourceProcessor::process() {

}

void VolumeSourceProcessor::initialize() throw (VoreenException) {

    Processor::initialize();

    outport_.setData(volumeHandle_.get());

    if (getProcessorWidget())
        getProcessorWidget()->updateFromProcessor();
}

void VolumeSourceProcessor::loadVolume(const std::string& filename)
    throw (tgt::FileException, std::bad_alloc) {

    clearVolume();

    volumeHandle_.loadVolume(filename);

    if (getProcessorWidget())
        getProcessorWidget()->updateFromProcessor();

}

void VolumeSourceProcessor::clearVolume() {
    if (outport_.getData()) {
        volumeHandle_.set(0);
    }
}

void VolumeSourceProcessor::setVolumeHandle(VolumeHandle* handle) {

    volumeHandle_.set(handle);
}

const VolumeHandle* VolumeSourceProcessor::getVolumeHandle() const {

    return volumeHandle_.get();
}

void VolumeSourceProcessor::invalidate(InvalidationLevel /*inv = INVALID_RESULT*/) {

    if (outport_.getData() != volumeHandle_.get()) {
        outport_.setData(volumeHandle_.get());
        if (getProcessorWidget())
            getProcessorWidget()->updateFromProcessor();
    }

}

} // namespace
