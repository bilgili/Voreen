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

#include "voreen/core/volume/volumeset.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/volume/volumesetcontainer.h"

#include <sstream>

namespace voreen {

using std::string;
using std::vector;
using std::set;
using std::map;
using std::pair;

const Identifier VolumeSet::msgUpdateVolumeSeries_("update.VolumeSeries");
const string VolumeSet::XmlElementName = "VolumeSet";

VolumeSet::VolumeSet(const string& filename)
    : name_(filename),
      parentContainer_(0)
{}

VolumeSet::~VolumeSet() {
    // delete the modalities on deleting the VolumeSet
    for (VolumeSeries::SeriesSet::iterator it = series_.begin();
        it != series_.end(); ++it)
    {
        delete (*it);
    }
}

bool VolumeSet::operator<(const VolumeSet& volset) const {
    return (name_ < volset.getName());
}

bool VolumeSet::operator==(const VolumeSet& volset) const {
    return (name_ == volset.getName());
}

const string& VolumeSet::getName() const {
    return name_;
}

void VolumeSet::setName(const string& name) {
    name_ = name;
}

const VolumeSetContainer* VolumeSet::getParentContainer() const {
    return parentContainer_;
}

void VolumeSet::setParentContainer(VolumeSetContainer* const parent) {
    parentContainer_ = parent;
}

const VolumeSeries::SeriesSet& VolumeSet::getSeries() const {
    return series_;
}

vector<string> VolumeSet::getSeriesNames() const {
    vector<string> modNames;
    for (VolumeSeries::SeriesSet::const_iterator it = series_.begin(); it != series_.end(); it++) {
        VolumeSeries* volseries = static_cast<VolumeSeries*>(*it);
        if (volseries != 0)
            modNames.push_back(volseries->getName());
    }
    return modNames;
}

bool VolumeSet::addSeries(voreen::VolumeSeries* series, const bool forceInsertion) {
    if (series == 0)
        return false;

    pair<VolumeSeries::SeriesSet::iterator, bool> pr = series_.insert(series);
    bool already = !pr.second;
    map<string, int>::iterator foundModality = modalityCounter_.find(series->getModality().getName());

    if (!already) {
        // If the insertion is successful, increase the counter for the number of
        // VolumeSeries having the same modality as the given series.
        if (foundModality == modalityCounter_.end())
            modalityCounter_.insert(pair<string, int>(series->getModality().getName(), 1));
        else
            foundModality->second++;

        series->setParentSet(this);
        notifyObservers();
        return true;
    }
    else if (already && forceInsertion) {
        // If insertion fails, the modality is already known. If insertion is enforced,
        // build a new name for the series by taking the modalities name and appending its
        // counter to it. Aferwards, the insertion should be successful and the counter for
        // series with this certain modality is increased.
        std::ostringstream num;
        num << (foundModality->second + 1);
        series->setName(foundModality->first + " " + num.str());

        series_.insert(series);
        foundModality->second++;

        series->setParentSet(this);
        notifyObservers();
        return true;
    }
    else {
        // Insertion failed and it was not enforced.
        return false;
    }
}

Volume* VolumeSet::getFirstVolume() const {
    VolumeSeries::SeriesSet::const_iterator first = series_.begin();
    if (first == series_.end())
        return 0;

    VolumeSeries* series = static_cast<VolumeSeries*>(*first);
    if (series == 0)
        return 0;

    VolumeHandle* handle = series->getVolumeHandle(0);
    if (handle == 0)
        return 0;

    return handle->getVolume();
}

vector<VolumeHandle*> VolumeSet::getAllVolumeHandles() const {
    vector<VolumeHandle*> result;
    for (VolumeSeries::SeriesSet::const_iterator itSeries = series_.begin();
        itSeries != series_.end();
        ++itSeries)
    {
        const VolumeSeries* const s = *itSeries;
        if (s == 0)
            continue;

        const VolumeHandle::HandleSet& handles = s->getVolumeHandles();
        for (VolumeHandle::HandleSet::const_iterator itHandles = handles.begin();
            itHandles != handles.end();
            ++itHandles)
        {
            if (*itHandles == 0)
                continue;
            result.push_back(*itHandles);
        }
    }
    return result;
}

void VolumeSet::forceModality(const Modality& modality) {
    VolumeSeries::SeriesSet::iterator it = series_.begin();
    for ( ; it != series_.end(); ++it)
        (*it)->setModality(modality);
}

VolumeSeries* VolumeSet::findSeries(const string& seriesName) {
    VolumeSeries::SeriesSet::const_iterator it = series_.begin();
    for ( ; it != series_.end(); ++it) {
        if ((*it)->getName() == seriesName)
            return *it;
    }
    return 0;
}

VolumeSeries* VolumeSet::findSeries(VolumeSeries* const series) {
    if (series_.find(series) != series_.end())
        return series;
    else
        return 0;
}

vector<VolumeSeries*> VolumeSet::findSeries(const Modality& modality) const {
    vector<VolumeSeries*> result;
    VolumeSeries::SeriesSet::const_iterator it = series_.begin();
    for ( ; it != series_.end(); ++it) {
        if ((*it)->getModality() == modality)
            result.push_back(*it);
    }
    return result;
}

VolumeSeries* VolumeSet::removeSeries(VolumeSeries* const series) {
    VolumeSeries* found = findSeries(series);
    if (found) {
        series_.erase(series);
        if (found->getParentSet() == this)
            found->setParentSet(0);

        notifyObservers();
    }
    return found;
}

VolumeSeries* VolumeSet::removeSeries(const string& name) {
    return removeSeries(findSeries(name));
}

vector<VolumeSeries*> VolumeSet::removeSeries(const Modality& modality) {
    vector<VolumeSeries*> series = findSeries(modality);
    vector<VolumeSeries*>::iterator it = series.begin();
    for ( ; it != series.end(); ++it)
        removeSeries(*it);

    return series;
}

bool VolumeSet::deleteSeries(VolumeSeries* const series) {
    VolumeSeries* vs = removeSeries(series);
    delete vs;
    return (vs != 0);
}

bool VolumeSet::deleteSeries(const string& name) {
    return deleteSeries(findSeries(name));
}

bool VolumeSet::deleteSeries(const Modality& modality) {
    vector<VolumeSeries*> series = removeSeries(modality);
    vector<VolumeSeries*>::iterator it = series.begin();
    for ( ; it != series.end(); ++it)
        delete *it;

    return (series.size() > 0);
}

TiXmlElement* VolumeSet::serializeToXml() const {
    TiXmlElement* setElem = new TiXmlElement(getXmlElementName());
    // Serialize Data
    setElem->SetAttribute("name", name_);
    // Serialize VolumeSeries and add them to the set element
    for (VolumeSeries::SeriesSet::const_iterator it = series_.begin(); it != series_.end(); it++)
        setElem->LinkEndChild((*it)->serializeToXml());
    return setElem;
}

void VolumeSet::updateFromXml(TiXmlElement* elem) {
    errors_.clear();
    // deserialize Set
    if (!elem->Attribute("name"))
        throw XmlAttributeException("Attribute 'name' missing on VolumeSet element");
    setName(elem->Attribute("name"));

    // deserialize VolumeSeries
    TiXmlElement* volumeseriesElem;
    for (volumeseriesElem = elem->FirstChildElement(VolumeSeries::XmlElementName);
        volumeseriesElem;
        volumeseriesElem = volumeseriesElem->NextSiblingElement(VolumeSeries::XmlElementName))
    {
        VolumeSeries* series = 0;
        try {
            series = new VolumeSeries();
            series->updateFromXml(volumeseriesElem);
            errors_.store(series->errors());
            addSeries(series);
        }
        catch (SerializerException& e) {
            delete series;
            errors_.store(e);
        }
    }
}

set<string> VolumeSet::getFileNamesFromXml(TiXmlElement* elem) {
    set<string> filenames;
    // get all Filenames from the Series
    TiXmlElement* volumeseriesElem;
    for (volumeseriesElem = elem->FirstChildElement(VolumeSeries::XmlElementName);
        volumeseriesElem;
        volumeseriesElem = volumeseriesElem->NextSiblingElement(VolumeSeries::XmlElementName))
    {
        set<string> seriesfilenames = VolumeSeries::getFileNamesFromXml(volumeseriesElem);
        filenames.insert(seriesfilenames.begin(), seriesfilenames.end());
    }
    return filenames;
}

void VolumeSet::notifyObservers() {
    if (parentContainer_)
        parentContainer_->notifyObservers();
}

} // namespace
