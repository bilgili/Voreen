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

#include "voreen/core/vis/properties/propertyvector.h"
#include "voreen/core/vis/properties/condition.h"
#include "voreen/core/vis/propertywidgetfactory.h"
#include <sstream>


namespace voreen {

const std::string PropertyVector::loggerCat_("voreen.Properties.PropertyVector");

PropertyVector::PropertyVector(const std::string& id, const std::string& guiText, std::vector<Property*> properties)
    : Property(id, guiText),
      properties_(properties)
{}

PropertyVector::~PropertyVector() {
    for (size_t i=0; i<properties_.size(); ++i)
        delete properties_[i];
}

PropertyWidget* PropertyVector::createWidget(PropertyWidgetFactory* f) {

    return f->createWidget(this);
}

const std::vector<Property*>& PropertyVector::getProperties() const {
    return properties_;
}

void PropertyVector::setOwner(Processor* processor){
    Property::setOwner(processor);
    for (size_t i=0; i < properties_.size(); ++i)
        properties_[i]->setOwner(processor);
}

void PropertyVector::addProperty(Property* prop) {
    properties_.push_back(prop);
}

void PropertyVector::updateFromXml(TiXmlElement* rootElem) {

    Property::updateFromXml(rootElem);

    TiXmlElement* propertyElementsElem = rootElem->FirstChildElement("ElementProperties");
    if (propertyElementsElem) {
        
        // iterate over Property Elements and restore them
        int propID = 0;
        TiXmlElement* propElem;
        for (propElem = propertyElementsElem->FirstChildElement("Property");
             propElem;
             propElem = propElem->NextSiblingElement("Property")) {
            
            if (propID < getNumProperties()) {
                try {
                    properties_[propID]->updateFromXml(propElem);
                }
                catch (XmlException& e){
                    errors_.store(e);
                }
            }
            else {
                errors_.store(XmlElementException(getIdent().getName() + ": Number of stored property elements \
                                                   exceeds number of properties in PropertyVector"));
                break;
            }

            propID++;
        }

        if (propID < getNumProperties()) 
            errors_.store(XmlElementException(getIdent().getName() + ": Less stored property elements than \
                                               properties in PropertyVector"));
    }
    else {
        errors_.store(XmlElementException("Element 'ElementProperties' missing in " + getIdent().getName()));
    }
    
}

TiXmlElement* PropertyVector::serializeToXml() const {

    TiXmlElement* root = Property::serializeToXml();
    
    // serialize element properties
    TiXmlElement* elementProp = new TiXmlElement("ElementProperties");
    for (size_t i=0; i<properties_.size(); ++i) {
        TiXmlElement* propXml = properties_[i]->serializeToXml();
        elementProp->LinkEndChild(propXml);
    }
    root->LinkEndChild(elementProp);

    return root;
}

std::string PropertyVector::toString() const {
    std::string str = "PropertyVector\n";
    for (size_t i=0; i<properties_.size(); ++i)
        str += Property::valueToString(properties_[i]) + "\n";

    return str;    
}

int PropertyVector::getNumProperties() const {
    return static_cast<int>(properties_.size());
}


}   // namespace
