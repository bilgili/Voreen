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

#ifndef VRN_PROCESSOR_H
#define VRN_PROCESSOR_H

#include <vector>
#include <set>

#include "tgt/event/eventlistener.h"

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

class ProcessorFactory;
class Port;
class VolumePort;
class CoProcessorPort;

class Property;
class EventProperty;
class InteractionHandler;

class ProcessorWidget;

/**
 * The base class for all processor classes used in Voreen.
 */
class Processor : public AbstractSerializable, public tgt::EventListener {

    friend class NetworkEvaluator;
    friend class ProcessorNetwork;
    friend class ProcessorFactory;
    friend class Port;
    friend class ProcessorWidget;
    friend class Property;

public:

    /**
     * @brief This enum specifies how invalid the processor is.
     * The NetworkEvaluator and Processor will only do as much as needed to get a Processor into a valid state.
     */
    enum InvalidationLevel {
        VALID = 0,
        INVALID_RESULT,        ///<  invalid rendering, volumes => call process()
        INVALID_PARAMETERS,    ///< invalid uniforms => set uniforms
        INVALID_PROGRAM,       ///< invalid shaders, cuda/opencl program => rebuild program
        INVALID_PORTS,         ///< ports added/removed  => check connections, re-evaluate network
        INVALID_PROCESSOR      ///< invalid python/matlab processor => re-create processor, re-connect ports (if possible)
    };

    /**
     * @brief Identifies the state of the code of this processor.
     * The default is CODE_STATE_EXPERIMENTAL
     */
    enum CodeState {
        CODE_STATE_OBSOLETE,
        CODE_STATE_BROKEN,
        CODE_STATE_EXPERIMENTAL,
        CODE_STATE_TESTING,
        CODE_STATE_STABLE
    };

    Processor();

    virtual ~Processor();

    /**
     * Returns the C++ class name of the derived class.
     * Necessary due to the lack of code reflection in C++.
     */
    virtual std::string getClassName() const = 0;

    /**
     * Returns the general category the processor belongs to. 
     *
     * This method is not intended to be re-implemented by each subclass,
     * but is rather defined by the more concrete base classes,
     * such as VolumeRaycaster or ImageProcessor.
     */
    virtual std::string getCategory() const = 0;

    /**
     * Returns an enum indicating the state of the code of this processor.
     *
     * This method is expected to be re-implemented by each concrete subclass.
     * The default value for all classes not re-writing this mehtod is
     * CODE_STATE_EXPERIMENTAL.
     */
    virtual Processor::CodeState getCodeState() const;

    /**
     * Returns a string identifying the name of the module this processor belongs to.
     *
     * This method is expected to be re-implemented by each concrete subclass.
     * Currently the returned default value is "unknown" which should be regarded as
     * an error.
     */
    virtual std::string getModuleName() const;

    /**
     * Returns true if this Processor is a utility Processor (i.e., performs smaller tasks).
     *
     * The default implementation returns false. Override it in order to define a processor
     * as utility.
     */
    virtual bool isUtility() const;

    /**
     * Returns a description of the processor's functionality.
     *
     * This method is expected to be re-implemented by each concrete subclass.
     */
    virtual const std::string getProcessorInfo() const;

    /** 
     * Returns the name of this processor instance.
     */
    const std::string& getName() const;

    /**
     * Returns a copy of the processor.
     */
    virtual Processor* clone() const;

    /**
     * Returns whether the processor has been successfully initialized.
     */
    bool isInitialized() const;

    /**
     * @brief Marks that the processor needs to be updated and propagate invalidation.
     *
     * The new InvalidationLevel is max(inv, currentInvalidationLevel).
     * Use setValid() to mark the processor as valid.
     * Calls invalidate() on all outports and coprocessor outports if the processor is valid when this method is called.
     *
     * @param inv Specifies what components of this processor are invalid.
     */
    virtual void invalidate(InvalidationLevel inv = INVALID_RESULT);

    /** 
     * Returns true, if this Processor is in interaction mode, false otherwise.
     */
    virtual bool interactionMode() const;

    /**
     * Returns the processor's current validation state.
     */
    InvalidationLevel getInvalidationLevel() const;

    /**
     * Returns true if process() can be called safely by the NetworkEvaluator.
     *
     * The default implementation checks, if all inports, outports and coprocessor-inports are ready,
     * i.e. if they are connected and the inports have data assigned. Override it for custom behaviour.
     */
    virtual bool isReady() const;

    /**
     * Returns whether the processor is valid, i.e. all of its output data is valid
     * so the processor does not have to be updated.
     *
     * The standard implementation returns true, if getInvalidationLevel() == VALID.
     * Override it for custom behaviour.
     */
    virtual bool isValid() const;

    /**
     * Convenience function setting the processors invalidation level to VALID.
     * This is called by the NetworkEvaluator after calling process().
     */
    void setValid();

    /// Is this processor an end processors as it does not have any output port?
    /// @todo more doc
    virtual bool isEndProcessor() const;

    /**
     * This shall return true in processor classes which can act as roots
     * for the network evaluation. Usually these processors have no inports
     * and "inject" data into the network, e.g. VolumeSource or VolumeCollectionSource
     * processor classes. This method is essential for the NetworkEvaluator and
     * its capability of evaluating network in real-time.
     */
    virtual bool isRootProcessor() const;

