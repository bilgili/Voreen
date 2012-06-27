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

#include "voreen/core/vis/processors/volumeselectionprocessor.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/volume/volumeset.h"
#include "voreen/core/volume/bricking/largevolumemanager.h"

namespace voreen {

// initialization of static members
const Identifier VolumeSelectionProcessor::msgSetCurrentModality_("set.currentModality");
const Identifier VolumeSelectionProcessor::msgSetCurrentTimestep_("set.currentTimestep");

VolumeSelectionProcessor::VolumeSelectionProcessor()
    : Processor()
    , curVolumeSet_(0)
    , volumeSeries_(0)
    , volumeHandle_(0)
    , timestepProp_(msgSetCurrentTimestep_, "Timestep", 0, 0, 100, false)
    , volumeSetInportName_("volumeset.volumeset")
    , volumeOutportName_("volumehandle.volumehandle")
{
    modalityProp_ = new StringSelectionProp(msgSetCurrentModality_, "Modality", true);
    modalityProp_->onChange(
        CallMemberAction<VolumeSelectionProcessor>(this, &VolumeSelectionProcessor::currentSeriesChanged));
    addProperty(modalityProp_);
      
    timestepProp_.onChange(
        CallMemberAction<VolumeSelectionProcessor>(this, &VolumeSelectionProcessor::currentTimestepChanged));
    addProperty(&timestepProp_);

    // These properties shall not be serialized as they are set, when a VolumeSet* is
    // associated with a VolumeSetSourceProcessor. This does not happen during
    // loading! The stored indices and values would be applied to an empty property and
    // would cause the application to crash on loading networks!
    timestepProp_.setSerializable(true);

    createInport(volumeSetInportName_);
    createGenericOutport(volumeOutportName_, &volumeHandle_);
    setName("VolumeSelector");
    setTag("VolumeSelectionProcessor");
    enableVolumeCaching(false);
}

VolumeSelectionProcessor::~VolumeSelectionProcessor() {
    delete modalityProp_;
}

const Identifier VolumeSelectionProcessor::getClassName() const {
    return "VolumeProcessor.VolumeSelector";
}

const std::string VolumeSelectionProcessor::getProcessorInfo() const {
    return "Selects a single volume from a volume set.";
}

Processor* VolumeSelectionProcessor::create() const {
    return new VolumeSelectionProcessor();
}

void VolumeSelectionProcessor::process(LocalPortMapping* portMapping) {
    if (portMapping == 0)
        return;

    VolumeSet** volumesetAddr = portMapping->getGenericData<VolumeSet**>(volumeSetInportName_);
    if (volumesetAddr != 0 && *volumesetAddr != curVolumeSet_)
        setVolumeSet(*volumesetAddr);
    else if (volumesetAddr == 0 && *volumesetAddr != curVolumeSet_)
        setVolumeSet(0);
}

void VolumeSelectionProcessor::processMessage(Message* msg, const Identifier& dest) {
    if (msg == 0)
        return;

    Processor::processMessage(msg, dest);
    /*
    if (msg->id_ == msgSetCurrentTimestep_)
        setCurrentTimestep(msg->getValue<int>());
    else if (msg->id_ == msgSetCurrentModality_)
        setCurrentSeries(msg->getValue<std::string>());
    else
    */
    if (msg->id_ == VolumeSet::msgUpdateVolumeSeries_) {
        updateAvailableModalities();
        updateAvailableTimesteps();
    }
    else if (msg->id_ == VolumeSeries::msgUpdateTimesteps_)
        updateAvailableTimesteps();

    // if volumesetcontainer is cleared don't use its volumeset
    else if (msg->id_ == "volumesetcontainer.clear") {
        VolumeSetContainer* volsetCont = msg->getValue<VolumeSetContainer*>();
        if (curVolumeSet_ && curVolumeSet_->getParentContainer() == volsetCont) {
            setVolumeSet(0);
            setVolumeSeries(0);
            setVolumeHandle(0);
        }
        invalidate();
    } else if (msg->id_ == VoreenPainter::switchCoarseness_) {
        if (volumeHandle_) {
		    LargeVolumeManager* lvm = volumeHandle_->getLargeVolumeManager();
		    bool coarseness = msg->getValue<bool>();
            if (lvm) {
			    lvm->postMessage(new BoolMsg(VoreenPainter::switchCoarseness_,coarseness));
            }
        }
	} else if (msg->id_ == VoreenPainter::cameraChanged_) {
		if (volumeHandle_) {
			LargeVolumeManager* lvm = volumeHandle_->getLargeVolumeManager();	
			tgt::Camera* camera = msg->getValue<tgt::Camera*>();
            if (lvm) {
				lvm->postMessage(new CameraPtrMsg(VoreenPainter::cameraChanged_,camera ) );
            }
		}
	}
}

VolumeHandle** VolumeSelectionProcessor::getVolumeHandleAddress() {
    return &volumeHandle_;
}

void VolumeSelectionProcessor::setVolumeSet(VolumeSet* const volumeset) {
    if (curVolumeSet_ != volumeset)
        curVolumeSet_ = volumeset;

    updateAvailableModalities();
    updateAvailableTimesteps();
    currentSeriesChanged();
}

void VolumeSelectionProcessor::setVolumeSeries(VolumeSeries* const volumeSeries) {
    if (volumeSeries != volumeSeries_)
        volumeSeries_ = volumeSeries;
}

void VolumeSelectionProcessor::setVolumeHandle(VolumeHandle* const volumeHandle) {
    if (!volumeHandle_ || !volumeHandle_->isIdentical(volumeHandle))
        volumeHandle_ = volumeHandle;
}

void VolumeSelectionProcessor::currentSeriesChanged() {
    setCurrentSeries(modalityProp_->get());
    invalidate();
}

void VolumeSelectionProcessor::currentTimestepChanged() {
    setCurrentTimestep(timestepProp_.get());
    invalidate();
}

void VolumeSelectionProcessor::updateAvailableModalities() {
    availableModalities_.clear();

    if (curVolumeSet_ != 0) {
        VolumeSeries::SeriesSet series = curVolumeSet_->getSeries();
        for (VolumeSeries::SeriesSet::const_iterator it = series.begin(); it != series.end(); it++) {
            VolumeSeries* volseries = static_cast<VolumeSeries*>(*it);
            availableModalities_.push_back(volseries->getModality().getName());
        }
    }
    modalityProp_->setChoices(availableModalities_);

    currentSeriesChanged();
}

void VolumeSelectionProcessor::updateAvailableTimesteps() {
    int numHandles = 1;
    if (volumeSeries_ != 0)
        numHandles = volumeSeries_->getNumVolumeHandles();
    else
        return;

    int curTimestep = timestepProp_.get();
     if (curTimestep >= numHandles)
         curTimestep = 0;
//    timestepProp_.setMinValue(0);
//     timestepProp_.setMaxValue(numHandles - 1);
    timestepProp_.set(curTimestep);

    if (volumeSeries_ == 0)
        curTimestep = -1;

    setCurrentTimestep(curTimestep);
}

void VolumeSelectionProcessor::setCurrentSeries(const std::string& seriesName) {
    volumeSeries_ = 0;

    if (curVolumeSet_ == 0) {
        setCurrentTimestep(-1);
        return;
    }

    if (seriesName.empty()) {
        // When series name is empty we first try to find a series with an empty name, then we
        // try "unknown", then we take the first one we get.
        volumeSeries_ = curVolumeSet_->findSeries("");

        if (!volumeSeries_) 
            volumeSeries_ = curVolumeSet_->findSeries("unknown");

        if (!volumeSeries_) {
            VolumeSeries::SeriesSet series = curVolumeSet_->getSeries();
            if (series.begin() != series.end()) 
                volumeSeries_ = *(series.begin());
        }
    }
    else
        volumeSeries_ = curVolumeSet_->findSeries(seriesName);

    updateAvailableTimesteps();
}

void VolumeSelectionProcessor::setCurrentTimestep(int timestepIndex) {
    if (volumeSeries_ != 0 && timestepIndex >= 0)
        setVolumeHandle(volumeSeries_->getVolumeHandle(timestepIndex));
    else
        setVolumeHandle(0);
}

} // namespace voreen
