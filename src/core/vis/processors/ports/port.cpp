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

#include "voreen/core/vis/processors/ports/port.h"
#include "voreen/core/vis/processors/processor.h"

#include <sstream>


namespace voreen {

Port::Port(const std::string& name, PortDirection direction, bool allowMultipleConnections, Processor::InvalidationLevel invalidationLevel)
    : name_(name),
      connectedPorts_(),
      processor_(0),
      direction_(direction),
      allowMultipleConnections_(allowMultipleConnections),
      hasChanged_(false),
      invalidationLevel_(invalidationLevel)
{
    if (isOutport())
        allowMultipleConnections_ = true;
}

Port::~Port() {
    disconnectAll();
}

void Port::setProcessor(Processor* p) {
    tgtAssert(processor_ == 0, "Processor already set!");
    processor_ = p;
}

bool Port::connect(Port* inport) {
    if (testConnectivity(inport) == true) {
        connectedPorts_.push_back(inport);
        inport->connectedPorts_.push_back(this);
        getProcessor()->invalidate(invalidationLevel_);
        inport->invalidate();
        return true;
    }
    return false;
}

void Port::disconnect(Port* other) {
    tgtAssert(((other != 0) && (other != this)), "error in Port::disconnect: other == 0 or other == this");

    for (size_t i = 0; i < connectedPorts_.size(); ++i) {
        if (connectedPorts_[i] == other) {
            connectedPorts_.erase(connectedPorts_.begin() + i);
            other->disconnect(this);
            getProcessor()->invalidate(invalidationLevel_);

            invalidate();
            return;
        }
    }
}

void Port::disconnectAll() {
    for (size_t i=0; i < connectedPorts_.size(); ++i) {
        connectedPorts_[i]->disconnect(this);
    }

    connectedPorts_.clear();
}

bool Port::testConnectivity(const Port* inport) const {
    if ((inport == 0) || (inport == this))
        return false;

    // This port must be an outport, the inport must be not an outport and they
    // must not be already connected.
    if ((isInport()) || (inport->isOutport()) ||
            (isConnectedTo(inport) == true))
        return false;

    // If the inport must allow multiple connections or may not be already
    // connected to any other port.
    if ((inport->allowMultipleConnections() == false) && (inport->isConnected() == true))
        return false;

    // The ports must not belong to the same processor:
    if ( getProcessor() == inport->getProcessor() )
        return false;

    // The ports must be of the same type:
    if ( typeid(*this) != typeid(*inport))
        return false;

    return true;
}

size_t Port::getNumConnections() const {
    return connectedPorts_.size();
}

// ----------------------------------------------------------------------------

const std::vector<Port*>& Port::getConnected() const {
       return connectedPorts_;
}

int Port::getIndexOf(const Port* port) const {
    for (size_t i = 0; i < connectedPorts_.size(); ++i) {
        if (connectedPorts_[i] == port)
           return i;
    }
    return -1;
}

bool Port::isConnected() const {
    return (connectedPorts_.size() > 0);
}

bool Port::isConnectedTo(const Port* port) const {
    for (size_t i = 0; i < connectedPorts_.size(); ++i) {
        if (connectedPorts_[i] == port)
            return true;
    }
    return false;
}

void Port::invalidate() {
    if (isOutport()) {
        for (size_t i = 0; i <  connectedPorts_.size(); ++i)
             connectedPorts_[i]->invalidate();
    }
    else {
        getProcessor()->invalidate(invalidationLevel_);
           hasChanged_ = true;
    }
}

bool Port::hasChanged() const {
       if (isOutport()) {
        LWARNINGC("voreen.port", "Called hasChanged() on outport!");
       }
       return hasChanged_;
}

void Port::setValid() {
       hasChanged_ = false;

    if (isOutport()) {
           LWARNINGC("voreen.port", "Called setValid() on outport!" << getName() );
    }
}

void Port::distributeEvent(tgt::Event* e) {
    if (isOutport()) {
        getProcessor()->onEvent(e);
    } else {
        for (size_t i = 0; i < connectedPorts_.size(); ++i) {
            connectedPorts_[i]->distributeEvent(e);
            if (e->isAccepted())
                return;
        }
    }
}

void Port::toggleInteractionMode(bool interactionMode, void* source) {
    if (isOutport()) {
        for (size_t i = 0; i < connectedPorts_.size(); ++i)
            connectedPorts_[i]->toggleInteractionMode(interactionMode, source);
    } else {
        getProcessor()->toggleInteractionMode(interactionMode, source);
    }
}

//void Port::serialize(XmlSerializer& s) const {
//    s.serialize("port", name_);
//    s.serialize("Processor", processor_);
//    s.serialize("ConnectedPorts", connectedPorts_, "Port");
//
//    s.serialize("isOutport", isOutport_);
//    s.serialize("allowMultipleConnections", allowMultipleConnections_);
//    s.serialize("invalidationLevel", invalidationLevel_);
//}
//
//void Port::deserialize(XmlDeserializer& s) {
//    s.deserialize("port", name_);
//    s.deserialize("Processor", processor_);
//    s.deserialize("ConnectedPorts", connectedPorts_, "Port");
//
//    bool isOutport = false;
//    int invalidationLevel = 0;
//
//    s.deserialize("isOutport", isOutport);
//    s.deserialize("allowMultipleConnections", allowMultipleConnections_);
//    s.deserialize("invalidationLevel", invalidationLevel);
//
//    *const_cast<bool*>(&isOutport_) = isOutport;
//    invalidationLevel_ = static_cast<Processor::InvalidationLevel>(invalidationLevel);
//}

} // namespace voreen
