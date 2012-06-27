#/**********************************************************************
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


#ifndef VRN_NETWORKEVALUATOR_H
#define VRN_NETWORKEVALUATOR_H

#include <deque>
#include <queue>

#include "tgt/glcanvas.h"
#include "voreen/core/io/volumecache.h"
#include "voreen/core/vis/rendertarget.h"
#include "voreen/core/vis/processors/graphvisitor.h"
#include "voreen/core/vis/network/networkgraph.h"
#include "voreen/core/vis/processors/renderprocessor.h"
#include "voreen/core/vis/processors/processor.h"

namespace voreen {

class ProcessorNetwork;

// ----------------------------------------------------------------------------

/**
 * GraphVisitor subclass designed to collect all visited processors during
 * graph traversing.
 */
class CollectSuccessorsVisitor : public GraphVisitor {
public:
    CollectSuccessorsVisitor();
    virtual ~CollectSuccessorsVisitor();
    const std::vector<Processor*>& getSuccessors() const;
    virtual bool visit(Processor* const processor);

private:
    std::vector<Processor*> successors_;
};

// ----------------------------------------------------------------------------

/**
 * Collects all processors which are predecessors (direct or indirect ones) for
 * processors marked as end-processors during graph traversing.
 */
class EndProcessorVisitor : public GraphVisitor {
public:
    EndProcessorVisitor();
    virtual ~EndProcessorVisitor();
    const std::set<Processor*>& getPredecessors() const;
    virtual bool visit(Processor* const processor);

private:
    std::set<Processor*> predecessors_;
    bool active_;
};

// ----------------------------------------------------------------------------

/**
 * Callback visitor which calls enableCaching() on all visited processors.
 */
class SetCachingGraphVisitor : public Call1ArgGraphVisitor<void, Processor, const bool>
{
public:
    SetCachingGraphVisitor(const bool activate, const PortTypeCheck& ptc);
    virtual ~SetCachingGraphVisitor() {}
    virtual bool visit(Processor* const processor);

private:
    const PortTypeCheck& portTypeCheck_;
};

// ----------------------------------------------------------------------------

class NetworkEvaluator {
public:
    /**
     * Wrapper around the process() calls. Can be used e.g. for benchmarking.
     */
    class ProcessWrapper  {
        friend class NetworkEvaluator;
    public:
        virtual ~ProcessWrapper() {}
    protected:
        virtual void beforeProcess(Processor* /*p*/) {}
        virtual void afterProcess(Processor* /*p*/) {}
        virtual void beforeNetworkProcess() {}
        virtual void afterNetworkProcess() {}
    };

public:

    /**
     * Constructor.
     *
     * @param sharedContext A canvas holding the OpenGL context that will be used
     *        for initializing the processors. More precisely, if a canvas has been passed
     *        <code>sharedContext->getGLFocus()</code> is called before <code>Processor::initialize()</code>
     *        in order to make sure that OpenGL initializations are performed
     *        within the correct OpenGL context.
     *        This is usually not necessary on single-context systems.
     */
    NetworkEvaluator(tgt::GLCanvas* sharedContext = 0);

    ~NetworkEvaluator();

    /**
     * Add a ProcessWrapper which is called before and after Processor::process() is called
     */
    void addProcessWrapper(ProcessWrapper* w);

    /**
     * Returns the processor network currently assigned to this evaluator.
     */
    const ProcessorNetwork* getProcessorNetwork() const;

    /**
     * Returns all render target currently used by the network.
     */
    const std::vector<RenderTarget*>& getCurrentRenderTargets() const;

    /**
     * Initializes all processors in the current ProcessorNetwork by calling
     * <code>initialize()</code> on each of them. If a shared context has been
     * passed to the constructor, it is used for the initialization. Otherwise,
     * the caller has to make sure that the correct context is present.
     *
     * @return  true if the initalization succeeds, false otherwise.
     */
    bool initialize() throw (VoreenException);

    /**
     * Returns whether the evaluator has been initialized with respect to the
     * current ProcessorNetwork.
     */
    bool isInitialized() const;

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
     * Performs all necessary updated when the connections between the processors
     * in the ProcessorNetwork are changed.
     */
    void onNetworkConnectionsChange();

    /**
     * Processes the currently assigned network. The rendering order is determined internally
     * according the network topology and the invalidation levels of the processors.
     */
    void process();

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
     * Removes the passed ProcessWrapper object, if it is currently registered.
     */
    void removeProcessWrapper(const ProcessWrapper* w);

    /**
     * Removes all registered ProcessWrapper objects. The objects are not freed.
     */
    void clearProcessWrappers();

