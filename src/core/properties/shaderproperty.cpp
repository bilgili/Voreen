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

#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/condition.h"

#include "tgt/filesystem.h"
#include "tgt/shadermanager.h"

namespace voreen {

void ShaderSource::serialize(XmlSerializer& s) const {
    //fragment:
    s.serialize("fragmentModified", fragmentModified_);
    s.serialize("fragmentIsExternal", fragmentIsExternal_);

    if (fragmentIsExternal_) {
        std::string relFragPath = tgt::FileSystem::relativePath(tgt::FileSystem::dirName(externalFragmentFilename_), tgt::FileSystem::dirName(s.getDocumentPath()));
        if (!relFragPath.empty()) relFragPath = relFragPath + "/";
        std::string relFragmentFilename =  relFragPath + tgt::FileSystem::fileName(externalFragmentFilename_);
        s.serialize("externalFragmentFilename", relFragmentFilename);
    }

    //vertex:
    s.serialize("vertexModified", vertexModified_);
    s.serialize("vertexIsExternal", vertexIsExternal_);

    if (vertexIsExternal_) {
        std::string relVertPath = tgt::FileSystem::relativePath(tgt::FileSystem::dirName(externalVertexFilename_), tgt::FileSystem::dirName(s.getDocumentPath()));
        if (!relVertPath.empty()) relVertPath = relVertPath + "/";
        std::string relVertexFilename = relVertPath + tgt::FileSystem::fileName(externalVertexFilename_);
        s.serialize("externalVertexFilename", relVertexFilename);
    }

    //geometry:
    s.serialize("geometryModified", geometryModified_);
    s.serialize("geometryIsExternal", geometryIsExternal_);

    if(geometryIsExternal_) {
        std::string relGeomPath = tgt::FileSystem::relativePath(tgt::FileSystem::dirName(externalGeometryFilename_), tgt::FileSystem::dirName(s.getDocumentPath()));
        if (!relGeomPath.empty()) relGeomPath = relGeomPath + "/";
        std::string relGeometryFilename = relGeomPath + tgt::FileSystem::fileName(externalGeometryFilename_);
        s.serialize("externalGeometryFilename", relGeometryFilename);
    }

    if(fragmentModified_)
        s.serialize("fragmentSource", fragmentSource_);
    if (vertexModified_)
        s.serialize("vertexSource", vertexSource_);
    if (geometryModified_)
        s.serialize("geometrySource", geometrySource_);
}

void ShaderSource::deserialize(XmlDeserializer& s) {

    // this is identical in the old and new formats:
    s.deserialize("fragmentModified", fragmentModified_);

    bool oldFormat = true;

    try {
        s.deserialize("fragmentFilename", externalFragmentFilename_);
    }
    catch (...) {
        s.removeLastError();
        oldFormat = false;
    }

    if(oldFormat) {
        if(externalFragmentFilename_.empty())
            fragmentIsExternal_ = true;

        s.deserialize("vertexModified", vertexModified_);
        s.deserialize("vertexFilename", externalVertexFilename_);
        if(!externalVertexFilename_.empty())
            vertexIsExternal_ = true;

        s.deserialize("geometryModified", geometryModified_);
        s.deserialize("geometryFilename", externalGeometryFilename_);

        if(!externalGeometryFilename_.empty())
            geometryIsExternal_ = true;

    } else {
        //current format:
        s.deserialize("fragmentIsExternal", fragmentIsExternal_);
        if (fragmentIsExternal_)
            s.deserialize("externalFragmentFilename", externalFragmentFilename_);

        s.deserialize("vertexModified", vertexModified_);
        s.deserialize("vertexIsExternal", vertexIsExternal_);
        if (vertexIsExternal_)
            s.deserialize("externalVertexFilename", externalVertexFilename_);

        s.deserialize("geometryModified", geometryModified_);
        s.deserialize("geometryIsExternal", geometryIsExternal_);
        if (geometryIsExternal_)
            s.deserialize("externalGeometryFilename", externalGeometryFilename_);
    }

    if(fragmentIsExternal_)
        externalFragmentFilename_ = tgt::FileSystem::absolutePath(tgt::FileSystem::dirName(s.getDocumentPath()) + "/" + externalFragmentFilename_);
    if(vertexIsExternal_)
        externalVertexFilename_ = tgt::FileSystem::absolutePath(tgt::FileSystem::dirName(s.getDocumentPath()) + "/" + externalVertexFilename_);
    if(geometryIsExternal_)
        externalGeometryFilename_ = tgt::FileSystem::absolutePath(tgt::FileSystem::dirName(s.getDocumentPath()) + "/" + externalGeometryFilename_);

    if (fragmentModified_)
        s.deserialize("fragmentSource", fragmentSource_);
    if (vertexModified_)
        s.deserialize("vertexSource", vertexSource_);
    if (geometryModified_)
        s.deserialize("geometrySource", geometrySource_);
}

//---------------------------------------------------------------------------------------------------------------

const std::string ShaderProperty::loggerCat_("voreen.ShaderProperty");

ShaderProperty::ShaderProperty(const std::string& id, const std::string& guiText,
        const std::string& fragmentFilename,
        const std::string& vertexFilename,
        const std::string& geometryFilename,
        int invalidationLevel)
                       : TemplateProperty<ShaderSource>(id, guiText, ShaderSource(geometryFilename, vertexFilename, fragmentFilename), invalidationLevel)
                       , header_("")
                       , originalGeometryFilename_(geometryFilename)
                       , originalVertexFilename_(vertexFilename)
                       , originalFragmentFilename_(fragmentFilename)
                       , geom_(0)
                       , vert_(0)
                       , frag_(0)
                       , shader_(0)
{}

ShaderProperty::ShaderProperty()
    : geom_(0)
    , vert_(0)
    , frag_(0)
    , shader_(0)
{}

ShaderProperty::~ShaderProperty() {
    if (shader_) {
        LWARNING(getFullyQualifiedGuiName() << " has not been deinitialized before destruction.");
    }
}

Property* ShaderProperty::create() const {
    return new ShaderProperty();
}

void ShaderProperty::initialize() throw (tgt::Exception) {
    TemplateProperty<ShaderSource>::initialize();

    if(!value_.fragmentModified_)
        value_.fragmentSource_ = getShaderAsString(value_.getCurrentFragmentName(), value_.fragmentIsExternal_);
    if(!value_.vertexModified_)
        value_.vertexSource_   = getShaderAsString(value_.getCurrentVertexName(), value_.vertexIsExternal_);
    if(!value_.geometryModified_)
        value_.geometrySource_ = getShaderAsString(value_.getCurrentGeometryName(), value_.geometryIsExternal_);
}

void ShaderProperty::deinitialize() throw (tgt::Exception) {
    delete shader_;
    shader_ = 0;
    LGL_ERROR;

    TemplateProperty<ShaderSource>::deinitialize();
}

void ShaderProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);
    get().serialize(s);
}

void ShaderProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);
    ShaderSource n = get();
    n.deserialize(s);
    set(n);
    invalidate();
    updateWidgets();
}

void ShaderProperty::setHeader(std::string header) {
    header_ = header;
}

std::string ShaderProperty::getHeader() const {
    return header_;
}

std::string ShaderProperty::getShaderAsString(std::string filename, bool external) {
    if(filename.empty())
        return "";

    if (!tgt::Singleton<tgt::ShaderManager>::isInited() && !external) {
        LWARNING("ShaderManager not instantiated");
        return "";
    }

    if (!tgt::Singleton<tgt::FileSystem>::isInited()) {
        LWARNING("FileSystem not instantiated");
        return "";
    }

    std::string completeFilename = filename;
    if(!external)
        completeFilename = ShdrMgr.completePath(filename);

    tgt::File* file = FileSys.open(completeFilename);

    // check if file is open
    if (!file || !file->isOpen()) {
        LERROR("File not found: " << filename);
        delete file;
        return "";
    }

    std::string s = file->getAsString();

    file->close();
    delete file;
    return s;
}

void ShaderProperty::resetFragmentShader() {
    if(!value_.originalFragmentFilename_.empty() || !value_.externalFragmentFilename_.empty()) {
        value_.fragmentSource_ = getShaderAsString(value_.getCurrentFragmentName(), value_.fragmentIsExternal_);
        value_.fragmentModified_ = false;
    }
    invalidate();
    updateWidgets();
}

