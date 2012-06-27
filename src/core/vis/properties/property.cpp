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
#include "voreen/core/vis/propertywidgetfactory.h"
#include "voreen/core/vis/propertywidget.h"

namespace voreen {

const std::string Property::XmlElementName_ = "Property";
bool Property::serializeTypeInformation_ = false;

Property::Property(const std::string& id, const std::string& guiText)
    : Serializable()
    , id_(id)
    , guiText_(guiText)
    , owner_(0)
    , visible_(true)
    , lod_(USER)
    , metaData_(0)
{}

Property::~Property() {
    disconnectWidgets();
    delete metaData_;
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
        (*it)->update();
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

Identifier Property::getIdent() const {
    return Identifier(id_);
}

PropertyWidget* Property::createWidget(PropertyWidgetFactory*) {
    return 0;
}

// XML serialization stuff
std::string Property::getXmlElementName() const {
    return XmlElementName_;
}

const Identifier Property::getIdent(TiXmlElement* propertyElem) {
    if (!propertyElem)
        throw XmlElementException("Can't get Ident of Null-Pointer!");
    if (propertyElem->Value() != XmlElementName_)
        throw XmlElementException(std::string("Cant get ClassName of a ") + propertyElem->Value()
                                  + " - need " + XmlElementName_ + "!");
    return Identifier(propertyElem->Attribute("id"));
}

TiXmlElement* Property::serializeToXml() const {
    TiXmlElement* propElem = new TiXmlElement(XmlElementName_);

    propElem->SetAttribute("id", id_);

    if (getSerializeTypeInformation()) {
        
        propElem->SetAttribute("label", guiText_);
    }

    // query property widgets for meta data to serialize
    std::vector<TiXmlElement*> widgetMetaData;
    std::set<PropertyWidget*>::const_iterator it = widgets_.begin();
    for ( ; it != widgets_.end(); ++it) {
        TiXmlElement* metaDataElem = (*it)->getWidgetMetaData();
        if (metaDataElem)
            widgetMetaData.push_back(metaDataElem);
    }
    // if there is meta data, add it to the DOM
    if (!widgetMetaData.empty()) {
        TiXmlElement* metaDataElem = new TiXmlElement("MetaData");
        propElem->LinkEndChild(metaDataElem);
        for (size_t i=0; i<widgetMetaData.size(); ++i)
            metaDataElem->LinkEndChild(widgetMetaData[i]);
    }

    if (lod_ != USER)
        propElem->SetAttribute("lod", lod_);

    return propElem;
}

void Property::updateFromXml(TiXmlElement* propElem) {
    errors_.clear();
    int result;
    if (propElem->QueryIntAttribute("lod", &result) == TIXML_SUCCESS)
        lod_ = static_cast<LODSetting>(result);
    else
        lod_ = USER;

    // read widget meta data
    delete metaData_;
    TiXmlElement* widgetMetaElem = propElem->FirstChildElement("MetaData");
    if (widgetMetaElem)
        metaData_ = new TiXmlElement(*widgetMetaElem);
    else 
        metaData_ = 0;
}

PropertyWidget* Property::createAndAddWidget(PropertyWidgetFactory* f) {
    PropertyWidget* widget = createWidget(f);
    if (!visible_)
        widget->setVisible(visible_);
    addWidget(widget);
    return widget;
}

std::set<PropertyWidget*> Property::getPropertyWidgets() const {
    return widgets_;
}

Property::LODSetting Property::getLevelOfDetail() {
    return lod_;
}

void Property::setLevelOfDetail(LODSetting lod) {
    lod_ = lod;
}

bool Property::getSerializeTypeInformation() {
    return serializeTypeInformation_;
}

void Property::setSerializeTypeInformation(bool enable) {
    serializeTypeInformation_ = enable;
}

const TiXmlElement* Property::getMetaData() const {
    return metaData_;
}

} // namespace voreen
