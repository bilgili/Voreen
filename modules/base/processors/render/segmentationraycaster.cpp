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

#include "segmentationraycaster.h"

#include "voreen/core/datastructures/volume/modality.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"
#include "voreen/core/ports/conditions/portconditionvolumetype.h"

#include "tgt/gpucapabilities.h"
#include "tgt/textureunit.h"

#include <sstream>
#include <cstdio>

namespace voreen {

using std::vector;
using std::map;
using tgt::vec3;
using tgt::col4;
using tgt::TextureUnit;

SegmentationRaycaster::SegmentationRaycaster()
    : VolumeRaycaster()
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle", "Volume Input")
    , segmentationInport_(Port::INPORT, "volumehandle.segmentation", "Segmentation Volume Input")
    , entryPort_(Port::INPORT, "image.entrypoints", "Entry-points Input", false, Processor::INVALID_RESULT)
    , exitPort_(Port::INPORT, "image.exitpoints", "Exit-points Input", false, Processor::INVALID_RESULT)
    , outport_(Port::OUTPORT, "image.outport1", "Image Output", true, Processor::INVALID_PROGRAM)
    , outport1_(Port::OUTPORT, "image.outport2", "Image1 Output", true, Processor::INVALID_PROGRAM)
    , outport2_(Port::OUTPORT, "image.outport3", "Image2 Output", true, Processor::INVALID_PROGRAM)
    , internalRenderPort_(Port::OUTPORT, "internalRenderPort", "Internal Render Port")
    , internalRenderPort1_(Port::OUTPORT, "internalRenderPort1", "Internal Render Port 1")
    , internalRenderPort2_(Port::OUTPORT, "internalRenderPort2", "Internal Render Port 2")
    , internalPortGroup_(true)
    , camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)), true)
    , transferFunc_("SegmentationRaycaster.TransFunc", "Transfer function")
    , shaderProp_("raycast.prg", "Raycasting Shader", "rc_segmentation.frag", "passthrough.vert")
    , segmentTransFuncs_(0)
    , segmentationTransFuncTex_(0)
    , segmentationTransFuncTexValid_(false)
    , segmentationHandle_(0)
    , applySegmentation_("SegmentationRaycatser.applySegmentation", "Apply Segmentation", true)
    , transFuncResolutionProp_("segmentationRaycaster.transFuncResolution", "TransFunc Resolution")
    , lastSegment_(-1)
    , compositingMode1_("compositing1", "Compositing (OP2)", Processor::INVALID_PROGRAM)
    , compositingMode2_("compositing2", "Compositing (OP3)", Processor::INVALID_PROGRAM)
{
    // ports
    volumeInport_.addCondition(new PortConditionVolumeTypeGL());
    addPort(volumeInport_);
    addPort(segmentationInport_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);
    addPort(outport1_);
    addPort(outport2_);

    // internal render destinations
    addPrivateRenderPort(internalRenderPort_);
    addPrivateRenderPort(internalRenderPort1_);
    addPrivateRenderPort(internalRenderPort2_);

    // properties
    addProperty(shaderProp_);

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
}

SegmentationRaycaster::~SegmentationRaycaster() {
    delete segmentTransFuncs_;
}

Processor* SegmentationRaycaster::create() const {
    return new SegmentationRaycaster();
}

void SegmentationRaycaster::initialize() throw (tgt::Exception) {

    VolumeRaycaster::initialize();
    compile();

    internalPortGroup_.initialize();
    internalPortGroup_.addPort(internalRenderPort_);
    internalPortGroup_.addPort(internalRenderPort1_);
    internalPortGroup_.addPort(internalRenderPort2_);

    createSegmentationTransFunc();
    initializeSegmentationTransFuncTex();

    invalidate(Processor::INVALID_PROGRAM);
}

