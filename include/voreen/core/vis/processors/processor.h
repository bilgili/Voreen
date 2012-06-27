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
#include "voreen/core/volume/volumehandlevalidator.h"
#include "voreen/core/vis/properties/allproperties.h"
#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

class ProxyGeometry;
class GeometryContainer;
class TextureContainer;

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

// ------------------------------------------------------------------------- //

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


    /**
     * @param camera The tgt::Camera which is used with this class.
     * @param tc Some processor subclasses must know the TextureContainer.
     */
    Processor(tgt::Camera* camera = 0, TextureContainer* tc = 0);

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

    virtual void setTextureContainer(TextureContainer* tc);
    virtual TextureContainer* getTextureContainer();

    virtual void setCamera(tgt::Camera* camera);

    /// Returns the camera used by this processor.
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

    /// Set the size of this processor.
    virtual void setSize(const tgt::ivec2& size);

    /// Set the size of this processor.
    virtual void setSize(const tgt::vec2& size);

    /// Returns the size of the processor canvas.
    tgt::ivec2 getSize() const;

    /// Returns the size of the processor canvas as a float.
    /// Non-integer values can be introduces by the CoarsenessRenderer.
    tgt::vec2 getSizeFloat() const;
    
    /// Marks that the processor needs to be updated
    virtual void invalidate();

    /// Adds a property to this processor
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
        Port* newOutport = new GenericOutPort<T>(type, data, this, isPersistent);
        outports_.push_back(newOutport);
        if (inportIdent != "dummy.port.unused") {
            if (getPort(inportIdent) != 0)
                outportToInportMap_.insert(std::pair<Port*, Port*>(newOutport, getPort(inportIdent)));
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
     * Set the GeometryContainer. The container must be the same as used by the
     * network evaluator. Therefore it will be set by the latter.
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
    static const Identifier setBackgroundColor_;
    static const std::string XmlElementName_;

protected:
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
    //TODO: remove, a general processor has no shaders. joerg
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

    // FIXME: does not work anymore, deprecated
    /// Internally used for making high resolution screenshots.
    void setSizeTiled(uint width, uint height);

    bool cacheable_; ///< is caching supported by this processor?
    bool useVolumeCaching_;

    /**
     * Is this processor a coprocessor? If you want to create a coprocessor you have to use the
     * setIsCoProcessor(bool) function in the constructor of your processor.
     */
    bool isCoprocessor_;

    GeometryContainer* geoContainer_; ///< container that holds geomtry, e.g. points or pointlists

    tgt::Camera* camera_;   ///< the camera that will be used in rendering
    TextureContainer* tc_;  ///< manages render targets. That are textures or the framebuffer.
    TexUnitMapper tm_;      ///< manages texture units

    std::string name_;      ///< name of the processor
    tgt::vec2 size_;        ///< size of the viewport of the processor

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

} // namespace voreen

#endif // VRN_PROCESSOR_H
