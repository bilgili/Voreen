/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/modules/base/processors/render/multivolumeraycaster.h"

#include "tgt/textureunit.h"

#include <sstream>

using tgt::vec3;
using tgt::TextureUnit;

namespace voreen {

MultiVolumeRaycaster::MultiVolumeRaycaster()
    : VolumeRaycaster()
    , volumeInport1_(Port::INPORT, "volume1", false, Processor::INVALID_PROGRAM)
    , volumeInport2_(Port::INPORT, "volume2", false, Processor::INVALID_PROGRAM)
    , volumeInport3_(Port::INPORT, "volume3", false, Processor::INVALID_PROGRAM)
    , volumeInport4_(Port::INPORT, "volume4", false, Processor::INVALID_PROGRAM)
    , entryPort_(Port::INPORT, "image.entrypoints")
    , exitPort_(Port::INPORT, "image.exitpoints")
    , outport_(Port::OUTPORT, "image.output", true, Processor::INVALID_PROGRAM, GL_RGBA16F_ARB)
    , outport1_(Port::OUTPORT, "image.output1", true, Processor::INVALID_PROGRAM, GL_RGBA16F_ARB)
    , outport2_(Port::OUTPORT, "image.output2", true, Processor::INVALID_PROGRAM, GL_RGBA16F_ARB)
    , raycastPrg_(0)
    , shadeMode1_("shading1", "Shading 1", Processor::INVALID_PROGRAM)
    , shadeMode2_("shading2", "Shading 2", Processor::INVALID_PROGRAM)
    , shadeMode3_("shading3", "Shading 3", Processor::INVALID_PROGRAM)
    , shadeMode4_("shading4", "Shading 4", Processor::INVALID_PROGRAM)
    , transferFunc1_("transferFunction1", "Transfer Function 1")
    , transferFunc2_("transferFunction2", "Transfer Function 2")
    , transferFunc3_("transferFunction3", "Transfer Function 3")
    , transferFunc4_("transferFunction4", "Transfer Function 4")
    , texClampMode1_("textureClampMode1_", "Texture Clamp 1")
    , texClampMode2_("textureClampMode2_", "Texture Clamp 2")
    , texClampMode3_("textureClampMode3_", "Texture Clamp 3")
    , texClampMode4_("textureClampMode4_", "Texture Clamp 4")
    , texBorderIntensity_("textureBorderIntensity", "Texture Border Intensity", 0.f)
    , camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , compositingMode1_("compositing1", "Compositing (OP2)", Processor::INVALID_PROGRAM)
    , compositingMode2_("compositing2", "Compositing (OP3)", Processor::INVALID_PROGRAM)
{

    addPort(volumeInport1_);
    addPort(volumeInport2_);
    addPort(volumeInport3_);
    addPort(volumeInport4_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);
    addPort(outport1_);
    addPort(outport2_);

    addProperty(transferFunc1_);
    addProperty(transferFunc2_);
    addProperty(transferFunc3_);
    addProperty(transferFunc4_);
    addProperty(camera_);

//    addProperty(maskingMode_);
    addProperty(gradientMode_);
    shadeMode1_.addOption("none", "none");
    shadeMode1_.addOption("phong-diffuse", "Phong (Diffuse)");
    shadeMode1_.addOption("phong-specular", "Phong (Specular)");
    shadeMode1_.addOption("phong-diffuse-ambient", "Phong (Diffuse+Amb.)");
    shadeMode1_.addOption("phong-diffuse-specular", "Phong (Diffuse+Spec.)");
    shadeMode1_.addOption("phong", "Phong (Full)");
    shadeMode1_.addOption("toon", "Toon");
    shadeMode1_.select("phong");
    addProperty(shadeMode1_);
    shadeMode2_.addOption("none", "none");
    shadeMode2_.addOption("phong-diffuse", "Phong (Diffuse)");
    shadeMode2_.addOption("phong-specular", "Phong (Specular)");
    shadeMode2_.addOption("phong-diffuse-ambient", "Phong (Diffuse+Amb.)");
    shadeMode2_.addOption("phong-diffuse-specular", "Phong (Diffuse+Spec.)");
    shadeMode2_.addOption("phong", "Phong (Full)");
    shadeMode2_.addOption("toon", "Toon");
    shadeMode2_.select("phong");
    addProperty(shadeMode2_);
    shadeMode3_.addOption("none", "none");
    shadeMode3_.addOption("phong-diffuse", "Phong (Diffuse)");
    shadeMode3_.addOption("phong-specular", "Phong (Specular)");
    shadeMode3_.addOption("phong-diffuse-ambient", "Phong (Diffuse+Amb.)");
    shadeMode3_.addOption("phong-diffuse-specular", "Phong (Diffuse+Spec.)");
    shadeMode3_.addOption("phong", "Phong (Full)");
    shadeMode3_.addOption("toon", "Toon");
    shadeMode3_.select("phong");
    addProperty(shadeMode3_);
    shadeMode4_.addOption("none", "none");
    shadeMode4_.addOption("phong-diffuse", "Phong (Diffuse)");
    shadeMode4_.addOption("phong-specular", "Phong (Specular)");
    shadeMode4_.addOption("phong-diffuse-ambient", "Phong (Diffuse+Amb.)");
    shadeMode4_.addOption("phong-diffuse-specular", "Phong (Diffuse+Spec.)");
    shadeMode4_.addOption("phong", "Phong (Full)");
    shadeMode4_.addOption("toon", "Toon");
    shadeMode4_.select("phong");
    addProperty(shadeMode4_);

    // volume texture clamping
    texClampMode1_.addOption("clamp",           "Clamp",             GL_CLAMP);
    texClampMode1_.addOption("clamp-to-edge",   "Clamp to Edge",     GL_CLAMP_TO_EDGE);
    texClampMode1_.addOption("clamp-to-border", "Clamp to Border",   GL_CLAMP_TO_BORDER);
    texClampMode1_.selectByKey("clamp-to-edge");
    addProperty(texClampMode1_);
    texClampMode2_.addOption("clamp",           "Clamp",             GL_CLAMP);
    texClampMode2_.addOption("clamp-to-edge",   "Clamp to Edge",     GL_CLAMP_TO_EDGE);
    texClampMode2_.addOption("clamp-to-border", "Clamp to Border",   GL_CLAMP_TO_BORDER);
    texClampMode2_.selectByKey("clamp-to-edge");
    addProperty(texClampMode2_);
    texClampMode3_.addOption("clamp",           "Clamp",             GL_CLAMP);
    texClampMode3_.addOption("clamp-to-edge",   "Clamp to Edge",     GL_CLAMP_TO_EDGE);
    texClampMode3_.addOption("clamp-to-border", "Clamp to Border",   GL_CLAMP_TO_BORDER);
    texClampMode3_.selectByKey("clamp-to-edge");
    addProperty(texClampMode3_);
    texClampMode4_.addOption("clamp",           "Clamp",             GL_CLAMP);
    texClampMode4_.addOption("clamp-to-edge",   "Clamp to Edge",     GL_CLAMP_TO_EDGE);
    texClampMode4_.addOption("clamp-to-border", "Clamp to Border",   GL_CLAMP_TO_BORDER);
    texClampMode4_.selectByKey("clamp-to-edge");
    addProperty(texClampMode4_);
    addProperty(texBorderIntensity_);

    // compositing modes
    addProperty(compositingMode_);
    compositingMode1_.addOption("dvr", "DVR");
    compositingMode1_.addOption("mip", "MIP");
    compositingMode1_.addOption("iso", "ISO");
    compositingMode1_.addOption("fhp", "W-FHP");
    //compositingMode1_.addOption("fhn", "FHN");
    addProperty(compositingMode1_);
    compositingMode2_.addOption("dvr", "DVR");
    compositingMode2_.addOption("mip", "MIP");
    compositingMode2_.addOption("iso", "ISO");
    compositingMode2_.addOption("fhp", "W-FHP");
    //compositingMode2_.addOption("fhn", "FHN");
    addProperty(compositingMode2_);

    addProperty(isoValue_);

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
}

MultiVolumeRaycaster::~MultiVolumeRaycaster() {
}

Processor* MultiVolumeRaycaster::create() const {
    return new MultiVolumeRaycaster();
}

std::string MultiVolumeRaycaster::getProcessorInfo() const {
    return "Performs a single-pass multi-volume raycasting of up to four volumes, providing several shading and compositing modes. "
           "It allows to generate three output renderings, whereas only the first one provides depth values.<br/>"
           "See MultiVolumeProxyGeometry, MeshEntryExitPoints.";
}

void MultiVolumeRaycaster::initialize() throw (VoreenException) {
    VolumeRaycaster::initialize();

    loadShader();

    if (!raycastPrg_)
        throw VoreenException(getClassName() + ": Failed to load shaders!");

    portGroup_.initialize();
    portGroup_.addPort(outport_);
    portGroup_.addPort(outport1_);
    portGroup_.addPort(outport2_);
}

void MultiVolumeRaycaster::deinitialize() throw (VoreenException) {
    portGroup_.deinitialize();

    ShdrMgr.dispose(raycastPrg_);
    raycastPrg_ = 0;
    LGL_ERROR;

    VolumeRaycaster::deinitialize();
}

void MultiVolumeRaycaster::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("passthrough.vert", "rc_multivolume.frag",
                                       generateHeader(), false);
}

