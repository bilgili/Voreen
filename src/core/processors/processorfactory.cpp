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

#include "voreen/core/processors/processorfactory.h"

#include <algorithm> // for sorting vectors
#include <functional>

#include "voreen/core/voreenapplication.h"
#include "voreen/core/voreenmodule.h"

namespace voreen {

const std::string ProcessorFactory::loggerCat_("voreen.ProcessorFactory");

ProcessorFactory* ProcessorFactory::instance_ = 0;

ProcessorFactory::ProcessorFactory()
    : initialized_(false)
{}

ProcessorFactory::~ProcessorFactory() {
    instance_ = 0;
}

ProcessorFactory* ProcessorFactory::getInstance() {
    if (!instance_)
        instance_ = new ProcessorFactory();

    return instance_;
}

const std::map<std::string, Processor*>& ProcessorFactory::getClassMap() const {
    if (!initialized_) {
        initialize();
        initialized_ = true;
    }
    return classMap_;
}

const std::vector<Processor*>& ProcessorFactory::getRegisteredProcessors() const {
    if (!initialized_) {
        initialize();
        initialized_ = true;
    }
    return processors_;
}

bool ProcessorFactory::isProcessorKnown(const std::string& className) const {
    return (getProcessor(className) != 0);
}


const Processor* ProcessorFactory::getProcessor(const std::string& className) const {
    if (!initialized_) {
        initialize();
        initialized_ = true;
    }
    std::map<std::string, Processor*>::iterator it = classMap_.find(className);
    if (it != classMap_.end())
        return it->second;
    else
        return 0;
}


Processor* ProcessorFactory::create(const std::string& name) {
    if (!initialized_) {
        initialize();
        initialized_ = true;
    }

    std::map<std::string, Processor*>::iterator it = classMap_.find(name);
    if (it != classMap_.end() && it->second != 0) {
        Processor* proc = it->second->create();
        tgtAssert(proc, "No processor created");
        proc->setModuleName(it->second->getModuleName());
        proc->setDescriptions();
        return proc;
    }

    return 0;
}

const std::string ProcessorFactory::getTypeString(const std::type_info& type) const {
    if (!initialized_) {
        initialize();
        initialized_ = true;
    }

    for (std::map<std::string, Processor*>::const_iterator it = classMap_.begin();
        it != classMap_.end(); ++it)
    {
        if (type == typeid(*(it->second)))
            return it->first;
    }

    return "";
}

Serializable* ProcessorFactory::createType(const std::string& typeString) {
    return create(typeString);
}

void ProcessorFactory::registerClass(Processor* const newClass) const {
    tgtAssert(newClass, "null pointer passed");
    tgtAssert(!initialized_, "already initialized");

    if (classMap_.find(newClass->getClassName()) == classMap_.end()) {
        processors_.push_back(newClass);
        classMap_.insert(std::make_pair(newClass->getClassName(), newClass));

        // check whether newClass is assigned to a valid module
        if (newClass->getModuleName() == "undefined") {
            LWARNING("Processor class '" << newClass->getClassName()
                     << "' is not assigned to a valid module. Check your module class.");
        }
    }
    else {
        LWARNING("Processor class '" << newClass->getClassName() << "' has already been registered. Skipping.");
    }
}

void ProcessorFactory::initialize() const {
    if (initialized_) {
        LERROR("initializeClassList() already initialized");
        return;
    }

    if (!VoreenApplication::app()) {
        LERROR("initializeClassList() VoreenApplication not instantiated");
        return;
    }

    // retrieve processors from modules and register them
    const std::vector<VoreenModule*> modules = VoreenApplication::app()->getModules();
    if (modules.empty())
        LWARNING("No modules found");

    for (size_t m=0; m<modules.size(); m++) {
        for (size_t p=0; p < modules.at(m)->getProcessors().size(); p++) {
            registerClass(modules.at(m)->getProcessors().at(p));
        }
    }

}


} // namespace voreen
