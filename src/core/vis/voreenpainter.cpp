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

#include "voreen/core/vis/voreenpainter.h"

#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/trackballnavigation.h"
#include "voreen/core/opengl/texturecontainer.h"

namespace voreen {

VoreenPainterOverlay::VoreenPainterOverlay(tgt::GLCanvas* canvas)
    : canvas_(canvas),
      activated_(false),
      activateable_(false),
      name_("Unnamed")
{
}

//------------------------------------------------------------------------------

// init statics
tgt::GLCanvas* VoreenPainter::lastCanvas_ = 0;

const Identifier VoreenPainter::removeMouseListener_("remove.mouseListener");
const Identifier VoreenPainter::addMouseListener_("add.mouseListener");
const Identifier VoreenPainter::addCanvasOverlay_("add.canvasOverlay");
const Identifier VoreenPainter::addFrameOverlay_("add.frameOverlay");
const Identifier VoreenPainter::delCanvasOverlay_("del.canvasOverlay");
const Identifier VoreenPainter::delFrameOverlay_("del.frameOverlay");
const Identifier VoreenPainter::repaint_("msg.repaint");
const Identifier VoreenPainter::resize_("msg.resize");
const Identifier VoreenPainter::visibleViews_("visibleViews");
const Identifier VoreenPainter::cameraChanged_("changed.camera");
const Identifier VoreenPainter::switchCoarseness_("switch.coarseness");
const Identifier VoreenPainter::renderingFinished_("msg.renderingFinished");

VoreenPainter::VoreenPainter(tgt::GLCanvas* canvas, tgt::Trackball* track, const Identifier& tag)
    : tgt::Painter(canvas)
    , MessageReceiver(tag)
    , evaluator_(0)
    , trackball_(track)
{
    overlayMgr_ = new OverlayManager();

    stereoMode_ = VRN_MONOSCOPIC;

	size_ = canvas->getSize();
}

VoreenPainter::~VoreenPainter() {
    delete overlayMgr_;
    delete trackball_;
    if (lastCanvas_ == getCanvas())
        lastCanvas_ = 0;
}

void VoreenPainter::sizeChanged(const tgt::ivec2& size) {
    getCanvas()->getGLFocus();

	tgt::ivec2 validSize = size;
	validSize.y = validSize.y ? validSize.y : 1;

    // setup viewport, projection etc.:
    glViewport(0, 0, static_cast<GLint>(validSize.x), static_cast<GLint>(validSize.y));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
	gluPerspective(getCamera()->getFovy(), static_cast<GLfloat>(validSize.x) / static_cast<GLfloat>(validSize.y), getCamera()->getNearDist(), getCamera()->getFarDist());

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    size_ = validSize;
    MsgDistr.postMessage(new CameraPtrMsg(cameraChanged_, getCanvas()->getCamera()));

    if (evaluator_) {
        if (evaluator_->getTextureContainer())
            evaluator_->getTextureContainer()->setSize(validSize);

        evaluator_->setSize(validSize);
    }

    if (overlayMgr_)
        overlayMgr_->resize(validSize.x, validSize.y);

    invalidateRendering();
}

void VoreenPainter::invalidateRendering() {
    if (evaluator_)
        evaluator_->invalidateRendering();
}

void VoreenPainter::paint() {
    switchCanvas();

    // normally, this should have been called from the canvas-paint-method, but apparently it's still necessary to
    // repeat it here
    getCanvas()->getGLFocus();

    if (evaluator_ && evaluator_->getTextureContainer() &&
        evaluator_->getTextureContainer()->getSize() != getSize() && getSize() != tgt::ivec2(0))
    {
        //FIXME: should be handled differently
        std::cout << "VoreenPainter::paint(): resizing TextureContainer from "
                  << evaluator_->getTextureContainer()->getSize()
                  << " to " << getSize() << std::endl;
        evaluator_->getTextureContainer()->setSize(getSize());
        evaluator_->invalidateRendering();
    }

    glMatrixMode(GL_MODELVIEW);

    if (stereoMode_ == VRN_STEREOSCOPIC) { // FIXME resupport stereo
//         glDrawBuffer(GL_BACK_LEFT);
// //         camera->setStereo(true); <- TODO doesn't exist anymore, FL
//         camera->setEye(tgt::Camera::EYE_LEFT);
//         glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//         if (evaluator_)
//             evaluator_->render();
//         if (overlayMgr_)
//             overlayMgr_->paint();
//
//         // we must invalidate here or nothing will happen when we render the other eye-perspective
//         evaluator_->invalidate();
//
//         glDrawBuffer(GL_BACK_RIGHT);
//         camera->setEye(tgt::Camera::EYE_RIGHT);
//         glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//         if (evaluator_)
//             evaluator_->render();
//         if (overlayMgr_)
//             overlayMgr_->paint();
//
//         glDrawBuffer(GL_BACK);
// //         camera->setStereo(false); TODO: doesn't exist anymore, FL
//         camera->setEye(tgt::Camera::EYE_MIDDLE);
    }
    else {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        if (evaluator_) {
            evaluator_->evaluate();
        }

        if (overlayMgr_)
            overlayMgr_->paint();
    
        MsgDistr.postMessage(new Message(renderingFinished_) );
        
    }

	//getCanvas()->swap();
}

//FIXME: where is this called? duplicating functionality from setEvaluator()? joerg
void VoreenPainter::initialize() {
    getCanvas()->getGLFocus();
    if (evaluator_) {
        evaluator_->initializeGL();
    }
}

void VoreenPainter::setEvaluator(NetworkEvaluator* eval) {
	getCanvas()->getGLFocus();

    evaluator_ = eval;
    evaluator_->initializeGL(); //TODO: check if this makes sense here. joerg
}

void VoreenPainter::setTrackball(tgt::Trackball* trackball) {

    trackball_ = trackball;
}

tgt::Trackball* VoreenPainter::getTrackball() const {
    return trackball_;
}

void VoreenPainter::processMessage(Message* msg, const Identifier& dest) {
    getCanvas()->getGLFocus();
    MessageReceiver::processMessage(msg, dest);

    if (msg->id_ == repaint_) {
        //FIXME: should this not better call update()? joerg
        getCanvas()->repaint();
    }
    else if (msg->id_ == resize_) {
        tgt::ivec2 newsize = msg->getValue<tgt::ivec2>();
        sizeChanged(newsize);
    }
    else if (msg->id_ == "switch.trackballContinuousSpin") {
        if (trackball_) {
            trackball_->setContinuousSpin(msg->getValue<bool>());
            postMessage(new CameraPtrMsg(cameraChanged_, getCanvas()->getCamera()), visibleViews_);
        }
    }
    else if (msg->id_ == "set.cameraApplyOrientationAndDistanceAnimated") {
        using tgt::quat;

        tgt::Camera* camera = getCanvas()->getCamera();
        std::vector<float> keyframe = msg->getValue<std::vector<float> >();

        quat newQuat;
        newQuat.x = keyframe[0];
        newQuat.y = keyframe[1];
        newQuat.z = keyframe[2];
        newQuat.w = keyframe[3];
        float newDist = keyframe[4];

        float t = 0.1f;

        quat oldQuat = camera->getQuat();
        float oldDist = camera->getFocalLength();
        trackball_->reset();
        quat initQuat = camera->getQuat();
        initQuat.invert();

        if (evaluator_)
            evaluator_->postMessage(new BoolMsg(switchCoarseness_, true));
        for (int i = 0; i <= 9; i++) {
            quat tmp = tgt::slerpQuat(oldQuat, newQuat, std::min(t, 1.f));
            float tmpDist = t*(newDist - oldDist) + oldDist;
            trackball_->reset();
            trackball_->zoomAbsolute(tmpDist);
            trackball_->rotate(initQuat);
            trackball_->rotate(tmp);

            postMessage(new CameraPtrMsg(cameraChanged_, camera), visibleViews_);

            getCanvas()->repaint();

            t += 0.1f;
        }

        trackball_->reset();
        trackball_->zoomAbsolute(newDist);
        trackball_->rotate(initQuat);
        trackball_->rotate(newQuat); 

        if (evaluator_)
            evaluator_->postMessage(new BoolMsg(switchCoarseness_, false));
        getCanvas()->repaint();
    }
    else if (msg->id_ == "set.cameraApplyOrientation") {
        using tgt::quat;

        tgt::Camera* camera = getCanvas()->getCamera();
        std::vector<float> keyframe = msg->getValue<std::vector<float> >();

        quat newQuat;
        newQuat.x = keyframe[0];
        newQuat.y = keyframe[1];
        newQuat.z = keyframe[2];
        newQuat.w = keyframe[3];

		//if (camera->getTrackball()->isSpinning())
			//camera->getTrackball()->setSpinning(false);

        evaluator_->postMessage(new BoolMsg(switchCoarseness_, true));

        float dist = camera->getFocalLength();
        trackball_->reset();
        quat initQuat = camera->getQuat();
        initQuat.invert();
        trackball_->rotate(initQuat);
        trackball_->rotate(newQuat);
        trackball_->zoomAbsolute(dist);

        postMessage(new CameraPtrMsg(cameraChanged_, camera), visibleViews_);

        evaluator_->postMessage(new BoolMsg(switchCoarseness_, false));

        getCanvas()->repaint();
    }
    else if (msg->id_ == "set.cameraApplyQuat") {
        tgt::Camera* cam = getCanvas()->getCamera();
        trackball_->rotate(msg->getValue<tgt::quat>());
        postMessage(new CameraPtrMsg(cameraChanged_, cam), MsgDistr.getCurrentViewId());
        postMessage(new Message(repaint_), MsgDistr.getCurrentViewId());
    }
    else if (msg->id_ == "set.cameraPosition") {
        tgt::Camera* cam = getCanvas()->getCamera();
        cam->setPosition(msg->getValue<tgt::vec3>());
        postMessage(new CameraPtrMsg(cameraChanged_, cam), MsgDistr.getCurrentViewId());
        postMessage(new Message(repaint_), MsgDistr.getCurrentViewId());
    }
    else if (msg->id_ == "set.cameraFocus") {
        tgt::Camera* cam = getCanvas()->getCamera();
        cam->setFocus(msg->getValue<tgt::vec3>());
        postMessage(new CameraPtrMsg(cameraChanged_, cam), MsgDistr.getCurrentViewId());
        postMessage(new Message(repaint_), MsgDistr.getCurrentViewId());
    }
    else if (msg->id_ == "set.cameraReset") {
        tgt::Camera* cam = getCanvas()->getCamera();
        cam->setPosition(tgt::vec3(0.0f, 0.0f, 3.75f));
        cam->setFocus(tgt::vec3(0.0f, 0.0f, 0.0f));
        cam->setUpVector(tgt::vec3(0.0f, 1.0f, 0.0f));
        postMessage(new CameraPtrMsg(cameraChanged_, cam), MsgDistr.getCurrentViewId());
        postMessage(new Message(repaint_), MsgDistr.getCurrentViewId());
    }
    else if (msg->id_ == "set.cameraZoom") {
        trackball_->zoomAbsolute(msg->getValue<float>());
        postMessage(new CameraPtrMsg(cameraChanged_, getCanvas()->getCamera()), visibleViews_);
        getCanvas()->repaint();
    }
    else if (msg->id_ == delCanvasOverlay_) {
        overlayMgr_->removeOverlay(msg->getValue<voreen::VoreenPainterOverlay*>());
    }
    else if (msg->id_ == addCanvasOverlay_) {
        if (trackball_)
            trackball_->reset();
        overlayMgr_->removeOverlay(msg->getValue<voreen::VoreenPainterOverlay*>());
        overlayMgr_->addOverlay(msg->getValue<voreen::VoreenPainterOverlay*>());
        getCanvas()->getGLFocus();
        msg->getValue<voreen::VoreenPainterOverlay*>()->resize(size_.x, size_.y);
    }
    else if (msg->id_ == delFrameOverlay_) {
        overlayMgr_->removeOverlay(msg->getValue<voreen::VoreenPainterOverlay*>());
    }
    else if (msg->id_ == addFrameOverlay_) {
        if (trackball_)
            trackball_->reset();

        overlayMgr_->removeOverlay(msg->getValue<voreen::VoreenPainterOverlay*>());
        overlayMgr_->addOverlay(msg->getValue<voreen::VoreenPainterOverlay*>());

        getCanvas()->getGLFocus();
        msg->getValue<voreen::VoreenPainterOverlay*>()->resize(size_.x, size_.y);
    }
    else if (msg->id_ == addMouseListener_) {
        addMouseListener(msg->getValue<tgt::EventListener*>());
    }
	else if (msg->id_ == removeMouseListener_) {
		removeMouseListener(msg->getValue<tgt::EventListener*>());
	}
    else if (msg->id_ == "msg.paintToFile") {
        renderToSnapshot(size_, msg->getValue<std::string>());
    }
    if (overlayMgr_)
        overlayMgr_->processMessage(msg,dest);

    if (evaluator_)
        evaluator_->processMessage(msg, dest);
}

void VoreenPainter::addMouseListener(tgt::EventListener* listener) {
    getCanvas()->getEventHandler()->removeListener(listener);
    getCanvas()->getEventHandler()->addListenerToFront(listener);
}

void VoreenPainter::removeMouseListener(tgt::EventListener* listener) {
    getCanvas()->getEventHandler()->removeListener(listener);
}

void VoreenPainter::removeAllMouseListener() {
    getCanvas()->getEventHandler()->clear();
}

void VoreenPainter::switchCanvas() {
    if (lastCanvas_ != getCanvas()) {
        if (lastCanvas_) {
            VoreenPainter* oldpainter = ((VoreenPainter*)(lastCanvas_->getPainter()));
            if ( oldpainter ) {
                lastCanvas_->getGLFocus();
                if ( oldpainter->getEvaluator() )
                    oldpainter->getEvaluator()->deinit();
                getCanvas()->getGLFocus();
            }
            if (evaluator_)
                evaluator_->init();
        }
        lastCanvas_ = getCanvas();
    }
}


void VoreenPainter::renderToSnapshot(tgt::ivec2 size, std::string fileName) {
#ifdef VRN_WITH_DEVIL
    ILuint img = renderToILImage(size, NULL, 2);
    ilEnable(IL_FILE_OVERWRITE);
    //FIXME: this should only be done for bmp-files (jms)
    //ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
    if (!ilSaveImage(const_cast<char*>(fileName.c_str())))
        LERROR("renderToSnapshot() failed for " << fileName);

    ilDeleteImages(1, &img);
#endif
}


#ifdef VRN_WITH_DEVIL
ILuint VoreenPainter::renderToILImage(tgt::ivec2 size, bool (progress)(float _progress), int _aaLevel) {
    
    bool sizeModified = false;

    // adjust viewport size
    if (size != getSize()) {
        sizeChanged(size);
        sizeModified = true;
    }

    // render to ILimage
    ILuint image = renderToILImage(progress, _aaLevel);

    // reset viewport size
    if (getCanvas()) {
        if (sizeModified)
            sizeChanged(getCanvas()->getSize());
    }
    else {
        LWARNING("No canvas");
    }

    return image;
}

ILuint VoreenPainter::renderToILImage(bool (progress)(float _progress), int _aaLevel) {
    
    tgtAssert(evaluator_, "No evaluator");

    if (_aaLevel > 4)
        _aaLevel = 4;

    if (progress != NULL)
        progress(0.0);

    // render
    paint();

    // copy over rendering to temporary buffer
    uint8_t* buffer = new uint8_t[static_cast<int>(size_.x) * static_cast<int>(size_.y) * 4];
    glReadPixels(0, 0, static_cast<int>(size_.x), static_cast<int>(size_.y), GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    
    // transfer buffer to result ILimage
    ILuint result;
    ilGenImages(1, &result);
    ilBindImage(result);
    ilTexImage(static_cast<int>(size_.x), static_cast<int>(size_.y), 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, buffer);

    if (progress != 0)
        progress(1.0);

    delete [] buffer;

    return result;
}
#endif // VRN_WITH_DEVIL

const std::string VoreenPainter::loggerCat_ = "voreen.core.vis.VoreenPainer";

//------------------------------------------------------------------------

OverlayManager::OverlayManager() {
    overlays_.clear();
}

void OverlayManager::addOverlay(VoreenPainterOverlay* overlay) {
    overlays_.push_back(overlay);
}

void OverlayManager::removeOverlay(VoreenPainterOverlay* overlay) {
    std::vector<VoreenPainterOverlay*> temp;
    for (size_t i = 0; i < overlays_.size(); ++i) {
        if (overlays_.at(i) != overlay) {
            temp.push_back(overlays_.at(i));
        }
    }
    overlays_.clear();
    overlays_ = temp;
}

std::vector<VoreenPainterOverlay*> OverlayManager::getOverlays() {
    return overlays_;
}

void OverlayManager::paint() {
    for (size_t i = 0; i < overlays_.size(); ++i) {
        if (overlays_.at(i)->getIsActivated())
            overlays_.at(i)->paint();
    }
}

void OverlayManager::resize(int width, int height) {
    for (size_t i=0; i < overlays_.size(); ++i)
        overlays_.at(i)->resize(width,height);
}

void OverlayManager::processMessage(Message* msg, const Identifier& dest) {
    for (size_t i = 0; i < overlays_.size(); ++i) {
        overlays_.at(i)->processMessage(msg,dest);
    }
    if (msg->id_ == "pushOverlayDirect") {
        overlays_.push_back(msg->getValue<VoreenPainterOverlay*>());
    }
    else if (msg->id_ == "popOverlayDirect") {
        overlays_.pop_back();
    }
}

} // namespace