void MultiVolumeRaycaster::compile(VolumeHandle* volumeHandle) {
    raycastPrg_->setHeaders(generateHeader(volumeHandle));
    raycastPrg_->rebuild();
}

bool MultiVolumeRaycaster::isReady() const {
    //check if all inports are connected:
    if(!entryPort_.isReady() || !exitPort_.isReady() || !volumeInport1_.isReady())
        return false;

    //check if at least one outport is connected:
    if (!outport_.isReady() && !outport1_.isReady() && !outport2_.isReady())
        return false;

    return true;
}

void MultiVolumeRaycaster::process() {
    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile(volumeInport1_.getData());
    LGL_ERROR;

    // bind transfer function
    TextureUnit transferUnit1, transferUnit2, transferUnit3, transferUnit4;
    transferUnit1.activate();
    if (transferFunc1_.get())
        transferFunc1_.get()->bind();

    transferUnit2.activate();
    if (transferFunc2_.get())
        transferFunc2_.get()->bind();

    transferUnit3.activate();
    if (transferFunc3_.get())
        transferFunc3_.get()->bind();

    transferUnit4.activate();
    if (transferFunc4_.get())
        transferFunc4_.get()->bind();

    portGroup_.activateTargets();
    portGroup_.clearTargets();
    LGL_ERROR;

    transferFunc1_.setVolumeHandle(volumeInport1_.getData());
    transferFunc2_.setVolumeHandle(volumeInport2_.getData());
    transferFunc3_.setVolumeHandle(volumeInport3_.getData());
    transferFunc4_.setVolumeHandle(volumeInport4_.getData());

    TextureUnit entryUnit, entryDepthUnit, exitUnit, exitDepthUnit;
    // bind entry params
    entryPort_.bindTextures(entryUnit.getEnum(), entryDepthUnit.getEnum());
    LGL_ERROR;

    // bind exit params
    exitPort_.bindTextures(exitUnit.getEnum(), exitDepthUnit.getEnum());
    LGL_ERROR;

    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

    // add volumes
    TextureUnit volUnit1, volUnit2, volUnit3, volUnit4;
    if(volumeInport1_.isReady()) {
        volumeTextures.push_back(VolumeStruct(
                    volumeInport1_.getData()->getVolumeGL(),
                    &volUnit1,
                    "volume1_",
                    "volumeParameters1_",
                    true,
                    texClampMode1_.getValue(),
                    tgt::vec4(texBorderIntensity_.get()))
                );
    }
    if(volumeInport2_.isReady()) {
        volumeTextures.push_back(VolumeStruct(
                    volumeInport2_.getData()->getVolumeGL(),
                    &volUnit2,
                    "volume2_",
                    "volumeParameters2_",
                    true,
                    texClampMode2_.getValue(),
                    tgt::vec4(texBorderIntensity_.get()))
                );
    }
    if(volumeInport3_.isReady()) {
        volumeTextures.push_back(VolumeStruct(
                    volumeInport3_.getData()->getVolumeGL(),
                    &volUnit3,
                    "volume3_",
                    "volumeParameters3_",
                    true,
                    texClampMode3_.getValue(),
                    tgt::vec4(texBorderIntensity_.get()))
                );
    }
    if(volumeInport4_.isReady()) {
        volumeTextures.push_back(VolumeStruct(
                    volumeInport4_.getData()->getVolumeGL(),
                    &volUnit4,
                    "volume4_",
                    "volumeParameters4_",
                    true,
                    texClampMode4_.getValue(),
                    tgt::vec4(texBorderIntensity_.get()))
                );
    }

    // initialize shader
    raycastPrg_->activate();

    // set common uniforms used by all shaders
    tgt::Camera cam = camera_.get();
    setGlobalShaderParameters(raycastPrg_, &cam);
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(raycastPrg_, volumeTextures, &cam, lightPosition_.get());

    // pass the remaining uniforms to the shader
    raycastPrg_->setUniform("entryPoints_", entryUnit.getUnitNumber());
    raycastPrg_->setUniform("entryPointsDepth_", entryDepthUnit.getUnitNumber());
    entryPort_.setTextureParameters(raycastPrg_, "entryParameters_");
    raycastPrg_->setUniform("exitPoints_", exitUnit.getUnitNumber());
    raycastPrg_->setUniform("exitPointsDepth_", exitDepthUnit.getUnitNumber());
    exitPort_.setTextureParameters(raycastPrg_, "exitParameters_");

    if (compositingMode_.get() ==  "iso" ||
        compositingMode1_.get() == "iso" ||
        compositingMode2_.get() == "iso")
        raycastPrg_->setUniform("isoValue_", isoValue_.get());

    if(volumeInport1_.isReady())
        raycastPrg_->setUniform("transferFunc_", transferUnit1.getUnitNumber());
    if(volumeInport2_.isReady())
        raycastPrg_->setUniform("transferFunc2_", transferUnit2.getUnitNumber());
    if(volumeInport3_.isReady())
        raycastPrg_->setUniform("transferFunc3_", transferUnit3.getUnitNumber());
    if(volumeInport4_.isReady())
        raycastPrg_->setUniform("transferFunc4_", transferUnit4.getUnitNumber());

    // determine ray step length in world coords
    if (volumeTextures.size() > 0) {
        float voxelSizeWorld = 999.f;
        float voxelSizeTexture = 999.f;
        for(size_t i=0; i<volumeTextures.size(); ++i) {
            const Volume* volume = volumeTextures[i].volume_->getVolume();
            tgtAssert(volume, "No volume");
            tgt::ivec3 volDim = volume->getDimensions();
            tgt::vec3 cubeSizeWorld = volume->getCubeSize() * volume->getTransformation().getScalingPart();

            float tVoxelSizeWorld = tgt::max(cubeSizeWorld / tgt::vec3(volDim));
            if (tVoxelSizeWorld < voxelSizeWorld) {
                voxelSizeWorld = tVoxelSizeWorld;
                voxelSizeTexture = tgt::max(1.f / tgt::vec3(volDim));
            }
        }

        float samplingStepSizeWorld = voxelSizeWorld / samplingRate_.get();
        float samplingStepSizeTexture = voxelSizeTexture / samplingRate_.get();

        if (interactionMode()) {
            samplingStepSizeWorld /= interactionQuality_.get();
            samplingStepSizeTexture /= interactionQuality_.get();
        }

        raycastPrg_->setUniform("samplingStepSize_", samplingStepSizeWorld);
        raycastPrg_->setUniform("samplingStepSizeComposite_", samplingStepSizeTexture * 200.f);
        LGL_ERROR;
    }
    LGL_ERROR;

    renderQuad();

    raycastPrg_->deactivate();
    portGroup_.deactivateTargets();

    glActiveTexture(GL_TEXTURE0);
    LGL_ERROR;
}

