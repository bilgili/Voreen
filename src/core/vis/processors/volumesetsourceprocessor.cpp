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

#include "voreen/core/vis/processors/volumesetsourceprocessor.h"

#ifndef VRN_VOLUMESELECTIONPROCESSOR_H
#include "voreen/core/vis/processors/volumeselectionprocessor.h"
#endif

namespace voreen {

// static members
//
const Identifier VolumeSetSourceProcessor::msgUpdateCurrentVolumeset_("update.currentVolumeSet");

VolumeSetSourceProcessor::VolumeSetSourceProcessor() :
    Processor(),
    volumesetContainer_(0),
    volumeset_(0),
    outportName_("volumeset.volumeset"),
    volumesetsProp_(0)
{
	createOutport(outportName_);
    setName("VolumeSetSource");

    volumesetsProp_ = new EnumProp(msgUpdateCurrentVolumeset_, "available VolumeSets", availableVolumesets_);
    volumesetsProp_->setSerializable(false);
    addProperty(volumesetsProp_);
    setTag("VolumeSetSourceProcessor");
    MsgDistr.insert(this);
}

VolumeSetSourceProcessor::~VolumeSetSourceProcessor() {
    if (tgt::Singleton<voreen::MessageDistributor>::isInited() == true)
        MsgDistr.remove(this);

    delete volumesetsProp_;
    volumesetsProp_ = 0;
}

const Identifier VolumeSetSourceProcessor::getClassName() const {
    return "VolumeSetSource.VolumeSetSource";
}

const std::string VolumeSetSourceProcessor::getProcessorInfo() const {
    return "Provides a set of volumes.";
}

bool VolumeSetSourceProcessor::connect(Port* outport, Port* inport) {
    // call inherited method first to ensure a proper connection
    // between the two processors.
    //
    bool ret = Processor::connect(outport, inport);

    if ((outport == 0) || (inport == 0))
        return ret;
    
    // check if the processor on the inport is a VolumeSelectionProcessor. If so,
    // propagate the current VolumeSet in this processor to it.
    //
    VolumeSelectionProcessor* vsp = dynamic_cast<VolumeSelectionProcessor*>(inport->getProcessor());
    if (vsp != 0)
        vsp->setVolumeSet(volumeset_);
    return ret;
}

Processor* VolumeSetSourceProcessor::create() {
    return new VolumeSetSourceProcessor();
}

void VolumeSetSourceProcessor::process(LocalPortMapping*) {
}

void VolumeSetSourceProcessor::processMessage(Message* msg, const Identifier& dest)
{
    Processor::processMessage(msg, dest);

    if (msg == 0)
        return;

    if (msg->id_ == msgUpdateCurrentVolumeset_) {
        const std::string& volumesetName = msg->getValue<std::string>();
        updateCurrentVolumeSet(volumesetName);
    } else if (msg->id_ == VolumeSetContainer::msgUpdateVolumeSetContainer_) {
        // Content of the message is not used when the container is already set.
        // The content would be a pointer to the posting VolumeSetContainer itself!
        //
        if (volumesetContainer_ == 0) {
            VolumeSetContainer* volsetCont = msg->getValue<VolumeSetContainer*>();
            setVolumeSetContainer(volsetCont);
        }
        else {
            updateAvailableVolumeSets();
            if (availableVolumesets_.empty() == false) {
                int curSelection = volumesetsProp_->get();
                if (curSelection < 0)
                    curSelection = 0;
                const int size = static_cast<int>(availableVolumesets_.size());
                updateCurrentVolumeSet(availableVolumesets_[ ((curSelection < size) ? curSelection : 0) ]);
            } else
                setVolumeSet(0);

        }
    } else if (msg->id_ == VolumeSetContainer::msgSetVolumeSetContainer_) {
        VolumeSetContainer* volsetCont = msg->getValue<VolumeSetContainer*>();
        setVolumeSetContainer(volsetCont);
    }

	// if volumesetcontainer is cleared dont use its volumeset
	else if (msg->id_ == "volumesetcontainer.clear") {
        VolumeSetContainer* volsetCont = msg->getValue<VolumeSetContainer*>();
		if (volsetCont == volumesetContainer_) {
			volumeset_ = 0;
			availableVolumesets_.clear();
		}
    }
}

VolumeSet* VolumeSetSourceProcessor::getVolumeSet() {
    return volumeset_;
}

VolumeSet** VolumeSetSourceProcessor::getVolumeSetAddress() {
    return &volumeset_;
}

void VolumeSetSourceProcessor::setVolumeSet(voreen::VolumeSet* const volumeset) {
    if( volumeset_ == volumeset )
        return;

    volumeset_ = volumeset;

    // notify connected VolumeSelectionProcessors about the change so they can
    // update their own properties
    //
    Port* out = getOutport(outportName_);
    if ( out == 0 )
        return;

    std::vector<Port*>& others = out->getConnected();
    for (size_t i = 0; i < others.size(); i++) {
        if (others[i] == 0)
            continue;
        VolumeSelectionProcessor* vsp = dynamic_cast<VolumeSelectionProcessor*>(others[i]->getProcessor());
        if (vsp != 0)
            vsp->setVolumeSet(volumeset_);
    }
}

const VolumeSetContainer* VolumeSetSourceProcessor::getVolumeSetContainer() const {
    return volumesetContainer_;
}

void VolumeSetSourceProcessor::setVolumeSetContainer(voreen::VolumeSetContainer* const volumesetContainer) {
    if (volumesetContainer_ != volumesetContainer)
        volumesetContainer_ = volumesetContainer;

    if (volumesetContainer_ == 0)
        return;

    updateAvailableVolumeSets();
    if (availableVolumesets_.empty() == false)
        updateCurrentVolumeSet(availableVolumesets_[0]);
}

// private methods
//
void VolumeSetSourceProcessor::updateAvailableVolumeSets() {
    if (volumesetsProp_ == 0 || volumesetContainer_ == 0) 
        return;

    availableVolumesets_.clear();
    availableVolumesets_ = volumesetContainer_->getVolumeSetNames();
    volumesetsProp_->setStrings(availableVolumesets_);
    volumesetsProp_->forwardChangesToPlugin();
}

void VolumeSetSourceProcessor::updateCurrentVolumeSet(const std::string& volumesetName) {
    if ((volumesetContainer_ == 0) || (volumesetName.empty() == true))
        return;

    // find the VolumeSet for the given name and call setVolumeSet() in order
    // to propagate the information about the currently chosen VolumeSet so
    // that the VolumeSelectionProcessors can update their modalities and
    // timesteps properties.
    //
    if (volumesetContainer_ != 0)
        setVolumeSet( volumesetContainer_->findVolumeSet(volumesetName) );
}

}   // namespace
