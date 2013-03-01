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

#ifndef VRN_SHADERPROPERTY_H
#define VRN_SHADERPROPERTY_H

#include "voreen/core/properties/templateproperty.h"
#include "tgt/shadermanager.h"

namespace voreen {

class VRN_CORE_API ShaderSource : public Serializable {
public:
    ShaderSource() :
        originalGeometryFilename_(""), originalVertexFilename_(""), originalFragmentFilename_(""),
        externalGeometryFilename_(""), externalVertexFilename_(""), externalFragmentFilename_(""),
        geometryIsExternal_(false), vertexIsExternal_(false), fragmentIsExternal_(false),
        geometryModified_(false), vertexModified_(false), fragmentModified_(false)
       {}

    ShaderSource(std::string geometryFilename, std::string vertexFilename, std::string fragmentFilename) :
        originalGeometryFilename_(geometryFilename), originalVertexFilename_(vertexFilename), originalFragmentFilename_(fragmentFilename),
        externalGeometryFilename_(""), externalVertexFilename_(""), externalFragmentFilename_(""),
        geometryIsExternal_(false), vertexIsExternal_(false), fragmentIsExternal_(false),
        geometryModified_(false), vertexModified_(false), fragmentModified_(false)
       {}

    std::string getCurrentFragmentName() const {
        return fragmentIsExternal_ ? externalFragmentFilename_ : originalFragmentFilename_;
    }

    std::string getCurrentVertexName() const {
        return vertexIsExternal_ ? externalVertexFilename_ : originalVertexFilename_;
    }

    std::string getCurrentGeometryName() const {
        return geometryIsExternal_ ? externalGeometryFilename_ : originalGeometryFilename_;
    }

//protected: FIXME!
    std::string originalGeometryFilename_;
    std::string originalVertexFilename_;
    std::string originalFragmentFilename_;

    std::string externalGeometryFilename_;
    std::string externalVertexFilename_;
    std::string externalFragmentFilename_;

    std::string geometrySource_;
    std::string vertexSource_;
    std::string fragmentSource_;

    bool geometryModified_;
    bool vertexModified_;
    bool fragmentModified_;

    bool geometryIsExternal_;
    bool vertexIsExternal_;
    bool fragmentIsExternal_;

public:
    /**
     * Operator to compare two ShaderSource objects.
     *
     * @param shaderSource ShaderSource instance that is compared to this instance
     * @return true when both ShaderSource instances are equal
     */
    bool operator==(const ShaderSource& shaderSource) const {
        bool fnEqual = (strcmp(shaderSource.externalGeometryFilename_.c_str(), externalGeometryFilename_.c_str()) == 0)
                    && (strcmp(shaderSource.externalVertexFilename_.c_str(), externalVertexFilename_.c_str()) == 0)
                    && (strcmp(shaderSource.externalFragmentFilename_.c_str(), externalFragmentFilename_.c_str()) == 0)
                    && (strcmp(shaderSource.originalGeometryFilename_.c_str(), originalGeometryFilename_.c_str()) == 0)
                    && (strcmp(shaderSource.originalVertexFilename_.c_str(), originalVertexFilename_.c_str()) == 0)
                    && (strcmp(shaderSource.originalFragmentFilename_.c_str(), originalFragmentFilename_.c_str()) == 0);

        if(!fnEqual)
            return false;

        bool anyModified = (shaderSource.geometryModified_ || geometryModified_
                         || shaderSource.fragmentModified_ || fragmentModified_
                         || shaderSource.vertexModified_   || vertexModified_);
        if(!anyModified)
            return true;

        return(strcmp(shaderSource.geometrySource_.c_str(), geometrySource_.c_str()) == 0)
           && (strcmp(shaderSource.fragmentSource_.c_str(), fragmentSource_.c_str()) == 0)
           && (strcmp(shaderSource.vertexSource_.c_str(), vertexSource_.c_str()) == 0);
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

class VRN_CORE_API ShaderProperty : public TemplateProperty<ShaderSource> {
public:
    ShaderProperty(const std::string& id, const std::string& guiText,
                   const std::string& fragmentFileName,
                   const std::string& vertexFileName = "",
                   const std::string& geometryFileName = "",
                   int invalidationLevel=Processor::INVALID_PROGRAM);
    ShaderProperty();
    ~ShaderProperty();

    virtual Property* create() const;

    virtual std::string getClassName() const       { return "ShaderProperty"; }
    virtual std::string getTypeDescription() const { return "Shader"; }

    void initialize() throw (tgt::Exception);
    void deinitialize() throw (tgt::Exception);

    /**
     * @see Property::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Property::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    void setHeader(std::string header);
    std::string getHeader() const;

    tgt::Shader* getShader();
    bool rebuild();
    bool hasValidShader() const;

    //Fragment Shader:
    void setFragmentSource(const std::string& fragmentSource);
    void resetFragmentShader();
    void resetFragmentFilename();
    void setFragmentFilename(const std::string& fragmentFilename);
    tgt::ShaderObject* getFragmentObject() { return frag_; }

    //Vertex Shader:
    void setVertexSource(const std::string& vertexSource);
    void resetVertexShader();
    void resetVertexFilename();
    void setVertexFilename(const std::string& vertexFilename);
    tgt::ShaderObject* getVertexObject() { return vert_; }

    //Geometry Shader:
    void setGeometrySource(const std::string& geometrySource);
    void resetGeometryShader();
    void resetGeometryFilename();
    void setGeometryFilename(const std::string& geometryFilename);
    tgt::ShaderObject* getGeometryObject() { return geom_; }

private:

    std::string getShaderAsString(std::string filename, bool external);

    std::string header_;
    std::string originalGeometryFilename_;
    std::string originalVertexFilename_;
    std::string originalFragmentFilename_;

    tgt::ShaderObject* geom_;
    tgt::ShaderObject* vert_;
    tgt::ShaderObject* frag_;
    tgt::Shader* shader_;

    static const std::string loggerCat_;
};

}   // namespace

#endif
