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

#ifndef VRN_RPTNETWORKEVALUATOR_H
#define VRN_RPTNETWORKEVALUATOR_H


#include <map>

#include "voreen/core/vis/message.h"
#include "voreen/core/vis/exception.h"


namespace voreen {

class Processor;
class Port;
class GeometryContainer;
class TextureContainer;
class VolumeContainer;
class PortData;
class LocalPortMapping;

/**
* This struct holds information about one port. It stores the ports that are connected to it. 
* This struct is used in order to free the target in the TextureContainer as soon as all 
* processors connected to this port are finished rendering. PortEntries are only created for outports of course. 
*/
struct portEntry {
		Port* port;
		std::vector<Port*> connectedPorts;
	};

/** 
* The evaluator is responsible to anaylse and then evaluate(that means render) the processors given to it.
* These processors are connected via ports, and the evaluator makes sure that all inports are connected, that
* there are no loops and so on. It then maps the ports to targets in the TextureContainer and makes that information
* available to every processor. The processors are then rendered in the correct order. 
* The processors are deleted when the destructor is called.
*/

class NetworkEvaluator : public MessageReceiver {
public:
	/**
	* Default constructor, it's better to use the one with parameters
	*/ 
	NetworkEvaluator();

    /**
     * Destructor - deletes all processors
     */
    ~NetworkEvaluator();

	/**
	* Constructor
	* @param processors The processors that are to be analyzed and evaluated
	*/
	NetworkEvaluator(std::vector<Processor*> processors);

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
	
    /** Sets the GeometryContainer
     */
    void setGeometryContainer(GeometryContainer* const geoContainer);

    /** Gets the GeometryContainer
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
	* Sets the size for all processors in the network. Not sure if we handle this correctly yet. 
	*/ 
	void setSize(const tgt::ivec2& size);

	/**
	* Accepts
	* "set.size.backwards" -> The evaluator reads the size from the given processor, and sets the size
	* of all processors connected to one of its inports to the exact same size. This is used by coarseness
	* processors to set the size of their predecessing processors to full size again. 
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
	void addForbiddenTargets(NetworkEvaluator* eval,std::vector<int> forbiddenTargets);

	/**
	* Returns the mapped texture container target for the given port.
	* If no texture container target is mapped, -1 is returned. This is used for the preview
	* in rptgui when selecting arrows.
	*/
	int getTextureContainerTarget(Port* p,int pos=0) throw (VoreenException);

	std::map<Processor*,int> getPriorityMap();

	void setVolumeContainer(VolumeContainer* volumeContainer);
	
	static const Identifier setReuseTextureContainerTargets_;
	static const Identifier setSizeBackward_;
	static const Identifier setCachedBackward_;
	static const Identifier unsetCachedForward_;
	static const Identifier unsetCachedBackward_;
	static const Identifier addForbiddenTargets_;

protected:
	
	/**
	* Called by the processMessage function if the message "set.size.backwards" arrives. All processors connected
	* to any inport of the given processor are set to the same size as the given processors. This is used by coarseness
	* processors to set the size of their predecessing processors to full size again.
	*/
	void setSizeBackwards(Processor* processor,float factor);

	/**
	* Tells every processor before the given one that their render result is cached, and they don't need
	* to render unless that changes
	*/
	void setCachedBackward(Processor* processor);

	/**
	* Tells every processor after the given one that an invalidate was called and that they have to render
	*/
	void unsetCachedForward(Processor* processor);

	/**
	* Tells every processor before the given one that an invalidate was called and that they have to render
	*/
	void unsetCachedBackward(Processor* processor, std::list<Processor*> visited);
	
	/**
	* Does the port mapping, meaning every port is given a target in the TextureContainer. No longer
	* needed targets can be reused as soon as possible. 
	*/
    void doPortMapping(); // throw(VoreenException); // FIXME: Linux g++ says doPortMapping throws other exceptions

	/**
	* Creates a new portEntry in the portEntries vector. Look at the portEntry comment for more information
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
    * Identify parts of the network that need multiple passes when working with large splitted datasets.
    * Note: For now this is a simplified version that can only cope with one dataset and raycaster!
    */
    void identifyMultipassProcessors();

