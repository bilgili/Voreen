/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#ifndef VRN_PROCESSOR_H
#define VRN_PROCESSOR_H

#include "voreen/core/properties/propertyowner.h"
#include "voreen/core/utils/observer.h"
#include "voreen/core/processors/profiling.h"

#include "tgt/exception.h"
#include "tgt/event/eventlistener.h"

#include <vector>
#include <boost/thread.hpp>

namespace voreen {

class CoProcessorPort;
class EventPropertyBase;
class InteractionHandler;
class Port;
class Processor;
class ProcessorFactory;
class ProcessorWidget;
class ProgressBar;
class VoreenModule;

class VRN_CORE_API ProcessorObserver : public PropertyOwnerObserver {
public:
    virtual void processorWidgetCreated(const Processor* processor) = 0;
    virtual void processorWidgetDeleted(const Processor* processor) = 0;

    virtual void portsChanged(const Processor* processor) = 0;
    virtual void stateChanged(const Processor* /*processor*/) {};
};

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API Observable<ProcessorObserver>;
#endif
/**
 * The base class for all processor classes used in Voreen.
 */
class VRN_CORE_API Processor : public PropertyOwner, public tgt::EventListener, public Observable<ProcessorObserver> {

    friend class NetworkEvaluator;
    friend class VoreenModule;
    friend class ProcessorNetwork;
    friend class ProcessorFactory;
    friend class Port;
    friend class ProcessorWidget;
    template <class T> friend class ProcessorBackgroundThread;

public:
    /**
     * @brief Specifies the invalidation status of the processor.
     * The NetworkEvaluator and Processor will only do as much as needed to get a Processor into a valid state.
     */
    enum InvalidationLevel {
        VALID = 0,
        INVALID_RESULT = 1,         ///< invalid rendering, volumes => call process()
        INVALID_PARAMETERS = 10,    ///< invalid uniforms => set uniforms
        INVALID_PATH = 15,          ///< path has been changed => update if necessary
        INVALID_PROGRAM = 20,       ///< invalid shaders, CUDA/OpenCL program => rebuild program
        INVALID_PORTS = 30,         ///< ports added/removed  => check connections, re-evaluate network
        INVALID_PROCESSOR = 40      ///< invalid python/matlab processor => re-create processor, re-connect ports (if possible)
    };

    /**
     * @brief Identifies the code stability of this processor implementation.
     * The default is CODE_STATE_EXPERIMENTAL
     */
    enum CodeState {
        CODE_STATE_OBSOLETE,
        CODE_STATE_BROKEN,
        CODE_STATE_EXPERIMENTAL,
        CODE_STATE_TESTING,
        CODE_STATE_STABLE
    };

     /**
     * @brief Identifies the state of this processor.
     */
    enum ProcessorState {
        PROCESSOR_STATE_NOT_INITIALIZED,
        PROCESSOR_STATE_NOT_READY,
        PROCESSOR_STATE_READY
    };

    Processor();

    virtual ~Processor();

    /**
     * Returns a copy of the processor.
     */
    virtual Processor* clone() const;

    /**
     * Returns the general category the processor belongs to.
     *
     * This method is not intended to be re-implemented by each subclass,
     * but is rather defined by the more concrete base classes,
     * such as VolumeRaycaster or ImageProcessor.
     */
    virtual std::string getCategory() const = 0;

    /**
     * Returns the development state of the processor implementation.
     *
     * This method is expected to be re-implemented by each concrete subclass.
     * The default value for all classes not re-writing this method is
     * CODE_STATE_EXPERIMENTAL.
     */
    virtual CodeState getCodeState() const;

    /**
     * Returns true if this Processor is a utility Processor (i.e., performs smaller tasks).
     *
     * The default implementation returns false. Override it in order to define a processor
     * as utility.
     */
    virtual bool isUtility() const;

    /**
     * Sets the guiname of this processor instance.
     *
     * @see PropertyOwner::setGuiName
     */
    void setGuiName();

    /**
     * Returns a string identifying the name of the module
     * this processor's class belongs to.
     */
    std::string getModuleName() const;

