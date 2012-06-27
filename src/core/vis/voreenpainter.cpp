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

#include "voreen/core/vis/voreenpainter.h"

#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/vis/processors/image/canvasrenderer.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/trackballnavigation.h"
#include "voreen/core/opengl/texturecontainer.h"

#ifdef VRN_WITH_FFMPEG
	#include "tgt/ffmpeg/videoencoder.h"
#endif

namespace voreen {

VoreenPainterOverlay::VoreenPainterOverlay(tgt::GLCanvas* canvas)
    : canvas_(canvas),
      activated_(false),
      activateable_(false),
      name_("Unnamed")
{}

//------------------------------------------------------------------------------

const Identifier VoreenPainter::addEventListener_("add.eventListener");
const Identifier VoreenPainter::removeEventListener_("remove.eventListener");
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

const std::string VoreenPainter::loggerCat_ = "voreen.core.vis.VoreenPainer";

#ifdef VRN_WITH_FFMPEG
    tgt::VideoEncoder* VoreenPainter::videoEncoder_ = 0;
#endif

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

#ifdef VRN_WITH_FFMPEG
    delete videoEncoder_;
#endif
}

void VoreenPainter::sizeChanged(const tgt::ivec2& size) {
    getCanvas()->getGLFocus();

    tgt::ivec2 validSize = size;
    validSize.y = validSize.y ? validSize.y : 1;

    // setup viewport, projection etc.:
    glViewport(0, 0, static_cast<GLint>(validSize.x), static_cast<GLint>(validSize.y));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //FIXME: should use camera->getProjectionMatrix() (and placed somewhere else?)
    gluPerspective(getCamera()->getFovy(), static_cast<GLdouble>(validSize.x) / static_cast<GLdouble>(validSize.y),
                   getCamera()->getNearDist(), getCamera()->getFarDist());

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
    if (evaluator_ && evaluator_->getTextureContainer() &&
        evaluator_->getTextureContainer()->getSize() != getSize() && getSize() != tgt::ivec2(0))
    {
        //FIXME: should be handled differently
        LINFO("VoreenPainter::paint(): resizing TextureContainer from "
              << evaluator_->getTextureContainer()->getSize()
              << " to " << getSize());
        evaluator_->getTextureContainer()->setSize(getSize());
        evaluator_->invalidateRendering();
    }

    if (stereoMode_ == VRN_STEREOSCOPIC) { // TODO: resupport stereo
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
// //         camera->setStereo(false); // doesn't exist anymore, FL
//         camera->setEye(tgt::Camera::EYE_MIDDLE);
    }
    else {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//FIXME: necessary? joerg

        if (evaluator_)
            evaluator_->evaluate();

        if (overlayMgr_)
            overlayMgr_->paint();

        MsgDistr.postMessage(new Message(renderingFinished_) );
    }
}

void VoreenPainter::setEvaluator(NetworkEvaluator* eval) {
    evaluator_ = eval;
}

void VoreenPainter::setTrackball(tgt::Trackball* trackball) {
    trackball_ = trackball;
}

tgt::Trackball* VoreenPainter::getTrackball() const {
    return trackball_;
}

