/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/core/properties/templateproperty.h"
#include "tgt/shadermanager.h"

namespace voreen {

class ShaderSource : public Serializable {
public:
    std::string geometryFilename_;
    std::string vertexFilename_;
    std::string fragmentFilename_;

    // Default ctor
    ShaderSource() :
        geometryFilename_(""), vertexFilename_(""), fragmentFilename_(""),
        geometryModified_(false), vertexModified_(false), fragmentModified_(false)
       {}

    ShaderSource( std::string geometryFilename, std::string vertexFilename, std::string fragmentFilename) :
        geometryFilename_(geometryFilename), vertexFilename_(vertexFilename), fragmentFilename_(fragmentFilename),
        geometryModified_(false), vertexModified_(false), fragmentModified_(false)
       {}

    std::string geometrySource_;
    std::string vertexSource_;
    std::string fragmentSource_;

    bool geometryModified_;
    bool vertexModified_;
    bool fragmentModified_;

    /**
     * Operator to compare two ShaderSource objects.
     *
     * @param shaderSource ShaderSource instance that is compared to this instance
     * @return true when both ShaderSource instances are equal
     */
    bool operator==(const ShaderSource& shaderSource) const {
        return     (strcmp(shaderSource.geometryFilename_.c_str(), geometryFilename_.c_str()) == 0)
                && (strcmp(shaderSource.vertexFilename_.c_str(), vertexFilename_.c_str()) == 0)
                && (strcmp(shaderSource.fragmentFilename_.c_str(), fragmentFilename_.c_str()) == 0)
                && (strcmp(shaderSource.geometrySource_.c_str(), geometrySource_.c_str()) == 0)
                && (strcmp(shaderSource.fragmentSource_.c_str(), fragmentSource_.c_str()) == 0)
                && (strcmp(shaderSource.vertexSource_.c_str(), vertexSource_.c_str()) == 0)
                && (shaderSource.geometryModified_ == geometryModified_)
                && (shaderSource.fragmentModified_ == fragmentModified_)
                && (shaderSource.vertexModified_ == vertexModified_);
        ;
    }

    /**
     * Operator to compare two ShaderSource objects.
     *
     * @param shaderSource ShaderSource instance that is compared to this instance
     * @return true when both ShaderSource instances are not equal
     */
     bool operator!=(const ShaderSource& shaderSource) const { return !(*this == shaderSource); }

     virtual void serialize(XmlSerializer& s) const;
     virtual void deserialize(XmlDeserializer& s);
};

class ShaderProperty : public TemplateProperty<ShaderSource> {
public:
    ShaderProperty(const std::string& id, const std::string& guiText,
                   const std::string& fragmentFileName, const std::string& geometryFileName = "",
                   const std::string& vertexFileName = "",
                   Processor::InvalidationLevel invalidationLevel=Processor::INVALID_PROGRAM);

    ~ShaderProperty();

    virtual std::string getTypeString() const;

    void initialize() throw (VoreenException);
    void deinitialize() throw (VoreenException);

    /**
     * @see Property::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Property::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    PropertyWidget* createWidget(PropertyWidgetFactory* f);

    void setHeader(std::string header);
    std::string getHeader() const;

    tgt::Shader* getShader();
    void rebuild();

    //Fragment Shader:
    void setFragmentSource(const std::string& fragmentSource);
    void resetFragmentShader();
    void resetFragmentFilename();
    void setFragmentFilename(const std::string& fragmentFilename);
    const tgt::ShaderObject* getFragmentObject() { return frag_; }
    //Vertex Shader:
    void setVertexSource(const std::string& vertexSource);
    void resetVertexShader();
    void resetVertexFilename();
    void setVertexFilename(const std::string& vertexFilename);
    const tgt::ShaderObject* getVertexObject() { return vert_; }
    //Geometry Shader:
    void setGeometrySource(const std::string& geometrySource);
    void resetGeometryShader();
    void resetGeometryFilename();
    void setGeometryFilename(const std::string& geometryFilename);
    const tgt::ShaderObject* getGeometryObject() { return geom_; }
private:
    std::string getShaderAsString(std::string filename);

    std::string header_;
    std::string originalGeometryFilename_;
    std::string originalVertexFilename_;
    std::string originalFragmentFilename_;

    tgt::ShaderObject* geom_;
    tgt::ShaderObject* vert_;
    tgt::ShaderObject* frag_;
    tgt::Shader* shader_;
};

}   // namespace

#endif
