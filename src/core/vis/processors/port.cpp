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

Port::Port(Identifier type, Processor* processor, bool isOutport, bool allowMultipleConnections, bool isPersistent)
    : type_(type)
    , processor_(processor)
    , isOutport_(isOutport)
    , allowMultipleConnections_(allowMultipleConnections)
    , isPersistent_(isPersistent)
{
    if (isOutport_)
        allowMultipleConnections_ = true;
}

Identifier Port::getType() const {
    return type_;
}

std::vector<Port*>& Port::getConnected() {
    return connectedPorts_;
}

void Port::setType(Identifier newType) {
    type_ = newType;
}

void Port::addConnection(Port* port) {
    connectedPorts_.push_back(port);
}

bool Port::isConnectedTo(Port* port) const {
    for (size_t i = 0; i < connectedPorts_.size(); ++i) {
        if (connectedPorts_[i] == port)
            return true;
    }
    return false;
}

int Port::getIndexOf(Port* port) const {
    for (size_t i = 0; i < connectedPorts_.size(); ++i) {
        if (connectedPorts_[i] == port)
            return i;
    }
    return -1;
}

//TODO: Is this correct?
bool Port::isCompatible(Identifier type) {
    if (type_.getSubString(0) == type.getSubString(0))
        return true;
    else
        return false;
}

std::string Port::printConnections() {
    std::stringstream connections;
    std::vector<Port*>& connectedTo = getConnected();
    for (size_t i=0; i<connectedTo.size(); ++i)
        connections << i << ": " << connectedTo[i]->getProcessor()->getClassName().getName() << std::endl;

    return connections.str();
}

} // namespace voreen
