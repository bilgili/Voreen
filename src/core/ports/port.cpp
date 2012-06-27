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

#include "voreen/core/ports/port.h"
#include "voreen/core/processors/processor.h"

#include <sstream>

namespace voreen {

const std::string Port::loggerCat_("voreen.Port");

Port::Port(const std::string& name, PortDirection direction, bool allowMultipleConnections, Processor::InvalidationLevel invalidationLevel)
    : name_(name)
    , connectedPorts_()
    , processor_(0)
    , direction_(direction)
    , allowMultipleConnections_(allowMultipleConnections)
    , hasChanged_(false)
    , invalidationLevel_(invalidationLevel)
    , isLoopPort_(false)
    , numLoopIterations_(1)
    , currentLoopIteration_(0)
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
    if (testConnectivity(inport)) {
        connectedPorts_.push_back(inport);
        inport->connectedPorts_.push_back(this);
        getProcessor()->invalidate(invalidationLevel_);
        inport->invalidate();
        return true;
    }
    return false;
}

void Port::disconnect(Port* other) {
    tgtAssert(other, "passed null pointer");
    tgtAssert(other != this, "tried to disconnect port from itself");

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

bool Port::isReady() const {
    return isConnected();
}

bool Port::testConnectivity(const Port* inport) const {
    if ((inport == 0) || (inport == this))
        return false;

    // This port must be an outport, the inport must be not an outport and they
    // must not be already connected.
    if ((isInport()) || (inport->isOutport()) || isConnectedTo(inport))
        return false;

    // If the inport must allow multiple connections or may not be already
    // connected to any other port.
    if ((inport->allowMultipleConnections() == false) && inport->isConnected())
        return false;

    // The ports must not belong to the same processor:
    if (getProcessor() == inport->getProcessor())
        return false;

    // The ports must be of the same type:
    if (typeid(*this) != typeid(*inport))
        return false;

    // Only loop ports are allowed to be connected to loop ports
    if (this->isLoopPort() != inport->isLoopPort())
        return false;

    return true;
}

size_t Port::getNumConnections() const {
    return connectedPorts_.size();
}

void Port::setLoopPort(bool isLoopPort) {
    isLoopPort_ = isLoopPort;
}

bool Port::isLoopPort() const {
    return isLoopPort_;
}

int Port::getNumLoopIterations() const {

    if (!isLoopPort()) {
        LWARNING("getNumLoopIterations() called on non loop-port");
        return 0;
    }

    if (isInport())
        return std::max(numLoopIterations_, 1);
    else if (isOutport() && isConnected()) {
        tgtAssert(!getConnected().empty(), "Connected ports vector is empty");
        return getConnected().front()->getNumLoopIterations();
    }
    else {
        return 1;
    }
}

void Port::setNumLoopIterations(int iterations) {

    if (!isLoopPort()) {
        LWARNING("setNumLoopIterations() called on non loop-port");
        return;
    }

    if (numLoopIterations_ != iterations) {
        numLoopIterations_ = iterations;
        if (getProcessor())
            getProcessor()->invalidate(Processor::INVALID_PORTS);
    }
}

int Port::getLoopIteration() const {

    if (!isLoopPort()) {
        LWARNING("getLoopIteration() called on non loop-port");
        return 0;
    }

    if (isInport())
        return currentLoopIteration_;
    else if (isOutport() && isConnected()) {
        tgtAssert(!getConnected().empty(), "Connected ports vector is empty");
        return getConnected().front()->getLoopIteration();
    }
    else {
        return 0;
    }
}

const std::vector<Port*>& Port::getConnected() const {
       return connectedPorts_;
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
    hasChanged_ = true;
    if (isOutport()) {
        for (size_t i = 0; i <  connectedPorts_.size(); ++i)
             connectedPorts_[i]->invalidate();
    }
    else {
        getProcessor()->invalidate(invalidationLevel_);
    }
}

bool Port::allowMultipleConnections() const {
    return allowMultipleConnections_;
}

Processor* Port::getProcessor() const {
    return processor_;
}

bool Port::isOutport() const {
    return direction_ == OUTPORT;
}

bool Port::isInport() const {
    return direction_ == INPORT;
}

std::string Port::getName() const {
    return name_;
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

void Port::setLoopIteration(int iteration) {
    currentLoopIteration_ = iteration;
    if (currentLoopIteration_ >= getNumLoopIterations())
        LWARNINGC("voreen.Port", "Current loop iteration greater than number of loop iterations");
}

void Port::initialize() throw (VoreenException) {
    // nothing
}

void Port::deinitialize() throw (VoreenException) {
    // nothing
}

} // namespace voreen
