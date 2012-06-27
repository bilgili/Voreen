/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/message.h"
#include "voreen/core/vis/exception.h"

#include <map>
#include <list>

namespace voreen {

class Processor;
class Port;
class GeometryContainer;
class TextureContainer;
class PortData;
class LocalPortMapping;

/** 
 * The evaluator is responsible to analyse and then evaluate (that means render) the processors
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
     * @param processors The processors that are to be analyzed and evaluated
     */
	NetworkEvaluator(std::vector<Processor*> processors);

    /**
     * Destructor - deletes all processors
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

    // TODO d_kirs04
    // void setNetwork(ProcessorNetwork net) { setProcessors(net.processors) }

	/**
	 * Gets the processors in this evaluator
	 */
	std::vector<Processor*>& getProcessors();
	
    /**
     * Sets the GeometryContainer
     */
    void setGeometryContainer(GeometryContainer* const geoContainer);

    /**
     * Gets the GeometryContainer
     */
    GeometryContainer* getGeometryContainer();

	/**
	 * Sets the TextureContainer
	 */
	void setTextureContainer(TextureContainer* tc);
	
	/**
	 * Gets the TextureContainer
	 */
	TextureContainer* getTextureContainer();

    /**
     * Initializes a texture container, sets it at the currently active one
     * and returns it.
     * \param finalTarget The final rendering target
     * \return The created texture container
     */
    TextureContainer* initTextureContainer(int finalTarget);
	
	/** 
	 * if an invalidate is called, the entire network is evaluated (rendered) again
	 */
	void invalidate();

    /** 
	 * Invalidates all processors in the network.
	 */
	void invalidateRendering();

	/**
	 * Calls initializeGL in all the processors in the network. Not sure if we really need this, but we think
	 * so.
	 */ 
	int initializeGL();

	/** 
	 * Sets the size for all processors in the network. This method is called when the size of
     * the canvas has changed.
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
	 * setCachedBackward_ -> Tells every processor before the given one that their render result is cached, and they don't need
	 * to render unless that changes
	 *
	 * unsetCachedForward_ -> Tells every processor after the given one that an invalidate was called and that they have to render
	 */
	void processMessage(Message* msg, const Identifier& dest);
	
	/**
	 * Calls init for all processors in the network. Not sure if we need this.
	 */ 
	void init();

	void deinit();

	/**
	 * Returns if the current network is valid and can be rendered.
	 */
	bool isValid();

	/**
	 * Gets the rendertargets this evaluator forbids other
	 * evaluators to use.
	 */
	std::vector<int> getNewForbiddenTargets();

	/**
	 * Adds rendertargets that this evaluator is forbidden to use.
	 */
	void addForbiddenTargets(NetworkEvaluator* eval, std::vector<int> forbiddenTargets);

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
     * This method is called when a processor in VoreenVe was deleted.
     */
    void removeProcessor(Processor* processor);
    
	static const Identifier setReuseTextureContainerTargets_;
	static const Identifier setCachedBackward_;
	static const Identifier unsetCachedForward_;
	static const Identifier addForbiddenTargets_;

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
    
	/**
	 * Tells every processor before the given one that their render result is cached, and they don't need
	 * to render unless that changes
	 */
	void updateCaching(Processor* processor, bool cached);

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
     * Searches for processors in the processor_ vector that don't have an outport, meaning they are last.
     */
    std::vector<Processor*> findEndProcessorsInNetwork();

	/**
     * At first all CoarsenessRenderer in the network are collected. After that the coarsenessFactor
     * is calculated and for every processor that is connected to a CoarsenessRenderer setSizeBackwards()
     * is called.
     *
     * @param active true when coarseness is on, false otherwise
	 */
	void switchCoarseness(bool active);

	/**
	 * Analyses which render results might be cached during the render process (which processors are connected to
	 * a CacheRenderer or a CanvasRenderer) and sets them cachable
	 */
	void initializeCaching();

	/**
	 * Iterates through all inports of the given processor and tells the connected ports that their render result
	 * might be cached
	 */
	void setCachingOfInports(Processor* processor);

	/**
	 * Checks if there are cachrenderers somewhere and iterates through its inports
	 * to find every port that might be used in caching. The rendertargets of those
	 * ports are then added to the given targetList. That list is later used to add these targets 
	 * to the newForbiddenTargets_ vector. (So that other evaluator don't
	 * override them) 
	 */
	std::list<int> getRenderTargetsUsedInCaching(std::list<int> targetList);

    /**
     * This method recursively searches all processors that are connected to given processor.
     * All these processors are stored in a list. The third parameter indicates whether the search goes
     * forward or backward.
     *
     * @param proc
     * @param processors list of visited processors
     * @param if true the network is traversed backwards from given processor, forward otherwise
     */
    void getConnectedProcessors(Processor* proc, std::list<Processor*>& processors, bool backward);
    
    /**
     * Wrapper around the Processor::process() calls to check that OpenGL state conforms to
     * default settings. Log a warning message if not.
     */
    class CheckOpenGLStateProcessWrapper : public ProcessWrapper {
    protected:
        void afterProcess(Processor* p);
        void warn(Processor* p, const std::string& message);
    };

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
	std::map<Processor*,int> priorityMap_;

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

    struct ForbiddenTargetsOfEvaluator {
        NetworkEvaluator* evaluator;
        std::vector<int> forbiddenTargets;
    };

    typedef TemplateMessage<ForbiddenTargetsOfEvaluator> ForbiddenTargetsMsg;
    
	/**
	 * The evaluator distributes rendertargets to the processors,
	 * and some of these rendertargets aren't allowed to be overwritten.
	 * That could however happen if another evaluator distributes these
	 * targets again. Therefore you can get those targets from this vector.
	 */
	std::vector<int> newForbiddenTargets_;

	/**
	 * Just a temporary version of the vector newForbiddenTargets. Handy because
	 * you can delete duplicate entrys from a list really easy.
	 */
	std::list<int> tempForbiddenTargets_;

	/**
	 * Targets not allowed to be distributed, because other evaluators
	 * already used them. The evaluator that forbid those targets is 
	 * also in the map because if that evaluator loads another network,
	 * the forbidden targets might change. If that happens, we have to
	 * remove previously forbidden targets from that evaluator. 
	 */
	std::map<NetworkEvaluator*, std::vector<int> > forbiddenTargetsFromOtherEvaluators_;
    
    std::vector<ProcessWrapper*> processWrappers_;
    
    static const std::string loggerCat_;

    /**
     * In this vector we save the current viewportsize so we need to call glViewport only
     * if the arguments have changed.
     */
    tgt::ivec2 currentViewportSize_;
  
       
    bool coarsenessActive_; ///< indicates whether coarsenessMode is active or not

};	

} // namespace voreen

#endif //VRN_NETWORKEVALUATOR_H
