/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/core/properties/property.h"
#include "voreen/core/properties/propertywidget.h"
#include "voreen/core/properties/link/linkevaluatorhelper.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/voreenmodule.h"

namespace voreen {

Property::Property(const std::string& id, const std::string& guiText, int invalidationLevel)
    : VoreenSerializableObject(id, guiText)
    , owner_(0)
    , invalidationLevel_(invalidationLevel)
    , widgetsEnabled_(true)
    , visible_(true)
    , lod_(USER)
    , views_(DEFAULT)
    , groupId_("")
    , groupName_("")
    , interactionModeVisited_(false)
    , serializeValue_(false)
    , linkCheckVisited_(false)
    , initialGuiName_(guiText)
{
//    tgtAssert(!id.empty(), "Property's id must not be empty");
}

Property::Property()
    : VoreenSerializableObject()
    , owner_(0)
    , invalidationLevel_(Processor::INVALID_RESULT)
    , widgetsEnabled_(true)
    , visible_(true)
    , lod_(USER)
    , views_(DEFAULT)
    , groupId_("")
    , groupName_("")
    , interactionModeVisited_(false)
    , serializeValue_(false)
    , linkCheckVisited_(false)
    , initialGuiName_("")
{}

Property::~Property() {
    disconnectWidgets();
    std::set<PropertyWidget*>::iterator it = widgets_.begin();
    for ( ; it != widgets_.end(); ++it)
        delete (*it);
}

int Property::getInvalidationLevel() const {
    return invalidationLevel_;
}

void Property::setInvalidationLevel(int invalidationLevel) {
    invalidationLevel_ = invalidationLevel;
}

void Property::setWidgetsEnabled(bool enabled) {
    widgetsEnabled_ = enabled;
    for (std::set<PropertyWidget*>::iterator it = widgets_.begin(); it != widgets_.end(); ++it)
        (*it)->setEnabled(widgetsEnabled_);
}

bool Property::getWidgetsEnabled() const {
    return widgetsEnabled_;
}

void Property::initialize() throw (tgt::Exception) {
    // currently nothing to do
}

void Property::deinitialize() throw (tgt::Exception) {
    // currently nothing to do
}

void Property::setVisible(bool state) {
    visible_ = state;

    // adjust visibility of assigned gui widgets
    std::set<PropertyWidget*>::iterator it = widgets_.begin();
    for ( ; it != widgets_.end(); ++it)
        (*it)->setVisible(state);}

bool Property::isVisible() const {
    return visible_;
}

void Property::setViews(View views) {
    views_ = views;
}

Property::View Property::getViews() const {
    return views_;
}

void Property::setGroupID(const std::string& gid) {
    groupId_ = gid;
}

std::string Property::getGroupID() const {
    return groupId_;
}

void Property::setGroupName(const std::string& name) {
    groupName_ = name;
}

std::string Property::getGroupName() const {
    return groupName_;
}

void Property::setOwner(PropertyOwner* processor) {
    owner_ = processor;
}

PropertyOwner* Property::getOwner() const {
    return owner_;
}

void Property::addWidget(PropertyWidget* widget) {
    if (widget) {
        if (!visible_)
            widget->setVisible(visible_);
        if (!widgetsEnabled_)
            widget->setEnabled(false);

        widgets_.insert(widget);
    }
}

void Property::removeWidget(PropertyWidget* widget) {
    if (widget)
        widgets_.erase(widget);
}

void Property::disconnectWidgets() {
    std::set<PropertyWidget*>::iterator it = widgets_.begin();
    for ( ; it != widgets_.end(); ++it)
        (*it)->disconnect();
}

void Property::updateWidgets() {
    std::set<PropertyWidget*>::iterator it = widgets_.begin();
    for ( ; it != widgets_.end(); ++it)
        (*it)->updateFromProperty();
}

std::string Property::getFullyQualifiedID() const {
    if (getOwner())
        return getOwner()->getID() + "." + getID();
    else
        return getID();
}

std::string Property::getFullyQualifiedGuiName() const {
    if (getOwner())
        return getOwner()->getGuiName() + "." + getGuiName();
    else
        return getGuiName();
}

void Property::serialize(XmlSerializer& s) const {
    if(serializeValue_)
        return;

    s.serialize("name", id_);

    if (guiName_ != initialGuiName_)
        s.serialize("guiName", guiName_);

    if (lod_ != USER)
        s.serialize("lod", lod_);

    for (std::set<PropertyWidget*>::const_iterator it = widgets_.begin(); it != widgets_.end(); ++it) {
        (*it)->updateMetaData();
        // FIXME What exactly is this supposed to do? The return value is not used... FL
        (*it)->getWidgetMetaData();
    }

    metaDataContainer_.serialize(s);
}

void Property::deserialize(XmlDeserializer& s) {
    if (serializeValue_)
        return;

    // deserialize level-of-detail, if available
    try {
        int lod;
        s.deserialize("lod", lod);
        lod_ = static_cast<LODSetting>(lod);
    }
    catch (XmlSerializationNoSuchDataException&) {
        s.removeLastError();
        lod_ = USER;
    }

    // deserialize gui name, if available
    try {
        std::string temp;
        s.deserialize("guiName", temp);
        guiName_ = temp;
    }
    catch (XmlSerializationNoSuchDataException&) {
        s.removeLastError();
    }

    metaDataContainer_.deserialize(s);
}

void Property::serializeValue(XmlSerializer& s) {
    serializeValue_ = true;
    serialize(s);
    serializeValue_ = false;
}

void Property::deserializeValue(XmlDeserializer& s) {
    serializeValue_ = true;
    deserialize(s);
    serializeValue_ = false;
}

MetaDataContainer& Property::getMetaDataContainer() const {
    return metaDataContainer_;
}

void Property::registerLink(PropertyLink* link) {
    links_.push_back(link);
}

void Property::removeLink(PropertyLink* link) {
    for (std::vector<PropertyLink*>::iterator it = links_.begin(); it != links_.end(); ++it) {
        if (*it == link) {
            links_.erase(it);
            break;
        }
    }
}

const std::set<PropertyWidget*> Property::getPropertyWidgets() const {
    return widgets_;
}

Property::LODSetting Property::getLevelOfDetail() const {
    return lod_;
}

void Property::setLevelOfDetail(LODSetting lod) {
    lod_ = lod;
}

void Property::invalidateOwner() {
    invalidateOwner(invalidationLevel_);
}

void Property::invalidateOwner(int invalidationLevel) {
    if (getOwner())
        getOwner()->invalidate(invalidationLevel);
}

void Property::toggleInteractionMode(bool interactionmode, void* source) {
    if (!interactionModeVisited_) {
        interactionModeVisited_ = true;

        // propagate to owner
        if (getOwner() && (invalidationLevel_ != Processor::VALID)) {
            getOwner()->toggleInteractionMode(interactionmode, source);
        }

        // propagate over links
        for (size_t i=0; i<getLinks().size(); ++i) {
            Property* destProperty = getLinks()[i]->getDestinationProperty();
            tgtAssert(destProperty, "Link without destination property");
            destProperty->toggleInteractionMode(interactionmode, source);
        }
        interactionModeVisited_ = false;
    }
}

void Property::invalidate() {
    invalidateOwner();
    // notify widgets of updated values
    updateWidgets();
}

const std::vector<PropertyLink*>& Property::getLinks() const {
    return links_;
}

PropertyLink* Property::getLink(const Property* dest) const {
    for (size_t i=0; i<links_.size(); ++i) {
        if (links_[i]->getDestinationProperty() == dest)
            return links_[i];
    }
    return 0;
}

bool Property::isLinkedWith(const Property* dest, bool transitive) const {
    // true if dest is the object itself
    if (this == dest)
        return true;

    // check for direct links
    bool linkedDirectly = (getLink(dest) != 0);
    if (linkedDirectly)
        return true;

    // recursive search for indirect links
    if (transitive) {
        if (linkCheckVisited_)
            return false;
        else {
            linkCheckVisited_ = true;
            for (size_t i=0; i<links_.size(); ++i) {
                if (links_[i]->getDestinationProperty()->isLinkedWith(dest, true)) {
                    linkCheckVisited_ = false;
                    return true;
                }
            }
            linkCheckVisited_ = false;
        }
    }

    return false;
}

bool Property::isLinkableWith(const voreen::Property* dst) const{
    return (LinkEvaluatorHelper::arePropertiesLinkable(this, dst));
}

std::vector<std::pair<std::string, std::string> > Property::getCompatibleEvaluators(const voreen::Property* dst) const{
    return LinkEvaluatorHelper::getCompatibleLinkEvaluators(this, dst);
}

std::string Property::getTypeDescription() const {
    return "<unknown>";
}

std::string Property::getDescription() const {
    return description_;
}

void Property::setDescription(std::string desc) {
    description_ = desc;
}

} // namespace voreen
