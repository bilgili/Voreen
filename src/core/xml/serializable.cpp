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

#include "voreen/core/xml/serializable.h"

namespace voreen {

void ErrorCollector::store(std::exception e) {
    errors_.push_back(e); // Exception gets copied - don't know any better
}

void ErrorCollector::store(std::vector<std::exception> v) {
    errors_.insert(errors_.end(), v.begin(), v.end());
}

void ErrorCollector::clear() {
    errors_.clear();
}

std::vector<std::exception> ErrorCollector::errors() const {
    return errors_; // Exceptions get copied here, too
}

std::vector<std::exception> ErrorCollector::pop() {
    std::vector<std::exception> errors = errors_;
    errors_.clear();
    return errors;
}

//---------------------------------------------------------------------------

Serializable::Serializable()
: serializable_(true)
{
    errors_ = ErrorCollector();
}

Serializable::~Serializable() {
}

void Serializable::setSerializable(const bool serializable) {
    serializable_ = serializable;
}

bool Serializable::isSerializable() const {
    return serializable_;
}

void Serializable::serializableSanityChecks(const TiXmlElement* elem) const {
    if (!serializable_)
        throw NotSerializableException("You tried to deserialize an Object, that isn't serializable.");
    if (elem && elem->Value() != getXmlElementName())
        throw XmlElementException(std::string("You tried to deserialize a ") + elem->Value() + " as a " + getXmlElementName());
}

std::vector<std::exception> Serializable::errors() {
    return errors_.pop();
}

std::vector<std::exception> Serializable::getErrors() const {
    return errors_.errors();
}

//---------------------------------------------------------------------------

MetaSerializer::MetaSerializer() : Serializable() {
    metadata_ = new TiXmlElement(getXmlElementName());
}

MetaSerializer::MetaSerializer(const MetaSerializer& m) : Serializable() {
    metadata_ = m.metadata_->Clone()->ToElement();
}

MetaSerializer::~MetaSerializer() {
    delete metadata_;
}

MetaSerializer& MetaSerializer::operator=(const MetaSerializer& m) {
    delete metadata_;
    metadata_ = m.metadata_->Clone()->ToElement();
    return *this;
}
    
TiXmlElement* MetaSerializer::serializeToXml() const {
    serializableSanityChecks();
    return metadata_->Clone()->ToElement(); // I hope this clones recursively
}

void MetaSerializer::updateFromXml(TiXmlElement* metaElem) {
    serializableSanityChecks(metaElem);
    if (metaElem) {
        delete metadata_;
        metadata_ = metaElem->Clone()->ToElement(); // I hope this clones recursively
    }
    else {
        delete metadata_;
        metadata_ = new TiXmlElement(getXmlElementName());
    }
}

void MetaSerializer::addData(TiXmlElement* data) {
    try {
        removeData(data->Value());
    }
    catch (XmlElementException) {}
    metadata_->LinkEndChild(data);
}

void MetaSerializer::removeData(std::string elemName) {
    TiXmlElement* child = metadata_->FirstChildElement(elemName);
    if (child)
        metadata_->RemoveChild(child);
    else
        throw XmlElementException("Metadata (" + elemName + ") does not exist and thus can't be removed!");
}

void MetaSerializer::clearData() {
    delete metadata_;
    metadata_ = new TiXmlElement(getXmlElementName());
}

TiXmlElement* MetaSerializer::getData(std::string elemName) const {
    TiXmlElement* metaElem = metadata_->FirstChildElement(elemName);
    if (!metaElem)
        throw XmlElementException("Requested Metadata (" + elemName + ") not found!");
    return metaElem->Clone()->ToElement();
}

bool MetaSerializer::hasData(std::string elemName) const {
    return metadata_->FirstChildElement(elemName) ? true : false;
}

} //namespace Voreen
