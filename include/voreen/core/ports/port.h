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

#ifndef VRN_PORT_H
#define VRN_PORT_H

#include "voreen/core/voreencoreapi.h"
#include "voreen/core/processors/processor.h"

#include "tgt/exception.h"

#include <string>
#include <vector>

namespace voreen {

class PortCondition;

/**
 * This class describes a port of a Processor. Processors are connected
 * by their ports.
 */
class VRN_CORE_API Port {

    friend class PortCondition;
    friend class Processor;
    friend class ProcessorNetwork;
    friend class NetworkEvaluator;

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

    /**
     * Destructor, disconnects all connected ports.
     */
    virtual ~Port();

    /**
     * Adds a port condition for checking the validity of the assigned port data.
     * If any of the assigned conditions is not fulfilled, isReady() returns false.
     * The port takes ownership of the passed object.
     *
     * @note PortConditions may only be added to inports.
     */
    void addCondition(PortCondition* condition);

    /**
     * Returns all ports that are connected to this port.
     */
    const std::vector<const Port*> getConnected() const;

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
     * @brief Invalidate this port and propagate invalidation.
     *
     * If the port is an outport: invalidate all connected (in)ports.
     * If the port is an inport: invalidate processor with the given InvalidationLevel and set hasChanged=true.
     */
    void invalidate();

    /**
     * Returns whether this port allows multiple connections or not.
     */
    bool allowMultipleConnections() const;

    /**
     * Returns the processor this port belongs to.
     */
    Processor* getProcessor() const;

    /**
     * Returns the number of ports connected to this one.
     */
    virtual size_t getNumConnections() const;

    /**
     * Returns the current loop iteration, which is intended to be retrieved
     * by the owning processor during process(). For non-loopports, 0 is returned.
     */
    int getLoopIteration() const;

    /**
     * Returns true, if the number of connected ports is greater zero.
     */
    bool isConnected() const;

    /**
     * Returns whether this port is connected to the one given as parameter.
     */
    bool isConnectedTo(const Port* port) const;

    /**
     * Returns whether this port is an outport or not.
     */
    bool isOutport() const;

    /**
     * Returns whether this port is an inport or not.
     */
    bool isInport() const;

    /**
     * Returns whether the port contains data.
     * To be overridden by a concrete subclass.
     * The default implementation returns false.
     */
    virtual bool hasData() const;

    /**
     * Returns whether the port is ready to be used
     * by its owning processor.
     *
     * This function is supposed to return true,
     * if the port is connected and contains valid data.
     */
    virtual bool isReady() const;

    /**
     * @brief Has the data in this port changed since the last process() call?
     *
     * @return True if the data has changed since Processor::process() [and setValid()] call of the processor this port belongs to.
     */
    bool hasChanged() const;

    /**
     * Returns the name of the port, must be unique per processor.
     */
    std::string getName() const;

    /**
     * Returns the name of the port, prefixed by the name of its processor.
     */
    std::string getQualifiedName() const;

    /**
     * Marks the port as valid. Is called by by Processor::setValid()
     */
    void setValid();

    virtual void clear();

    /**
     * Returns whether the port has been initialized.
     */
    bool isInitialized() const;

    /**
     * Specifies whether this port is to be used as loop port.
     *
     * When a loop outport is connected to a loop inport of a
     * processor that is a predecessor of the loop outport's owner, all processors
     * between the owners of the two loop ports are regarded as loop.
     * This loop is then executed multiple times per rendering pass.
     * The number of loop iterations is determined by getNumIterations().
     */
    void setLoopPort(bool isLoopPort);

    /**
     * Returns whether this port is a loop port.
     *
     * @see setLoopPort
     */
    bool isLoopPort() const;

    /**
     * \brief Specifies the number of loop iterations. Only relevant,
     * if the port is a loop port.
     *
     * @see setLoopPort
     *
     * @note A iteration count less than 1 is ignored, so the loop
     *       is executed at least once per rendering pass.
     */
    void setNumLoopIterations(int iterations);

    /**
     * \brief Returns the number of loop iterations. Only relevant,
     * if the port is a loop port.
     */
    int getNumLoopIterations() const;

