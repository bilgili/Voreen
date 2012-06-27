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

#include "voreen/core/processors/processorfactory.h"

#include <algorithm> // for sorting vectors
#include <functional>

#include "voreen/core/voreenapplication.h"
#include "voreen/core/voreenmodule.h"

#include "voreen/core/processors/canvasrenderer.h"

namespace voreen {

const std::string ProcessorFactory::loggerCat_("voreen.ProcessorFactory");

ProcessorFactory* ProcessorFactory::instance_ = 0;

ProcessorFactory::ProcessorFactory() {
    initializeClassList();
}

ProcessorFactory::~ProcessorFactory() {
    // Delete only those processors that belong to the core, as all others will be deleted by
    // their individual module.
    for (std::map<std::string, Processor*>::iterator it = classList_.begin(); it != classList_.end(); ++it) {
        if (it->second->getModuleName() == "Core")
            delete it->second;
    }
}

ProcessorFactory* ProcessorFactory::getInstance() {
    if (!instance_)
        instance_ = new ProcessorFactory();

    return instance_;
}

bool ProcessorFactory::isProcessorKnown(const std::string& className) const {
    return (classList_.find(className) != classList_.end());
}

std::string ProcessorFactory::getProcessorInfo(const std::string& name) const {
    std::map<std::string, Processor*>::const_iterator it = classList_.find(name);
    if (it != classList_.end() && it->second != 0)
        return it->second->getProcessorInfo();

    return "";
}

std::string ProcessorFactory::getProcessorCategory(const std::string& name) const {
    std::map<std::string, Processor*>::const_iterator it = classList_.find(name);
    if (it != classList_.end() && it->second != 0)
        return it->second->getCategory();

    return "";
}

std::string ProcessorFactory::getProcessorModuleName(const std::string& name) const {
    std::map<std::string, Processor*>::const_iterator it = classList_.find(name);
    if (it != classList_.end() && it->second != 0)
        return it->second->getModuleName();

    return "";
}

// Returns processor code state
Processor::CodeState ProcessorFactory::getProcessorCodeState(const std::string& name) const {
    std::map<std::string, Processor*>::const_iterator it = classList_.find(name);
    if (it != classList_.end() && it->second != 0)
        return it->second->getCodeState();

    return Processor::CODE_STATE_BROKEN;
}

void ProcessorFactory::destroy() {
    delete instance_;
    instance_ = 0;
}

Processor* ProcessorFactory::create(const std::string& name) {
    std::map<std::string, Processor*>::iterator it = classList_.find(name);
    if (it != classList_.end() && it->second != 0) {
        Processor* proc = it->second->create();
        tgtAssert(proc, "No processor created");
        proc->setModuleName(it->second->getModuleName());
        return proc;
    }

    return 0;
}

const std::string ProcessorFactory::getTypeString(const std::type_info& type) const {
    for (std::map<std::string, Processor*>::const_iterator it = classList_.begin();
        it != classList_.end(); ++it)
    {
        if (type == typeid(*(it->second)))
            return it->first;
    }

    return "";
}

Serializable* ProcessorFactory::createType(const std::string& typeString) {
    return create(typeString);
}

void ProcessorFactory::registerClass(Processor* const newClass, bool isCore) {
    tgtAssert(newClass, "null pointer passed");

    if (!isProcessorKnown(newClass->getClassName())) {
        classList_.insert(std::make_pair(newClass->getClassName(), newClass));

        // In the KnownClassesVector the elements are pairs of string, with
        // the first part of the pair identifying the category name of the
        // processor and the second one the class name.
        knownClasses_.push_back(std::make_pair(newClass->getCategory(), newClass->getClassName()));

        // assign class to the core if needed
        if (isCore)
            newClass->setModuleName("Core");

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

namespace {

// helper for sorting
struct KnownClassesOrder {
    bool operator()(const ProcessorFactory::StringPair& lhs, const ProcessorFactory::StringPair& rhs) const {
        return ((lhs.first + "." + lhs.second) < (rhs.first + "." + rhs.second));
    }
};

} // namespace

void ProcessorFactory::initializeClassList() {
    if (!VoreenApplication::app()) {
        LERROR("VoreenApplication not instantiated");
        return;
    }

    // register processors belonging to the core directly
    registerClass(new CanvasRenderer(), true);

    // retrieve processors from modules and register them
    const std::vector<VoreenModule*> modules = VoreenApplication::app()->getModules();
    if (modules.empty())
        LWARNING("No modules found");

    for (size_t m=0; m<modules.size(); m++) {
        for (size_t p=0; p < modules.at(m)->getProcessors().size(); p++) {
            registerClass(modules.at(m)->getProcessors().at(p));
        }
    }

    // Sort processor list in alphabetical order.
    std::sort(knownClasses_.begin(), knownClasses_.end(), KnownClassesOrder());
}

} // namespace voreen
