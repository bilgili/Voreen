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

#include "modules/opencl/openclmodule.h"
#include "modules/opencl/properties/openclproperty.h"
#include "voreen/core/properties/condition.h"

#include "tgt/filesystem.h"

namespace voreen {

void OpenCLSource::serialize(XmlSerializer& s) const {
    s.serialize("programModified", programModified_);
    std::string relPath = tgt::FileSystem::relativePath(tgt::FileSystem::dirName(programFilename_),
        tgt::FileSystem::dirName(s.getDocumentPath()));
    std::string relProgramFilename = relPath + "/" + tgt::FileSystem::fileName(programFilename_);
    s.serialize("programFilename", relProgramFilename);
    if (programModified_)
        s.serialize("programSource", programSource_);
}

void OpenCLSource::deserialize(XmlDeserializer& s) {
    //current format:
    s.deserialize("programModified", programModified_);
    s.deserialize("programFilename", programFilename_);
    programFilename_ = tgt::FileSystem::absolutePath(
        tgt::FileSystem::dirName(s.getDocumentPath())) + "/" + programFilename_;
    if (programModified_)
        s.deserialize("programSource", programSource_);
}

//---------------------------------------------------------------------------------------------------------------

OpenCLProperty::OpenCLProperty(const std::string& id, const std::string& guiText, const std::string& programFilename, Processor::InvalidationLevel invalidationLevel)
                       : TemplateProperty<OpenCLSource>(id, guiText, OpenCLSource(programFilename), invalidationLevel)
                       , programDefines_("")
                       , originalProgramFilename_(programFilename)
                       , program_(0)
{
    value_.programFilename_ = originalProgramFilename_;
}

OpenCLProperty::OpenCLProperty()
    : program_(0)
{}

OpenCLProperty::~OpenCLProperty() {
    if (program_) {
        LWARNINGC("voreen.OpenCLProperty",
            getFullyQualifiedGuiName() << " has not been deinitialized before destruction.");
    }
}

Property* OpenCLProperty::create() const {
    return new OpenCLProperty();
}

void OpenCLProperty::initialize() throw (tgt::Exception) {
    TemplateProperty<OpenCLSource>::initialize();
    LGL_ERROR;
}

void OpenCLProperty::deinitialize() throw (tgt::Exception) {
    clearProgram();

    TemplateProperty<OpenCLSource>::deinitialize();
}

void OpenCLProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);
    get().serialize(s);
}

void OpenCLProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    OpenCLSource n = get();
    n.deserialize(s);
    set(n);

    if(!value_.programModified_)
        value_.programSource_ = getProgramAsString(value_.programFilename_);

    invalidate();
    updateWidgets();
}

void OpenCLProperty::setDefines(std::string defs) {
    programDefines_ = defs;
}

std::string OpenCLProperty::getDefines() const {
    return programDefines_;
}

std::string OpenCLProperty::getProgramAsString(std::string filename) {
    //std::string completeFilename = ShdrMgr.completePath(filename);
    tgt::File* file = FileSys.open(filename);

    // check if file is open
    if (!file || !file->isOpen()) {
        LERRORC("voreen.openclproperty", "File not found: " << filename);
        delete file;
        return "";
    }

    std::string s = file->getAsString();

    file->close();
    delete file;
    return s;
}

void OpenCLProperty::resetProgramSource() {
    if(!value_.programFilename_.empty()) {
        value_.programSource_ = getProgramAsString(value_.programFilename_);
        value_.programModified_ = false;
    }
    invalidate();
    updateWidgets();
}

void OpenCLProperty::setProgramSource(const std::string& programSource) {
    if(programSource != get().programSource_) {
        OpenCLSource n = get();
        n.programSource_ = programSource;
        n.programModified_ = true;
        set(n);
    }
    invalidate();
    updateWidgets();
}

void OpenCLProperty::resetProgramFilename() {
    value_.programFilename_ = originalProgramFilename_;
    resetProgramSource();
}

void OpenCLProperty::setProgramFilename(const std::string& programFilename) {
    if(programFilename != get().programFilename_) {
        OpenCLSource n = get();
        n.programFilename_ = programFilename;
        set(n);
        resetProgramSource();
    }
}

void OpenCLProperty::rebuild() {
    delete program_;
    program_ = new cl::Program(OpenCLModule::getInstance()->getCLContext());

    if (!value_.programFilename_.empty()) {

        if (!value_.programModified_) {
            program_->loadSource(value_.programFilename_);
            value_.programSource_ = getProgramAsString(value_.programFilename_);
        }
        else {
            program_->setSource(value_.programSource_);
        }

        program_->setBuildOptions(programDefines_);
        bool success = program_->build(OpenCLModule::getInstance()->getCLDevice());

        if (!success)
            throw VoreenException("Unable to build program");
    }

    updateWidgets();
}

void OpenCLProperty::clearProgram() {
    delete program_;
    program_ = 0;
    LGL_ERROR;
}

cl::Program* OpenCLProperty::getProgram() const {
   return program_;
}

}   // namespace
