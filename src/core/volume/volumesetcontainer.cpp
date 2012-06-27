/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/volume/volumesetcontainer.h"
#include "voreen/core/vis/processors/volumesetsourceprocessor.h"
#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

// static members
//
const Identifier VolumeSetContainer::msgUpdateVolumeSetContainer_("update.VolumeSetContainer");
const Identifier VolumeSetContainer::msgSetVolumeSetContainer_("set.VolumeSetContainer");

VolumeSetContainer::VolumeSetContainer() {
}

VolumeSetContainer::~VolumeSetContainer() {
    clear();
}

void VolumeSetContainer::clear() {
    VolumeSet::VSPSet::iterator it;
    for (it = volumesets_.begin(); it != volumesets_.end(); it++)
        delete *it;

    volumesets_.clear();
}

/*
VolumeSet* VolumeSetContainer::insertContainer(VolumeContainer* container, bool replace) {
    if (container == 0)
        return 0;

    if (replace)
        clear();
    
    VolumeSet* volset = 0;
    VolumeContainer::Entries entries = container->getEntries();
    VolumeContainer::Entries::iterator it = entries.begin();
printf("\n\tVolumeSetContainer::insertContainer():\n");
    for( ; it != entries.end(); it++) {
        VolumeContainer::Entry& e = it->second;
printf("\t\tadding volume [%d, %s, %s, %f, %d]...\n", e.volume_, e.name_.c_str(), e.modality_.getName().c_str(), e.time_, e.volumeGL_);
        if( this->findVolumeSet(e.name_) != 0 ) {
printf("\t\tVolume '%s' already contained! Skipping...\n", e.name_.c_str());
            continue;
        }
        // ugly hack: VolumeGL will be generated again by calling
        // VolumeHandle::generateHardwareVolumes(). This will vanish
        // together with the VolumeContainer.
        //
        delete e.volumeGL_;
        e.volumeGL_ = 0;

        if (volset == 0)
            volset = new VolumeSet(e.name_);

        VolumeSeries* series = new VolumeSeries(volset, e.modality_.getName());
        volset->addSeries(series);

        VolumeHandle* handle = new VolumeHandle(series, e.volume_, e.time_);
        handle->generateHardwareVolumes(VolumeHandle::HARDWARE_VOLUME_GL);
        series->addVolumeHandle(handle);

        // re-assign the VolumeGL
        //
        e.volumeGL_ = handle->getVolumeGL();
    }
    addVolumeSet(volset);
printf("\n");
    return volset;
}
*/

bool VolumeSetContainer::addVolumeSet(VolumeSet*& volset) {
    if (volset == 0)
        return false;

    std::pair<VolumeSet::VSPSet::iterator, bool> pr = volumesets_.insert(volset);

    // After adding a new VolumeSet, notify all Processors by posting
    // a message so that they can update their properties with available VolumeSets
    // 
    if (pr.second) {
        MsgDistr.postMessage(new VolumeSetContainerMsg(VolumeSetContainer::msgUpdateVolumeSetContainer_, this));
    } else {
        // The insertion failed because the VolumeSet in volset was already contained in the
        // VolumeSetContainer.
        // Therefore replace volset by the one from the container if it is not itself!
        //
        VolumeSet* containedVolSet = static_cast<VolumeSet*>(*(pr.first));
        if (containedVolSet != volset) {
            delete volset;
            volset = containedVolSet;
        }
    }
    return pr.second;
}

VolumeSet* VolumeSetContainer::findVolumeSet(VolumeSet* const volsetWanted) {
    if ( (volumesets_.empty() == true) || (volsetWanted == 0) )
        return 0;

    VolumeSet::VSPSet::iterator it = volumesets_.find(volsetWanted);
    if (it != volumesets_.end())
        return *it;

    return 0;
}

VolumeSet* VolumeSetContainer::findVolumeSet(const std::string& name) {
    VolumeSet volset(name);
    return findVolumeSet(&volset);
}

bool VolumeSetContainer::containsVolumeSet(VolumeSet* const volset) {
   return (findVolumeSet(volset) != 0);
}

VolumeSet* VolumeSetContainer::removeVolumeSet(VolumeSet* const volset) {
    VolumeSet* found = findVolumeSet(volset);
    if (found != 0)
        volumesets_.erase(volset);

    // As the content of the VolumeSetContainer has changed, the responsible processors
    // need to update their properties. Therefore this message is sent.
    //
    MsgDistr.postMessage(new VolumeSetContainerMsg(VolumeSetContainer::msgUpdateVolumeSetContainer_, this));
    return found;
}

VolumeSet* VolumeSetContainer::removeVolumeSet(const std::string& name) {
    VolumeSet volset(name);
    return removeVolumeSet(&volset);
}

bool VolumeSetContainer::deleteVolumeSet(VolumeSet* const volset) {
    VolumeSet* vs = removeVolumeSet(volset);
    if (vs == 0)
        return false;

    delete vs;
    return true;
}

bool VolumeSetContainer::deleteVolumeSet(const std::string& name) {
    VolumeSet volset(name);
    return deleteVolumeSet(&volset);
}

std::vector<std::string> VolumeSetContainer::getVolumeSetNames() const {
    std::vector<std::string> names;
    VolumeSet::VSPSet::const_iterator itVS = volumesets_.begin();
    for (; itVS != volumesets_.end(); ++itVS) {
        VolumeSet* vs = *itVS;
        if (vs == 0)
            continue;

        const std::string& name = vs->getName();
        if (!name.empty())
            names.push_back(name);
    }
    return names;
}

const VolumeSet::VSPSet& VolumeSetContainer::getVolumeSets() const {
    return volumesets_;
}

} // namespace
