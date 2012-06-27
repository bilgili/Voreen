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

#ifndef VRN_NETWORKEVALUATOR_H
#define VRN_NETWORKEVALUATOR_H

#include "voreen/core/io/volumecache.h"
#include "voreen/core/vis/message.h"
#include "voreen/core/vis/exception.h"

#include <map>
#include <list>

namespace voreen {

class CanvasRenderer;
class Processor;
class Port;
class GeometryContainer;
class TextureContainer;
class PortData;
class PortMapping;
class LocalPortMapping;

/**
 * The evaluator is responsible to analyze and then evaluate (that means render) the processors
 * given to it. These processors are connected via ports, and the evaluator makes sure that all
 * inports are connected, that there are no loops and so on. It then maps the ports to targets
 * in the TextureContainer and makes that information available to every processor. The
 * processors are then rendered in the correct order. The processors are deleted when the
 * destructor is called.
 */
class NetworkEvaluator : public MessageReceiver {
public:
    /**
     * Wrapper around the Processor::process() calls. Can be used e.g. for benchmarking.
     */
    class ProcessWrapper : public MessageReceiver {
        friend class NetworkEvaluator;
    public:
        virtual ~ProcessWrapper() {}
    protected:
        virtual void beforeProcess(Processor* /*p*/) {}
        virtual void afterProcess(Processor* /*p*/) {}
    };


    /**
     * Default constructor, it's better to use the one with parameters
     */
    NetworkEvaluator();

    /**
     * Constructor
     *
     * @param processors The processors that are to be analyzed and evaluated
     */
    NetworkEvaluator(std::vector<Processor*> processors);

    /**
     * Destructor
     */
    ~NetworkEvaluator();

    /**
     * Analyses the network (consisting of processors in a vector), meaning they are checked for loops, if
     * all their inputs are connected etc. Their ports are mapped to targets in the TextureContainer,
     * and the order in which the processors are to be rendered is determined. If analyze() returns 0,
     * it means that all processors are ready to be evaluated (rendered)
     */
    int analyze();

    /**
     * Renders the network given by the processors in the processor_ vector. The targets in the TextureContainer
     * can be released as soon as it's not needed anymore.
     */
    int evaluate();

    /**
     * Evaluates the given network, but only until the given endProcessor is finished. This is mainly used
     * for coarseness, so this might be made protected in the future.
     * EDIT: this is not used at all anymore, but may come handy once we have multiple end nodes in the network
     */
    int evaluate(Processor* endProcessor);

    /**
     * Sets the processors to be analyzed and evaluated
     */
    void setProcessors(std::vector<Processor*> processors);

    /**
     * Gets the processors in this evaluator
     */
    std::vector<Processor*>& getProcessors();

    /**
     * Sets the GeometryContainer
     */
    void setGeometryContainer(GeometryContainer* const geoContainer);

    /**
     * Returns the GeometryContainer
     */
    GeometryContainer* getGeometryContainer();

    /**
     * Sets the TextureContainer
     */
    void setTextureContainer(TextureContainer* tc);

    /**
     * Returns the TextureContainer
     */
    TextureContainer* getTextureContainer();

    /**
     * Creates and initializes a new TextureContainer. The new TextureContainer is returned if
     * the initialization suceeds, otherwise 0 is returned.
     *
     * @param finalTarget the number of the final rendering target, that is simultaneously the number of usable render targets
     * @return the created texture container or 0 if initialization fails
     */
    TextureContainer* initTextureContainer(int finalTarget);

    /**
     * Return the CanvasRenderer in the network, if any.
     */
    CanvasRenderer* getCanvasRenderer() const { return canvas_; }
    
    /**
     * Invalidates the cached result of all processors that are used in caching. That
     *
     */
    void invalidateRendering();

    /**
     * Calls initializeGL in all the processors in the network.
     */
    int initializeGL();

    /**
     * Sets the size for all processors in the network. This method is called when the size of
     * the canvas has changed.
     *
     * @param size new size of the canvas
     */
    void setSize(const tgt::ivec2& size);