    const std::vector<Port*>& getInports() const;
    const std::vector<Port*>& getOutports() const;
    const std::vector<CoProcessorPort*>& getCoProcessorInports() const;
    const std::vector<CoProcessorPort*>& getCoProcessorOutports() const;
    const std::vector<Port*> getPorts() const;

    const std::vector<Property*>& getProperties() const;

    // todo: documentation of event/interaction stuff
    virtual void onEvent(tgt::Event* e);
    const std::vector<EventProperty*> getEventProperties() const;
    virtual const std::vector<InteractionHandler*>& getInteractionHandlers() const;

    /**
     * Returns the processor widget that has been generated in
     * initialize() by using the ProcessorWidgetFactory retrieved from
     * VoreenApplication.
     *
     * Returns null, if no widget is present.
     */
    ProcessorWidget* getProcessorWidget() const;

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);
   
    /** 
     * Returns the meta data container of this processor.
     * External objects, such as GUI widgets, can use it
     * to store and retrieve persistent meta data without 
     * having to bother with the serialization themselves.
     *
     * @see MetaDataContainer
     */
    MetaDataContainer& getMetaDataContainer() const;

    /**
     * Returns the state of this Processor object. The state is defined
     * by the set of the values of all Property objects held by this processor.
     * The returned value is basically meaningless and only serves for compared
     * to a previously obtained value.
     * If those values are equal, not state change has occured. If the values
     * are different, the state of the processor has been changed.
     *
     * This is used i.e. by the NetworkEvaluator in order to apply caching for
     * data.
     *
     * @deprecated
     */
    std::string getState() const;

protected:

    /**
     * @brief This method is called by the NetworkEvaluator when the processor should be processed.
     *        All rendering and volume/image processing is to be done here.
     *
     * @note The NetworkEvaluator assumes that the processor is valid after calling this method
     *       and sets the invalidation level to VALID.
     */
    virtual void process() = 0;

    /**
     * Returns an instance of the concrete Processor.
     */
    virtual Processor* create() const = 0;

    /**
     * Initializes the processor, its properties and its widget, if present.
     *
     * All initialization should be done in this method, instead of the constructor.
     * It is issued by the NetworkEvaluator.
     *
     * @note The superclass' function must be called when it is overwritten.
     *
     * @throw VoreenException if the initialization failed
     */
    virtual void initialize() throw (VoreenException);

    /**
     * This method is called if the Processor is switched into or out of interaction mode.
     *
     * Overwrite this method if a processor needs to react (e.g., resize RenderPorts, change samplingrate...)
     * The default implementation does nothing.
     */
    virtual void interactionModeToggled();

    /**
     * This method is called if the Processor is switched into or out of interaction mode.
     *
     * @param interactionMode
     * @param source The source (usually a property) that
     */
    void toggleInteractionMode(bool interactionMode, void* source);

    /**
     * Registers a port.
     *
     * Only registered ports can be connected and are visible in the GUI.
     * Added ports will not be deleted in the destructor. Ports should
     * be registered in the processor's constructor.
     */
    void addPort(Port* port);

    /// @overload
    void addPort(Port& port);

    /**
     * Registers a property.
     *
     * This is necessary for a property to get serialized.
     * Properties should be added in the processor's constructor.
     */
    void addProperty(Property* prop);

    /// \overload
    void addProperty(Property& prop);

    void addEventProperty(EventProperty* prop);

    void addInteractionHandler(InteractionHandler* handler);

    /// @overload
    void addInteractionHandler(InteractionHandler& handler);

    std::map<std::string, Port*> portMap_;

    /// Set to true after successful initialization.
    bool initialized_;  

    static const std::string loggerCat_; ///< category used in logging

private:

    /** 
     * Set a name for this processor instance. To be called
     * by the owning processor network.
     */
    void setName(const std::string& name);

    /**
     * Disconnects all ports. To be called by the 
     * owning processor network.
     */
    void disconnectAllPorts();

    /**
     * Causes the processor to give up ownership of its GUI widget without deleting it.
     * This is called by the ProcessorWidget's destructor and prevents the processor
     * from double freeing its widget.
     */
    void deregisterWidget();

    /// Name of the Processor instance.
    std::string name_;

    /// The processor's current invalidation state.
    InvalidationLevel invalidationLevel_;

    /// List of ports that specifies which inputs this processor needs.
    std::vector<Port*> inports_;

    /// List of ports that specifies which outputs this processor creates
    std::vector<Port*> outports_;

    /// The CoProcessorInports this processor has.
    std::vector<CoProcessorPort*> coProcessorInports_;

    /// The CoProcessorOutports this processor has.
    std::vector<CoProcessorPort*> coProcessorOutports_;

    /// Vector with all properties of the processor.
    std::vector<Property*> properties_;

    /// Vector of all event properties of the processor.
    std::vector<EventProperty*> eventProperties_;

    /// Vector of all interaction handlers of the processor.
    std::vector<InteractionHandler*> interactionHandlers_;

    /// Optional GUI widget representing Processor instance.
    ProcessorWidget* processorWidget_;

    /// used for cycle prevention during invalidation propagation
    bool invalidationVisited_;

    std::set<void*> interactionModeSources_;

    /**
     * Contains the associated meta data.
     * 
     * We want to return a non-const reference to it from a const member function 
     * and since the MetaDataContainer does not affect the processor itself,
     * mutable appears justifiable.
     */
    mutable MetaDataContainer metaDataContainer_;

};

} // namespace voreen

#endif // VRN_PROCESSOR_H
