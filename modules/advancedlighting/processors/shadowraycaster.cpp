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

#include "shadowraycaster.h"

#include "tgt/framebufferobject.h"

#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/utils/voreenpainter.h"

#include <cmath>
#include "tgt/glmath.h"
#include "tgt/textureunit.h"

#ifdef VRN_MODULE_DEVIL
    #include <IL/il.h>
#endif

//#define SHOW_BLEND_TEXTURE

using tgt::vec3;
using tgt::TextureUnit;

namespace voreen {

ShadowRaycaster::ShadowRaycaster()
    : VolumeRaycaster()
    , shaderProp_("raycast.prg", "Raycasting shader", "rc_shadow.frag", "passthrough.vert")
    , transferFunc_("transferFunction", "Transfer function")
    , camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)), true)
    , shadowsActive_("shadowsActive", "Scattering+Shadowing", false, Processor::INVALID_PROGRAM)
    , zSlice_("setZslice", "Rendered z slice", 0, 0, 256)
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle", "Volume Input", false, Processor::INVALID_PROGRAM)
    , entryPort_(Port::INPORT, "image.entrypoints", "Entry-points Input", false, Processor::INVALID_RESULT)
    , exitPort_(Port::INPORT, "image.exitpoints", "Exit-points Input", false, Processor::INVALID_RESULT)
    , outport_(Port::OUTPORT, "image.output", "Image Output", true, Processor::INVALID_RESULT)
    , internalRenderPort_(Port::OUTPORT, "internalRenderPort", "Internal Render Port")
{
    addProperty(shaderProp_);
    addProperty(transferFunc_);

    addProperty(shadowsActive_);
    shadowsActive_.onChange(CallMemberAction<ShadowRaycaster>(this, &ShadowRaycaster::shadowModeChanged));

    renderMode_ = new OptionProperty<bool>("rendermode", "Render mode");
    renderMode_->addOption("volume", "Render Volume", false);
    renderMode_->addOption("slices", "Render Slices", true);
    renderMode_->selectByValue(false);
    addProperty(renderMode_);
    renderMode_->onChange(CallMemberAction<ShadowRaycaster>(this, &ShadowRaycaster::renderModeChanged));

    addProperty(zSlice_);
    zSlice_.setVisible(false);

    lightVolumeGenerator_ = new LightVolumeGenerator();
    lightVolumeGenerator_->setTransFunc(&transferFunc_);
    lightVolumeProperties_ = lightVolumeGenerator_->getProperties();
    for (size_t i = 0; i < lightVolumeProperties_.size(); ++i) {
        if (!getProperty(lightVolumeProperties_[i]->getID())) {
            addProperty(lightVolumeProperties_[i]);
            lightVolumeProperties_[i]->setVisible(false);
            //lightVolumeProperties_[i]->setOwner(lightVolumeGenerator_);
        }
    }

    addProperty(lightPosition_);
    lightPosition_.onChange(Call1ParMemberAction<LightVolumeGenerator, LightSourceProperty*>(lightVolumeGenerator_, &LightVolumeGenerator::setLightPosition, &lightPosition_));
    lightPosition_.setVisible(true);

    addProperty(camera_);

    addPort(volumeInport_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);

    addPrivateRenderPort(internalRenderPort_);
}

ShadowRaycaster::~ShadowRaycaster() {
    delete renderMode_;
    delete lightVolumeGenerator_;
}

Processor* ShadowRaycaster::create() const {
    return new ShadowRaycaster();
}

void ShadowRaycaster::initialize() throw (tgt::Exception) {

    VolumeRaycaster::initialize();
    compile();

    if(!shaderProp_.hasValidShader()) {
        LERROR("Failed to load shaders!");
        processorState_ = PROCESSOR_STATE_NOT_INITIALIZED;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }
    lightVolumeGenerator_->initialize();

    processorState_ = PROCESSOR_STATE_NOT_READY;
}

void ShadowRaycaster::deinitialize() throw (tgt::Exception) {

    VolumeRaycaster::deinitialize();

    if (lightVolumeGenerator_) {
        lightVolumeGenerator_->deinitialize();
        lightVolumeGenerator_->processorState_ = PROCESSOR_STATE_NOT_INITIALIZED;
        delete lightVolumeGenerator_;
        lightVolumeGenerator_ = 0;
    }
}

void ShadowRaycaster::compile() {
    shaderProp_.setHeader(generateHeader());
    shaderProp_.rebuild();
}

std::string ShadowRaycaster::generateHeader() {
    std::string header = VolumeRaycaster::generateHeader();

    header += transferFunc_.get()->getShaderDefines();

    if (shadowsActive_.get()) {
        header += "#define SHADOWS_ACTIVE\n";
        header += lightVolumeGenerator_->generateShadowHeader();
    }

    return header;
}

void ShadowRaycaster::process() {

    if (!volumeInport_.isReady() || !shaderProp_.hasValidShader())
        return;

    if (volumeInport_.hasChanged()) {
        invalidate(Processor::INVALID_PROGRAM);
        transferFunc_.setVolumeHandle(volumeInport_.getData());
        lightVolumeGenerator_->setVolumeHandle(volumeInport_.getData());
        if(volumeInport_.hasData())
            camera_.adaptInteractionToScene(volumeInport_.getData()->getBoundingBox().getBoundingBox());
    }

    TextureUnit lightUnit, blendUnit;
    lightUnit.activate();
    blendUnit.activate();
    TextureUnit::setZeroUnit();

    if (shadowsActive_.get())
        lightVolumeGenerator_->generateLightVolume(&lightUnit, &blendUnit);

    const bool renderCoarse = interactionMode() && interactionCoarseness_.get() > 1;
    tgt::svec2 renderSize;
    if (renderCoarse) {
        renderSize = outport_.getSize() / interactionCoarseness_.get();
        internalRenderPort_.resize(renderSize);
    }
    else {
        renderSize = outport_.getSize();
    }

    RenderPort& renderDestination = (renderCoarse ? internalRenderPort_ : outport_);
    renderDestination.activateTarget();
    renderDestination.clearTarget();

    if (renderMode_->getValue())
        renderSlices();
    else
        renderVolume(&lightUnit, renderSize);

    LGL_ERROR;

    renderDestination.deactivateTarget();

    // copy over rendered image from internal port to outport,
    // thereby rescaling it to outport dimensions
    if (renderCoarse && outport_.isConnected())
        rescaleRendering(internalRenderPort_, outport_);

    TextureUnit::setZeroUnit();
    glDisable(GL_TEXTURE_3D);

    LGL_ERROR;
}

