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

#include "stereocanvasrenderer.h"

#include "voreen/core/utils/voreenpainter.h"
#include "voreen/core/utils/stringutils.h"
#include "voreen/core/processors/processorwidget.h"
#include "voreen/core/processors/processorwidgetfactory.h"
#include "voreen/core/voreenapplication.h"

#include "tgt/glcanvas.h"
#include "tgt/texturemanager.h"
#include "tgt/gpucapabilities.h"
#include "tgt/filesystem.h"
#include "tgt/camera.h"

#ifdef VRN_MODULE_DEVIL
#include "modules/devil/devilmodule.h"
#endif

namespace voreen {

const std::string StereoCanvasRenderer::loggerCat_("voreen.stereoscopy.StereoCanvasRenderer");

//------------------------------------------------------------------------------------------------------------------
//      basic processor functions                                                                                   
//------------------------------------------------------------------------------------------------------------------
StereoCanvasRenderer::StereoCanvasRenderer()
    : CanvasRenderer()
    //ports
    , tempPort_(Port::OUTPORT, "private.temp", "TempPort")
    , storagePort_(Port::OUTPORT, "private.storage", "StoragePort")
    //properties
        //stereo settings
    , stereoModeProp_("stereoModeProp","Stereo Mode")
    , eyeInvertProp_("eyeInvertProp","Invert Eyes", false)
    , calibrateDisplayProp_("calibrateDisplay", "Show Calibration", false)
    , anaglyphModeProp_("anaglyphModeProp","Anaglyph Mode")
        //camera settings
    , cameraProp_("cameraProp","Camera", tgt::Camera(tgt::vec3(0.0f, 0.0f, 50.f), tgt::vec3(0.0f, 0.0f, 0.0f), 
                                                 tgt::vec3(0.0f, 1.0f, 0.0f), 45.f,1.f,0.1f,500.f))
    , cameraHandler_(0)
    , eyeSeparationProp_("eyeSeparationProp","Eye Separation", 1.f, 0.0f, 10.0f)
    , stereoAxisModeProp_("stereoAxisModeProp","Stereo Axis Mode")
        //events
    , mouseMoveEventProp_("mouseEvent.move", "Move Event", this, &StereoCanvasRenderer::mouseMove,
      tgt::MouseEvent::MOUSE_BUTTON_NONE, tgt::MouseEvent::MOTION | tgt::MouseEvent::CLICK | tgt::MouseEvent::ENTER_EXIT, tgt::MouseEvent::MODIFIER_NONE)
    //in case pof head tracking
#ifdef VRN_MODULE_HEADTRACKING
    , coPort_(Port::INPORT, "coInport", "coInport", false)
#endif
    //set member to default value
    , calibrationRightTexture_(0), calibrationLeftTexture_(0)
    , splitScreenShader_(0), anaglyphShader_(0), autostereoscopicShader_(0)
    , previousCameraProjectionMode_(tgt::Camera::PERSPECTIVE)
    , nextExpectedImage_(NORMAL)
    , lastRunWasInInteractionMode_(false) 
    , w8ingOn2Eye_(false)
{

    //port settings (normal render port added by CanvasRenderer)
    addPrivateRenderPort(tempPort_);
    addPrivateRenderPort(storagePort_);
#ifdef VRN_MODULE_HEADTRACKING
    addPort(coPort_);
#endif
    addEventProperty(mouseMoveEventProp_);

    // stereo settings
    addProperty(stereoModeProp_);
        stereoModeProp_.addOption("nostereo","No Stereo",StereoCanvasRenderer::NO_STEREO_MODE);
        stereoModeProp_.addOption("splitscreen","Split Screen",StereoCanvasRenderer::SPLITSCREEN_STEREO_MODE);
        stereoModeProp_.addOption("autostereoscopic","Auto-Stereoscopic",StereoCanvasRenderer::AUTOSTEREOSCOPIC_STEREO_MODE);
        stereoModeProp_.addOption("anaglyph","Anaglyph",StereoCanvasRenderer::ANAGLYPH_STEREO_MODE);
        //stereoModeProp_.addOption("quadbuffer","Quadbuffer",StereoCanvasRenderer::QUADBUFFER_STEREO_MODE);
        stereoModeProp_.onChange(CallMemberAction<StereoCanvasRenderer>(this, &StereoCanvasRenderer::stereoModeOnChange));
        stereoModeProp_.setGroupID("stereo settings");
    addProperty(eyeInvertProp_);
        eyeInvertProp_.onChange(CallMemberAction<StereoCanvasRenderer>(this, &StereoCanvasRenderer::eyeInvertOnChange));
        eyeInvertProp_.setGroupID("stereo settings");
    addProperty(calibrateDisplayProp_);
        calibrateDisplayProp_.onChange(CallMemberAction<StereoCanvasRenderer>(this, &StereoCanvasRenderer::calibrateDisplayOnChange));
        calibrateDisplayProp_.setGroupID("stereo settings");
    addProperty(anaglyphModeProp_);
        anaglyphModeProp_.addOption("redcyan","Red - Cyan",StereoCanvasRenderer::RED_CYAN);
        anaglyphModeProp_.addOption("redblue","Red - Blue",StereoCanvasRenderer::RED_BLUE);
        anaglyphModeProp_.addOption("redgreen","Red - Green",StereoCanvasRenderer::RED_GREEN);
        anaglyphModeProp_.onChange(CallMemberAction<StereoCanvasRenderer>(this, &StereoCanvasRenderer::anaglyphModeOnChange));
        anaglyphModeProp_.setGroupID("stereo settings");
    setPropertyGroupGuiName("stereo settings", "Stereo Settings");

    // camera settings
    addProperty(cameraProp_);
        cameraProp_.setGroupID("camera settings");
        cameraHandler_ = new CameraInteractionHandler("cameraHandler", "Camera", &cameraProp_);
        addInteractionHandler(cameraHandler_);
    addProperty(eyeSeparationProp_);
        eyeSeparationProp_.onChange(CallMemberAction<StereoCanvasRenderer>(this, &StereoCanvasRenderer::eyeSeparationOnChange));
        eyeSeparationProp_.setGroupID("camera settings");
    addProperty(stereoAxisModeProp_);
        stereoAxisModeProp_.addOption("onAxis", "On Axis", tgt::Camera::ON_AXIS);
        stereoAxisModeProp_.addOption("onAxisHMD","On Axis (HMD)",tgt::Camera::ON_AXIS_HMD);
        stereoAxisModeProp_.onChange(CallMemberAction<StereoCanvasRenderer>(this, &StereoCanvasRenderer::stereoAxisModeOnChange));
        stereoAxisModeProp_.setGroupID("camera settings");
    setPropertyGroupGuiName("camera settings", "Camera Settings");
}

StereoCanvasRenderer::~StereoCanvasRenderer() {
    //delete camera handle
    delete cameraHandler_;
}

void StereoCanvasRenderer::initialize() throw (tgt::Exception) {
    CanvasRenderer::initialize();
    //load shaders
    splitScreenShader_ = ShdrMgr.loadSeparate("passthrough.vert", "copyimagesplitscreen.frag", generateHeader(), false);
    anaglyphShader_ = ShdrMgr.loadSeparate("passthrough.vert", "anaglyph.frag", generateHeader(), false);
    autostereoscopicShader_ = ShdrMgr.loadSeparate("passthrough.vert", "autostereoscopic.frag", generateHeader(), false);
    //load textures
    calibrationRightTexture_ = TexMgr.load(VoreenApplication::app()->getResourcePath("textures/stereocalibrationR.png"));
    calibrationLeftTexture_ = TexMgr.load(VoreenApplication::app()->getResourcePath("textures/stereocalibrationL.png"));
    //prepare camera (set ProjectionMode to FRUSTUM)
    tgt::Camera* cam = const_cast<tgt::Camera*>(&cameraProp_.get());
    previousCameraProjectionMode_ = cam->getProjectionMode();
    cam->setProjectionMode(tgt::Camera::FRUSTUM);
    cameraProp_.invalidate();
    //get properties right
    stereoModeOnChange();
}

void StereoCanvasRenderer::deinitialize() throw (tgt::Exception) {
    //delete shaders
    ShdrMgr.dispose(splitScreenShader_);
    splitScreenShader_ = 0;
    ShdrMgr.dispose(anaglyphShader_);
    anaglyphShader_ = 0;
    ShdrMgr.dispose(autostereoscopicShader_);
    autostereoscopicShader_ = 0;
    //delete textures
    if (calibrationRightTexture_)
       TexMgr.dispose(calibrationRightTexture_);
    calibrationRightTexture_ = 0;
    if (calibrationLeftTexture_)
       TexMgr.dispose(calibrationLeftTexture_);
    calibrationLeftTexture_ = 0;
    //set camera projection mode back to previous mode
    tgt::Camera* cam = const_cast<tgt::Camera*>(&cameraProp_.get());
    cam->setProjectionMode(previousCameraProjectionMode_);
    //call super function
    CanvasRenderer::deinitialize();
}

void StereoCanvasRenderer::onEvent(tgt::Event* e) {
    if (canvas_) {
        canvas_->getGLFocus();
        tgt::MouseEvent* me = dynamic_cast<tgt::MouseEvent*>(e);
        //pass, if no mouseevent and stereo mode is split screen
        if (!me || mouseMoveEventProp_.accepts(me) || stereoModeProp_.getValue() != SPLITSCREEN_STEREO_MODE) {
            RenderProcessor::onEvent(e);
            return;
        }

        //set right viewport for mouse events in split screen mode
        tgt::ivec2 view = me->viewport();
        switch(stereoModeProp_.getValue()){
            case NO_STEREO_MODE:
            case AUTOSTEREOSCOPIC_STEREO_MODE:
            case ANAGLYPH_STEREO_MODE:
            case QUADBUFFER_STEREO_MODE:
                //should not get here
                tgtAssert(false,"StereoCanvasRenderer::onEvent: Unexpected stereo mode");
                break;
            case SPLITSCREEN_STEREO_MODE:
                view.x /= 2;
                if (me->x() < (me->viewport().x / 2)) {
                    tgt::MouseEvent newme(me->x(), me->y(), me->action(), me->modifiers(), me->button(), view);
                    newme.ignore();  // accepted is set to true by default
                    inport_.distributeEvent(&newme);
                    if (newme.isAccepted())
                        me->accept();
                }
                else {
                    tgt::MouseEvent newme(me->x() - (me->viewport().x / 2), me->y(), me->action(), me->modifiers(), me->button(), view);
                    newme.ignore();  // accepted is set to true by default
                    inport_.distributeEvent(&newme);
                    if (newme.isAccepted())
                        me->accept();
                }
                break;
            default:
                //should not get here
                tgtAssert(false,"StereoCanvasRenderer::onEvent: Unexpected stereo mode");
                break;
        }    
    }
}

void StereoCanvasRenderer::process() {
    if (!canvas_)
        return;
    //process based on stereo mode
    switch(stereoModeProp_.getValue()){
    case NO_STEREO_MODE:
        //set camera eye to middle and process like a normal canvas
        if(cameraProp_.setStereoEyeMode(tgt::Camera::EYE_MIDDLE))
            cameraProp_.invalidate();
        CanvasRenderer::process();
        break;
    case SPLITSCREEN_STEREO_MODE:
    case ANAGLYPH_STEREO_MODE:
    case AUTOSTEREOSCOPIC_STEREO_MODE:
    case QUADBUFFER_STEREO_MODE:
        if (inport_.isReady()) {
            if(calibrateDisplayProp_.get()){ // show calibration textures
                if (!calibrationRightTexture_ || !calibrationLeftTexture_) {
                   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                   return;
                } 
                copyIntoStorage(calibrationLeftTexture_,calibrationRightTexture_);
                copyIntoCanvas();
            } 
            else {
                if(inport_.hasChanged() || !storagePort_.hasValidResult())
                    processStereo();
                else
                    copyIntoCanvas(); 
            }
        } 
        else { // not ready (show error texture)
            if (!errorTex_) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                return;
            }
            copyIntoStorage(errorTex_,errorTex_);
            copyIntoCanvas();
        } 
        break;
    default:
        LERROR("StereoCanvas: Unknown StereoMode!!!");
        break;
    }
#ifdef VRN_MODULE_HEADTRACKING
     //head tracking
    getTrackingUpdate();
#endif
    LGL_ERROR;
}

