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

#include "voreen/core/processors/volumeraycaster.h"
#include "voreen/core/utils/voreenpainter.h"

#include "voreen/core/datastructures/volume/bricking/brickedvolumegl.h"

#include "tgt/vector.h"

using tgt::mat4;

namespace voreen {

const std::string VolumeRaycaster::loggerCat_("voreen.VolumeRaycaster");

/*
    constructor and destructor
*/

VolumeRaycaster::VolumeRaycaster()
    : VolumeRenderer()
    //, raycastPrg_(0)
    , samplingRate_("samplingRate", "Sampling Rate", 4.f, 0.01f, 20.f)
    , isoValue_("isoValue", "Iso Value", 0.5f, 0.0f, 1.0f)
    , maskingMode_("masking", "Masking", Processor::INVALID_PROGRAM)
    , gradientMode_("gradient", "Gradient Calculation", Processor::INVALID_PROGRAM)
    , classificationMode_("classification", "Classification", Processor::INVALID_PROGRAM)
    , shadeMode_("shading", "Shading", Processor::INVALID_PROGRAM)
    , compositingMode_("compositing", "Compositing", Processor::INVALID_PROGRAM)
    , interactionCoarseness_("interactionCoarseness","Interaction Coarseness", 4, 1, 16, Processor::VALID)
    , interactionQuality_("interactionQuality","Interaction Quality", 1.0f, 0.01f, 1.0f, Processor::VALID)
    , useInterpolationCoarseness_("interpolation.coarseness","Use Interpolation Coarseness", false, Processor::INVALID_PROGRAM)
    , brickingInterpolationMode_("interpolationMode","Interpolation", Processor::INVALID_PROGRAM)
    , brickingStrategyMode_("bricking.strategy.mode", "Bricking Strategy")
    , brickingUpdateStrategy_("bricking.update.strategy", "Update Bricks")
    , brickLodSelector_("brickLodSelector", "Brick LOD Selection")
    , useAdaptiveSampling_("adaptive.sampling", "Use Adaptive Sampling", false, Processor::INVALID_PROGRAM)
    , size_(128, 128)
    , switchToInteractionMode_(false)
    , brickingParametersChanged_(false)
{
    initProperties();
}

/*
    further methods
*/
std::string VolumeRaycaster::generateHeader(VolumeHandle* volumeHandle) {
    std::string headerSource = VolumeRenderer::generateHeader();

    // configure gradient calculation
    headerSource += "#define RC_CALC_GRADIENTS(voxel, samplePos, volume, volumeParameters, t, rayDirection, entryPoints, entryParameters) ";
    if (gradientMode_.isSelected("none"))
        headerSource += "(voxel.xyz-vec3(0.5))*2.0;\n";
    else if (gradientMode_.isSelected("forward-differences"))
        headerSource += "calcGradientAFD(volume, volumeParameters, samplePos, t, rayDirection, entryPoints, entryParameters);\n";
    else if (gradientMode_.isSelected("central-differences"))
        headerSource += "calcGradientA(volume, volumeParameters, samplePos, t, rayDirection, entryPoints, entryParameters);\n";
    else if (gradientMode_.isSelected("filtered"))
        headerSource += "calcGradientFiltered(volume, volumeParameters, samplePos, entryPoints, entryParameters);\n";

    // configure classification
    headerSource += "#define RC_APPLY_CLASSIFICATION(transferFunc, voxel) ";
    if (classificationMode_.isSelected("none"))
        headerSource += "vec4(voxel.a);\n";
    else if (classificationMode_.isSelected("transfer-function"))
        headerSource += "applyTF(transferFunc, voxel);\n";

    // configure shading mode
    headerSource += "#define RC_APPLY_SHADING(gradient, samplePos, volumeParameters, ka, kd, ks) ";
    if (shadeMode_.isSelected("none"))
        headerSource += "ka;\n";
    else if (shadeMode_.isSelected("phong-diffuse"))
        headerSource += "phongShadingD(gradient, samplePos, volumeParameters, kd);\n";
    else if (shadeMode_.isSelected("phong-specular"))
        headerSource += "phongShadingS(gradient, samplePos, volumeParameters, ks);\n";
    else if (shadeMode_.isSelected("phong-diffuse-ambient"))
        headerSource += "phongShadingDA(gradient, samplePos, volumeParameters, kd, ka);\n";
    else if (shadeMode_.isSelected("phong-diffuse-specular"))
        headerSource += "phongShadingDS(gradient, samplePos, volumeParameters, kd, ks);\n";
    else if (shadeMode_.isSelected("phong"))
        headerSource += "phongShading(gradient, samplePos, volumeParameters, ka, kd, ks);\n";
    else if (shadeMode_.isSelected("toon"))
        headerSource += "toonShading(gradient, samplePos, volumeParameters, kd, 3);\n";

    // configure compositing mode
    headerSource += "#define RC_APPLY_COMPOSITING(result, color, samplePos, gradient, t, tDepth) ";
    if (compositingMode_.isSelected("dvr"))
        headerSource += "compositeDVR(result, color, t, tDepth);\n";
    else if (compositingMode_.isSelected("mip"))
        headerSource += "compositeMIP(result, color, t, tDepth);\n";
    else if (compositingMode_.isSelected("iso"))
        headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
    else if (compositingMode_.isSelected("fhp"))
        headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
    else if (compositingMode_.isSelected("fhn"))
        headerSource += "compositeFHN(gradient, result, t, tDepth);\n";

    // configure bricking
    if (volumeHandle) {
        if (volumeHandle->getModality() == Modality::MODALITY_BRICKED_VOLUME) {
            headerSource+= "#define BRICKED_VOLUME\n";
            headerSource+= "#define LOOKUP_VOXEL(sample,brickStartPos,indexVolumeSample) ";
            if (brickingInterpolationMode_.isSelected("intrablock"))
                headerSource += "clampedPackedVolumeLookup(sample,brickStartPos,indexVolumeSample);\n";
            else if (brickingInterpolationMode_.isSelected("interblock"))
                headerSource += "interBlockInterpolationLookup(brickStartPos,sample, indexVolumeSample);\n";

            if (useAdaptiveSampling_.get() ) {
                headerSource += "#define ADAPTIVE_SAMPLING\n";
            }
            if (useInterpolationCoarseness_.get() ) {
                headerSource += "#define INTERPOLATION_COARSENESS\n";
            }
        }
    }

    if (applyLightAttenuation_.get())
        headerSource += "#define PHONG_APPLY_ATTENUATION\n";

    return headerSource;
}

void VolumeRaycaster::initProperties() {
    addProperty(samplingRate_);

    // initialization of the rendering properties
    // the properties are added in the respective subclasses
    maskingMode_.addOption("none", "none");
    maskingMode_.addOption("Segmentation", "Segmentation");

    gradientMode_.addOption("none",                 "none"                  );
    gradientMode_.addOption("forward-differences",  "Forward Differences"   );
    gradientMode_.addOption("central-differences",  "Central Differences"   );
    gradientMode_.addOption("filtered",             "Filtered"              );
    gradientMode_.select("central-differences");

    classificationMode_.addOption("none", "none");
    classificationMode_.addOption("transfer-function", "Transfer Function");
    classificationMode_.select("transfer-function");

    shadeMode_.addOption("none",                   "none"                   );
    shadeMode_.addOption("phong-diffuse",          "Phong (Diffuse)"        );
    shadeMode_.addOption("phong-specular",         "Phong (Specular)"       );
    shadeMode_.addOption("phong-diffuse-ambient",  "Phong (Diffuse+Amb.)"   );
    shadeMode_.addOption("phong-diffuse-specular", "Phong (Diffuse+Spec.)"  );
    shadeMode_.addOption("phong",                  "Phong (Full)"           );
    shadeMode_.addOption("toon",                   "Toon"                   );
    shadeMode_.select("phong");

    compositingMode_.addOption("dvr", "DVR");
    compositingMode_.addOption("mip", "MIP");
    compositingMode_.addOption("iso", "ISO");
    compositingMode_.addOption("fhp", "FHP");
    compositingMode_.addOption("fhn", "FHN");

    brickingInterpolationMode_.addOption("intrablock", "Intrablock Interpolation");
    brickingInterpolationMode_.addOption("interblock", "Interblock Interpolation (slow)");
    brickingInterpolationMode_.setVisible(false);
    addProperty(brickingInterpolationMode_);

    brickLodSelector_.addOption("error-based", "Error-based");
    brickLodSelector_.addOption("camera-based", "Camera-based");
    brickLodSelector_.setVisible(false);
    Call1ParMemberAction<VolumeRaycaster,std::string> errorBasedLodSelector(this,
        &VolumeRaycaster::changeBrickLodSelector, "error-based");

    Call1ParMemberAction<VolumeRaycaster,std::string> cameraBasedLodSelector(this,
        &VolumeRaycaster::changeBrickLodSelector,"camera-based");

    brickLodSelector_.onValueEqual("error-based", errorBasedLodSelector);
    brickLodSelector_.onValueEqual("camera-based", cameraBasedLodSelector);
    addProperty(brickLodSelector_);

    brickingStrategyMode_.addOption("balanced", "Balanced");
    brickingStrategyMode_.addOption("max-only", "Only Max Bricks");
    brickingStrategyMode_.setVisible(false);
    addProperty(brickingStrategyMode_);

    Call1ParMemberAction<VolumeRaycaster,std::string> useBalancedBricks(this,
        &VolumeRaycaster::changeBrickResolutionCalculator,"balanced");

    Call1ParMemberAction<VolumeRaycaster,std::string> useMaximumBricks(this,
        &VolumeRaycaster::changeBrickResolutionCalculator,"maximum");

    brickingStrategyMode_.onValueEqual("balanced", useBalancedBricks);
    brickingStrategyMode_.onValueEqual("max-only", useMaximumBricks);

    brickingUpdateStrategy_.addOption("never", "Never");
    brickingUpdateStrategy_.addOption("on-mouse-release", "On mouse release");
    brickingUpdateStrategy_.setVisible(false);

    Call1ParMemberAction<VolumeRaycaster,std::string> updateBricks(this,
        &VolumeRaycaster::changeBrickingUpdateStrategy, "on-mouse-release");

    Call1ParMemberAction<VolumeRaycaster,std::string> dontUpdateBricks(this,
        &VolumeRaycaster::changeBrickingUpdateStrategy, "never");

    brickingUpdateStrategy_.onValueEqual("on-mouse-release", updateBricks);
    brickingUpdateStrategy_.onValueEqual("never", dontUpdateBricks);

    addProperty(brickingUpdateStrategy_);

    useAdaptiveSampling_.setVisible(false);
    useInterpolationCoarseness_.setVisible(false);
    addProperty(useAdaptiveSampling_);
    addProperty(useInterpolationCoarseness_);
    addProperty(interactionCoarseness_);
    addProperty(interactionQuality_);
}

void VolumeRaycaster::updateBrickingParameters(VolumeHandle* volumeHandle) {

    if (!volumeHandle || !brickingParametersChanged_)
        return;

    LargeVolumeManager* lvm = volumeHandle->getLargeVolumeManager();
    if (lvm) {

        // resolution
        lvm->changeBrickResolutionCalculator(brickResoluationModeStr_);

        // bricking update
        if (brickUpdateStrategyStr_ == "on-mouse-release") {
            lvm->setUpdateBricks(true);
        }
        else {
            lvm->setUpdateBricks(false);
        }

        // lod selection
        if (brickLodSelectorStr_ == "error-based") {
            brickingUpdateStrategy_.setVisible(false);
            brickingStrategyMode_.setVisible(false);
            lvm->changeBrickLodSelector("error-based");
        }
        else if (brickLodSelectorStr_ == "camera-based") {
            brickingUpdateStrategy_.setVisible(true);
            brickingStrategyMode_.setVisible(true);
            lvm->changeBrickLodSelector("camera-based");
        }
    }

    brickingParametersChanged_ = false;

}

void VolumeRaycaster::changeBrickResolutionCalculator(std::string mode) {
    brickResoluationModeStr_ = mode;
    brickingParametersChanged_ = true;
}

void VolumeRaycaster::changeBrickingUpdateStrategy(std::string mode) {
    brickUpdateStrategyStr_ = mode;
    brickingParametersChanged_ = true;
}

void VolumeRaycaster::changeBrickLodSelector(std::string selector) {
    brickLodSelectorStr_ = selector;
    brickingParametersChanged_ = true;
}

void VolumeRaycaster::showBrickingProperties(bool b) {
    if (b) {
        brickingInterpolationMode_.setVisible(true);
        if (brickLodSelector_.get() != "error-based") {
            brickingUpdateStrategy_.setVisible(true);
            brickingStrategyMode_.setVisible(true);
        }
        useAdaptiveSampling_.setVisible(true);
        useInterpolationCoarseness_.setVisible(true);
        brickLodSelector_.setVisible(true);
    }
    else {
        brickingInterpolationMode_.setVisible(false);
        brickingUpdateStrategy_.setVisible(false);
        brickingStrategyMode_.setVisible(false);
        useAdaptiveSampling_.setVisible(false);
        useInterpolationCoarseness_.setVisible(false);
        brickLodSelector_.setVisible(false);
    }
}

void VolumeRaycaster::setBrickedVolumeUniforms(tgt::Shader* shader, VolumeHandle* volumeHandle) {

    tgtAssert(shader, "no shader passed");

    Volume* eepVolume;
    Volume* packedVolume;

    if (!volumeHandle ||
        volumeHandle->getModality() != Modality::MODALITY_BRICKED_VOLUME)
    {
        showBrickingProperties(false);
        return;
    }

   showBrickingProperties(true);

    BrickedVolume* brickedVolume = dynamic_cast<BrickedVolume*>(volumeHandle->getVolume() );
    if (!brickedVolume)
        return;

    eepVolume = brickedVolume->getEepVolume();
    packedVolume = brickedVolume->getPackedVolume();

    LGL_ERROR;

    size_t bricksize = eepVolume->meta().getBrickSize();
    tgt::ivec3 eepDimensions = eepVolume->getDimensions();
    tgt::ivec3 brickedDimensions = packedVolume->getDimensions();

    float numbricksX = (float)ceil((float)eepDimensions.x/(float)bricksize);
    float numbricksY = (float)ceil((float)eepDimensions.y/(float)bricksize);
    float numbricksZ = (float)ceil((float)eepDimensions.z/(float)bricksize);

    float brickSizeX = 1.0f / numbricksX;
    float brickSizeY = 1.0f / numbricksY;
    float brickSizeZ = 1.0f / numbricksZ;

    LGL_ERROR;
    if (brickingInterpolationMode_.get() == "interblock") {
        shader->setUniform("brickSizeX_",brickSizeX);
        shader->setUniform("brickSizeY_",brickSizeY);
        shader->setUniform("brickSizeZ_",brickSizeZ);
    }

    shader->setUniform("numbricksX_",numbricksX);
    shader->setUniform("numbricksY_",numbricksY);
    shader->setUniform("numbricksZ_",numbricksZ);
    float temp1 = 1.0f / (2.0f * numbricksX);
    float temp2 = 1.0f / (2.0f * numbricksY);
    float temp3 = 1.0f / (2.0f * numbricksZ);
    shader->setUniform("temp1",temp1);
    shader->setUniform("temp2",temp2);
    shader->setUniform("temp3",temp3);

    float temp4 = 1.0f / (2.0f * bricksize);
    shader->setUniform("temp4",temp4);

    float temp5 = 1.0f / numbricksX;
    float temp6 = 1.0f / numbricksY;
    float temp7 = 1.0f / numbricksZ;
    shader->setUniform("temp5",temp5);
    shader->setUniform("temp6",temp6);
    shader->setUniform("temp7",temp7);

    float boundaryX = 1.0f - (1.0f / numbricksX);
    float boundaryY = 1.0f - (1.0f / numbricksY);
    float boundaryZ = 1.0f - (1.0f / numbricksZ);

    shader->setUniform("boundaryX_",boundaryX);
    shader->setUniform("boundaryY_",boundaryY);
    shader->setUniform("boundaryZ_",boundaryZ);

    LGL_ERROR;

    //raycastPrg_->setUniform("maxbricksize_",(float)bricksize);
    LGL_ERROR;
    float offsetFactorX = (float)eepDimensions.x / (float)brickedDimensions.x;
    float offsetFactorY = (float)eepDimensions.y / (float)brickedDimensions.y;
    float offsetFactorZ = (float)eepDimensions.z / (float)brickedDimensions.z;

    shader->setUniform("offsetFactorX_",offsetFactorX);
    shader->setUniform("offsetFactorY_",offsetFactorY);
    shader->setUniform("offsetFactorZ_",offsetFactorZ);
    LGL_ERROR;
    float indexVolumeFactorX = 65535.0f / brickedDimensions.x;
    float indexVolumeFactorY = 65535.0f / brickedDimensions.y;
    float indexVolumeFactorZ = 65535.0f / brickedDimensions.z;

    shader->setUniform("indexVolumeFactorX_",indexVolumeFactorX);
    shader->setUniform("indexVolumeFactorY_",indexVolumeFactorY);
    shader->setUniform("indexVolumeFactorZ_",indexVolumeFactorZ);
    LGL_ERROR;

}

void VolumeRaycaster::addBrickedVolumeModalities(VolumeHandle* volumeHandle, std::vector<VolumeStruct>& volumeTextures,
                                                 tgt::TextureUnit* unit1, tgt::TextureUnit* unit2)
{
    if (volumeHandle && volumeHandle->getModality() == Modality::MODALITY_BRICKED_VOLUME) {
        VolumeGL* vgl = volumeHandle->getVolumeGL();
        BrickedVolumeGL* brickedVolumeGL = dynamic_cast<BrickedVolumeGL*>(vgl);

        if (!brickedVolumeGL)
            return;

        VolumeGL* packedVolumeGL = brickedVolumeGL->getPackedVolumeGL();
        VolumeGL* indexVolumeGL = brickedVolumeGL->getIndexVolumeGL();

        if (!packedVolumeGL || !indexVolumeGL)
            return;

        volumeTextures.push_back(VolumeStruct(
            packedVolumeGL,
            unit1,
            "packedVolume_",
            "packedVolumeParameters_",
            true)
        );

        volumeTextures.push_back(VolumeStruct(
            indexVolumeGL,
            unit2,
            "indexVolume_",
            "indexVolumeParameters_",
            true)
        );

    }
}

void VolumeRaycaster::setGlobalShaderParameters(tgt::Shader* shader, tgt::Camera* camera) {
    VolumeRenderer::setGlobalShaderParameters(shader, camera);

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

void VolumeRaycaster::portResized(RenderPort* p, tgt::ivec2 newsize) {
    size_ = newsize;

    int scale = 1;
    if (interactionMode())
        scale = interactionCoarseness_.get();

    VolumeRenderer::portResized(p, scale*newsize);
}

void VolumeRaycaster::interactionModeToggled() {
    if (interactionMode()) {
        switchToInteractionMode_ = true;
    }
    else {
        switchToInteractionMode_ = false;

        if (interactionCoarseness_.get() != 1) {
            // propagate to predecessing RenderProcessors
            const std::vector<Port*> inports = getInports();
            for(size_t i=0; i<inports.size(); ++i) {
                RenderPort* rp = dynamic_cast<RenderPort*>(inports[i]);
                if (rp)
                    rp->resize(size_);
            }

            //distribute to outports:
            const std::vector<Port*> outports = getOutports();
            for(size_t i=0; i<outports.size(); ++i) {
                RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
                if (rp)
                    rp->resize(size_);
            }

            //distribute to private ports:
            const std::vector<RenderPort*> pports = getPrivateRenderPorts();
            for (size_t i=0; i<pports.size(); ++i) {
                RenderPort* rp = pports[i];
                rp->resize(size_);
            }
        }
        if(interactionQuality_.get() != 1.0f)
            invalidate();
    }
    VolumeRenderer::interactionModeToggled();
}

void VolumeRaycaster::invalidate(int inv) {
    if (switchToInteractionMode_) {
        switchToInteractionMode_ = false;

        if (interactionCoarseness_.get() != 1) {
            // propagate to predecessing RenderProcessors
            const std::vector<Port*> inports = getInports();
            for(size_t i=0; i<inports.size(); ++i) {
                RenderPort* rp = dynamic_cast<RenderPort*>(inports[i]);
                if (rp)
                    rp->resize(size_ / interactionCoarseness_.get());
            }

            //distribute to outports:
            const std::vector<Port*> outports = getOutports();
            for(size_t i=0; i<outports.size(); ++i) {
                RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
                if (rp)
                    rp->resize(size_ / interactionCoarseness_.get());
            }

            //distribute to private ports:
            const std::vector<RenderPort*> pports = getPrivateRenderPorts();
            for (size_t i=0; i<pports.size(); ++i) {
                RenderPort* rp = pports[i];
                rp->resize(size_ / interactionCoarseness_.get());
            }
        }
    }

    VolumeRenderer::invalidate(inv);
}

void VolumeRaycaster::bindVolumes(tgt::Shader* shader, const std::vector<VolumeStruct> &volumes,
        const tgt::Camera* camera, const tgt::vec4& lightPosition) {

    VolumeRenderer::bindVolumes(shader, volumes, camera, lightPosition);

    shader->setIgnoreUniformLocationError(true);

    //TODO: This uses the first volume to set the step size. Could be changed so that step
    // size is calculated for each volume, but shaders need to be adapted as well to have volume
    // parameters available in ray setup and compositing. joerg
    if (volumes.size() > 0) {
        if (!volumes[0].volume_ || !volumes[0].volume_->getTexture()) {
            LWARNING("No volume texture");
        }
        else {
            tgt::ivec3 dim = volumes[0].volume_->getTexture()->getDimensions();

            // use dimension with the highest resolution for calculating the sampling step size
            float samplingStepSize = 1.f / (tgt::max(dim) * samplingRate_.get());

            if (interactionMode())
                samplingStepSize /= interactionQuality_.get();

            shader->setUniform("samplingStepSize_", samplingStepSize);
            LGL_ERROR;
        }
    }
    shader->setIgnoreUniformLocationError(false);
}

} // namespace voreen