    /**
     * Returns the absolute directory of the module this processor belongs to.
     */
    std::string getModulePath() const;

    /**
     * Returns whether the processor has been successfully initialized.
     */
    bool isInitialized() const;

    /**
     * Returns true if process() can be called safely by the NetworkEvaluator.
     *
     * The default implementation checks, whether the processor has been initialized and
     * all inports, outports and coprocessor-inports are ready i.e. if they are connected
     * and the inports have data assigned. Overwrite it for custom behaviour.
     */
    virtual bool isReady() const;

    /**
     * Updates the processor's invalidation level.
     *
     * @see PropertyOwner::invalidate
     */
    virtual void invalidate(int inv = INVALID_RESULT);

    /**
     * Marks the processor as valid.
     */
    virtual void setValid();

    /**
     * Returns whether the processor is valid, i.e. all of its output data is valid
     * so the processor does not have to be updated.
     *
     * The standard implementation returns true, if getInvalidationLevel() == VALID.
     * Override it for custom behaviour.
     */
    virtual bool isValid() const;

    /// Is this processor an end processors as it does not have any output port?
    /// @todo more doc
    virtual bool isEndProcessor() const;

    /**
     * @brief Returns the processor's data flow inports.
     * This does not include its co-processor inports.
     *
     * @see getPorts
     */
    virtual const std::vector<Port*>& getInports() const;

    /**
     * @brief Returns the processor's data flow outports.
     * This does not include its co-processor outports.
     *
     * @see getPorts
     */
    virtual const std::vector<Port*>& getOutports() const;

    /**
     * Returns the processor's co-processor inports.
     *
     * @see getPorts
     */
    virtual const std::vector<CoProcessorPort*>& getCoProcessorInports() const;

    /**
     * Returns the processor's co-processor outports.
     *
     * @see getPorts
     */
    virtual const std::vector<CoProcessorPort*>& getCoProcessorOutports() const;

    /**
     * Convenience function collecting all of the processor's ports
     * and returning them in a single vector.
     */
    virtual std::vector<Port*> getPorts() const;

    /**
     * Returns the port with the given name, or null if such a port does not exist.
     */
    virtual Port* getPort(const std::string& name) const;

    /**
     * Returns the performance record of this processor.
     */
    const PerformanceRecord* getPerformanceRecord() const;

    /**
     * Resets the performance record of this processor.
     */
    void resetPerformanceRecord();

    /**
     * Processors may overwrite this function in order to gain access to
     * events that are propagated through the network.
     *
     * @note It is strongly recommended to receive events through
     *  EventProperties instead. Overwriting onEvent interferes
     *  with the network event flow in a very direct manner and
     *  may introduce hard to find bugs with global impact on the network.
     *  Furthermore, EventProperties allow convenient customization
     *  of the event handling within a particular network.
     *
     * @see EventProperty
     */
    virtual void onEvent(tgt::Event* e);

    /**
     * Same function as onEvent with an extra parameter for the port distributing the event.
     * This is used in Aggregation.
     * @see onEvent
     */
    virtual void onPortEvent(tgt::Event* e, Port* p);

    /**
     * Returns the event properties owned by the processor.
     *
     * @see onEvent
     */
    const std::vector<EventPropertyBase*> getEventProperties() const;

    /**
     * Returns the interaction handlers attached to the processor.
     */
    const std::vector<InteractionHandler*>& getInteractionHandlers() const;

    /**
     * Returns the processor widget that has been generated in
     * initialize() by using the ProcessorWidgetFactory retrieved from
     * VoreenApplication.
     *
     * Returns null, if no widget is present.
     */
    ProcessorWidget* getProcessorWidget() const;

    /**
     * A derived class should return true, if its process() method
     * Returns if this processor uses expensive computation.
     * is time-consuming, i.e., causes noticable delay.
     *
     * Normal renderers without expensive data processing should return false (default).
     */
    virtual bool usesExpensiveComputation() const;