//------------------------------------------------------------------------------------------------------------------
//      resize functions                                                                                            
//------------------------------------------------------------------------------------------------------------------
void StereoCanvasRenderer::resizeAllPorts(tgt::ivec2 newsize) {
    tgt::ivec2 size;
    switch(stereoModeProp_.getValue()){
    case NO_STEREO_MODE:
    case ANAGLYPH_STEREO_MODE:
        inport_.requestSize(newsize);
        storagePort_.resize(newsize);
        tempPort_.resize(newsize);
        nextExpectedImage_ = NORMAL;
        invalidate();
        break;
    case SPLITSCREEN_STEREO_MODE:
    case AUTOSTEREOSCOPIC_STEREO_MODE:
        size = newsize;
        size.x /= 2;
        inport_.requestSize(size);
        storagePort_.resize(newsize);
        tempPort_.resize(size);
        nextExpectedImage_ = NORMAL;
        invalidate();
        break;
    case QUADBUFFER_STEREO_MODE:
        size = newsize;
        size.x *= 2;
        inport_.requestSize(newsize);
        storagePort_.resize(size);
        tempPort_.resize(newsize);
        nextExpectedImage_ = NORMAL;
        invalidate();
        break;
    default:
        break;
    }
}

void StereoCanvasRenderer::canvasResized(tgt::ivec2 newsize) {
    if (canvas_) {
        canvas_->getGLFocus();
        resizeAllPorts(newsize);
        canvasSize_.set(newsize);
    }
}

