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
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"

namespace voreen {

// static members
//
const Identifier VolumeSetContainer::msgUpdateVolumeSetContainer_("update.VolumeSetContainer");
const Identifier VolumeSetContainer::msgSetVolumeSetContainer_("set.VolumeSetContainer");
const std::string VolumeSetContainer::XmlElementName = "VolumeSetContainer";

VolumeSetContainer::VolumeSetContainer() {
}

VolumeSetContainer::~VolumeSetContainer() {
    // can not use clear() here because of postMessage()
    for (VolumeSet::VSPSet::iterator it = volumesets_.begin(); it != volumesets_.end(); ++it)
        delete *it;

    volumesets_.clear();
}

void VolumeSetContainer::clear() {
	MsgDistr.postMessage(new VolumeSetContainerMsg("volumesetcontainer.clear", this));
	for (VolumeSet::VSPSet::iterator it = volumesets_.begin(); it != volumesets_.end(); ++it)
        delete *it;

    volumesets_.clear();
}

bool VolumeSetContainer::addVolumeSet(VolumeSet*& volset) {
    if (volset == 0)
        return false;

    std::pair<VolumeSet::VSPSet::iterator, bool> pr = volumesets_.insert(volset);

    // After adding a new VolumeSet, notify all Processors by posting
    // a message so that they can update their properties with available VolumeSets
    // 
    if (pr.second) {
        volset->setParentContainer(this);

        if (tgt::Singleton<voreen::MessageDistributor>::isInited() == true)
            MsgDistr.postMessage(new VolumeSetContainerMsg(VolumeSetContainer::msgUpdateVolumeSetContainer_, this));

        notifyObservers();
    }
    else {
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
    if ( volumesets_.empty() || (volsetWanted == 0) )
        return 0;

    VolumeSet::VSPSet::iterator it = volumesets_.find(volsetWanted);
    if (it != volumesets_.end())
        return *it;

    return 0;
}

VolumeSet* VolumeSetContainer::findVolumeSet(const std::string& name) {
    VolumeSet volset(0, name);
    return findVolumeSet(&volset);
}

bool VolumeSetContainer::containsVolumeSet(VolumeSet* const volset) {
   return (findVolumeSet(volset) != 0);
}

VolumeSet* VolumeSetContainer::removeVolumeSet(VolumeSet* const volset) {
    VolumeSet* found = findVolumeSet(volset);
    if (found != 0) {
        volumesets_.erase(volset);
        found->setParentContainer(0);

        // As the content of the VolumeSetContainer has changed, the responsible processors
        // need to update their properties. Therefore this message is sent.
        //
        if (tgt::Singleton<voreen::MessageDistributor>::isInited() == true)
            MsgDistr.postMessage(new VolumeSetContainerMsg(VolumeSetContainer::msgUpdateVolumeSetContainer_, this));

        notifyObservers();
    }

    return found;
}

VolumeSet* VolumeSetContainer::removeVolumeSet(const std::string& name) {
    VolumeSet volset(0, name);
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
    VolumeSet volset(0, name);
    return deleteVolumeSet(&volset);
}

std::vector<std::string> VolumeSetContainer::getVolumeSetNames() const {
    std::vector<std::string> names;
    VolumeSet::VSPSet::const_iterator itVS = volumesets_.begin();
    for (VolumeSet::VSPSet::const_iterator itVS = volumesets_.begin();
        itVS != volumesets_.end();
        ++itVS)
    {
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

void VolumeSetContainer::notifyObservers() const {
    Observable::notifyObservers();

    if (tgt::Singleton<voreen::MessageDistributor>::isInited() == true)
        MsgDistr.postMessage(new BoolMsg(VolumeSet::msgUpdateVolumeSeries_, true), "VolumeSelectionProcessor");
}

// ---------------------------------------------------------------------------

TiXmlElement* VolumeSetContainer::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* containerElem = new TiXmlElement(getXmlElementName());
    // Serialize VolumeSets and add them to the container element
    for (VolumeSet::VSPSet::const_iterator it = volumesets_.begin();
        it != volumesets_.end();
        it++)
    {
        containerElem->LinkEndChild((*it)->serializeToXml());
    }
    return containerElem;
}

void VolumeSetContainer::updateFromXml(TiXmlElement* elem) {
    clear();
    errors_.clear();
    serializableSanityChecks(elem);
    // get all the filenames of loaded Files
    std::set<std::string> filenames;
    TiXmlElement* volumesetElem;
    for (volumesetElem = elem->FirstChildElement(VolumeSet::XmlElementName);
        volumesetElem;
        volumesetElem = volumesetElem->NextSiblingElement(VolumeSet::XmlElementName))
    {
        std::set<std::string> volsetfilenames = VolumeSet::getFileNamesFromXml(volumesetElem);
        filenames.insert(volsetfilenames.begin(), volsetfilenames.end());
    }
    
    // load these files into VolumeSets and get the Handles from them
    std::vector<VolumeHandle*> originalhandles;
    // need VolumeSerializerPopulator
    // FIXME there's got to be a better way than creating my own
    VolumeSerializerPopulator* populator = new VolumeSerializerPopulator();
    VolumeSerializer* serializer = populator->getVolumeSerializer();

    std::set<std::string>::iterator it;
    for (it = filenames.begin(); it != filenames.end(); ++it) {
        try {
            std::string filename = *it;
            VolumeSet* tempvolumeset = serializer->load(filename);
            std::vector<VolumeHandle*> handles = tempvolumeset->getAllVolumeHandles();
            // remove all handles from parent series
            size_t i;
            for (i=0;i<handles.size();++i) {
                handles.at(i)->getParentSeries()->removeVolumeHandle(handles.at(i));
            }
            // now the temporary volumeset can safely be deleted
            delete tempvolumeset;
            originalhandles.insert(originalhandles.end(), handles.begin(), handles.end());
        }
        catch (tgt::Exception& e) {
            errors_.store(e); // TODO I should instead store my own Exception - d_kirs04
        }
    }
    delete populator;

    // Store all Volumes in a Map that maps Origin -> (Volume, VolumeUsed?)
    std::map<VolumeHandle::Origin, std::pair<Volume*, bool> > volumeMap;
    size_t j;
    for (j=0; j<originalhandles.size(); ++j) {
        VolumeHandle* handle = originalhandles.at(j);
        volumeMap.insert(std::make_pair(handle->getOrigin(), std::make_pair(handle->getVolume(), false)));
    }
    // now deserialize VolumeSets
    for (volumesetElem = elem->FirstChildElement(VolumeSet::XmlElementName);
        volumesetElem;
        volumesetElem = volumesetElem->NextSiblingElement(VolumeSet::XmlElementName))
    {
        try {
            VolumeSet* volset = new VolumeSet();
            volset->updateFromXml(volumesetElem, volumeMap);
            errors_.store(volset->errors());
            addVolumeSet(volset);
        }
        catch (SerializerException& e) {
            errors_.store(e);
        }
    }
    // delete all unused volumes
    std::map<VolumeHandle::Origin, std::pair<Volume*, bool> >::iterator ite = volumeMap.begin();
    while (ite != volumeMap.end()) {
        if (!ite->second.second) { // This Volume is unused
            delete ite->second.first;
            volumeMap.erase(ite++); // The iterator is increased before the element is erased so this should work
        }
        else
            ++ite;
    }
}

} // namespace
