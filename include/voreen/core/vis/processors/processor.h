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

#include "tgt/camera.h"
#include "tgt/shadermanager.h"

#include "voreen/core/opengl/texunitmapper.h"
#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/message.h"
#include "voreen/core/xml/serializable.h"
#include "voreen/core/vis/processors/port.h"
#include "voreen/core/vis/processors/portmapping.h"
#include "voreen/core/vis/properties/allproperties.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/volume/volumehandlevalidator.h"

namespace voreen {

class GeometryContainer;
class LocalPortMapping;
class ProxyGeometry;
class TextureContainer;

/**
 * The base class for all processor classes used in Voreen.
 */
class Processor : public MessageReceiver, public Serializable {
public:
    enum Status {
        VRN_OK                  = 0x200,
        VRN_ERROR               = 0x202
    };

    // One side of a connection between two processors. Needed for serialization.
    struct ConnectionSide {
        int processorId;
        std::string portId;
        int order;
    };

    // Needed to be able to sort connections based on the order of the outgoing side.
    struct ConnectionCompare {
        bool operator() ( std::pair< ConnectionSide, ConnectionSide > l,
                          std::pair< ConnectionSide, ConnectionSide > r) {
            // connect outgoing connections with lower order prior to those with higher order
            return (l.second.order<r.second.order);
        }
    };

    // Needed for Serialization.
    // Note: The order for the first port is not really needed.
    typedef std::vector< std::pair< ConnectionSide, ConnectionSide > > ConnectionMap;

    Processor();

    virtual ~Processor();

    virtual const Identifier getClassName() const = 0;
    static const Identifier getClassName(TiXmlElement* processorElem);

    virtual Processor* create() const = 0;

    virtual Processor* clone() const //= 0;
    // TODO: remove this implementation as soon as all Processors have a clone()-method
    {
        return create();
    }

    /**
     * Should be called after OpenGL is initialized, initializes the processor.
     *
     * @return Returns an error code. VRN_OK means everything ok.
     */
    virtual int initializeGL();

    /// Returns the error code which occured during \a initializeGL().
    int getInitStatus() const;

    /// This method is called when the processor should be processed.
    virtual void process(LocalPortMapping* portMapping) = 0;

    /**
     * CoProcessors add a function pointer to their CoprocessorOutports. This
     * function pointer usually points to this function, which is overwritten in the
     * CoProcessors.
     */
    virtual Message* call(Identifier ident, LocalPortMapping* portMapping);

    /**
	 * @deprecated: Messaging will be removed.
     */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

    /// Set a name for this processor
    void setName(const std::string& name);

    /// Returns the name of this processor.
    const std::string& getName() const;

    // Returns processor information
    virtual const std::string getProcessorInfo() const;

    /// Marks that the processor needs to be updated
    virtual void invalidate();

    /// Adds a property to this processor
    virtual void addProperty(Property* prop);

    /// Returns the properties.
    virtual const Properties& getProperties() const;

    /**
     * Returns the inports of this processor.
     */
    virtual const std::vector<Port*>& getInports() const;

    /**
    * Returns the outports of this processor.
    */
    virtual const std::vector<Port*>& getOutports() const;

    /**
    * Returns the coprocessor inports of this processor.
    */
    virtual const std::vector<Port*>& getCoProcessorInports() const;

    /**
    * Returns the coprocessor outports of this processor.
    */
    virtual const std::vector<Port*>& getCoProcessorOutports() const;

    /**
    * Returns the private ports of this processor.
    */
    virtual const std::vector<Port*>& getPrivatePorts() const;

    bool hasPortOfCertainType(int portTypeMask) const;

    /**
    * Returns the port that contains (or better that is represented by) the given Identifier.
    */
    virtual Port* getPort(Identifier ident);

    /**
     * Creates a new inport for this processor.
     *
     * @param type The identifier by which this port is adressed in the processor. Something
     * like "image.entrypoints" for example.
     * @param allowMultipleConnections Can multiple ports be connected to this processor? This
     * should usually be false
     */
    virtual void createInport(Identifier type, bool allowMultipleConnections = false);

    /**
     * Creates a new outport for this processor.
     *
     * @param type The identifier by which this port is adressed in the processor. Something
     *             like "image.entrypoints" for example.
     * @param isPersistent Should the render result carried by this port be persistent in the TC?
     * @param inportIdent Some processors need to render to the same rendertarget they got their
     *                    input from. If this is neccessary, inportIdent holds the Identifier of
     *                    that inport.
     */
    virtual void createOutport(Identifier type, bool isPersistent = false, 
        Identifier inportIdent = "dummy.port.unused");

