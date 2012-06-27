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

#include "voreen/core/vis/processors/render/segmentationraycaster.h"

#include "voreen/core/vis/lightmaterial.h"
#include "voreen/core/volume/modality.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"

#include "tgt/gpucapabilities.h"

#include <sstream>
#include <cstdio>

namespace voreen {

using std::vector;
using std::map;
using tgt::col4;

SegmentationRaycaster::SegmentationRaycaster()
    : VolumeRaycaster()
    , transferFunc_("SegmentationRaycaster.TransFunc", "Transfer function")
    , segmentationTransFuncTex_(0)
    , segmentationTransFuncTexValid_(false)
    , segmentationHandle_(0)
    , applySegmentation_("SegmentationRaycatser.applySegmentation", "Apply Segmentation", true)
    , lastSegment_(-1)
{
    setName("SegmentationRaycaster");

    transferFunc_.disableEditor(TransFuncProp::Editors(TransFuncProp::ALL & ~TransFuncProp::INTENSITY));
    addProperty(&transferFunc_);

    applySegmentation_.onChange(CallMemberAction<SegmentationRaycaster>(this, &SegmentationRaycaster::applySegmentationChanged));
    addProperty(&applySegmentation_);

    // Segment transfer functions
    std::vector<Property*> transferFunctions;
    for (int i=0; i<255; ++i) {
        std::ostringstream segmentID;
        segmentID << i;
        TransFuncProp* tfProp = new TransFuncProp("SegmentationRaycaster.SegmentTransFunc" + segmentID.str(), "Segment " + segmentID.str(), 
            true, false, static_cast<TransFuncProp::Editors>(TransFuncProp::INTENSITY | TransFuncProp::INTENSITY_RAMP), true);
        tfProp->onChange(Call1ParMemberAction<SegmentationRaycaster, int>(this, &SegmentationRaycaster::segmentationTransFuncChanged, i));
        transferFunctions.push_back(tfProp);
    }
    segmentTransFuncs_ = new PropertyVector("SegmentationRaycaster.SegmentTransFuncs", "Segment Transfer Functions:", transferFunctions);
    addProperty(segmentTransFuncs_);

    // segmentation transfunc horizontal resolution
    transFuncResolutions_.push_back("128");
    transFuncResolutions_.push_back("256");
    transFuncResolutions_.push_back("512");
    if (GpuCaps.getMaxTextureSize() >= 1024)
        transFuncResolutions_.push_back("1024");
    if (GpuCaps.getMaxTextureSize() >= 2048)
        transFuncResolutions_.push_back("2048");
    if (GpuCaps.getMaxTextureSize() >= 4096)
        transFuncResolutions_.push_back("4096");
    transFuncResolutionProp_ = new EnumProp("segmentationRaycaster.transFuncResolution",
        "TransFunc Resolution", transFuncResolutions_, 1);
    transFuncResolutionProp_->onChange(CallMemberAction<SegmentationRaycaster>(this, &SegmentationRaycaster::transFuncResolutionChanged));
    addProperty(transFuncResolutionProp_);

    addProperty(gradientMode_);
    //addProperty(classificationMode_);
    addProperty(shadeMode_);
    addProperty(compositingMode_);

    std::vector<std::string> compositingModes;
    compositingModes.push_back("DVR");
    compositingModes.push_back("MIP");
    compositingModes.push_back("ISO");
    compositingModes.push_back("FHP");
    compositingModes.push_back("FHN");
    compositingMode1_ = new EnumProp("set.compositing1", "Compositing (OP2)", compositingModes, 0, true, true);
    addProperty(compositingMode1_);

    compositingMode2_ = new EnumProp("set.compositing2", "Compositing (OP3)", compositingModes, 0, true, true);
    addProperty(compositingMode2_);

    addProperty(&lightPosition_);
    addProperty(&lightAmbient_);
    addProperty(&lightDiffuse_);
    addProperty(&lightSpecular_);

    destActive_[0] = false;
    destActive_[1] = false;
    destActive_[2] = false;

    tm_.addTexUnit("segmentationTransferTexUnit");

    createInport("volumehandle.volume");
    createInport("volumehandle.segmentation");
    createInport("image.entrypoints");
    createInport("image.exitpoints");
    createOutport("image.output");
    createOutport("image.output1");
    createOutport("image.output2");

}

SegmentationRaycaster::~SegmentationRaycaster() {

    delete segmentationTransFuncTex_;
    delete segmentTransFuncs_;
   
}

BoolProp& SegmentationRaycaster::getApplySegmentationProp() {
    return applySegmentation_;
}


const std::string SegmentationRaycaster::getProcessorInfo() const {
    return "Renders a segmented dataset. Each segment can be assigned a 1D transfer functions";
}

int SegmentationRaycaster::initializeGL() {

    loadShader();
    initStatus_ = raycastPrg_ ? VRN_OK : VRN_ERROR;

    setLightingParameters();

    createSegmentationTransFunc();
    initializeSegmentationTransFuncTex();

    invalidateShader();

    return initStatus_;
}

void SegmentationRaycaster::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "rc_segmentation.frag",
                                       generateHeader(), false);
}

