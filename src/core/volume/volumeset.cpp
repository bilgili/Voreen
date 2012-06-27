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

#include "voreen/core/volume/volumeset.h"
#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

const Identifier VolumeSet::msgUpdateVolumeSeries_("update.VolumeSeries");

VolumeSet::VolumeSet()
  : name_("")
{
}

VolumeSet::VolumeSet(const std::string& filename)
  : name_(filename)
{
}

VolumeSet::~VolumeSet() {
    // delete the modalities on deleting the VolumeSet
    VolumeSeries::SeriesSet::iterator it;
    for (it = series_.begin(); it != series_.end(); ++it)
        delete (*it);
    series_.clear();
}

bool VolumeSet::operator<(const VolumeSet& volset) const {
    return (name_ < volset.getName());
}

bool VolumeSet::operator==(const VolumeSet& volset) const {
    return (name_ == volset.getName());
}

const std::string& VolumeSet::getName() const {
    return name_;
}

const VolumeSeries::SeriesSet& VolumeSet::getSeries() const {
    return series_;
}

std::vector<std::string> VolumeSet::getSeriesNames() const {
    std::vector<std::string> modNames;
    VolumeSeries::SeriesSet::const_iterator it = series_.begin();
    for ( ; it != series_.end(); it++) {
        VolumeSeries* volseries = static_cast<VolumeSeries*>(*it);
        if (volseries != 0)
            modNames.push_back( volseries->getName() );
    }
    return modNames;
}