void SegmentationRaycaster::deinitialize() throw (tgt::Exception) {
    delete segmentationTransFuncTex_;
    segmentationTransFuncTex_ = 0;
    LGL_ERROR;

    internalPortGroup_.removePort(internalRenderPort_);
    internalPortGroup_.removePort(internalRenderPort1_);
    internalPortGroup_.removePort(internalRenderPort2_);
    internalPortGroup_.deinitialize();
    LGL_ERROR;

    VolumeRaycaster::deinitialize();

    delete segmentTransFuncs_;
    segmentTransFuncs_ = 0;
}

void SegmentationRaycaster::compile() {
    shaderProp_.setHeader(generateHeader());
    shaderProp_.rebuild();
}

bool SegmentationRaycaster::isReady() const {
    //check if all inports are connected:
    if(!entryPort_.isReady() || !exitPort_.isReady() || !volumeInport_.isReady())
        return false;

    if (!outport_.isReady() && !outport1_.isReady() && !outport2_.isReady())
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
        if(volumeInport_.getData())
            camera_.adaptInteractionToScene(volumeInport_.getData()->getBoundingBox().getBoundingBox());
    }

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
    LGL_ERROR;

    const VolumeBase* volumeHandle = volumeInport_.getData();

    if (volumeInport_.hasChanged()) {
        transferFunc_.setVolumeHandle(volumeHandle);
    }

    if (!volumeHandle) {
        outport1_.deactivateTarget();
        LGL_ERROR;
        return;
    }

    if (applySegmentation_.get() && !segmentationHandle_) {
        LGL_ERROR;
        return;
    }

    // determine render size and activate internal port group
    const bool renderCoarse = interactionMode() && interactionCoarseness_.get() > 1;
    const tgt::svec2 renderSize = (renderCoarse ? (outport_.getSize() / interactionCoarseness_.get()) : outport_.getSize());
    internalPortGroup_.resize(renderSize);
    internalPortGroup_.activateTargets();
    internalPortGroup_.clearTargets();
    LGL_ERROR;

    // initialize shader
    tgt::Shader* raycastPrg = shaderProp_.getShader();
    raycastPrg->activate();
    LGL_ERROR;

    // set common uniforms used by all shaders
    tgt::Camera cam = camera_.get();
    setGlobalShaderParameters(raycastPrg, &cam, renderSize);
    LGL_ERROR;

    // bind entry/exit param textures
    tgt::TextureUnit entryUnit, entryDepthUnit, exitUnit, exitDepthUnit;
    entryPort_.bindTextures(entryUnit, entryDepthUnit, GL_NEAREST);
    raycastPrg->setUniform("entryPoints_", entryUnit.getUnitNumber());
    raycastPrg->setUniform("entryPointsDepth_", entryDepthUnit.getUnitNumber());
    entryPort_.setTextureParameters(raycastPrg, "entryParameters_");

    exitPort_.bindTextures(exitUnit, exitDepthUnit, GL_NEAREST);
    raycastPrg->setUniform("exitPoints_", exitUnit.getUnitNumber());
    raycastPrg->setUniform("exitPointsDepth_", exitDepthUnit.getUnitNumber());
    exitPort_.setTextureParameters(raycastPrg, "exitParameters_");
    LGL_ERROR;

    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

    // add main volume
    TextureUnit volTexUnit;
    volumeTextures.push_back(VolumeStruct(
        volumeHandle,
        &volTexUnit,
        "volume_","volumeStruct_")
    );

    // segmentation volume
    const VolumeGL* segVolume = 0;
    if (segmentationHandle_) {
        segVolume = segmentationHandle_->getRepresentation<VolumeGL>();
    }

    TextureUnit segUnit, segTransferUnit, transferUnit;
    if (segVolume) {
        volumeTextures.push_back(VolumeStruct(
            segmentationHandle_,
            &segUnit,
            "segmentation_","segmentationParameters_",
            GL_CLAMP_TO_EDGE,
            tgt::vec4(0.f),
            GL_NEAREST)
        );
    }

    if (segVolume && applySegmentation_.get()) {
        segTransferUnit.activate();
        segmentationTransFuncTex_->bind();
        if (!segmentationTransFuncTexValid_) {
            segmentationTransFuncTex_->uploadTexture();
            segmentationTransFuncTexValid_ = true;
        }
    }
    else {
        transferUnit.activate();
        transferFunc_.get()->bind();
    }
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(raycastPrg, volumeTextures, &cam, lightPosition_.get());

    // assign segment tf to texture units
    if (segVolume && applySegmentation_.get())
        raycastPrg->setUniform("segmentationTransferFunc_", segTransferUnit.getUnitNumber());
    else
        transferFunc_.get()->setUniform(raycastPrg, "transferFunc_", "transferFuncTex_", transferUnit.getUnitNumber());

    // perform the actual raycasting by drawing a screen-aligned quad
    renderQuad();

    raycastPrg->deactivate();
    internalPortGroup_.deactivateTargets();
    LGL_ERROR;

    // copy over rendered images from internal port group to outports,
    // thereby rescaling them to outport dimensions
    if (outport_.isConnected())
        rescaleRendering(internalRenderPort_, outport_);
    if (outport1_.isConnected())
        rescaleRendering(internalRenderPort1_, outport1_);
    if (outport2_.isConnected())
        rescaleRendering(internalRenderPort2_, outport2_);

    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

