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

#ifndef VRN_NETWORKEVALUATOR_H
#define VRN_NETWORKEVALUATOR_H

#include <deque>
#include <queue>

#include "tgt/glcanvas.h"
#include "voreen/core/datastructures/rendertarget.h"
#include "voreen/core/network/networkgraph.h"
#include "voreen/core/network/processornetworkobserver.h"
#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/processors/profiling.h"

namespace voreen {

class ProcessorNetwork;

class VRN_CORE_API NetworkEvaluator : public ProcessorNetworkObserver {
public:
    /**
     * Wrapper around the process() calls. Can be used e.g. for benchmarking.
     */
    class VRN_CORE_API ProcessWrapper  {
        friend class NetworkEvaluator;
    public:
        virtual ~ProcessWrapper() {}
    protected:
        virtual void beforeProcess(Processor* /*p*/) {}
        virtual void afterProcess(Processor* /*p*/) {}
        virtual void beforeNetworkProcess() {}
        virtual void afterNetworkProcess() {}

        virtual void beforeNetworkInitialize() {}
        virtual void afterNetworkInitialize() {}
        virtual void beforeNetworkDeinitialize() {}
        virtual void afterNetworkDeinitialize() {}
    };

public:

    /**
     * Constructor.
     *
     * @param glMode if false, the network evaluator does not make any OpenGL calls.
     *          Should be set to false in applications that do not use a valid OpenGL context.
     * @param sharedContext A canvas holding the OpenGL context that will be used
     *          for initializing the processors. More precisely, if a canvas has been passed
     *          <code>sharedContext->getGLFocus()</code> is called before <code>Processor::initialize()</code>
     *          in order to make sure that OpenGL initializations are performed
     *          within the correct OpenGL context.
     *          This is usually not necessary on single-context systems.
     *          In non OpenGL mode the shared context is ignored.
     */
    NetworkEvaluator(bool glMode = true, tgt::GLCanvas* sharedContext = 0);

    ~NetworkEvaluator();

    /**
     * Assigns the processor network to be evaluated.
     *
     * @param network the network the evaluator will operate on. May be null.
     * @param deinitializePrevious if true, the currently assigned network is deinitialized
     *        before the passed one is assigned
     */
    void setProcessorNetwork(ProcessorNetwork* network, bool deinitializeCurrent = true);

    /**
     * Returns the processor network currently assigned to this evaluator.
     */
    const ProcessorNetwork* getProcessorNetwork() const;

    /**
     * Processes the currently assigned network. The rendering order is determined internally
     * according the network topology and the invalidation levels of the processors.
     */
    void process();

    /**
     * Performs all necessary updates whenever the network has changed. Call this
     * method carefully and only if the NETWORK changed, not the connections. I.e.,
     * call it whenever processors were added or removed from the network, like
     * it is done by the NetworkEditor class/widget. If the entire ProcessorNetwork
     * was changed, via <code>setProcessorNetwork()</code>, this method is called
     * automatically.
     */
    void onNetworkChange();

    /**
     * Initializes all processors in the current ProcessorNetwork by calling
     * <code>initialize()</code> on each of them. If a shared context has been
     * passed to the constructor, it is used for the initialization. Otherwise,
     * the caller has to make sure that the correct context is present.
     *
     * @return true if initialization of all processors has been successful
     */
    bool initializeNetwork();

    /**
     * Deinitializes all processors in the current ProcessorNetwork by calling
     * <code>initialize()</code> on each of them. If a shared context has been
     * passed to the constructor, it is used for the initialization. Otherwise,
     * the caller has to make sure that the correct context is present.
     *
     * @return true if deinitialization of all processors has been successful
     */
    bool deinitializeNetwork();

    /**
     * Invalidates all processors in the current network with the specified invalidation level.
     */
    void invalidateProcessors(Processor::InvalidationLevel level = Processor::INVALID_RESULT);

    /**
     * Invalidates all processors in the current network and afterwards re-processes it.
     *
     * @note: Calling this function is usually not necessary, since all directly or indirectly
     *        involved processors are invalidated automatically whenever a processor's state
     *        (\sa invalidate()) or the network topology change, and it should be avoided,
     *        since it breaks caching.
     *        It might, however, be useful in certain situations such as after program initialization.
     */
    void forceUpdate();

    /**
     * Invalidates all CanvasRenderers and calls process() afterwards.
     */
    void updateCanvases();

    /**
     * Add a ProcessWrapper which is called before and after Processor::process() is called
     */
    void addProcessWrapper(ProcessWrapper* w);

    /**
     * Removes the passed ProcessWrapper object, if it is currently registered.
     */
    void removeProcessWrapper(const ProcessWrapper* w);

    /**
     * Removes all registered ProcessWrapper objects. The objects are not freed.
     */
    void clearProcessWrappers();