bool VolumeSet::addSeries(voreen::VolumeSeries*& series, const bool forceInsertion) {
    if (series == 0)
        return false;

    std::pair<VolumeSeries::SeriesSet::iterator, bool> pr = series_.insert(series);
    std::map<std::string, int>::iterator found = modalityCounter_.find(series->getModality().getName());

    // If the insertion is successful, increase the counter for the number of 
    // VolumeSeries having the same modality as the given series.
    //
    if (pr.second) {
        if (found == modalityCounter_.end())
            modalityCounter_.insert( std::pair<std::string, int>(series->getModality().getName(), 1) );
        else
            found->second++;
        (*(pr.first))->setParentVolumeSet(this);
        MsgDistr.postMessage(new BoolMsg(VolumeSet::msgUpdateVolumeSeries_, true), "VolumeSelectionProcessor");
        return true;
    }

    // If insertion fails, the modality is already known and insertion is enforced,
    // build a new name for the series by taking the modalities name and appending its
    // counter to it. Aferwards, the insertion should be successful and the counter for
    // series with this certain modality is increased.
    //
    if (forceInsertion && (found != modalityCounter_.end()) ) {
        std::string name(found->first);
        char c[12] = {0};
        sprintf(c, " %d", (found->second + 1));
        name += c;
        series->setName(name);
        pr = series_.insert(series);
        if (pr.second) {
            (*(pr.first))->setParentVolumeSet(this);
            found->second++;
        }
    }

    // if insertion fails, the series already exists! If insertion is enforced
    // but failed nevertheless, some serious problem has probably occured.
    //
    if (!pr.second) {
        // Replace the value in series by the value from the VolumeSet if they
        // are not identical!
        //
        VolumeSeries* containedSeries = static_cast<VolumeSeries*>(*(pr.first));
        if (series != containedSeries) {
            delete series;
            series = containedSeries;
        }
    } else
        MsgDistr.postMessage(new BoolMsg(VolumeSet::msgUpdateVolumeSeries_, true), "VolumeSelectionProcessor");

    return pr.second;
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

std::vector<VolumeHandle*> VolumeSet::getAllVolumeHandles() const {
    std::vector<VolumeHandle*> result;
    VolumeSeries::SeriesSet::const_iterator itSeries = series_.begin();
    for ( ; itSeries != series_.end(); ++itSeries) {
        const VolumeSeries* const s = *itSeries;
        if( s == 0 )
            continue;

        const VolumeHandle::HandleSet& handles = s->getVolumeHandles();
        VolumeHandle::HandleSet::const_iterator itHandles = handles.begin();
        for ( ; itHandles != handles.end(); ++itHandles ) {
            if(*itHandles == 0)
                continue;
            result.push_back( *itHandles );
        }
    }
    return result;
}

void VolumeSet::forceModality(const Modality& modality) {
    // in order to force the series to become of the given type,
    // all series are removed and their handles are collected
    // into a new series of the wanted type!
    //
    VolumeSeries* newSeries = new VolumeSeries(this, modality.getName(), modality);
    newSeries->setModality(modality);
printf("\tVolumeSet::forceSeries()...\n");
printf("\t\tforcing series to become '%s'\n", modality.getName().c_str());
    VolumeSeries::SeriesSet::iterator it = series_.begin();
    for( ; it != series_.end(); ++it) {
        VolumeSeries* series = *it;
        if (series == 0)
            continue;
printf("\t\tconverting series from '%s'...\n", series->getName().c_str());
        const VolumeHandle::HandleSet& handles = series->getVolumeHandles();
        VolumeHandle::HandleSet::const_iterator itHandles = handles.begin();

        // remove all VolumeHandles from the series and add them to the new
        // series. Therefore the deletion of the VolumeSeries does not cause
        // the VolumeHadle to become deleted and copying is not necessary.
        //
        for( ; itHandles != handles.end(); ) {
            VolumeHandle* handle = series->removeVolumeHandle(*(itHandles++));
            newSeries->addVolumeHandle(handle);
        }

        delete series;
    }
printf("\n");
    // Clear all existing series and add the new one to it.
    //
    series_.clear();
    series_.insert(newSeries);
}

VolumeSeries* VolumeSet::findSeries(VolumeSeries* const series) {
    if (series == 0)
        return 0;

    VolumeSeries::SeriesSet::iterator it = series_.find(series);
    if (it != series_.end())
        return (*it);

    return 0;
}

VolumeSeries* VolumeSet::findSeries(const std::string& seriesName) {
    VolumeSeries volseries(0, seriesName);
    return findSeries(&volseries);
}

std::vector<VolumeSeries*> VolumeSet::findSeries(const Modality& modality) const {
    std::vector<VolumeSeries*> result;

    VolumeSeries::SeriesSet::const_iterator it = series_.begin();
    for ( ; it != series_.end(); ++it) {
        if (*it == 0)
            continue;

        if ((*it)->getModality() == modality)
            result.push_back(*it);
    }
    return result;
}

VolumeSeries* VolumeSet::removeSeries(VolumeSeries* const series) {
    VolumeSeries* found = findSeries(series);
    if (found != 0) {
        series_.erase(series);
        MsgDistr.postMessage(new BoolMsg(VolumeSet::msgUpdateVolumeSeries_, true), "VolumeSelectionProcessor");
    }
    return found;
}

VolumeSeries* VolumeSet::removeSeries(const std::string& name) {
    VolumeSeries series(0, name);
    return removeSeries(&series);
}

std::vector<VolumeSeries*> VolumeSet::removeSeries(const Modality& modality) {
    std::vector<VolumeSeries*> series = findSeries(modality);
    std::vector<VolumeSeries*>::iterator it = series.begin();
    for ( ; it != series.end(); ++it)
        series_.erase(*it);
    return series;
}

bool VolumeSet::deleteSeries(VolumeSeries* const series) {
    VolumeSeries* vs = removeSeries(series);
    if (vs == 0)
        return false;

    delete vs;
    return true;
}

bool VolumeSet::deleteSeries(const std::string& name) {
    VolumeSeries series(0, name);
    return deleteSeries(&series);
}

bool VolumeSet::deleteSeries(const Modality& modality) {
    std::vector<VolumeSeries*> series = removeSeries(modality);
    std::vector<VolumeSeries*>::iterator it = series.begin();
    bool result = false;
    for ( ; it != series.end(); ++it) {
        if (*it == 0)
            continue;

        result = true;
        delete *it;
    }
    return result;
}

} // namespace
