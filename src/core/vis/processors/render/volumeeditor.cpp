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

#include "voreen/core/vis/lightmaterial.h"
#include "voreen/core/volume/modality.h"
#include "voreen/core/vis/voreenpainter.h"

#include <fboClass/framebufferObject.h>

#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/io/datvolumewriter.h"
#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

VolumeEditor::VolumeEditor()
    : VolumeRaycaster()
    , brushSize_("set.brushSize", "Brush size", 10, 1, 50)
    , brushColor_("set.brushColor", "Brush color", tgt::col4(255,0,0,255))
    , saveDialogProp_("save.SegDataSet", "Save Segmentation Data", "")
    , transferFunc_(setTransFunc_, "Transfer Function")
    , fbo_(0)
{
    setName("VolumeEditor");

    addProperty(&transferFunc_);

    addProperty(maskingMode_);
    addProperty(gradientMode_);
    addProperty(classificationMode_);
    addProperty(shadeMode_);
    addProperty(compositingMode_);
    addProperty(&brushSize_);
    addProperty(&brushColor_);
    addProperty(&saveDialogProp_);

    createInport("volumehandle.volumehandle");
    createInport("image.entrypoints");
    createInport("image.exitpoints");
    createPrivatePort("image.firsthit");
    createOutport("image.output");

    mouseDown_ = false;
}

VolumeEditor::~VolumeEditor() {
}

Processor* VolumeEditor::create() const {
    VolumeEditor* sr = new VolumeEditor();
    MsgDistr.postMessage(new TemplateMessage<tgt::EventListener*>(VoreenPainter::addEventListener_, (tgt::EventListener*)sr));
    return sr;
}

bool VolumeEditor::saveSegmentationDataSet(std::string filename) {

    DatVolumeWriter data;
    VolumeSerializerPopulator* populator = new VolumeSerializerPopulator();
    VolumeSerializer* serializer = populator->getVolumeSerializer();
    try {
        serializer->save(filename, currentVolumeHandle_->getVolume());
    } catch (tgt::UnsupportedFormatException) {
        return false;
    }

    return true;
}

const std::string VolumeEditor::getProcessorInfo() const {
    return "Allows to perform simple segmentations.";
}

void VolumeEditor::processMessage(Message* msg, const Identifier& dest) {
    VolumeRaycaster::processMessage(msg, dest);
    // send invalidate and update context, if lighting parameters have changed
    if (msg->id_ == LightMaterial::setLightPosition_   ||
        msg->id_ == LightMaterial::setLightAmbient_         ||
        msg->id_ == LightMaterial::setLightDiffuse_         ||
        msg->id_ == LightMaterial::setLightSpecular_        ||
        msg->id_ == LightMaterial::setLightAttenuation_     ||
        msg->id_ == LightMaterial::setMaterialAmbient_      ||
        msg->id_ == LightMaterial::setMaterialDiffuse_      ||
        msg->id_ == LightMaterial::setMaterialSpecular_     ||
        msg->id_ == LightMaterial::setMaterialShininess_        ) {
            setLightingParameters();
            invalidate();
    }
    else if (msg->id_ == "save.SegDataSet")
        saveSegmentationDataSet(msg->getValue<std::string>());
}

int VolumeEditor::initializeGL() {
    // generate fbo for rendering onto the textures
    delete fbo_;
    fbo_ = new FramebufferObject();

    loadShader();
    initStatus_ = raycastPrg_ ? VRN_OK : VRN_ERROR;

    setLightingParameters();

    return initStatus_;
}

void VolumeEditor::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "rc_volumeeditor.frag",
                                       generateHeader(), false);
}

void VolumeEditor::compile() {
    raycastPrg_->setHeaders(generateHeader(), false);
    raycastPrg_->rebuild();
}

