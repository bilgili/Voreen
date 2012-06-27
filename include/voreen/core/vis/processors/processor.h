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

#ifndef VRN_PROCESSOR_H
#define VRN_PROCESSOR_H

#include <vector>

#include "tgt/camera.h"

#include "voreen/core/opengl/texunitmapper.h"
#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/message.h"
#include "voreen/core/vis/property.h"
#include "voreen/core/xml/serializable.h"
#include "voreen/core/vis/processors/port.h"

namespace tgt {
class Shader;
}

namespace voreen {

class ProxyGeometry;
class PortMapping;
class LocalPortMapping;
class GeometryContainer;
class TextureContainer;
class VolumeContainer;
class VolumeGL;
class VolumeTexture;

//TODO: move this into processor as local structs
// One site of a connection between to processors. Needed for serialization.
struct ConnectionSite {
    int processorId;
    std::string portId;
    int order;
};

// Needed to be able to sort connections based on the order of the outgoing side.
struct ConnectionCompare {
    bool operator() ( std::pair< ConnectionSite, ConnectionSite > l,
                            std::pair< ConnectionSite, ConnectionSite > r) {
        // connect outgoing connections with lower order prior to those with higher order
        return (l.second.order<r.second.order);
    }
};

// Needed for Serialization.
// Note: The order for the first port is not really needed.
typedef std::vector< std::pair< ConnectionSite, ConnectionSite > > ConnectionMap;
           

class PropertyOwner {
public:

};

/**
 * Macro for automatic registration of Processors using static initialization. The information
 * is used in ProcessorFactory::initializeClassList() which also takes care of freeing the objects.
 *
 * Insert this macro into the declaration (private part) of every processor that should
 * automatically register itself.
 */
#define VRN_PROCESSOR_CLASS(A) \
    static const bool isRegistered; \
    static Processor* createStatic() { return new A; }

/**
 * Insert this macro into the definition of every processor that should automatically register
 * itself.
 */
#define VRN_PROCESSOR_REGISTER(A) \
    const bool A::isRegistered = Processor::registerProcessor(&A::createStatic);

/**
 * The base class for all processor classes used in Voreen.
 */
class Processor : public MessageReceiver, public Serializable, public PropertyOwner {
public:
    enum Status {
        VRN_OK                  = 0x200,
        VRN_OPENGL_INSUFFICIENT = 0x201,
        VRN_ERROR               = 0x202
    };

    // Function pointer to a static method that returns a new instance of this class. Used for
    // automatic registration of processors.
    typedef Processor* (*CreateProcessorFunctionPointer)();

    // Maximum number of different processor types that can register using static
    // initialization. Can not use a dynamic std::vector because the order of initialization is
    // unknown.
    static const size_t MAX_REGISTERED_PROCESSORS = 100;
    
    /**
    * @param camera The tgt::Camera which is used with this class.
    * @param tc Some processor subclasses must know of the TextureContainer.
    */
    Processor(tgt::Camera* camera=0, TextureContainer* tc = 0);
    virtual ~Processor();
    
    virtual const Identifier getClassName() const = 0;
    static const Identifier getClassName(TiXmlElement* processorElem);
    virtual Processor* create() = 0;

    /// This virtual -- here abstract -- method shall be called if the processor should be processed.
	virtual void process(LocalPortMapping* portMapping) /*throw (std::exception)*/ = 0;

    /**
     * Should be called after OpenGL is initialized, initializes the processor.
     *
     * @return Returns an error code. VRN_OK means everything ok.
     */
    virtual int initializeGL();

    /**
     * This method shall be called before a processor::process() call
     * but only if another processor was used before.
     */
    virtual void init();

	/**
    * CoProcessors add a function pointer to their CoprocessorOutports. This 
    * function pointer usually points to this function, which is overwritten in the
    * CoProcessors.
    */
    virtual Message* call(Identifier ident, LocalPortMapping* portMapping);

