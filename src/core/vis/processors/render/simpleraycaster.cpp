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

#include "voreen/core/vis/processors/render/simpleraycaster.h"

namespace voreen {

SimpleRaycaster::SimpleRaycaster()
    : VolumeRaycaster()
    , transferFunc_(setTransFunc_, "Transfer Function")
{
    setName("SimpleRaycaster");

    addProperty(&transferFunc_);

    createInport("volumehandle.volumehandle");
    createInport("image.entrypoints");
    createInport("image.exitpoints");
    createOutport("image.output");
}

SimpleRaycaster::~SimpleRaycaster() {
}

const std::string SimpleRaycaster::getProcessorInfo() const {
    return "Performs a simple single pass raycasting with only some capabilites.";
}

int SimpleRaycaster::initializeGL() {
    loadShader();
    initStatus_ = raycastPrg_ ? VRN_OK : VRN_ERROR;

    return initStatus_;
}

void SimpleRaycaster::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "rc_simple.frag",
                                       generateHeader(), false);
}

void SimpleRaycaster::compile() {
    if (!raycastPrg_)
        return;

    raycastPrg_->setHeaders(generateHeader(), false);
    raycastPrg_->rebuild();
}

void SimpleRaycaster::process(LocalPortMapping* portMapping) {

    int entryParams = portMapping->getTarget("image.entrypoints");
    int exitParams = portMapping->getTarget("image.exitpoints");

    int dest = portMapping->getTarget("image.output");
    tc_->setActiveTarget(dest,"SimpleRaycaster::image.output");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (VolumeHandleValidator::checkVolumeHandle(currentVolumeHandle_,
        portMapping->getVolumeHandle("volumehandle.volumehandle")) == false)
    {
        return;
    }

    transferFunc_.setVolumeHandle(currentVolumeHandle_);

    // compile program
    compileShader();
    LGL_ERROR;

    // bind entry params
    glActiveTexture(tm_.getGLTexUnit(entryParamsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(entryParams), tc_->getGLTexID(entryParams));
    glActiveTexture(tm_.getGLTexUnit(entryParamsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(entryParams), tc_->getGLDepthTexID(entryParams));
    LGL_ERROR;

    // bind exit params
    glActiveTexture(tm_.getGLTexUnit(exitParamsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(exitParams), tc_->getGLTexID(exitParams));
    glActiveTexture(tm_.getGLTexUnit(exitParamsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(exitParams), tc_->getGLDepthTexID(exitParams));
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

       // segmentation volume
    VolumeGL* volumeSeg = currentVolumeHandle_->getRelatedVolumeGL(Modality::MODALITY_SEGMENTATION);

    if (useSegmentation_.get() && (volumeSeg != 0) ) {
        volumeTextures.push_back(VolumeStruct(
            volumeSeg,
            segmentationTexUnit_,
            "segmentation_",
            "segmentationParameters_")
        );

        glActiveTexture(tm_.getGLTexUnit(segmentationTexUnit_));

        // set texture filters for this texture
        //FIXME: this does NOTHING! is this the right place to set filtering for segmentation?
        glPushAttrib(GL_TEXTURE_BIT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glPopAttrib();
    }

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
    raycastPrg_->setUniform("exitPoints_", tm_.getTexUnit(exitParamsTexUnit_));
    raycastPrg_->setUniform("exitPointsDepth_", tm_.getTexUnit(exitParamsDepthTexUnit_));
    raycastPrg_->setUniform("transferFunc_", tm_.getTexUnit(transferTexUnit_));
    if (useSegmentation_.get() && currentVolumeHandle_->getRelatedVolumeGL(Modality::MODALITY_SEGMENTATION))
        raycastPrg_->setUniform("segment_" , static_cast<float>(segment_.get()));

    renderQuad();

    raycastPrg_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

std::string SimpleRaycaster::generateHeader() {
    std::string header = VolumeRenderer::generateHeader();

    header += transferFunc_.get()->getShaderDefines();

    if (useSegmentation_.get() && currentVolumeHandle_->getRelatedVolumeGL(Modality::MODALITY_SEGMENTATION))
        header += "#define USE_SEGMENTATION\n";

    return header;
}

} // namespace voreen
