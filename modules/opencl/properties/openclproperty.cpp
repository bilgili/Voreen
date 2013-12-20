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

const std::string OpenCLProperty::loggerCat_("voreen.opencl.OpenCLProperty");

OpenCLProperty::OpenCLProperty(const std::string& id, const std::string& guiText, const std::string& programFilename, Processor::InvalidationLevel invalidationLevel)
                       : TemplateProperty<OpenCLSource>(id, guiText, OpenCLSource(programFilename), invalidationLevel)
                       , programDefines_()
                       , originalProgramFilename_(programFilename)
                       , programs_()
{
    value_.programFilename_ = originalProgramFilename_;
    programDefines_.push_back("");
}

OpenCLProperty::OpenCLProperty()
{
    programDefines_.push_back("");
}

OpenCLProperty::~OpenCLProperty() {
    if (!programs_.empty()) {
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

void OpenCLProperty::setDefines(std::string defs, size_t programID) {
    while (programID >= programDefines_.size())
        programDefines_.push_back("");
    programDefines_.at(programID) = defs;
}

std::string OpenCLProperty::getDefines(size_t programID) const {
    tgtAssert(!programDefines_.empty(), "no program defines");
    if (programID >= programDefines_.size()) {
        LERROR("No program configuration for index " + itos(programID) + " available");
        return "";
    }

    return programDefines_.at(programID);
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

bool OpenCLProperty::rebuild() {
    tgtAssert(!programDefines_.empty(), "no program defines");

    bool success = true;

    clearProgram();

    // create for all assigned build configurations
    for (size_t i=0; i<programDefines_.size(); i++) {
        cl::Program* program = new cl::Program(OpenCLModule::getInstance()->getCLContext());

        if (!value_.programFilename_.empty()) {

            if (!value_.programModified_) {
                program->loadSource(value_.programFilename_);
                value_.programSource_ = getProgramAsString(value_.programFilename_);
            }
            else {
                program->setSource(value_.programSource_);
            }

            program->setBuildOptions(programDefines_.at(i));
            bool success = program->build(OpenCLModule::getInstance()->getCLDevice());

            if (!success) {
                LERROR("Unable to build program " + itos(i));
                delete program;
                program = 0;
                success = false;
            }

            programs_.push_back(program);
        }

    }

    updateWidgets();

    return success;
}

void OpenCLProperty::clearProgram() {
    for (size_t i=0; i<programs_.size(); i++)
        delete programs_.at(i);
    programs_.clear();
    LGL_ERROR;
}

cl::Program* OpenCLProperty::getProgram(size_t programID) const {
    if (programID >= programs_.size() || programs_.at(programID) == 0) {
        if (programID > 0)
            LWARNING("Program with id " + itos(programID) + " not available");
        else
            LDEBUG("Program not available");
        return 0;
    }
    return programs_.at(programID);
}

}   // namespace
