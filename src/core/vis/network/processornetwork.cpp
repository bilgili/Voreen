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

#include "voreen/core/vis/network/processornetwork.h"

#include "voreen/core/vis/processors/processorfactory.h"
#include "voreen/core/vis/processors/renderprocessor.h"
#include "voreen/core/vis/processors/ports/allports.h"
#include "voreen/core/vis/properties/link/propertylink.h"
#include "voreen/core/vis/properties/link/linkevaluatorfactory.h"
#include "voreen/core/vis/transfunc/transfuncfactory.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"

#include <sstream>

using std::vector;

namespace voreen {

const std::string ProcessorNetwork::loggerCat_("voreen.ProcessorNetwork");

#define NETWORK_VERSION 5

ProcessorNetwork::ProcessorNetwork()
    : version_(NETWORK_VERSION)
{}

ProcessorNetwork::~ProcessorNetwork() {
    for (size_t i = 0; i < propertyLinks_.size(); ++i)
        delete propertyLinks_[i];
    for (size_t i = 0; i < processors_.size(); ++i) {
       processors_[i]->disconnectAllPorts();
    }
    for (size_t i = 0; i < processors_.size(); ++i)
       delete processors_[i];
}

void ProcessorNetwork::addProcessor(Processor* processor, const std::string& name) 
    throw (VoreenException) 
{

    tgtAssert(!processor->getClassName().empty(), "Processor's class name is empty");

    if (std::find(processors_.begin(), processors_.end(), processor) != processors_.end())
        throw VoreenException("Passed processor has already been added to the network: " + processor->getName());

    std::string processorName;
    if (name.empty())
        processorName = processor->getClassName();
    else
        processorName = name;

    if (getProcessorByName(name))
        throw VoreenException("Processor with the passed name already exists in the network: " + processor->getName()); 

    // everything fine => rename and add processor
    processor->setName(processorName);
    processors_.push_back(processor);

    // notify observers
    notifyProcessorAdded(processor);
}

void ProcessorNetwork::removeProcessor(Processor* processor){
    // remove processor if it is contained by the network
    bool processorRemoved = false;
    for (size_t i=0; i<processors_.size(); ++i) {
        if (processor == processors_.at(i)) {
            processors_.erase(processors_.begin() + i);
            processorRemoved = true;
            break;
        }
    }

    if (!processorRemoved) {
        LWARNING("Trying to remove processor that is not part of the network.");
    }
    else {
        // notify observers before any modification of the processor itself
        notifyProcessorRemoved(processor);

        processor->disconnectAllPorts();
        delete processor;
    }
}

const std::vector<Processor*>& ProcessorNetwork::getProcessors() const {
    return processors_;
}

void ProcessorNetwork::setProcessorName(Processor* processor, const std::string& name) const 
    throw (VoreenException) 
{
    if (std::find(processors_.begin(), processors_.end(), processor) == processors_.end())
        throw VoreenException("Passed processor is not part of the network: " + processor->getName());

    if (name.empty())
        throw VoreenException("Passed processor name is empty");

    for (size_t i=0; i<processors_.size(); ++i) {
        if (processors_[i] != processor && processors_[i]->getName() == name)
            throw VoreenException("Passed processor name is not unique in the network");
    }

    // everything fine => rename processor
    std::string prevName = processor->getName();
    processor->setName(name);

    // notify observers
    notifyProcessorRenamed(processor, prevName);
    
}

Processor* ProcessorNetwork::getProcessorByName(const std::string& name) const {
    
    for (size_t i=0; i<processors_.size(); ++i)
        if (processors_[i]->getName() == name)
            return processors_[i];

    return 0;
}

int ProcessorNetwork::getNumProcessors() const {
    return processors_.size();
}

bool ProcessorNetwork::connectPorts(Port* inport, Port* outport) {

    tgtAssert(inport->isInport(), "Inport expected");
    tgtAssert(outport->isOutport(), "Outport expected");

    bool result = outport->connect(inport);
    if (result)
        notifyConnectionsChanged();

    return result;
}

void ProcessorNetwork::disconnectPorts(Port* inport, Port* outport)  {
    tgtAssert(inport->isInport(), "Inport expected");
    tgtAssert(outport->isOutport(), "Outport expected");

    outport->disconnect(inport);
    notifyConnectionsChanged();
}

bool ProcessorNetwork::arePropertiesLinked(const Property* src, const Property* dest) const {

    for (size_t i=0; i<src->getLinks().size(); ++i) {
        if (src->getLinks()[i]->getDestinationProperty() == dest)
            return true;
    }

    return false;
}

PropertyLink* ProcessorNetwork::addPropertyLink(Property* src, Property* dest, LinkEvaluatorBase* linkEvaluator) {

    tgtAssert(src && dest, "Null pointer passed");

    // linking a property with itself is not allowed
    if (src == dest) {
        LWARNING("Trying to link property " << src->getOwner()->getName() << "::" << src->getId() << " with itself");
        return 0;
    }

    // multiple links (with same direction) between two properties are not allowed
    if (arePropertiesLinked(src, dest)) {
        LWARNING(src->getOwner()->getName() << "::" << src->getId() << " and " <<
                dest->getOwner()->getName() << "::" << dest->getId() << " are already linked");
        return 0;
    }

    PropertyLink* link = new PropertyLink(src, dest, linkEvaluator);

    // if link is valid
    if (link->testPropertyLink()) {
        src->registerLink(link);
        propertyLinks_.push_back(link);
        notifyPropertyLinkAdded(link);
        return link;
    }
    else {
        LWARNING(src->getOwner()->getName() << "::" << src->getId() << " is not linkable to " <<
                dest->getOwner()->getName() << "::" << dest->getId());
        delete link;
        return 0;
    }

}

void ProcessorNetwork::removePropertyLink(const PropertyLink* propertyLink) {

    tgtAssert(propertyLink, "Null pointer passed");

    // remove property link if it is contained by the network
    bool propertyLinkRemoved = false;
    for (size_t i=0; i<propertyLinks_.size(); ++i) {
        if (propertyLink == propertyLinks_.at(i)) {
            propertyLinks_.erase(propertyLinks_.begin() + i);
            propertyLinkRemoved = true;
            break;
        }
    }

    // notify observers and then delete the link
    if (propertyLinkRemoved) {
        notifyPropertyLinkRemoved(propertyLink);
        delete propertyLink;
    }

}

const vector<PropertyLink*>& ProcessorNetwork::getPropertyLinks() const {
    return propertyLinks_;
}

void ProcessorNetwork::setVersion(int version) {
    version_ = version;
}

int ProcessorNetwork::getVersion() const {
    return version_;
}

ProcessorNetwork::PortConnection::PortEntry::PortEntry(Port* port)
    : port_(port)
{
}

void ProcessorNetwork::PortConnection::PortEntry::serialize(XmlSerializer& s) const {
    s.serialize("name", port_->getName());
    s.serialize("Processor", port_->getProcessor());
}

void ProcessorNetwork::PortConnection::PortEntry::deserialize(XmlDeserializer& s) {
    std::string name;
    Processor* processor;

    s.deserialize("name", name);
    s.deserialize("Processor", processor);

    // Was processor not deserialized?
    if (!processor)
        // Cancel port search, since we are not able to proceed...
        return;

    std::vector<Port*> ports = processor->getPorts();
    for (size_t i=0; i<ports.size(); ++i) {
        if (ports[i]->getName() == name) {
            port_ = ports[i];
            break;
        }
    }
}

Port* ProcessorNetwork::PortConnection::PortEntry::getPort() const {
    return port_;
}

ProcessorNetwork::PortConnection::PortConnection(Port* outport, Port* inport)
    : outport_(outport)
    , inport_(inport)
{
}

ProcessorNetwork::PortConnection::PortConnection()
    : outport_(0)
    , inport_(0)
{
}
void ProcessorNetwork::PortConnection::serialize(XmlSerializer& s) const {
    s.serialize("Outport", outport_);
    s.serialize("Inport", inport_);
}

void ProcessorNetwork::PortConnection::deserialize(XmlDeserializer& s) {
    s.deserialize("Outport", outport_);
    s.deserialize("Inport", inport_);
}

void ProcessorNetwork::PortConnection::setOutport(Port* value) {
    outport_ = PortEntry(value);
}

Port* ProcessorNetwork::PortConnection::getOutport() const {
    return outport_.getPort();
}

void ProcessorNetwork::PortConnection::setInport(Port* value) {
    inport_ = PortEntry(value);
}

Port* ProcessorNetwork::PortConnection::getInport() const {
    return inport_.getPort();
}

void ProcessorNetwork::serialize(XmlSerializer& s) const {
    const_cast<ProcessorNetwork*>(this)->errorList_.clear();

    s.registerFactory(ProcessorFactory::getInstance());
    s.registerFactory(TransFuncFactory::getInstance());
    s.registerFactory(LinkEvaluatorFactory::getInstance());

    // misc
    s.serialize("version", version_);

    // general Settings for the Network
    s.serialize("reuseTCTargets", reuseTCTargets_);

    // Serialize Processors and add them to the network element
    s.serialize("Processors", processors_, "Processor");

    // Serialize the port connections...
    PortConnectionCollection portConnections;
    PortConnectionCollection coProcessorPortConnections;

    typedef vector<Processor*> Processors;
    typedef vector<Port*> Ports;
    typedef vector<CoProcessorPort*> CoProcessorPorts;

    for (Processors::const_iterator processorIt = processors_.begin(); processorIt != processors_.end(); ++processorIt) {
        Ports ports = (*processorIt)->getOutports();
        for (Ports::const_iterator portIt = ports.begin(); portIt != ports.end(); ++portIt) {
            Ports connectedPorts = (*portIt)->getConnected();
            for (Ports::const_iterator connectedPortIt = connectedPorts.begin(); connectedPortIt != connectedPorts.end(); ++connectedPortIt)
                portConnections.push_back(PortConnection(*portIt, *connectedPortIt));
        }

        CoProcessorPorts coProcessorPorts = (*processorIt)->getCoProcessorOutports();
        for (CoProcessorPorts::const_iterator portIt = coProcessorPorts.begin(); portIt != coProcessorPorts.end(); ++portIt) {
            Ports connectedPorts = (*portIt)->getConnected();
            for (Ports::const_iterator connectedPortIt = connectedPorts.begin(); connectedPortIt != connectedPorts.end(); ++connectedPortIt)
                coProcessorPortConnections.push_back(PortConnection(*portIt, *connectedPortIt));
        }
    }

    s.serialize("Connections", portConnections, "Connection");
    s.serialize("CoProcessorConnections", coProcessorPortConnections, "CoProcessorConnection");

    // Serialize PropertyLinks and add them to the network element
    s.serialize("PropertyLinks", propertyLinks_, "PropertyLink");
}

void ProcessorNetwork::deserialize(XmlDeserializer& s) {
    errorList_.clear();

    s.registerFactory(ProcessorFactory::getInstance());
    s.registerFactory(TransFuncFactory::getInstance());
    s.registerFactory(LinkEvaluatorFactory::getInstance());

    // read general settings
    s.deserialize("reuseTCTargets", reuseTCTargets_);

    // deserialize Processors
    s.deserialize("Processors", processors_, "Processor");

    // deserialize port connections...
    PortConnectionCollection portConnections;
    PortConnectionCollection coProcessorPortConnections;

    s.deserialize("Connections", portConnections, "Connection");
    s.deserialize("CoProcessorConnections", coProcessorPortConnections, "CoProcessorConnection");

    for (PortConnectionCollection::iterator it = portConnections.begin(); it != portConnections.end(); ++it) {
        if (it->getOutport() && it->getInport())
            it->getOutport()->connect(it->getInport());
        else
            s.addError(SerializationException("Port connection could not be established."));
    }

    for (PortConnectionCollection::iterator it = coProcessorPortConnections.begin(); it != coProcessorPortConnections.end(); ++it) {
        if (it->getOutport() && it->getInport())
            it->getOutport()->connect(it->getInport());
        else
            s.addError(SerializationException("Coprocessor port connection could not be established."));
    }

    // deserialize PropertyLinks
    s.deserialize("PropertyLinks", propertyLinks_, "PropertyLink");

    // check for duplicate processor names 
    vector<vector<Processor*> > duplicates;
    for (size_t i=0; i<processors_.size(); ++i) {
        Processor* processor = processors_[i];
        bool isDuplicate = false;
        for (size_t j=0; j<processors_.size() && !isDuplicate; ++j) {
            if ((i != j) && (processor->getName() == processors_[j]->getName()))
                isDuplicate = true;
        }
        if (isDuplicate) {
            // insert into duplicates vector
            bool newNameGroup = true;
            for (size_t j=0; j<duplicates.size(); ++j) {
                if (duplicates[j][0]->getName() == processor->getName()) {
                    duplicates[j].push_back(processor);
                    newNameGroup = false;
                    break;
                }
            }
            if (newNameGroup) {
                duplicates.push_back(vector<Processor*>());
                duplicates.back().push_back(processor);
            }
        }
    }
    // correct duplicate processor names
    for (size_t i=0; i<duplicates.size(); ++i) {
        LWARNING("Duplicate processor name: \"" << duplicates[i][0]->getName() << "\" (" << duplicates[i].size() << " occurrences)");
        for (size_t j=1; j<duplicates[i].size(); ++j) {
            std::ostringstream stream;
            stream << duplicates[i][j]->getName() << " " << j+1;
            std::string newName = stream.str();
            LINFO("Renaming \"" << duplicates[i][j]->getName() << "\" to \"" << newName << "\"");
            duplicates[i][j]->setName(newName);
        }
    }

    notifyNetworkChanged();
}

void ProcessorNetwork::notifyNetworkChanged() const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->networkChanged();
}