void StereoCanvasRenderer::resizeCanvas(tgt::ivec2 newsize) {
    if (!tgt::hand(tgt::greaterThanEqual(newsize, tgt::ivec2(canvasSize_.getMinValue()))) && tgt::hand(tgt::lessThanEqual(newsize, canvasSize_.getMaxValue()))) {
        LWARNING("Invalid canvas dimensions: " << newsize << ". Ignoring.");
    }

    if (!canvas_)
        return;

    if (getProcessorWidget() && getProcessorWidget()->getSize() != newsize) {
        if (getProcessorWidget()->isVisible()) {
            getProcessorWidget()->setSize(newsize.x, newsize.y);
        }
        newsize = getProcessorWidget()->getSize();
    }

    if (newsize != inport_.getSize()) {
        canvas_->getGLFocus();
        glViewport(0, 0, static_cast<GLint>(newsize.x), static_cast<GLint>(newsize.y));
        resizeAllPorts(newsize);
    }
    canvasSize_.set(newsize);
}

void StereoCanvasRenderer::setCanvas(tgt::GLCanvas* canvas) {
    if (canvas == canvas_)
        return;

    //remove from old canvas:
    if (canvas_) {
        tgt::EventHandler* eh = canvas_->getEventHandler();
        if (eh) {
            eh->removeListener(this);
        }
    }
    canvas_ = canvas;
    //register at new canvas:
    if (canvas_) {
        tgt::EventHandler* eh = canvas_->getEventHandler();
        if (eh) {
            eh->addListenerToFront(this);
        }
        resizeAllPorts(canvas->getSize());
    }
    invalidate();
}