void ShaderProperty::setFragmentSource(const std::string& fragmentSource) {
    if(fragmentSource != get().fragmentSource_) {
        ShaderSource n = get();
        n.fragmentSource_ = fragmentSource;
        n.fragmentModified_ = true;
        set(n);
    }
}

void ShaderProperty::setVertexSource(const std::string& vertexSource) {
    if(vertexSource != get().vertexSource_) {
        ShaderSource n = get();
        n.vertexSource_ = vertexSource;
        n.vertexModified_ = true;
        set(n);
    }
}

void ShaderProperty::setGeometrySource(const std::string& geometrySource) {
    if(geometrySource != get().geometrySource_) {
        ShaderSource n = get();
        n.geometrySource_ = geometrySource;
        n.geometryModified_ = true;
        set(n);
    }
}

void ShaderProperty::resetFragmentFilename() {
    value_.fragmentIsExternal_ = false;
    value_.externalFragmentFilename_= "";
    resetFragmentShader();
}

void ShaderProperty::resetVertexShader() {
    if(!value_.originalVertexFilename_.empty() || !value_.externalVertexFilename_.empty()) {
        value_.vertexSource_ = getShaderAsString(value_.getCurrentVertexName(), value_.vertexIsExternal_);
        value_.vertexModified_ = false;
    }
    invalidate();
    updateWidgets();
}

void ShaderProperty::resetVertexFilename() {
    value_.vertexIsExternal_ = false;
    value_.externalVertexFilename_ = "";
    resetVertexShader();
}

void ShaderProperty::resetGeometryShader() {
    if(!value_.originalGeometryFilename_.empty() || !value_.externalGeometryFilename_.empty()) {
        value_.geometrySource_ = getShaderAsString(value_.getCurrentGeometryName(), value_.geometryIsExternal_);
        value_.geometryModified_ = false;
    }
    invalidate();
    updateWidgets();
}

void ShaderProperty::resetGeometryFilename() {
    value_.geometryIsExternal_ = false;
    value_.externalGeometryFilename_ = "";
    resetGeometryShader();
}

void ShaderProperty::setFragmentFilename(const std::string& fragmentFilename) {
    if(fragmentFilename != get().externalFragmentFilename_) {
        ShaderSource n = get();
        n.fragmentIsExternal_ = true;
        n.externalFragmentFilename_ = fragmentFilename;
        set(n);
        resetFragmentShader();
    }
}

void ShaderProperty::setVertexFilename(const std::string& vertexFilename) {
    if(vertexFilename != get().externalVertexFilename_) {
        ShaderSource n = get();
        n.vertexIsExternal_ = true;
        n.externalVertexFilename_ = vertexFilename;
        set(n);
        resetVertexShader();
    }
}
void ShaderProperty::setGeometryFilename(const std::string& geometryFilename) {
    if(geometryFilename != get().externalGeometryFilename_) {
        ShaderSource n = get();
        n.geometryIsExternal_ = true;
        n.externalGeometryFilename_ = geometryFilename;
        set(n);
        resetGeometryShader();
    }
}

