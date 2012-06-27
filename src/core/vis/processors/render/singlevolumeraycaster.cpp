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

#include "voreen/core/vis/processors/render/singlevolumeraycaster.h"

#include "voreen/core/vis/lightmaterial.h"

#include <sstream>

namespace voreen {

SingleVolumeRaycaster::SingleVolumeRaycaster()
    : VolumeRaycaster()
    , transferFunc_(setTransFunc_, "Transfer function")
{
    setName("SingleVolumeRaycaster");

    addProperty(&transferFunc_);
    addProperty(maskingMode_);
    addProperty(gradientMode_);
    addProperty(classificationMode_);
    addProperty(shadeMode_);
    addProperty(compositingMode_);

    compositingMode1_ = new EnumProp("set.compositing1", "Compositing (OP2)", compositingModes_, 0, true, true);
    addProperty(compositingMode1_);

    compositingMode2_ = new EnumProp("set.compositing2", "Compositing (OP3)", compositingModes_, 0, true, true);
    addProperty(compositingMode2_);

    addProperty(&lightPosition_);
    addProperty(&lightAmbient_);
    addProperty(&lightDiffuse_);
    addProperty(&lightSpecular_);

    destActive_[0] = false;
    destActive_[1] = false;
    destActive_[2] = false;

    createInport("volumehandle.volumehandle");
    createInport("image.entrypoints");
    createInport("image.exitpoints");
    createOutport("image.output");
    createOutport("image.output1");
    createOutport("image.output2");
}

SingleVolumeRaycaster::~SingleVolumeRaycaster() {
}

const std::string SingleVolumeRaycaster::getProcessorInfo() const {
    return "Performs a simple single pass raycasting with only some capabilites.";
}

void SingleVolumeRaycaster::processMessage(Message* msg, const Identifier& dest) {
    VolumeRaycaster::processMessage(msg, dest);
    // send invalidate and update context, if lighting parameters have changed
    if (msg->id_ == LightMaterial::setLightPosition_   ||
        msg->id_ == LightMaterial::setLightAmbient_         ||
        msg->id_ == LightMaterial::setLightDiffuse_         ||
        msg->id_ == LightMaterial::setLightSpecular_        ||
        msg->id_ == LightMaterial::setLightAttenuation_     ||
        msg->id_ == LightMaterial::setMaterialAmbient_      ||
        msg->id_ == LightMaterial::setMaterialDiffuse_      ||
        msg->id_ == LightMaterial::setMaterialSpecular_     ||
        msg->id_ == LightMaterial::setMaterialShininess_        ) {
            invalidate();
    }
}

int SingleVolumeRaycaster::initializeGL() {
    loadShader();
    initStatus_ = raycastPrg_ ? VRN_OK : VRN_ERROR;

    return initStatus_;
}

void SingleVolumeRaycaster::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "rc_singlevolume.frag",
                                       generateHeader(), false);
}

void SingleVolumeRaycaster::compile() {
    raycastPrg_->setHeaders(generateHeader(), false);
    raycastPrg_->rebuild();
}