    /**
     * Updates the progress bar, if one has been assigned.
     *
     * @param progress The overall progress of the operations
     *  performed in process(). Range: [0.0, 1.0]
     *
     * @see usesExpensiveComputation
     */
    void setProgress(float progress);

    /**
     * Assigns a progress handler that the processor may use for indicating progress
     * of time-consuming operations. Usually assigned by the GUI layer.
     */
    virtual void setProgressBar(ProgressBar* progressBar);

    /**
     * @see PropertyOwner::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see PropertyOwner::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    /**
     * Returns the meta data container of this processor.
     * External objects, such as GUI widgets, can use it
     * to store and retrieve persistent meta data without
     * having to bother with the serialization themselves.
     */
    virtual MetaDataContainer& getMetaDataContainer() const;

    /**
     * @brief Returns the cache path for this processor.
     * Cache path = Application cache path + Classname
     *
     * @see VoreenApplication::getCachePath()
     */
    std::string getCachePath() const;

    std::string getDescription() const;
    std::string getPropertyDescription(const std::string& propId) const;
    std::string getPortDescription(const std::string& portId) const;

    ProcessorState getProcessorState() const;
protected:
    /**
     * @brief This method is called by the NetworkEvaluator when the processor should be processed.
     *        All rendering and volume/image processing is to be done here.
     *
     * @note The NetworkEvaluator assumes the processor to be valid after calling this method
     *       and sets its invalidation level to VALID.
     */
    virtual void process() = 0;

    /**
     * Initializes the processor, its properties and its processor widget.
     *
     * All initialization should be done in this method, instead of the constructor.
     * It is issued by the NetworkEvaluator.
     *
     * @note The superclass' function must be called as first statement when it is overwritten.
     *
     * @note All OpenGL initializations must be done here,
     *       instead of the constructor! Time-consuming operations
     *       should also happen here.
     *
     * @throw tgt::Exception if the initialization failed
     */
    virtual void initialize() throw (tgt::Exception);

    /**
     * Deinitializes the processor.
     *
     * @note The superclass' function must be called as \e last statement when it is overwritten.
     *
     * @note All OpenGL deinitializations must be done here,
     *       instead of the destructor!
     *
     * @throw tgt::Exception if the deinitialization failed
     */
    virtual void deinitialize() throw (tgt::Exception);

    /**
     * Is called by the NetworkEvaluator immediately before it calls process().
     *
     * Override it for performing preparation operations, but make sure to call
     * the superclass' function as first statement, if you do so.
     */
    virtual void beforeProcess();

    /**
     * Is called by the NetworkEvaluator immediately after process() has been called.
     *
     * Override it for performing follow-up operations, but make sure to call
     * the superclass' function as \e last statement, if you do so.
     */
    virtual void afterProcess();

    /// Calls clear() on all outports
    virtual void clearOutports();

    /**
     * Lock the processor-mutex.
     * The processor-mutex is locked by the NetworkEvaluator before beforeProcess() and unlocked after afterProcess().
     * When changing the state (invalidation level etc.) of a processor from a background thread
     * make sure to lock the mutex to avoid parallel changes by the process() method.
     */
    void lockMutex();
    /// @look lockMutex()
    void unlockMutex();

    /**
     * Registers a port.
     *
     * Only registered ports can be connected and are visible in the GUI.
     * Added ports will not be deleted in the destructor. Ports should
     * be registered in the processor's constructor.
     */
    virtual void addPort(Port* port);

    /// @overload
    virtual void addPort(Port& port);

    /**
     * Unregister a port.
     */
    virtual void removePort(Port* port);

    /// @overload
    virtual void removePort(Port& port);

    /**
     * Initializes the passed port.
     *
     * @note Ports are automatically initialized by the Processor
     *  base class. Therefore, it is not necessary to call this
     *  function, unless a port is added \e after its processor
     *  has been initialized.
     *
     * @throw tgt::Exception If port initialization has failed.
     */
    void initializePort(Port* port) throw (tgt::Exception);