void VoreenPainter::processMessage(Message* msg, const Identifier& dest) {
    MessageReceiver::processMessage(msg, dest);

    if (msg->id_ == repaint_) {
        // we want an explicit repaint here, not just update()
        getCanvas()->repaint();
    }
    else if (msg->id_ == VoreenPainter::switchCoarseness_) {
        // set correct GL context and call switchCoarseness of the evaluator
        // the new size for the processors is calculated and set there
        getCanvas()->getGLFocus();
        evaluator_->switchCoarseness(msg->getValue<bool>());
    }
    else if (msg->id_ == resize_) {
        tgt::ivec2 newsize = msg->getValue<tgt::ivec2>();
        sizeChanged(newsize);
    }
    else if (msg->id_ == "switch.trackballContinuousSpin") {
        if (trackball_) {
            trackball_->setContinuousSpin(msg->getValue<bool>());
            MsgDistr.postMessage(new CameraPtrMsg(cameraChanged_, getCanvas()->getCamera()), visibleViews_);
        }
    }
    else if (msg->id_ == "set.cameraApplyOrientationAndDistanceAnimated") {
        tgt::Camera* camera = getCanvas()->getCamera();
        std::vector<float> keyframe = msg->getValue<std::vector<float> >();

        tgt::quat newQuat;
        newQuat.x = keyframe[0];
        newQuat.y = keyframe[1];
        newQuat.z = keyframe[2];
        newQuat.w = keyframe[3];
        float newDist = keyframe[4];

        float t = 0.1f;

        tgt::quat oldQuat = camera->getQuat();
        float oldDist = camera->getFocalLength();
        trackball_->reset();
        tgt::quat initQuat = camera->getQuat();
        initQuat.invert();

        if (evaluator_)
            evaluator_->postMessage(new BoolMsg(switchCoarseness_, true));
        for (int i = 0; i <= 9; ++i) {
            tgt::quat tmp = tgt::slerpQuat(oldQuat, newQuat, std::min(t, 1.f));
            float tmpDist = t*(newDist - oldDist) + oldDist;
            trackball_->reset();
            trackball_->zoomAbsolute(tmpDist);
            trackball_->rotate(initQuat);
            trackball_->rotate(tmp);

            MsgDistr.postMessage(new CameraPtrMsg(cameraChanged_, camera), visibleViews_);

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
        tgt::Camera* camera = getCanvas()->getCamera();
        std::vector<float> keyframe = msg->getValue<std::vector<float> >();

        tgt::quat newQuat;
        newQuat.x = keyframe[0];
        newQuat.y = keyframe[1];
        newQuat.z = keyframe[2];
        newQuat.w = keyframe[3];

        //if (camera->getTrackball()->isSpinning())
            //camera->getTrackball()->setSpinning(false);

        evaluator_->postMessage(new BoolMsg(switchCoarseness_, true));

        float dist = camera->getFocalLength();
        trackball_->reset();
        tgt::quat initQuat = camera->getQuat();
        initQuat.invert();
        trackball_->rotate(initQuat);
        trackball_->rotate(newQuat);
        trackball_->zoomAbsolute(dist);

        MsgDistr.postMessage(new CameraPtrMsg(cameraChanged_, camera), visibleViews_);

        evaluator_->postMessage(new BoolMsg(switchCoarseness_, false));

        getCanvas()->repaint();
    }
    else if (msg->id_ == "set.cameraApplyQuat") {
        tgt::Camera* cam = getCanvas()->getCamera();
        trackball_->rotate(msg->getValue<tgt::quat>());
        MsgDistr.postMessage(new CameraPtrMsg(cameraChanged_, cam), MsgDistr.getCurrentViewId());
        MsgDistr.postMessage(new Message(repaint_), MsgDistr.getCurrentViewId());
    }
    else if (msg->id_ == "set.cameraPosition") {
        tgt::Camera* cam = getCanvas()->getCamera();
        cam->setPosition(msg->getValue<tgt::vec3>());
        MsgDistr.postMessage(new CameraPtrMsg(cameraChanged_, cam));
    //    MsgDistr.postMessage(new Message(repaint_), MsgDistr.getCurrentViewId());
    }
    else if (msg->id_ == "set.cameraFocus") {
        tgt::Camera* cam = getCanvas()->getCamera();
        cam->setFocus(msg->getValue<tgt::vec3>());
        MsgDistr.postMessage(new CameraPtrMsg(cameraChanged_, cam));
    //    MsgDistr.postMessage(new Message(repaint_), MsgDistr.getCurrentViewId());
    }
    else if (msg->id_ == "set.cameraUp") {
        tgt::Camera* cam = getCanvas()->getCamera();
        cam->setUpVector(msg->getValue<tgt::vec3>());
        MsgDistr.postMessage(new CameraPtrMsg(cameraChanged_, cam));
    //    MsgDistr.postMessage(new Message(repaint_), MsgDistr.getCurrentViewId());
    }
    else if (msg->id_ == "set.cameraReset") {
        tgt::Camera* cam = getCanvas()->getCamera();
        cam->setPosition(tgt::vec3(0.0f, 0.0f, 3.75f));
        cam->setFocus(tgt::vec3(0.0f, 0.0f, 0.0f));
        cam->setUpVector(tgt::vec3(0.0f, 1.0f, 0.0f));
        MsgDistr.postMessage(new CameraPtrMsg(cameraChanged_, cam), MsgDistr.getCurrentViewId());
        MsgDistr.postMessage(new Message(repaint_), MsgDistr.getCurrentViewId());
    }
    else if (msg->id_ == "set.cameraZoom") {
        trackball_->zoomAbsolute(msg->getValue<float>());
        MsgDistr.postMessage(new CameraPtrMsg(cameraChanged_, getCanvas()->getCamera()), visibleViews_);
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
    else if (msg->id_ == addEventListener_) {
        addEventListener(msg->getValue<tgt::EventListener*>());
    }
    else if (msg->id_ == removeEventListener_) {
        removeEventListener(msg->getValue<tgt::EventListener*>());
    }
    else if (msg->id_ == "msg.paintToFile") {
        renderToSnapshot(size_, msg->getValue<std::string>());
    }
    else if (msg->id_ == "msg.paintToEncoder")
    	renderToEncoder();

    if (overlayMgr_)
        overlayMgr_->processMessage(msg,dest);
}

void VoreenPainter::addEventListener(tgt::EventListener* listener) {
    getCanvas()->getEventHandler()->removeListener(listener);
    getCanvas()->getEventHandler()->addListenerToFront(listener);
}

void VoreenPainter::removeEventListener(tgt::EventListener* listener) {
    getCanvas()->getEventHandler()->removeListener(listener);
}

void VoreenPainter::removeAllEventListeners() {
    getCanvas()->getEventHandler()->clear();
}


#ifdef VRN_WITH_FFMPEG
/**
 * lazy init pattern to shrink memory footprint
 */
tgt::VideoEncoder* VoreenPainter::getVideoEncoder(){
	if (!videoEncoder_)
	    videoEncoder_ = new tgt::VideoEncoder();
	return videoEncoder_;
}
#endif /* VRN_WITH_FFMPEG */

/**
 * pass current screen as frame to videoencoder
 */
void VoreenPainter::renderToEncoder() {
#ifdef VRN_WITH_FFMPEG
	// TODO check:in fact we don't need to worry about the canvas as we address directly to the canvas renderer
	// render our image
	//getCanvas()->repaint();

	// We need to access the input render target of the canvas renderer, as the final target
	// will be cropped to the size of the visible canvas.
	CanvasRenderer* canvas = evaluator_->getCanvasRenderer();
	if (!canvas) {
		LERROR("renderToEncoder(): no CanvasRenderer found");
		return;
	}

	// read pixel data as floats as expected by <code>nextFrame(void*)</code>
    float* pixels = evaluator_->getTextureContainer()->getTargetAsFloats(canvas->getImageID());
	getVideoEncoder()->nextFrame(pixels);
    delete[] pixels;

#else /* !defined(VRN_WITH_FFMPEG) */
	LWARNING("FFMpeg is required for Video Encoding.");
#endif
}

/**
 * @param fileName if NULL and ifdef VRN_WITH_FFMPEG matched rendering to encoder
 */
#ifdef VRN_WITH_DEVIL
void VoreenPainter::renderToSnapshot(tgt::ivec2 size, std::string fileName)
    throw (std::bad_alloc, tgt::FileException)
{
    ILuint img = renderToILImage(size);
    ilEnable(IL_FILE_OVERWRITE);
    if (!ilSaveImage(const_cast<char*>(fileName.c_str())))
        throw tgt::FileException("Could not save snapshot to file " + fileName);

    ilDeleteImages(1, &img);
    
#else /* !defined(VRN_WITH_DEVIL) */
void VoreenPainter::renderToSnapshot(tgt::ivec2, std::string) {
    LWARNING("DevIL is required for snapshot generation.");
#endif
}

#ifdef VRN_WITH_DEVIL
ILuint VoreenPainter::renderToILImage(const tgt::ivec2& size) {
    bool sizeModified = false;

    // adjust viewport size
    if (size != getSize()) {
        sizeChanged(size);
        sizeModified = true;
    }

    ILuint image;
    try {
        // render to ILimage
        image = renderToILImageInternal(size);
    }
    catch (const std::bad_alloc& /*e*/){
        // reset viewport size
        if (sizeModified)
            sizeChanged(getCanvas()->getSize());
        
        throw; // throw it to the caller
    }

    // reset viewport size
    if (sizeModified)
        sizeChanged(getCanvas()->getSize());

    return image;
}

ILuint VoreenPainter::renderToILImageInternal(const tgt::ivec2& size) {
    tgtAssert(evaluator_, "No evaluator");

    // render our image
    getCanvas()->repaint();

    // We need to access the input render target of the canvas renderer, as the final target
    // will be cropped to the size of the visible canvas.
    CanvasRenderer* canvas = evaluator_->getCanvasRenderer();
    if (!canvas) {
        LERROR("renderToILImage(): no CanvasRenderer found");
        return 0;
    }

    // read pixel data as floats and convert them to bytes
    tgt::vec4* pixels_f = reinterpret_cast<tgt::vec4*>(evaluator_->getTextureContainer()->getTargetAsFloats(canvas->getImageID()));
    tgt::col4* pixels_b = new tgt::col4[size.x * size.y];

    for (int i=0; i < size.x * size.y; i++)
        pixels_b[i] = tgt::clamp(pixels_f[i], 0.f, 1.f) * 255.f;

    // put pixels into IL-Image
    ILuint result;
    ilGenImages(1, &result);
    ilBindImage(result);
    ilTexImage(size.x, size.y, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pixels_b);

    delete[] pixels_f;
    delete[] pixels_b;

    return result;
}
#endif // VRN_WITH_DEVIL

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
        if (overlays_.at(i) != overlay)
            temp.push_back(overlays_.at(i));
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
    for (size_t i = 0; i < overlays_.size(); ++i)
        overlays_.at(i)->processMessage(msg,dest);

    if (msg->id_ == "pushOverlayDirect")
        overlays_.push_back(msg->getValue<VoreenPainterOverlay*>());
    else if (msg->id_ == "popOverlayDirect")
        overlays_.pop_back();
}

} // namespace
