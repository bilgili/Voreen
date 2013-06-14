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

#include "singlevolumeraycaster.h"

#include "tgt/textureunit.h"
#include "voreen/core/ports/conditions/portconditionvolumetype.h"
#include "voreen/core/datastructures/transfunc/preintegrationtable.h"
#include "voreen/core/utils/classificationmodes.h"

#include <sstream>

using tgt::vec3;
using tgt::TextureUnit;

namespace voreen {

const std::string SingleVolumeRaycaster::loggerCat_("voreen.SingleVolumeRaycaster");

SingleVolumeRaycaster::SingleVolumeRaycaster()
    : VolumeRaycaster()
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle", "Volume Input", false, Processor::INVALID_PROGRAM)
    , entryPort_(Port::INPORT, "image.entrypoints", "Entry-points Input", false, Processor::INVALID_RESULT)
    , exitPort_(Port::INPORT, "image.exitpoints", "Exit-points Input", false, Processor::INVALID_RESULT)
    , outport_(Port::OUTPORT, "image.output", "Image Output", true, Processor::INVALID_PROGRAM)
    , outport1_(Port::OUTPORT, "image.output1", "Image1 Output", true, Processor::INVALID_PROGRAM)
    , outport2_(Port::OUTPORT, "image.output2", "Image2 Output", true, Processor::INVALID_PROGRAM)
    , internalRenderPort_(Port::OUTPORT, "internalRenderPort", "Internal Render Port")
    , internalRenderPort1_(Port::OUTPORT, "internalRenderPort1", "Internal Render Port 1")
    , internalRenderPort2_(Port::OUTPORT, "internalRenderPort2", "Internal Render Port 2")
    , internalPortGroup_(true)
    , shaderProp_("raycast.prg", "Raycasting Shader", "rc_singlevolume.frag", "passthrough.vert")
    , transferFunc_("transferFunction", "Transfer Function")
    , camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)), true)
    , compositingMode1_("compositing1", "Compositing (OP2)", Processor::INVALID_PROGRAM)
    , compositingMode2_("compositing2", "Compositing (OP3)", Processor::INVALID_PROGRAM)
    , gammaValue_("gammaValue", "Gamma Value (OP1)", 0, -1, 1)
    , gammaValue1_("gammaValue1", "Gamma Value (OP2)", 0, -1, 1)
    , gammaValue2_("gammaValue2", "Gamma Value (OP3)", 0, -1, 1)
{
    // ports
    volumeInport_.addCondition(new PortConditionVolumeTypeGL());
    volumeInport_.showTextureAccessProperties(true);
    addPort(volumeInport_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);
    addPort(outport1_);
    addPort(outport2_);

    // internal render destinations
    addPrivateRenderPort(internalRenderPort_);
    addPrivateRenderPort(internalRenderPort1_);
    addPrivateRenderPort(internalRenderPort2_);

    addProperty(shaderProp_);

    // shading / classification props
    addProperty(transferFunc_);
    addProperty(camera_);
    addProperty(gradientMode_);
    addProperty(classificationMode_);
    addProperty(shadeMode_);

    gammaValue_.setTracking(true);
    addProperty(gammaValue_);
    gammaValue1_.setTracking(true);
    addProperty(gammaValue1_);
    gammaValue2_.setTracking(true);
    addProperty(gammaValue2_);

    // compositing modes
    addProperty(compositingMode_);
    compositingMode1_.addOption("dvr", "DVR");
    compositingMode1_.addOption("mip", "MIP");
    compositingMode1_.addOption("mida", "MIDA");
    compositingMode1_.addOption("iso", "ISO");
    compositingMode1_.addOption("fhp", "FHP");
    compositingMode1_.addOption("fhn", "FHN");
    addProperty(compositingMode1_);

    compositingMode2_.addOption("dvr", "DVR");
    compositingMode2_.addOption("mip", "MIP");
    compositingMode2_.addOption("mida", "MIDA");
    compositingMode2_.addOption("iso", "ISO");
    compositingMode2_.addOption("fhp", "FHP");
    compositingMode2_.addOption("fhn", "FHN");
    addProperty(compositingMode2_);

    addProperty(isoValue_);

    // lighting
    addProperty(lightPosition_);
    addProperty(lightAmbient_);
    addProperty(lightDiffuse_);
    addProperty(lightSpecular_);
    addProperty(materialShininess_);
    addProperty(applyLightAttenuation_);
    addProperty(lightAttenuation_);

    // assign lighting properties to property group
    lightPosition_.setGroupID("lighting");
    lightAmbient_.setGroupID("lighting");
    lightDiffuse_.setGroupID("lighting");
    lightSpecular_.setGroupID("lighting");
    materialShininess_.setGroupID("lighting");
    applyLightAttenuation_.setGroupID("lighting");
    lightAttenuation_.setGroupID("lighting");
    setPropertyGroupGuiName("lighting", "Lighting Parameters");

    // listen to changes of properties that influence the GUI state (i.e. visibility of other props)
    classificationMode_.onChange(CallMemberAction<SingleVolumeRaycaster>(this, &SingleVolumeRaycaster::adjustPropertyVisibilities));
    shadeMode_.onChange(CallMemberAction<SingleVolumeRaycaster>(this, &SingleVolumeRaycaster::adjustPropertyVisibilities));
    compositingMode_.onChange(CallMemberAction<SingleVolumeRaycaster>(this, &SingleVolumeRaycaster::adjustPropertyVisibilities));
    compositingMode1_.onChange(CallMemberAction<SingleVolumeRaycaster>(this, &SingleVolumeRaycaster::adjustPropertyVisibilities));
    compositingMode2_.onChange(CallMemberAction<SingleVolumeRaycaster>(this, &SingleVolumeRaycaster::adjustPropertyVisibilities));
    applyLightAttenuation_.onChange(CallMemberAction<SingleVolumeRaycaster>(this, &SingleVolumeRaycaster::adjustPropertyVisibilities));
}