void SegmentationRaycaster::compile() {
    raycastPrg_->setHeaders(generateHeader(), false);
    raycastPrg_->rebuild();
}

void SegmentationRaycaster::process(LocalPortMapping* portMapping) {

    bool volumeChanged;
    if (VolumeHandleValidator::checkVolumeHandle(currentVolumeHandle_,
            portMapping->getVolumeHandle("volumehandle.volume"), &volumeChanged) == false) {
        return;
    }
    if (volumeChanged) {
        transferFunc_.setVolumeHandle(currentVolumeHandle_);
        invalidateShader();
    }

    bool segmentationChanged;
    try {
        VolumeHandleValidator::checkVolumeHandle(segmentationHandle_,
            portMapping->getVolumeHandle("volumehandle.segmentation"), &segmentationChanged);
    }
    catch (std::exception& ) {
        if (segmentationHandle_) {
            segmentationHandle_ = 0;
            segmentationChanged = true;
        }
    }
    if (segmentationChanged) {
        applySegmentationChanged();
        invalidateShader();
    }

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

    tc_->setActiveTargets(activeTargets, "SegmentationRaycaster");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    VolumeGL* segVolume = 0;
    if (segmentationHandle_) {
        segVolume = segmentationHandle_->getVolumeGL();
    }

    if (segVolume) {
        volumeTextures.push_back(VolumeStruct(
            segVolume,
            segmentationTexUnit_,
            "segmentation_",
            "segmentationParameters_")
        );

        glActiveTexture(tm_.getGLTexUnit(segmentationTexUnit_));

        // set texture filtering for this texture unit
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    if (segVolume && applySegmentation_.get()) {
        glActiveTexture(tm_.getGLTexUnit("segmentationTransferTexUnit"));
        segmentationTransFuncTex_->bind();
        if (!segmentationTransFuncTexValid_) {
            segmentationTransFuncTex_->uploadTexture();
            segmentationTransFuncTexValid_ = true;
        }
    }
    else {
        glActiveTexture(tm_.getGLTexUnit("transferTexUnit"));
        transferFunc_.get()->bind();
    }

    addBrickedVolumeModalities(volumeTextures);


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

    // assign segment tf to texture units
    if (segVolume && applySegmentation_.get())
        raycastPrg_->setUniform("segmentationTransferFunc_", tm_.getTexUnit("segmentationTransferTexUnit"));
    else
        raycastPrg_->setUniform("transferFunc_", tm_.getTexUnit("transferTexUnit"));

    setBrickedVolumeUniforms();

    renderQuad();

    raycastPrg_->deactivate();

    // restore default texture filtering mode
    glActiveTexture(tm_.getGLTexUnit(segmentationTexUnit_));
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

std::string SegmentationRaycaster::generateHeader() {

    std::string headerSource = VolumeRaycaster::generateHeader(getVolumeHandle());

    if (transferFunc_.get())
        headerSource += transferFunc_.get()->getShaderDefines();

    if (applySegmentation_.get() && segmentationHandle_) {
        headerSource += "#define MOD_APPLY_SEGMENTATION\n";
        std::ostringstream tfHeight;
        tfHeight << segmentationTransFuncTex_->getHeight();
        headerSource += "#define SEGMENTATION_TRANSFUNC_HEIGHT " + tfHeight.str() + "\n";
    }

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

void SegmentationRaycaster::segmentationTransFuncChanged(int segment) {
    if (segmentationTransFuncTex_)
        updateSegmentationTransFuncTex(segment);
}

void SegmentationRaycaster::updateSegmentationTransFuncTex(int segment) {

    
        //TransFuncIntensity* intensityTF = dynamic_cast<TransFuncIntensity*>(transferFunc_.get());
        TransFuncIntensity* intensityTF = dynamic_cast<TransFuncIntensity*>(segmentTransFuncs_->getProperty<TransFuncProp>(segment)->get());
        if (!intensityTF) {
            LERROR("1D transfer function expected");
            return;
        }

        if (!segmentationTransFuncTex_) {
            LWARNING("No segmentation transfer function texture");
            return;
        }

        if (segment < 0 || segment >= tgt::ifloor(segmentationTransFuncTex_->getHeight() / 3.f)) {
            LWARNING("Invalid segment id");
            return;
        }

        // A segment's 1D transfer function is stored in the 2D segmentation tf texture as a
        // 3-row wide stripe which is centered around the row 3*i+1.

        intensityTF->updateTexture();
        tgt::Texture* tfTex1D = intensityTF->getTexture();
        // copy the content of the segment's transfer function to the combined 2D tf texture
        int tfRow = 3*segment+1;
        float intensityScale = static_cast<float>(tfTex1D->getWidth()) / static_cast<float>(segmentationTransFuncTex_->getWidth());
        for (int j=0; j<segmentationTransFuncTex_->getWidth(); ++j) {
            tgt::col4 texel1D = tfTex1D->texel<tgt::col4>(tgt::ifloor(j*intensityScale));
            segmentationTransFuncTex_->texel<tgt::col4>(j,tfRow-1) = texel1D;
            segmentationTransFuncTex_->texel<tgt::col4>(j,tfRow) = texel1D;
            segmentationTransFuncTex_->texel<tgt::col4>(j,tfRow+1) = texel1D;
        }

        // transfer func texture has to be uploaded before next rendering pass
        segmentationTransFuncTexValid_ = false; 

}

TransFuncProp& SegmentationRaycaster::getTransFunc() {
    return transferFunc_;
}


void SegmentationRaycaster::transFuncResolutionChanged() {
    createSegmentationTransFunc();
    initializeSegmentationTransFuncTex();
}

void SegmentationRaycaster::createSegmentationTransFunc() {

    delete segmentationTransFuncTex_;

    int hres = 256;
    switch (transFuncResolutionProp_->get()) {
        case 0:
            hres = 128;
            break;
        case 1:
            hres = 256;
            break;
        case 2:
            hres = 512;
            break;
        case 3:
            hres = 1024;
            break;
        case 4:
            hres = 2048;
            break;
        case 5:
            hres = 4096;
            break;
    }

    int vres = std::min(GpuCaps.getMaxTextureSize(), segmentTransFuncs_->getNumProperties() * 4);

    segmentationTransFuncTex_ = new tgt::Texture(tgt::ivec3(hres, vres, 1));
    segmentationTransFuncTex_->setFilter(tgt::Texture::LINEAR);
    
}

void SegmentationRaycaster::initializeSegmentationTransFuncTex() {

    for (int i=0; i<segmentTransFuncs_->getNumProperties(); ++i)
        updateSegmentationTransFuncTex(i);

    segmentationTransFuncTex_->uploadTexture();

    segmentationTransFuncTexValid_ = true;
    
}

void SegmentationRaycaster::applySegmentationChanged() {
    invalidateShader();
}

PropertyVector& SegmentationRaycaster::getSegmentationTransFuncs() {
    return *segmentTransFuncs_;
}

} // namespace voreen
