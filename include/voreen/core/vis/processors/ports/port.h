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

#ifndef VRN_PORT_H
#define VRN_PORT_H

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/volume/volumehandle.h"
#include "tgt/tgt_gl.h"

#include <map>
#include <string>
#include <typeinfo>
#include <vector>

namespace voreen {

/**
 * This class describes a port of a Processor. Processors are connected
 * by their ports.
 */
class Port {

    friend class Processor;
    friend class ProcessorNetwork;

public:
    enum PortDirection {
        OUTPORT = 0,
        INPORT
    };

    /**
     * @brief
     *
     * @param name The name of the port, must be unique per processor.
     * @param processor The processor this port belongs to.
     * @param direction Is this port an inport or outport?
     * @param allowMultipleConnections  Can this port handle multiple connections? (Outports always can)
     * @param invalidationLevel For inports: The Processor is invalidated with this invalidationlevel when the data on this port changes.
     * For outports: The Processor is invalidated with this invalidationlevel when this port is connected.
     */
    Port(const std::string& name, PortDirection direction, bool allowMultipleConnections = false,
         Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT);

    virtual ~Port();

    /**
     * @brief Test if this (out)port can connect to a given inport.
     * Conditions tested:
     * This port must be an outport, the port to connect to must be an inport.
     * The ports must not already be connected.
     * The inport must allow multiple connections or may not be already connected to any other port.
     * Both ports must be of the same exact type (Checked using typeid).
     * @note If you plan on implementing this method in subclasses be sure to perform
     * typechecks. The port subclasses assume to be connected to ports of the same type and
     * therefore use static_cast.
     *
     * @param inport The port to connect to. Must be an inport.
     *
     * @return True if connecting is possible, false otherwise.
     */
    virtual bool testConnectivity(const Port* inport) const;

    /**
     * Returns whether this port allows multiple connections or not.
     */
    bool allowMultipleConnections() const { return allowMultipleConnections_; }

    /**
     * Returns the index of the given port in the vector of connected ports
     */
    int getIndexOf(const Port* port) const;

    virtual size_t getNumConnections() const;

    /**
     * Returns the processor this port belongs to.
     */
    Processor* getProcessor() const { return processor_; }

    bool isConnected() const;

    /**
     * Returns whether this port is connected to the one given as parameter.
     */
    bool isConnectedTo(const Port* port) const;

    /**
     * Returns whether this port is an outport or not.
     */
    bool isOutport() const { return direction_ == OUTPORT; }

    /**
     * Returns whether this port is an inport or not.
     */
    bool isInport() const { return direction_ == INPORT; }

    /**
     * Returns whether the port is ready to be used
     * by its owning processor.
     *
     * This function is supposed to return true,
     * if the port is connected and contains valid data.
     */
    virtual bool isReady() const = 0;

    /**
     * @brief Invalidate this port and propagate invalidation.
     *
     * If the port is an outport: invalidate all connected (in)ports.
     * If the port is an inport: invalidate processor with the given InvalidationLevel and set hasChanged=true.
     */
    void invalidate();

    /**
     * @brief Has the data in this port changed since the last process() call?
     *
     * @return True if the data has changed since Processor::process() [and setValid()] call of the processor this port belongs to.
     */
    bool hasChanged() const;

    /// Sets hasChanged to false. All inports are validated by Processor::setValid().
    void setValid();

    std::string getName() { return name_; }

    /**
     * Returns all the ports that are connected to this port.
     */
    const std::vector<Port*>& getConnected() const;

    void distributeEvent(tgt::Event* e);

    void toggleInteractionMode(bool interactionMode, void* source);

protected:

    virtual bool connect(Port* inport);
    ///Disconnect from other port (must not be NULL or this port)
    virtual void disconnect(Port* other);
    virtual void disconnectAll();

    virtual void setProcessor(Processor* p);

    std::string name_;
    std::vector< Port* > connectedPorts_; /** The ports connected to this one */
    Processor* processor_;        /** The processor this port belongs to */
    const PortDirection direction_;                    /** Is this port an outport or not */
    bool allowMultipleConnections_;     /** Is this port allowed to have multiple connections? */
    bool hasChanged_;
    Processor::InvalidationLevel invalidationLevel_;
};

}   // namespace voreen

#endif  // VRN_PORT_H
