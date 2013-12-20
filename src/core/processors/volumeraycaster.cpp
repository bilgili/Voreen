/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/core/processors/volumeraycaster.h"
#include "voreen/core/utils/classificationmodes.h"
#include "voreen/core/utils/glsl.h"
#include "voreen/core/properties/cameraproperty.h"

#include "tgt/textureunit.h"
#include "tgt/vector.h"

using tgt::mat4;
using tgt::TextureUnit;

namespace voreen {

const std::string VolumeRaycaster::loggerCat_("voreen.VolumeRaycaster");

VolumeRaycaster::VolumeRaycaster()
    : VolumeRenderer()
    , samplingRate_("samplingRate", "Sampling Rate", 2.f, 0.01f, 20.f)
    , isoValue_("isoValue", "Iso Value", 0.5f, 0.0f, 1.0f)
    , maskingMode_("masking", "Masking", Processor::INVALID_PROGRAM)
    , gradientMode_("gradient", "Gradient Calculation", Processor::INVALID_PROGRAM)
    , classificationMode_("classification", "Classification", Processor::INVALID_PROGRAM)
    , shadeMode_("shading", "Shading", Processor::INVALID_PROGRAM)
    , compositingMode_("compositing", "Compositing", Processor::INVALID_PROGRAM)
    , interactionCoarseness_("interactionCoarseness","Interaction Coarseness", 3, 1, 8, Processor::VALID)
    , interactionQuality_("interactionQuality","Interaction Quality", 1.0f, 0.01f, 1.0f, Processor::VALID)
    , useInterpolationCoarseness_("interpolation.coarseness","Use Interpolation Coarseness", false, Processor::INVALID_PROGRAM)
{
    addProperty(samplingRate_);

    // initialization of the rendering properties
    // the properties are added in the respective subclasses
    maskingMode_.addOption("none", "none");
    maskingMode_.addOption("Segmentation", "Segmentation");

    gradientMode_.addOption("none",                 "none"                  );
    gradientMode_.addOption("forward-differences",  "Forward Differences"   );
    gradientMode_.addOption("central-differences",  "Central Differences"   );
    gradientMode_.addOption("sobel",                "Sobel"                 );
    gradientMode_.addOption("filtered",             "Filtered"              );
    gradientMode_.select("central-differences");

    ClassificationModes::fillProperty(&classificationMode_);

    fillShadingModesProperty(shadeMode_);

    compositingMode_.addOption("dvr", "DVR");
    compositingMode_.addOption("mip", "MIP");
    compositingMode_.addOption("mida", "MIDA");
    compositingMode_.addOption("iso", "ISO");
    compositingMode_.addOption("fhp", "FHP");
    compositingMode_.addOption("fhn", "FHN");

    addProperty(useInterpolationCoarseness_);
    addProperty(interactionCoarseness_);
    addProperty(interactionQuality_);
}

void VolumeRaycaster::initialize() throw (tgt::Exception) {
    VolumeRenderer::initialize();

    // load rescale shader program
    rescaleShader_ = ShdrMgr.loadSeparate("passthrough.vert", "copyimage.frag", generateHeader(), false);

    // if no camera property has been passed to the light property until now, try to find one automatically
    if(!lightPosition_.hasCamera())
        lightPosition_.getCamera();
}

void VolumeRaycaster::deinitialize() throw (tgt::Exception) {
    ShdrMgr.dispose(rescaleShader_);
    rescaleShader_ = 0;

    VolumeRenderer::deinitialize();
}

/*
    further methods
*/
std::string VolumeRaycaster::generateHeader(const tgt::GpuCapabilities::GlVersion* version) {
    std::string headerSource = VolumeRenderer::generateHeader(version);

    // configure gradient calculation
    headerSource += "#define CALC_GRADIENT(volume, volumeStruct, samplePos) ";
    if (gradientMode_.isSelected("none"))
        headerSource += "(voxel.xyz-vec3(0.5))*2.0;\n";
    else if (gradientMode_.isSelected("forward-differences"))
        headerSource += "calcGradientAFD(volume, volumeStruct, samplePos);\n";
    else if (gradientMode_.isSelected("central-differences"))
        headerSource += "calcGradientA(volume, volumeStruct, samplePos);\n";
    else if (gradientMode_.isSelected("filtered"))
        headerSource += "calcGradientFiltered(volume, volumeStruct, samplePos);\n";
    else if (gradientMode_.isSelected("sobel"))
        headerSource += "calcGradientSobel(volume, volumeStruct, samplePos);\n";

    // configure classification
    headerSource += ClassificationModes::getShaderDefineFunction(classificationMode_.get());

    // configure shading mode
    headerSource += getShaderDefine(shadeMode_.get(), "APPLY_SHADING");

    // configure compositing mode
    headerSource += "#define RC_APPLY_COMPOSITING(result, color, samplePos, gradient, t, samplingStepSize, tDepth) ";
    if (compositingMode_.isSelected("dvr"))
        headerSource += "compositeDVR(result, color, t, samplingStepSize, tDepth);\n";
    else if (compositingMode_.isSelected("mip"))
        headerSource += "compositeMIP(result, color, t, tDepth);\n";
    else if (compositingMode_.isSelected("mida"))
        headerSource += "compositeMIDA(result, voxel, color, f_max_i, t, samplingStepSize, tDepth, gammaValue_);\n";
    else if (compositingMode_.isSelected("iso"))
        headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
    else if (compositingMode_.isSelected("fhp"))
        headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
    else if (compositingMode_.isSelected("fhn"))
        headerSource += "compositeFHN(gradient, result, t, tDepth);\n";

    if (applyLightAttenuation_.get())
        headerSource += "#define PHONG_APPLY_ATTENUATION\n";

    return headerSource;
}

void VolumeRaycaster::setGlobalShaderParameters(tgt::Shader* shader, tgt::Camera* camera, tgt::ivec2 screenDim) {
    VolumeRenderer::setGlobalShaderParameters(shader, camera, screenDim);

    shader->setIgnoreUniformLocationError(true);

    // provide values needed for correct depth value calculation
    if (camera) {
        float n = camera->getNearDist();
        float f = camera->getFarDist();
        shader->setUniform("const_to_z_e_1", 0.5f + 0.5f*((f+n)/(f-n)));
        shader->setUniform("const_to_z_e_2", ((f-n)/(f*n)));
        shader->setUniform("const_to_z_w_1", ((f*n)/(f-n)));
        shader->setUniform("const_to_z_w_2", 0.5f*((f+n)/(f-n))+0.5f);
    }

    shader->setIgnoreUniformLocationError(false);
}

void VolumeRaycaster::interactionModeToggled() {
    VolumeRenderer::interactionModeToggled();

    // make sure, we re-render with full resolution after switching out of interaction mode
    if (!interactionMode())
        invalidate();
}

void VolumeRaycaster::invalidate(int inv) {
    VolumeRenderer::invalidate(inv);
}

float VolumeRaycaster::getSamplingStepSize(const VolumeBase* vol) {
    tgt::ivec3 dim = vol->getDimensions();

    // use dimension with the highest resolution for calculating the sampling step size
    float samplingStepSize = 1.f / (tgt::max(dim) * samplingRate_.get());

    if (interactionMode())
        samplingStepSize /= interactionQuality_.get();

    return samplingStepSize;
}

bool VolumeRaycaster::bindVolumes(tgt::Shader* shader, const std::vector<VolumeStruct> &volumes,
        const tgt::Camera* camera, const tgt::vec4& lightPosition) {

    if (!VolumeRenderer::bindVolumes(shader, volumes, camera, lightPosition))
        return false;

    shader->setIgnoreUniformLocationError(true);

    //TODO: This uses the first volume to set the step size. Could be changed so that step
    // size is calculated for each volume, but shaders need to be adapted as well to have volume
    // parameters available in ray setup and compositing. joerg
    if (volumes.size() > 0) {
        if (!volumes[0].volume_ || !volumes[0].volume_->getRepresentation<VolumeGL>() || !volumes[0].volume_->getRepresentation<VolumeGL>()->getTexture()) {
            LWARNING("No volume texture");
        }
        else {
            shader->setUniform("samplingStepSize_", getSamplingStepSize(volumes[0].volume_));
            LGL_ERROR;
        }
    }
    shader->setIgnoreUniformLocationError(false);

    return true;
}

void VolumeRaycaster::rescaleRendering(RenderPort& srcPort, RenderPort& destPort) {
    // activate and clear output render target
    destPort.activateTarget();
    destPort.clearTarget();

    // activate shader and set uniforms
    tgtAssert(rescaleShader_, "bypass shader not loaded");
    rescaleShader_->activate();
    setGlobalShaderParameters(rescaleShader_, 0, destPort.getSize());

    // bind input rendering to texture units
    tgt::TextureUnit colorUnit, depthUnit;
    srcPort.bindTextures(colorUnit.getEnum(), depthUnit.getEnum(), GL_LINEAR);
    srcPort.setTextureParameters(rescaleShader_, "texParams_");
    rescaleShader_->setUniform("colorTex_", colorUnit.getUnitNumber());
    rescaleShader_->setUniform("depthTex_", depthUnit.getUnitNumber());

    // render screen aligned quad
    renderQuad();

    // cleanup
    rescaleShader_->deactivate();
    destPort.deactivateTarget();
    TextureUnit::setZeroUnit();

    // check for OpenGL errors
    LGL_ERROR;
}

} // namespace voreen
