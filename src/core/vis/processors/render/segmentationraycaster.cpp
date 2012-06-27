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

#include "voreen/core/volume/modality.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"
#include "voreen/core/vis/properties/cameraproperty.h"

#include "tgt/gpucapabilities.h"

#include <sstream>
#include <cstdio>

namespace voreen {

using std::vector;
using std::map;
using tgt::vec3;
using tgt::col4;

SegmentationRaycaster::SegmentationRaycaster()
    : VolumeRaycaster()
    , camera_("camera", "Camera", new tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , transferFunc_("SegmentationRaycaster.TransFunc", "Transfer function")
    , segmentTransFuncs_(0)
    , segmentationTransFuncTex_(0)
    , segmentationTransFuncTexValid_(false)
    , segmentationHandle_(0)
    , applySegmentation_("SegmentationRaycatser.applySegmentation", "Apply Segmentation", true)
    , transFuncResolutionProp_("segmentationRaycaster.transFuncResolution", "TransFunc Resolution")
    , lastSegment_(-1)
    , compositingMode1_("compositing1", "Compositing (OP2)", Processor::INVALID_PROGRAM)
    , compositingMode2_("compositing2", "Compositing (OP3)", Processor::INVALID_PROGRAM)
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle")
    , segmentationInport_(Port::INPORT, "volumehandle.segmentation")
    , entryPort_(Port::INPORT, "image.entrypoints")
    , exitPort_(Port::INPORT, "image.exitpoints")
    , outport1_(Port::OUTPORT, "image.outport1", true, Processor::INVALID_PROGRAM)
    , outport2_(Port::OUTPORT, "image.outport2", true, Processor::INVALID_PROGRAM)
    , outport3_(Port::OUTPORT, "image.outport3", true, Processor::INVALID_PROGRAM)
{

    transferFunc_.disableEditor(TransFuncProperty::Editors(TransFuncProperty::ALL & ~TransFuncProperty::INTENSITY));
    addProperty(transferFunc_);

    applySegmentation_.onChange(CallMemberAction<SegmentationRaycaster>(this, &SegmentationRaycaster::applySegmentationChanged));
    addProperty(applySegmentation_);

    // Segment transfer functions
    std::vector<Property*> transferFunctions;
    for (int i=0; i<256; ++i) {
        std::ostringstream segmentID;
        segmentID << i;
        TransFuncProperty* tfProp = new TransFuncProperty("SegmentationRaycaster.SegmentTransFunc" + segmentID.str(), "Segment " + segmentID.str(),
            Processor::INVALID_RESULT, static_cast<TransFuncProperty::Editors>(TransFuncProperty::INTENSITY | TransFuncProperty::INTENSITY_RAMP), true);
        tfProp->onChange(Call1ParMemberAction<SegmentationRaycaster, int>(this, &SegmentationRaycaster::segmentationTransFuncChanged, i));
        transferFunctions.push_back(tfProp);
    }
    segmentTransFuncs_ = new PropertyVector("SegmentationRaycaster.SegmentTransFuncs", "Segment Transfer Functions:", transferFunctions);
    addProperty(segmentTransFuncs_);

    // segmentation transfunc horizontal resolution
    transFuncResolutionProp_.addOption("128", "128", 128);
    transFuncResolutionProp_.addOption("256", "256", 256);
    transFuncResolutionProp_.addOption("512", "512", 512);
/*    if (GpuCaps.getMaxTextureSize() >= 1024)
        transFuncResolutionProp_.addOption("1024", "1024", 1024);
    if (GpuCaps.getMaxTextureSize() >= 2048)
        transFuncResolutionProp_.addOption("2048", "2048", 2048);
    if (GpuCaps.getMaxTextureSize() >= 4096)
        transFuncResolutionProp_.addOption("4096", "4096", 4096); */
    transFuncResolutionProp_.selectByValue(256);
    transFuncResolutionProp_.onChange(CallMemberAction<SegmentationRaycaster>(this, &SegmentationRaycaster::transFuncResolutionChanged));
    addProperty(transFuncResolutionProp_);

    addProperty(gradientMode_);
    //addProperty(classificationMode_);
    addProperty(shadeMode_);
    addProperty(compositingMode_);

    compositingMode1_.addOption("dvr", "DVR");
    compositingMode1_.addOption("mip", "MIP");
    compositingMode1_.addOption("iso", "ISO");
    compositingMode1_.addOption("fhp", "FHP");
    compositingMode1_.addOption("fhn", "FHN");
    addProperty(compositingMode1_);

    compositingMode2_.addOption("dvr", "DVR");
    compositingMode2_.addOption("mip", "MIP");
    compositingMode2_.addOption("iso", "ISO");
    compositingMode2_.addOption("fhp", "FHP");
    compositingMode2_.addOption("fhn", "FHN");
    addProperty(compositingMode2_);

    addProperty(camera_);

    addProperty(lightPosition_);
    addProperty(lightAmbient_);
    addProperty(lightDiffuse_);
    addProperty(lightSpecular_);

    destActive_[0] = false;
    destActive_[1] = false;
    destActive_[2] = false;

    tm_.addTexUnit("segmentationTransferTexUnit");

    addPort(volumeInport_);
    addPort(segmentationInport_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport1_);
    addPort(outport2_);
    addPort(outport3_);

}

SegmentationRaycaster::~SegmentationRaycaster() {
    delete segmentationTransFuncTex_;
    delete segmentTransFuncs_;
}

BoolProperty& SegmentationRaycaster::getApplySegmentationProp() {
    return applySegmentation_;
}

const std::string SegmentationRaycaster::getProcessorInfo() const {
    return "Renders a segmented dataset. Each segment can be assigned a 1D transfer functions";
}

Processor* SegmentationRaycaster::create() const {
    return new SegmentationRaycaster();
}

void SegmentationRaycaster::initialize() throw (VoreenException) {

    VolumeRaycaster::initialize();

    loadShader();

    if (!raycastPrg_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }
    initialized_ = true;

    setLightingParameters();

    portGroup_.initialize();
    portGroup_.addPort(outport1_);
    portGroup_.addPort(outport2_);
    portGroup_.addPort(outport3_);

    createSegmentationTransFunc();
    initializeSegmentationTransFuncTex();

    invalidate(Processor::INVALID_PROGRAM);
}

void SegmentationRaycaster::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "rc_segmentation.frag",
                                       generateHeader(), false, false);
}