    /**
     * Call this method before calling processor::process() of another processor object.
     */
    virtual void deinit();

    virtual void setTextureContainer(TextureContainer* tc);
    virtual TextureContainer* getTextureContainer();

    virtual void setCamera(tgt::Camera* camera);

    /// Returns the camera used by this class.
    virtual tgt::Camera* getCamera() const { return camera_; }

    /// Returns the light source position in world coordinates
    virtual tgt::vec3 getLightPosition() const;

    /**
     * Processes:
     * - Identifier::setBackgroundColor
     * - "set.viewport"
     * - Identifier::setTargetTypeMapping
     * - LightMaterial::setLightPosition_, type Vec4Msg
     * - LightMaterial::setLightAmbient_, type ColorMsg
     * - LightMaterial::setLightDiffuse_, type ColorMsg
     * - LightMaterial::setLightSpecular_, type ColorMsg
     * - LightMaterial::setLightAttenuation_, type Vec3Msg (constant, linear, quadratic)
     * - LightMaterial::setMaterialAmbient_, type ColorMsg
     * - LightMaterial::setMaterialDiffuse_, type ColorMsg
     * - LightMaterial::setMaterialSpecular_, type ColorMsg
     * - LightMaterial::setMaterialShininess_, type FloatMsg
     */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

    /// Set a name for this processor
    void setName(const std::string& name);
    /// Returns the name of this processor.
    std::string getName() const;

	// Returns processor information
	virtual const std::string getProcessorInfo() const { return "No information available"; }

    /// Set the size of this processor. Forces an update of the projection matrix.
    virtual void setSize(const tgt::ivec2& size);

    /// Marks that the processor needs to be updated
    virtual void invalidate();

    /// Adds a property for the automatic generated GUI.
    virtual void addProperty(Property* prop);

    /// Returns the properties.
    virtual const Properties& getProperties() const;

    /**
    * Returns the inports of this processor.
    */
    virtual std::vector<Port*> getInports() const {return inports_;}
    
    /**
    * Returns the outports of this processor.
    */
    virtual std::vector<Port*> getOutports() const {return outports_;}

    /**
    * Returns the coprocessor inports of this processor.
    */
	virtual std::vector<Port*> getCoProcessorInports() const {return coProcessorInports_;}

    /**
    * Returns the coprocessor outports of this processor.
    */
	virtual std::vector<Port*> getCoProcessorOutports() const {return coProcessorOutports_;}

    /**
    * Returns the private ports of this processor.
    */
	virtual std::vector<Port*> getPrivatePorts() const {return privatePorts_;}

    /**
    * Returns the port that contains (or better that is represented by) the given Identifier. 
    */
	virtual Port* getPort(Identifier ident);

    /**
    * Creates a new inport for this processor.
    * @param type The identifier by which this port is adressed in the processor. Something like "image.entrypoints" for example.
    * @param allowMultipleConnections Can multiple ports be connected to this processor? This should usually be false
    */
	virtual void createInport(Identifier type, bool allowMultipleConnections = false);
	
    /**
    * Creates a new outport for this processor.
    * @param type The identifier by which this port is adressed in the processor. Something like "image.entrypoints" for example.
    * @param inportIdent Some processors need to render to the same rendertarget they got their input from. If this is neccessary,
    *                    inportIdent holds the Identifier of that inport.
    * @param allowMultipleConnections Can multiple ports be connected to this processor? Default is true for outports
    * @param isPersistent Should the render result carried by this port be persistent in the TC?
    */
    virtual void createOutport(Identifier type, bool isPersistent = false,Identifier inportIdent="dummy.port.unused");

