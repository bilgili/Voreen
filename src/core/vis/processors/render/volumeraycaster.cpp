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

#include "voreen/core/vis/processors/render/volumeraycaster.h"
#include "voreen/core/vis/voreenpainter.h"

#include "voreen/core/volume/bricking/brickedvolumegl.h"
#include "voreen/core/volume/volumehandlevalidator.h"

#include "tgt/vector.h"

using tgt::mat4;

namespace voreen {

/*
    init statics
*/

const std::string VolumeRaycaster::loggerCat_("voreen.VolumeRaycaster");

const Identifier VolumeRaycaster::setSegment_              = "set.Segment";
const Identifier VolumeRaycaster::switchSegmentation_      = "switch.Segmentation";

const Identifier VolumeRaycaster::entryParamsTexUnit_        = "entryParamsTexUnit";
const Identifier VolumeRaycaster::entryParamsDepthTexUnit_   = "entryParamsDepthTexUnit";
const Identifier VolumeRaycaster::exitParamsTexUnit_         = "exitParamsTexUnit";
const Identifier VolumeRaycaster::exitParamsDepthTexUnit_    = "exitParamsDepthTexUnit";
const Identifier VolumeRaycaster::entryParamsTexUnit2_       = "entryParamsTexUnit2";
const Identifier VolumeRaycaster::entryParamsDepthTexUnit2_  = "entryParamsDepthTexUnit2";
const Identifier VolumeRaycaster::exitParamsTexUnit2_        = "exitParamsTexUnit2";
const Identifier VolumeRaycaster::exitParamsDepthTexUnit2_   = "exitParamsDepthTexUnit2";
const Identifier VolumeRaycaster::volTexUnit_                = "volTexUnit";
const Identifier VolumeRaycaster::volTexUnit2_               = "volTexUnit2";
const Identifier VolumeRaycaster::volTexUnit3_               = "volTexUnit3";
const Identifier VolumeRaycaster::transferTexUnit_           = "transferTexUnit";
const Identifier VolumeRaycaster::transferTexUnit2_          = "transferTexUnit2";
const Identifier VolumeRaycaster::transferTexUnit3_          = "transferTexUnit3";
const Identifier VolumeRaycaster::segmentationTexUnit_       = "segmentationTexUnit";
const Identifier VolumeRaycaster::ambTexUnit_                = "ambTexUnit";
const Identifier VolumeRaycaster::ambLookupTexUnit_          = "ambLookupTexUnit";
const Identifier VolumeRaycaster::normalsTexUnit_            = "normalsTexUnit";
const Identifier VolumeRaycaster::gradientsTexUnit_          = "gradientsTexUnit";
const Identifier VolumeRaycaster::gradientMagnitudesTexUnit_ = "gradientMagnitudesTexUnit";

/*
    constructor and destructor
*/

VolumeRaycaster::VolumeRaycaster()
    : VolumeRenderer()
    , raycastPrg_(0)
    , useAdaptiveSampling_("swith.adaptive.sampling", "Use Adaptive Sampling",false,true,true)
    , segment_(setSegment_, "Active segment", 0)
    , useSegmentation_(switchSegmentation_, "Use Segmentation", false, true, true)
    , useInterpolationCoarseness_("switch.interpolation.coarseness","Use Interpolation Coarseness",false,true,true)
    , coarsenessOn_(false)
{
    // set texture unit identifiers and register
    std::vector<Identifier> units;
    units.push_back(entryParamsTexUnit_);
    units.push_back(entryParamsDepthTexUnit_);
    units.push_back(exitParamsTexUnit_);
    units.push_back(exitParamsDepthTexUnit_);
    units.push_back(entryParamsTexUnit2_);
    units.push_back(entryParamsDepthTexUnit2_);
    units.push_back(exitParamsTexUnit2_);
    units.push_back(exitParamsDepthTexUnit2_);
    units.push_back(volTexUnit_);
    units.push_back(volTexUnit2_);
    units.push_back(volTexUnit3_);
    units.push_back(transferTexUnit_);
    units.push_back(transferTexUnit2_);
    units.push_back(segmentationTexUnit_);
    tm_.registerUnits(units);

    initProperties();

}

VolumeRaycaster::~VolumeRaycaster() {
    if (raycastPrg_)
        ShdrMgr.dispose(raycastPrg_);

    delete raycastingQualityFactor_;
    delete maskingMode_;
    delete gradientMode_;
    delete classificationMode_;
    delete shadeMode_;
    delete compositingMode_;

    if (brickingInterpolationMode_)
        delete brickingInterpolationMode_;

    if (brickingStrategyMode_)
        delete brickingStrategyMode_;

    if (brickingUpdateStrategy_)
        delete brickingUpdateStrategy_;

    if (brickLodSelector_)
        delete brickLodSelector_;
}

/*
    further methods
*/
std::string VolumeRaycaster::generateHeader(VolumeHandle* volumeHandle) {
    std::string headerSource = VolumeRenderer::generateHeader();
	
	if (volumeHandle != 0 && volumeHandle->getParentSeries()) {
		if (volumeHandle->getParentSeries()->getModality() == Modality::MODALITY_BRICKED_VOLUME) {
			headerSource+= "#define BRICKED_VOLUME\n";
			headerSource+= "#define LOOKUP_VOXEL(sample,brickStartPos,indexVolumeSample) ";
			switch (brickingInterpolationMode_->get() ) {
				case 0: 
					headerSource += "clampedPackedVolumeLookup(sample,brickStartPos,indexVolumeSample);\n";
					break;
				case 1:
					//headerSource += "interBlockInterpolation(sample);\n";
					headerSource += "interBlockInterpolationLookup(brickStartPos,sample, indexVolumeSample);\n";
					break;
			}
            if (useAdaptiveSampling_.get() ) {
                headerSource += "#define ADAPTIVE_SAMPLING\n";
            } 
            if (useInterpolationCoarseness_.get() ) {
                headerSource += "#define INTERPOLATION_COARSENESS\n";
            }
		}
	}

    if (maskingMode_->get() == 1)
         headerSource += "#define USE_SEGMENTATION\n";

    headerSource += "#define RC_NOT_MASKED(samplePos, intensity) ";
    switch (maskingMode_->get()) {
        case 0:
            headerSource += "true\n";
            break;
        case 1:
            headerSource += "inSegmentation(samplePos)\n";
            break;
    }

    // configure gradient calculation
    headerSource += "#define RC_CALC_GRADIENTS(voxel, samplePos, volume, volumeParameters, t, rayDirection, entryPoints) ";
    switch (gradientMode_->get()) {
        case 0:
            headerSource += "voxel.xyz-vec3(0.5);\n";
            break;
        case 1:
            headerSource += "calcGradientAFD(volume, volumeParameters, samplePos, t, rayDirection, entryPoints);\n";
            break;
        case 2:
            headerSource += "calcGradientA(volume, volumeParameters, samplePos, t, rayDirection, entryPoints);\n";
            break;
        case 3:
            headerSource += "calcGradientFiltered(volume, volumeParameters, samplePos, entryPoints);\n";
            break;
    }

    // configure classififcation
    headerSource += "#define RC_APPLY_CLASSIFICATION(transferFunc, voxel) ";
    switch (classificationMode_->get()) {
        case 0:
            headerSource += "vec4(voxel.a);\n";
            break;
        case 1:
            headerSource += "applyTF(transferFunc, voxel);\n";
            break;
    }

    // configure shading mode
    headerSource += "#define RC_APPLY_SHADING(gradient, samplePos, volumeParameters, ka, kd, ks) ";
    switch (shadeMode_->get()) {
        case 0:
            headerSource += "ka;\n";
            break;
        case 1:
            headerSource += "phongShadingD(gradient, samplePos, volumeParameters, kd);\n";
            break;
        case 2:
            headerSource += "phongShadingS(gradient, samplePos, volumeParameters, ks);\n";
            break;
        case 3:
            headerSource += "phongShadingDA(gradient, samplePos, volumeParameters, kd, ka);\n";
            break;
        case 4:
            headerSource += "phongShadingDS(gradient, samplePos, volumeParameters, kd, ks);\n";
            break;
        case 5:
            headerSource += "phongShading(gradient, samplePos, volumeParameters, ka, kd, ks);\n";
            break;
        case 6:
            headerSource += "toonShading(gradient, samplePos, volumeParameters, kd, 3);\n";
            break;
    }

    // configure compositing mode
    headerSource += "#define RC_APPLY_COMPOSITING(result, color, samplePos, gradient, t, tDepth) ";
    switch (compositingMode_->get()) {
        case 0:
            headerSource += "compositeDVR(result, color, t, tDepth);\n";
            break;
        case 1:
            headerSource += "compositeMIP(result, color, t, tDepth);\n";
            break;
        case 2:
            headerSource += "compositeISO(result, color, t, tDepth, 0.5);\n";
            break;
        case 3:
            headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
            break;
        case 4:
            headerSource += "compositeFHN(gradient, result, t, tDepth);\n";
            break;
    }

    return headerSource;
}

void VolumeRaycaster::initProperties() {
    // Options for raycastingquality
    Option<float> rcq[] = {
        { "lowest",  "lowest",   0.2f},
        { "lower",   "lower",    0.5f},
        { "normal",  "normal",   1.f },
        { "high",    "high",     2.f },
        { "higher",  "higher",   5.f },
        { "highest", "highest", 10.f },
    };
    std::vector<Option<float> > raycastingQualities(rcq, rcq+6);
    raycastingQualityFactor_ = new OptionProperty<float>("raycasting.quality", "Raycasting quality", raycastingQualities, true, true);
    raycastingQualityFactor_->set("normal");
    addProperty(raycastingQualityFactor_);

    // initialization of the rendering properties
    // the properties are added in the respective subclasses
    maskingModes_.push_back("none");
    maskingModes_.push_back("Segmentation");
    maskingMode_ = new EnumProp("set.masking", "Masking", maskingModes_, 0, true, true);

    gradientModes_.push_back("none");
    gradientModes_.push_back("Forward Differences");
    gradientModes_.push_back("Central Differences");
    gradientModes_.push_back("Filtered");
    gradientMode_ = new EnumProp("set.gradient", "Gradient calculation", gradientModes_, 1, true, true);

    classificationModes_.push_back("none");
    classificationModes_.push_back("Transfer Function");
    classificationMode_ = new EnumProp("set.classification", "Classification", classificationModes_, 1, true, true);

    shadeModes_.push_back("none");
    shadeModes_.push_back("Phong (Diffuse)");
    shadeModes_.push_back("Phong (Specular)");
    shadeModes_.push_back("Phong (Diffuse+Ambient)");
    shadeModes_.push_back("Phong (Diffuse+Specular)");
    shadeModes_.push_back("Phong (Full)");
    shadeModes_.push_back("Toon");
    shadeMode_ = new EnumProp("set.shading", "Shading", shadeModes_, 5, true, true);

    compositingModes_.push_back("DVR");
    compositingModes_.push_back("MIP");
    compositingModes_.push_back("ISO");
    compositingModes_.push_back("FHP");
    compositingModes_.push_back("FHN");
    compositingMode_ = new EnumProp("set.compositing", "Compositing", compositingModes_, 0, true, true);

    brickingInterpolationModes_.push_back("Intrablock Interpolation");
    brickingInterpolationModes_.push_back("Interblock Interpolation (slow)");
        brickingInterpolationMode_ = new EnumProp("set.interpolation.mode","Interpolation",
            brickingInterpolationModes_,0,true,true);
    
		brickingInterpolationMode_->setVisible(false);
	addProperty(brickingInterpolationMode_);

    brickLodSelectors_.push_back("Error-based");
    brickLodSelectors_.push_back("Camera-based");
    brickLodSelector_ = new EnumProp("set.brickLodSelector","Brick LOD Selection",
        brickLodSelectors_,0,true,false);
	brickLodSelector_->setVisible(false);
    Call1ParMemberAction<VolumeRaycaster,std::string> errorBasedLodSelector(this,
		&VolumeRaycaster::changeBrickLodSelector,"Error-based");

	Call1ParMemberAction<VolumeRaycaster,std::string> cameraBasedLodSelector(this,
        &VolumeRaycaster::changeBrickLodSelector,"Camera-based");

	brickLodSelector_->onValueEqual(0,errorBasedLodSelector);
	brickLodSelector_->onValueEqual(1,cameraBasedLodSelector);
    addProperty(brickLodSelector_);

    brickingStrategyModes_.push_back("Balanced");
    brickingStrategyModes_.push_back("Only Max Bricks");
        brickingStrategyMode_ = new EnumProp("set.bricking.strategy.mode","Bricking Strategy",
            brickingStrategyModes_,0,true,false);
	
	brickingStrategyMode_->setVisible(false);
    addProperty(brickingStrategyMode_);

    Call1ParMemberAction<VolumeRaycaster,std::string> useBalancedBricks(this,
        &VolumeRaycaster::changeBrickResolutionCalculator,"balanced");

    Call1ParMemberAction<VolumeRaycaster,std::string> useMaximumBricks(this,
        &VolumeRaycaster::changeBrickResolutionCalculator,"maximum");


    brickingStrategyMode_->onValueEqual(0,useBalancedBricks);
    brickingStrategyMode_->onValueEqual(1,useMaximumBricks);

    brickingUpdateStrategies_.push_back("Never");
	brickingUpdateStrategies_.push_back("On mouse release");	
	

	brickingUpdateStrategy_ = new EnumProp("set.bricking.update.strategy","Update Bricks",
        brickingUpdateStrategies_,0,true,false);
	brickingUpdateStrategy_->setVisible(false);

	Call1ParMemberAction<VolumeRaycaster,std::string> updateBricks(this,
		&VolumeRaycaster::changeBrickingUpdateStrategy,"On mouse release");

	Call1ParMemberAction<VolumeRaycaster,std::string> dontUpdateBricks(this,
        &VolumeRaycaster::changeBrickingUpdateStrategy,"Never");

	brickingUpdateStrategy_->onValueEqual(1,updateBricks);
	brickingUpdateStrategy_->onValueEqual(0,dontUpdateBricks);

	addProperty(brickingUpdateStrategy_);
    
    useAdaptiveSampling_.setVisible(false);
    useInterpolationCoarseness_.setVisible(false);
    addProperty(&useAdaptiveSampling_);
    addProperty(&useInterpolationCoarseness_);
}

void VolumeRaycaster::processMessage(Message* msg, const Identifier& dest/*=Message::all_*/) {
   VolumeRenderer::processMessage(msg, dest);

   if (msg->id_ == VoreenPainter::switchCoarseness_) {
       if (coarsenessOn_ == true) {
           coarsenessOn_ = false;
       } 
       else if (coarsenessOn_ == false) {
           coarsenessOn_ = true;
       }
   }
}

void VolumeRaycaster::changeBrickResolutionCalculator(std::string mode) {
    if (currentVolumeHandle_) {
        LargeVolumeManager* lvm = currentVolumeHandle_->getLargeVolumeManager();
        if (lvm) {
            lvm->changeBrickResolutionCalculator(mode);
        }
    }
}

void VolumeRaycaster::changeBrickingUpdateStrategy(std::string mode) {
    if (currentVolumeHandle_) {
        LargeVolumeManager* lvm = currentVolumeHandle_->getLargeVolumeManager();
        if (lvm) {
			if (mode == "On mouse release") {
				lvm->setUpdateBricks(true);
			} else {
				lvm->setUpdateBricks(false);
			}

        }
    }
}

void VolumeRaycaster::changeBrickLodSelector(std::string selector) {
    if (currentVolumeHandle_) {
        LargeVolumeManager* lvm = currentVolumeHandle_->getLargeVolumeManager();
        if (lvm) {
			if (selector == "Error-based") {
                brickingUpdateStrategy_->setVisible(false);
                brickingStrategyMode_->setVisible(false);
				lvm->changeBrickLodSelector("Error-based");
			} else if (selector == "Camera-based") {
                brickingUpdateStrategy_->setVisible(true);
                brickingStrategyMode_->setVisible(true);
				lvm->changeBrickLodSelector("Camera-based");
			}
            //lvm->changeBrickLodSelector(selector);
        }
    }
}

void VolumeRaycaster::showBrickingProperties(bool b) {
	if (b==true) {
		brickingInterpolationMode_->setVisible(true);
        if (brickLodSelector_->get() != 0) {
		    brickingUpdateStrategy_->setVisible(true);
		    brickingStrategyMode_->setVisible(true);
        }
        useAdaptiveSampling_.setVisible(true);
        useInterpolationCoarseness_.setVisible(true);
        brickLodSelector_->setVisible(true);
	} else {
		brickingInterpolationMode_->setVisible(false);
		brickingUpdateStrategy_->setVisible(false);
		brickingStrategyMode_->setVisible(false);
        useAdaptiveSampling_.setVisible(false);
        useInterpolationCoarseness_.setVisible(false);
        brickLodSelector_->setVisible(false);
	}
}

void VolumeRaycaster::setBrickedVolumeUniforms() {

	Volume* eepVolume;
	Volume* packedVolume;

	if (!currentVolumeHandle_->getParentSeries() ||
        currentVolumeHandle_->getParentSeries()->getModality() != Modality::MODALITY_BRICKED_VOLUME)
    {
        showBrickingProperties(false);
		return;
	} 

   showBrickingProperties(true);

	BrickedVolume* brickedVolume = dynamic_cast<BrickedVolume*>(currentVolumeHandle_->getVolume() );
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
	if (brickingInterpolationMode_->get() == 1) {
		raycastPrg_->setUniform("brickSizeX_",brickSizeX);
		raycastPrg_->setUniform("brickSizeY_",brickSizeY);
		raycastPrg_->setUniform("brickSizeZ_",brickSizeZ);
	}

	raycastPrg_->setUniform("numbricksX_",numbricksX);
	raycastPrg_->setUniform("numbricksY_",numbricksY);
	raycastPrg_->setUniform("numbricksZ_",numbricksZ);
    float temp1 = 1.0f / (2.0f * numbricksX);
    float temp2 = 1.0f / (2.0f * numbricksY);
    float temp3 = 1.0f / (2.0f * numbricksZ);
    raycastPrg_->setUniform("temp1",temp1);
	raycastPrg_->setUniform("temp2",temp2);
	raycastPrg_->setUniform("temp3",temp3);

    float temp4 = 1.0f / (2.0f * bricksize);
    raycastPrg_->setUniform("temp4",temp4);

    float temp5 = 1.0f / numbricksX;
    float temp6 = 1.0f / numbricksY;
    float temp7 = 1.0f / numbricksZ;
    raycastPrg_->setUniform("temp5",temp5);
    raycastPrg_->setUniform("temp6",temp6);
    raycastPrg_->setUniform("temp7",temp7);

    float boundaryX = 1.0f - (1.0f / numbricksX);
    float boundaryY = 1.0f - (1.0f / numbricksY);
    float boundaryZ = 1.0f - (1.0f / numbricksZ);

    raycastPrg_->setUniform("boundaryX_",boundaryX);
    raycastPrg_->setUniform("boundaryY_",boundaryY);
    raycastPrg_->setUniform("boundaryZ_",boundaryZ);

	LGL_ERROR;
	
	//raycastPrg_->setUniform("maxbricksize_",(float)bricksize);
	LGL_ERROR;
	float offsetFactorX = (float)eepDimensions.x / (float)brickedDimensions.x;
	float offsetFactorY = (float)eepDimensions.y / (float)brickedDimensions.y;
	float offsetFactorZ = (float)eepDimensions.z / (float)brickedDimensions.z;

	raycastPrg_->setUniform("offsetFactorX_",offsetFactorX);
	raycastPrg_->setUniform("offsetFactorY_",offsetFactorY);
	raycastPrg_->setUniform("offsetFactorZ_",offsetFactorZ);
	LGL_ERROR;
	float indexVolumeFactorX = 65535.0f / brickedDimensions.x;
	float indexVolumeFactorY = 65535.0f / brickedDimensions.y;
	float indexVolumeFactorZ = 65535.0f / brickedDimensions.z;

	raycastPrg_->setUniform("indexVolumeFactorX_",indexVolumeFactorX);
	raycastPrg_->setUniform("indexVolumeFactorY_",indexVolumeFactorY);
	raycastPrg_->setUniform("indexVolumeFactorZ_",indexVolumeFactorZ);
	LGL_ERROR;

    if (useInterpolationCoarseness_.get() )
        raycastPrg_->setUniform("coarsenessOn_",coarsenessOn_);
}

void VolumeRaycaster::addBrickedVolumeModalities(std::vector<VolumeStruct>& volumeTextures) {

	if (currentVolumeHandle_->getParentSeries() &&
        currentVolumeHandle_->getParentSeries()->getModality() == Modality::MODALITY_BRICKED_VOLUME)
    {
		VolumeGL* vgl = currentVolumeHandle_->getVolumeGL();
		BrickedVolumeGL* brickedVolumeGL = dynamic_cast<BrickedVolumeGL*>(vgl);

		if (!brickedVolumeGL) {
			return;
		}

		VolumeGL* packedVolumeGL = brickedVolumeGL->getPackedVolumeGL();
		VolumeGL* indexVolumeGL = brickedVolumeGL->getIndexVolumeGL();

		if (!packedVolumeGL || !indexVolumeGL) {
			return;
		}

		volumeTextures.push_back(VolumeStruct(
			packedVolumeGL,
			volTexUnit2_,
			"packedVolume_",
			"packedVolumeParameters_")
		);

		volumeTextures.push_back(VolumeStruct(
			indexVolumeGL,
			volTexUnit3_,
			"indexVolume_",
			"indexVolumeParameters_")
		);

	}
}

void VolumeRaycaster::setGlobalShaderParameters(tgt::Shader* shader) {
    VolumeRenderer::setGlobalShaderParameters(shader);

    shader->setIgnoreUniformLocationError(true);
    // raycasting quality factor
    shader->setUniform("raycastingQualityFactor_", raycastingQualityFactor_->getValue());
    shader->setUniform("raycastingQualityFactorRCP_", 1.f / raycastingQualityFactor_->getValue());

    // provide values needed for correct depth value calculation
    float n = camera_->getNearDist();
    float f = camera_->getFarDist();
    shader->setUniform("const_to_z_e_1", 0.5f + 0.5f*((f+n)/(f-n)));
    shader->setUniform("const_to_z_e_2", ((f-n)/(f*n)));
    shader->setUniform("const_to_z_w_1", ((f*n)/(f-n)));
    shader->setUniform("const_to_z_w_2", 0.5f*((f+n)/(f-n))+0.5f);

    shader->setIgnoreUniformLocationError(false);
}

bool VolumeRaycaster::checkVolumeHandle(VolumeHandle*& handle, VolumeHandle* const newHandle,
                                        bool* handleChanged, const bool omitVolumeCheck) 
{
	bool b;
	if (!handleChanged)
		handleChanged = &b;

    bool result = VolumeHandleValidator::checkVolumeHandle(handle, newHandle, handleChanged, omitVolumeCheck);

	if (*handleChanged) {
		if (newHandle && dynamic_cast<BrickedVolume*>(newHandle->getVolume()) ) {
			LargeVolumeManager* lvm = newHandle->getLargeVolumeManager();
			if (lvm) {
				if (brickingUpdateStrategy_->get() == 1) {
					lvm->setUpdateBricks(true);
				} else {
					lvm->setUpdateBricks(false);
				}
			}

		}
	}
	return result;
}

} // namespace voreen