    /**
     * Accepts
     *
     * "evaluate" -> evaluates the network, but only until the given processor is finished (not used atm)
     *
     * setReuseTextureContainerTargets_ -> sets if TextureContainerTargets are reused or not when they aren't needed anymore
     *
     * processorInvalidated_ -> updates the caching status of all successors for the ivalidated processor
     *
     * removeForbiddenTargets_ -> removes targets from forbiddenTargets_ vector
     *
     * addForbiddenTargets_ -> adds targets to forbiddenTargets_ vector
     *
     */
    void processMessage(Message* msg, const Identifier& dest);

    /**
     * Returns if the current network is valid and can be rendered.
     */
    bool isValid();

    /**
     * Returns the mapped texture container target for the given port.
     * If no texture container target is mapped, -1 is returned. This is used for the preview
     * in rptgui when selecting arrows.
     */
    int getTextureContainerTarget(Port* p,int pos = 0) throw (VoreenException);

    std::map<Processor*, int> getPriorityMap();

    /**
     * Add a ProcessWrapper which is called before and after Processor::process() is called
     */
    void addProcessWrapper(ProcessWrapper* w);

    /**
     * Removes and frees all registered ProcessWraper objects
     */
    void removeProcessWrappers();

    /**
     * Removes given processor from processor-vector.
     */
    void removeProcessor(Processor* processor);

    /**
     * At first all CoarsenessRenderer in the network are collected. After that the coarsenessFactor
     * is calculated and for every processor that is connected to a CoarsenessRenderer the new size is
     * set.
     *
     * @param active true when coarseness is on, false otherwise
     */
    void switchCoarseness(bool active);

    void setCamera(tgt::Camera* camera) { camera_ = camera; }
    
    static const Identifier setReuseTextureContainerTargets_;
    static const Identifier processorInvalidated_;
    static const Identifier addForbiddenTargets_;
    static const Identifier removeForbiddenTargets_;

protected:
    /**
     * This struct holds information about one port. It stores the ports that are connected to it.
     * This struct is used in order to free the target in the TextureContainer as soon as all
     * processors connected to this port are finished rendering. PortEntries are only created for outports of course.
     */
    struct PortEntry {
        Port* port;
        std::vector<Port*> connectedPorts;
    };

    struct ForbiddenTextureTargets {
        TextureContainer* tc;
        std::vector<int> forbiddenTargets;
    };

    typedef TemplateMessage<ForbiddenTextureTargets> ForbiddenTargetsMsg;

    /**
     * Adds rendertargets that this evaluator is forbidden to use.
     */
    void addForbiddenTargets(ForbiddenTextureTargets forbidden);

    /**
     * Adds rendertargets that this evaluator is forbidden to use.
     */
    void removeForbiddenTargets(ForbiddenTextureTargets forbidden);

    /**
     * Analyzes which render results might be cached during the render process and creates
     * 2 maps that store the cachingstatus of a processor and all successors for every processor
     * whose render results are cached.
     */
    void initializeCaching();

    /**
     * Tests whether the given port is forward connected to an endprocessor or cacherenderer and
     * no processor that is not cacheable lies between it. If the canvas is the found endprocessor
     * the caching status of the canvas is returned.
     *
     * @param p port for which is tested whether it is connected to an endprocessor or cacherenderer
     * @return true when port is connected to an endprocessor or cacherenderer, false otherwise
     */
    bool isConnectedToEndProcessorOrCacheRenderer(Port* p) const;

    /**
     * Tests whether the given port is lying after a processor that is not cacheable.
     * The processor that belongs to the port is not able to cache the results in that case.
     *
     * @param p port for which is tested whether it is lying after a not cacheable processor
     * @return true when port is lying after a processor that is not cacheable, false otherwise
     */
    bool isAfterNotCacheableProcessor(Port* p) const;

