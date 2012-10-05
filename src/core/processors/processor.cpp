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

#include "voreen/core/processors/processor.h"
#include "voreen/core/properties/property.h"

#include "tgt/glmath.h"
#include "tgt/shadermanager.h"
#include "tgt/gpucapabilities.h"
#include "tgt/textureunit.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/voreenmodule.h"
#include "voreen/core/processors/processorwidgetfactory.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/processors/processorwidget.h"
#include "voreen/core/properties/eventproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/interaction/interactionhandler.h"

#include "voreen/core/io/serialization/xmlserializer.h"
#include "voreen/core/io/serialization/xmldeserializer.h"
#include "voreen/core/io/progressbar.h"

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
    //setDescriptions(); // not allowed
}

Processor::~Processor() {
    if (isInitialized()) {
        LERROR("~Processor(): '" << getName() << "' has not been deinitialized");
        return;
    }
}

Processor* Processor::clone() const {
    try {
        std::stringstream stream;

        // first serialize
        XmlSerializer s;
        s.serialize("this", this);
        s.write(stream);

        // then deserialize again
        XmlDeserializer d;
        d.read(stream);
        Processor* proc = 0;
        d.deserialize("this", proc);

        proc->setDescriptions();
        return proc;
    }
    catch (std::exception& e) {
        LERROR("Failed to clone processor '" << getName() << "': " << e.what());
        return 0;
    }
}


void Processor::initialize() throw (tgt::Exception) {
    if(description_ == "")
        setDescriptions();

    if (!VoreenApplication::app()) {
        LERROR("VoreenApplication not instantiated");
        throw new VoreenException("VoreenApplication not instantiated");
    }

    if (isInitialized()) {
        LWARNING("initialize(): '" << getName() << "' already initialized");
        return;
    }

    bool glMode = VoreenApplication::app() && VoreenApplication::app()->isInitializedGL();
    if (!glMode)
        LDEBUG("initialize() not in OpenGL mode");

    // create and initialize processor widget
    processorWidget_ = VoreenApplication::app()->createProcessorWidget(this);
    if (processorWidget_) {
        processorWidget_->initialize();
        if (glMode)
            LGL_ERROR;
        // inform the observers about the new widget
        std::vector<ProcessorObserver*> observers = Observable<ProcessorObserver>::getObservers();
        for (size_t i = 0; i < observers.size(); ++i)
            observers[i]->processorWidgetCreated(this);
    }


    // initialize ports
    const std::vector<Port*>& ports = getPorts();
    for (size_t i=0; i < ports.size(); ++i) {
        if (!ports[i]->isInitialized())
            ports[i]->initialize();
        if (glMode)
            LGL_ERROR;
    }

    // initialize properties
    const std::vector<Property*>& properties = getProperties();
    for (size_t i=0; i < properties.size(); ++i) {
        properties[i]->initialize();
        if (glMode)
            LGL_ERROR;
    }
}

void Processor::deinitialize() throw (tgt::Exception) {
    if (!isInitialized()) {
        LWARNING("deinitialize(): '" << getName() << "' (" << getClassName() << ") not initialized");
        return;
    }

    bool glMode = VoreenApplication::app() && VoreenApplication::app()->isInitializedGL();
    if (!glMode)
        LDEBUG("deinitialize() not in OpenGL mode");

    if (glMode)
        LGL_ERROR;

    // deinitialize ports
    const std::vector<Port*>& ports = getPorts();
    for (size_t i=0; i < ports.size(); ++i)
        ports[i]->deinitialize();
    if (glMode)
        LGL_ERROR;

    // deinitialize properties
    const std::vector<Property*>& properties = getProperties();
    for (size_t i=0; i < properties.size(); ++i)
        properties[i]->deinitialize();
    if (glMode)
        LGL_ERROR;

    // delete processor widget
    delete processorWidget_;
    processorWidget_ = 0;

    // notify observers about the deleted widget
    std::vector<ProcessorObserver*> observers = Observable<ProcessorObserver>::getObservers();
    for (size_t i = 0; i < observers.size(); ++i)
        observers[i]->processorWidgetDeleted(this);
}

void Processor::beforeProcess() {
}

void Processor::afterProcess() {
}

void Processor::clearOutports() {
    const std::vector<Port*>& outports = getOutports();
    for(size_t i=0; i<outports.size(); i++)
        outports[i]->clear();
}