    /**
     * Indicates whether the port supports caching of its content,
     * default: false.
     *
     * A cachable port type is supposed to override this method
     * and return true. In this case, the subclass must also re-implement
     * getHash, saveData and loadData.
     *
     * @see Cache
     */
    virtual bool supportsCaching() const;

    /**
     * Returns an hash of the port's data. This method has to be
     * re-implemented by a concrete port type, if it supports caching.
     * The default implementation returns an empty string.
     *
     * @see supportsCaching
     */
    virtual std::string getHash() const;

    /**
     * Saves the port's data to the given path.
     *
     * Since implementing the saving routine is up to the
     * concrete subclasses, saving is not necessarily supported
     * by all port types. A cachable port type, however,
     * is required to provide saving and loading of its data.
     *
     * @see supportsCaching
     *
     * @throws VoreenException If saving failed or is generally
     *      not supported by the port type.
     */
    virtual void saveData(const std::string& path) const
        throw (VoreenException);

    /**
     * Loads port data from the given path and assigns it
     * to the port on success.
     *
     * Since implementing the loading routine is up to the
     * concrete subclasses, loading is not necessarily supported
     * by all port types. A cachable port type, however,
     * is required to provide saving and loading of its data.
     *
     * @see supportsCaching
     *
     * @throws VoreenException If loading failed or is generally
     *      not supported by the port type.
     */
    virtual void loadData(const std::string& path)
        throw (VoreenException);

    virtual void distributeEvent(tgt::Event* e);

    void toggleInteractionMode(bool interactionMode, void* source);

    std::string getDescription() const;

    /// Sets the description
    void setDescription(std::string desc);

    /**
     * Returns this port-type's suggested color in case of a GUI representation.
     *
     */
    virtual tgt::col3 getColorHint() const;

protected:
    /**
     * Performs OpenGL dependent and expensive initializations,
     * which are not allowed to be done in the constructor.
     *
     * @note The superclass' function must be called as first statement
     *       when it is overwritten.
     *
     * @note Is called by the owning processor during its initialization.
     *       Do not call it directly in a subclass.
     *
     * @throw tgt::Exception if the initialization failed
     */
    virtual void initialize() throw (tgt::Exception);

    /**
     * Performs OpenGL dependent deinitializations.
     * Is called by the owning processor during its deinitialization.
     *
     * All resources that have been allocated by initialize() have to be freed
     * in this function rather than the destructor.
     *
     * @note The superclass' function must be called as \e last statement
     *       when it is overwritten.
     *
     * @note Is called by the owning processor during its initialization.
     *       Do not call it directly in a subclass.
     *
     * @throw tgt::Exception if the deinitialization failed
     */
    virtual void deinitialize() throw (tgt::Exception);

    /**
     * Iterates over all assigned port conditions and returns false,
     * if any of them fails. Additionally, an error message is logged.
     *
     * To be called by isReady() of the subclasses.
     */
    virtual bool checkConditions() const;

    virtual bool connect(Port* inport);
    ///Disconnect from other port (must not be NULL or this port)
    virtual void disconnect(Port* other);
    virtual void disconnectAll();

    virtual void setProcessor(Processor* p);

    std::string name_;
    std::vector<Port*> connectedPorts_; ///< The ports connected to this one
    Processor* processor_;                ///< The processor this port belongs to
    const PortDirection direction_;       ///< Is this port an outport or not
    bool allowMultipleConnections_;       ///< Is this port allowed to have multiple connections?
    bool hasChanged_;

    std::vector<PortCondition*> conditions_;

    Processor::InvalidationLevel invalidationLevel_;

    /// category used in logging
    static const std::string loggerCat_;
private:

    /**
     * Defines the current loop iteration, which is intended to be retrieved
     * by the owning processor during process(). For non-loopports, this function
     * is a no-op.
     *
     * @note Is called by the NetworkEvaluator.
     */
    void setLoopIteration(int iteration);

    bool isLoopPort_;          ///< defines if the port is a loop port , i.e., if
                               ///  it is used to define loops in the network
    int numLoopIterations_;    ///< specifies the number of iterations, in case the port is a loop.
    int currentLoopIteration_; ///< current iteration, to be retrieved by the processor

    /// Set to true by after successful initialization.
    bool initialized_;

    /// Description for display in GUI etc.
    std::string description_;
};

}   // namespace voreen

#endif  // VRN_PORT_H
