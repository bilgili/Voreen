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

#ifndef VRN_RENDERPROCESSOR_H
#define VRN_RENDERPROCESSOR_H

#include <vector>

#include "tgt/camera.h"
#include "tgt/shadermanager.h"

#include "voreen/core/opengl/texunitmapper.h"
#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/message.h"
#include "voreen/core/xml/serializable.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/volume/volumehandlevalidator.h"
#include "voreen/core/vis/properties/allproperties.h"
#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

/**
 * The base class for all processor classes that render to the TextureContainer 
 */
class RenderProcessor : public Processor {
public:
    /**
     * @param camera The tgt::Camera which is used with this class.
     * @param tc Some processor subclasses must know the TextureContainer.
     */
    RenderProcessor(tgt::Camera* camera = 0, TextureContainer* tc = 0);

    virtual ~RenderProcessor();

    /// This method is called when the processor should be processed.
    virtual void process(LocalPortMapping* portMapping) = 0;

    virtual void setTextureContainer(TextureContainer* tc);
    virtual TextureContainer* getTextureContainer();

    /// Set the size of this processor.
    virtual void setSize(const tgt::ivec2& size);

    /// Set the size of this processor.
    virtual void setSize(const tgt::vec2& size);

    /// Returns the size of the processor canvas.
    tgt::ivec2 getSize() const;

    /// Returns the size of the processor canvas as a float.
    /// Non-integer values can be introduced by the CoarsenessRenderer.
    tgt::vec2 getSizeFloat() const;

    /**
     * Set the GeometryContainer. The container must be the same as used by the
     * network evaluator. Therefore it will be set by the latter.
     */
    virtual void setGeometryContainer(GeometryContainer* geoCont);

    virtual GeometryContainer* getGeometryContainer() const;

	//---------------------------------------------------------
	//Some deprecated methods:
    virtual void setCamera(tgt::Camera* camera);

    /// Returns the camera used by this processor.
    virtual tgt::Camera* getCamera() const;

    /// Returns the light source position in world coordinates
    virtual tgt::vec3 getLightPosition() const;

    /**
     * Processes:
     * - LightMaterial::setLightPosition_, type Vec4Msg
     */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);


    // identifiers commonly used in processors
    static const Identifier setBackgroundColor_;

	//---------------------------------------------------------
    static const std::string XmlElementName_;

protected:

    GeometryContainer* geoContainer_; ///< container that holds geomtry, e.g. points or pointlists

    TextureContainer* tc_;  ///< manages render targets. That are textures or the framebuffer.
    TexUnitMapper tm_;      ///< manages texture units

    tgt::vec2 size_;        ///< size of the viewport of the processor

    static const std::string loggerCat_; ///< category used in logging

	//---------------------------------------------------------
	//Some deprecated stuff:
	
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
	
    tgt::Camera* camera_;   ///< the camera that will be used in rendering

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
	//---------------------------------------------------------
};

} // namespace voreen

#endif // VRN_RENDERPROCESSOR_H
