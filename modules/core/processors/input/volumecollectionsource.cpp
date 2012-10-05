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

#include "volumecollectionsource.h"

#include "voreen/core/processors/processorwidget.h"
#include "voreen/core/processors/processorwidgetfactory.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumecollection.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/progressbar.h"
#include "voreen/core/voreenapplication.h"

namespace voreen {

const std::string VolumeCollectionSource::loggerCat_("voreen.core.VolumeCollectionSource");

VolumeCollectionSource::VolumeCollectionSource()
    : Processor(),
      outport_(Port::OUTPORT, "volumecollection", "VolumeCollection Output", false),
      volumeURLList_("volumeURLList", "Volume URL List", std::vector<std::string>())
{
    addPort(outport_);
    addProperty(volumeURLList_);
}

VolumeCollectionSource::~VolumeCollectionSource() {
}

Processor* VolumeCollectionSource::create() const {
    return new VolumeCollectionSource();
}

void VolumeCollectionSource::process() {
    // nothing
}

void VolumeCollectionSource::initialize() throw (tgt::Exception) {
    Processor::initialize();

    volumeURLList_.loadVolumes(false, true);

    outport_.setData(volumeURLList_.getVolumes(true), true);

    if (getProcessorWidget())
        getProcessorWidget()->updateFromProcessor();
}

void VolumeCollectionSource::invalidate(int inv) {
    outport_.setData(volumeURLList_.getVolumes(true), true);

    Processor::invalidate(inv);
}

void VolumeCollectionSource::setVolumeCollection(VolumeCollection* collection, bool owner) {
    volumeURLList_.clear();
    if (!collection)
        return;
    for (size_t i=0; i<collection->size(); i++)
        volumeURLList_.addVolume(collection->at(i), owner, true);
}

VolumeCollection* VolumeCollectionSource::getVolumeCollection() const {
    return volumeURLList_.getVolumes(false);
}

} // namespace
