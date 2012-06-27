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

#include "voreen/core/volume/volumeseries.h"

#ifndef VRN_MESSAGEDISTRIBUTOR_H
#include "voreen/core/vis/messagedistributor.h"
#endif

namespace voreen {

const Identifier VolumeSeries::msgUpdateTimesteps_("update.Timesteps");

// public methods
//

VolumeSeries::VolumeSeries(VolumeSet* const parentSet, const std::string& name,
                           const Modality& modality) : 
    name_(name), parentVolumeSet_(parentSet), modality_(modality), maximumTimestep_(-1.0f)
{
}

VolumeSeries::VolumeSeries(const VolumeSeries& series) {
    clone(series);
}

VolumeSeries::~VolumeSeries() {
    freeHandles();
}

VolumeSeries& VolumeSeries::operator=(const VolumeSeries& m) {
    clone(m);
    return *this;
}

bool VolumeSeries::operator==(const VolumeSeries& m) const {
    return m.name_ == name_;
}

bool VolumeSeries::operator!=(const VolumeSeries& m) const {
    return m.name_ != name_;
}

bool VolumeSeries::operator<(const VolumeSeries& m) const {
    return m.name_ < name_;
}

const std::string& VolumeSeries::getName() const {
    return name_;
}

void VolumeSeries::setName(const std::string& name) {
    name_ = name;
}

const Modality& VolumeSeries::getModality() const {
    return modality_;
}

void VolumeSeries::setModality(const Modality& modality) {
    modality_ = modality;
}

VolumeSet* VolumeSeries::getParentVolumeSet() const {
    return parentVolumeSet_;
}

void VolumeSeries::setParentVolumeSet(VolumeSet* const volumeSet) {
    parentVolumeSet_ = volumeSet;
}

const VolumeHandle::HandleSet& VolumeSeries::getVolumeHandles() const {
    return handles_;
}

VolumeHandle* VolumeSeries::getVolumeHandle(const int index) const
{
    if ((index < 0) || (static_cast<size_t>(index) >= handles_.size()))
        return 0;

    VolumeHandle::HandleSet::const_iterator it = handles_.begin();
    for (int i = 0; it != handles_.end(); ++it, i++) {
        if (i == index)
            return const_cast<VolumeHandle*>(*it);
    }
    return 0;
}

int VolumeSeries::getNumVolumeHandles() const {
    return (static_cast<int>(handles_.size()));
}

bool VolumeSeries::isUnknown() const {
    return (modality_ == Modality::MODALITY_UNKNOWN);
}

bool VolumeSeries::addVolumeHandle(VolumeHandle*& handle, const bool forceInsertion) {
    if (handle == 0)
        return false;

    std::pair<VolumeHandle::HandleSet::iterator, bool> pr = handles_.insert(handle);

    // If the handle has been inserted, the maximum timestep eventually needs to be
    // updated and success is indicated by returning "true"
    //
    if( pr.second == true ) {
        if( handle->getTimestep() > maximumTimestep_ )
            maximumTimestep_ = handle->getTimestep();
        (*(pr.first))->setParentSeries(this);
        MsgDistr.postMessage(new BoolMsg(VolumeSeries::msgUpdateTimesteps_, true), "VolumeSelectionProcessor");
        return true;
    }

    // If the insertion is forced and the handle has not been inserted yet, its timestep
    // must be already contained. So update its timestep to maximum + 1.0f. Insertion
    // by then should never fail. If it does nevertheless, some serious problem has
    // been occured and enforcing insertion would probably make no sense either.
    //
    if( forceInsertion == true ) {
        handle->setTimestep(maximumTimestep_ + 1.0f);
        pr = handles_.insert(handle);
        if( pr.second == true ) {
            (*(pr.first))->setParentSeries(this);
            maximumTimestep_ += 1.0f;
        }
    }

    // If insertion fails and the contaiend handle is not the same as the one which is
    // about to be inserted, replace the given one with the contained one.
    //
    if( pr.second == false ) {
        VolumeHandle* containedHandle = static_cast<VolumeHandle*>(*(pr.first));
        if (handle != containedHandle) {
            delete handle;
            handle = containedHandle;
        }
    } else
        MsgDistr.postMessage(new BoolMsg(VolumeSeries::msgUpdateTimesteps_, true), "VolumeSelectionProcessor");

    return pr.second;
}

VolumeHandle* VolumeSeries::findVolumeHandle(VolumeHandle* const handle) const {
    if( handle == 0 )
        return 0;

    VolumeHandle::HandleSet::const_iterator it = handles_.find(handle);
    if( it != handles_.end() )
        return *it;

    return 0;
}

VolumeHandle* VolumeSeries::findVolumeHandle(const float timestep) const {
    VolumeHandle handle(0, 0, timestep);
    return findVolumeHandle(&handle);
}

VolumeHandle* VolumeSeries::removeVolumeHandle(VolumeHandle* const handle) {
    if( handle == 0 )
        return 0;

    VolumeHandle* found = findVolumeHandle(handle);
    if( found != 0 ) {
        handles_.erase(handle);
        adjustMaximumTimestep();
        MsgDistr.postMessage(new BoolMsg(VolumeSeries::msgUpdateTimesteps_, true), "VolumeSelectionProcessor");
    }
    return found;
}

VolumeHandle* VolumeSeries::removeVolumeHandle(const float timestep) {
    VolumeHandle handle(0, 0, timestep);
    return removeVolumeHandle(&handle);
}

bool VolumeSeries::deleteVolumeHandle(VolumeHandle* const handle) {
    VolumeHandle* found = removeVolumeHandle(handle);
    if( found == 0 )
        return false;

    delete found;
    return true;
}

bool VolumeSeries::deleteVolumeHandle(const float timestep) {
    VolumeHandle handle(0, 0, timestep);
    return deleteVolumeHandle(&handle);
}

// private methods
//

void VolumeSeries::adjustMaximumTimestep() {
    VolumeHandle::HandleSet::const_iterator it = handles_.begin();
    float max = -1.0f;
    for( ; it != handles_.end(); ++it ) {
        if( *it == 0 )
            continue;

        if( (*it)->getTimestep() > max )
            max = (*it)->getTimestep();
    }
    maximumTimestep_ = max;
}

void VolumeSeries::clone(const VolumeSeries& series) {
    name_ = series.getName();
    parentVolumeSet_ = series.getParentVolumeSet();
    modality_ = series.getModality();

    freeHandles();
    handles_ = series.getVolumeHandles();
}

void VolumeSeries::freeHandles() {
    VolumeHandle::HandleSet::iterator it;
    for (it = handles_.begin(); it != handles_.end(); ++it)
        delete (*it);
    handles_.clear();
}

} // namespace voreen
