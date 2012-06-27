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

#include "tgt/gpucapabilities.h"
#include "voreen/core/vis/processors/render/volumerenderer.h"
#include "voreen/core/volume/volumeset.h"

using tgt::vec3;
using tgt::vec4;
using tgt::Color;

namespace voreen {

const Identifier VolumeRenderer::setLowerThreshold_("set.lowerThreshold");
const Identifier VolumeRenderer::setUpperThreshold_("set.upperThreshold");
const Identifier VolumeRenderer::setIsoValue_("set.isoValue");
const Identifier VolumeRenderer::setTransFunc_("set.TransferFunction");
const Identifier VolumeRenderer::setTransFunc2_("set.TransferFunction2");
const Identifier VolumeRenderer::setTransFunc3_("set.TransferFunction3");

VolumeRenderer::VolumeRenderer(tgt::Camera* camera, TextureContainer* tc)
    : Processor(camera, tc)
    , lowerTH_(setLowerThreshold_, "Lower Threshold", 0.0f)
    , upperTH_(setUpperThreshold_, "Upper Threshold", 1.0f)
    , currentVolumeHandle_(0)
{}

void VolumeRenderer::processMessage(Message* msg, const Identifier& dest) {
    Processor::processMessage(msg, dest);
    if (msg->id_ == setLowerThreshold_) {
		lowerTH_.set(msg->getValue<float>());
		invalidate();
	}
	else if (msg->id_ == setUpperThreshold_) {
		upperTH_.set(msg->getValue<float>());
		invalidate();
	}
}

VolumeHandle* VolumeRenderer::getVolumeHandle() {
    return currentVolumeHandle_;
}

const VolumeHandle* VolumeRenderer::getVolumeHandle() const {
    return currentVolumeHandle_;
}

void VolumeRenderer::setVolumeHandle(VolumeHandle* const handle) {
    currentVolumeHandle_ = handle;
}

tgt::mat4 VolumeRenderer::getModelViewMatrix() const {
    if (camera_)
        return camera_->getViewMatrix();
    else
        return tgt::mat4::identity;
}

TransFunc* VolumeRenderer::getTransFunc() {
    return 0;
}

TransFunc* VolumeRenderer::getTransFunc(int i) {
    if (i == 0)
        return getTransFunc();
    else
        return 0;
}

std::string VolumeRenderer::generateHeader() {
    std::string header = Processor::generateHeader();

    if (GpuCaps.isNpotSupported())
        header += "#define VRN_TEXTURE_3D\n";
    else
        header += "#define VRN_TEXTURE_3D_SCALED\n";

    return header;
}

void VolumeRenderer::setGlobalShaderParameters(tgt::Shader* shader) {
    Processor::setGlobalShaderParameters(shader);

    int loc;
    // viewmatrix, projection matrix
    loc = shader->getUniformLocation("viewMatrix_", true);
    if (loc != -1)
        shader->setUniform(loc, camera_->getViewMatrix());
    loc = shader->getUniformLocation("projectionMatrix_", true);
    if (loc != -1)
        shader->setUniform(loc, getProjectionMatrix());
   
}

void VolumeRenderer::bindVolumes(tgt::Shader* shader, const std::vector<VolumeStruct> &volumeStructs) {
    bool texCoordScaling = ! GpuCaps.isNpotSupported();
    
    for (size_t i=0; i<volumeStructs.size(); ++i) {
        // some shortcuts
        const VolumeStruct volumeStruct = volumeStructs[i];
        const VolumeGL* volumeGL = volumeStruct.volume_;
        tgtAssert(volumeGL, "No volumeGL");       //FIXME: dont only do asserts, implement real
                                                  // error handling here and below
        const Volume* volume = volumeGL->getVolume();
        tgtAssert(volume, "No volume");
        const VolumeTexture* volumeTex = volumeGL->getTexture();
        tgtAssert(volumeTex, "No volumeTex");

        // bind volume texture and pass sampler to the shader
        GLint loc = shader->getUniformLocation(volumeStruct.samplerIdentifier_, true);
        if ((loc != -1) && (volumeStruct.textureUnitIdent_ != "")) {
            tgtAssert(volumeGL->getTexture(), "No volume texture");
            GLint texUnit = tm_.getTexUnit(volumeStruct.textureUnitIdent_);
            GLint OGLTexUnit = tm_.getGLTexUnit(volumeStruct.textureUnitIdent_);
            glActiveTexture(OGLTexUnit);
#ifdef VRN_MODULE_CLOSEUPS
		    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
#endif
            volumeTex->bind();
            
            shader->setUniform(loc, texUnit);
        }

        
        // set volume meta-data
        std::string paramsIdent = volumeStruct.volumeParametersIdentifier_;
        // volume size, i.e. dimensions of the proxy geometry in world coordinates
        loc = shader->getUniformLocation(paramsIdent + ".datasetDimensions_", true);
        if (loc != -1)
            shader->setUniform(loc, tgt::vec3(volume->getDimensions()));
        loc = shader->getUniformLocation(paramsIdent + ".datasetDimensionsRCP_", true);
        if (loc != -1)
            shader->setUniform(loc, vec3(1.f) / tgt::vec3(volume->getDimensions()));

        // volume spacing, i.e. voxel size
        loc = shader->getUniformLocation(paramsIdent + ".datasetSpacing_", true);
        if (loc != -1)
            shader->setUniform(loc, volume->getSpacing());
        loc = shader->getUniformLocation(paramsIdent + ".datasetSpacingRCP_", true);
        if (loc != -1)
            shader->setUniform(loc, vec3(1.f) / volume->getSpacing());

        // volume's size in its object coordinates
        loc = shader->getUniformLocation(paramsIdent + ".volumeCubeSize_", true);
        if (loc != -1)
            shader->setUniform(loc, volume->getCubeSize());
        loc = shader->getUniformLocation(paramsIdent + ".volumeCubeSizeRCP_", true);
        if (loc != -1)
            shader->setUniform(loc, vec3(1.f) / volume->getCubeSize());

        // scaling of texture coords, if a resize of a npot texture to pot dimensions was necessary
        if (texCoordScaling) {
            // we are only interested in the scaling part of the texture matrix
            vec3 texScaleVector = volumeTex->getMatrix().getScalingPart();
            loc = shader->getUniformLocation(paramsIdent + ".texCoordScaleFactor_", true);
            if (loc != -1)
                shader->setUniform(loc, texScaleVector);
            loc = shader->getUniformLocation(paramsIdent + ".texCoordScaleFactorRCP_", true);
            if (loc != -1)
                shader->setUniform(loc, vec3(1.f) / texScaleVector);
        
        }

        // is the volume a 12 bit volume
        loc = shader->getUniformLocation(paramsIdent + ".bitDepthScale_", true);
        if (loc != -1) {
			if (volume->getBitsStored()==12)
				shader->setUniform(loc,  16.0f);
			else
				shader->setUniform(loc,  1.0f);
        }
    }

    LGL_ERROR;
}

VolumeRenderer::VolumeStruct::VolumeStruct() {
    volume_ = NULL;
}

float VolumeRenderer::getLowerThreshold() const {
    return lowerTH_.get();
}

float VolumeRenderer::getUpperThreshold() const {
    return upperTH_.get();
}

VolumeRenderer::VolumeStruct::VolumeStruct(const VolumeGL* volume,
                                           Identifier textureUnitIdent,
                                           std::string samplerIdentifier,
                                           std::string volumeParametersIdentifier)
{
    volume_ = volume;
    textureUnitIdent_ = textureUnitIdent;
    samplerIdentifier_ = samplerIdentifier;
    volumeParametersIdentifier_ = volumeParametersIdentifier;
}

} // namespace voreen
