/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/core/voreenmodule.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/ports/port.h"
#include "voreen/core/utils/stringutils.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/processors/processorwidgetfactory.h"
#include "voreen/core/properties/property.h"
#include "voreen/core/properties/propertyfactory.h"
#include "voreen/core/properties/propertywidget.h"
#include "voreen/core/properties/propertywidgetfactory.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "voreen/core/io/volumereader.h"
#include "voreen/core/io/volumewriter.h"
#include "voreen/core/io/serialization/serializablefactory.h"

#include "tgt/shadermanager.h"
#include "tgt/filesystem.h"

namespace voreen {

const std::string VoreenModule::loggerCat_("voreen.VoreenModule");

VoreenModule::VoreenModule(const std::string& modulePath)
    : initialized_(false)
    , initializedGL_(false)
    , name_("<undefined>")
    , dirName_("<undefined>")
    , modulePath_("<undefined>")
{
    if (modulePath == "")
        LWARNING("Module path is empty");
    else {
        if (!tgt::FileSystem::isAbsolutePath(modulePath))
            modulePath_ = VoreenApplication::app()->getBasePath(modulePath); //< module path is relative to base dir
        else
            modulePath_ = modulePath; //< module path lies outside the source tree

        dirName_ = tgt::FileSystem::fileName(modulePath);

        if (!tgt::FileSystem::dirExists(modulePath_))
            LDEBUG("Module path does not exist: " + modulePath);
    }
}

VoreenModule::~VoreenModule() {

    if (isInitializedGL()) {
        LWARNING("Module '" << getName() << "' has not been OpenGL deinitialized before destruction");
    }

    if (isInitialized()) {
        LWARNING("Module '" << getName() << "' has not been deinitialized before destruction");
    }

    // free resources
    for (size_t i=0; i<processors_.size(); i++)
        delete processors_[i];
    processors_.clear();

    for (size_t i=0; i<properties_.size(); i++)
        delete properties_[i];
    properties_.clear();

    for (size_t i=0; i<volumeReaders_.size(); i++)
        delete volumeReaders_[i];
    volumeReaders_.clear();

    for (size_t i=0; i<volumeWriters_.size(); i++)
        delete volumeWriters_[i];
    volumeWriters_.clear();

    for (size_t i=0; i<serializerFactories_.size(); i++)
        delete serializerFactories_[i];
    serializerFactories_.clear();

    for (size_t i=0; i<processorWidgetFactories_.size(); i++)
        delete processorWidgetFactories_[i];
    processorWidgetFactories_.clear();

    for (size_t i=0; i<propertyWidgetFactories_.size(); i++)
        delete propertyWidgetFactories_[i];
    propertyWidgetFactories_.clear();

    // each link evaluator factory is also registered as serializable factory
    // and has thus already been deleted!
    /*for (size_t i=0; i<linkEvaluatorFactories_.size(); i++)
        delete linkEvaluatorFactories_[i];
    linkEvaluatorFactories_.clear(); */
}

std::string voreen::VoreenModule::getName() const {
    return name_;
}

std::string VoreenModule::getDirName() const {
    return dirName_;
}

const std::vector<Processor*>& VoreenModule::getProcessors() const {
    return processors_;
}

const std::vector<Property*>& VoreenModule::getRegisteredProperties() const {
    return properties_;
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

const std::vector<ProcessorWidgetFactory*>& VoreenModule::getProcessorWidgetFactories() const {
    return processorWidgetFactories_;
}

const std::vector<PropertyWidgetFactory*>& VoreenModule::getPropertyWidgetFactories() const {
    return propertyWidgetFactories_;
}

const std::vector<LinkEvaluatorFactory*>& VoreenModule::getLinkEvaluatorFactories() const {
    return linkEvaluatorFactories_;
}

const std::vector<std::string>& VoreenModule::getShaderPaths() const {
    return shaderPaths_;
}


// protected

void VoreenModule::initialize() throw (tgt::Exception) {
    // create and register property factory
    if (!properties_.empty()) {
        PropertyFactory* propFactory = new PropertyFactory();
        for (size_t i=0; i<properties_.size(); i++)
            propFactory->registerProperty(properties_[i]);
        registerSerializerFactory(propFactory);
    }
}

void voreen::VoreenModule::deinitialize() throw (tgt::Exception) {
}

void VoreenModule::initializeGL() throw (tgt::Exception) {
    for (size_t i=0; i<shaderPaths_.size(); i++) {
        ShdrMgr.addPath(shaderPaths_.at(i));
    }
}

void VoreenModule::deinitializeGL() throw (tgt::Exception)
{}

void VoreenModule::setName(const std::string& name) {
    name_ = name;
}

void VoreenModule::registerProcessor(Processor* processor) {
    tgtAssert(processor, "null pointer passed");
    if (name_ == "<undefined>") {
        LWARNING("registerProcessor(): module name is undefined");
    }
    processor->setModuleName(name_);
    processors_.push_back(processor);
}

void VoreenModule::registerProperty(Property* property) {
    tgtAssert(property, "null pointer passed");
    properties_.push_back(property);
}

void VoreenModule::registerVolumeReader(VolumeReader* reader) {
    tgtAssert(reader, "null pointer passed");
    volumeReaders_.push_back(reader);
}

void VoreenModule::registerVolumeWriter(VolumeWriter* writer) {
    tgtAssert(writer, "null pointer passed");
    volumeWriters_.push_back(writer);
}

void VoreenModule::registerSerializerFactory(SerializableFactory* factory) {
    tgtAssert(factory, "null pointer passed");
    serializerFactories_.push_back(factory);
}

void VoreenModule::registerProcessorWidgetFactory(ProcessorWidgetFactory* factory) {
    tgtAssert(factory, "null pointer passed");
    processorWidgetFactories_.push_back(factory);
}

void VoreenModule::registerPropertyWidgetFactory(PropertyWidgetFactory* factory) {
    tgtAssert(factory, "null pointer passed");
    propertyWidgetFactories_.push_back(factory);
}

void VoreenModule::registerLinkEvaluatorFactory(LinkEvaluatorFactory* factory) {
    tgtAssert(factory, "null pointer passed");
    linkEvaluatorFactories_.push_back(factory);

    // link evaluator factories are also serializable factories
    serializerFactories_.push_back(factory);
}

void VoreenModule::addShaderPath(const std::string& path) {
    shaderPaths_.push_back(path);
}

std::string VoreenModule::getModulePath(const std::string& suffix) const {
    /*if (!VoreenApplication::app()) {
        LERROR("getModulePath(): VoreenApplication not instantiated");
        return "";
    }
    return VoreenApplication::app()->getModulePath(getDirName() + (suffix.empty() ? "" : "/" + suffix)); */

    return tgt::FileSystem::cleanupPath(modulePath_ + (suffix.empty() ? "" : "/" + suffix));
}

bool VoreenModule::isInitialized() const {
    return initialized_;
}

bool VoreenModule::isInitializedGL() const {
    return initializedGL_;
}

std::string VoreenModule::getDocumentationDescription(const std::string& className) const {
    for(size_t i=0; i<processors_.size(); i++) {
        if(processors_[i]->getClassName() == className) {
            return processors_[i]->getDescription();
        }
    }
    return "";
}

std::vector<std::pair<std::string, std::string> > VoreenModule::getDocumentationPorts(const std::string& className) const {
    std::vector<std::pair<std::string, std::string> > result;

    for(size_t i=0; i<processors_.size(); i++) {
        if(processors_[i]->getClassName() == className) {
            Processor* proc = processors_[i];

            const std::vector<Port*>& ports = proc->getPorts();
            for(size_t i=0; i<ports.size(); i++) {
                if(ports[i]->getDescription() != "")
                    result.push_back(std::pair<std::string, std::string>(ports[i]->getName(), ports[i]->getDescription()));
            }
            return result;
        }
    }

    return result;
}

std::string VoreenModule::getDocumentationPort(const std::string& className, const std::string& portName) const {
    std::vector<std::pair<std::string, std::string> > result;
    for(size_t i=0; i<processors_.size(); i++) {
        if(processors_[i]->getClassName() == className) {
            return processors_[i]->getPortDescription(portName);
        }
    }
    return "";
}

std::vector<std::pair<std::string, std::string> > VoreenModule::getDocumentationProperties(const std::string& className) const {
    std::vector<std::pair<std::string, std::string> > result;

    for(size_t i=0; i<processors_.size(); i++) {
        if(processors_[i]->getClassName() == className) {
            Processor* proc = processors_[i];

            const std::vector<Property*>&  props = proc->getProperties();
            for(size_t i=0; i<props.size(); i++) {
                if(props[i]->getDescription() != "")
                    result.push_back(std::pair<std::string, std::string>(props[i]->getID(), props[i]->getDescription()));
            }
            return result;
        }
    }

    return result;
}

std::string VoreenModule::getDocumentationProperty(const std::string& className, const std::string& propertyID) const {
    std::vector<std::pair<std::string, std::string> > result;
    for(size_t i=0; i<processors_.size(); i++) {
        if(processors_[i]->getClassName() == className) {
            return processors_[i]->getPropertyDescription(propertyID);
        }
    }
    return "";
}

} // namespace
