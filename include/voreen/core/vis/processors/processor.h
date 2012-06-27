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
#include "tgt/shadermanager.h"

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

class PropertyOwner {
};

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
    virtual tgt::Camera* getCamera() const;

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
	virtual const std::string getProcessorInfo() const;

    /// Set the size of this processor. Forces an update of the projection matrix.
    virtual void setSize(const tgt::ivec2& size);

    /// Set the size of this processor. Forces an update of the projection matrix.
    virtual void setSize(const tgt::vec2& size);

    /// Marks that the processor needs to be updated
    virtual void invalidate();

    /// Adds a property for the automatic generated GUI.
    virtual void addProperty(Property* prop);

    /// Returns the properties.
    virtual const Properties& getProperties() const;

    /**
    * Returns the inports of this processor.
    */
    virtual std::vector<Port*> getInports() const;
    
    /**
    * Returns the outports of this processor.
    */
    virtual std::vector<Port*> getOutports() const;

    /**
    * Returns the coprocessor inports of this processor.
    */
	virtual std::vector<Port*> getCoProcessorInports() const;

    /**
    * Returns the coprocessor outports of this processor.
    */
	virtual std::vector<Port*> getCoProcessorOutports() const;

    /**
    * Returns the private ports of this processor.
    */
	virtual std::vector<Port*> getPrivatePorts() const;

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
    void addToMeta(TiXmlElement* elem);
    void removeFromMeta(std::string elemName);
    void clearMeta();
    TiXmlElement* getFromMeta(std::string elemName) const;
    bool hasInMeta(std::string elemName) const;

    /**
     * Checks whether this processor is a coprocessor or not
     */
    bool getIsCoprocessor() const;

    /**
     * Sets whether this processor is a coprocessor or not. If you want to create a coprocessor you have
     * to use this function in the constructor of your processor. This is far from optimal of course, we 
     * should have a CoProcessor class, but we dont't :) 
     */ 
    void setIsCoprocessor(bool b);

    /**
     * Returns whether the render results of this processor are currently cached or not. The networkevaluator
     * checks this before every rendering, you shouldn't have to use this function at all.
     */
    bool getCached() const;

    /**
     * Sets whether the render results of this processor are currently cached or not. The networkevaluator
     * can do this after every rendering, you shouldn't have to use this function at all.
     */
	void setCached(bool b);

    /**
     * Checks if it is possible to cache the render results of this processor. (That means if every outport is somehow
     * connected to a cacherenderer, not necesserily directly). The networkevaluator checks this when analyzing a network.
     */
	bool getCachable() const;

    /**
     * Sets if it is possible to cache the render results of this processor. (That means if every outport is somehow
     * connected to a cacherenderer, not necesserily directly). The networkevaluator does this when analyzing a network.
     */
	void setCachable(bool b);

    /// Returns the error code which occured during \a initializeGL().
    int getInitStatus() const;

    /// Returns the size of the processor canvas.
    tgt::ivec2 getSize() const;

    /// Returns the size of the processor canvas.
    tgt::vec2 getSizeFloat() const;

    /**
     * Returns the projection matrix which was used set
     * after the last setSize call. If you override setSize it is important
     * to set \a projectionMatrix_ accordingly.
     */
    tgt::mat4 getProjectionMatrix() const;

    /**
     * Tells the processor the size of the canvas.
     *
     * @param size size of the canvas
     */
    void setCanvasSize(const tgt::ivec2& size);

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

    std::map<Port*,Port*> getOutportToInportMap();

    /// Is this processor an end processors as it does not have any output port?
    virtual bool isEndProcessor() const;
    
    // identifiers commonly used in processors
    static const Identifier setBackgroundColor_;
    static const std::string XmlElementName_;

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

    GeometryContainer* geoContainer_; ///< container that holds geomtry, e.g. points or pointlists

    tgt::Camera* camera_;   ///< the camera that will be used in rendering
    TextureContainer* tc_;  ///< manages render targets. That are textures or the framebuffer.
    TexUnitMapper tm_;      ///< manages texture units 

    std::string name_;      ///< name of the processor
    tgt::vec2 size_;        ///< size of the viewport of the processor
    tgt::ivec2 canvasSize_; ///< size of the canvas
	
    ColorProp backgroundColor_; ///< the color of the background

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

    std::vector<Property*> props_; ///< vector with all properties of the processor

    static const std::string loggerCat_; ///< category used in logging

    int initStatus_; ///< status of initialization

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

    MetaSerializer meta_;
};

typedef TemplateMessage<Processor*> ProcessorPtrMsg;

} // namespace

#endif // VRN_PROCESSOR_H