    /**
     * Assigns the processor network to be evaluated.
     */
    void setProcessorNetwork(ProcessorNetwork* const network);

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


protected:
    /** Used for caching volume data via VolumeCache. */
    enum CacheState {
        CACHE_STATE_UNKNOWN,                // default value
        CACHE_STATE_NO_CACHING,             // processor does not cache data
        CACHE_STATE_ERROR_PROCESSOR_INVALID,// processor pointer is NULL
        CACHE_STATE_ERROR_STATEMAP_EMPTY,   // the StateMap is corrupted
        CACHE_STATE_ERROR_STATE_MISSING,    // the state for a particular processor is missing
        CACHE_STATE_INCOMPATIBLE_PROCESSOR, // the processor is not affected by that kind of cache
        CACHE_STATE_OK,                     // successful cache query
        CACHE_STATE_STATE_UPDATE_NEEDED,    // only the state for a particular processor needs to be updated
        CACHE_STATE_NEED_UPDATE             // the cache entry and processor state need to be updated
    };

    typedef std::map<Processor* const, std::string> StateMap;

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

private:
    /**
     * Assigns RenderTargets to ports dealing with image generation.
     */
    void assignRenderTargets();

    /**
     * Causes the member renderingOrder_ to be updated and defines a rendering order
     * for the processors in the current ProcessorNetwork according to the current
     * NetworkGraph.
     * This method will do nothing if netGraph_ is NULL. If netGraphTransposed_ is
     * also NULL, this member will be re-created from netGraph_.
     */
    void defineRenderingOrder();

    /**
     * Determines whether the given processor has data which are handle by the Volume-
     * Cache or not and whether they are up-to-date.
     *
     * @param   processor   Processor to be investigated.
     * @param   state   [output] The state from the VolumeCache for this processor
     * @return  true if the VolumeCache has determined, that this processor contains
     *          data concerning it and that they are not up-to-date or if its data do
     *          not affect the cache. False is returned, if the cache is affected by
     *          that processor but if its data are up-to-date.
     */
    bool needsProcessingForVolumeCache(Processor* const processor, CacheState& state);

    /**
     * Updates the VolumeCache for the given processor according to the passed state.
     *
     * @param   processor   Processor whose the cached entries shall be update.
     * @param   state   The state of that processor from the last query to
     *                  needsProcessorForVolumeCache().
     */
    void updateVolumeCache(Processor* const processor, const CacheState state);

    /**
     * Returns all processor from the current RenderingNetwork which return
     * true by calls to <code>isEndProcessor()</code> and thereby acts as
     * 'drains' in the network, i.e. CanvasRenderer.
     */
    const std::vector<Processor*> getEndProcessors() const;

    /** The network that is to be evaluated */
    const ProcessorNetwork* network_;

    /** Graph (directed) representing the entire network for performing operations on it. */
    NetworkGraph* netGraph_;

    /** The transposed version of the graph held in netGraph_. This means all edges
     * are reversed. */
    NetworkGraph* netGraphTransposed_;

    /** Vector containing all processors which can be rendererd. This vector is obtained
     * by sorting netGraph_ topolocial. */
    std::vector<Processor*> renderingOrder_;

    /** Map holding textual representations of the 'states' all processors in the network. This
     * is the settings of all their properties. */
    StateMap stateMap_;

    /** Cache used to cache volume data from processors which support caching. */
    VolumeCache volumeCache_;

    /** Vector holding all processor wrappers which might have been added. */
    std::vector<ProcessWrapper*> processWrappers_;

    /// Canvas holding the shared context that is to be used for processor initializations
    tgt::GLCanvas* sharedContext_;

private:
    /**
     * Determines the state for the given processor in the given cache by using
     * the cache and the stateMap_ member.
     * The return value is explained above in its respective comment.
     */
    CacheState getCacheState(Processor* const processor, const CacheBase& cache);

    /**
     * Injects the data from the volumeCache_ member, which are stored for the
     * given processor back into the processor. Thereby the cached data are
     * employed. It is vital to perform checks whether the data are up-to-date
     * to prevent unexpected results.
     * Furthermore the processor to which the data are injected MUST NOT DELETE
     * these data, as the cache will attempt to do so!
     *
     * @param   processor   The processor for which the cached data shall be
     *                      retrieved and used.
     * @return  true will be returned if the data injections succeeds, false
     *          otherwise or if the parameter processor is already NULL.
     */
    bool injectCachedVolumeData(Processor* const processor);

private:
    static const std::string loggerCat_;

    /**
     * If this member is set to true, the method <code>assignRenderTargets()</code>
     * will not attempt to assign the same target in multiple ports on processors
     * in the current ProcessorNetwork. This permits better debugging, but is normally
     * a greate waste of resources.
     */
    static bool reuseRenderTargets_;

    std::vector<RenderTarget*> renderTargets_;

    bool initialized_;

    bool locked_;
};

}   // namespace

#endif  // VRN_NETWORKEVALUATOR_H