//------------------------------------------------------------------------------------------------------------------
//      get/renderTo texture functions                                                                              
//------------------------------------------------------------------------------------------------------------------
const tgt::Texture* StereoCanvasRenderer::getImageColorTexture() const {
    switch(stereoModeProp_.getValue()){
        case NO_STEREO_MODE:
            return CanvasRenderer::getImageColorTexture();
            break;
        case SPLITSCREEN_STEREO_MODE:
        case ANAGLYPH_STEREO_MODE:
        case AUTOSTEREOSCOPIC_STEREO_MODE:
        case QUADBUFFER_STEREO_MODE:
            if (storagePort_.hasRenderTarget())
                return storagePort_.getColorTexture(); 
            else
                return 0; 
            break;
        default:
            tgtAssert(false,"Unknown StereoMode!");
            return 0;
    }
}

tgt::Texture* StereoCanvasRenderer::getImageColorTexture(){
     switch(stereoModeProp_.getValue()){
        case NO_STEREO_MODE:
            return CanvasRenderer::getImageColorTexture();
            break;
        case SPLITSCREEN_STEREO_MODE:
        case ANAGLYPH_STEREO_MODE:
        case AUTOSTEREOSCOPIC_STEREO_MODE:
        case QUADBUFFER_STEREO_MODE:
            if (storagePort_.hasRenderTarget())
                return storagePort_.getColorTexture(); 
            else
                return 0; 
            break;
        default:
            tgtAssert(false,"Unknown StereoMode!");
            return 0;
    }
}

const tgt::Texture* StereoCanvasRenderer::getImageDepthTexture() const{
    switch(stereoModeProp_.getValue()){
        case NO_STEREO_MODE:
            return CanvasRenderer::getImageDepthTexture();
            break;
        case SPLITSCREEN_STEREO_MODE:
        case ANAGLYPH_STEREO_MODE:
        case AUTOSTEREOSCOPIC_STEREO_MODE:
        case QUADBUFFER_STEREO_MODE:
            if (storagePort_.hasRenderTarget())
                return storagePort_.getDepthTexture(); 
            else
                return 0; 
            break;
        default:
            tgtAssert(false,"Unknown StereoMode!");
            return 0;
    }
}
    
tgt::Texture* StereoCanvasRenderer::getImageDepthTexture() {
    switch(stereoModeProp_.getValue()){
        case NO_STEREO_MODE:
            return CanvasRenderer::getImageDepthTexture();
            break;
        case SPLITSCREEN_STEREO_MODE:
        case ANAGLYPH_STEREO_MODE:
        case AUTOSTEREOSCOPIC_STEREO_MODE:
        case QUADBUFFER_STEREO_MODE:
            if (storagePort_.hasRenderTarget())
                return storagePort_.getDepthTexture(); 
            else
                return 0; 
            break;
        default:
            tgtAssert(false,"Unknown StereoMode!");
            return 0;
    }
}   

