/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/properties/property.h"

#include "tgt/glmath.h"
#include "tgt/shadermanager.h"
#include "tgt/gpucapabilities.h"

#include "voreen/core/application.h"
#include "voreen/core/vis/processors/processorwidgetfactory.h"
#include "voreen/core/vis/processors/ports/allports.h"
#include "voreen/core/vis/processors/processorwidget.h"
#include "voreen/core/vis/properties/eventproperty.h"
#include "voreen/core/vis/properties/transferfuncproperty.h"
#include "voreen/core/vis/interaction/interactionhandler.h"

#include "voreen/core/io/serialization/xmlserializer.h"
#include "voreen/core/io/serialization/xmldeserializer.h"
#include "voreen/core/vis/processors/processorfactory.h"
#include "voreen/core/vis/transfunc/transfuncfactory.h"

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
    : initialized_(false)
    , name_("Processor")
    , invalidationLevel_(INVALID_PROCESSOR)
    , processorWidget_(0)
    , invalidationVisited_(false)
{
}

Processor::~Processor() {
    delete processorWidget_;
    processorWidget_ = 0;
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
    if (isInitialized()) {
        LWARNING("initialize(): already initialized");
    }

    // create and initialize processor widget
    if (VoreenApplication::app()->getProcessorWidgetFactory()) {
        processorWidget_ = VoreenApplication::app()->getProcessorWidgetFactory()->createWidget(this);
        if (processorWidget_)
            processorWidget_->initialize();
    }

    // initialize properties
    for (size_t i=0; i<properties_.size(); ++i) {
        properties_[i]->initialize();
    }

    initialized_ = true;
}

bool Processor::isInitialized() const {
    return initialized_;
}

void Processor::addProperty(Property* prop) {
    properties_.push_back(prop);
    prop->setOwner(this);
}

void Processor::addProperty(Property& prop) {
    addProperty(&prop);
}

const Properties& Processor::getProperties() const {
    return properties_;
}

void Processor::disconnectAllPorts() {
    for (size_t i=0; i<inports_.size(); ++i)
        inports_[i]->disconnectAll();

    for (size_t i=0; i<outports_.size(); ++i)
        outports_[i]->disconnectAll();

    for (size_t i=0; i<coProcessorInports_.size(); ++i)
        coProcessorInports_[i]->disconnectAll();

    for (size_t i=0; i<coProcessorOutports_.size(); ++i)
        coProcessorOutports_[i]->disconnectAll();
}