std::string MultiVolumeRaycaster::generateHeader(VolumeHandle* volumeHandle) {
    std::string headerSource = VolumeRaycaster::generateHeader(volumeHandle);

    if(volumeInport1_.isReady())
        headerSource += "#define VOLUME_1_ACTIVE\n";
    if(volumeInport2_.isReady())
        headerSource += "#define VOLUME_2_ACTIVE\n";
    if(volumeInport3_.isReady())
        headerSource += "#define VOLUME_3_ACTIVE\n";
    if(volumeInport4_.isReady())
        headerSource += "#define VOLUME_4_ACTIVE\n";

    headerSource += "#define TF_SAMPLER_TYPE_1 " + transferFunc1_.get()->getSamplerType() + "\n";
    headerSource += "#define TF_SAMPLER_TYPE_2 " + transferFunc2_.get()->getSamplerType() + "\n";
    headerSource += "#define TF_SAMPLER_TYPE_3 " + transferFunc3_.get()->getSamplerType() + "\n";
    headerSource += "#define TF_SAMPLER_TYPE_4 " + transferFunc4_.get()->getSamplerType() + "\n";

    // configure shading mode
    headerSource += "#define RC_APPLY_SHADING_1(gradient, samplePos, volumeParameters, ka, kd, ks) ";
    if (shadeMode1_.get() == "none")
        headerSource += "ka;\n";
    else if (shadeMode1_.get() == "phong-diffuse")
        headerSource += "phongShadingD(gradient, samplePos, volumeParameters, kd);\n";
    else if (shadeMode1_.get() == "phong-specular")
        headerSource += "phongShadingS(gradient, samplePos, volumeParameters, ks);\n";
    else if (shadeMode1_.get() == "phong-diffuse-ambient")
        headerSource += "phongShadingDA(gradient, samplePos, volumeParameters, kd, ka);\n";
    else if (shadeMode1_.get() == "phong-diffuse-specular")
        headerSource += "phongShadingDS(gradient, samplePos, volumeParameters, kd, ks);\n";
    else if (shadeMode1_.get() == "phong")
        headerSource += "phongShading(gradient, samplePos, volumeParameters, ka, kd, ks);\n";
    else if (shadeMode1_.get() == "toon")
        headerSource += "toonShading(gradient, samplePos, volumeParameters, kd, 3);\n";

    headerSource += "#define RC_APPLY_SHADING_2(gradient, samplePos, volumeParameters, ka, kd, ks) ";
    if (shadeMode2_.get() == "none")
        headerSource += "ka;\n";
    else if (shadeMode2_.get() == "phong-diffuse")
        headerSource += "phongShadingD(gradient, samplePos, volumeParameters, kd);\n";
    else if (shadeMode2_.get() == "phong-specular")
        headerSource += "phongShadingS(gradient, samplePos, volumeParameters, ks);\n";
    else if (shadeMode2_.get() == "phong-diffuse-ambient")
        headerSource += "phongShadingDA(gradient, samplePos, volumeParameters, kd, ka);\n";
    else if (shadeMode2_.get() == "phong-diffuse-specular")
        headerSource += "phongShadingDS(gradient, samplePos, volumeParameters, kd, ks);\n";
    else if (shadeMode2_.get() == "phong")
        headerSource += "phongShading(gradient, samplePos, volumeParameters, ka, kd, ks);\n";
    else if (shadeMode2_.get() == "toon")
        headerSource += "toonShading(gradient, samplePos, volumeParameters, kd, 3);\n";

    headerSource += "#define RC_APPLY_SHADING_3(gradient, samplePos, volumeParameters, ka, kd, ks) ";
    if (shadeMode3_.get() == "none")
        headerSource += "ka;\n";
    else if (shadeMode3_.get() == "phong-diffuse")
        headerSource += "phongShadingD(gradient, samplePos, volumeParameters, kd);\n";
    else if (shadeMode3_.get() == "phong-specular")
        headerSource += "phongShadingS(gradient, samplePos, volumeParameters, ks);\n";
    else if (shadeMode3_.get() == "phong-diffuse-ambient")
        headerSource += "phongShadingDA(gradient, samplePos, volumeParameters, kd, ka);\n";
    else if (shadeMode3_.get() == "phong-diffuse-specular")
        headerSource += "phongShadingDS(gradient, samplePos, volumeParameters, kd, ks);\n";
    else if (shadeMode3_.get() == "phong")
        headerSource += "phongShading(gradient, samplePos, volumeParameters, ka, kd, ks);\n";
    else if (shadeMode3_.get() == "toon")
        headerSource += "toonShading(gradient, samplePos, volumeParameters, kd, 3);\n";

    headerSource += "#define RC_APPLY_SHADING_4(gradient, samplePos, volumeParameters, ka, kd, ks) ";
    if (shadeMode4_.get() == "none")
        headerSource += "ka;\n";
    else if (shadeMode4_.get() == "phong-diffuse")
        headerSource += "phongShadingD(gradient, samplePos, volumeParameters, kd);\n";
    else if (shadeMode4_.get() == "phong-specular")
        headerSource += "phongShadingS(gradient, samplePos, volumeParameters, ks);\n";
    else if (shadeMode4_.get() == "phong-diffuse-ambient")
        headerSource += "phongShadingDA(gradient, samplePos, volumeParameters, kd, ka);\n";
    else if (shadeMode4_.get() == "phong-diffuse-specular")
        headerSource += "phongShadingDS(gradient, samplePos, volumeParameters, kd, ks);\n";
    else if (shadeMode4_.get() == "phong")
        headerSource += "phongShading(gradient, samplePos, volumeParameters, ka, kd, ks);\n";
    else if (shadeMode4_.get() == "toon")
        headerSource += "toonShading(gradient, samplePos, volumeParameters, kd, 3);\n";

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
    headerSource += portGroup_.generateHeader(raycastPrg_);
    return headerSource;
}

} // namespace