    /**
    * Creates a new CoProcessorInport for this processor.
    * @param type The identifier by which this port is adressed in the processor. Something like "image.entrypoints" for example.
    * @param allowMultipleConnections Can multiple ports be connected to this processor? This should usually be false
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
    virtual void createCoProcessorOutport(Identifier type, FunctionPointer function=0, bool allowMultipleConnections = true);

    /**
    * Creates a new private port for this processor. Private ports are ports that get a render
    * target, but that target isn't connected to any other processors. (That port doesn't show
    * up in rptgui). The combiner for example needs two temporary TC targets to work correctly,
    * and this is how it gets them. Just create two privatePorts and render your temporary
    * render results to those targets. Private ports are always persistent.
    * 
    * @param type The identifier by which this port is adressed in the processor. Something
    * like "image.entrypoints" for example.
    */
	virtual void createPrivatePort(Identifier type);

    /**
    * Connects one outport of this processor with one inport of another processor. You can use
    * this function to manually build processor networks, but the easier way is to use rptgui.
    *
    * @param outport The outport of this processor that is to be connected
    * @param inport The inport of the target processor to which the outport should connect.
    */
    virtual bool connect(Port* outport, Port* inport);

    /**
    * Tests if the outport of this processor can be connected to the inport of the other processor
    */
	virtual bool testConnect(Port* outport, Port* inport);

    /**
    * Disconnectes the outport of this processor from the inport from another processor
    */
    virtual bool disconnect(Port* outport, Port* inport);
    
    static const std::string XmlElementName;
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
    TiXmlElement* serializeToXml(const std::map<Processor*,int> idMap) const;
     
    /**
     * Updates the processor xml and returns connection info so that
     * processors can be connected, after all have been read, in the form
     * (id, ConnectionMap)
     */
    std::pair<int, ConnectionMap> getMapAndupdateFromXml(TiXmlElement* processorElem);
    
    /**
     * Serialization for metadata
     */
    void addToMeta(TiXmlElement* elem) { meta_.addData(elem); }
    void removeFromMeta(std::string elemName) { meta_.removeData(elemName); }
    void clearMeta() { meta_.clearData(); }
    TiXmlElement* getFromMeta(std::string elemName) const { return meta_.getData(elemName); }
    bool hasInMeta(std::string elemName) const { return meta_.hasData(elemName); }

    /**
    * Checks whether this processor is a coprocessor or not
    */
    bool getIsCoprocessor() const {return isCoprocessor_;}

    /**
    * Sets whether this processor is a coprocessor or not. If you want to create a coprocessor you have
    * to use this function in the constructor of your processor. This is far from optimal of course, we 
    * should have a CoProcessor class, but we dont't :) 
    */ 
    void setIsCoprocessor(bool b) {isCoprocessor_ = b;}

    /**
    * Returns whether the render results of this processor are currently cached or not. The networkevaluator
    * checks this before every rendering, you shouldn't have to use this function at all.
    */
    bool getCached() const {return cached_;}

    /**
    * Sets whether the render results of this processor are currently cached or not. The networkevaluator
    * can do this after every rendering, you shouldn't have to use this function at all.
    */
	void setCached(bool b) {
		cached_ = b;
	}

    /**
    * Checks if it is possible to cache the render results of this processor. (That means if every outport is somehow
    * connected to a cacherenderer, not necesserily directly). The networkevaluator checks this when analyzing a network.
    */
	bool getCachable() const {return cachable_;}

    /**
    * Sets if it is possible to cache the render results of this processor. (That means if every outport is somehow
    * connected to a cacherenderer, not necesserily directly). The networkevaluator does this when analyzing a network.
    */
	void setCachable(bool b) {cachable_ = b;}

    /// Returns the error code which occured during \a initializeGL().
    int getInitStatus() const { return initStatus_; }

    /// Returns the size of the processor canvas.
    tgt::ivec2 getSize() const { return size_; }

    /**
     * Returns the projection matrix which was used set
     * after the last setSize call. If you override setSize it is important
     * to set \a projectionMatrix_ accordingly.
     */
    tgt::mat4 getProjectionMatrix() const { return camera_->getProjectionMatrix(); }

    void setCoarseness(float factor);

