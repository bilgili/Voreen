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

#include "voreen/core/vis/processors/render/volumeeditor.h"

#include "voreen/core/volume/modality.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/properties/cameraproperty.h"

#include <fboClass/fboclass_framebufferobject.h>

#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/io/datvolumewriter.h"

using tgt::vec3;

namespace voreen {

VolumeEditor::VolumeEditor()
    : VolumeRaycaster()
    , camera_("camera", "Camera", new tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , brushSize_("brushSize", "Brush size", 10, 1, 50)
    , brushColor_("brushColor", "Brush color", tgt::col4(255,0,0,255))
    , saveDialogProp_("saveSegDataSet", "Save segmentation Data", "")
    , transferFunc_("transferFunction", "Transfer function")
    , fbo_(0)
    , volumePort_(Port::INPORT, "volumehandle.volumehandle")
    , entryPort_(Port::INPORT, "image.entrypoints")
    , exitPort_(Port::INPORT, "image.exitpoints")
    , firstHitpointsPort_(Port::OUTPORT, "image.firsthit", true)
    , outport_(Port::OUTPORT, "image.output", true)
{

    addProperty(transferFunc_);

    addProperty(maskingMode_);
    addProperty(gradientMode_);
    addProperty(classificationMode_);
    addProperty(shadeMode_);
    addProperty(compositingMode_);
    addProperty(brushSize_);
    addProperty(brushColor_);
    addProperty(saveDialogProp_);

    addPort(volumePort_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPrivateRenderPort(&firstHitpointsPort_);
    addPort(outport_);

    mouseDown_ = false;
}

VolumeEditor::~VolumeEditor() {
}

Processor* VolumeEditor::create() const {
    VolumeEditor* sr = new VolumeEditor();
    return sr;
}

bool VolumeEditor::saveSegmentationDataSet(std::string filename) {
    DatVolumeWriter data;
    VolumeSerializerPopulator* populator = new VolumeSerializerPopulator();
    const VolumeSerializer* serializer = populator->getVolumeSerializer();
    try {
        serializer->save(filename, currentVolumeHandle_);
    } catch (tgt::UnsupportedFormatException) {
        return false;
    }

    return true;
}

const std::string VolumeEditor::getProcessorInfo() const {
    return "Allows to perform simple segmentations.";
}

void VolumeEditor::initialize() throw (VoreenException) {

    VolumeRaycaster::initialize();

    // generate fbo for rendering onto the textures
    delete fbo_;
    fbo_ = new FramebufferObject();

    loadShader();

    if(!raycastPrg_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }
    initialized_ = true;
    setLightingParameters();
}

void VolumeEditor::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "rc_volumeeditor.frag",
                                       generateHeader(), false, false);
}

void VolumeEditor::compile(VolumeHandle* volumeHandle) {
    raycastPrg_->setHeaders(generateHeader(volumeHandle), false);
    raycastPrg_->rebuild();
}

void VolumeEditor::process() {

    if (volumePort_.isReady())
        currentVolumeHandle_ = volumePort_.getData();
    else
        currentVolumeHandle_ = 0;

    // pass volumehandle to transfer function
    transferFunc_.setVolumeHandle(currentVolumeHandle_);

    if (!currentVolumeHandle_)
        return;

    //TODO: tc_
    //std::vector<int> activeTargets;
    //activeTargets.push_back(outport_.getTextureID());
    //activeTargets.push_back(firstHitpointsPort_.getTextureID());
    //outport_.getTextureContainer()->setActiveTargets(activeTargets);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // compile program
    if(getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile(volumePort_.getData());
    LGL_ERROR;

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
        currentVolumeHandle_->getVolumeGL(),
        volTexUnit_,
        "volume_",
        "volumeParameters_")
    );

    addBrickedVolumeModalities(volumePort_.getData(), volumeTextures);

