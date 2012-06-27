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

#include "voreen/core/vis/processors/render/firsthitrenderer.h"

#include "voreen/core/volume/modality.h"

namespace voreen {

FirstHitRenderer::FirstHitRenderer()
    : VolumeRaycaster()
    , transferFunc_(setTransFunc_, "Transfer Function")
{
    setName("FirstHitRenderer");

    addProperty(&transferFunc_);

    addProperty(gradientMode_);
    addProperty(classificationMode_);
    addProperty(shadeMode_);

    createInport("volumehandle.volumehandle");
    createInport("image.entrypoints");
    createOutport("image.output");
}

FirstHitRenderer::~FirstHitRenderer() {
}

const std::string FirstHitRenderer::getProcessorInfo() const {
    return "Performs a first hit rendering.";
}

int FirstHitRenderer::initializeGL() {
    loadShader();
    initStatus_ = raycastPrg_ ? VRN_OK : VRN_ERROR;

    setLightingParameters();

    return initStatus_;
}

void FirstHitRenderer::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "rc_firsthit.frag",
                                       generateHeader(), false);
}

void FirstHitRenderer::compile() {
    raycastPrg_->setHeaders(generateHeader(), false);
    raycastPrg_->rebuild();
}

void FirstHitRenderer::process(LocalPortMapping* portMapping) {
    if (VolumeHandleValidator::checkVolumeHandle(currentVolumeHandle_,
        portMapping->getVolumeHandle("volumehandle.volumehandle")) == false)
    {
        return;
    }

    transferFunc_.setVolumeHandle(currentVolumeHandle_);

    int entryParams = portMapping->getTarget("image.entrypoints");

    tc_->setActiveTarget(portMapping->getTarget("image.output"));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //FIXME: is this needed? (tr)

    // compile program
    compileShader();
    LGL_ERROR;

    // bind entry params
    glActiveTexture(tm_.getGLTexUnit(entryParamsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(entryParams), tc_->getGLTexID(entryParams));
    glActiveTexture(tm_.getGLTexUnit(entryParamsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(entryParams), tc_->getGLDepthTexID(entryParams));
    LGL_ERROR;

    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

    // add main volume
    volumeTextures.push_back(VolumeStruct(
        currentVolumeHandle_->getVolumeGL(),
        volTexUnit_,
        "volume_",
        "volumeParameters_")
    );

    // bind transfer function
    glActiveTexture(tm_.getGLTexUnit(transferTexUnit_));
    if (transferFunc_.get())
        transferFunc_.get()->bind();

    // initialize shader
    raycastPrg_->activate();

    // set common uniforms used by all shaders
    setGlobalShaderParameters(raycastPrg_);
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(raycastPrg_, volumeTextures);

    // pass the remaining uniforms to the shader
    raycastPrg_->setUniform("entryPoints_", tm_.getTexUnit(entryParamsTexUnit_));
    raycastPrg_->setUniform("entryPointsDepth_", tm_.getTexUnit(entryParamsDepthTexUnit_));
    raycastPrg_->setUniform("transferFunc_", tm_.getTexUnit(transferTexUnit_));

    renderQuad();

    raycastPrg_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

std::string FirstHitRenderer::generateHeader() {
    std::string headerSource = VolumeRaycaster::generateHeader();

    headerSource += transferFunc_.get()->getShaderDefines();

    return headerSource;
}

} // namespace voreen
