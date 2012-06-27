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

#include "voreen/modules/base/processors/datasource/volumesource.h"

#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/processors/processorwidget.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/voreenapplication.h"

namespace voreen {

const std::string VolumeSource::loggerCat_("voreen.VolumeSourceProcessor");

VolumeSource::VolumeSource()
    : Processor()
    , volumeHandle_("volumeHandle", "Volume", 0)
    , outport_(Port::OUTPORT, "volumehandle.volumehandle", 0)
{
    addPort(outport_);
    addProperty(volumeHandle_);
}

Processor* VolumeSource::create() const {
    return new VolumeSource();
}

std::string VolumeSource::getProcessorInfo() const {
    return "Provides a single volume.";
}

void VolumeSource::process() {}

void VolumeSource::initialize() throw (VoreenException) {
    Processor::initialize();

    outport_.setData(volumeHandle_.get());

    if (volumeHandle_.get())
        volumeHandle_.get()->addObserver(this);

    if (getProcessorWidget())
        getProcessorWidget()->updateFromProcessor();
}

void VolumeSource::loadVolume(const std::string& filename) throw (tgt::FileException, std::bad_alloc) {
    clearVolume();

    if (volumeHandle_.get())
        stopObservation(volumeHandle_.get());

    volumeHandle_.loadVolume(filename);

    if (volumeHandle_.get())
        volumeHandle_.get()->addObserver(this);

    if (getProcessorWidget())
        getProcessorWidget()->updateFromProcessor();
}

void VolumeSource::clearVolume() {
    if (volumeHandle_.get()) {
        stopObservation(volumeHandle_.get());
        volumeHandle_.set(0);
    }
}

void VolumeSource::setVolumeHandle(VolumeHandle* handle) {
    if (volumeHandle_.get())
        stopObservation(volumeHandle_.get());

    volumeHandle_.set(handle);

    if (handle)
        handle->addObserver(this);
}

const VolumeHandle* VolumeSource::getVolumeHandle() const {
    return volumeHandle_.get();
}

void VolumeSource::invalidate(int /*inv*/) {
    if (outport_.getData() != volumeHandle_.get()) {
        outport_.setData(volumeHandle_.get());
        if (getProcessorWidget())
            getProcessorWidget()->updateFromProcessor();
    }
}

void VolumeSource::volumeHandleDelete(const VolumeHandle* source) {
    if (volumeHandle_.get() == source)
        volumeHandle_.set(0);
}

void VolumeSource::volumeChange(const VolumeHandle* /*source*/) {
    // noop
}

} // namespace