    // segmentation volume
    // TODO: fetch segmentation from inport
    //VolumeGL* volumeSeg = currentVolumeHandle_->getRelatedVolumeGL(Modality::MODALITY_SEGMENTATION);
    VolumeGL* volumeSeg = 0;

    if (useSegmentation_.get() && (volumeSeg != 0) ) {
        volumeTextures.push_back(VolumeStruct(
            volumeSeg,
            segmentationTexUnit_,
            "segmentation_",
            "segmentationParameters_")
        );

        glActiveTexture(tm_.getGLTexUnit(segmentationTexUnit_));

        // set texture filters for this texture
        //FIXME: this does NOTHING! is this the right place to set filtering for segmentation?
        glPushAttrib(GL_TEXTURE_BIT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glPopAttrib();
    }

    // bind transfer function
    glActiveTexture(tm_.getGLTexUnit(transferTexUnit_));
    if (transferFunc_.get())
        transferFunc_.get()->bind();

    // initialize shader
    raycastPrg_->activate();

    // set common uniforms used by all shaders
    setGlobalShaderParameters(raycastPrg_, camera_.get());
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(raycastPrg_, volumeTextures);

    // pass the remaining uniforms to the shader
    raycastPrg_->setUniform("entryPoints_", tm_.getTexUnit(entryParamsTexUnit_));
    raycastPrg_->setUniform("entryPointsDepth_", tm_.getTexUnit(entryParamsDepthTexUnit_));
    raycastPrg_->setUniform("exitPoints_", tm_.getTexUnit(exitParamsTexUnit_));
    raycastPrg_->setUniform("exitPointsDepth_", tm_.getTexUnit(exitParamsDepthTexUnit_));
    raycastPrg_->setUniform("transferFunc_", tm_.getTexUnit(transferTexUnit_));
    if (useSegmentation_.get() && volumeSeg)
        raycastPrg_->setUniform("segment_" , static_cast<GLfloat>(segment_.get()));

    setBrickedVolumeUniforms(volumePort_.getData());
    renderQuad();

    raycastPrg_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

std::string VolumeEditor::generateHeader(VolumeHandle* volumeHandle) {
    std::string headerSource = VolumeRaycaster::generateHeader(volumeHandle);

    headerSource += transferFunc_.get()->getShaderDefines();

    // TODO: fetch segmentation from inport
    /*if (useSegmentation_.get() && currentVolumeHandle_->getRelatedVolumeGL(Modality::MODALITY_SEGMENTATION))
        headerSource += "#define USE_SEGMENTATION\n"; */

    return headerSource;
}

void VolumeEditor::applyBrush(tgt::ivec2 mousePos) {

    if (!firstHitpointsPort_.isReady()) {
        LWARNING("Firsthit port not ready");
        return;
    }

    tgt::vec4 volCoord= firstHitpointsPort_.getData()->getColorAtPos(mousePos);

    int centerSlice = static_cast<int>(volCoord.z*currentVolumeHandle_->getVolume()->getDimensions().z);

    for (int curSlice=static_cast<int>(centerSlice-(brushSize_.get()/2.0));curSlice<static_cast<int>(centerSlice+(brushSize_.get()/2.0));curSlice++) {

    // attach the determined layer to an FBO and draw into it
    fbo_->Bind();
    fbo_->AttachTexture(GL_TEXTURE_3D, currentVolumeHandle_->getVolumeGL()->getTexture()->getId(), GL_COLOR_ATTACHMENT0_EXT, 0, curSlice);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    //fbo_->IsValid();
    //LGL_ERROR;

    glPushAttrib(GL_TRANSFORM_BIT);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, currentVolumeHandle_->getVolume()->getDimensions().x, currentVolumeHandle_->getVolume()->getDimensions().y);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, currentVolumeHandle_->getVolume()->getDimensions().x, 0, currentVolumeHandle_->getVolume()->getDimensions().y);
    glPopAttrib();

    /*
    paintPrg_->activate();

    // bind texture stacks
    glActiveTexture(tm_.getGLTexUnit("texturestack"));
    if (paintMode_->get() == 0) // coloring
        textureStack_.arrayTexture_[curLayer]->bind();
    else if (paintMode_->get() == 1) // specularing
        textureStack_.arrayTexture_[curLayer]->bind();
    else if (paintMode_->get() == 2) // sculpting
        textureStack_.arrayTextureNH_[curLayer]->bind();
    else if (paintMode_->get() == 3) // ghosting
        textureStack_.arrayTextureA_[curLayer]->bind();

    GLint loc = paintPrg_->getUniformLocation("brushColor_", true);
    if (loc != -1)
        paintPrg_->setUniform("brushColor_", brushColor_.get());
    loc = paintPrg_->getUniformLocation("brushSize_", true);
    if (loc != -1)
        paintPrg_->setUniform("brushSize_", brushSize_.get());
    loc = paintPrg_->getUniformLocation("curPos_", true);
    if (loc != -1)
        paintPrg_->setUniform("curPos_", cursorPos.xy());
    loc = paintPrg_->getUniformLocation("curPosAbs_", true);
    if (loc != -1)
        paintPrg_->setUniform("curPosAbs_", cursorPos.xy()*tgt::vec2(textureStack_.arrayTexture_[0]->getWidth(), textureStack_.arrayTexture_[0]->getHeight()));
    loc = paintPrg_->getUniformLocation("curLayer_", true);
    if (loc != -1)
        paintPrg_->setUniform("curLayer_", (GLint) tm_.getTexUnit("texturestack"));
    loc = paintPrg_->getUniformLocation("layerSize_", true);
    if (loc != -1)
        paintPrg_->setUniform("layerSize_", tgt::ivec2(textureStack_.arrayTexture_[0]->getWidth(), textureStack_.arrayTexture_[0]->getHeight()));

    glPointSize(brushSize_.get());
    */

    glPointSize(static_cast<GLfloat>(brushSize_.get()));
    glColor4f(brushColor_.get().r, brushColor_.get().g, brushColor_.get().b, brushColor_.get().a);
    glBegin(GL_POINTS);
    glVertex2f(volCoord.x*currentVolumeHandle_->getVolume()->getDimensions().x,
               volCoord.y*currentVolumeHandle_->getVolume()->getDimensions().y);
    glEnd();

    glPushAttrib(GL_TRANSFORM_BIT);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();

    /*
    paintPrg_->deactivate();

    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, textureStack_.arrayTexture_[0]->getWidth(), textureStack_.arrayTexture_[0]->getHeight(), 0);
    */

    fbo_->UnattachAll();
    fbo_->Disable();
    }
    invalidate();

}

void VolumeEditor::mousePressEvent(tgt::MouseEvent* e) {
    //if (!tc_) return;
    if (e->modifiers() & tgt::MouseEvent::LSHIFT) {
        mouseDown_ = true;
        //tgt::ivec2 mousePos = tgt::ivec2(e->coord().x, getSize().y-e->coord().y);
        //applyBrush(mousePos);
        e->accept();
    } else
        e->ignore();
}

void VolumeEditor::mouseMoveEvent(tgt::MouseEvent* e) {
    //if (!tc_) return;
    if (mouseDown_ && (e->modifiers() & tgt::MouseEvent::LSHIFT)) {
        //tgt::ivec2 mousePos = tgt::ivec2(e->coord().x, getSize().y-e->coord().y);
        //applyBrush(mousePos);
        e->accept();
    } else
        e->ignore();
}

void VolumeEditor::mouseReleaseEvent(tgt::MouseEvent* e) {
    mouseDown_ = false;
    e->ignore();
}

void VolumeEditor::mouseDoubleClickEvent(tgt::MouseEvent* e) {
    //TODO: insert selected texture into stack
    e->ignore();
}

} // namespace voreen