void VolumeEditor::process(LocalPortMapping* portMapping) {
    if ( VolumeHandleValidator::checkVolumeHandle(currentVolumeHandle_,
        portMapping->getVolumeHandle("volumehandle.volumehandle")) == false)
    {
        return;
    }

    // pass volumehandle to transfer function
    transferFunc_.setVolumeHandle(currentVolumeHandle_);

    portMapping_ = portMapping;

    int entryParams = portMapping->getTarget("image.entrypoints");
    int exitParams = portMapping->getTarget("image.exitpoints");

    std::vector<int> activeTargets;
    activeTargets.push_back(portMapping->getTarget("image.output"));
    activeTargets.push_back(portMapping->getTarget("image.firsthit"));
    tc_->setActiveTargets(activeTargets);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // compile program
    compileShader();
    LGL_ERROR;

    // bind entry params
    glActiveTexture(tm_.getGLTexUnit(entryParamsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(entryParams), tc_->getGLTexID(entryParams));
    glActiveTexture(tm_.getGLTexUnit(entryParamsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(entryParams), tc_->getGLDepthTexID(entryParams));
    LGL_ERROR;

    // bind exit params
    glActiveTexture(tm_.getGLTexUnit(exitParamsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(exitParams), tc_->getGLTexID(exitParams));
    glActiveTexture(tm_.getGLTexUnit(exitParamsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(exitParams), tc_->getGLDepthTexID(exitParams));
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

       // segmentation volume
    VolumeGL* volumeSeg = currentVolumeHandle_->getRelatedVolumeGL(Modality::MODALITY_SEGMENTATION);

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
    setGlobalShaderParameters(raycastPrg_);
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(raycastPrg_, volumeTextures);

    // pass the remaining uniforms to the shader
    raycastPrg_->setUniform("entryPoints_", tm_.getTexUnit(entryParamsTexUnit_));
    raycastPrg_->setUniform("entryPointsDepth_", tm_.getTexUnit(entryParamsDepthTexUnit_));
    raycastPrg_->setUniform("exitPoints_", tm_.getTexUnit(exitParamsTexUnit_));
    raycastPrg_->setUniform("exitPointsDepth_", tm_.getTexUnit(exitParamsDepthTexUnit_));
    raycastPrg_->setUniform("transferFunc_", tm_.getTexUnit(transferTexUnit_));
    if (useSegmentation_.get() && currentVolumeHandle_->getRelatedVolumeGL(Modality::MODALITY_SEGMENTATION))
        raycastPrg_->setUniform("segment_" , static_cast<GLfloat>(segment_.get()));

    renderQuad();

    raycastPrg_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

std::string VolumeEditor::generateHeader() {
    std::string headerSource = VolumeRaycaster::generateHeader();

    headerSource += transferFunc_.get()->getShaderDefines();

    if (useSegmentation_.get() && currentVolumeHandle_->getRelatedVolumeGL(Modality::MODALITY_SEGMENTATION))
        headerSource += "#define USE_SEGMENTATION\n";

    return headerSource;
}

void VolumeEditor::applyBrush(tgt::ivec2 mousePos) {
    float* curFirstHit = getTextureContainer()->getTargetAsFloats(portMapping_->getTarget("image.firsthit"), mousePos.x, mousePos.y);
    tgt::vec4 volCoord = tgt::vec4(curFirstHit);
    delete[] curFirstHit;

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
    MsgDistr.postMessage(new Message(VoreenPainter::repaint_));
}

void VolumeEditor::mousePressEvent(tgt::MouseEvent* e) {
    if (!tc_) return;
    if (e->modifiers() & tgt::MouseEvent::LSHIFT) {
        mouseDown_ = true;
        tgt::ivec2 mousePos = tgt::ivec2(e->coord().x, getSize().y-e->coord().y);
        applyBrush(mousePos);
        e->accept();
    } else
        e->ignore();
}

void VolumeEditor::mouseMoveEvent(tgt::MouseEvent* e) {
    if (!tc_) return;
    if (mouseDown_ && (e->modifiers() & tgt::MouseEvent::LSHIFT)) {
        tgt::ivec2 mousePos = tgt::ivec2(e->coord().x, getSize().y-e->coord().y);
        applyBrush(mousePos);
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
