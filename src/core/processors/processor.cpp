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

#include "voreen/core/processors/processor.h"
#include "voreen/core/properties/property.h"

#include "tgt/glmath.h"
#include "tgt/shadermanager.h"
#include "tgt/gpucapabilities.h"
#include "tgt/textureunit.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/processors/processorwidgetfactory.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/processors/processorwidget.h"
#include "voreen/core/properties/eventproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/interaction/interactionhandler.h"

#include "voreen/core/io/serialization/xmlserializer.h"
#include "voreen/core/io/serialization/xmldeserializer.h"
#include "voreen/core/processors/processorfactory.h"
#include "voreen/core/datastructures/transfunc/transfuncfactory.h"

#include <sstream>

using tgt::vec3;
using tgt::vec4;
using tgt::Color;

using std::string;
using std::map;
using std::vector;

namespace voreen {

const std::string Processor::loggerCat_("voreen.Processor");

Processor::Processor()
    : PropertyOwner()
    , initialized_(false)
    , progressBar_(0)
    , name_("Processor")
    , moduleName_("undefined")
    , processorWidget_(0)
    , invalidationVisited_(false)
    , interactionModeVisited_(false)
    , eventVisited_(false)
{
}

Processor::~Processor() {
    if (isInitialized()) {
        LERROR("~Processor(): '" << getName() << "' has not been deinitialized");
        return;
    }
}

Processor* Processor::clone() const {
    std::stringstream stream;

    // first serialize
    XmlSerializer s;
    s.registerFactory(ProcessorFactory::getInstance());
    s.registerFactory(TransFuncFactory::getInstance());
    s.serialize("this", this);
    s.write(stream);

    // then deserialize again
    XmlDeserializer d;
    d.registerFactory(ProcessorFactory::getInstance());
    d.registerFactory(TransFuncFactory::getInstance());
    d.read(stream);
    Processor* proc = 0;
    d.deserialize("this", proc);

    return proc;
}

void Processor::initialize() throw (VoreenException) {

    tgtAssert(VoreenApplication::app(), "VoreenApplication not instantiated");

    if (isInitialized()) {
        LWARNING("initialize(): '" << getName() << "' already initialized");
        return;
    }

    // create and initialize processor widget
    if (VoreenApplication::app()->getProcessorWidgetFactory()) {
        processorWidget_ = VoreenApplication::app()->getProcessorWidgetFactory()->createWidget(this);
        if (processorWidget_) {
            processorWidget_->initialize();

            // inform the observers about the new widget
            std::vector<ProcessorObserver*> observers = getObservers();
            for (size_t i = 0; i < observers.size(); ++i)
                observers[i]->processorWidgetCreated(this);
        }
    }

    // initialize ports
    const std::vector<Port*>& ports = getPorts();
    for (size_t i=0; i < ports.size(); ++i)
        ports[i]->initialize();


    // initialize properties
    const std::vector<Property*>& properties = getProperties();
    for (size_t i=0; i < properties.size(); ++i)
        properties[i]->initialize();

    initialized_ = true;
}

void Processor::deinitialize() throw (VoreenException) {
    if (!isInitialized()) {
        LWARNING("deinitialize(): '" << getName() << "' (" << getClassName() << ") not initialized");
        return;
    }

    LGL_ERROR;

    // deinitialize ports
    const std::vector<Port*>& ports = getPorts();
    for (size_t i=0; i < ports.size(); ++i)
        ports[i]->deinitialize();
    LGL_ERROR;

    // deinitialize properties
    const std::vector<Property*>& properties = getProperties();
    for (size_t i=0; i < properties.size(); ++i)
        properties[i]->deinitialize();
    LGL_ERROR;

    // delete processor widget
    delete processorWidget_;
    processorWidget_ = 0;

    // notify observers about the deleted widget
    std::vector<ProcessorObserver*> observers = getObservers();
    for (size_t i = 0; i < observers.size(); ++i)
        observers[i]->processorWidgetDeleted(this);

    initialized_ = false;
}

void Processor::beforeProcess() {
}

void Processor::afterProcess() {
}

bool Processor::isInitialized() const {
    return initialized_;
}

void Processor::addPort(Port* port) {
    tgtAssert(port, "Null pointer passed");
    port->setProcessor(this);
    CoProcessorPort* cpp = dynamic_cast<CoProcessorPort*>(port);
    if (port->isOutport()) {
        if (cpp)
            coProcessorOutports_.push_back(cpp);
        else
            outports_.push_back(port);
    }
    else {
        if (cpp)
            coProcessorInports_.push_back(cpp);
        else
            inports_.push_back(port);
    }

    map<std::string, Port*>::const_iterator it = portMap_.find(port->getName());
    if (it == portMap_.end())
        portMap_.insert(std::make_pair(port->getName(), port));
    else {
        LERROR("Port with name " << port->getName() << " has already been inserted!");
        tgtAssert(false, std::string("Port with name " + port->getName() + " has already been inserted").c_str());
    }
}

void Processor::addPort(Port& port) {
    addPort(&port);
}

Processor::CodeState Processor::getCodeState() const {
    return Processor::CODE_STATE_EXPERIMENTAL;
}

void Processor::setName(const std::string& name) {
    name_ = name;
    if (processorWidget_)
        processorWidget_->processorNameChanged();
}

std::string Processor::getName() const {
    return name_;
}

void Processor::setModuleName(const std::string& moduleName) {
    moduleName_ = moduleName;
}

std::string Processor::getModuleName() const {
    return moduleName_;
}

bool Processor::isUtility() const {
    return false;
}

std::string Processor::getProcessorInfo() const {
    return "No information available";
}

const std::vector<Port*>& Processor::getInports() const {
    return inports_;
}

const std::vector<Port*>& Processor::getOutports() const {
    return outports_;
}

const std::vector<CoProcessorPort*>& Processor::getCoProcessorInports() const {
    return coProcessorInports_;
}

const std::vector<CoProcessorPort*>& Processor::getCoProcessorOutports() const {
    return coProcessorOutports_;
}

std::vector<Port*> Processor::getPorts() const {
    std::vector<Port*> result;

    const std::vector<Port*> inports = getInports();
    result.insert(result.begin(), inports.begin(), inports.end());

    const std::vector<Port*> outports = getOutports();
    result.insert(result.end(), outports.begin(), outports.end());

    const std::vector<CoProcessorPort*> coProcessorInports = getCoProcessorInports();
    result.insert(result.begin(), coProcessorInports.begin(), coProcessorInports.end());

    const std::vector<CoProcessorPort*> coProcessorOutports = getCoProcessorOutports();
    result.insert(result.end(), coProcessorOutports.begin(), coProcessorOutports.end());

    return result;
}

Port* Processor::getPort(const std::string& name) const {
    const std::vector<Port*> ports = getPorts();

    for (size_t i=0; i < ports.size(); i++) {
        if (ports[i]->getName() == name)
            return ports[i];
    }

    return 0;
}

void Processor::invalidate(int inv) {
    PropertyOwner::invalidate(inv);

    if (inv == Processor::VALID)
        return;

    if (!isInitialized())
        return;

    if (!invalidationVisited_) {
        invalidationVisited_ = true;

        for (size_t i=0; i<outports_.size(); ++i)
            outports_[i]->invalidate();

        for (size_t i=0; i<coProcessorOutports_.size(); ++i)
            coProcessorOutports_[i]->invalidate();

        if (isEndProcessor()) {
            tgtAssert(VoreenApplication::app(), "VoreenApplication not instantiated");
            // triggers non-blocking network update
            VoreenApplication::app()->scheduleNetworkProcessing();
        }

        invalidationVisited_ = false;
    }
}

bool Processor::isReady() const {
    if (!isInitialized())
        return false;

    for(size_t i=0; i<inports_.size(); ++i)
        if (!inports_[i]->isReady())
            return false;

    for (size_t i=0; i<outports_.size(); ++i)
        if(!outports_[i]->isReady())
            return false;

    for (size_t i=0; i<coProcessorInports_.size(); ++i)
        if (!coProcessorInports_[i]->isReady())
            return false;

    return true;
}

void Processor::toggleInteractionMode(bool interactionMode, void* source) {

    if (!interactionModeVisited_) {

        interactionModeVisited_ = true;

        if (interactionMode) {
            if (interactionModeSources_.find(source) == interactionModeSources_.end()) {
                PropertyOwner::toggleInteractionMode(interactionMode, source);
                // propagate over outports
                for (size_t i=0; i<outports_.size(); ++i)
                    outports_[i]->toggleInteractionMode(true, source);

                for (size_t i=0; i<coProcessorOutports_.size(); ++i)
                    coProcessorOutports_[i]->toggleInteractionMode(true, source);
            }
        } else {
            if (interactionModeSources_.find(source) != interactionModeSources_.end()) {
                PropertyOwner::toggleInteractionMode(interactionMode, source);
                // propagate over outports
                for (size_t i=0; i<outports_.size(); ++i)
                    outports_[i]->toggleInteractionMode(false, source);

                for (size_t i=0; i<coProcessorOutports_.size(); ++i)
                    coProcessorOutports_[i]->toggleInteractionMode(false, source);
            }
        }

        interactionModeVisited_ = false;
    }
}

void Processor::interactionModeToggled() {
    if (interactionMode())
        LDEBUG(getName() << " interactionModeSwitched  on");
    else
        LDEBUG(getName() << " interactionModeSwitched  off");
}

bool Processor::isValid() const {
    return PropertyOwner::isValid();
}

void Processor::setValid() {
    PropertyOwner::setValid();

    for (size_t i=0; i<inports_.size(); ++i)
        inports_[i]->setValid();

    for (size_t i=0; i<coProcessorInports_.size(); ++i)
        coProcessorInports_[i]->setValid();
}

bool Processor::isEndProcessor() const {
    return (outports_.empty() && coProcessorOutports_.empty());
}

void Processor::serialize(XmlSerializer& s) const {
    // meta data
    metaDataContainer_.serialize(s);

    // misc settings
    s.serialize("name", name_);

    // serialize properties
    PropertyOwner::serialize(s);

    // create temporary interaction handler map for deserialization
    map<string, InteractionHandler*> handlerMap;
    const std::vector<InteractionHandler*>& handlers = getInteractionHandlers();
    for (vector<InteractionHandler*>::const_iterator it = handlers.begin(); it != handlers.end(); ++it)
        handlerMap[(*it)->getID()] = *it;

    // serialize handlers
    const bool usePointerContentSerialization = s.getUsePointerContentSerialization();
    s.setUsePointerContentSerialization(true);
    s.serialize("InteractionHandlers", handlerMap, "Handler", "name");
    s.setUsePointerContentSerialization(usePointerContentSerialization);
}

void Processor::deserialize(XmlDeserializer& s) {
    // meta data
    metaDataContainer_.deserialize(s);

    // misc settings
    s.deserialize("name", name_);

    // deserialize properties
    PropertyOwner::deserialize(s);

    // create temporary interaction handler map for deserialization
    map<string, InteractionHandler*> handlerMap;
    const std::vector<InteractionHandler*>& handlers = getInteractionHandlers();
    for (vector<InteractionHandler*>::const_iterator it = handlers.begin(); it != handlers.end(); ++it)
        handlerMap[(*it)->getID()] = *it;

    // deserialize handlers
    const bool usePointerContentSerialization = s.getUsePointerContentSerialization();
    try {
        s.setUsePointerContentSerialization(true);
        s.deserialize("InteractionHandlers", handlerMap, "Handler", "name");
    }
    catch (XmlSerializationNoSuchDataException& /*e*/){
        // interaction handler key missing => just ignore
        s.removeLastError();
    }
    s.setUsePointerContentSerialization(usePointerContentSerialization);
}

MetaDataContainer& Processor::getMetaDataContainer() const {
    return metaDataContainer_;
}

void Processor::setProgressBar(ProgressBar* progressBar) {
    progressBar_ = progressBar;
}

void Processor::setProgress(float progress) {
    if (progressBar_)
        progressBar_->setProgress(progress);
}

ProcessorWidget* Processor::getProcessorWidget() const {
    return processorWidget_;
}

bool Processor::usesExpensiveComputation() const {
    return false;
}

void Processor::addEventProperty(EventPropertyBase* prop) {
    tgtAssert(prop, "Null pointer passed");
    addProperty(prop);
    prop->setOwner(this);
    eventProperties_.push_back(prop);
}

void Processor::addEventProperty(EventPropertyBase& prop) {
    addEventProperty(&prop);
}

void Processor::addInteractionHandler(InteractionHandler* handler) {
    tgtAssert(handler, "Null pointer passed");
    handler->setOwner(this);
    interactionHandlers_.push_back(handler);

    if (handler->getEventProperties().empty())
        LWARNING("Interaction handler '" << handler->getName() << "' has no event property.");
}

void Processor::addInteractionHandler(InteractionHandler& handler) {
    addInteractionHandler(&handler);
}
const std::vector<EventPropertyBase*> Processor::getEventProperties() const {
    return eventProperties_;
}

const std::vector<InteractionHandler*>& Processor::getInteractionHandlers() const {
    return interactionHandlers_;
}

void Processor::onEvent(tgt::Event* e) {
    // cycle prevention
    if (eventVisited_)
        return;

    eventVisited_ = true;
    e->ignore();

    // propagate to interaction handlers (via their event properties)
    for (size_t i=0; i<interactionHandlers_.size() && !e->isAccepted(); ++i) {
        for (size_t j=0; j<interactionHandlers_[i]->getEventProperties().size() && !e->isAccepted(); ++j) {
            interactionHandlers_[i]->getEventProperties().at(j)->execute(e);
        }
    }

    // propagate to event properties
    for (size_t i = 0; (i < eventProperties_.size()) && !e->isAccepted(); ++i)
        eventProperties_[i]->execute(e);

    // pass-over event to processor's default event handling
    if (!e->isAccepted())
        tgt::EventListener::onEvent(e);

    // propagate to coprocessor inports
    for (size_t i=0; i<coProcessorInports_.size() && !e->isAccepted(); ++i)
        coProcessorInports_[i]->distributeEvent(e);

    // propagate to inports
    for (size_t i=0; i<inports_.size() && !e->isAccepted(); ++i)
        inports_[i]->distributeEvent(e);

    eventVisited_ = false;
}

void Processor::deregisterWidget() {
    processorWidget_ = 0;
}

} // namespace voreen
