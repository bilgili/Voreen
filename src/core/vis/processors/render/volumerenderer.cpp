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

#include "tgt/gpucapabilities.h"
#include "voreen/core/vis/processors/render/volumerenderer.h"

using tgt::vec3;
using tgt::vec4;
using tgt::Color;

namespace voreen {

const std::string VolumeRenderer::loggerCat_("voreen.VolumeRenderer");

const Identifier VolumeRenderer::setTransFunc_("set.TransferFunction");
const Identifier VolumeRenderer::setTransFunc2_("set.TransferFunction2");


VolumeRenderer::VolumeRenderer(tgt::Camera* camera, TextureContainer* tc)
    : RenderProcessor(camera, tc),
      currentVolumeHandle_(0)
{}

std::string VolumeRenderer::generateHeader() {
    std::string header = RenderProcessor::generateHeader();

    if (GpuCaps.isNpotSupported())
        header += "#define VRN_TEXTURE_3D\n";
    else
        header += "#define VRN_TEXTURE_3D_SCALED\n";

    return header;
}

void VolumeRenderer::setGlobalShaderParameters(tgt::Shader* shader) {
    RenderProcessor::setGlobalShaderParameters(shader);

    shader->setIgnoreUniformLocationError(true);

    shader->setUniform("viewMatrix_", getCamera()->getViewMatrix());
    shader->setUniform("projectionMatrix_", getCamera()->getProjectionMatrix());

    shader->setIgnoreUniformLocationError(false);
}

void VolumeRenderer::bindVolumes(tgt::Shader* shader, const std::vector<VolumeStruct>& volumeStructs) {
    bool texCoordScaling = ! GpuCaps.isNpotSupported();

    for (size_t i=0; i < volumeStructs.size(); ++i) {
        // some shortcuts
        const VolumeStruct& volumeStruct = volumeStructs[i];
        const VolumeGL* volumeGL = volumeStruct.volume_;
        const Volume* volume = volumeGL->getVolume();
        const VolumeTexture* volumeTex = volumeGL->getTexture();

        shader->setIgnoreUniformLocationError(true);

        // bind volume texture and pass sampler to the shader
        GLint loc = shader->getUniformLocation(volumeStruct.samplerIdentifier_);
        if (loc != -1 && volumeStruct.textureUnitIdent_ != "") {
            GLint texUnit = tm_.getTexUnit(volumeStruct.textureUnitIdent_);
            GLint OGLTexUnit = tm_.getGLTexUnit(volumeStruct.textureUnitIdent_);
            glActiveTexture(OGLTexUnit);

            volumeTex->bind();

            // Returns the residence status of the target texture. If the value returned in params is
            // GL_TRUE, the texture is resident in texture memory
            GLint resident;
            glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_RESIDENT, &resident);

            if (resident != GL_TRUE)
                LWARNING("texture not resident: " << volume->meta().getFileName());

            shader->setUniform(loc, texUnit);
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

        // scaling of texture coords, if a resize of a npot texture to pot dimensions was necessary
        if (texCoordScaling) {
            // we are only interested in the scaling part of the texture matrix
            vec3 texScaleVector = volumeTex->getMatrix().getScalingPart();
            shader->setUniform(paramsIdent + ".texCoordScaleFactor_", texScaleVector);
            shader->setUniform(paramsIdent + ".texCoordScaleFactorRCP_", vec3(1.f) / texScaleVector);
        }

        // bit depth of the volume
        loc = shader->setUniform(paramsIdent + ".bitDepth_", volume->getBitsStored());

        // is the volume a 12 bit volume => fetched texel values have to be normalized in the shader
        if (volume->getBitsStored() == 12)
            shader->setUniform(paramsIdent + ".bitDepthScale_", 16.0f);
        else
            shader->setUniform(paramsIdent + ".bitDepthScale_", 1.0f);

        shader->setIgnoreUniformLocationError(false);
    }

    LGL_ERROR;
}

VolumeRenderer::VolumeStruct::VolumeStruct()
    : volume_(0)
{}

VolumeRenderer::VolumeStruct::VolumeStruct(const VolumeGL* volume, const Identifier& textureUnitIdent,
                                           const std::string& samplerIdentifier,
                                           const std::string& volumeParametersIdentifier)
    : volume_(volume), textureUnitIdent_(textureUnitIdent),
      samplerIdentifier_(samplerIdentifier), volumeParametersIdentifier_(volumeParametersIdentifier)
{}

} // namespace voreen
