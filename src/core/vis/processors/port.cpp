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

#include "voreen/core/vis/processors/port.h"
#include "voreen/core/vis/processors/processor.h"

#include <sstream>


namespace voreen {

PortData::~PortData() {}

PortData::PortData(Identifier type)
    : type_(type)
{}

PortDataCoProcessor::PortDataCoProcessor(Processor* processor, FunctionPointer functionPointer)
    : PortDataGeneric<FunctionPointer>(functionPointer, "portdata.coprocessor")
    , processor_(processor)
{}

Message* PortDataCoProcessor::call(Identifier ident, LocalPortMapping* portMapping) {
    return (processor_->*getData())(ident, portMapping);
}

//------------------------------------------------------------------------------

Port::Port(Identifier type, Processor* const processor, bool isOutport, bool allowMultipleConnections, 
           bool isPersistent, const bool isPrivate)
    : typeID_(type),
    portType_(getPortType(type)),
    connectedPorts_(),
    processor_(processor),
    isOutport_(isOutport),
    isCoProcessorPort_(false),
    allowMultipleConnections_(allowMultipleConnections),
    isPrivatePort_(isPrivate),
    isPersistent_(isPersistent),
    function_(0)
{
    if (isOutport_)
        allowMultipleConnections_ = true;
    isCoProcessorPort_ = (portType_ == PORT_TYPE_COPROCESSOR);
}

Port::~Port() {
    for (size_t i = 0; i < connectedPorts_.size(); ++i)
        connectedPorts_[i]->disconnect(this, false);
}

// ----------------------------------------------------------------------------

bool Port::connect(Port* const inport) {
    if (testConnectivity(inport) == true) {
        connectedPorts_.push_back(inport);
        inport->connectedPorts_.push_back(this);
        return true;
    }
    return false;
}

bool Port::disconnect(Port* const other, const bool mutualDisconnect) {
    if ((other == 0) || (other == this))
        return false;

    bool found = false;
    for (size_t i = 0; i < connectedPorts_.size(); ++i) {
        if (connectedPorts_[i] == other) {
            found = true;
            connectedPorts_.erase(connectedPorts_.begin() + i);
        }
    }

    if ((mutualDisconnect == true) && (found == true))
        return other->disconnect(this, false);

    return found;
}

bool Port::testConnectivity(Port* const inport) const {
    if ((inport == 0) || (inport == this))
        return false;

    // This port must be an outport, the inport must be not an outport and
    // they must not be already connected.
    //
    if ((isOutport_ == false) || (inport->isOutport() == true) || (isConnectedTo(inport) == true))
        return false;

    // If the inport must allow multiple connections or may not be already connected to any
    // other port.
    //
    if ((inport->allowMultipleConnections() == false) && (inport->isConnected() == true))
        return false;

    return true;
}

// ----------------------------------------------------------------------------

void Port::addConnection(Port* port) {
    if (port != 0)
        connectedPorts_.push_back(port);
}

std::vector<Port*>& Port::getConnected() {
    return connectedPorts_;
}

int Port::getIndexOf(Port* port) const {
    for (size_t i = 0; i < connectedPorts_.size(); ++i) {
        if (connectedPorts_[i] == port)
            return i;
    }
    return -1;
}

Port::PortType Port::getPortType(const Identifier& identifier) {
    const PortTypeMap& types = getPortTypes();
    PortTypeMap::const_iterator it = types.find(identifier.getSubString(0));
    if (it == types.end())
        return PORT_TYPE_UNSPECIFIED;
    return (it->second);
}

const Port::PortType& Port::getType() const {
    return portType_;
}

Identifier Port::getTypeIdentifier() const {
    return typeID_;
}

bool Port::isCompatible(Identifier type) const {
    if (typeID_.getSubString(0) == type.getSubString(0))
        return true;
    else
        return false;
}

bool Port::isConnected() const {
    return (connectedPorts_.size() > 0);
}

bool Port::isConnectedTo(Port* port) const {
    for (size_t i = 0; i < connectedPorts_.size(); ++i) {
        if (connectedPorts_[i] == port)
            return true;
    }
    return false;
}

bool Port::isCoProcessorPort() const {
    return isCoProcessorPort_;
}

std::string Port::printConnections() {
    std::stringstream connections;
    std::vector<Port*>& connectedTo = getConnected();
    for (size_t i = 0; i < connectedTo.size(); ++i)
        connections << i << ": " << connectedTo[i]->getProcessor()->getClassName().getName() << std::endl;

    return connections.str();
}

void Port::setType(Identifier newType) {
    typeID_ = newType;
    portType_ = getPortType(typeID_);
}

// private methods
//

Port::PortTypeMap& Port::getPortTypes() {
    static PortTypeMap typeMap;
    if (typeMap.empty() == true) {
        typeMap.insert(std::make_pair("volumeset", PORT_TYPE_VOLUMESET));
        typeMap.insert(std::make_pair("volumehandle", PORT_TYPE_VOLUMEHANDLE));
        typeMap.insert(std::make_pair("image", PORT_TYPE_IMAGE));
        typeMap.insert(std::make_pair("geometry", PORT_TYPE_GEOMETRY));
        typeMap.insert(std::make_pair("coprocessor", PORT_TYPE_COPROCESSOR));
    }
    return typeMap;
}

} // namespace voreen