void Processor::lockMutex() {
    mutex_.lock();
}

void Processor::unlockMutex() {
    mutex_.unlock();
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

    map<std::string, Port*>::const_iterator it = portMap_.find(port->getID());
    if (it == portMap_.end())
        portMap_.insert(std::make_pair(port->getID(), port));
    else {
        LERROR("Port with name " << port->getID() << " has already been inserted!");
        tgtAssert(false, std::string("Port with name " + port->getID() + " has already been inserted").c_str());
    }
}

void Processor::addPort(Port& port) {
    addPort(&port);
}

void Processor::removePort(Port* port) {
    tgtAssert(port, "Null pointer passed");

    if (port->isInitialized()) {
        LWARNING("removePort() Port '" << getName() << "." << port->getID() << "' "
            << "has not been deinitialized");
    }

    //port->setProcessor(NULL);
    CoProcessorPort* cpp = dynamic_cast<CoProcessorPort*>(port);
    if (port->isOutport()) {
        if (cpp)
            coProcessorOutports_.erase(std::find(coProcessorOutports_.begin(), coProcessorOutports_.end(), port));
        else
            outports_.erase(std::find(outports_.begin(), outports_.end(), port));
    }
    else {
        if (cpp)
            coProcessorInports_.erase(std::find(coProcessorInports_.begin(), coProcessorInports_.end(), port));
        else
            inports_.erase(std::find(inports_.begin(), inports_.end(), port));
    }

    map<std::string, Port*>::iterator it = portMap_.find(port->getID());
    if (it != portMap_.end())
        portMap_.erase(it);
    else {
        LERROR("Port with name " << port->getID() << " was not found in port map!");
        tgtAssert(false, std::string("Port with name " + port->getID() + " was not found in port map!").c_str());
    }
}

