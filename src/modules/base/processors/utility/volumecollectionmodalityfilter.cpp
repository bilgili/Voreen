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

#include "voreen/modules/base/processors/utility/volumecollectionmodalityfilter.h"

namespace voreen {

const std::string VolumeCollectionModalityFilter::loggerCat_ = "VolumeCollectionModalityFilter";

VolumeCollectionModalityFilter::VolumeCollectionModalityFilter()
    : Processor(),
    inport_(Port::INPORT, "volumecollection"),
    outport_(Port::OUTPORT, "volumecollection.filtered"),
    modalityProp_("modality", "modality: "),
    currentModality_(Modality::MODALITY_ANY),
    filteredCollection_()
{
    addPort(inport_);
    addPort(outport_);

    std::vector<Option<Modality*> > options;
    const std::vector<Modality*>& modalities = Modality::getModalities();
    for (size_t i = 0; i < modalities.size(); ++i) {
        modalityProp_.addOption(modalities[i]->getName(), modalities[i]->getName(), modalities[i]);
    }
    modalityProp_.set(Modality::MODALITY_ANY.getName());
    modalityProp_.onChange(CallMemberAction<VolumeCollectionModalityFilter>(this,
        &VolumeCollectionModalityFilter::adjustFilteredCollection));
    addProperty(modalityProp_);
}

Processor* VolumeCollectionModalityFilter::create() const {
    return new VolumeCollectionModalityFilter();
}

std::string VolumeCollectionModalityFilter::getProcessorInfo() const {
    return "Permits to select one specific modality among those of all VolumeHandles within "
           "the incoming VolumeCollection. The filtered VolumeCollection contains only VolumeHandles of "
           "the specified modality.";
}

void VolumeCollectionModalityFilter::initialize() throw (VoreenException) {
    Processor::initialize();
    adjustFilteredCollection();
}

void VolumeCollectionModalityFilter::deinitialize() throw (VoreenException) {
    outport_.setData(0);

    Processor::deinitialize();
}

void VolumeCollectionModalityFilter::invalidate(int /*inv*/) {
    adjustFilteredCollection();
}

void VolumeCollectionModalityFilter::process() {
    // nothing
}

// private methods
//

void VolumeCollectionModalityFilter::adjustFilteredCollection() {
    VolumeCollection* collection = inport_.getData();
    if ((collection == 0) || (collection->empty() == true)) {
        filteredCollection_.clear();
        outport_.setData(0);
        return;
    }

    if (currentModality_.getName() != modalityProp_.get()) {
        currentModality_ = *(modalityProp_.getValue());
        filteredCollection_.clear();
        if (currentModality_ != Modality::MODALITY_ANY) {
            for (size_t i = 0; i < collection->size(); ++i) {
                if (collection->at(i)->getModality() == currentModality_)
                    filteredCollection_.add(collection->at(i));
            }
            outport_.setData(&filteredCollection_);
        } else
            outport_.setData(inport_.getData());
    }
}

}   // namespace voreen
