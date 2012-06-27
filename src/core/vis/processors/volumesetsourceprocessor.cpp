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

#include "voreen/core/vis/processors/volumesetsourceprocessor.h"
#include "voreen/core/vis/processors/volumeselectionprocessor.h"
#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

const Identifier VolumeSetSourceProcessor::msgUpdateCurrentVolumeset_("update.currentVolumeSet");

VolumeSetSourceProcessor::VolumeSetSourceProcessor()
    : Processor(),
      volumesetContainer_(0),
      volumeset_(0),
      outportName_("volumeset.volumeset"),
      volumesetsProp_(0)
{
    createGenericOutport<VolumeSet**>(outportName_, &volumeset_);
    setName("VolumeSetSource");

    volumesetsProp_ = new StringSelectionProp(msgUpdateCurrentVolumeset_, "VolumeSet", false);
    volumesetsProp_->onChange(
        CallMemberAction<VolumeSetSourceProcessor>(this, &VolumeSetSourceProcessor::currentVolumeSetChanged));
    volumesetsProp_->setSerializable(false);

    addProperty(volumesetsProp_);
    setTag("VolumeSetSourceProcessor");
}

VolumeSetSourceProcessor::~VolumeSetSourceProcessor() {
    delete volumesetsProp_;
}

const Identifier VolumeSetSourceProcessor::getClassName() const {
    return "VolumeProcessor.VolumeSetSource";
}

const std::string VolumeSetSourceProcessor::getProcessorInfo() const {
    return "Provides a set of volumes.";
}

bool VolumeSetSourceProcessor::connect(Port* outport, Port* inport) {
    // call inherited method first to ensure a proper connection
    // between the two processors.
    bool ret = Processor::connect(outport, inport);

    if (outport == 0 || inport == 0)
        return ret;

    // check if the processor on the inport is a VolumeSelectionProcessor. If so,
    // propagate the current VolumeSet in this processor to it.
    VolumeSelectionProcessor* vsp = dynamic_cast<VolumeSelectionProcessor*>(inport->getProcessor());
    if (vsp != 0)
        vsp->setVolumeSet(volumeset_);
    return ret;
}

Processor* VolumeSetSourceProcessor::create() const {
    return new VolumeSetSourceProcessor();
}

void VolumeSetSourceProcessor::processMessage(Message* msg, const Identifier& dest) {
    Processor::processMessage(msg, dest);

    /*
    if (msg->id_ == msgUpdateCurrentVolumeset_) {
        const std::string& volumesetName = msg->getValue<std::string>();
        updateCurrentVolumeSet(volumesetName);
        invalidate();
    }
    else
    */
    if (msg->id_ == VolumeSetContainer::msgUpdateVolumeSetContainer_) {
        // Content of the message is not used when the container is already set.
        // The content would be a pointer to the posting VolumeSetContainer itself!
        if (volumesetContainer_ == 0) {
            VolumeSetContainer* volsetCont = msg->getValue<VolumeSetContainer*>();
            setVolumeSetContainer(volsetCont);
        }
        updateAvailableVolumeSets();
        currentVolumeSetChanged();
        invalidate();
    }
    else if (msg->id_ == VolumeSetContainer::msgSetVolumeSetContainer_) {
        VolumeSetContainer* volsetCont = msg->getValue<VolumeSetContainer*>();
        setVolumeSetContainer(volsetCont);
        invalidate();
    }

    // if volumesetcontainer is cleared dont use its volumeset
    else if (msg->id_ == "volumesetcontainer.clear") {
        VolumeSetContainer* volsetCont = msg->getValue<VolumeSetContainer*>();
        if (volsetCont == volumesetContainer_) {
            volumeset_ = 0;
            availableVolumeSets_.clear();
        }
        invalidate();
    }
}

VolumeSet* VolumeSetSourceProcessor::getVolumeSet() {
    return volumeset_;
}

VolumeSet** VolumeSetSourceProcessor::getVolumeSetAddress() {
    return &volumeset_;
}

void VolumeSetSourceProcessor::setVolumeSet(VolumeSet* const volumeset) {
    if (volumeset_ == 0 && volumeset_ == volumeset)
        return;
    
    volumeset_ = volumeset;

    if (volumeset_ == 0)
        volumesetsProp_->set("");
    else 
        volumesetsProp_->set(volumeset->getName());
    
    // notify connected VolumeSelectionProcessors about the change so they can
    // update their own properties
    Port* out = getOutport(outportName_);
    if (out == 0)
        return;

    std::vector<Port*>& others = out->getConnected();
    for (size_t i = 0; i < others.size(); ++i) {
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

void VolumeSetSourceProcessor::setVolumeSetContainer(VolumeSetContainer* const volumesetContainer) {
    if (volumesetContainer_ != volumesetContainer)
        volumesetContainer_ = volumesetContainer;
}

void VolumeSetSourceProcessor::currentVolumeSetChanged() {
    updateCurrentVolumeSet(volumesetsProp_->get());
}

void VolumeSetSourceProcessor::updateAvailableVolumeSets() {
    availableVolumeSets_.clear();
    availableVolumeSets_ = volumesetContainer_->getVolumeSetNames();
    volumesetsProp_->setChoices(availableVolumeSets_);

    bool found = false;
    for (size_t i=0; i < availableVolumeSets_.size(); i++)
        if (availableVolumeSets_[i] == volumesetsProp_->get()) {
            found = true;
            break;
        }

    // the last selected set is not in the list, so we select the first one
    if (!found && !availableVolumeSets_.empty())
        volumesetsProp_->set(availableVolumeSets_[0]);
}

void VolumeSetSourceProcessor::updateCurrentVolumeSet(const std::string& volumesetName) {
    // find the VolumeSet for the given name and call setVolumeSet() in order
    // to propagate the information about the currently chosen VolumeSet so
    // that the VolumeSelectionProcessors can update their modalities and
    // timesteps properties.
    if (volumesetContainer_ != 0)
        setVolumeSet(volumesetContainer_->findVolumeSet(volumesetName));
    else
        setVolumeSet(0);

    invalidate();
}

} // namespace
