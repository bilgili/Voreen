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

#include "voreen/core/voreenmodule.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/io/volumereader.h"
#include "voreen/core/io/volumewriter.h"
#include "voreen/core/io/serialization/serializablefactory.h"

#include "tgt/shadermanager.h"

namespace voreen {

const std::string VoreenModule::loggerCat_("voreen.VoreenModule");

VoreenModule::VoreenModule() :
    name_("undefined"),
    initialized_(false)
{}

VoreenModule::~VoreenModule() {

    if (isInitialized()) {
        LWARNING("Module '" << getName() << "' has not been deinitialized before destruction");
    }

    // free resources
    for (size_t i=0; i<processors_.size(); i++)
        delete processors_[i];
    processors_.clear();

    for (size_t i=0; i<volumeReaders_.size(); i++)
        delete volumeReaders_[i];
    volumeReaders_.clear();

    for (size_t i=0; i<volumeWriters_.size(); i++)
        delete volumeWriters_[i];
    volumeWriters_.clear();

    for (size_t i=0; i<serializerFactories_.size(); i++)
        delete serializerFactories_[i];
    serializerFactories_.clear();
}

std::string voreen::VoreenModule::getName() const {
    return name_;
}

const std::vector<Processor*>& VoreenModule::getProcessors() const {
    return processors_;
}

const std::vector<VolumeReader*>& VoreenModule::getVolumeReaders() const {
    return volumeReaders_;
}

const std::vector<VolumeWriter*>& VoreenModule::getVolumeWriters() const {
    return volumeWriters_;
}

const std::vector<SerializableFactory*>& VoreenModule::getSerializerFactories() const {
    return serializerFactories_;
}

const std::vector<std::string>& VoreenModule::getShaderPaths() const {
    return shaderPaths_;
}

// protected

void VoreenModule::initialize()
    throw (VoreenException)
{
    for (size_t i=0; i<shaderPaths_.size(); i++) {
        ShdrMgr.addPath(shaderPaths_.at(i));
    }
}

void voreen::VoreenModule::deinitialize()
    throw (VoreenException)
{ }

void VoreenModule::setName(const std::string& name) {
    if (name == "Core" || name == "core") {
        LWARNING("Module name 'Core' is reserved. Renaming to 'CustomCore'.");
        name_ = "CustomCore";
    }
    else {
        name_ = name;
    }
}

void VoreenModule::addProcessor(Processor* processor) {
    tgtAssert(processor, "null pointer passed");
    if (name_ == "undefined") {
        LWARNING("addProcessor(): module name is undefined");
    }
    processor->setModuleName(name_);
    processors_.push_back(processor);
}

void VoreenModule::addVolumeReader(VolumeReader* reader) {
    tgtAssert(reader, "null pointer passed");
    volumeReaders_.push_back(reader);
}

void VoreenModule::addVolumeWriter(VolumeWriter* writer) {
    tgtAssert(writer, "null pointer passed");
    volumeWriters_.push_back(writer);
}

void VoreenModule::addSerializerFactory(SerializableFactory* factory) {
    tgtAssert(factory, "null pointer passed");
    serializerFactories_.push_back(factory);
}

void VoreenModule::addShaderPath(const std::string& path) {
    shaderPaths_.push_back(path);
}

std::string VoreenModule::getModulesPath(const std::string& suffix) const {
    if (!VoreenApplication::app()) {
        LERROR("getModulesPath(): VoreenApplication not instantiated");
        return "";
    }
    return VoreenApplication::app()->getModulePath(suffix);
}

bool VoreenModule::isInitialized() const {
    return initialized_;
}

} // namespace