    /**
     * \brief Locks the evaluator. In this state, it does not perform
     * any operations, such as initializing or processing, on the processor network.
     *
     * This function can be used for preventing the evaluator from
     * operating on an inconsistent network.
     */
    void lock();

    /**
     * Unlocks the evaluator, restoring its default state. \sa lock
     */
    void unlock();

    /**
     * Returns whether the evaluator is locked. \sa lock
     */
    bool isLocked() const;

    /**
     * Returns all render ports currently used by the network that contain valid data.
     */
    std::vector<RenderPort*> collectRenderPorts() const;

    /**
     * Returns all performance records currently used by the network.
     */
    std::vector<const PerformanceRecord*> collectPerformanceRecords() const;

    /**
     * Clears all performance records of the processors in the network.
     */
    void clearPerformanceRecords();

    /**
     * Marks the assigned network as modified, which causes
     * onNetworkChanged() to be called during next process() call.
     *
     * @see ProcessorNetworkObserver
     */
    virtual void networkChanged();

    /**
     * Deinitializes the processor before removal.
     */
    virtual void processorRemoved(const Processor* processor);

    /**
     * This network modification is not relevant to the evaluator and therefore ignored.
     *
     * @see ProcessorNetworkObserver
     */
    virtual void processorRenamed(const Processor* processor, const std::string& prevName);

    /**
     * This network modification is not relevant to the evaluator and therefore ignored.
     *
     * @see ProcessorNetworkObserver
     */
    virtual void propertyLinkAdded(const PropertyLink* link);

    /**
     * This network modification is not relevant to the evaluator and therefore ignored.
     *
     * @see ProcessorNetworkObserver
     */
    virtual void propertyLinkRemoved(const PropertyLink* link);

private:

    /**
     * Wrapper around the Processor::process() calls to check that OpenGL state conforms to
     * default settings. Log a warning message if not.
     */
    class CheckOpenGLStateProcessWrapper : public ProcessWrapper {
    protected:
        void afterProcess(Processor* p);
        void beforeNetworkProcess();
        void checkState(Processor* p = 0);
        void warn(Processor* p, const std::string& message);
    };

    /**
     * Causes the member renderingOrder_ to be updated and defines a rendering order
     * for the processors in the current ProcessorNetwork according to the current
     * NetworkGraph.
     * This method will do nothing if netGraph_ is NULL. If netGraphTransposed_ is
     * also NULL, this member will be re-created from netGraph_.
     */
    void defineRenderingOrder();

    /**
     * Creates and assigns the RenderTargets to the RenderPorts,
     * if render target sharing is enabled.
     */
    void assignRenderTargets();

    /**
     * Check the states of all processors in the network and returns true,
     * if any invalidation state is greater or equal INVALID_PORTS.
     */
    bool checkForInvalidPorts();

    /**
     * Returns all processor from the current RenderingNetwork which return
     * true by calls to <code>isEndProcessor()</code> and thereby acts as
     * 'drains' in the network, i.e. CanvasRenderer.
     */
    const std::set<Processor*> getEndProcessors() const;

    /// The network that is to be evaluated
    ProcessorNetwork* network_;

    /// Canvas holding the shared context that is to be used for processor initializations
    tgt::GLCanvas* sharedContext_;

    /**
     * If set to false, the network evaluator does not make any OpenGL calls and does
     * also not use the OpenGLStateProcessWrapper.
     */
    bool glMode_;

    /// Vector holding all processor wrappers which might have been added.
    std::vector<ProcessWrapper*> processWrappers_;

    /**
     * If this member is set to true, the method <code>assignRenderTargets()</code>
     * will not attempt to assign the same target in multiple ports on processors
     * in the current ProcessorNetwork. This permits better debugging, but is normally
     * a greate waste of resources. (currently not in use)
     */
    bool reuseRenderTargets_;

    /// Vector containing all processors which can be rendered. This vector is obtained
    /// by sorting netGraph_ topological.
    std::vector<Processor*> renderingOrder_;

    /// Maps from processors to their loop inports.
    /// Is used for incrementing the current iteration counter.
    std::map<Processor*, std::vector<Port*> > loopPortMap_;

    /// set to true, if the evaluated network has changed since last onNetworkChanged() call
    bool networkChanged_;

    /// indicates that the evaluator is in lock mode, i.e., not performing any operations
    bool locked_;

    /**
     * Indicates that a network processing has been scheduled,
     * which usually happens when process() is called on an locked evaluator
     */
    bool processPending_;

    /// Used for performance profiling (experimental).
    PerformanceRecord performanceRecord_;

    /// category used for logging
    static const std::string loggerCat_;
};

}   // namespace

#endif  // VRN_NETWORKEVALUATOR_H
