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

#include "voreen/core/vis/properties/property.h"
#include "voreen/core/vis/properties/propertywidgetfactory.h"
#include "voreen/core/vis/properties/propertywidget.h"
#include "voreen/core/vis/properties/link/changeaction.h"

namespace voreen {

Property::Property(const std::string& id, const std::string& guiText, Processor::InvalidationLevel invalidationLevel)
    : id_(id)
    , guiText_(guiText)
    , owner_(0)
    , invalidationLevel_(invalidationLevel)
    , widgetsEnabled_(true)
    , visible_(true)
    , lod_(USER)
    , interactionModeVisited_(false)
{}

Property::~Property() {
    disconnectWidgets();
}

Processor::InvalidationLevel Property::getInvalidationLevel() {
    return invalidationLevel_;
}

void Property::setWidgetsEnabled(bool enabled) {
    widgetsEnabled_ = enabled;
    for (std::set<PropertyWidget*>::iterator it = widgets_.begin(); it != widgets_.end(); ++it)
        (*it)->setEnabled(widgetsEnabled_);
}

bool Property::getWidgetsEnabled() const {
    return widgetsEnabled_;
}

void Property::initialize() throw (VoreenException) {
    // currently nothing to do
}

void Property::setVisible(bool state) {
    visible_ = state;
    setWidgetsVisible(state);
}

bool Property::isVisible() const {
    return visible_;
}

void Property::setOwner(Processor* processor) {
    owner_ = processor;
}

Processor* Property::getOwner() const {
    return owner_;
}

void Property::addWidget(PropertyWidget* widget) {
    if (widget)
        widgets_.insert(widget);
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

void Property::setWidgetsVisible(bool state) {
    std::set<PropertyWidget*>::iterator it = widgets_.begin();
    for ( ; it != widgets_.end(); ++it)
        (*it)->setVisible(state);
}

std::string Property::getGuiText() const {
    return guiText_;
}

std::string Property::getId() const {
    return id_;
}

PropertyWidget* Property::createWidget(PropertyWidgetFactory*) {
    return 0;
}

void Property::serialize(XmlSerializer& s) const {
    if (lod_ != USER)
        s.serialize("lod", lod_);

    for (std::set<PropertyWidget*>::const_iterator it = widgets_.begin(); it != widgets_.end(); ++it)
    {
        const_cast<PropertyWidget*>(*it)->updateMetaData();
        const_cast<PropertyWidget*>(*it)->getWidgetMetaData();
    }

    metaDataContainer_.serialize(s);
}

void Property::deserialize(XmlDeserializer& s) {
    try {
        int lod;

        s.deserialize("lod", lod);

        lod_ = static_cast<LODSetting>(lod);
    } catch (XmlSerializationNoSuchDataException&) {
        s.removeLastError();
        lod_ = USER;
    }

    metaDataContainer_.deserialize(s);
}

MetaDataContainer& Property::getMetaDataContainer() const {
    return metaDataContainer_;
}

PropertyWidget* Property::createAndAddWidget(PropertyWidgetFactory* f) {
    PropertyWidget* widget = createWidget(f);
    if (!visible_)
        widget->setVisible(visible_);
    if (!widgetsEnabled_)
        widget->setEnabled(false);
    addWidget(widget);
    return widget;
}

void Property::registerLink(PropertyLink* link) {
    links_.push_back(link);
}

void Property::removeLink(PropertyLink* link) {
    for (std::vector<PropertyLink*>::iterator it = links_.begin(); it != links_.end(); it++)
        if (*it == link) {
            links_.erase(it);
            break;
        }
}

const std::set<PropertyWidget*> Property::getPropertyWidgets() const {
    return widgets_;
}

Property::LODSetting Property::getLevelOfDetail() {
    return lod_;
}

void Property::setLevelOfDetail(LODSetting lod) {
    lod_ = lod;
}

void Property::invalidateOwner() {
    invalidateOwner(invalidationLevel_);
}

void Property::invalidateOwner(Processor::InvalidationLevel invalidationLevel) {
    if (getOwner())
        getOwner()->invalidate(invalidationLevel);
}

void Property::toggleInteractionMode(bool interactionmode, void* source) {

    // propagate to owner
    if (getOwner() && (invalidationLevel_ != Processor::VALID)) {
        getOwner()->toggleInteractionMode(interactionmode, source);
    }

    // propagate over links
    if (!interactionModeVisited_) {
        interactionModeVisited_ = true;
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
}

const std::vector<PropertyLink*>& Property::getLinks() const {
    return links_;
}

} // namespace voreen
