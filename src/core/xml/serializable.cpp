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

#include "voreen/core/xml/serializable.h"

namespace voreen {

void ErrorCollector::store(const std::string& s) {
    errors_.push_back(s);
}

void ErrorCollector::store(const std::exception& e) {
    errors_.push_back(e.what());
}

void ErrorCollector::store(std::vector<std::string> v) {
    errors_.insert(errors_.end(), v.begin(), v.end());
}

void ErrorCollector::clear() {
    errors_.clear();
}

std::vector<std::string> ErrorCollector::errors() const {
    return errors_;
}

std::vector<std::string> ErrorCollector::pop() {
    std::vector<std::string> errors = errors_;
    errors_.clear();
    return errors;
}

//---------------------------------------------------------------------------

bool Serializable::ignoreIsSerializable_ = false;

Serializable::Serializable()
    : serializable_(true)
{}

Serializable::~Serializable() {
}

void Serializable::setSerializable(const bool serializable) {
    serializable_ = serializable;
}

bool Serializable::isSerializable() const {
    return serializable_;
}

std::vector<std::string> Serializable::errors() {
    return errors_.pop();
}

std::vector<std::string> Serializable::getErrors() const {
    return errors_.errors();
}

void Serializable::setIgnoreIsSerializable(bool ignore) {
    ignoreIsSerializable_ = ignore;
}

bool Serializable::ignoreIsSerializable() {
    return ignoreIsSerializable_;
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
    return metadata_->Clone()->ToElement(); // I hope this clones recursively
}

void MetaSerializer::updateFromXml(TiXmlElement* metaElem) {
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

std::vector<TiXmlElement*> MetaSerializer::getAllData() const {
    std::vector<TiXmlElement*> metaElements;
    TiXmlElement* metaElem;
    // iterate all children of metadata_
    for (metaElem = metadata_->FirstChildElement();
         metaElem;
         metaElem = metaElem->NextSiblingElement())
    {
        metaElements.push_back(metaElem->Clone()->ToElement());
    }
    return metaElements;
}

} // namespace voreen