void SingleVolumeRaycaster::process(LocalPortMapping* portMapping) {

    int entryParams = portMapping->getTarget("image.entrypoints");
    int exitParams = portMapping->getTarget("image.exitpoints");

    std::vector<int> activeTargets;
    try {
        int dest0 = portMapping->getTarget("image.output");
        if (!destActive_[0]) {
            // first outport was not enabled in shader => recompile shader now
            destActive_[0] = true;
            invalidateShader();
        }
        activeTargets.push_back(dest0);
    } catch (std::exception& ) {
        if (destActive_[0]) {
            // no first outport target, but it was enabled in shader => recompile
            destActive_[0] = false;
            invalidateShader();
        }
    }
    try {
        int dest1 = portMapping->getTarget("image.output1");
        if (!destActive_[1]) {
            // second outport was not enabled in shader => recompile shader now
            destActive_[1] = true;
            invalidateShader();
        }
        activeTargets.push_back(dest1);
    } catch (std::exception& ) {
        if (destActive_[1]) {
            // no second outport target, but it was enabled in shader => recompile
            destActive_[1] = false;
            invalidateShader();
        }
    }
    try {
        int dest2 = portMapping->getTarget("image.output2");
        if (!destActive_[2]) {
            // writing first hit normals was not enabled in shader => recompile shader now
            destActive_[2] = true;
            invalidateShader();
        }
        activeTargets.push_back(dest2);
    } catch (std::exception& ) {
        if (destActive_[2]) {
            // no firstHitPoints target, but writing FHP are enabled in shader => recompile
            destActive_[2] = false;
            invalidateShader();
        }
    }

    tc_->setActiveTargets(activeTargets, "SingleVolumeRaycaster");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!VolumeHandleValidator::checkVolumeHandle(currentVolumeHandle_,
                                                  portMapping->getVolumeHandle("volumehandle.volumehandle")))
    {
        return;
    }

    transferFunc_.setVolumeHandle(currentVolumeHandle_);
    
    // compile program if needed
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
    VolumeHandle* volumeSeg = currentVolumeHandle_->getRelatedVolumeHandle(Modality::MODALITY_SEGMENTATION);

    bool usingSegmentation = (maskingMode_->get() == 1) && volumeSeg;
    if (usingSegmentation) {
        // Important to set the correct texture unit before getVolumeGL() is called or
        // glTexParameter() might influence the wrong texture.
        glActiveTexture(tm_.getGLTexUnit(segmentationTexUnit_));

        volumeTextures.push_back(VolumeStruct(volumeSeg->getVolumeGL(),
                                              segmentationTexUnit_,
                                              "segmentation_",
                                              "segmentationParameters_"));

        // set texture filtering for this texture unit
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
    if (classificationMode_->get() == 1)
        raycastPrg_->setUniform("transferFunc_", tm_.getTexUnit(transferTexUnit_));

    if (usingSegmentation) {
        GLfloat seg = segment_.get() / 255.f;
        raycastPrg_->setUniform("segment_" , seg);
    }


    glPushAttrib(GL_LIGHTING_BIT);
    setLightingParameters();

    renderQuad();

    glPopAttrib();
    raycastPrg_->deactivate();

    if (usingSegmentation) {
        // restore default texture filtering mode
        glActiveTexture(tm_.getGLTexUnit(segmentationTexUnit_));
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

std::string SingleVolumeRaycaster::generateHeader() {
    std::string headerSource = VolumeRaycaster::generateHeader();

    headerSource += transferFunc_.get()->getShaderDefines();

    // configure compositing mode for port 2
    headerSource += "#define RC_APPLY_COMPOSITING_1(result, color, samplePos, gradient, t, tDepth) ";
    switch (compositingMode1_->get()) {
        case 0: headerSource += "compositeDVR(result, color, t, tDepth);\n";
            break;
        case 1: headerSource += "compositeMIP(result, color, t, tDepth);\n";
            break;
        case 2: headerSource += "compositeISO(result, color, t, tDepth, 0.5);\n";
            break;
        case 3: headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
            break;
        case 4: headerSource += "compositeFHN(gradient, result, t, tDepth);\n";
            break;
    }

    // configure compositing mode for port 3
    headerSource += "#define RC_APPLY_COMPOSITING_2(result, color, samplePos, gradient, t, tDepth) ";
    switch (compositingMode2_->get()) {
        case 0: headerSource += "compositeDVR(result, color, t, tDepth);\n";
            break;
        case 1: headerSource += "compositeMIP(result, color, t, tDepth);\n";
            break;
        case 2: headerSource += "compositeISO(result, color, t, tDepth, 0.5);\n";
            break;
        case 3: headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
            break;
        case 4: headerSource += "compositeFHN(gradient, result, t, tDepth);\n";
            break;
    }

    // map ports to render targets
    int active = 0;
    for (int i=0; i < 3; i++) {
        std::ostringstream op, num;
        op << i;
        num << active;
        if (destActive_[i]) {
            headerSource += "#define OP" + op.str() + " " + num.str() + "\n";
            active++;
        }
    }

    return headerSource;
}

} // namespace