	float getCoarsenessFactor() const;

    /** 
	 * Set the GeometryContainer. The container must be the same as used by the
     * network evaluator. Therefore it will be set by the later.
     */
    virtual void setGeometryContainer(GeometryContainer* geoCont);

    virtual GeometryContainer* getGeometryContainer() const;

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

    std::map<Port*,Port*> getOutportToInportMap() {
        return outportToInportMap_;
    }

    /// Is this processor an end processors as it does not have any output port?
    virtual bool isEndProcessor() const;
    
    ///Can this processor be used in the multipass section of a network (between DS and RC)
    virtual bool isMultipassCompatible() { return false; }
    
    ///Set to true if this Processor is part of the multipass section. Default is false.
    virtual void setMultipass(bool mp) { isMultipass_ = mp; }
    
    ///Is this processor part of the multipass section?
    virtual bool isMultipass() { return isMultipass_; }
    
    ///For multipass rendering: Do some init before first pass. (Clear target...)
    virtual void initFirstPass(LocalPortMapping* /*portMapping*/) {}
    
    /// Returns function pointers to the createStatic() method of all processors that have
    /// registered using static initialization.
    static std::vector<CreateProcessorFunctionPointer> getRegisteredProcessors() {
        std::vector<CreateProcessorFunctionPointer> v;
        for (size_t i=0; i < registeredProcessorsCount(); i++)
            v.push_back(registeredProcessors_[i]);
        return v;
    }

    // identifiers commonly used in processors
    static const Identifier delete_;
    static const Identifier setBackgroundColor_;
    static const Identifier setCoarseness_;
    // obsolete in the near future
    static const Identifier setVolumeContainer_;
    static const Identifier setCurrentDataset_;

protected:
    
    /**
    * Are the render results of this processor cachable? That means are all outports somehow connected to a cacherenderer. Only
    * used in the networkevaluator.
    */
	bool cachable_;

    /**
    * Are the render results of this processor currently cached? The networkevaluator checks this before rendering.
    */ 
	bool cached_;

    /**
    * Is this processor a coprocessor? If you want to create a coprocessor you have
    * to use the setIsCoProcessor(bool) function in the constructor of your processor. This is far from optimal of course, we 
    * should have a CoProcessor class, but we dont't :) 
    */
	bool isCoprocessor_; //just temporary until we introduce coprocessors classes
    
    ///Is this processor in a multipass section?
    bool isMultipass_;
    
    /// Renders a screen aligned quad.
    void renderQuad();

    /**
     * This generates the header that will be used at the beginning of the shaders. It includes the necessary #defines that
     * are to be used with the shaders.
     * \note If you overwrite this function in a subclass, you have to the call the superclass' function first and
     * append your additions to its result!
     */
    virtual std::string generateHeader();

    /**
     * Sets some uniforms potentially needed by every shader.
     * @note This function should be called for every shader before every rendering pass!
     * @param shader the shader to set up
     */
    virtual void setGlobalShaderParameters(tgt::Shader* shader);

    /**
     * \brief Updates the current OpenGL context according to the 
     *        object's lighting properties (e.g. lightPosition_). 
     *
     * The following parameters are set for GL_LIGHT0:
     * - Light source position
     * - Light ambient / diffuse / specular colors
     * - Light attenuation factors
     *
     * The following material parameters are set (GL_FRONT_AND_BACK):
     * - Material ambient / diffuse / specular / emissive colors
     * - Material shininess
     *
     */
    virtual void setLightingParameters();

    // FIXME: does not work anymore
    /// Internally used for making high resolution screenshots.
    void setSizeTiled(uint width, uint height);