void SegmentationRaycaster::compile(VolumeHandle* volumeHandle) {
    raycastPrg_->setHeaders(generateHeader(volumeHandle), false);
    raycastPrg_->rebuild();
}

bool SegmentationRaycaster::isReady() const {
    //check if all inports are connected:
    if(!entryPort_.isReady() || !exitPort_.isReady() || !volumeInport_.isReady())
        return false;

    if (!outport1_.isReady() && !outport2_.isReady() && !outport3_.isReady())
        return false;

    if (applySegmentation_.get() && !segmentationInport_.isReady())
        return false;

    return true;
}

void SegmentationRaycaster::process() {
    if (segmentationInport_.hasChanged()) {
        segmentationHandle_ = segmentationInport_.getData();
        invalidate(Processor::INVALID_PROGRAM);
        applySegmentationChanged();
    }

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile(volumeInport_.getData());
    LGL_ERROR;

    portGroup_.activateTargets();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    VolumeHandle* volumeHandle = volumeInport_.getData();

    if (volumeInport_.hasChanged()) {
        transferFunc_.setVolumeHandle(volumeHandle);
    }

    if (!volumeHandle) {
        outport1_.deactivateTarget();
        glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
        LGL_ERROR;
        return;
    }

    if (applySegmentation_.get() && !segmentationHandle_) {
        glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
        LGL_ERROR;
        return;
    }

    // bind entry params
    entryPort_.bindTextures(tm_.getGLTexUnit(entryParamsTexUnit_), tm_.getGLTexUnit(entryParamsDepthTexUnit_));
    LGL_ERROR;

    // bind exit params
    exitPort_.bindTextures(tm_.getGLTexUnit(exitParamsTexUnit_), tm_.getGLTexUnit(exitParamsDepthTexUnit_));
    LGL_ERROR;

    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

    // add main volume
    volumeTextures.push_back(VolumeStruct(
        volumeHandle->getVolumeGL(),
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
        segVolume->getTexture()->bind();
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

    updateBrickingParameters(volumeInport_.getData());
    addBrickedVolumeModalities(volumeInport_.getData(), volumeTextures);

    // initialize shader
    raycastPrg_->activate();

    // set common uniforms used by all shaders
    setGlobalShaderParameters(raycastPrg_, camera_.get());
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(raycastPrg_, volumeTextures);

    // pass the remaining uniforms to the shader
    raycastPrg_->setUniform("entryPoints_", tm_.getTexUnit(entryParamsTexUnit_));
    raycastPrg_->setUniform("entryPointsDepth_", tm_.getTexUnit(entryParamsDepthTexUnit_));
    entryPort_.setTextureParameters(raycastPrg_, "entryParameters_");
    raycastPrg_->setUniform("exitPoints_", tm_.getTexUnit(exitParamsTexUnit_));
    raycastPrg_->setUniform("exitPointsDepth_", tm_.getTexUnit(exitParamsDepthTexUnit_));
    exitPort_.setTextureParameters(raycastPrg_, "exitParameters_");

    // assign segment tf to texture units
    if (segVolume && applySegmentation_.get())
        raycastPrg_->setUniform("segmentationTransferFunc_", tm_.getTexUnit("segmentationTransferTexUnit"));
    else
        raycastPrg_->setUniform("transferFunc_", tm_.getTexUnit("transferTexUnit"));

    setBrickedVolumeUniforms(volumeInport_.getData());
    setLightingParameters();

    renderQuad();

    raycastPrg_->deactivate();

    // restore default texture filtering mode
    glActiveTexture(tm_.getGLTexUnit(segmentationTexUnit_));
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

std::string SegmentationRaycaster::generateHeader(VolumeHandle* volumeHandle) {

    std::string headerSource = VolumeRaycaster::generateHeader(volumeHandle);

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
    if (compositingMode1_.get() == "dvr")
        headerSource += "compositeDVR(result, color, t, tDepth);\n";
    else if (compositingMode1_.get() == "mip")
        headerSource += "compositeMIP(result, color, t, tDepth);\n";
    else if (compositingMode1_.get() == "iso")
        headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
    else if (compositingMode1_.get() == "fhp")
        headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
    else if (compositingMode1_.get() == "fhn")
        headerSource += "compositeFHN(gradient, result, t, tDepth);\n";

    // configure compositing mode for port 3
    headerSource += "#define RC_APPLY_COMPOSITING_2(result, color, samplePos, gradient, t, tDepth) ";
    if (compositingMode2_.get() == "dvr")
        headerSource += "compositeDVR(result, color, t, tDepth);\n";
    else if (compositingMode2_.get() == "mip")
        headerSource += "compositeMIP(result, color, t, tDepth);\n";
    else if (compositingMode2_.get() == "iso")
        headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
    else if (compositingMode2_.get() == "fhp")
        headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
    else if (compositingMode2_.get() == "fhn")
        headerSource += "compositeFHN(gradient, result, t, tDepth);\n";

    portGroup_.reattachTargets();
    headerSource += portGroup_.generateHeader();
    return headerSource;
}

void SegmentationRaycaster::segmentationTransFuncChanged(int segment) {
    if (segmentationTransFuncTex_)
        updateSegmentationTransFuncTex(segment);
}

void SegmentationRaycaster::updateSegmentationTransFuncTex(int segment) {

        TransFuncIntensity* intensityTF = dynamic_cast<TransFuncIntensity*>(segmentTransFuncs_->getProperty<TransFuncProperty*>(segment)->get());
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

TransFuncProperty& SegmentationRaycaster::getTransFunc() {
    return transferFunc_;
}


void SegmentationRaycaster::transFuncResolutionChanged() {
    createSegmentationTransFunc();
    initializeSegmentationTransFuncTex();
}

void SegmentationRaycaster::createSegmentationTransFunc() {

    delete segmentationTransFuncTex_;

    int hres = transFuncResolutionProp_.getValue();
    int vres = std::min(GpuCaps.getMaxTextureSize(), segmentTransFuncs_->size() * 4);

    segmentationTransFuncTex_ = new tgt::Texture(tgt::ivec3(hres, vres, 1));
    segmentationTransFuncTex_->setFilter(tgt::Texture::LINEAR);

}

void SegmentationRaycaster::initializeSegmentationTransFuncTex() {

    for (int i=0; i<segmentTransFuncs_->size(); ++i)
        updateSegmentationTransFuncTex(i);

    segmentationTransFuncTex_->uploadTexture();

    segmentationTransFuncTexValid_ = true;

}

void SegmentationRaycaster::applySegmentationChanged() {
    invalidate(Processor::INVALID_PROGRAM);
}

const PropertyVector& SegmentationRaycaster::getSegmentationTransFuncs()
{
    return *segmentTransFuncs_;
}

} // namespace voreen