void ShadowRaycaster::renderSlices() {
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.loadMatrix(camera_.get().getProjectionMatrix(outport_.getSize()));
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.loadMatrix(camera_.get().getViewMatrix());
    LGL_ERROR;

    lightVolumeGenerator_->renderSlice(zSlice_.get());

    LGL_ERROR;

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.popMatrix();

}

void ShadowRaycaster::renderVolume(TextureUnit* lightUnit, const tgt::ivec2& renderSize) {

    if (!volumeInport_.isReady())
        return;

    TextureUnit entryUnit, entryDepthUnit, exitUnit, exitDepthUnit;

    // bind entry params
    entryPort_.bindTextures(entryUnit.getEnum(), entryDepthUnit.getEnum(), GL_NEAREST);
    LGL_ERROR;

    // bind exit params
    exitPort_.bindTextures(exitUnit.getEnum(), exitDepthUnit.getEnum(), GL_NEAREST);
    LGL_ERROR;


    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

    // add main volume
    TextureUnit volUnit;
    volumeTextures.push_back(VolumeStruct(
        volumeInport_.getData(),
        &volUnit,
        "volume_","volumeStruct_")
        );

    // bind transfer function
    TextureUnit transferUnit;
    transferUnit.activate();
    if (transferFunc_.get())
        transferFunc_.get()->bind();

    // bind light volume
    //if (shadowsActive_.get())
        //lightVolumeGenerator_->bindLightVolume(lightUnit);

    if(getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
    tgt::Shader* raycastPrg = shaderProp_.getShader();
    raycastPrg->activate();

    // set common uniforms used by all shaders
    tgt::Camera cam = camera_.get();
    setGlobalShaderParameters(raycastPrg, &cam, renderSize);
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(raycastPrg, volumeTextures, &cam, lightPosition_.get());

    // pass the remaining uniforms to the shader
    raycastPrg->setUniform("entryPoints_", entryUnit.getUnitNumber());
    raycastPrg->setUniform("entryPointsDepth_", entryDepthUnit.getUnitNumber());
    entryPort_.setTextureParameters(raycastPrg, "entryParameters_");
    raycastPrg->setUniform("exitPoints_", exitUnit.getUnitNumber());
    raycastPrg->setUniform("exitPointsDepth_", exitDepthUnit.getUnitNumber());
    exitPort_.setTextureParameters(raycastPrg, "exitParameters_");
    if (classificationMode_.get() == "transfer-function")
        transferFunc_.get()->setUniform(raycastPrg, "transferFunc_", "transferFuncTex_", transferUnit.getUnitNumber());

    if (shadowsActive_.get())
        lightVolumeGenerator_->setShaderUniforms(raycastPrg, lightUnit);

    renderQuad();

    raycastPrg->deactivate();

    LGL_ERROR;
}

void ShadowRaycaster::shadowModeChanged() {
    bool active = shadowsActive_.get();
    for (size_t i = 0; i < lightVolumeProperties_.size(); ++i) {
        lightVolumeProperties_[i]->setVisible(active);
    }
}

void ShadowRaycaster::renderModeChanged() {
    zSlice_.setVisible(renderMode_->getValue());
}


//---------------------------------------------------------------------------

LightVolumeGenerator::LightVolumeGenerator()
    : VolumeRaycaster()
    , fbo_(0)
    , raycastPrg_(0)
    , recomputeNeeded_(true)
    , cubeFace_(-1)
    , shadowTexture_(0)
    , signFactor_(-1.f)
    , borderColor_(0.f)
    , fboBlend_(0)
    , cubeFaceBlend_(-1)
    , shadowTextureBlend_(0)
    , signFactorBlend_(-1.f)
    , viewpointTexture_(0)
    , fboViewpoint_(0)
    , signFactorViewpoint_(-1.f)
    , camera_("LightVolumeGenerator.camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , scalingMode_("LightVolumeGenerator.ScalingMode", "Scaling Mode", Processor::INVALID_PROGRAM)
    , neighborhood_("LightVolumeGenerator.Neighborhood", "Neighborhood", Processor::INVALID_PROGRAM)
    , filterModes_("LightVolumeGenerator.filterModes", "Filter Modes", Processor::INVALID_PROGRAM)
    , viewpointModes_("LightVolumeGenerator.viewpointMode", "Change direction", Processor::INVALID_PROGRAM)
    , writeSlices_("write", "Write Slices to images?", false)
    , recomputeOnEveryFrame_("recompute", "Recompute 3D Texture each frame?", false)
    , lightInitialized_(false)
    , verifying_(false)
{
    // remove all properties (raycastingQuality is not needed and lightposition is set by
    // ShadowRaycaster)
    //properties_.clear();

    // init array with normals
    normals_[0] = tgt::vec3(-1.f,  0.f,  0.f);
    normals_[1] = tgt::vec3( 1.f,  0.f,  0.f);
    normals_[2] = tgt::vec3( 0.f, -1.f,  0.f);
    normals_[3] = tgt::vec3( 0.f,  1.f,  0.f);
    normals_[4] = tgt::vec3( 0.f,  0.f, -1.f);
    normals_[5] = tgt::vec3( 0.f,  0.f,  1.f);

    neighborhood_.addOption("0", "no neighborhood", 0);
    neighborhood_.addOption("4", "4 neighbors",     4);
    neighborhood_.addOption("8", "8 neighbors",     8);
    neighborhood_.selectByValue(0);
    neighborhood_.onChange(CallMemberAction<LightVolumeGenerator>(this, &LightVolumeGenerator::recomputeNeeded));
    addProperty(neighborhood_);

    filterModes_.addOption("mean",  "Mean Filter",     "USE_MEAN_FILTER");
    filterModes_.addOption("gauss", "Gaussian Filter", "USE_GAUSSIAN_FILTER");
    filterModes_.selectByValue("USE_MEAN_FILTER");
    filterModes_.setVisible(false);
    filterModes_.onChange(CallMemberAction<LightVolumeGenerator>(this, &LightVolumeGenerator::recomputeNeeded));
    addProperty(filterModes_);
    Call1ParMemberAction<Property, bool> visible(&filterModes_, &Property::setVisible, true);
    Call1ParMemberAction<Property, bool> notVisible(&filterModes_, &Property::setVisible, false);
    neighborhood_.onValueEqual("0", notVisible, visible);

    scalingMode_.addOption("full",    "full dimension",    1.f);
    scalingMode_.addOption("half",    "half dimension",    0.5f);
    scalingMode_.addOption("quarter", "quarter dimension", 0.25f);
    scalingMode_.selectByValue(1.f);
    scalingMode_.onChange(CallMemberAction<LightVolumeGenerator>(this, &LightVolumeGenerator::scalingChanged));
    addProperty(scalingMode_);

    viewpointModes_.addOption("light",  "Lightsource position", LIGHTSOURCE_POSITION);
    viewpointModes_.addOption("camera", "Camera position",      CAMERA_POSITION);
    viewpointModes_.selectByValue(LIGHTSOURCE_POSITION);
    viewpointModes_.onChange(CallMemberAction<LightVolumeGenerator>(this, &LightVolumeGenerator::viewpointModeChanged));
    addProperty(viewpointModes_);

    // debug properties
    addProperty(writeSlices_);
    writeSlices_.onChange(CallMemberAction<LightVolumeGenerator>(this, &LightVolumeGenerator::recomputeNeeded));
    addProperty(recomputeOnEveryFrame_);
    addProperty(camera_);

    currentVolume_ = 0;
}

LightVolumeGenerator::~LightVolumeGenerator() {
}

Processor* LightVolumeGenerator::create() const {
    return new LightVolumeGenerator();
}

std::string LightVolumeGenerator::getProcessorInfo() const {
    return "";
}

void LightVolumeGenerator::initialize() throw (tgt::Exception) {

    VolumeRaycaster::initialize();

    loadShader();

    if (!fbo_)
        fbo_ = new tgt::FramebufferObject();

    if (!fboBlend_)
        fboBlend_ = new tgt::FramebufferObject();

    if (!fboViewpoint_)
        fboViewpoint_ = new tgt::FramebufferObject();

    if(!raycastPrg_ || !fbo_ || !fboBlend_ || !fboViewpoint_) {
        LERROR("Failed to initialize!");
        throw VoreenException(getClassName() + ": Failed to initialize!");
    }
    processorState_ = PROCESSOR_STATE_NOT_READY;
}

void LightVolumeGenerator::deinitialize() throw (tgt::Exception) {
    delete fbo_;
    delete fboBlend_;
    delete shadowTexture_;
    delete shadowTextureBlend_;

    delete viewpointTexture_;
    delete fboViewpoint_;

    ShdrMgr.dispose(raycastPrg_);
    raycastPrg_ = 0;

    VolumeRaycaster::deinitialize();
}

void LightVolumeGenerator::loadShader() {
    if (!raycastPrg_) {
        try {
            raycastPrg_ = ShdrMgr.load("lightvolumegenerator", generateHeader(), false);
        } catch(tgt::Exception) {}
    }
}


std::string LightVolumeGenerator::generateHeader() {
    std::string header = VolumeRaycaster::generateHeader();

    if (transferFunc_->get())
        header += transferFunc_->get()->getShaderDefines();

    std::stringstream s;
    s << neighborhood_.getValue();
    header += "#define NEIGHBORS " + s.str() + "\n";

    header += "#define " + filterModes_.getValue() + "\n";

    return header;
}

std::string LightVolumeGenerator::generateShadowHeader() {
    std::string header = "#include \"mod_shadows.frag\"\n";

    return header;
}

void LightVolumeGenerator::setShaderUniforms(tgt::Shader* shader, TextureUnit* lightUnit) {
#ifndef SHOW_BLEND_TEXTURE
    shader->setUniform("permutationIndex_", indexing_.z);
    shader->setUniform("increment_", -signFactor_ / (shadowTexture_->getDepth() - 1.f));
    shader->setUniform("shadowTexture_", lightUnit->getUnitNumber());
#else
    shader->setUniform("permutationIndex_", indexingBlend_.z);
    shader->setUniform("increment_", -signFactorBlend_ / (shadowTextureBlend_->getDepth() - 1.f));
    shader->setUniform("shadowTexture_", lightUnit->getUnitNumber());
#endif
}

void LightVolumeGenerator::bindLightVolume(TextureUnit* lightUnit) {
    //TextureUnit tempUnit;
    //tempUnit.activate();
    lightUnit->activate();

#ifndef SHOW_BLEND_TEXTURE
    shadowTexture_->enable();
    shadowTexture_->bind();
    shadowTexture_->setWrapping(tgt::Texture::CLAMP_TO_BORDER);
    shadowTexture_->setFilter(tgt::Texture::LINEAR);
    //viewpointTexture_->enable();
    //viewpointTexture_->bind();
    //viewpointTexture_->setWrapping(tgt::Texture::CLAMP_TO_BORDER);
#else
    shadowTextureBlend_->enable();
    shadowTextureBlend_->bind();
    shadowTextureBlend_->setWrapping(tgt::Texture::CLAMP_TO_BORDER);
#endif

    glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor_.elem);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void LightVolumeGenerator::compile() {
    if (raycastPrg_) {
        raycastPrg_->setHeaders(generateHeader());
        raycastPrg_->rebuild();
    }
}

void LightVolumeGenerator::renderSlice(int sliceNumber) {
    //render quad and use shadow texture for texturing
    glActiveTexture(GL_TEXTURE2);
    shadowTexture_->enable();
    shadowTexture_->bind();
    shadowTexture_->setWrapping(tgt::Texture::CLAMP_TO_BORDER);

    glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor_.elem);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    float texCoordZ = (sliceNumber * scalingMode_.getValue())/ (shadowTexture_->getDepth() - 1.f);
    float coordZ = signFactor_ * (texCoordZ * 2.f - 1.f);

    tgt::vec2 coords2D[4];
    if (signFactor_ < 0.f) {
        coords2D[0] = tgt::vec2( 1.f, -1.f);
        coords2D[1] = tgt::vec2(-1.f, -1.f);
        coords2D[2] = tgt::vec2(-1.f,  1.f);
        coords2D[3] = tgt::vec2( 1.f,  1.f);
    }
    else {
        coords2D[0] = tgt::vec2(-1.f, -1.f);
        coords2D[1] = tgt::vec2( 1.f, -1.f);
        coords2D[2] = tgt::vec2( 1.f,  1.f);
        coords2D[3] = tgt::vec2(-1.f,  1.f);
    }

    tgt::vec3 coords3D[4];
    for (int i = 0; i < 4; ++i) {
        coords3D[i][indexing_.x] = coords2D[i].x;
        coords3D[i][indexing_.y] = coords2D[i].y;
        coords3D[i][indexing_.z] = coordZ;

        // scale according to cubesize
        //tgt::vec3 cubeSize = currentVolume_->getCubeSize();
        //coords3D[i] = (coords3D[i] + 1.0f) / 2.f;
        //coords3D[i] = currentVolume_->getTextureToWorldMatrix(false) * coords3D[i];
        //coords3D[i] = coords3D[i] * cubeSize / 2.f;
        coords3D[i] = coords3D[i];
    }

    glBegin(GL_QUADS);
        glMultiTexCoord3f(GL_TEXTURE2, 0, 0, texCoordZ); glVertex3fv(coords3D[0].elem);
        glMultiTexCoord3f(GL_TEXTURE2, 1, 0, texCoordZ); glVertex3fv(coords3D[1].elem);
        glMultiTexCoord3f(GL_TEXTURE2, 1, 1, texCoordZ); glVertex3fv(coords3D[2].elem);
        glMultiTexCoord3f(GL_TEXTURE2, 0, 1, texCoordZ); glVertex3fv(coords3D[3].elem);
    glEnd();
    shadowTexture_->disable();

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    TextureUnit::setZeroUnit();
}

void LightVolumeGenerator::setVolumeHandle(const VolumeBase* handle) {
    currentVolume_ = handle;

    cubeFace_ = -1;
    cubeFaceBlend_ = -1;
    cubeFaceView_ = -1;

    setupTextures();
    verifyCubeFaces();
    recomputeNeeded_ = true;
}

void LightVolumeGenerator::setTransFunc(TransFuncProperty* tf) {
    transferFunc_ = tf;
    transferFunc_->onChange(CallMemberAction<LightVolumeGenerator>(this, &LightVolumeGenerator::transferFunctionChanged));
    transferFunctionChanged();
}

void LightVolumeGenerator::transferFunctionChanged() {
    TransFunc1DKeys* tfInt = dynamic_cast<TransFunc1DKeys*>(transferFunc_->get());
    if (tfInt)
        borderColor_ = tfInt->getMappingForValue(0.f);

    recomputeNeeded_ = true;
}

void LightVolumeGenerator::setLightPosition(LightSourceProperty* lightPosition) {
    // do nothing if lightposition is unchanged
    if (lightPosition_.get() == lightPosition->get())
        return;

    // initialize both lightpositions with given value
    lightPosition_.set(lightPosition->get());
    lightPositionBlend_ = lightPosition->get().xyz();

    // return if no volume is present
    if (!currentVolume_ || !currentVolume_->getRepresentation<VolumeRAM>())
        return;

    // return if viewpoint is camera
    if (viewpointModes_.getValue() == CAMERA_POSITION)
        return;

    // recalculate cubeFaces because light changed
    verifyCubeFaces();

    // verify light position
    verifyLightPositions();

    recomputeNeeded();
}

void LightVolumeGenerator::scalingChanged() {
    updateTextureDimensions();

    recomputeNeeded_ = true;
}

void LightVolumeGenerator::cameraChanged() {
    // return if viewpoint is lightsource
    //if (viewpointModes_->getValue() == LIGHTSOURCE_POSITION)
      //  return;
    if (!currentVolume_)
        return;

    verifyCubeFaces();

    recomputeNeeded_ = true;
}

void LightVolumeGenerator::viewpointModeChanged() {
    if (!verifying_)
        verifyCubeFaces();

    recomputeNeeded_ = true;
}

void LightVolumeGenerator::recomputeNeeded() {
    recomputeNeeded_ = true;
}

void LightVolumeGenerator::generateLightVolume(TextureUnit* lightUnit, TextureUnit* blendUnit) {
    if (recomputeNeeded_ || recomputeOnEveryFrame_.get()) {
        createTexture(lightUnit, blendUnit);

        recomputeNeeded_ = false;
    }
}

void LightVolumeGenerator::verifyCubeFaces() {
    bool update = false;
    int newCubeFace = calculateCubeFace(false);
    if (newCubeFace != cubeFace_) {
        cubeFace_ = newCubeFace;
        update = true;
    }

    int newCubeFaceBlend = calculateCubeFace(true);
    if (newCubeFaceBlend != cubeFaceBlend_) {
        cubeFaceBlend_ = newCubeFaceBlend;
        update = true;
    }

    int newCubeFaceView;
    if (viewpointModes_.getValue() != CAMERA_POSITION) {
        verifying_ = true;
        viewpointModes_.selectByValue(CAMERA_POSITION);
        newCubeFaceView = calculateCubeFace();
        viewpointModes_.selectByValue(LIGHTSOURCE_POSITION);
        verifying_ = false;
    }
    else
        newCubeFaceView = newCubeFace;
    if (newCubeFaceView != cubeFaceView_) {
        cubeFaceView_ = newCubeFaceView;
        update = true;
    }

    if (update)
        updateTextureDimensions();
}

int LightVolumeGenerator::calculateCubeFace(bool calculateSecondBest) {
    std::vector<tgt::vec3> cubeVertices = currentVolume_->getCubeVertices();
    int index[24] = { 0, 3, 4, 7,  // left
                      1, 2, 5, 6,  // right
                      0, 1, 4, 5,  // bottom
                      2, 3, 6, 7,  // top
                      0, 1, 2, 3,  // back
                      4, 5, 6, 7}; // front

    tgt::vec3 viewPos;
    if (viewpointModes_.getValue() == LIGHTSOURCE_POSITION)
        viewPos = lightPosition_.get().xyz();
    else
        viewPos = camera_.get().getPosition();

    float minRadians = tgt::PIf;
    int ind = -1;
    for (int i = 0; i < 6; ++i) {
        // calculate middle of the cubeface
        tgt::vec3 middle = cubeVertices[index[4*i+0]] + cubeVertices[index[4*i+1]] +
                           cubeVertices[index[4*i+2]] + cubeVertices[index[4*i+3]];
        middle /= 4.f;
        middle.z *= -1.f;

        // vector to correct viewpoint
        tgt::vec3 lightVec = tgt::normalize(viewPos - middle);

        float skalar = tgt::dot(normals_[i], lightVec);
        skalar = acos(skalar);
        if (skalar < minRadians) {
            if (!calculateSecondBest) {
                minRadians = skalar;
                ind = i;
                angles_.x = skalar / tgt::PIf; //normalized to [0,1]
            }
            else if (i != cubeFace_) {
                minRadians = skalar;
                ind = i;
                angles_.y = skalar / tgt::PIf; // normalized to [0,1]
            }
        }
    }

    return ind;
}

void LightVolumeGenerator::verifyLightPositions() {
    if (!currentVolume_ || !currentVolume_->getRepresentation<VolumeRAM>())
        return;

    // return if viewpoint is camera
    if (viewpointModes_.getValue() == CAMERA_POSITION)
        return;

    // avoid light position inside the volume
    int index = cubeFace_ / 2;
    tgt::vec4 pos = lightPosition_.get();
    tgt::vec3* cubeVertices = new tgt::vec3[8];
    for (int i=0; i<8; i++) {
        cubeVertices[i] = tgt::vec3(currentVolume_->getCubeVertices()[i]);
    }
    for (int i=0; i<4; i++)
        std::swap(cubeVertices[i].z, cubeVertices[i+4].z);
    if ((pos[index] > cubeVertices[0][index]) && (pos[index] < -cubeVertices[0][index])) {
        if (cubeFace_ % 2 == 1)
            pos[index] = -cubeVertices[0][index];
        else
            pos[index] = cubeVertices[0][index];

        lightPosition_.set(pos);
    }

    index = cubeFaceBlend_ / 2;
    if ((lightPositionBlend_[index] > cubeVertices[0][index]) &&
        (lightPositionBlend_[index] < -cubeVertices[0][index]))
    {
        if (cubeFaceBlend_ % 2 == 1)
            lightPositionBlend_[index] = -cubeVertices[0][index];
        else
            lightPositionBlend_[index] = cubeVertices[0][index];

    }

    lightInitialized_ = true;
    delete[] cubeVertices;
}

void LightVolumeGenerator::setupTextures() {
    fbo_->activate();
    if (shadowTexture_) {
        fbo_->detachTexture(GL_COLOR_ATTACHMENT0_EXT);
        delete shadowTexture_;
    }
    tgt::ivec3 dims = currentVolume_->getDimensions();
    shadowTexture_ = new tgt::Texture(0, dims, GL_RGBA, GL_RGBA16, GL_FLOAT, tgt::Texture::NEAREST);
    shadowTexture_->setWrapping(tgt::Texture::CLAMP_TO_BORDER);
    shadowTexture_->uploadTexture();
    fbo_->deactivate();

    fboBlend_->activate();
    if (shadowTextureBlend_) {
        fboBlend_->detachTexture(GL_COLOR_ATTACHMENT0_EXT);
        delete shadowTextureBlend_;
    }
    shadowTextureBlend_ = new tgt::Texture(0, dims, GL_RGBA, GL_RGBA16, GL_FLOAT, tgt::Texture::NEAREST);
    shadowTextureBlend_->setWrapping(tgt::Texture::CLAMP_TO_BORDER);
    shadowTextureBlend_->uploadTexture();
    fboBlend_->deactivate();

    fboViewpoint_->activate();
    if (viewpointTexture_) {
        fboViewpoint_->detachTexture(GL_COLOR_ATTACHMENT0_EXT);
        delete viewpointTexture_;
    }
    viewpointTexture_ = new tgt::Texture(0, dims, GL_RGBA, GL_RGBA16, GL_FLOAT, tgt::Texture::NEAREST);
    viewpointTexture_->setWrapping(tgt::Texture::CLAMP_TO_BORDER);
    viewpointTexture_->uploadTexture();
    fboViewpoint_->deactivate();
}

void LightVolumeGenerator::createTexture(TextureUnit* lightUnit, TextureUnit* blendUnit) {
    if (!lightInitialized_)
        verifyLightPositions();

    //----------------- first pass -----------------//
    fboBlend_->activate();
    glPushAttrib(GL_VIEWPORT_BIT);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glViewport(0, 0, shadowTextureBlend_->getWidth(), shadowTextureBlend_->getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.loadIdentity();
    glOrtho(0.f, 1.f, 0.f, 1.f, -2.f, 1.f);

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.loadIdentity();

    createBlendTexture(blendUnit);

    LGL_ERROR;
    glPopAttrib();
    fboBlend_->deactivate();


    //----------------- second pass -----------------//
    /*
    fboViewpoint_->activate();
    glPushAttrib(GL_VIEWPORT_BIT);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glViewport(0, 0, viewpointTexture_->getWidth(), viewpointTexture_->getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // create viewpoint texture
    createViewpointTexture();

    LGL_ERROR;
    glPopAttrib();
    fboViewpoint_->deactivate();
    */

    //----------------- third pass -----------------//
    fbo_->activate();
    glPushAttrib(GL_VIEWPORT_BIT);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glViewport(0, 0, shadowTexture_->getWidth(), shadowTexture_->getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // create light texture
    createShadowTexture(lightUnit, blendUnit);

    LGL_ERROR;

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    LGL_ERROR;
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.popMatrix();

    TextureUnit::setZeroUnit();

    glPopAttrib();
    fbo_->deactivate();

    LGL_ERROR;
}

void LightVolumeGenerator::createBlendTexture(TextureUnit* blendUnit) {
    tgt::vec2 coords2D[4];
    if (((signFactorBlend_ > 0.f) && (indexingBlend_.z != 2)) ||
        ((signFactorBlend_ < 0.f) && (indexingBlend_.z == 2)))
    {
        coords2D[0] = tgt::vec2(-1.f, -1.f);
        coords2D[1] = tgt::vec2( 1.f, -1.f);
        coords2D[2] = tgt::vec2( 1.f,  1.f);
        coords2D[3] = tgt::vec2(-1.f,  1.f);
    }
    else {
        coords2D[0] = tgt::vec2( 1.f, -1.f);
        coords2D[1] = tgt::vec2(-1.f, -1.f);
        coords2D[2] = tgt::vec2(-1.f,  1.f);
        coords2D[3] = tgt::vec2( 1.f,  1.f);
    }

    TextureUnit transferUnit;
    transferUnit.activate();
    transferFunc_->get()->bind();

    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

    // add main volume
    TextureUnit volUnit;
    volumeTextures.push_back(VolumeStruct(
        currentVolume_,
        &volUnit,
        "volume_","volumeStruct_")
    );

    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
    raycastPrg_->activate();

    tgt::Camera cam = camera_.get();
    bindVolumes(raycastPrg_, volumeTextures, &cam, lightPosition_.get());

    if (viewpointModes_.getValue() == LIGHTSOURCE_POSITION)
        raycastPrg_->setUniform("viewpoint_", lightPositionBlend_);
    else
        raycastPrg_->setUniform("viewpoint_", camera_.get().getPosition());
    transferFunc_->get()->setUniform(raycastPrg_, "transferFunc_", "transferFuncTex_", transferUnit.getUnitNumber());
    raycastPrg_->setUniform("increment_", -signFactorBlend_ / (shadowTextureBlend_->getDepth() - 1.f));
    raycastPrg_->setUniform("permutationIndex_", indexingBlend_.z);
    raycastPrg_->setUniform("textureDimension_", shadowTextureBlend_->getDimensions());
    raycastPrg_->setUniform("firstPass_", true);
    raycastPrg_->setUniform("scalingFactor_", static_cast<int>(1.f / scalingMode_.getValue()));

    for (int i = 0; i < shadowTextureBlend_->getDepth(); ++i) {
        raycastPrg_->setUniform("sliceNumber_", i);

        blendUnit->activate();
        glEnable(GL_TEXTURE_3D);
        shadowTextureBlend_->bind();
        shadowTextureBlend_->setFilter(tgt::Texture::NEAREST);
        shadowTextureBlend_->setWrapping(tgt::Texture::CLAMP_TO_BORDER);

        glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor_.elem);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        raycastPrg_->setUniform("shadowTexture_", blendUnit->getUnitNumber());

        // calculate texture coordinates for lookup in volume dataset
        tgt::vec3 texCoord[4];
        for (int j = 0; j < 4; ++j) {
            // set texture coordinates in [-1, 1] in each direction even if the dataset is not uniform
            texCoord[j][indexingBlend_.x] = coords2D[j].x;
            texCoord[j][indexingBlend_.y] = coords2D[j].y;
            // normalize value to the interval [-1, 1]
            texCoord[j][indexingBlend_.z] = static_cast<float>(i) / (shadowTextureBlend_->getDepth() - 1.f);
            texCoord[j][indexingBlend_.z] = signFactorBlend_ * (texCoord[j][indexingBlend_.z] * 2.f - 1.f);

            // transform texture coordinates to the interval [0, 1] in each direction
            texCoord[j] = tgt::vec3(0.5f) * texCoord[j] + 0.5f;
        }

        fboBlend_->attachTexture(shadowTextureBlend_, GL_COLOR_ATTACHMENT0_EXT, 0, i);
        fboBlend_->isComplete();

        glBegin(GL_QUADS);
            glTexCoord3fv(texCoord[0].elem); glVertex2f(0.f, 0.f);
            glTexCoord3fv(texCoord[1].elem); glVertex2f(1.f, 0.f);
            glTexCoord3fv(texCoord[2].elem); glVertex2f(1.f, 1.f);
            glTexCoord3fv(texCoord[3].elem); glVertex2f(0.f, 1.f);
        glEnd();
    }
}

void LightVolumeGenerator::createShadowTexture(TextureUnit* lightUnit, TextureUnit* blendUnit) {
    tgt::vec2 coords2D[4];
    if (((signFactor_ > 0.f) && (indexing_.z != 2)) ||
        ((signFactor_ < 0.f) && (indexing_.z == 2)))
    {
        coords2D[0] = tgt::vec2(-1.f, -1.f);
        coords2D[1] = tgt::vec2( 1.f, -1.f);
        coords2D[2] = tgt::vec2( 1.f,  1.f);
        coords2D[3] = tgt::vec2(-1.f,  1.f);
    }
    else {
        coords2D[0] = tgt::vec2( 1.f, -1.f);
        coords2D[1] = tgt::vec2(-1.f, -1.f);
        coords2D[2] = tgt::vec2(-1.f,  1.f);
        coords2D[3] = tgt::vec2( 1.f,  1.f);
    }

    raycastPrg_->setUniform("increment_", -signFactor_ / (shadowTexture_->getDepth() - 1.f));
    raycastPrg_->setUniform("permutationIndex_", indexing_.z);
    raycastPrg_->setUniform("textureDimension_", shadowTexture_->getDimensions());
    raycastPrg_->setUniform("firstPass_", false);
    raycastPrg_->setUniform("angles_", angles_);
    LGL_ERROR;
    // bind blend texture
    blendUnit->activate();
    glEnable(GL_TEXTURE_3D);
    shadowTextureBlend_->bind();
    shadowTextureBlend_->setFilter(tgt::Texture::LINEAR);
    raycastPrg_->setUniform("blendTexture_", blendUnit->getUnitNumber());
    raycastPrg_->setUniform("permutationIndexBlend_", indexingBlend_.z);
    raycastPrg_->setUniform("signBlend_", -signFactorBlend_);
    LGL_ERROR;

    // bind viewpoint texture
    TextureUnit viewUnit;
    viewUnit.activate();
    glEnable(GL_TEXTURE_3D);
    viewpointTexture_->bind();
    viewpointTexture_->setFilter(tgt::Texture::LINEAR);
    raycastPrg_->setUniform("viewpointTexture_", viewUnit.getUnitNumber());
    raycastPrg_->setUniform("permutationIndexView_", indexingViewpoint_.z);
    raycastPrg_->setUniform("signView_", -signFactorViewpoint_);
    LGL_ERROR;

    if (viewpointModes_.getValue() == LIGHTSOURCE_POSITION)
        raycastPrg_->setUniform("viewpoint_", lightPosition_.get().xyz());
    else
        raycastPrg_->setUniform("viewpoint_", camera_.get().getPosition());

    for (int i = 0; i < shadowTexture_->getDepth(); ++i) {
        raycastPrg_->setUniform("sliceNumber_", i);

        lightUnit->activate();
        glEnable(GL_TEXTURE_3D);
        shadowTexture_->bind();
        shadowTexture_->setFilter(tgt::Texture::NEAREST);
        shadowTexture_->setWrapping(tgt::Texture::CLAMP_TO_BORDER);

        glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor_.elem);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        raycastPrg_->setUniform("shadowTexture_", lightUnit->getUnitNumber());

        // calculate texture coordinates for lookup in volume dataset
        tgt::vec3 texCoord[4];
        for (int j = 0; j < 4; ++j) {
            // set texture coordinates in [-1, 1] in each direction even if the dataset is not uniform
            texCoord[j][indexing_.x] = coords2D[j].x;
            texCoord[j][indexing_.y] = coords2D[j].y;
            // normalize value to the interval [-1, 1]
            texCoord[j][indexing_.z] = static_cast<float>(i) / (shadowTexture_->getDepth() - 1.f);
            texCoord[j][indexing_.z] = signFactor_ * (texCoord[j][indexing_.z] * 2.f - 1.f);

            // transform texture coordinates to the interval [0, 1] in each direction
            texCoord[j] = tgt::vec3(0.5f) * texCoord[j] + 0.5f;
        }

        fbo_->attachTexture(shadowTexture_, GL_COLOR_ATTACHMENT0_EXT, 0, i);
        fbo_->isComplete();

        glBegin(GL_QUADS);
            glTexCoord3fv(texCoord[0].elem); glVertex2f(0.f, 0.f);
            glTexCoord3fv(texCoord[1].elem); glVertex2f(1.f, 0.f);
            glTexCoord3fv(texCoord[2].elem); glVertex2f(1.f, 1.f);
            glTexCoord3fv(texCoord[3].elem); glVertex2f(0.f, 1.f);
        glEnd();

        if (writeSlices_.get())
            // write current slice to an image file for debug purposes
            writeSliceToImage(i);
    }

    LGL_ERROR;

    raycastPrg_->deactivate();

    LGL_ERROR;
}

void LightVolumeGenerator::createViewpointTexture() {
    tgt::vec2 coords2D[4];
    if (((signFactorViewpoint_ > 0.f) && (indexingViewpoint_.z != 2)) ||
        ((signFactorViewpoint_ < 0.f) && (indexingViewpoint_.z == 2)))
    {
        coords2D[0] = tgt::vec2(-1.f, -1.f);
        coords2D[1] = tgt::vec2( 1.f, -1.f);
        coords2D[2] = tgt::vec2( 1.f,  1.f);
        coords2D[3] = tgt::vec2(-1.f,  1.f);
    }
    else {
        coords2D[0] = tgt::vec2( 1.f, -1.f);
        coords2D[1] = tgt::vec2(-1.f, -1.f);
        coords2D[2] = tgt::vec2(-1.f,  1.f);
        coords2D[3] = tgt::vec2( 1.f,  1.f);
    }
    LGL_ERROR;
    raycastPrg_->setUniform("increment_", -signFactorViewpoint_ / (viewpointTexture_->getDepth() - 1.f));
    raycastPrg_->setUniform("permutationIndex_", indexingViewpoint_.z);
    raycastPrg_->setUniform("textureDimension_", viewpointTexture_->getDimensions());
    raycastPrg_->setUniform("firstPass_", true);

    LGL_ERROR;

    raycastPrg_->setUniform("viewpoint_", camera_.get().getPosition());
    TextureUnit viewUnit;

    for (int i = 0; i < viewpointTexture_->getDepth(); ++i) {
        raycastPrg_->setUniform("sliceNumber_", i);

        viewUnit.activate();
        glEnable(GL_TEXTURE_3D);
        viewpointTexture_->bind();
        viewpointTexture_->setFilter(tgt::Texture::NEAREST);
        viewpointTexture_->setWrapping(tgt::Texture::CLAMP_TO_BORDER);

        glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor_.elem);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        raycastPrg_->setUniform("shadowTexture_", viewUnit.getUnitNumber());

        // calculate texture coordinates for lookup in volume dataset
        tgt::vec3 texCoord[4];
        for (int j = 0; j < 4; ++j) {
            // set texture coordinates in [-1, 1] in each direction even if the dataset is not uniform
            texCoord[j][indexingViewpoint_.x] = coords2D[j].x;
            texCoord[j][indexingViewpoint_.y] = coords2D[j].y;
            // normalize value to the interval [-1, 1]
            texCoord[j][indexingViewpoint_.z] = static_cast<float>(i) / (viewpointTexture_->getDepth() - 1.f);
            texCoord[j][indexingViewpoint_.z] = signFactorViewpoint_ * (texCoord[j][indexingViewpoint_.z] * 2.f - 1.f);

            // transform texture coordinates to the interval [0, 1] in each direction
            texCoord[j] = tgt::vec3(0.5f) * texCoord[j] + 0.5f;
        }

        fboViewpoint_->attachTexture(viewpointTexture_, GL_COLOR_ATTACHMENT0_EXT, 0, i);
        fboViewpoint_->isComplete();

        glBegin(GL_QUADS);
            glTexCoord3fv(texCoord[0].elem); glVertex2f(0.f, 0.f);
            glTexCoord3fv(texCoord[1].elem); glVertex2f(1.f, 0.f);
            glTexCoord3fv(texCoord[2].elem); glVertex2f(1.f, 1.f);
            glTexCoord3fv(texCoord[3].elem); glVertex2f(0.f, 1.f);
        glEnd();
    }

    LGL_ERROR;
}