bool ShaderProperty::rebuild() {
    delete shader_;
    shader_ = 0;
    vert_ = 0;
    frag_ = 0;
    geom_ = 0;

    bool allSuccessful = true;
    shader_ = new tgt::Shader();

    if (!value_.originalVertexFilename_.empty()) {
        std::string vertFilename;
        bool validFilename = true;

        if(!value_.vertexIsExternal_) {
            vertFilename = ShdrMgr.completePath(value_.originalVertexFilename_);
            if(vertFilename.empty()) {
                LWARNING("Vertex shader file not found: " << FileSys.cleanupPath(value_.originalVertexFilename_));
                validFilename = false;
            }
        } else {
            // this should always be an absolute path after deserialization
            vertFilename = value_.externalVertexFilename_;
            if(!tgt::FileSystem::fileExists(vertFilename)) {
                LWARNING("Vertex shader external file not found: " << FileSys.cleanupPath(vertFilename));
                validFilename = false;
            }
        }

        // should we allow the use of a modified source if the file does not exist in the file-system?
        if(validFilename || value_.vertexModified_) {
            vert_ = new tgt::ShaderObject(vertFilename, tgt::ShaderObject::VERTEX_SHADER);
            vert_->setHeader(header_);
            if (!value_.vertexModified_) {
                vert_->loadSourceFromFile(vertFilename);
                value_.vertexSource_ = vert_->getSource();
            } else
                vert_->setSource(value_.vertexSource_);

            vert_->compileShader();
            if (vert_->isCompiled())
                shader_->attachObject(vert_);
            else {
                allSuccessful = false;
                LWARNINGC("voreen.ShaderProperty", "Failed to compile vertex shader (original filename" << vertFilename << "): " << vert_->getCompilerLog());
            }
        }
    }

    if (!value_.originalFragmentFilename_.empty()) {
        std::string fragFilename;
        bool validFilename = true;

        if(!value_.fragmentIsExternal_) {
            fragFilename = ShdrMgr.completePath(value_.originalFragmentFilename_);
            if(fragFilename.empty()) {
                LWARNING("Fragment shader file not found: " << FileSys.cleanupPath(value_.originalFragmentFilename_));
                validFilename = false;
            }
        } else {
            // this should always be an absolute path after deserialization
            fragFilename = value_.externalFragmentFilename_;
            if(!tgt::FileSystem::fileExists(fragFilename)) {
                LWARNING("Fragment shader external file not found: " << FileSys.cleanupPath(fragFilename));
                validFilename = false;
            }
        }

        // should we allow the use of a modified source if the file does not exist in the file-system?
        if(validFilename || value_.fragmentModified_) {
            frag_ = new tgt::ShaderObject(fragFilename, tgt::ShaderObject::FRAGMENT_SHADER);
            frag_->setHeader(header_);
            if (!value_.fragmentModified_) {
                frag_->loadSourceFromFile(fragFilename);
                value_.fragmentSource_ = frag_->getSource();
            } else
                frag_->setSource(value_.fragmentSource_);

            frag_->compileShader();
            if (frag_->isCompiled())
                shader_->attachObject(frag_);
            else {
                allSuccessful = false;
                LWARNINGC("voreen.ShaderProperty", "Failed to compile fragment shader (original filename" << fragFilename << "): " << frag_->getCompilerLog());
            }
        }
    }

    if (!value_.originalGeometryFilename_.empty()) {
        std::string geomFilename;
        bool validFilename = true;

        if(!value_.geometryIsExternal_) {
            geomFilename = ShdrMgr.completePath(value_.originalGeometryFilename_);
            if(geomFilename.empty()) {
                LWARNING("Geometry shader file not found: " << FileSys.cleanupPath(value_.originalGeometryFilename_));
                validFilename = false;
            }
        } else {
            // this should always be an absolute path after deserialization
            geomFilename= value_.externalGeometryFilename_;
            if(!tgt::FileSystem::fileExists(geomFilename)) {
                LWARNING("Geometry shader external file not found: " << FileSys.cleanupPath(geomFilename));
                validFilename = false;
            }
        }

        // should we allow the use of a modified source if the file does not exist in the file-system?
        if(validFilename || value_.geometryModified_) {
            geom_ = new tgt::ShaderObject(geomFilename, tgt::ShaderObject::GEOMETRY_SHADER);
            geom_->setHeader(header_);
            if (!value_.geometryModified_) {
                geom_->loadSourceFromFile(geomFilename);
                value_.geometrySource_ = geom_->getSource();
            } else
                geom_->setSource(value_.geometrySource_);

            geom_->compileShader();
            if (geom_->isCompiled())
                shader_->attachObject(geom_);
            else {
                allSuccessful = false;
                LWARNINGC("voreen.ShaderProperty", "Failed to compile geometry shader (original filename" << geomFilename << "): " << geom_->getCompilerLog());
            }
        }
    }

    if(shader_->hasObjects()) {
        shader_->linkProgram();
        if(!shader_->isLinked()) {
            LWARNINGC("voreen.ShaderProperty", "Failed to link shader: " << shader_->getLinkerLog());
            allSuccessful = false;
        }
    } else {
        LWARNINGC("voreen.ShaderProperty", "No objects have been attached to shader.");
        allSuccessful = false;
    }

    updateWidgets();

    return allSuccessful;
}

tgt::Shader* ShaderProperty::getShader() {
   return shader_;
}

bool ShaderProperty::hasValidShader() const {
   return (shader_ && shader_->isLinked());
}


}   // namespace