bool StereoCanvasRenderer::renderToImage(const std::string &filename) {
    switch(stereoModeProp_.getValue()){
        case NO_STEREO_MODE:
            return CanvasRenderer::renderToImage(filename);
            break;
        case SPLITSCREEN_STEREO_MODE:
        case ANAGLYPH_STEREO_MODE:
        case AUTOSTEREOSCOPIC_STEREO_MODE:
        case QUADBUFFER_STEREO_MODE:
            if (!canvas_) {
                LWARNING("StereoCanvasRenderer::renderToImage(): no canvas assigned");
                return false;
            }

            if (!storagePort_.hasRenderTarget()) {
                LWARNING("StereoCanvasRenderer::renderToImage(): storagePort has no data");
                return false;
            }

            renderToImageError_.clear();

            try {
                storagePort_.saveToImage(filename);
                LINFO("Saved rendering " << storagePort_.getSize() << " to file: " << tgt::FileSystem::cleanupPath(renderToImageFilename_));
            }
            catch (std::bad_alloc& /*e*/) {
                LERROR("Exception in StereoCanvasRenderer::renderToImage(): bad allocation (" << getID() << ")");
                renderToImageError_ = "Not enough system memory (bad allocation)";
            }
            catch (VoreenException& e) {
                LERROR(e.what());
                renderToImageError_ = std::string(e.what());
            }
            catch (std::exception& e) {
                LERROR("Exception in StereoCanvasRenderer::renderToImage(): " << e.what() << " (" << getID() << ")");
                renderToImageError_ = std::string(e.what());
            }
           
            return (renderToImageError_.empty());
            break;
        default:
            tgtAssert(false,"Unknown StereoMode!");
            return false;
    }
}
bool StereoCanvasRenderer::renderToImage(const std::string &filename, tgt::ivec2 dimensions){
    bool success; tgt::ivec2 oldDimensions;
    switch(stereoModeProp_.getValue()){
        case NO_STEREO_MODE:
            return CanvasRenderer::renderToImage(filename,dimensions);
            break;
        case SPLITSCREEN_STEREO_MODE:
        case ANAGLYPH_STEREO_MODE:
        case AUTOSTEREOSCOPIC_STEREO_MODE:
        case QUADBUFFER_STEREO_MODE:
             if (!canvas_) {
                LWARNING("StereoCanvasRenderer::renderToImage(): no canvas assigned");
                return false;
            }

            if (!storagePort_.hasRenderTarget()) {
                LWARNING("StereoCanvasRenderer::renderToImage(): storagePort has no data");
                return false;
            }

            oldDimensions = storagePort_.getSize();
            // resize texture container to desired image dimensions and propagate change
            resizeAllPorts(dimensions);
            canvas_->getGLFocus();
            canvas_->repaint();
            canvas_->repaint();
            canvas_->repaint();
            // render with adjusted viewport size
            success = renderToImage(filename);
            // reset texture container dimensions from canvas size
            resizeAllPorts(oldDimensions);

            return success;
            break;
        default:
            tgtAssert(false,"Unknown StereoMode!");
            return false;
    }
}

//------------------------------------------------------------------------------------------------------------------
//      on change and event functions                                                                               
//------------------------------------------------------------------------------------------------------------------
void StereoCanvasRenderer::stereoModeOnChange() {
    nextExpectedImage_ = NORMAL;
    canvasResized(canvasSize_.get());
    //get autostereo frustum right
    if(stereoModeProp_.getValue() != AUTOSTEREOSCOPIC_STEREO_MODE) {
        if(BoolMetaData* meta = dynamic_cast<BoolMetaData*>(getMetaDataContainer().getMetaData("AutoStereoscopicFrustum"))) {
            if(meta->getValue()){
                meta->setValue(false);
                const_cast<tgt::Camera*>(&cameraProp_.get())->setFrustTop(cameraProp_.get().getFrustTop()*2.f);
                const_cast<tgt::Camera*>(&cameraProp_.get())->setFrustBottom(cameraProp_.get().getFrustBottom()*2.f);
                cameraProp_.invalidate();
            }
        } else {
            getMetaDataContainer().addMetaData("AutoStereoscopicFrustum", new BoolMetaData(false));
        }
    }
    //switch based on StereoMode
    switch(stereoModeProp_.getValue()) {
    case NO_STEREO_MODE:
        eyeInvertProp_.set(false);
        eyeInvertProp_.setWidgetsEnabled(false);
        calibrateDisplayProp_.set(false);
        calibrateDisplayProp_.setWidgetsEnabled(false);
        anaglyphModeProp_.setVisible(false);
        eyeSeparationProp_.setWidgetsEnabled(false);
        stereoAxisModeProp_.setWidgetsEnabled(false);
        break;
    case AUTOSTEREOSCOPIC_STEREO_MODE:
        if(BoolMetaData* meta = dynamic_cast<BoolMetaData*>(getMetaDataContainer().getMetaData("AutoStereoscopicFrustum"))) {
            if(!meta->getValue()){
                meta->setValue(true);
                const_cast<tgt::Camera*>(&cameraProp_.get())->setFrustTop(cameraProp_.get().getFrustTop()/2.f);
                const_cast<tgt::Camera*>(&cameraProp_.get())->setFrustBottom(cameraProp_.get().getFrustBottom()/2.f);
                cameraProp_.invalidate();
            }
        } else {
            getMetaDataContainer().addMetaData("AutoStereoscopicFrustum", new BoolMetaData(true));
            const_cast<tgt::Camera*>(&cameraProp_.get())->setFrustTop(cameraProp_.get().getFrustTop()/2.f);
            const_cast<tgt::Camera*>(&cameraProp_.get())->setFrustBottom(cameraProp_.get().getFrustBottom()/2.f);
            cameraProp_.invalidate();
        }
        //no break
    case QUADBUFFER_STEREO_MODE:
    case SPLITSCREEN_STEREO_MODE:
        eyeInvertProp_.setWidgetsEnabled(true);
        calibrateDisplayProp_.setWidgetsEnabled(true);
        anaglyphModeProp_.setVisible(false);
        eyeSeparationProp_.setWidgetsEnabled(true);
        stereoAxisModeProp_.setWidgetsEnabled(true);
        break;
    case ANAGLYPH_STEREO_MODE:
        eyeInvertProp_.setWidgetsEnabled(true);
        calibrateDisplayProp_.setWidgetsEnabled(true);
        anaglyphModeProp_.setVisible(true);
        eyeSeparationProp_.setWidgetsEnabled(true);
        stereoAxisModeProp_.setWidgetsEnabled(true);
        break;
    default:
        tgtAssert(false,"Unknown StereoMode!");
        break;
    }
    storagePort_.invalidateResult(); //force new rendering
}