Processor* SingleVolumeRaycaster::create() const {
    return new SingleVolumeRaycaster();
}

void SingleVolumeRaycaster::initialize() throw (tgt::Exception) {
    VolumeRaycaster::initialize();
    compile();

    internalPortGroup_.initialize();
    internalPortGroup_.addPort(internalRenderPort_);
    internalPortGroup_.addPort(internalRenderPort1_);
    internalPortGroup_.addPort(internalRenderPort2_);

    adjustPropertyVisibilities();

    if (transferFunc_.get()) {
        transferFunc_.get()->getTexture();
        transferFunc_.get()->invalidateTexture();
    }
}

void SingleVolumeRaycaster::deinitialize() throw (tgt::Exception) {
    internalPortGroup_.deinitialize();
    internalPortGroup_.removePort(internalRenderPort_);
    internalPortGroup_.removePort(internalRenderPort1_);
    internalPortGroup_.removePort(internalRenderPort2_);
    LGL_ERROR;

    VolumeRaycaster::deinitialize();
}

void SingleVolumeRaycaster::compile() {
    shaderProp_.setHeader(generateHeader());
    shaderProp_.rebuild();
}

bool SingleVolumeRaycaster::isReady() const {
    //check if all inports are connected
    if(!entryPort_.isReady() || !exitPort_.isReady() || !volumeInport_.isReady())
        return false;

    //check if at least one outport is connected
    if (!outport_.isReady() && !outport1_.isReady() && !outport2_.isReady())
        return false;

    if(!shaderProp_.hasValidShader())
        return false;

    return true;
}

void SingleVolumeRaycaster::beforeProcess() {
    VolumeRaycaster::beforeProcess();

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM) {
        PROFILING_BLOCK("compile");
        compile();
    }
    LGL_ERROR;

    transferFunc_.setVolumeHandle(volumeInport_.getData());
    //HACK: 2D TFs use FBOs to update the texture, we trigger the update here to prevent conflicts in process()
    //if(transferFunc_.get() && dynamic_cast<TransFunc2DPrimitives*>(transferFunc_.get()))
        transferFunc_.get()->bind();

    // A new volume was loaded
    if(volumeInport_.hasChanged() && volumeInport_.hasData())
        camera_.adaptInteractionToScene(volumeInport_.getData()->getBoundingBox().getBoundingBox(), tgt::min(volumeInport_.getData()->getSpacing()));

    //if the pre-integration table is computed on the gpu, this must be done before the rendering process
    if (classificationMode_.get() == "pre-integrated-gpu")
        if (TransFunc1DKeys* tf1d = dynamic_cast<TransFunc1DKeys*>(transferFunc_.get()))
            tf1d->getPreIntegrationTable(getSamplingStepSize(volumeInport_.getData()), 0, false, true)->getTexture();

}

