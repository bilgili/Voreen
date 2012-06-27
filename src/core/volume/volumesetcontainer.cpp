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

VolumeSetContainer::VolumeSetContainer()
    : volumesets_(VolumeSet::VolumeSetSet())
{
}

VolumeSetContainer::~VolumeSetContainer() {
    // can not use clear() here because of postMessage()
    for (VolumeSet::VolumeSetSet::iterator it = volumesets_.begin(); it != volumesets_.end(); ++it)
        delete *it;

    volumesets_.clear();
}

void VolumeSetContainer::clear() {
    MsgDistr.postMessage(new VolumeSetContainerMsg("volumesetcontainer.clear", this));
    for (VolumeSet::VolumeSetSet::iterator it = volumesets_.begin(); it != volumesets_.end(); ++it)
        delete *it;

    volumesets_.clear();
}

bool VolumeSetContainer::addVolumeSet(VolumeSet* volset) {
    if (volset == 0)
        return false;

    std::pair<VolumeSet::VolumeSetSet::iterator, bool> pr = volumesets_.insert(volset);
    bool already = !pr.second;

    // After adding a new VolumeSet, notify all Processors by posting
    // a message so that they can update their properties with available VolumeSets
    if (!already) {
        volset->setParentContainer(this);

        //FIXME: remove this when possible
        if (tgt::Singleton<voreen::MessageDistributor>::isInited() == true)
            MsgDistr.postMessage(new VolumeSetContainerMsg(VolumeSetContainer::msgUpdateVolumeSetContainer_, this));

        notifyObservers();
        return true;
    }
    else {
        return false;
    }
}

VolumeSet* VolumeSetContainer::findVolumeSet(VolumeSet* const volsetWanted) {
    VolumeSet::VolumeSetSet::iterator it = volumesets_.find(volsetWanted);
    return (it != volumesets_.end()) ? *it : 0;
}

VolumeSet* VolumeSetContainer::findVolumeSet(const std::string& name) {
    for (VolumeSet::VolumeSetSet::const_iterator it = volumesets_.begin();
         it != volumesets_.end();
         ++it)
    {
        if ((*it)->getName() == name)
            return *it;
    }
    return 0;
}

VolumeSet* VolumeSetContainer::removeVolumeSet(VolumeSet* const volset) {
    VolumeSet* found = findVolumeSet(volset);
    if (found) {
        volumesets_.erase(volset);
        if (volset->getParentContainer() == this)
            volset->setParentContainer(0);

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
    return removeVolumeSet(findVolumeSet(name));
}

bool VolumeSetContainer::deleteVolumeSet(VolumeSet* const volset) {
    VolumeSet* vs = removeVolumeSet(volset);
    delete vs;
    return (vs != 0);
}

bool VolumeSetContainer::deleteVolumeSet(const std::string& name) {
    return deleteVolumeSet(findVolumeSet(name));
}

std::vector<std::string> VolumeSetContainer::getVolumeSetNames() const {
    std::vector<std::string> names;
    VolumeSet::VolumeSetSet::const_iterator itVS = volumesets_.begin();
    for ( ; itVS != volumesets_.end(); ++itVS) {
        if (!(*itVS)->getName().empty())
            names.push_back((*itVS)->getName());
    }
    return names;
}

const VolumeSet::VolumeSetSet& VolumeSetContainer::getVolumeSets() const {
    return volumesets_;
}

void VolumeSetContainer::notifyObservers() const {
    Observable::notifyObservers();

    if (tgt::Singleton<voreen::MessageDistributor>::isInited() == true)
        MsgDistr.postMessage(new BoolMsg(VolumeSet::msgUpdateVolumeSeries_, true), "VolumeSelectionProcessor");
}

TiXmlElement* VolumeSetContainer::serializeToXml() const {
    TiXmlElement* containerElem = new TiXmlElement(getXmlElementName());
    // Serialize VolumeSets and add them to the container element
    for (VolumeSet::VolumeSetSet::const_iterator it = volumesets_.begin();
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

    for (TiXmlElement* volumesetElem = elem->FirstChildElement(VolumeSet::XmlElementName);
        volumesetElem;
        volumesetElem = volumesetElem->NextSiblingElement(VolumeSet::XmlElementName))
    {
        try {
            VolumeSet* volset = new VolumeSet();
            volset->updateFromXml(volumesetElem);
            errors_.store(volset->errors());
            addVolumeSet(volset);
        }
        catch (SerializerException& e) {
            errors_.store(e);
        }
    }
}

} // namespace