void StereoCanvasRenderer::eyeInvertOnChange() {
    storagePort_.invalidateResult();
    invalidate();
}

void StereoCanvasRenderer::calibrateDisplayOnChange() {
    storagePort_.invalidateResult();
    invalidate();
}

void StereoCanvasRenderer::anaglyphModeOnChange() {
    storagePort_.invalidateResult();
    invalidate();
}

void StereoCanvasRenderer::eyeSeparationOnChange() {
    if(cameraProp_.setStereoEyeSeparation(eyeSeparationProp_.get()))
        cameraProp_.invalidate();
}

void StereoCanvasRenderer::stereoAxisModeOnChange() {
    if(cameraProp_.setStereoAxisMode(stereoAxisModeProp_.getValue()))
        cameraProp_.invalidate();
}

void StereoCanvasRenderer::mouseMove(tgt::MouseEvent* e) {
    tgt::ivec2 view;
    tgt::MouseEvent* me;
    switch(stereoModeProp_.getValue()){
        case NO_STEREO_MODE:
        case ANAGLYPH_STEREO_MODE:
        case AUTOSTEREOSCOPIC_STEREO_MODE:
        case QUADBUFFER_STEREO_MODE:
            break;
        case SPLITSCREEN_STEREO_MODE: 
            e->accept();
            view = e->viewport();
            view.x /= 2;
            me = new tgt::MouseEvent(e->x() % (e->viewport().x/2), e->y() , tgt::MouseEvent::MOTION, e->modifiers(), e->button(), view);
            me->ignore();
            inport_.distributeEvent(me);
            if(me->isAccepted())
                e->accept();
            delete me;
            break;
        default:
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------
//      render functions                                                                                            
//------------------------------------------------------------------------------------------------------------------
void StereoCanvasRenderer::processStereo() {
    if(nextExpectedImage_ == NORMAL){
        nextExpectedImage_ = LEFT_1;
        if (cameraProp_.setStereoEyeMode(tgt::Camera::EYE_LEFT))
            cameraProp_.invalidate();
    }
    else{
        switch(nextExpectedImage_){
        case LEFT_1:
            copyIntoCanvas();
            copyIntoPort(&inport_,&tempPort_);
            lastRunWasInInteractionMode_ = interactionMode();
            nextExpectedImage_ = RIGHT_1; 
            if (cameraProp_.setStereoEyeMode(tgt::Camera::EYE_RIGHT))
                cameraProp_.invalidate();
            w8ingOn2Eye_ = true;
            break;
        case RIGHT_1:
            if(interactionMode() == lastRunWasInInteractionMode_){
                copyIntoStorage(&tempPort_,&inport_);
                copyIntoCanvas();
                w8ingOn2Eye_ = false;
                nextExpectedImage_ = RIGHT_2;
            }
            else { //interactionMode toggeled
                copyIntoPort(&inport_,&tempPort_);
                lastRunWasInInteractionMode_ = interactionMode();
                nextExpectedImage_ = LEFT_2;
                copyIntoCanvas();
                if(cameraProp_.setStereoEyeMode(tgt::Camera::EYE_LEFT))
                    cameraProp_.invalidate();
                w8ingOn2Eye_ = true;
            }
            break;
        case RIGHT_2:
            copyIntoCanvas();
            copyIntoPort(&inport_,&tempPort_);
            lastRunWasInInteractionMode_ = interactionMode();
            nextExpectedImage_ = LEFT_2;
            if(cameraProp_.setStereoEyeMode(tgt::Camera::EYE_LEFT))
                cameraProp_.invalidate();
            w8ingOn2Eye_ = true;
            break;
        case LEFT_2:
            if (interactionMode() == lastRunWasInInteractionMode_) {
                copyIntoStorage(&inport_,&tempPort_);
                copyIntoCanvas();
                nextExpectedImage_ = LEFT_1; 
                w8ingOn2Eye_ = false;
            } 
            else { //interactionMode toggeled
                copyIntoPort(&inport_,&tempPort_);
                lastRunWasInInteractionMode_ = interactionMode();
                nextExpectedImage_ = RIGHT_1;
                copyIntoCanvas();
                if(cameraProp_.setStereoEyeMode(tgt::Camera::EYE_RIGHT))
                    cameraProp_.invalidate();
                w8ingOn2Eye_ = true;
            }
            break;
        default:
            LERROR("StereoCanvas: Unknown NextExpectedImageMode!!!");
            break;
        }
    }
}

void StereoCanvasRenderer::copyIntoCanvas(){
    //copied from CanvasRenderer
    canvas_->getGLFocus();
    glViewport(0, 0, canvas_->getSize().x, canvas_->getSize().y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //switch based on StereoMode
    switch(stereoModeProp_.getValue()) {
    case NO_STEREO_MODE:
        //shouldn't get here
        tgtAssert(false,"copyIntoCanvas does not support NO_STEREO_MODE!!!");
        break;
    case AUTOSTEREOSCOPIC_STEREO_MODE:
    case ANAGLYPH_STEREO_MODE:
    case SPLITSCREEN_STEREO_MODE:
        // activate shader
        shader_->activate();
        // set common uniforms
        setGlobalShaderParameters(shader_);
        // manually pass the viewport dimensions to the shader,
        // since setGlobalShaderParameters() expects a render outport, which we do not have    
        shader_->setIgnoreUniformLocationError(true);
        shader_->setUniform("screenDim_", static_cast<tgt::vec2>(canvas_->getSize()));
        shader_->setUniform("screenDimRCP_", 1.f / static_cast<tgt::vec2>(canvas_->getSize()));
        shader_->setIgnoreUniformLocationError(false);
        // pass texture parameters to the shader 
        // bind input textures
        storagePort_.bindTextures(GL_TEXTURE0, GL_TEXTURE1);
        shader_->setUniform("colorTex_", 0);
        shader_->setUniform("depthTex_", 1);
        storagePort_.setTextureParameters(shader_, "texParams_");
        LGL_ERROR;
        renderQuad();
        shader_->deactivate();
        break;
    case QUADBUFFER_STEREO_MODE:
        //TODO
        LERROR("QUADBUFFER_STEREO_MODE not implemented yet!");
        break;
    default:
        //shouldn't get here
        tgtAssert(false,"Unknown StereoMode!!!");
        break;
    }

    glActiveTexture(GL_TEXTURE0); //default voreen settings
    LGL_ERROR; 
}

void StereoCanvasRenderer::copyIntoPort(RenderPort* input, RenderPort* output){
    output->activateTarget(); 
        // activate shader
        shader_->activate();

        // set common uniforms
        setGlobalShaderParameters(shader_);

        // manually pass the viewport dimensions to the shader,
        // since setGlobalShaderParameters() expects a render outport, which we do not have
        shader_->setIgnoreUniformLocationError(true);
        shader_->setUniform("screenDim_", output->getSize());
        shader_->setUniform("screenDimRCP_", 1.f / static_cast<tgt::vec2>(output->getSize()));
        shader_->setIgnoreUniformLocationError(false);

        // bind input textures
        input->bindTextures(GL_TEXTURE0, GL_TEXTURE1);

        // pass texture parameters to the shader
        shader_->setUniform("colorTex_", 0);
        shader_->setUniform("depthTex_", 1);
        input->setTextureParameters(shader_, "texParams_");
        LGL_ERROR;

        // execute the shader
        renderQuad();
        shader_->deactivate();
    
    output->deactivateTarget(); 

    glActiveTexture(GL_TEXTURE0); //default voreen settings 
    LGL_ERROR;    
}

void StereoCanvasRenderer::copyIntoStorage(tgt::Texture* left, tgt::Texture* right){
    //invert eye textures
    if (eyeInvertProp_.get()){
        tgt::Texture* help = left;
        left = right;
        right = help;
    }

    storagePort_.activateTarget();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    tgt::Shader* modeShader;

    switch(stereoModeProp_.getValue()) {
    case NO_STEREO_MODE:
        //shouldn't get here
        tgtAssert(false,"NO_STEREO_MODE is not supported!!!");
        break;
    case AUTOSTEREOSCOPIC_STEREO_MODE:
        modeShader = autostereoscopicShader_;
        break;
    case ANAGLYPH_STEREO_MODE:
        modeShader = anaglyphShader_;
        break;
    case QUADBUFFER_STEREO_MODE:
    case SPLITSCREEN_STEREO_MODE:
        modeShader = splitScreenShader_;
        break;
    default:
        //shouldn't get here
        tgtAssert(false,"Unknown StereoMode!!!");
        break;
    }

    // activate shader
    modeShader->activate();

    // set common uniforms
    setGlobalShaderParameters(modeShader);

    // manually pass the viewport dimensions to the shader,
    // since setGlobalShaderParameters() expects a render outport, which we do not have  
    modeShader->setIgnoreUniformLocationError(true);
    modeShader->setUniform("screenDim_", static_cast<tgt::vec2>(tempPort_.getSize()));
    modeShader->setUniform("screenDimRCP_", 1.f / static_cast<tgt::vec2>(tempPort_.getSize()));
    modeShader->setIgnoreUniformLocationError(false);

    // pass texture parameters to the shader
    // bind input textures
    glActiveTexture(GL_TEXTURE0);
    left->bind();
    modeShader->setUniform("colorTexLeft_", 0);
    LGL_ERROR;

    glActiveTexture(GL_TEXTURE1);
    right->bind();
    modeShader->setUniform("colorTexRight_", 1);
    LGL_ERROR;

    modeShader->setUniform("useDepthTex_",false);
    if (modeShader == anaglyphShader_)
        modeShader->setUniform("colorCode_", static_cast<int>(anaglyphModeProp_.getValue()));

    storagePort_.setTextureParameters(modeShader, "texParams_");    
 
    renderQuad();

    modeShader->deactivate();       
    storagePort_.deactivateTarget();
    storagePort_.validateResult();

    glActiveTexture(GL_TEXTURE0); //default voreen settings
    LGL_ERROR;
}

void StereoCanvasRenderer::copyIntoStorage(RenderPort* left, RenderPort* right){
    //invert eye textures
    if(eyeInvertProp_.get()){
        RenderPort* help = left;
        left = right;
        right = help;
    }

    storagePort_.activateTarget();

    tgt::Shader* modeShader;

    switch(stereoModeProp_.getValue()) {
    case NO_STEREO_MODE:
        //shouldn't get here
        tgtAssert(false,"NO_STEREO_MODE is not supported!!!");
        break;
    case AUTOSTEREOSCOPIC_STEREO_MODE:
        modeShader = autostereoscopicShader_;
        break;
    case ANAGLYPH_STEREO_MODE:
        modeShader = anaglyphShader_;
        break;
    case QUADBUFFER_STEREO_MODE:
    case SPLITSCREEN_STEREO_MODE:
        modeShader = splitScreenShader_;
        break;
    default:
        //shouldn't get here
        tgtAssert(false,"Unknown StereoMode!!!");
        break;
    }

    // activate shader
    modeShader->activate();

    // set common uniforms
    setGlobalShaderParameters(modeShader);

    // manually pass the viewport dimensions to the shader,
    // since setGlobalShaderParameters() expects a render outport, which we do not have  
    modeShader->setIgnoreUniformLocationError(true);
    modeShader->setUniform("screenDim_", static_cast<tgt::vec2>(tempPort_.getSize()));
    modeShader->setUniform("screenDimRCP_", 1.f / static_cast<tgt::vec2>(tempPort_.getSize()));
    modeShader->setIgnoreUniformLocationError(false);

    // pass texture parameters to the shader
    // bind input textures
    left->bindTextures(GL_TEXTURE0, GL_TEXTURE1);
    modeShader->setUniform("colorTexLeft_", 0);
    modeShader->setUniform("depthTexLeft_", 1);
    LGL_ERROR;
   
    right->bindTextures(GL_TEXTURE2, GL_TEXTURE3);
    modeShader->setUniform("colorTexRight_", 2);
    modeShader->setUniform("depthTexRight_", 3);
    LGL_ERROR;

    modeShader->setUniform("useDepthTex_",true);
    if(modeShader == anaglyphShader_)
        modeShader->setUniform("colorCode_", static_cast<int>(anaglyphModeProp_.getValue()));

    right->setTextureParameters(modeShader, "texParams_");

    renderQuad();

    modeShader->deactivate();
    storagePort_.deactivateTarget();
    storagePort_.validateResult();

    glActiveTexture(GL_TEXTURE0); //default voreen settings
    LGL_ERROR;
}

#ifdef VRN_MODULE_HEADTRACKING
//------------------------------------------------------------------------------------------------------------------
//      head tracking functions                                                                                     
//------------------------------------------------------------------------------------------------------------------
void StereoCanvasRenderer::getTrackingUpdate(){
    if(!w8ingOn2Eye_ && coPort_.isConnected()){
        TrackingProcessorBase* tpb = dynamic_cast<TrackingProcessorBase*>(coPort_.getConnectedProcessor());
        if(tpb)
            tpb->updateTracker();
    }
}
#endif

} // namespace voreen