    /**
     * Contains an outport which actually contains the passed data. The data will be deleted
     * together with the port. This method is a generalization of <code>createOutport()</code>
     * and is used for the VolumeSet / VolumeHandle concept enabling to pass pointers through
     * the network.
     *
     * @param   type    Identifies determining what kind of port the created port shall become
     *                  (e.g. "volumehandle.input")
     * @param   data    the data which become mapped to the port mapping and are acutally passed
     *                  to the </code>process()</code> methods of connected processors.
     * @param   isPersistend    just like in <code>createOutport()</code>
     * @param   inportIdent    just like in <code>createOutport()</code>
     */
    template<typename T>
    void createGenericOutport(Identifier type, const T data, const bool isPersistent = false,
                              const Identifier& inportIdent = "dummy.port.unused")
    {
        Port* outport = new GenericOutPort<T>(type, data, this, isPersistent);
        outports_.push_back(outport);
        portMap_.insert(std::make_pair(type, outport));
        if (inportIdent != "dummy.port.unused") {
            if (getPort(inportIdent) != 0)
                outportToInportMap_.insert(std::pair<Port*, Port*>(outport, getPort(inportIdent)));
            else
                LWARNING("Couldn't find the inport in Processor::createGenericOutport())");
        }
    }

    /**
     * Creates a new CoProcessorInport for this processor.
     *
     * @param type The identifier by which this port is adressed in the processor. Something
     * like "image.entrypoints" for example.
     * @param allowMultipleConnections Can multiple ports be connected to this processor? This
     * should usually be false
     */
    virtual void createCoProcessorInport(Identifier type, bool allowMultipleConnections = false);

    /**
     * Creates a new outport for this processor.
     *
     * @param type The identifier by which this port is adressed in the processor. Something
     *             like "image.entrypoints" for example.
     * @param function CoProcessors add a functionpointer to one of their functions (usually the
     *                 "call" function). This is that pointer. Use "&Renderer::call" as that
     *                 pointer if you are unsure
     * @param allowMultipleConnections Can multiple ports be connected to this processor?
     *        Default is true for outports
    */
    virtual void createCoProcessorOutport(Identifier type, FunctionPointer function = 0,
                                          bool allowMultipleConnections = true);

    /**
     * Creates a new private port for this processor. Private ports are ports that get a render
     * target, but that target isn't connected to any other processors. (That port doesn't show
     * up in network editor). The combiner for example needs two temporary TC targets to work correctly,
     * and this is how it gets them. Just create two privatePorts and render your temporary
     * render results to those targets. Private ports are always persistent.
     *
     * @param type The identifier by which this port is adressed in the processor. Something
     * like "image.entrypoints" for example.
     */
    virtual void createPrivatePort(Identifier type);

    /**
     * Connects one outport of this processor with one inport of another processor. You can use
     * this function to manually build processor networks, but the easier way is to use network editor.
     *
     * @param outport The outport of this processor that is to be connected
     * @param inport The inport of the target processor to which the outport should connect.
     */
    virtual bool connect(Port* outport, Port* inport);

    /**
     * Disconnectes the outport of this processor from the inport from another processor
     */
    virtual bool disconnect(Port* outport, Port* inport);

    /**
     * Tests if the outport of this processor can be connected to the inport of the other processor
     */
    virtual bool testConnect(Port* outport, Port* inport);


    /**
     * Returns the name of the xml element used when serializing the object
     */
    virtual std::string getXmlElementName() const;

    /**
     * Overridden Serializable methods - serialize and deserialize everything except connections
     */
    virtual TiXmlElement* serializeToXml() const;
    virtual void updateFromXml(TiXmlElement* processorElem);

    /**
     * serializes the Processor to xml
     */
    TiXmlElement* serializeToXml(const std::map<Processor*, int> idMap) const;

    /**
     * Updates the processor xml and returns connection info so that
     * processors can be connected, after all have been read, in the form
     * (id, ConnectionMap)
     */
    std::pair<int, ConnectionMap> getMapAndUpdateFromXml(TiXmlElement* processorElem);

    /**
     * Serialization for metadata
     */
    void addToMeta(TiXmlElement* elem);
    void removeFromMeta(std::string elemName);
    void clearMeta();
    TiXmlElement* getFromMeta(std::string elemName) const;
    std::vector<TiXmlElement*> getAllFromMeta() const;
    bool hasInMeta(std::string elemName) const;

