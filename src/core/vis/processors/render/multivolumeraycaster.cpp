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

#include "voreen/core/vis/processors/render/multivolumeraycaster.h"
#include "voreen/core/vis/processors/entryexitpoints/depthpeelingentryexitpoints.h"

#include "voreen/core/volume/modality.h"
#include "voreen/core/voreenglobal.h"

namespace voreen {

MultiVolumeRaycaster::MultiVolumeRaycaster()
    : VolumeRaycaster()
    , transferFunc_("transferFunction", "transfer function")
    , transferFunc2_("transferFunction2", "transfer function2")
    , transferFunc3_("transferFunction2", "transfer function3")
    , dpEepPort_(Port::INPORT, "coprocessor.depthpeeling")
    , private1_(Port::INPORT, "image.img1")
    , private2_(Port::INPORT, "image.img2")
    , private3_(Port::INPORT, "image.img3")
    , private4_(Port::INPORT, "image.img4")
    , private5_(Port::INPORT, "image.img5")
    , private6_(Port::INPORT, "image.img6")
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle")
    , layersInport_(Port::INPORT, "image.layers", true)
    , outport_(Port::OUTPORT, "image.output")
{

/*    compositingMode1_ = new EnumProperty("compositing1", "Compositing (Second Volume)", compositingModes_, 0, Processor::INVALID_PROGRAM);
    compositingMode2_ = new EnumProperty("compositing2", "Compositing (Third Volume)", compositingModes_, 0, Processor::INVALID_PROGRAM); */

    addProperty(transferFunc_);
    addProperty(transferFunc2_);
    addProperty(transferFunc3_);

    addProperty(maskingMode_);
    addProperty(gradientMode_);
    addProperty(classificationMode_);
    addProperty(shadeMode_);

    addProperty(compositingMode_);
//    addProperty(compositingMode1_);
//    addProperty(compositingMode2_);

    addProperty(lightPosition_);
    addProperty(lightAmbient_);
    addProperty(lightDiffuse_);
    addProperty(lightSpecular_);

    /* Private ports containing textures needed for saving intermediate texures*/
    addPrivateRenderPort(&private1_);
    addPrivateRenderPort(&private2_);
    addPrivateRenderPort(&private3_);
    addPrivateRenderPort(&private4_);
    addPrivateRenderPort(&private5_);
    addPrivateRenderPort(&private6_);

    addPort(volumeInport_); // Multi Input Port for the 3D textures
    addPort(layersInport_); // Multi Input port for the 2D textures
    addPort(outport_);
    addPort(dpEepPort_);

    numOfVolumes_ = 0;
    activ1 = false;
    activ2 = false;
    activ3 = false;

    tm_.addTexUnit("texunit.multivolumelayer0");
    tm_.addTexUnit("texunit.multivolumelayer1");
    tm_.addTexUnit("texunit.multivolumelayer2");
}

MultiVolumeRaycaster::~MultiVolumeRaycaster() {}

const std::string MultiVolumeRaycaster::getProcessorInfo() const {
    return "Performs a multivolume raycasting based on depth peeling. Up to three volumes can be fused.";
}

Processor* MultiVolumeRaycaster::create() const {
    return new MultiVolumeRaycaster();
}

void MultiVolumeRaycaster::initialize() throw (VoreenException) {

    VolumeRaycaster::initialize();

    loadShader();

    if(!raycastPrg_ || !raycastPrg2_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }

    setLightingParameters();

    initialized_ = true;

}

void MultiVolumeRaycaster::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "rc_multivolume.frag",
                                       generateHeader(), false, false);
    raycastPrg2_ = ShdrMgr.loadSeparate("pp_identity.vert", "multivolumeblender.frag",
                                       generateHeader(), false, false);

}
int MultiVolumeRaycaster::lookup(int objectid){

    for (unsigned int c = 0 ; c < map.size() ; ++c){
        if (map.at(c) == objectid)
            return c;
    }
    return -1;
}

void MultiVolumeRaycaster::compile() {
    raycastPrg_->setHeaders(generateHeader(), false);
    raycastPrg_->rebuild();

    raycastPrg2_->setHeaders(generateHeader2(), false);
    raycastPrg2_->rebuild();

}