    /**
     * Deinitializes the passed port.
     *
     * @note Ports are automatically deinitialized by the Processor
     *  base class. Therefore, it is not necessary to call this
     *  function, unless a port is to be removed \e before its processor
     *  has been deinitialized.
     *
     * @throw tgt::Exception If port deinitialization has failed.
     */
    void deinitializePort(Port* port) throw (tgt::Exception);

    /// Adds an event property to this processor.
    void addEventProperty(EventPropertyBase* prop);

    /// @overload
    void addEventProperty(EventPropertyBase& prop);

    /// Attaches an interaction handler to this processor.
    void addInteractionHandler(InteractionHandler* handler);

    /// @overload
    void addInteractionHandler(InteractionHandler& handler);

    /**
     * This method is called if the Processor is switched into or out of interaction mode.
     *
     * Overwrite this method if a processor needs to react to this event (e.g., resize
     * RenderPorts, change samplingrate, etc.). The default implementation does nothing.
     */
    virtual void interactionModeToggled();

    /**
     * Call this method if the Processor should be switched into or out of interaction mode.
     *
     * @param interactionMode
     * @param source The source (usually a property widget).
     *
     * @see PropertyOwner::interactionMode
     */
    virtual void toggleInteractionMode(bool interactionMode, void* source);

    virtual void notifyPortsChanged() const;
    virtual void notifyStateChanged() const;

    virtual void setDescriptions() = 0;

    /// Sets the description
    void setDescription(std::string desc);

    /// Set to true after successful initialization.
    ProcessorState processorState_;

    /// Used for the detection of duplicate port names.
    std::map<std::string, Port*> portMap_;

    /// category used in logging
    static const std::string loggerCat_;

    /// Used for performance profiling (experimental).
    PerformanceRecord performanceRecord_;

    /**
     * Used by the processor for indicating progress
     * of time-consuming operations.
     *
     * @see setProgress
     */
    ProgressBar* progressBar_;

protected:
    /**
     * Sets the name of the module the processor's class belongs to.
     * To be called by VoreenModule.
     */
    void setModuleName(const std::string& moduleName);

    /**
     * Set the guiname for this processor instance. To be called
     * by the owning processor network.
     */
    void setGuiName(const std::string& guiName);

    /**
     * Set the id for this processor instance. To be called
     * by the owning processor network.
     */
    void setID(const std::string& id);

    /**
     * Causes the processor to give up ownership of its GUI widget without deleting it.
     * This is called by the ProcessorWidget's destructor and prevents the processor
     * from double freeing its widget.
     */
    void deregisterWidget();

    /// Name of the module the Processor's class belongs to.
    std::string moduleName_;

    /// List of ports specifying the input the processor expects.
    std::vector<Port*> inports_;

    /// List of ports specifying the output the processor generates.
    std::vector<Port*> outports_;

    /// The CoProcessorInports this processor has.
    std::vector<CoProcessorPort*> coProcessorInports_;

    /// The CoProcessorOutports this processor has.
    std::vector<CoProcessorPort*> coProcessorOutports_;

    /// Vector of all event properties of the processor.
    std::vector<EventPropertyBase*> eventProperties_;

    /// Vector of all interaction handlers of the processor.
    std::vector<InteractionHandler*> interactionHandlers_;

    /// Optional GUI widget representing Processor instance.
    ProcessorWidget* processorWidget_;

    /// used for cycle prevention during invalidation propagation
    bool invalidationVisited_;

    /// used for cycle prevention during interaction mode propagation
    bool interactionModeVisited_;

    /// used for cycle prevention during event propagation
    bool eventVisited_;

    /// This mutex is locked by the NetworkEvaluator before beforeProcess() and unlocked after afterProcess().
    boost::mutex mutex_;

    /**
     * Contains the associated meta data.
     *
     * We want to return a non-const reference to it from a const member function
     * and since the MetaDataContainer does not affect the processor state itself,
     * mutable appears justifiable.
     */
    mutable MetaDataContainer metaDataContainer_;

    /// Description for display in GUI etc.
    std::string description_;
};

} // namespace voreen

#endif // VRN_PROCESSOR_H