    /**
     * Collects all outports and coprocessoroutports for the given processor and returns them.
     *
     * @param processor processor whose connected outports are returned
     * @return vector with all outports and coprocessoroutports that are connected to another port
     */
    std::vector<Port*> getConnectedOutports(const Processor* processor) const;

    /**
     * Collects all inports and coprocessorinports for the given processor and returns them.
     *
     * @param processor processor whose connected inports are returned
     * @return vector with all inports and coprocessorinports that are connected to another port
     */
    std::vector<Port*> getConnectedInports(const Processor* processor) const;

    /**
     * Iterates forward over the network and collects all processors that are visited
     * till an endprocessor is found starting at the given processor.
     *
     * @param processor processor whose successors are collected
     * @return vector with all visited processors starting at the given processor
     */
    std::vector<Processor*> getConnectedForward(Processor* processor) const;

    /**
     * Recursive method for iteration over the network. It stores all visited processors in
     * the parameter result. The recursion stops when the given processor is an endprocessor.
     *
     * @param processor the current visited processor, i.e. starting point for new recursion
     * @param result vector with all visited processors
     */
    void getConnectedForwardRecursive(Processor* processor, std::vector<Processor*>& result) const;

    /**
     * Recursive method for iteration over the network. It stores all visited processors in
     * the parameter result. The recursion stops when the given processor has no outports.
     *
     * @param processor the current visited processor, i.e. starting point for new recursion
     * @param result vector with all visited processors
     */
    void getConnectedBackwardRecursive(Processor* processor, std::vector<Processor*>& result) const;

    /**
     * Updates the caching status for the given processor itself and for all successors.
     * If the processor is not involved in caching, nothing will happen
     *
     * @param processor processor that is invalidated
     */
    void updateCachingStatus(Processor* processor);

    /**
     * Clears the localPortMap_ member and deletes the pointers to the
     * LocalPortMap which is assigned in order to prevent memory leaks.
     */
    void clearLocalPortMap();

    /**
     * Does the port mapping, meaning every port is given a target in the TextureContainer. No longer
     * needed targets can be reused as soon as possible.
     */
    void doPortMapping();

    /**
     * Creates a new PortEntry in the portEntries vector. Look at the portEntry comment for more information
     */
    void newPortEntry(Port* p);

    /**
     * Recursively traverses the processor network and assigns each processor a priority depending on its
     * maximum (NOT minimum) distance(amount of processors) to the last processor in the network.
     * @param processor The last processor in the network
     * @param priority The priority the last processor should have (default 0)
     */
    void assignPriority(Processor* processor,int priority=0);

    /**
     * Sorts the processors in the processors_ vector by the priority assigned to them by the
     * assignPriority() function
     */
    void sortProcessorsByPriority();

    /**
     * Checks if processor is in a loop in the network
     */
    int checkForLoops(Processor* processor,std::vector<Processor*>& processorVector);

    /**
     * Checks if the given vector contains the given processor
     */
    bool checkIfVectorContainsProcessor(Processor* processor, std::vector<Processor*>& processorVector);

    /**
     * Searches for processors in the processors_ vector that are endprocessors.
     *
     * @return vector with all endprocessors in the network
     */
    std::vector<Processor*> findEndProcessors();

    /**
     * Stores the texture targets that are used by processors that are involved in caching in the given
     * list.
     */
    void addRenderTargetsUsedInCaching();

    /**
     * Indicates whether the processors are ready to be evaluated (rendered) or not. This is set to true
     * if analyze() returns 0.
     */
    bool readyToEvaluate_;

    /**
     * The TextureContainer. It must be the same tc as in the processors.
     */
    TextureContainer* tc_;

    /**
     * GeometryContainer. Must be the same geometry container as used by processors.
     */
    GeometryContainer* geoContainer_;

    /**
     * The Processors to be evaluated by this Evaluator
     */
    std::vector<Processor*> processors_;

    /**
     * If this is set to true, the evaluator checks if a target can be used again once its
     * content is no longer used and allocates it to the next processor. If false, every outport
     * of every processor gets its own target in the texturecontainer. This is especially useful
     * if you want to check/debug your network.
     */
    bool reuseTextureContainerTargets_;