void ProcessorNetwork::notifyProcessorAdded(const Processor* processor) const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->processorAdded(processor);
}


void ProcessorNetwork::notifyProcessorRemoved(const Processor* processor) const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->processorRemoved(processor);
}

void ProcessorNetwork::notifyProcessorRenamed(const Processor* processor, const std::string& prevName) const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->processorRenamed(processor, prevName);
}

void ProcessorNetwork::notifyConnectionsChanged() const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->connectionsChanged();
}

void ProcessorNetwork::notifyPropertyLinkAdded(const PropertyLink* link) const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->propertyLinkAdded(link);
}

void ProcessorNetwork::notifyPropertyLinkRemoved(const PropertyLink* link) const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->propertyLinkRemoved(link);
}

void ProcessorNetwork::setReuseTargets(bool reuse) {
    reuseTCTargets_ = reuse;
}

bool ProcessorNetwork::getReuseTargets() const {
    return reuseTCTargets_;
}

void ProcessorNetwork::setErrors(const ErrorListType& errorList) {
    errorList_ = errorList;
}

ErrorListType ProcessorNetwork::getErrors() const {
    return errorList_;
}

// -----------------------------------------------------------------------------------

void ProcessorNetworkObserver::processorAdded(const Processor* /*processor*/) {
    networkChanged();
}

void ProcessorNetworkObserver::processorRemoved(const Processor* /*processor*/) {
    networkChanged();
}

void ProcessorNetworkObserver::processorRenamed(const Processor* /*processor*/, const std::string& /*prevName*/) {
    networkChanged();
}

void ProcessorNetworkObserver::connectionsChanged() {
    networkChanged();
}

void ProcessorNetworkObserver::propertyLinkAdded(const PropertyLink* /*link*/) {
    networkChanged();
}

void ProcessorNetworkObserver::propertyLinkRemoved(const PropertyLink* /*link*/) {
    networkChanged();
}

} // namespace voreen