void MultiVolumeRaycaster::process() {

    // Todo: port merge

    /*if (!dpEepPort_.isConnected())
        return;

   // check that atleast there is one volume to render
    currentVolumeHandle_ = volumeInport_.getData();

    if (currentVolumeHandle_ == 0)
        return;

    std::vector<VolumeHandle*> vecVolumes = volumeInport_.getAllData(); // Read all volumes in the scene
    numOfVolumes_ = vecVolumes.size();
    std::vector<RenderTarget*> layers = layersInport_.getAllData(); // Read all the different entry/exit params resulting from depth peeling

    // At least layers for one volume
    if (layers.size() <= 1)
        return;

    int layerNumbers = dpEepPort_.getConnectedProcessor()->getNumLayers();
    map = dpEepPort_.getConnectedProcessor()->getMap();

    std::vector<int> targets;
    targets.push_back(getTarget("image.img1"));
    targets.push_back(getTarget("image.img2"));
    targets.push_back(getTarget("image.img3"));
    targets.push_back(getTarget("image.img4"));
    targets.push_back(getTarget("image.img5"));
    targets.push_back(getTarget("image.img6"));

    int finalTarget = getTarget("image.output");

    // compile program if needed
    if(getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
    LGL_ERROR;

    // TODO: adapt further to new port concept (ab)


    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;
    int r;
    // set transfer functions for volumes found in the scene
    for (unsigned int q = 0; q < vecVolumes.size() && q < 3; ++q) {
        VolumeHandleValidator::checkVolumeHandle(currentVolumeHandle_,
                                                 getVolumeHandle("volumehandle.volumehandle", q));
        if (q == 0) {
            transferFunc_.setVolumeHandle(currentVolumeHandle_);
            glActiveTexture(tm_.getGLTexUnit(transferTexUnit_));
            if (transferFunc_.get())
                transferFunc_.get()->bind();
            activ1 = true;

        }
        else if (q == 1) {
            transferFunc2_.setVolumeHandle(currentVolumeHandle_);
            glActiveTexture(tm_.getGLTexUnit(transferTexUnit2_));
            if (transferFunc2_.get())
                transferFunc2_.get()->bind();
            activ2 = true;

        }
        else {
            transferFunc3_.setVolumeHandle(currentVolumeHandle_);
            glActiveTexture(tm_.getGLTexUnit(transferTexUnit3_));
            if (transferFunc3_.get())
                transferFunc3_.get()->bind();
            activ3 = true;
        }

        r = lookup(currentVolumeHandle_->getObjectID());  // get unique ID of the current volume
         // add volume
        volumeTextures.push_back(VolumeStruct(
        currentVolumeHandle_->getVolumeGL(),
        "texunit.multivolumelayer"+itos(r),
        "volume"+itos(r)+"_",
        "volumeParameters"+itos(r)+"_")
            );
    }

    // Multi-Pass rendering
    for (int p = 0; p < layerNumbers-1; ++p) {
        tc_->setActiveTarget(targets.at(p) , "MMRC_output"+itos(p));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        LGL_ERROR;
        // bind entry params
        glActiveTexture(tm_.getGLTexUnit(entryParamsTexUnit_));
        glBindTexture(tc_->getGLTexTarget(layers.at(p)->textureID_), tc_->getGLTexID(layers.at(p)->textureID_));
        glActiveTexture(tm_.getGLTexUnit(entryParamsDepthTexUnit_));
        glBindTexture(tc_->getGLDepthTexTarget(layers.at(p)->textureID_), tc_->getGLDepthTexID(layers.at(p)->textureID_));
        LGL_ERROR;

        // bind exit params
        glActiveTexture(tm_.getGLTexUnit(exitParamsTexUnit_));
        glBindTexture(tc_->getGLTexTarget(layers.at(p+1)->textureID_), tc_->getGLTexID(layers.at(p+1)->textureID_));
        glActiveTexture(tm_.getGLTexUnit(exitParamsDepthTexUnit_));
        glBindTexture(tc_->getGLDepthTexTarget(layers.at(p+1)->textureID_), tc_->getGLDepthTexID(layers.at(p+1)->textureID_));
        LGL_ERROR;

           //segmentation volume

        VolumeHandle* volumeSeg = currentVolumeHandle_->getRelatedVolumeHandle(Modality::MODALITY_SEGMENTATION);
        bool usingSegmentation = (maskingMode_->get() == 1 ) && volumeSeg;
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

        // initialize rendering shader
        raycastPrg_->activate();
        // set common uniforms used by athe rendering shaders
        setGlobalShaderParameters(raycastPrg_);
        // bind the volumes and pass the necessary information to the rendering shader
        bindVolumes(raycastPrg_, volumeTextures);
        // pass the remaining uniforms to the rendering shader
        raycastPrg_->setUniform("entryPoints_", tm_.getTexUnit(entryParamsTexUnit_));
        raycastPrg_->setUniform("entryPointsDepth_", tm_.getTexUnit(entryParamsDepthTexUnit_));
        raycastPrg_->setUniform("exitPoints_", tm_.getTexUnit(exitParamsTexUnit_));
        raycastPrg_->setUniform("exitPointsDepth_", tm_.getTexUnit(exitParamsDepthTexUnit_));
        if (classificationMode_->get() == 1){
            if(activ1){
                raycastPrg_->setUniform("transferFunc_", tm_.getTexUnit(transferTexUnit_));
            }
            if(activ2){
                raycastPrg_->setUniform("transferFunc2_", tm_.getTexUnit(transferTexUnit2_));
            }
            if(activ3){
                raycastPrg_->setUniform("transferFunc3_", tm_.getTexUnit(transferTexUnit3_));
            }
        }

        if (usingSegmentation) {
            GLfloat seg = segment_.get() / 255.f;
            raycastPrg_->setUniform("segment_" , seg);
        }
        renderQuad();
        raycastPrg_->deactivate();


        if (usingSegmentation) {
            // restore default texture filtering mode
            glActiveTexture(tm_.getGLTexUnit(segmentationTexUnit_));
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
    }/// end of Multi pass rendering loop
     glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
     LGL_ERROR;

     // For blending the layers produced by the multi-pass rendering shader
     if(layerNumbers==4){
        tc_->setActiveTarget(finalTarget,"MMRC_output4");
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // initialize blending shader
        raycastPrg2_->activate();
        // First Output
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tc_->getGLTexTarget(targets.at(0)), tc_->getGLTexID(targets.at(0)));
        LGL_ERROR;
        // Second Output
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(tc_->getGLTexTarget(targets.at(1)), tc_->getGLTexID(targets.at(1)));
        LGL_ERROR;
        // Third Output
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(tc_->getGLTexTarget(targets.at(2)), tc_->getGLTexID(targets.at(2)));
        LGL_ERROR;
        // set common uniforms used by blending shaders
        setGlobalShaderParameters(raycastPrg2_);
        // pass the remaining uniforms to the blending shader
        raycastPrg2_->setUniform("layer1_", 0);//GL_TEXTURE0);
        raycastPrg2_->setUniform("layer2_", 1);//GL_TEXTURE1);
        raycastPrg2_->setUniform("layer3_", 2);//GL_TEXTURE1);
        renderQuad();
        raycastPrg2_->deactivate();
    }
    else if(layerNumbers==6){

        // The first three layers are blended together
        tc_->setActiveTarget(targets.at(5),"MMRC_output");
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // initialize blending shader
        raycastPrg2_->activate();
        //First Output
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tc_->getGLTexTarget(targets.at(0)), tc_->getGLTexID(targets.at(0)));
        LGL_ERROR;
        //Second Output
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(tc_->getGLTexTarget(targets.at(1)), tc_->getGLTexID(targets.at(1)));
        LGL_ERROR;
        //Third Output
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(tc_->getGLTexTarget(targets.at(2)), tc_->getGLTexID(targets.at(2)));
        LGL_ERROR;

        // set common uniforms used by the blending shaders
        setGlobalShaderParameters(raycastPrg2_);
        // pass the remaining uniforms to the blending shader
        raycastPrg2_->setUniform("layer1_", 0);//GL_TEXTURE0);
        raycastPrg2_->setUniform("layer2_", 1);//GL_TEXTURE1);
        raycastPrg2_->setUniform("layer3_", 2);//GL_TEXTURE1);
        renderQuad();
        raycastPrg2_->deactivate();

        // The last two layers are blended together with the blended output of first three layers
        tc_->setActiveTarget(finalTarget,"MMRC_output");
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        raycastPrg2_->activate();
        //Fourth Output
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tc_->getGLTexTarget(targets.at(3)), tc_->getGLTexID(targets.at(3)));
        LGL_ERROR;
        //Fifth Output
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(tc_->getGLTexTarget(targets.at(4)), tc_->getGLTexID(targets.at(4)));
        LGL_ERROR;
        //First three layers blended
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(tc_->getGLTexTarget(targets.at(5)), tc_->getGLTexID(targets.at(5)));
        LGL_ERROR;
        // set common uniforms used by all shaders
        setGlobalShaderParameters(raycastPrg2_);
        // pass the remaining uniforms to the shader
        raycastPrg2_->setUniform("layer1_", 0);//GL_TEXTURE0);
        raycastPrg2_->setUniform("layer2_", 1);//GL_TEXTURE1);
        raycastPrg2_->setUniform("layer3_", 2);//GL_TEXTURE1);
        renderQuad();
        raycastPrg2_->deactivate();

    }

    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR; */
}