	/**
	*Checks if processor is in a loop in the network
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
	* Checks if some parts of the network are to be rendered coarse, and if yes, makes the neccessary size
	* adjustements to the processors. That means they are made smaller by a factor defined in a coarseness processor.
	* This coarseness processor is also responsible for resizing the processors to their original size afterwards. This
	* can be done by sending a special message to the evaluator. See process message comments for more information
	*/ 
	void prepareCoarsenessRenderers();

	/**
	 * Tells all processors connected to inports of this processor to render coarse, meaning their setSize function
	 * is being called. This function is called in the checkForCoarsenessRenderers() function. 
	 */
	void setCoarseness(Processor* processor, float factor);

	/**
	* Analyses which render results might be cached during the render process (which processors are connected to
	* CacheProcessors) and sets them cachable
	*/
	void initializeCaching();

	/**
	* Iterates through all inports of the given processor and tells the connected ports that their render result
	* might be cached
	*/
	void iterateThroughInportsAndSetCaching(Processor* processor);

	/**
	* Checks if there are cachrenderers somewhere and iterates through its inports
	* to find every port that might be used in caching. The rendertargets of those
	* ports are then added to the given targetList. That list is later used to add these targets 
	* to the newForbiddenTargets_ vector. (So that other evaluator don't
	* override them) 
	*/
	std::list<int> getRenderTargetsUsedInCaching(std::list<int> targetList);

    
    /**
     * Thrown when OpenGL state does not conform to default settings
     */
    class OpenGLStateException : public VoreenException {
    public:
        OpenGLStateException(std::string what = "") : VoreenException("invalid OpenGL state: " + what) {}
    };

    /**
     * Check that OpenGL state conforms to default settings and throw an exception if not
     */
    void checkOpenGLState() throw (OpenGLStateException);
    
	/**
	* Indicates whether the processors are ready to be evaluated (rendered) or not. This is set to true
	* if analyze() returns 0. 
	*/
	bool readyToEvaluate_;
	
	/**
	* The TextureContainer. It must be the same tc as in the processors.
	*/
	TextureContainer* tc_;

    /** GeometryContainer. Must be the same geometry container as used by processors.
    */
    GeometryContainer* geoContainer_;

	/**
	* The Processors to be evaluated by this Evaluator
	*/
	std::vector<Processor*> processors_;

	/**
	* Alot of tasks in the evaluator require it to traverse the complete network from the end to the
	* beginning. Therefor alot of renderes might be visited multiple times in a worst case scenario.
	* To prevent that, the processors are put into this list when they are visited, and if they are visited again,
	* the function (whatever it may be) can return, as this processor was already visited. 
	*/
	std::list<Processor*> visited_;
	std::list<Processor*> visited2_;
	
	/**
	* If this is set to true, the evaluator checks if a target can be used again once its content is no longer used and 
	* allocates it to the next processor. If false, every outport of every processor gets its own target in the texturecontainer. This
	* is especially useful if you want to check/debug your network.
	*/ 
	bool reuseTextureContainerTargets_;
	
	/**
	* A map containing each processor together with its priority. The priority indicates in which order
	* the processors are rendered. 
	*/
	std::map<Processor*,int> priorityMap_;

	/**
	* This vector contains a portEntry for every outport in the entire network. Each time a processor
	* is finished, all ports of that processor are deleted in every portEntry->connectedPorts. If a portEntry->connectedPorts->size()
	* gets 0, the TextureContainer target associated with that port can be released. 
	*/ 
	std::vector<portEntry*> portEntries_;
	
	/**
	* A map containing every port together with its PortData. This map is given
	* to every processor as soon as they are rendered.
	*/
	std::map<Port*,std::vector<PortData*> > portMap_;

	std::map<Processor*,LocalPortMapping*> localPortMap_;

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
	std::map<NetworkEvaluator*,std::vector<int> > forbiddenTargetsFromOtherEvaluators_;

    static const std::string loggerCat_;

};	

struct ForbiddenTargetsOfEvaluator {
	NetworkEvaluator* evaluator;
	std::vector<int> forbiddenTargets;
};

typedef TemplateMessage<ForbiddenTargetsOfEvaluator> ForbiddenTargetsMsg;


} //namespace voreen 



#endif
