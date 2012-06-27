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

#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/condition.h"
#include "voreen/core/properties/propertywidgetfactory.h"

namespace voreen {

void ShaderSource::serialize(XmlSerializer& s) const {
    //fragment:
    s.serialize("fragmentModified", fragmentModified_);
    s.serialize("fragmentFilename", fragmentFilename_);
    if (fragmentModified_)
        s.serialize("fragmentSource", fragmentSource_);

    //vertex:
    s.serialize("vertexModified", vertexModified_);
    s.serialize("vertexFilename", vertexFilename_);
    if (vertexModified_)
        s.serialize("vertexSource", vertexSource_);

    //geometry:
    s.serialize("geometryModified", geometryModified_);
    s.serialize("geometryFilename", geometryFilename_);
    if (geometryModified_)
        s.serialize("geometrySource", geometrySource_);
}

void ShaderSource::deserialize(XmlDeserializer& s) {
    //current format:
    s.deserialize("fragmentModified", fragmentModified_);
    s.deserialize("fragmentFilename", fragmentFilename_);
    if (fragmentModified_)
        s.deserialize("fragmentSource", fragmentSource_);
    //else
        //resetfragmentShader();

    s.deserialize("vertexModified", vertexModified_);
    s.deserialize("vertexFilename", vertexFilename_);
    if (vertexModified_)
        s.deserialize("vertexSource", vertexSource_);
    //else
        //resetVertexShader();

    s.deserialize("geometryModified", geometryModified_);
    s.deserialize("geometryFilename", geometryFilename_);
    if (geometryModified_)
        s.deserialize("geometrySource", geometrySource_);
    //else
        //resetGeometryShader();
}

//---------------------------------------------------------------------------------------------------------------

ShaderProperty::ShaderProperty(const std::string& id, const std::string& guiText,
        const std::string& fragmentFilename, const std::string& geometryFilename,const std::string& vertexFilename,
        Processor::InvalidationLevel invalidationLevel)
                       : TemplateProperty<ShaderSource>(id, guiText, ShaderSource(geometryFilename, vertexFilename, fragmentFilename), invalidationLevel)
                       , header_("")
                       , originalGeometryFilename_(geometryFilename)
                       , originalVertexFilename_(vertexFilename)
                       , originalFragmentFilename_(fragmentFilename)
                       , geom_(0)
                       , vert_(0)
                       , frag_(0)
                       , shader_(0)
{
    value_.geometryFilename_ = originalGeometryFilename_;
    value_.vertexFilename_ = originalVertexFilename_;
    value_.fragmentFilename_ = originalFragmentFilename_;
}

ShaderProperty::~ShaderProperty() {
    if (shader_) {
        LWARNINGC("voreen.ShaderProperty",
            getFullyQualifiedGuiName() << " has not been deinitialized before destruction.");
    }
}

void ShaderProperty::initialize() throw (VoreenException) {
    TemplateProperty<ShaderSource>::initialize();
    rebuild();
    LGL_ERROR;
}

void ShaderProperty::deinitialize() throw (VoreenException) {
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

    bool oldFormat = true;
    //TODO: put in converter
    try {
        bool test;
        s.deserialize("sourceModified", test);
    }
    catch (...) {
        s.removeLastError();
        oldFormat = false;
    }

    if(oldFormat) {
        ShaderSource n = get();
        s.deserialize("sourceModified", n.fragmentModified_);
        if (n.fragmentModified_)
            s.deserialize("source", n.fragmentSource_);

        n.fragmentFilename_ = originalFragmentFilename_;

        n.vertexModified_ = false;
        n.vertexSource_ = "";
        n.vertexFilename_ = "";

        n.geometryModified_ = false;
        n.geometrySource_ = "";
        n.geometryFilename_ = "";

        set(n);
    }
    else {
        ShaderSource n = get();
        n.deserialize(s);
        set(n);
    }

    invalidate();
    updateWidgets();
}

PropertyWidget* ShaderProperty::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

void ShaderProperty::setHeader(std::string header) {
    header_ = header;
}

std::string ShaderProperty::getHeader() const {
    return header_;
}

std::string ShaderProperty::getShaderAsString(std::string filename) {
    std::string completeFilename = ShdrMgr.completePath(filename);
    tgt::File* file = FileSys.open(completeFilename);

    // check if file is open
    if (!file || !file->isOpen()) {
        LERRORC("voreen.shaderproperty", "File not found: " << filename);
        delete file;
        return "";
    }

    std::string s = file->getAsString();

    file->close();
    delete file;
    return s;
}

void ShaderProperty::resetFragmentShader() {
    if(!value_.fragmentFilename_.empty()) {
        value_.fragmentSource_ = getShaderAsString(value_.fragmentFilename_);
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
    value_.fragmentFilename_ = originalFragmentFilename_;
    resetFragmentShader();
}

void ShaderProperty::resetVertexShader() {
    if(!value_.vertexFilename_.empty()) {
        value_.vertexSource_ = getShaderAsString(value_.vertexFilename_);
        value_.vertexModified_ = false;
    }
    invalidate();
    updateWidgets();
}

void ShaderProperty::resetVertexFilename() {
    value_.vertexFilename_ = originalVertexFilename_;
    resetVertexShader();
}

void ShaderProperty::resetGeometryShader() {
    if(!value_.geometrySource_.empty()) {
        value_.geometrySource_ = getShaderAsString(value_.geometryFilename_);
        value_.geometryModified_ = false;
    }
    invalidate();
    updateWidgets();
}

void ShaderProperty::resetGeometryFilename() {
    value_.geometryFilename_ = originalGeometryFilename_;
    resetGeometryShader();
}

void ShaderProperty::setFragmentFilename(const std::string& fragmentFilename) {
    if(fragmentFilename != get().fragmentFilename_) {
        ShaderSource n = get();
        n.fragmentFilename_ = fragmentFilename;
        set(n);
        resetFragmentShader();
    }
}

void ShaderProperty::setVertexFilename(const std::string& vertexFilename) {
    if(vertexFilename != get().vertexFilename_) {
        ShaderSource n = get();
        n.vertexFilename_ = vertexFilename;
        set(n);
        resetVertexShader();
    }
}
void ShaderProperty::setGeometryFilename(const std::string& geometryFilename) {
    if(geometryFilename != get().geometryFilename_) {
        ShaderSource n = get();
        n.geometryFilename_ = geometryFilename;
        set(n);
        resetGeometryShader();
    }
}

void ShaderProperty::rebuild() {
    delete shader_;
    shader_ = 0;
    vert_ = 0;
    frag_ = 0;
    geom_ = 0;

    shader_ = new tgt::Shader();
    //bool failedCompile = false;

    if (!value_.vertexFilename_.empty()) {
        vert_ = new tgt::ShaderObject(value_.vertexFilename_, tgt::ShaderObject::VERTEX_SHADER);
        vert_->setHeader(header_);
        if (!value_.vertexModified_) {
            vert_->loadSourceFromFile(ShdrMgr.completePath(value_.vertexFilename_));
            value_.vertexSource_ = vert_->getSource();
        }
        else {
            vert_->setSource(value_.vertexSource_);
        }
        vert_->compileShader();
        if (vert_->isCompiled())
            shader_->attachObject(vert_);
        else {
            //failedCompile = true;
            LWARNINGC("voreen.shaderproperty", "Failed to compile vertex shader: " << vert_->getCompilerLog());
        }
    }

    if (!value_.fragmentFilename_.empty()) {
        frag_ = new tgt::ShaderObject(value_.fragmentFilename_, tgt::ShaderObject::FRAGMENT_SHADER);
        frag_->setHeader(header_);
        if (!value_.fragmentModified_) {
            frag_->loadSourceFromFile(ShdrMgr.completePath(value_.fragmentFilename_));
            value_.fragmentSource_ = frag_->getSource();
        }
        else {
            frag_->setSource(value_.fragmentSource_);
        }
        frag_->compileShader();
        if (frag_->isCompiled())
            shader_->attachObject(frag_);
        else {
            //failedCompile = true;
            LWARNINGC("voreen.shaderproperty", "Failed to compile fragment shader: " << frag_->getCompilerLog());
        }
    }

    if (!value_.geometryFilename_.empty()) {
        geom_ = new tgt::ShaderObject(value_.geometryFilename_, tgt::ShaderObject::GEOMETRY_SHADER);
        geom_->setHeader(header_);
        if (!value_.geometryModified_) {
            geom_->loadSourceFromFile(ShdrMgr.completePath(value_.geometryFilename_));
            value_.geometrySource_ = geom_->getSource();
        }
        else {
            geom_->setSource(value_.geometrySource_);
        }
        geom_->compileShader();
        if (geom_->isCompiled())
            shader_->attachObject(geom_);
        else {
            //failedCompile = true;
            LWARNINGC("voreen.shaderproperty", "Failed to compile geometry shader: " << geom_->getCompilerLog());
        }
    }

    //if(!failedCompile) {
        shader_->linkProgram();
        if(!shader_->isLinked()) {
            LWARNINGC("voreen.shaderproperty", "Failed to link shader: " << shader_->getLinkerLog());
        }
    //}

    updateWidgets();
}

tgt::Shader* ShaderProperty::getShader() {
   return shader_;
}

std::string ShaderProperty::getTypeString() const {
    return "Shader";
}

}   // namespace