std::string MultiVolumeRaycaster::generateHeader() {

    /* TODO: use option properties

    std::string headerSource = VolumeRaycaster::generateHeader();

    headerSource += "#include \"modules/vrn_shaderincludes.frag\"\n";
    headerSource += "#define SAMPLER_3D_NUMBER "+itos(numOfVolumes_)+"\n";

    for(int r=0; r<3; ++r){
        headerSource += "uniform sampler3D volume" + itos(r) + "_; \n";
        headerSource += "uniform VOLUME_PARAMETERS volumeParameters" + itos(r) + "_; \n";
    }

    // configure compositing mode for second volume
    headerSource += "#define RC_APPLY_COMPOSITING_1(result, color, samplePos, gradient, t, tDepth) ";
    switch (compositingMode1_->get()) {
        case 0: headerSource += "compositeDVR(result, color, t, tDepth);\n";
            break;
        case 1: headerSource += "compositeMIP(result, color, t, tDepth);\n";
            break;
        case 2: headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
            break;
        case 3: headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
            break;
        case 4: headerSource += "compositeFHN(gradient, result, t, tDepth);\n";
            break;
    }

    // configure compositing mode for third volume
    headerSource += "#define RC_APPLY_COMPOSITING_2(result, color, samplePos, gradient, t, tDepth) ";
    switch (compositingMode2_->get()) {
        case 0: headerSource += "compositeDVR(result, color, t, tDepth);\n";
            break;
        case 1: headerSource += "compositeMIP(result, color, t, tDepth);\n";
            break;
        case 2: headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
            break;
        case 3: headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
            break;
        case 4: headerSource += "compositeFHN(gradient, result, t, tDepth);\n";
            break;
    }

    if ((compositingMode_->get() == 1) && (compositingMode1_->get() == 1))
        headerSource += "#define RC_APPLY_COMPOSITING_Multi01(result, color, samplePos, gradient, t, tDepth) compositeMIP(result, color, t, tDepth);\n";
    else
        headerSource += "#define RC_APPLY_COMPOSITING_Multi01(result, color, samplePos, gradient, t, tDepth) compositeDVR(result, color, t, tDepth);\n";

    if ((compositingMode1_->get() == 1) && (compositingMode2_->get() ==1 ))
        headerSource += "#define RC_APPLY_COMPOSITING_Multi12(result, color, samplePos, gradient, t, tDepth) compositeMIP(result, color, t, tDepth);\n";
    else
        headerSource += "#define RC_APPLY_COMPOSITING_Multi12(result, color, samplePos, gradient, t, tDepth) compositeDVR(result, color, t, tDepth);\n";

    if ((compositingMode_->get() == 1) && (compositingMode2_->get() == 1))
        headerSource += "#define RC_APPLY_COMPOSITING_Multi02(result, color, samplePos, gradient, t, tDepth) compositeMIP(result, color, t, tDepth);\n";
    else
        headerSource += "#define RC_APPLY_COMPOSITING_Multi02(result, color, samplePos, gradient, t, tDepth) compositeDVR(result, color, t, tDepth);\n";

    if ((compositingMode_->get() == 1) && (compositingMode2_->get() == 1))
        headerSource += "#define RC_APPLY_COMPOSITING_Multi012(result, color, samplePos, gradient, t, tDepth) compositeMIP(result, color, t, tDepth);\n";
    else if ((compositingMode1_->get() == 1) && (compositingMode2_->get() == 1))
        headerSource += "#define RC_APPLY_COMPOSITING_Multi012(result, color, samplePos, gradient, t, tDepth) compositeMIP(result, color, t, tDepth);\n";
    else if ((compositingMode_->get() == 1) && (compositingMode1_->get() == 1))
        headerSource += "#define RC_APPLY_COMPOSITING_Multi012(result, color, samplePos, gradient, t, tDepth) compositeMIP(result, color, t, tDepth);\n";
    else
        headerSource += "#define RC_APPLY_COMPOSITING_Multi012(result, color, samplePos, gradient, t, tDepth) compositeDVR(result, color, t, tDepth);\n";

    headerSource += "#define TF_SAMPLER_TYPE0 " + transferFunc_.get()->getSamplerType() + "\n";
    headerSource += "#define TF_SAMPLER_TYPE1 " + transferFunc2_.get()->getSamplerType() + "\n";
    headerSource += "#define TF_SAMPLER_TYPE2 " + transferFunc3_.get()->getSamplerType() + "\n";

    return headerSource; */

    return "";
}

std::string MultiVolumeRaycaster::generateHeader2() {
    std::string headerSource = VolumeRaycaster::generateHeader();

    headerSource += "#include \"modules/vrn_shaderincludes.frag\"\n";

    return headerSource;
}

} // namespace