    /**
     * Checks whether this processor is a coprocessor or not
     */
    bool getIsCoprocessor() const;

    /**
     * Sets whether this processor is a coprocessor or not. If you want to create a coprocessor
     * you have to use this function in the constructor of your processor.
     */
    void setIsCoprocessor(bool b);

    /**
     * Returns whether the processor supports caching of it's results  or not.
     *
     * @return true when the processor supports caching, false otherwise
     */
    bool getCacheable() const;

    /**
     * Sets if the results of this processor can be cachend, e.g. whether this processor
     * supports caching or not.
     *
     * @param b is caching supported by this processor?
     */
    void setCacheable(bool b);

    // FIXME: should be temporary and somehow combined with exisiting cachabel attribute (df)
    /**
     * Determines whether the caching of VolumeHandles on this processor's outports shall be used. 
     * Note that only processors with outports passing VolumeHandle* are concerned of this 
     * setting. The setting has no effect if no VolumeHandle* outports exist on this processor.
     */
    void enableVolumeCaching(bool enable) { useVolumeCaching_ = enable; }
    
    // FIXME: should be temporary and somehow combined with exisiting cachabel attribute (df)
    /**
     * Indicates whether ports passing VolumeHandle* of this processor make use of caching or
     * not. Note that return value is irrelevant for processors not passing VolumeHandle*.
     */
    bool usesVolumeCaching() const { return useVolumeCaching_; }

    /**
     * Returns the inport with the identifier type
     * @param type unique port type
     * @return port with given type, null if not existing
     */
    Port* getInport(Identifier type);

    /**
     * Returns the outport with the identifier type
     * @param type unique port type
     * @return port with given type, null if not existing
     */
    Port* getOutport(Identifier type);

    std::map<Port*, Port*> getOutportToInportMap();

    /// Is this processor an end processors as it does not have any output port?
    virtual bool isEndProcessor() const;

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
     */
    std::string getState() const;

    // identifiers commonly used in processors
    static const std::string XmlElementName_;

protected:
    bool cacheable_; ///< is caching supported by this processor?
    bool useVolumeCaching_;

    /**
     * Is this processor a coprocessor? If you want to create a coprocessor you have to use the
     * setIsCoProcessor(bool) function in the constructor of your processor.
     */
    bool isCoprocessor_;

    std::string name_;      ///< name of the processor

    Properties props_; ///< vector with all properties of the processor

    int initStatus_; ///< status of OpenGL initialization

    static const std::string loggerCat_; ///< category used in logging

private:

    /**
     * Some processors need to render to the same rendertarget they got their input from. If this is neccessary,
     * you can specify that in the createOutport function, which then makes an entry in this map. This map is then
     * analyzed in the networkevaluator, where the appropriate rendertargets are given.
     */
    std::map<Port*, Port*> outportToInportMap_;
    
    typedef std::map<Identifier, Port*> PortMap;
    PortMap portMap_;

    /**
     * List of ports that specifies which inputs this processor needs.
     */
    std::vector<Port*> inports_;

    /**
     * List of ports that specifies which outputs this processor creates
     */
    std::vector<Port*> outports_;

    /**
     * The CoProcessorInports this processor has.
     */
    std::vector<Port*> coProcessorInports_;

    /**
     * The CoProcessorOutports this Coprocessor has. (Only CoProcessors should have CoProcessorOutports)
     */
    std::vector<Port*> coProcessorOutports_;

    /**
     * The private ports this processor has. PrivatePorts are mapped to rendertargets no other processor has access to.
     */
    std::vector<Port*> privatePorts_;

    MetaSerializer meta_;
};

typedef TemplateMessage<Processor*> ProcessorPtrMsg;

// ------------------------------------------------------------------------- //

//Deprecated
class HasShader {
public:
    HasShader();

    virtual ~HasShader();

    /**
     * Invalidate the shader, so that recompiling is performed before the next
     * rendering.
     */
    virtual void invalidateShader();

protected:
    /**
     * Compile the shader if necessary.
     *
     * Checks the bool flag isShaderValid_ and calls the virtual method compile()
     * if necessary. So derived classes should place their compile code there and
     * call the method invalidateShader() when recompiling is required.
     */
    virtual void compileShader();

    /**
     * Compile and link the shader program
     */
    virtual void compile() = 0;

private:
    bool needRecompileShader_; ///< should the shader recompiled?
};


} // namespace voreen

#endif // VRN_PROCESSOR_H