void SingleVolumeRaycaster::process() {

    LGL_ERROR;

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

    // bind the volumes and pass the necessary information to the shader
    TextureUnit volUnit;
    std::vector<VolumeStruct> volumeTextures;
    volumeTextures.push_back(VolumeStruct(
        volumeInport_.getData(),
        &volUnit,
        "volume_","volumeStruct_",
        volumeInport_.getTextureClampModeProperty().getValue(),
        tgt::vec4(volumeInport_.getTextureBorderIntensityProperty().get()),
        volumeInport_.getTextureFilterModeProperty().getValue())
    );
    bindVolumes(raycastPrg, volumeTextures, &cam, lightPosition_.get());
    LGL_ERROR;

    // bind transfer function
    tgt::TextureUnit transferUnit;
    transferUnit.activate();
    ClassificationModes::bindTexture(classificationMode_.get(), transferFunc_.get(), getSamplingStepSize(volumeInport_.getData()));
    LGL_ERROR;

    // pass remaining uniforms to shader
    if (compositingMode_.isSelected("iso")  ||
        compositingMode1_.isSelected("iso") ||
        compositingMode2_.isSelected("iso") )
        raycastPrg->setUniform("isoValue_", isoValue_.get());

    if (ClassificationModes::usesTransferFunction(classificationMode_.get()))
        transferFunc_.get()->setUniform(raycastPrg, "transferFunc_", "transferFuncTex_", transferUnit.getUnitNumber());

    if (compositingMode_.isSelected("mida"))
        raycastPrg->setUniform("gammaValue_", gammaValue_.get());

    if (compositingMode1_.isSelected("mida"))
        raycastPrg->setUniform("gammaValue1_", gammaValue1_.get());

    if (compositingMode2_.isSelected("mida"))
        raycastPrg->setUniform("gammaValue2_", gammaValue2_.get());

    LGL_ERROR;

    // perform the actual raycasting by drawing a screen-aligned quad
    {
        PROFILING_BLOCK("raycasting");
        renderQuad();
    }

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

std::string SingleVolumeRaycaster::generateHeader() {
    std::string headerSource = VolumeRaycaster::generateHeader();

    headerSource += ClassificationModes::getShaderDefineSamplerType(classificationMode_.get(), transferFunc_.get());

    // configure compositing mode for port 2
    headerSource += "#define RC_APPLY_COMPOSITING_1(result, color, samplePos, gradient, t, samplingStepSize, tDepth) ";
    if (compositingMode1_.isSelected("dvr"))
        headerSource += "compositeDVR(result, color, t, samplingStepSize, tDepth);\n";
    else if (compositingMode1_.isSelected("mip"))
        headerSource += "compositeMIP(result, color, t, tDepth);\n";
    else if (compositingMode1_.isSelected("mida"))
        headerSource += "compositeMIDA(result, voxel, color, f_max_i1, t, tDepth, gammaValue1_);\n";
    else if (compositingMode1_.isSelected("iso"))
        headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
    else if (compositingMode1_.isSelected("fhp"))
        headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
    else if (compositingMode1_.isSelected("fhn"))
        headerSource += "compositeFHN(gradient, result, t, tDepth);\n";

    // configure compositing mode for port 3
    headerSource += "#define RC_APPLY_COMPOSITING_2(result, color, samplePos, gradient, t, samplingStepSize, tDepth) ";
    if (compositingMode2_.isSelected("dvr"))
        headerSource += "compositeDVR(result, color, t, samplingStepSize, tDepth);\n";
    else if (compositingMode2_.isSelected("mip"))
        headerSource += "compositeMIP(result, color, t, tDepth);\n";
    else if (compositingMode2_.isSelected("mida"))
        headerSource += "compositeMIDA(result, voxel, color, f_max_i2, t, tDepth, gammaValue2_);\n";
    else if (compositingMode2_.isSelected("iso"))
        headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
    else if (compositingMode2_.isSelected("fhp"))
        headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
    else if (compositingMode2_.isSelected("fhn"))
        headerSource += "compositeFHN(gradient, result, t, tDepth);\n";

    internalPortGroup_.reattachTargets();
    headerSource += internalPortGroup_.generateHeader(shaderProp_.getShader());
    return headerSource;
}

void SingleVolumeRaycaster::adjustPropertyVisibilities() {
    bool useLighting = !shadeMode_.isSelected("none");
    setPropertyGroupVisible("lighting", useLighting);

    bool useIsovalue = (compositingMode_.isSelected("iso")  ||
                        compositingMode1_.isSelected("iso") ||
                        compositingMode2_.isSelected("iso")   );
    isoValue_.setVisible(useIsovalue);

    lightAttenuation_.setVisible(applyLightAttenuation_.get());

    gammaValue_.setVisible(compositingMode_.isSelected("mida"));
    gammaValue1_.setVisible(compositingMode1_.isSelected("mida"));
    gammaValue2_.setVisible(compositingMode2_.isSelected("mida"));
}

} // namespace