void Processor::addPort(Port* port) {
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
    else
        LERROR("Port with name " << port->getName() << " has already been inserted!");
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

const std::string& Processor::getName() const {
    return name_;
}

std::string Processor::getModuleName() const {
    return "unknown";
}

bool Processor::isUtility() const {
    return false;
}

const std::string Processor::getProcessorInfo() const {
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

const std::vector<Port*> Processor::getPorts() const {
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

void Processor::invalidate(InvalidationLevel inv) {
    if (!invalidationVisited_) {
        invalidationVisited_ = true;

        invalidationLevel_ = std::max(invalidationLevel_, inv);

        for (size_t i=0; i<outports_.size(); ++i) {
            outports_[i]->invalidate();
        }
        for (size_t i=0; i<coProcessorOutports_.size(); ++i) {
            coProcessorOutports_[i]->invalidate();
        }

        invalidationVisited_ = false;
    }
}

bool Processor::isReady() const {
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

    if (interactionMode) {
        if (interactionModeSources_.find(source) == interactionModeSources_.end()) {
            interactionModeSources_.insert(source);

            // propagate over outports
            for (size_t i=0; i<outports_.size(); ++i) {
                outports_[i]->toggleInteractionMode(true, source);
            }
            for (size_t i=0; i<coProcessorOutports_.size(); ++i) {
                coProcessorOutports_[i]->toggleInteractionMode(true, source);
            }

            if (interactionModeSources_.size() == 1) {
                interactionModeToggled();
            }
        }
    }
    else {
        if (interactionModeSources_.find(source) != interactionModeSources_.end()) {
            interactionModeSources_.erase(source);

            // propagate over outports
            for (size_t i=0; i<outports_.size(); ++i) {
                outports_[i]->toggleInteractionMode(false, source);
            }
            for (size_t i=0; i<coProcessorOutports_.size(); ++i) {
                coProcessorOutports_[i]->toggleInteractionMode(false, source);
            }

            if (interactionModeSources_.empty()) {
                interactionModeToggled();
            }
        }

    }
}

bool Processor::interactionMode() const {
    return (!interactionModeSources_.empty());
}

void Processor::interactionModeToggled() {
    if (interactionMode()) {
        LDEBUG(getName() << " interactionModeSwitched  on");
    }
    else {
        LDEBUG(getName() << " interactionModeSwitched  off");
    }
}

Processor::InvalidationLevel Processor::getInvalidationLevel() const {
    return invalidationLevel_;
}

bool Processor::isValid() const {
    return (invalidationLevel_ == VALID);
}

void Processor::setValid() {
    invalidationLevel_ = VALID;

    for (size_t i=0; i<inports_.size(); ++i) {
        inports_[i]->setValid();
    }

    for (size_t i=0; i<coProcessorInports_.size(); ++i) {
        coProcessorInports_[i]->setValid();
    }
}

bool Processor::isEndProcessor() const {
    return ((outports_.empty()) && (coProcessorOutports_.empty()));
}

bool Processor::isRootProcessor() const {
    return false;
}

std::string Processor::getState() const {
    std::string state;
    for (size_t i = 0; i < properties_.size(); ++i)
        state += properties_[i]->toString();
    return state;
}

void Processor::serialize(XmlSerializer& s) const {
    // meta data
    metaDataContainer_.serialize(s);

    // misc settings
    s.serialize("name", name_);

    // create temporary property map for serialization
    map<string, Property*> propertyMap;
    for (vector<Property*>::const_iterator it = properties_.begin(); it != properties_.end(); ++it)
        propertyMap[(*it)->getId()] = *it;

    // add properties of the processor's interaction handlers to the serialization map
    for (size_t i=0; i<interactionHandlers_.size(); ++i) {
        const std::vector<Property*> handlerProps = interactionHandlers_[i]->getProperties();
        for (vector<Property*>::const_iterator it = handlerProps.begin(); it != handlerProps.end(); ++it)
            propertyMap[(*it)->getId()] = *it;
    }

    // serialize properties
    const bool usePointerContentSerialization = s.getUsePointerContentSerialization();
    s.setUsePointerContentSerialization(true);
    s.serialize("Properties", propertyMap, "Property", "name");
    s.setUsePointerContentSerialization(usePointerContentSerialization);
}

void Processor::deserialize(XmlDeserializer& s) {
    // meta data
    metaDataContainer_.deserialize(s);

    // misc settings
    s.deserialize("name", name_);

    // create temporary property map for deserialization
    map<string, Property*> propertyMap;
    for (vector<Property*>::const_iterator it = properties_.begin(); it != properties_.end(); ++it)
        propertyMap[(*it)->getId()] = *it;

    // add properties of the processor's interaction handlers to the deserialization map
    for (size_t i=0; i<interactionHandlers_.size(); ++i) {
        const std::vector<Property*> handlerProps = interactionHandlers_[i]->getProperties();
        for (vector<Property*>::const_iterator it = handlerProps.begin(); it != handlerProps.end(); ++it)
            propertyMap[(*it)->getId()] = *it;
    }

    // deserialize properties
    const bool usePointerContentSerialization = s.getUsePointerContentSerialization();
    s.setUsePointerContentSerialization(true);
    s.deserialize("Properties", propertyMap, "Property", "name");
    s.setUsePointerContentSerialization(usePointerContentSerialization);
}

MetaDataContainer& Processor::getMetaDataContainer() const {
    return metaDataContainer_;
}

ProcessorWidget* Processor::getProcessorWidget() const {
    return processorWidget_;
}

void Processor::addEventProperty(EventProperty* prop) {
    prop->setOwner(this);
    eventProperties_.push_back(prop);
}

void Processor::addInteractionHandler(InteractionHandler* handler) {
    interactionHandlers_.push_back(handler);
    handler->setOwner(this);
}

void Processor::addInteractionHandler(InteractionHandler& handler) {
    addInteractionHandler(&handler);
}
const std::vector<EventProperty*> Processor::getEventProperties() const {
    return eventProperties_;
}

const std::vector<InteractionHandler*>& Processor::getInteractionHandlers() const {
    return interactionHandlers_;
}

void Processor::onEvent(tgt::Event* e) {

    e->ignore();

    for (size_t i=0; i<coProcessorInports_.size(); ++i) {
        coProcessorInports_[i]->distributeEvent(e);
        if (e->isAccepted())
            return;
    }
    // progate to interaction handlers
    for (size_t i=0; i<interactionHandlers_.size() && !e->isAccepted(); ++i)
        interactionHandlers_[i]->onEvent(e);

    // propagate to event properties
    if (typeid(*e) == typeid(tgt::MouseEvent)) {
        tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);

        for (size_t i = 0; i < eventProperties_.size(); ++i) {
            MouseEventProperty* mep = dynamic_cast<MouseEventProperty*>(eventProperties_[i]);

            if (mep) {
                mep->execute(me);
            }
        }

    }
    else if (typeid(*e) == typeid(tgt::KeyEvent)) {
        tgt::KeyEvent* me = static_cast<tgt::KeyEvent*>(e);

        for (size_t i = 0; i < eventProperties_.size(); ++i) {
            KeyboardEventProperty* kep = dynamic_cast<KeyboardEventProperty*>(eventProperties_[i]);

            if (kep) {
                kep->execute(me);
            }
        }
    }
    else {
        tgt::EventListener::onEvent(e);
    }

    if (e->isAccepted())
        return;

    for (size_t i=0; i<inports_.size(); ++i) {
        inports_[i]->distributeEvent(e);
        if (e->isAccepted())
            return;
    }
}

void Processor::deregisterWidget() {
    processorWidget_ = 0;
}

} // namespace voreen

