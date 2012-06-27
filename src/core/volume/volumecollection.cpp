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

#include "voreen/core/volume/volumecollection.h"

#include "voreen/core/volume/volumehandle.h"

using std::vector;

namespace voreen {

const std::string VolumeCollection::loggerCat_ = "voreen.core.VolumeCollection";

VolumeCollection::VolumeCollection() :
    Observable<VolumeCollectionObserver>()
{
}

VolumeCollection::~VolumeCollection()     {

    volumeHandles_.clear();
}

void VolumeCollection::add(VolumeHandle* volumeHandle) {

    tgtAssert(volumeHandle, "Null pointer as VolumeHandle passed");
    if (!contains(volumeHandle)) {
        volumeHandles_.push_back(volumeHandle);
        notifyVolumeAdded(volumeHandle);
    }
}

void VolumeCollection::add(const VolumeCollection& volumeCollection) {

    for (size_t i=0; i<volumeCollection.size(); ++i) {
        add(volumeCollection.at(i));
    }
}

void VolumeCollection::remove(const VolumeHandle* volumeHandle){

    std::vector<VolumeHandle*>::iterator handleIter = find(volumeHandle);
    if (handleIter != volumeHandles_.end()) {
        volumeHandles_.erase(handleIter);
        notifyVolumeRemoved(volumeHandle);
    }
}

void VolumeCollection::remove(const VolumeCollection& volumeCollection) {
    for (size_t i=0; i<volumeCollection.size(); ++i) {
        remove(volumeCollection.at(i));
    }
}

std::vector<VolumeHandle*>::iterator VolumeCollection::find(const VolumeHandle* volumeHandle) {

    return std::find(volumeHandles_.begin(), volumeHandles_.end(), volumeHandle);

}

std::vector<VolumeHandle*>::const_iterator VolumeCollection::find(const VolumeHandle* volumeHandle) const {

    return std::find(volumeHandles_.begin(), volumeHandles_.end(), volumeHandle);

}

bool VolumeCollection::contains(const VolumeHandle* volumeHandle) const {
    return (find(volumeHandle) != volumeHandles_.end());
}

VolumeHandle* VolumeCollection::at(size_t i) const {

    tgtAssert(i < volumeHandles_.size(), "Invalid index");
    return volumeHandles_.at(i);
}

VolumeHandle* VolumeCollection::first() const {
    return (!empty() ? volumeHandles_.front() : 0);
}

void VolumeCollection::clear() {
    volumeHandles_.clear();
//    notifyObservers();
}

VolumeCollection VolumeCollection::selectEntity(const std::string& /*name*/) const {
    return VolumeCollection();
}

VolumeCollection VolumeCollection::selectRepresentation(const std::string& /*name*/) const {
    return VolumeCollection();
}

VolumeCollection VolumeCollection::selectModality(const Modality& modality) const {

    VolumeCollection collection;
    for (size_t i=0; i<volumeHandles_.size(); ++i) {
        if (volumeHandles_[i]->getModality() == modality)
            collection.add(volumeHandles_[i]);
    }
    return collection;
}

VolumeCollection VolumeCollection::selectTimestep(float timestep) const {

    VolumeCollection collection;
    for (size_t i=0; i<volumeHandles_.size(); ++i) {
        if (volumeHandles_[i]->getTimestep() == timestep)
            collection.add(volumeHandles_[i]);
    }
    return collection;
}

voreen::VolumeCollection VolumeCollection::selectOrigin(const VolumeOrigin& origin) const {
    VolumeCollection collection;
    for (size_t i=0; i<volumeHandles_.size(); ++i) {
        if (volumeHandles_[i]->getOrigin() == origin)
            collection.add(volumeHandles_[i]);
    }
    return collection;
}

size_t VolumeCollection::size() const {
    return volumeHandles_.size();
}

bool VolumeCollection::empty() const {
    return (size() == 0);
}

void VolumeCollection::notifyVolumeAdded(const VolumeHandle* handle) {

    const vector<VolumeCollectionObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->volumeAdded(this, handle);

}

void VolumeCollection::notifyVolumeRemoved(const VolumeHandle* handle) {

    const vector<VolumeCollectionObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->volumeRemoved(this, handle);

}

void VolumeCollection::serialize(XmlSerializer& s) const
{
    s.serialize("VolumeHandles", volumeHandles_, "VolumeHandle");
}

void VolumeCollection::deserialize(XmlDeserializer& s)
{
    s.deserialize("VolumeHandles", volumeHandles_, "VolumeHandle");

    // add volume handles of loaded volumes...
    VolumeHandleList removeList;
    for (VolumeHandleList::iterator it = volumeHandles_.begin(); it != volumeHandles_.end(); ++it)
        if ((*it) && (*it)->getVolume())
            add(*it);
        else
            removeList.push_back(*it);

    // remove volume handles that was not able to load its volume...
    for (VolumeHandleList::iterator it = removeList.begin(); it != removeList.end(); ++it) {
        remove(*it);
        s.freePointer(*it);
        delete *it;
    }
}

} // namespace
