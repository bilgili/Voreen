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

#ifndef VRN_SHADERPROPERTY_H
#define VRN_SHADERPROPERTY_H

#include "voreen/core/vis/properties/templateproperty.h"
#include "tgt/shadermanager.h"

namespace voreen {

class ShaderProperty : public TemplateProperty<tgt::ShaderObject*> {
public:
    ShaderProperty(const std::string& id, const std::string& guiText, const std::string& fileName,
               tgt::ShaderObject::ShaderType type, Processor::InvalidationLevel invalidationLevel=Processor::INVALID_PROGRAM);

    ~ShaderProperty();

    /// @see Property::initialize
    void initialize() throw (VoreenException);

    /**
     * @see Property::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Property::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    PropertyWidget* createWidget(PropertyWidgetFactory* f);
    virtual std::string toString() const;

    void setSource(std::string source);
    std::string getSource() const;

    void setFileName(std::string fileName);
    std::string getFileName() const;

    void setHeader(std::string header);
    std::string getHeader() const;

    tgt::ShaderObject::ShaderType getType() const;
private:

    bool sourceModified_;
    std::string source_;
    std::string header_;
    tgt::ShaderObject::ShaderType type_;
    std::string origFileName_;

};

}   // namespace

#endif
