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
    , segment_(setSegment_, "Active segment", 0)
    , useSegmentation_(switchSegmentation_, "Use Segmentation", false, true, true)
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
}

/*
    further methods
*/
std::string VolumeRaycaster::generateHeader() {
    std::string headerSource = VolumeRenderer::generateHeader();

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

} // namespace voreen
