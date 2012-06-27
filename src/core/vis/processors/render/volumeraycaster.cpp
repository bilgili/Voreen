/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/portmapping.h"

#include "tgt/vector.h"

using tgt::mat4;

namespace voreen {

/*
    init statics
*/

const Identifier VolumeRaycaster::setRaycastingQualityFactor_("set.RaycastingQualityFactor");
const Identifier VolumeRaycaster::selectRaycaster_         = "select.Raycaster";
const Identifier VolumeRaycaster::setSegment_              = "set.Segment";
const Identifier VolumeRaycaster::switchSegmentation_      = "switch.Segmentation";
const Identifier VolumeRaycaster::switchGradientsOnTheFly_ = "switch.GradientsOnTheFly";

const Identifier VolumeRaycaster::entryParamsTexUnit_      = "entryParamsTexUnit";
const Identifier VolumeRaycaster::entryParamsDepthTexUnit_ = "entryParamsDepthTexUnit";
const Identifier VolumeRaycaster::exitParamsTexUnit_       = "exitParamsTexUnit";
const Identifier VolumeRaycaster::exitParamsDepthTexUnit_  = "exitParamsDepthTexUnit";
const Identifier VolumeRaycaster::volTexUnit_              = "volTexUnit";
const Identifier VolumeRaycaster::volTexUnit2_             = "volTexUnit2";
const Identifier VolumeRaycaster::volTexUnit3_             = "volTexUnit3";
const Identifier VolumeRaycaster::transferTexUnit_         = "transferTexUnit";
const Identifier VolumeRaycaster::transferTexUnit2_        = "transferTexUnit2";
const Identifier VolumeRaycaster::transferTexUnit3_        = "transferTexUnit3";
const Identifier VolumeRaycaster::segmentationTexUnit_     = "segmentationTexUnit";
const Identifier VolumeRaycaster::ambTexUnit_              = "ambTexUnit";
const Identifier VolumeRaycaster::ambLookupTexUnit_        = "ambLookupTexUnit";
const Identifier VolumeRaycaster::normalsTexUnit_          = "normalsTexUnit";
const Identifier VolumeRaycaster::gradientMagnitudesTexUnit_ = "gradientMagnitudesTexUnit";

/*
    constructor and destructor
*/

VolumeRaycaster::VolumeRaycaster(std::string fragmentShaderFilename)
    : VolumeRenderer()
    , needRecompileShader_(true)
    , raycastPrg_(0)
    , fragmentShaderFilename_(fragmentShaderFilename)
    , splitMode_("set.splitMode", "Set Splitmode", &needRecompileShader_, 0, 0, 10)
    , pg_(0)
    , raycastingQualityFactor_(setRaycastingQualityFactor_, "Raycasting Quality", &needRecompileShader_, 1.0f)
    , segment_(setSegment_, "Set Segment", 0)
    , useSegmentation_(switchSegmentation_, "Use Segmentation", &needRecompileShader_, false)
{
    // set texture unit identifiers and register
    std::vector<Identifier> units;
    units.push_back(entryParamsTexUnit_);
    units.push_back(entryParamsDepthTexUnit_);
    units.push_back(exitParamsTexUnit_);
    units.push_back(exitParamsDepthTexUnit_);
    units.push_back(volTexUnit_);
    units.push_back(volTexUnit2_);
    units.push_back(volTexUnit3_);
    units.push_back(transferTexUnit_);
    units.push_back(segmentationTexUnit_);
    tm_.registerUnits(units);

    initProperties();
}

VolumeRaycaster::~VolumeRaycaster() {
    if (raycastPrg_)
        ShdrMgr.dispose(raycastPrg_);

    if (raycastingQualitiesEnumProp_)
        delete raycastingQualitiesEnumProp_;
}

void VolumeRaycaster::setVolumeHandle(VolumeHandle* const handle) {
    VolumeRenderer::setVolumeHandle(handle);
    if ( currentVolumeHandle_ == 0 )
        return;
    
    VolumeGL* volumeGL = currentVolumeHandle_->getVolumeGL();
    if ( (volumeGL != 0) && (volumeGL->getNumTextures() > 1) ) {
        // free tex units
        for (size_t i = 0; i < splitNames_.size(); ++i)
            tm_.removeTexUnit(splitNames_[i]);
        splitNames_.clear();

        // set split mode
        splitMode_.set(volumeGL->getNumTextures());

        // alloc tex units
        for (int i = 0; i < splitMode_.get(); i++) {
            // create proper name
            std::ostringstream oss;
            oss << "split" << i;
            // register in the TexUnitManager
            tm_.addTexUnit(oss.str());
            splitNames_.push_back(oss.str());
        }
    }
    else {
        // free tex units
        for (size_t i = 0; i < splitNames_.size(); ++i)
            tm_.removeTexUnit(splitNames_[i]);
        splitNames_.clear();
        splitMode_.set(0);
    }
    invalidateShader();
}

/*
    further methods
*/

std::string VolumeRaycaster::generateHeader() {
    std::string headerSource = VolumeRenderer::generateHeader();

	// enable support for multiple render targets
	//headerSource += "#extension GL_ARB_draw_buffers : enable\n";

	if (getTransFunc())
        headerSource += getTransFunc()->getShaderDefines();
    else
        headerSource += "#define TF_INTENSITY\n";

    
    if (maskingMode_->get() == 3 || maskingMode_->get() == 4) // FIXME: HACK needed until USE_SGEMENTATION is obsolete
		headerSource += "#define USE_SEGMENTATION\n";
/*
	// include required shader modules
	headerSource += "#include \"modules/mod_sampler2d.frag\"\n";
	headerSource += "#include \"modules/mod_sampler3d.frag\"\n";
	headerSource += "#include \"modules/mod_raysetup.frag\"\n";
	headerSource += "#include \"modules/mod_masking.frag\"\n";
	headerSource += "#include \"modules/mod_gradients.frag\"\n";
	headerSource += "#include \"modules/mod_transfunc.frag\"\n";
	headerSource += "#include \"modules/mod_shading.frag\"\n";
	headerSource += "#include \"modules/mod_compositing.frag\"\n";
	headerSource += "#include \"modules/mod_depth.frag\"\n";
*/

	// configure masking
	headerSource += "#define RC_NOT_MASKED(samplePos, intensity) ";
	switch (maskingMode_->get()) {
		case 0:
            headerSource += "true\n";
			break;
		case 1:
            headerSource += "inThresholdInterval(intensity)\n";
			break;
		case 2:
            headerSource += "inSegmentation(samplePos)\n";
			break;
		case 3:
            headerSource += "inThresholdInterval(intensity) && inSegmentation(samplePos)\n";
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
	headerSource += "#define RC_APPLY_CLASSIFICATION(voxel) ";
	switch (classificationMode_->get()) {
		case 0:
            headerSource += "vec4(voxel.a);\n";
			break;
		case 1:
            headerSource += "applyTF(voxel);\n";
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
            headerSource += "phongShadingS(gradient, samplePos, volumeParameters, kd);\n";
			break;
		case 3: headerSource += "phongShadingDA(gradient, samplePos, volumeParameters, ka, kd);\n";
			break;
		case 4:
            headerSource += "phongShadingDS(gradient, samplePos, volumeParameters, ka, kd);\n";
			break;
		case 5:
            headerSource += "phongShading(gradient, samplePos, volumeParameters, ka, kd, ks);\n";
			break;
		case 6:
            headerSource += "toonShading(gradient, samplePos, volumeParameters, kd, 3);\n";
			break;
	}

	// configure compositing mode
	headerSource += "#define RC_APPLY_COMPOSITING(result, color, samplePos, gradient, t) ";
	switch (compositingMode_->get()) {
		case 0:
            headerSource += "compositeDVR(color, result, t, tDepth);\n";
			break;
		case 1:
            headerSource += "compositeMIP(color, result, t, tDepth);\n";
			break;
		case 2:
            headerSource += "compositeISO(color, result, t, tDepth, 0.5);\n";
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
	raycastingQualities_.push_back("very low");
    raycastingQualities_.push_back("low");
    raycastingQualities_.push_back("normal");
    raycastingQualities_.push_back("high");
    raycastingQualities_.push_back("higher");
    raycastingQualities_.push_back("highest");
    raycastingQualitiesEnumProp_ = new EnumProp(setRaycastingQualityFactor_, "Sampling Rate",
                                                raycastingQualities_, &needRecompileShader_, 2);
    addProperty(raycastingQualitiesEnumProp_);

	// initialization of the rendering properties
	// the properties are added in the respective subclasses
	maskingModes_.push_back("none");
	maskingModes_.push_back("Thresholding");
	maskingModes_.push_back("Segmentation");
	maskingModes_.push_back("Thresholding+Segmentation");
	maskingMode_ = new EnumProp("set.masking", "Masking", maskingModes_, &needRecompileShader_, 1);

	gradientModes_.push_back("none");
	gradientModes_.push_back("Forward Differences");
	gradientModes_.push_back("Central Differences");
	gradientModes_.push_back("Filtered");
	gradientMode_ = new EnumProp("set.gradient", "Gradient Calculation", gradientModes_, &needRecompileShader_, 1);

	classificationModes_.push_back("none");
	classificationModes_.push_back("Transfer Function");
	classificationMode_ = new EnumProp("set.classification", "Classification", classificationModes_, &needRecompileShader_, 1);

	shadeModes_.push_back("none");
	shadeModes_.push_back("Phong (Diffuse)");
	shadeModes_.push_back("Phong (Specular)");
	shadeModes_.push_back("Phong (Diffuse+Ambient)");
	shadeModes_.push_back("Phong (Diffuse+Specular)");
	shadeModes_.push_back("Phong (Full)");
	shadeModes_.push_back("Toon");
	shadeMode_ = new EnumProp("set.shading", "Shading", shadeModes_, &needRecompileShader_, 5);

	compositingModes_.push_back("DVR");
	compositingModes_.push_back("MIP");
	compositingModes_.push_back("ISO");
	compositingModes_.push_back("FHP");
	compositingModes_.push_back("FHN");
	compositingMode_ = new EnumProp("set.compositing", "Compositing", compositingModes_, &needRecompileShader_, 0);
}

void VolumeRaycaster::setPropertyDestination(Identifier tag) {
    MessageReceiver::setTag(tag);
    raycastingQualitiesEnumProp_->setMsgDestination(tag);
    lowerTH_.setMsgDestination(tag);
    upperTH_.setMsgDestination(tag);
}

void VolumeRaycaster::restoreMatrixStacks() {
}

int VolumeRaycaster::initializeGL() {
    return VRN_OK;
}

void VolumeRaycaster::compileShader() {
    if (needRecompileShader_) {
        compile();
        needRecompileShader_ = false;
    }
}

void VolumeRaycaster::invalidateShader() {
    needRecompileShader_ = true;
}

void VolumeRaycaster::processMessage(Message* msg, const Identifier& dest/*=Message::all_*/) {
	VolumeRenderer::processMessage(msg, dest);

    if (msg->id_ == "msg.invalidate") { //FIXME: hack, shouldn't be necessary, but still needed
                                        // sometimes to enforce redraw.
        std::cout << "VolumeRaycaster::processMessage: msg.invalidate was sent - THIS IS A HACK!"
                  << std::endl;
        invalidate();
    }
	else if (msg->id_ == setSegment_) {
        segment_.set(msg->getValue<int>());
        invalidate();
    }
	else if (msg->id_ == switchSegmentation_) {
		useSegmentation_.set(msg->getValue<bool>());
		invalidate();
	}
    else if (msg->id_ == setRaycastingQualityFactor_) {
        std::string method = msg->getValue<std::string>();
        if (method == raycastingQualities_[0])
            raycastingQualityFactor_.set(0.2f);
        else if (method == raycastingQualities_[1])
            raycastingQualityFactor_.set(0.5f);
        else if (method == raycastingQualities_[2])
            raycastingQualityFactor_.set(1.0f);
        else if (method == raycastingQualities_[3])
            raycastingQualityFactor_.set(2.0f);
        else if (method == raycastingQualities_[4])
            raycastingQualityFactor_.set(5.f);
        else if (method == raycastingQualities_[5])
            raycastingQualityFactor_.set(10.f);
        invalidate();
    }
}

void VolumeRaycaster::setGlobalShaderParameters(tgt::Shader* shader) {
    VolumeRenderer::setGlobalShaderParameters(shader);

    // raycasting quality factor
    int loc = shader->getUniformLocation("raycastingQualityFactor_", true);
    if (loc != -1)
        shader->setUniform( loc, raycastingQualityFactor_.get());

    loc = shader->getUniformLocation("raycastingQualityFactorRCP_", true);
    if (loc != -1)
        shader->setUniform(loc, 1.f / raycastingQualityFactor_.get());
}

} // namespace voreen