    /**
     * A map containing each processor together with its priority. The priority indicates in which order
     * the processors are rendered.
     */
    std::map<Processor*, int> priorityMap_;

    /**
     * This vector contains a portEntry for every outport in the entire network. Each time a
     * processor is finished, all ports of that processor are deleted in every
     * portEntry->connectedPorts. If a portEntry->connectedPorts->size() gets 0, the
     * TextureContainer target associated with that port can be released.
     */
    std::vector<PortEntry*> portEntries_;

    /**
     * A map containing every port together with its PortData. This map is given
     * to every processor as soon as they are rendered.
     */
    std::map<Port*, std::vector<PortData*> > portMap_;

    std::map<Processor*, LocalPortMapping*> localPortMap_;

    /**
     * temporary vector with targets of the texturecontainer that are used
     * in rednering
     */
    std::vector<int> tempForbiddenTargets_;

    /**
     * Targets not allowed to be used, because other evaluators
     * already used them.
     */
    std::vector<int> forbiddenTargets_;

    static const std::string loggerCat_; ///< the logger category

    /**
     * In this vector we save the current viewportsize so we need to call glViewport only
     * if the arguments have changed.
     */
    tgt::ivec2 currentViewportSize_;

    bool coarsenessActive_; ///< indicates whether coarsenessMode is active or not

    CanvasRenderer* canvas_; ///< the canvasRenderer in the network
    bool canvasCaches_; ///< value of the useCaching-Property in the canvasRenderer. It is updated in every renderpass

    /**
     * Map that stores whether the render result of a processor is valid and thus itmust not
     * render again.
     */
    std::map<Processor*, bool> cachedProcessorsStatus_;

    /**
     * Map that stores the successors for all processors that are involved in caching.
     * It is used when a processor is invalidated. In that case all successors must also be
     * invalidated.
     */
    std::map<Processor*, std::vector<Processor*> > successorMap_;

    /**
     * Wrapper around the Processor::process() calls to check that OpenGL state conforms to
     * default settings. Log a warning message if not.
     */
    class CheckOpenGLStateProcessWrapper : public ProcessWrapper {
    protected:
        void afterProcess(Processor* p);
        void warn(Processor* p, const std::string& message);
    };

    std::vector<ProcessWrapper*> processWrappers_;
    PortMapping* portMapping_;  /** used by doPortMapping. placed as a member in order to prevent memory leaks. */

    ///////////////////////////////////////////////////////////////////////////
    //
    // TODO: UNDER CONSTRUCTION - caching
    //
    enum CacheState {
        CACHE_STATE_UNKNOWN,    // default value
        CACHE_STATE_ERROR_PROCESSOR_INVALID,    // processor pointer is NULL
        CACHE_STATE_ERROR_STATEMAP_EMPTY,   // the StateMap is corrupted
        CACHE_STATE_ERROR_STATE_MISSING,  // the state for a particular processor is missing
        CACHE_STATE_INCOMPATIBLE_PROCESSOR, // the processor is not affected by that kind of cache
        CACHE_STATE_OK, // successful cache query
        CACHE_STATE_STATE_UPDATE_NEEDED,  // only the state for a particular processor needs to be updated
        CACHE_STATE_NEED_UPDATE // the cache entry and processor state need to be updated
    };

    typedef std::map<Processor* const, std::string> StateMap;
    StateMap stateMap_;
    VolumeCache volumeCache_;

    CacheState handleVolumeCaching(Processor* const processor);
    bool remapVolumeHandlePortMapping(Processor* const processor);
    void updateStateMap(Processor** const processorAddr);
    //
    ///////////////////////////////////////////////////////////////////////////

    tgt::Camera* camera_; //TODO: temporary place for the camera, remove sometime. joerg
};

} // namespace voreen

#endif //VRN_NETWORKEVALUATOR_H
