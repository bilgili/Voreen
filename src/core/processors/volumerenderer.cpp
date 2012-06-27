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

#include "tgt/gpucapabilities.h"
#include "tgt/textureunit.h"
#include "voreen/core/processors/volumerenderer.h"

using tgt::vec3;
using tgt::vec4;
using tgt::Color;
using tgt::TextureUnit;

namespace voreen {

const std::string VolumeRenderer::loggerCat_("voreen.VolumeRenderer");

VolumeRenderer::VolumeRenderer()
    : RenderProcessor()
    , lightPosition_("lightPosition", "Light source position", tgt::vec4(2.3f, 1.5f, 1.5f, 1.f),
                     tgt::vec4(-10), tgt::vec4(10))
    , lightAmbient_("lightAmbient", "Ambient light", tgt::Color(0.4f, 0.4f, 0.4f, 1.f))
    , lightDiffuse_("lightDiffuse", "Diffuse light", tgt::Color(0.8f, 0.8f, 0.8f, 1.f))
    , lightSpecular_("lightSpecular", "Specular light", tgt::Color(0.6f, 0.6f, 0.6f, 1.f))
    , lightAttenuation_("lightAttenuation", "Attenuation", tgt::vec3(1.f, 0.f, 0.f))
    , applyLightAttenuation_("applyLightAttenuation", "Apply Attenuation", false, Processor::INVALID_PROGRAM)
    , materialAmbient_("materialAmbient", "Ambient material color", tgt::Color(1.f, 1.f, 1.f, 1.f))
    , materialDiffuse_("materialDiffuse", "Diffuse material color", tgt::Color(1.f, 1.f, 1.f, 1.f))
    , materialSpecular_("materialSpecular", "Specular material color", tgt::Color(1.f, 1.f, 1.f, 1.f))
    , materialEmission_("materialEmission", "Emissive material color", tgt::Color(0.f, 0.f, 0.f, 1.f))
    , materialShininess_("materialShininess", "Shininess", 60.f, 0.1f, 128.f)
{
}

VolumeRenderer::~VolumeRenderer() {
}

std::string VolumeRenderer::generateHeader(VolumeHandle* /*volumehandle*/) {
    std::string header = RenderProcessor::generateHeader();

    if (GpuCaps.isNpotSupported())
        header += "#define VRN_TEXTURE_3D\n";
    else
        header += "#define VRN_TEXTURE_3D_SCALED\n";

    return header;
}

void VolumeRenderer::setGlobalShaderParameters(tgt::Shader* shader, tgt::Camera* camera) {
    RenderProcessor::setGlobalShaderParameters(shader, camera);

    shader->setIgnoreUniformLocationError(true);

    // light source position in world coordinates
    shader->setUniform("lightPosition_", lightPosition_.get().xyz());

    shader->setIgnoreUniformLocationError(false);
}

void VolumeRenderer::bindVolumes(tgt::Shader* shader, const std::vector<VolumeStruct>& volumes) {
    bool texCoordScaling = !GpuCaps.isNpotSupported();
    shader->setIgnoreUniformLocationError(true);

    for (size_t i=0; i < volumes.size(); ++i) {
        // some shortcuts
        const VolumeStruct& volumeStruct = volumes[i];
        const VolumeGL* volumeGL = volumeStruct.volume_;
        if (!volumeGL || !volumeGL->getTexture()) {
            LWARNING("No volume texture while binding volumes");
            continue;
        }
        const Volume* volume = volumeGL->getVolume();
        const VolumeTexture* volumeTex = volumeGL->getTexture();

        // bind volume texture and pass sampler to the shader
        GLint loc = shader->getUniformLocation(volumeStruct.samplerIdentifier_);

        TextureUnit* texUnit = volumes[i].texUnit_;
        if (!texUnit) {
            LERROR("No texture unit while binding volumes");
            continue;
        }

        if (loc != -1) {
            texUnit->activate();

            volumeTex->bind();

            // Returns the residence status of the target texture. If the value returned in params is
            // GL_TRUE, the texture is resident in texture memory
            GLint resident;
            glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_RESIDENT, &resident);

            if (resident != GL_TRUE)
                LWARNING("texture not resident: " /*<< volume->meta().getFileName()*/);

            shader->setUniform(loc, texUnit->getUnitNumber());

            LGL_ERROR;
        }

        // set volume meta-data
        std::string paramsIdent = volumeStruct.volumeParametersIdentifier_;
        // volume size, i.e. dimensions of the proxy geometry in world coordinates
        shader->setUniform(paramsIdent + ".datasetDimensions_", tgt::vec3(volume->getDimensions()));
        shader->setUniform(paramsIdent + ".datasetDimensionsRCP_", vec3(1.f) / tgt::vec3(volume->getDimensions()));

        // volume spacing, i.e. voxel size
        shader->setUniform(paramsIdent + ".datasetSpacing_", volume->getSpacing());
        shader->setUniform(paramsIdent + ".datasetSpacingRCP_", vec3(1.f) / volume->getSpacing());

        // volume's size in its object coordinates
        shader->setUniform(paramsIdent + ".volumeCubeSize_", volume->getCubeSize());
        shader->setUniform(paramsIdent + ".volumeCubeSizeRCP_", vec3(1.f) / volume->getCubeSize());

        // volume's transformation matrix
        tgt::mat4 tm = volume->getTransformation();
        shader->setUniform(paramsIdent + ".volumeTransformation_", tm);

        tgt::mat4 invTm;
        if(!tm.invert(invTm))
            LWARNING("Failed to invert volume transformation matrix!");

        shader->setUniform(paramsIdent + ".volumeTransformationINV_", invTm);


        LGL_ERROR;

        // scaling of texture coords, if a resize of a npot texture to pot dimensions was necessary
        if (texCoordScaling) {
            // we are only interested in the scaling part of the texture matrix
            vec3 texScaleVector = volumeTex->getMatrix().getScalingPart();
            shader->setUniform(paramsIdent + ".texCoordScaleFactor_", texScaleVector);
            shader->setUniform(paramsIdent + ".texCoordScaleFactorRCP_", vec3(1.f) / texScaleVector);
            LGL_ERROR;
        }

        // bit depth of the volume
        loc = shader->setUniform(paramsIdent + ".bitDepth_", volume->getBitsStored());

        // is the volume a 12 bit volume => fetched texel values have to be normalized in the shader
        if (volume->getBitsStored() == 12)
            shader->setUniform(paramsIdent + ".bitDepthScale_", 16.0f);
        else
            shader->setUniform(paramsIdent + ".bitDepthScale_", 1.0f);

        LGL_ERROR;
    }

    shader->setIgnoreUniformLocationError(false);
    LGL_ERROR;
}

void VolumeRenderer::setLightingParameters() {
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient_.get().elem);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse_.get().elem);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular_.get().elem);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, lightAttenuation_.get().x);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, lightAttenuation_.get().y);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, lightAttenuation_.get().z);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmission_.get().elem);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess_.get());
    LGL_ERROR;
}

VolumeRenderer::VolumeStruct::VolumeStruct()
    : volume_(0)
{}

VolumeRenderer::VolumeStruct::VolumeStruct(const VolumeGL* volume, tgt::TextureUnit* texUnit,
                                           const std::string& samplerIdentifier,
                                           const std::string& volumeParametersIdentifier)
    : volume_(volume), texUnit_(texUnit),
      samplerIdentifier_(samplerIdentifier), volumeParametersIdentifier_(volumeParametersIdentifier)
{}

} // namespace voreen
