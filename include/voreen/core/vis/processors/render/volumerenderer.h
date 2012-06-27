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

#ifndef VRN_VOLUMERENDERER_H
#define VRN_VOLUMERENDERER_H

#include "voreen/core/vis/processors/renderprocessor.h"

namespace voreen {

/**
 * All classes which render volumes should inherit from this class.
 */
class VolumeRenderer : public RenderProcessor {
public:
    VolumeRenderer();
protected:
    /**
     * This struct contains information about a volume. It is exclusively used
     * as parameter type for the bindVolumes() function.
     */
    struct VolumeStruct {
        VolumeStruct();
        VolumeStruct(const VolumeGL* volume, const std::string& textureUnitIdent,
                     const std::string& samplerIdentifier, const std::string& volumeParametersIdentifier);

        const VolumeGL* volume_;                        ///< the volume whose texture is to bound
        std::string textureUnitIdent_;                   ///< specifies the texture unit the volume texture is bound to
        std::string samplerIdentifier_;                 ///< the sampler by which the
                                                        ///  volume is accessed in the shader
        std::string volumeParametersIdentifier_;        ///< the identifier of the volume parameter struct
                                                        ///  for this volume in the shader

    };

    virtual std::string generateHeader(VolumeHandle* volumehandle = 0);

    virtual void setGlobalShaderParameters(tgt::Shader* shader, tgt::Camera* camera = 0);

    /**
     * This function binds the volume textures used by the volume renderer and passes the
     * corresponding samplers (texture units) to the shader.
     * \note This function also sets additional meta data like the volume's dimensions and spacing.
     *       For this reason, every volume renderer that uses a volume texture must call this function
     *       for all volumes accessed during a rendering pass. Do not bypass this function by binding
     *       volume textures directly unless you know exactly what you are doing!
     * @param shader the shader that performs the volume rendering
     * @param volumes vector of the volumes to bind
     */
    virtual void bindVolumes(tgt::Shader* shader, const std::vector<VolumeStruct> &volumes);

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
    ColorProperty lightAmbient_;
    /// The light source's diffuse color according to the Phong lighting model
    ColorProperty lightDiffuse_;
    /// The light source's specular color according to the Phong lighting model
    ColorProperty lightSpecular_;
    /// The light source's attenuation factors (x = constant, y = linear, z = quadratic)
    FloatVec3Property lightAttenuation_;
    /// The ambient material color according to the Phong lighting model
    ColorProperty materialAmbient_;
    /// The diffuse material color according to the Phong lighting model
    ColorProperty materialDiffuse_;
    /// The specular material color according to the Phong lighting model
    ColorProperty materialSpecular_;
    /// The emission material color according to the Phong lighting model
    ColorProperty materialEmission_;
    /// The material's specular exponent according to the Phong lighting model
    FloatProperty materialShininess_;
    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_VOLUMERENDERER_H
