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

#include "voreen/core/vis/properties/shaderproperty.h"
#include "voreen/core/vis/properties/condition.h"
#include "voreen/core/vis/properties/propertywidgetfactory.h"

namespace voreen {

ShaderProperty::ShaderProperty(const std::string& id, const std::string& guiText, const std::string& fileName,
                         tgt::ShaderObject::ShaderType type, Processor::InvalidationLevel invalidationLevel)
                       : TemplateProperty<tgt::ShaderObject*>(id, guiText, 0, invalidationLevel)
                       , sourceModified_(false)
                       , source_("")
                       , header_("")
                       , type_(type)
                       , origFileName_(fileName)
{ }

ShaderProperty::~ShaderProperty() {
    delete value_;
}

void ShaderProperty::initialize() throw (VoreenException) {
    value_ = new tgt::ShaderObject(ShdrMgr.completePath(origFileName_), type_);
    value_->setHeader(header_);
    if (!source_.empty()) {
        value_->setSource(source_);
    }
    else {
        value_->loadSourceFromFile(ShdrMgr.completePath(origFileName_));
        source_ = value_->getSource();
    }
    value_->compileShader();
}

void ShaderProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    s.serialize("sourceModified", sourceModified_);
    if (sourceModified_)
        s.serialize("source", source_);
    else
        s.serialize("source", origFileName_);
}

void ShaderProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    s.deserialize("sourceModified", sourceModified_);
    if (sourceModified_) {
        std::string source;
        s.deserialize("source", source);
        setSource(source);
    }
    else {
        s.deserialize("source", origFileName_);
        // no OpenGL-dependent initializations during deserialization
        //value_->loadSourceFromFile(ShdrMgr.completePath(origFileName_));
        //source_ = value_->getSource();
    }

    //value_->compileShader();
}

PropertyWidget* ShaderProperty::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

std::string ShaderProperty::toString() const {
    return source_;
}

void ShaderProperty::setSource(std::string source) {
    if (source_ != source)
        sourceModified_ = true;

    source_ = source;
    if (value_) {
        value_->setSource(source);
        invalidateOwner();
    }
}

std::string ShaderProperty::getSource() const {
    return source_;
}

void ShaderProperty::setFileName(std::string fileName) {
    origFileName_ = fileName;
}

std::string ShaderProperty::getFileName() const {
    return origFileName_;
}

void ShaderProperty::setHeader(std::string header) {
    header_ = header;
}

std::string ShaderProperty::getHeader() const {
    return header_;
}

tgt::ShaderObject::ShaderType ShaderProperty::getType() const {
    return type_;
}


}   // namespace