void Processor::removePort(Port& port) {
    removePort(&port);
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

std::string Processor::getModulePath() const {
    return VoreenApplication::app()->getModulePath(getModuleName());
}

bool Processor::isUtility() const {
    return false;
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

const PerformanceRecord* Processor::getPerformanceRecord() const {
    return &performanceRecord_;
}

void Processor::resetPerformanceRecord() {
    performanceRecord_.deleteSamples();
}

void Processor::invalidate(int inv) {
    PropertyOwner::invalidate(inv);

    if (inv == Processor::VALID)
        return;

    if (!isInitialized())
        return;

    if (!invalidationVisited_) {
        invalidationVisited_ = true;

        for (size_t i=0; i<coProcessorOutports_.size(); ++i)
            coProcessorOutports_[i]->invalidatePort();

        invalidationVisited_ = false;

        tgtAssert(VoreenApplication::app(), "VoreenApplication not instantiated");
        VoreenApplication::app()->scheduleNetworkProcessing();
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


    // ---
    // the following entities are static resources (i.e. already existing at this point) 
    // that should therefore not be dynamically created by the serializer 
    //
    const bool usePointerContentSerialization = s.getUsePointerContentSerialization();
    s.setUsePointerContentSerialization(true);

    // serialize inports using a temporary map
    std::map<std::string, Port*> inportMap;
    for (std::vector<Port*>::const_iterator it = inports_.begin(); it != inports_.end(); ++it)
        inportMap[(*it)->getID()] = *it;
    try {
        s.serialize("Inports", inportMap, "Port", "name");
    }
    catch (SerializationException& e) {
        LWARNING(e.what());
    }

    // serialize outports using a temporary map
    std::map<std::string, Port*> outportMap;
    for (std::vector<Port*>::const_iterator it = outports_.begin(); it != outports_.end(); ++it)
        outportMap[(*it)->getID()] = *it;
    try {
        s.serialize("Outports", outportMap, "Port", "name");
    }
    catch (SerializationException& e) {
        LWARNING(e.what());
    }

    // serialize interaction handlers using a temporary map
    map<string, InteractionHandler*> handlerMap;
    const std::vector<InteractionHandler*>& handlers = getInteractionHandlers();
    for (vector<InteractionHandler*>::const_iterator it = handlers.begin(); it != handlers.end(); ++it)
        handlerMap[(*it)->getID()] = *it;
    try {
        s.serialize("InteractionHandlers", handlerMap, "Handler", "name");
    }
    catch (SerializationException& e) {
        LWARNING(e.what());
    }

    s.setUsePointerContentSerialization(usePointerContentSerialization);
    // --- static resources end ---

}

void Processor::deserialize(XmlDeserializer& s) {
    // meta data
    metaDataContainer_.deserialize(s);

    // misc settings
    s.deserialize("name", name_);

    // deserialize properties
    PropertyOwner::deserialize(s);


    // ---
    // the following entities are static resources that should not be dynamically created by the serializer 
    //
    const bool usePointerContentSerialization = s.getUsePointerContentSerialization();
    s.setUsePointerContentSerialization(true);

    // deserialize inports using a temporary map
    map<string, Port*> inportMap;
    for (vector<Port*>::const_iterator it = inports_.begin(); it != inports_.end(); ++it)
        inportMap[(*it)->getName()] = *it;
    try {
        s.deserialize("Inports", inportMap, "Port", "name");
    }
    catch (XmlSerializationNoSuchDataException& /*e*/){
        // port key missing => just ignore
        s.removeLastError();
    }

    // deserialize outports using a temporary map
    map<string, Port*> outportMap;
    for (vector<Port*>::const_iterator it = outports_.begin(); it != outports_.end(); ++it)
        outportMap[(*it)->getName()] = *it;
    try {
        s.deserialize("Outports", outportMap, "Port", "name");
    }
    catch (XmlSerializationNoSuchDataException& /*e*/){
        // port key missing => just ignore
        s.removeLastError();
    }

    // deserialize interaction handlers using a temporary map
    map<string, InteractionHandler*> handlerMap;
    const std::vector<InteractionHandler*>& handlers = getInteractionHandlers();
    for (vector<InteractionHandler*>::const_iterator it = handlers.begin(); it != handlers.end(); ++it)
        handlerMap[(*it)->getID()] = *it;
    try {
        s.deserialize("InteractionHandlers", handlerMap, "Handler", "name");
    }
    catch (XmlSerializationNoSuchDataException& /*e*/){
        // interaction handler key missing => just ignore
        s.removeLastError();
    }

    s.setUsePointerContentSerialization(usePointerContentSerialization);
    // --- static resources end ---

}

MetaDataContainer& Processor::getMetaDataContainer() const {
    return metaDataContainer_;
}

std::string Processor::getCachePath() const {
    return VoreenApplication::app()->getCachePath() + "/" + getClassName();
}

std::string Processor::getDescription() const {
    if(description_ == "")
        const_cast<Processor*>(this)->setDescriptions(); //make setDescriptions const?

    return description_;
}

void Processor::setDescription(std::string desc) {
    description_ = desc;
}

std::string Processor::getPropertyDescription(const std::string& propId) const {
    if(description_ == "")
        const_cast<Processor*>(this)->setDescriptions(); //make setDescriptions const?

    const Property* p = getProperty(propId);
    if(p)
        return p->getDescription();
    else {
        LWARNING("No such property: " << propId);
        return "";
    }
}

std::string Processor::getPortDescription(const std::string& portId) const {
    if(description_ == "")
        const_cast<Processor*>(this)->setDescriptions(); //make setDescriptions const?

    const Port* p = getPort(portId);
    if(p)
        return p->getDescription();
    else {
        LWARNING("No such port: " << portId);
        return "";
    }
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

    //if (e->isAccepted())
        //return;

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

void Processor::initializePort(Port* port) throw (tgt::Exception) {
    tgtAssert(port, "Null pointer passed");
    if (port->isInitialized()) {
        LWARNING("initializePort() port '" << getName() << "." << port->getID()
            << "' already initialized");
        return;
    }

    port->initialize();
}

void Processor::deinitializePort(Port* port) throw (tgt::Exception) {
    tgtAssert(port, "Null pointer passed");
    if (!port->isInitialized()) {
        LWARNING("deinitializePort() port '" << getName() << "." << port->getID()
            << "' not initialized");
        return;
    }

    port->deinitialize();
}

void Processor::notifyPortsChanged() const {
    std::vector<ProcessorObserver*> procObservers = Observable<ProcessorObserver>::getObservers();
    for (size_t i = 0; i < procObservers.size(); ++i)
        procObservers[i]->portsChanged(this);
}

bool Processor::usesExpensiveComputation() const {
    return false;
}

} // namespace voreen
