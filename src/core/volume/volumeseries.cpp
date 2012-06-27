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

#include "voreen/core/volume/volumeseries.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/volume/volumesetcontainer.h"

namespace voreen {

const Identifier VolumeSeries::msgUpdateTimesteps_("update.Timesteps");
const std::string VolumeSeries::XmlElementName = "VolumeSeries";

VolumeSeries::VolumeSeries(const std::string& name, const Modality& modality)
    : name_(name),
      modality_(modality),
      maximumTimestep_(-1.0f),
      parentSet_(0)
{}

VolumeSeries::VolumeSeries(const VolumeSeries& series)
  : Serializable()
{
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

bool VolumeSeries::setName(const std::string& name) {
    if (name != name_) {
        // change the name only if there is no
        // VolumeSeries of the same name already contained
        // within a possibly existing parent VolumeSet!
        // The names need to be unique.
        if (parentSet_ != 0 && parentSet_->findSeries(name) == 0) {
            name_ = name;
            notifyObservers();
            return true;
        }
        if (parentSet_ == 0) {
            name_ = name;
            notifyObservers();
            return true;
        }
    }
    return false;
}

std::string VolumeSeries::getLabel() const {
    return name_ + " (" + modality_.getName() + ")";
}

const Modality& VolumeSeries::getModality() const {
    return modality_;
}

void VolumeSeries::setModality(const Modality& modality) {
    if (modality != modality_) {
        modality_ = modality;
        notifyObservers();
    }
}

VolumeSet* VolumeSeries::getParentSet() const {
    return parentSet_;
}

void VolumeSeries::setParentSet(VolumeSet* const volumeSet) {
    parentSet_ = volumeSet;
}

const VolumeHandle::HandleSet& VolumeSeries::getVolumeHandles() const {
    return handles_;
}

//FIXME: does not do what you want. joerg
VolumeHandle* VolumeSeries::getVolumeHandle(const int index) const {
    if (index < 0 || static_cast<size_t>(index) >= handles_.size())
        return 0;

    VolumeHandle::HandleSet::const_iterator it = handles_.begin();
    for (int i = 0; it != handles_.end(); ++it, ++i) {
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

bool VolumeSeries::addVolumeHandle(VolumeHandle* handle, const bool forceInsertion) {
    if (handle == 0)
        return false;

    std::pair<VolumeHandle::HandleSet::iterator, bool> pr = handles_.insert(handle);
    bool already = !pr.second;

    if (!already) {
        // If the handle has been inserted, the maximum timestep eventually needs to be
        // updated and success is indicated by returning "true"
        if (handle->getTimestep() > maximumTimestep_)
            maximumTimestep_ = handle->getTimestep();
        (*(pr.first))->setParentSeries(this);

        notifyObservers();
        return true;
    }
    else if (already && forceInsertion) {
        // If the insertion is forced and the handle has not been inserted yet, its timestep
        // must be already contained. So update its timestep to maximum + 1.0f. Insertion
        // by then should never fail.
        maximumTimestep_ += 1.0f;
        handle->setTimestep(maximumTimestep_);
        handles_.insert(handle);

        (*(pr.first))->setParentSeries(this);
        notifyObservers();
        return true;
    }
    else {
        // Insertion failed and it was not enforced.
        return false;
    }
}

VolumeHandle* VolumeSeries::findVolumeHandle(VolumeHandle* const handle) const {
    if (handles_.find(handle) != handles_.end())
        return handle;
    else
        return 0;
}

VolumeHandle* VolumeSeries::findVolumeHandle(const float timestep) const {
    VolumeHandle::HandleSet::const_iterator it = handles_.begin();
    for ( ; it != handles_.end(); ++it) {
        if ((*it)->getTimestep() == timestep)
            return *it;
    }
    return 0;
}

VolumeHandle* VolumeSeries::removeVolumeHandle(VolumeHandle* const handle) {
    VolumeHandle* found = findVolumeHandle(handle);
    if (found) {
        handles_.erase(handle);
        if (found->getParentSeries() == this)
            found->setParentSeries(0);
        adjustMaximumTimestep();

        notifyObservers();
    }
    return found;
}

VolumeHandle* VolumeSeries::removeVolumeHandle(const float timestep) {
    return removeVolumeHandle(findVolumeHandle(timestep));
}

bool VolumeSeries::deleteVolumeHandle(VolumeHandle* const handle) {
    VolumeHandle* found = removeVolumeHandle(handle);
    delete found;
    return (found != 0);
}

bool VolumeSeries::deleteVolumeHandle(const float timestep) {
    return deleteVolumeHandle(findVolumeHandle(timestep));
}

void VolumeSeries::timestepChanged(VolumeHandle* /*handle*/) {
    adjustMaximumTimestep();
    notifyObservers();
}

TiXmlElement* VolumeSeries::serializeToXml() const {
    TiXmlElement* seriesElem = new TiXmlElement(getXmlElementName());
    // Serialize Data
    seriesElem->SetAttribute("name", name_);
    seriesElem->SetAttribute("modality", modality_.getName());
    // Serialize VolumeHandles and add them to the series' element
    VolumeHandle::HandleSet::const_iterator it;
    for (it = handles_.begin(); it != handles_.end(); it++)
        seriesElem->LinkEndChild((*it)->serializeToXml());
    return seriesElem;
}

void VolumeSeries::updateFromXml(TiXmlElement* elem) {
    errors_.clear();
    // deserialize series
    if (!(elem->Attribute("name") &&
          elem->Attribute("modality")))
        throw XmlAttributeException("Attributes missing on VolumeSeries element"); // TODO Better Exception
    setName(elem->Attribute("name"));
    setModality(Modality(elem->Attribute("modality")));

    // deserialize VolumeHandles
    TiXmlElement* volumehandleElem;
    for (volumehandleElem = elem->FirstChildElement(VolumeHandle::XmlElementName);
        volumehandleElem;
        volumehandleElem = volumehandleElem->NextSiblingElement(VolumeHandle::XmlElementName))
    {
        VolumeHandle* handle = 0;
        try {
            handle = new VolumeHandle(0, 0);
            handle->updateFromXml(volumehandleElem);
            errors_.store(handle->errors());
            addVolumeHandle(handle);
        }
        catch (SerializerException& e) {
            delete handle;
            errors_.store(e);
        }
    }
}

std::set<std::string> VolumeSeries::getFileNamesFromXml(TiXmlElement* elem) {
    std::set<std::string> filenames;
    // get all Filenames from the Series
    TiXmlElement* volumehandleElem;
    for (volumehandleElem = elem->FirstChildElement(VolumeHandle::XmlElementName);
        volumehandleElem;
        volumehandleElem = volumehandleElem->NextSiblingElement(VolumeHandle::XmlElementName))
    {
        try {
            std::string filename = VolumeHandle::getFileNameFromXml(volumehandleElem);
            filenames.insert(filename);
        }
        catch (SerializerException& /*e*/) {
        }
    }
    return filenames;
}

void VolumeSeries::adjustMaximumTimestep() {
    VolumeHandle::HandleSet::const_iterator it = handles_.begin();
    float max = -1.0f;
    for (VolumeHandle::HandleSet::const_iterator it = handles_.begin(); it != handles_.end(); ++it ) {
        if ( (*it != 0) && ((*it)->getTimestep() > max) )
            max = (*it)->getTimestep();
    }
    maximumTimestep_ = max;
}

void VolumeSeries::clone(const VolumeSeries& series) {
    name_ = series.getName();
    parentSet_ = series.getParentSet();
    modality_ = series.getModality();

    freeHandles();
    handles_ = series.getVolumeHandles();
}

void VolumeSeries::freeHandles() {
    for (VolumeHandle::HandleSet::iterator it = handles_.begin(); it != handles_.end(); ++it)
        delete *it;
    handles_.clear();
}

void VolumeSeries::notifyObservers() {
    if (parentSet_ != 0 && parentSet_->getParentContainer() != 0)
            parentSet_->getParentContainer()->notifyObservers();
}

} // namespace voreen