void LightVolumeGenerator::updateTextureDimensions() {
    if (normals_[cubeFace_].x != 0)
        indexing_ = tgt::ivec3(2, 1, 0);
    else if (normals_[cubeFace_].y != 0)
        indexing_ = tgt::ivec3(0, 2, 1);
    else
        indexing_ = tgt::ivec3(0, 1, 2);

    if (normals_[cubeFace_][indexing_.z] > 0)
        signFactor_ = -1.f;
    else
        signFactor_ =  1.f;

    // set new texture dimensions, so that we are looping over z coordinate
    tgt::ivec3 datasetDimension = currentVolume_->getDimensions();
    datasetDimension.x = static_cast<int>(scalingMode_.getValue() * static_cast<float>(datasetDimension.x));
    datasetDimension.y = static_cast<int>(scalingMode_.getValue() * static_cast<float>(datasetDimension.y));
    datasetDimension.z = static_cast<int>(scalingMode_.getValue() * static_cast<float>(datasetDimension.z));
    shadowTexture_->setDimensions(tgt::ivec3(datasetDimension[indexing_.x],
                                             datasetDimension[indexing_.y],
                                             datasetDimension[indexing_.z]));
    fbo_->activate();
    shadowTexture_->uploadTexture();
    fbo_->attachTexture(shadowTexture_, GL_COLOR_ATTACHMENT0_EXT);
    fbo_->isComplete();

    // blendtexture
    if (normals_[cubeFaceBlend_].x != 0)
        indexingBlend_ = tgt::ivec3(2, 1, 0);
    else if (normals_[cubeFaceBlend_].y != 0)
        indexingBlend_ = tgt::ivec3(0, 2, 1);
    else
        indexingBlend_ = tgt::ivec3(0, 1, 2);

    if (normals_[cubeFaceBlend_][indexingBlend_.z] > 0)
        signFactorBlend_ = -1.f;
    else
        signFactorBlend_ =  1.f;

    shadowTextureBlend_->setDimensions(tgt::ivec3(datasetDimension[indexingBlend_.x],
                                                  datasetDimension[indexingBlend_.y],
                                                  datasetDimension[indexingBlend_.z]));
    fboBlend_->activate();
    shadowTextureBlend_->uploadTexture();
    fboBlend_->attachTexture(shadowTextureBlend_, GL_COLOR_ATTACHMENT0_EXT);
    fboBlend_->isComplete();

    // viewpoint texture
    if (normals_[cubeFaceView_].x != 0)
        indexingViewpoint_ = tgt::ivec3(2, 1, 0);
    else if (normals_[cubeFaceView_].y != 0)
        indexingViewpoint_ = tgt::ivec3(0, 2, 1);
    else
        indexingViewpoint_ = tgt::ivec3(0, 1, 2);

    if (normals_[cubeFaceView_][indexingViewpoint_.z] > 0)
        signFactorViewpoint_ =  1.f;
    else
        signFactorViewpoint_ = -1.f;
    viewpointTexture_->setDimensions(tgt::ivec3(datasetDimension[indexingViewpoint_.x],
                                                datasetDimension[indexingViewpoint_.y],
                                                datasetDimension[indexingViewpoint_.z]));
    fboViewpoint_->activate();
    viewpointTexture_->uploadTexture();
    fboViewpoint_->attachTexture(viewpointTexture_, GL_COLOR_ATTACHMENT0_EXT);
    fboViewpoint_->isComplete();
}

#ifdef VRN_MODULE_DEVIL
void LightVolumeGenerator::writeSliceToImage(int slice) {
    glFlush();
    uint8_t* buffer = new uint8_t[shadowTexture_->getWidth() * shadowTexture_->getHeight() * 4];
    glReadPixels(0, 0, shadowTexture_->getWidth(), shadowTexture_->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    ILuint result;
    ilGenImages(1, &result);
    ilBindImage(result);
    ilTexImage(shadowTexture_->getWidth(), shadowTexture_->getHeight(), 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, buffer);
    ilEnable(IL_FILE_OVERWRITE);
    std::stringstream s;
    s << slice;
    std::string filename = "shadowTextureDebug" + s.str() + ".png";
    ilSaveImage(const_cast<char *>(filename.c_str()));
    delete[] buffer;
}
#else
void LightVolumeGenerator::writeSliceToImage(int /*slice*/) {
}

#endif

} // namespace voreen
