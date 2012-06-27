/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#ifndef VRN_VOLUMERENDERER_H
#define VRN_VOLUMERENDERER_H

#include "voreen/core/processors/renderprocessor.h"

#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/ports/volumeport.h"

#include "tgt/textureunit.h"

namespace voreen {

/**
 * All classes which render volumes should inherit from this class.
 */
class VolumeRenderer : public RenderProcessor {
public:
    VolumeRenderer();
    ~VolumeRenderer();

protected:
    /**
     * This struct contains information about a volume. It is exclusively used
     * as parameter type for the bindVolumes() function.
     */
    struct VolumeStruct {
        VolumeStruct();
        VolumeStruct(const VolumeGL* volume, const std::string& textureUnitIdent,
                     const std::string& samplerIdentifier, const std::string& volumeParametersIdentifier,
                     bool applyDatasetTrafoMatrix = true);
        VolumeStruct(const VolumeGL* volume, tgt::TextureUnit* texUnit,
                     const std::string& samplerIdentifier, const std::string& volumeParametersIdentifier,
                     bool applyDatasetTrafoMatrix = true);

        const VolumeGL* volume_;                        ///< the volume whose texture is to bound
        tgt::TextureUnit* texUnit_;
        std::string samplerIdentifier_;                 ///< the sampler by which the
                                                        ///  volume is accessed in the shader
        std::string volumeParametersIdentifier_;        ///< the identifier of the volume parameter struct
                                                        ///  for this volume in the shader
        bool applyDatasetTrafoMatrix_;                  ///< apply the volume's trafo matrix in the shader?
    };

    virtual std::string generateHeader(VolumeHandle* volumehandle = 0);

    virtual void setGlobalShaderParameters(tgt::Shader* shader, const tgt::Camera* camera = 0);

    /**
     * This function binds the volume textures used by the volume renderer and passes the
     * corresponding samplers (texture units) to the shader.
     * \note This function also sets additional meta data like the volume's dimensions and spacing.
     *       For this reason, every volume renderer that uses a volume texture must call this function
     *       for all volumes accessed during a rendering pass. Do not bypass this function by binding
     *       volume textures directly unless you know exactly what you are doing!
     * @param shader the shader that performs the volume rendering
     * @param volumes vector of the volumes to bind
     * @param camera the scene's camera position. Is transformed to volume object coordinates for lighting calculations.
     * @param lightPosition the scene's light position. Is transformed to volume object coordinates for lighting calculations.
     */
    virtual void bindVolumes(tgt::Shader* shader, const std::vector<VolumeStruct> &volumes,
        const tgt::Camera* camera = 0, const tgt::vec4& lightPosition = tgt::vec4(0.f));

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

    /// The position of the light source used for lighting calculations in world coordinates
    FloatVec4Property lightPosition_;
    /// The light source's ambient color according to the Phong lighting model

    FloatVec4Property lightAmbient_;
    /// The light source's diffuse color according to the Phong lighting model
    FloatVec4Property lightDiffuse_;
    /// The light source's specular color according to the Phong lighting model
    FloatVec4Property lightSpecular_;

    /// The light source's attenuation factors (x = constant, y = linear, z = quadratic)
    FloatVec3Property lightAttenuation_;
    /// Apply light attenuation?
    BoolProperty applyLightAttenuation_;
    /// The ambient material color according to the Phong lighting model

    FloatVec4Property materialAmbient_;
    /// The diffuse material color according to the Phong lighting model
    FloatVec4Property materialDiffuse_;
    /// The specular material color according to the Phong lighting model
    FloatVec4Property materialSpecular_;
    /// The emission material color according to the Phong lighting model
    FloatVec4Property materialEmission_;

    /// The material's specular exponent according to the Phong lighting model
    FloatProperty materialShininess_;
    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_VOLUMERENDERER_H