std::string SegmentationRaycaster::generateHeader() {
    std::string headerSource = VolumeRaycaster::generateHeader();

    if (transferFunc_.get())
        headerSource += transferFunc_.get()->getShaderDefines();

    if (applySegmentation_.get() && segmentationHandle_) {
        headerSource += "#define MOD_APPLY_SEGMENTATION\n";
        std::ostringstream tfHeight;
        tfHeight << segmentationTransFuncTex_->getHeight();
        headerSource += "#define SEGMENTATION_TRANSFUNC_HEIGHT " + tfHeight.str() + "\n";
    }

    // configure compositing mode for port 2
    headerSource += "#define RC_APPLY_COMPOSITING_1(result, color, samplePos, gradient, t, samplingStepSize, tDepth) ";
    if (compositingMode1_.get() == "dvr")
        headerSource += "compositeDVR(result, color, t, samplingStepSize, tDepth);\n";
    else if (compositingMode1_.get() == "mip")
        headerSource += "compositeMIP(result, color, t, tDepth);\n";
    else if (compositingMode1_.get() == "iso")
        headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
    else if (compositingMode1_.get() == "fhp")
        headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
    else if (compositingMode1_.get() == "fhn")
        headerSource += "compositeFHN(gradient, result, t, tDepth);\n";

    // configure compositing mode for port 3
    headerSource += "#define RC_APPLY_COMPOSITING_2(result, color, samplePos, gradient, t, samplingStepSize, tDepth) ";
    if (compositingMode2_.get() == "dvr")
        headerSource += "compositeDVR(result, color, t, samplingStepSize, tDepth);\n";
    else if (compositingMode2_.get() == "mip")
        headerSource += "compositeMIP(result, color, t, tDepth);\n";
    else if (compositingMode2_.get() == "iso")
        headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
    else if (compositingMode2_.get() == "fhp")
        headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
    else if (compositingMode2_.get() == "fhn")
        headerSource += "compositeFHN(gradient, result, t, tDepth);\n";

    internalPortGroup_.reattachTargets();
    headerSource += internalPortGroup_.generateHeader(shaderProp_.getShader());
    return headerSource;
}

void SegmentationRaycaster::segmentationTransFuncChanged(int segment) {
    if (segmentationTransFuncTex_)
        updateSegmentationTransFuncTex(segment);
}

void SegmentationRaycaster::updateSegmentationTransFuncTex(int segment) {

        TransFunc1DKeys* intensityTF = dynamic_cast<TransFunc1DKeys*>(segmentTransFuncs_->getProperty<TransFuncProperty*>(segment)->get());
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

const PropertyVector& SegmentationRaycaster::getSegmentationTransFuncs() {
    return *segmentTransFuncs_;
}

BoolProperty& SegmentationRaycaster::getApplySegmentationProp() {
    return applySegmentation_;
}

} // namespace voreen
