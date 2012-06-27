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

#include "voreen/core/vis/processors/volumeselectionprocessor.h"
#include "voreen/core/vis/processors/portmapping.h"
#include "voreen/core/volume/volumeset.h"

namespace voreen {

// initialization of static members
//
const Identifier VolumeSelectionProcessor::msgSetCurrentModality_("set.currentModality");
const Identifier VolumeSelectionProcessor::msgSetCurrentTimestep_("set.currentTimestep");

VolumeSelectionProcessor::VolumeSelectionProcessor() : 
    Processor(),
    volumeSeries_(0),
    volumeHandle_(0),
    curVolumeSet_(0),
    volumeSetInportName_("volumeset.volumeset"),
    volumeOutportName_("volumehandle.volumehandle")
{
    modalitiesProp_ = new EnumProp(msgSetCurrentModality_, "Available Modalities", availableModalities_);
    addProperty(modalitiesProp_);

    timestepProp_ = new IntProp(msgSetCurrentTimestep_, "Timestep", 0, 0, 0, false);
    addProperty(timestepProp_);

    // These properties shall not be serialized as the are set, when a VolumeSet* is
    // associated with a VolumeSetSourceProcessor. This does not happen during
    // loading! The stored indices and values would be applied to an empty property and
    // cause the application to crash on loading networks!
    //
    modalitiesProp_->setSerializable(false);
    timestepProp_->setSerializable(true);

    createInport(volumeSetInportName_);
	createOutport(volumeOutportName_);
    setName("VolumeSelector");
    setTag("VolumeSelectionProcessor");
    MsgDistr.insert(this);
}

VolumeSelectionProcessor::~VolumeSelectionProcessor() {
    if( tgt::Singleton<voreen::MessageDistributor>::isInited() == true )
        MsgDistr.remove(this);
    
    delete modalitiesProp_;
    modalitiesProp_ = 0;
    
    delete timestepProp_;
    timestepProp_ = 0;
}

const Identifier VolumeSelectionProcessor::getClassName() const {
    return "VolumeSetSource.VolumeSelector";
}

const std::string VolumeSelectionProcessor::getProcessorInfo() const {
    return "Selects a single volume from a volume set.";
}

Processor* VolumeSelectionProcessor::create() {
    return new VolumeSelectionProcessor();
}

void VolumeSelectionProcessor::process(LocalPortMapping* portMapping) {
    if( portMapping == 0)
        return;

    VolumeSet** volumesetAddr = portMapping->getGenericData<VolumeSet**>(volumeSetInportName_);
    if( (volumesetAddr != 0) && (*volumesetAddr != curVolumeSet_) )
        setVolumeSet(*volumesetAddr);
}

void VolumeSelectionProcessor::processMessage(Message* msg, const Identifier& dest) {
    if( msg == 0 )
        return;

    Processor::processMessage(msg, dest);
    if( msg->id_ == msgSetCurrentTimestep_ )
        setCurrentTimestep(msg->getValue<int>());
    else if( msg->id_ == msgSetCurrentModality_ )
        setCurrentSeries(msg->getValue<std::string>());
    else if( msg->id_ == VolumeSet::msgUpdateVolumeSeries_ ) {
        updateAvailableModalities();
        updateAvailableTimesteps();
    } else if( msg->id_ == VolumeSeries::msgUpdateTimesteps_) 
        updateAvailableTimesteps();
}

VolumeHandle** VolumeSelectionProcessor::getVolumeHandleAddress() {
    return &volumeHandle_;
}

void VolumeSelectionProcessor::setVolumeSet(VolumeSet* const volumeset) {
    if( curVolumeSet_ != volumeset )
        curVolumeSet_ = volumeset;

    updateAvailableModalities();
    updateAvailableTimesteps();
}

// private
//
void VolumeSelectionProcessor::updateAvailableModalities() {
    availableModalities_.clear();
    if( curVolumeSet_ != 0 ) {
        availableModalities_ = curVolumeSet_->getSeriesNames();
        if( (availableModalities_.size() > 0) )
            setCurrentSeries(availableModalities_[0]);
    }
    
    if( modalitiesProp_ != 0 ) {
        modalitiesProp_->setStrings(availableModalities_);
        modalitiesProp_->forwardChangesToPlugin();
    }
}

void VolumeSelectionProcessor::updateAvailableTimesteps() {
    if( volumeSeries_ == 0 )
        return;

    int numHandles = volumeSeries_->getNumVolumeHandles();
    int curTimestep = timestepProp_->get();
    if( numHandles > 0 ) {
        if( timestepProp_ != 0 ) {
            timestepProp_->setMinValue(0);
            timestepProp_->setMaxValue(numHandles - 1);
            timestepProp_->set( (curTimestep < (numHandles - 1) ? curTimestep : 0) );
        }
        setCurrentTimestep(timestepProp_->get());
    }
    else
        volumeHandle_ = 0;
}

void VolumeSelectionProcessor::setCurrentSeries(const std::string& seriesName) {
    volumeSeries_ = 0;
    if( curVolumeSet_ == 0 )
        return;

    volumeSeries_ = curVolumeSet_->findSeries(seriesName);
    setCurrentTimestep(0);
}

void VolumeSelectionProcessor::setCurrentTimestep(const int timestepIndex) {
    volumeHandle_ = 0;
    if( volumeSeries_ == 0 ) {
        return;
    }

    volumeHandle_ = volumeSeries_->getVolumeHandle(timestepIndex);
}

} // namespace