    /**
     * Registers the processor into the processor list for use with the VRN_PROCESSOR_CLASS()
     * and VRN_PROCESSOR_REGISTER() macros. This information is retrieved by the
     * ProcessorFactory.
     * @param createFunc Function pointers to a method that returns a new instance of the
     * processor to be registered.
     */   
    static bool registerProcessor(CreateProcessorFunctionPointer createFunc) {
        if (registeredProcessorsCount() >= MAX_REGISTERED_PROCESSORS) {
            return false;
        } else {
            registeredProcessors_[registeredProcessorsCount()] = createFunc;
            registeredProcessorsCount()++;
            return true;
        }
    }
    
    GeometryContainer* geoContainer_;

    tgt::Camera* camera_;
    TextureContainer* tc_;
    TexUnitMapper tm_;//TODO: rename?

    std::string name_;
    tgt::ivec2 size_;
	float coarsenessFactor_;
    ColorProp backgroundColor_;

    /// The position of the light source used for lighting calculations in world coordinates
    FloatVec4Prop lightPosition_;
    /// The light source's ambient color according to the Phong lighting model
    ColorProp lightAmbient_;
    /// The light source's diffuse color according to the Phong lighting model
    ColorProp lightDiffuse_;
    /// The light source's specular color according to the Phong lighting model
    ColorProp lightSpecular_;
    /// The light source's attenuation factors (x = constant, y = linear, z = quadratic)
    FloatVec3Prop lightAttenuation_;
    /// The ambient material color according to the Phong lighting model
    ColorProp materialAmbient_;
    /// The diffuse material color according to the Phong lighting model
    ColorProp materialDiffuse_;
    /// The specular material color according to the Phong lighting model
    ColorProp materialSpecular_;
    /// The emission material color according to the Phong lighting model
    ColorProp materialEmission_;
    /// The material's specular exponent according to the Phong lighting model
    FloatProp materialShininess_;

    std::vector<Property*> props_;

    static const std::string loggerCat_;

    // commonly used texture units
    static const Identifier entryParamsTexUnitIdent_;
    static const Identifier entryParamsDepthTexUnitIdent_;
    static const Identifier exitParamsTexUnitIdent_;
    static const Identifier exitParamsDepthTexUnitIdent_;
    static const Identifier volTexUnitIdent_;
    static const Identifier volTexUnit2Ident_;
    static const Identifier transferTexUnitIdent_;
    static const Identifier segmentationTexUnitIdent_;
    static const Identifier shadowTexUnit1Ident_;
    static const Identifier shadowTexUnit2Ident_;
    static const Identifier shadowTexUnit3Ident_;
    static const Identifier shadowTexUnit4Ident_;
    static const Identifier ambTexUnitIdent_;
    static const Identifier ambLookupTexUnitIdent_;
    static const Identifier firstHitNormalsTexUnitIdent_;
    static const Identifier firstHitPointsTexUnitIdent_;

    int initStatus_;

private:

    /**
     * Some processors need to render to the same rendertarget they got their input from. If this is neccessary,
     * you can specify that in the createOutport function, which then makes an entry in this map. This map is then
     * analyzed in the networkevaluator, where the appropriate rendertargets are given. 
     */
    std::map<Port*,Port*> outportToInportMap_;

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

    /**
     * List of all processors which have registered through the VRN_PROCESSOR_CLASS() and
     * VRN_PROCESSOR_REGISTER() macros. Contains for each processor a functions pointer to a
     * method that returns a new instance of the respective processor. A static array is used
     * instead of a std::vector because the initialization order for the vector is unknown.
     */
    static CreateProcessorFunctionPointer registeredProcessors_[MAX_REGISTERED_PROCESSORS];

    /**
     * Number of processors which have registered through the VRN_PROCESSOR_CLASS() and
     * VRN_PROCESSOR_REGISTER() macros. Use a method instead a static variable to ensure
     * correct initialization order.
     */
    static size_t& registeredProcessorsCount() { static size_t count = 0; return count; } 

    MetaSerializer meta_;

};

typedef TemplateMessage<Processor*> ProcessorPtrMsg;

} // namespace

#endif // VRN_PROCESSOR_H
