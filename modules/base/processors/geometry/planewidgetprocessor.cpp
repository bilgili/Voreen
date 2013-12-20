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

#include "planewidgetprocessor.h"

#include "tgt/glmath.h"

using tgt::vec3;

namespace voreen {

PlaneWidgetProcessor::PlaneWidgetProcessor()
    : GeometryRendererBase()
    , grabbed_(-1)
    , syncMovement_(false)
    , arrowDisplayList_(0)
    , sphereDisplayList_(0)
    , enable_("enable", "Enable", true)
    , showHandles_("showHandles", "Show Handles", true)
    , manipulatorScaling_("manipulatorScaling", "Manipulator scaling", 1.0f, 0.1f, 10.0f)
    , xColor_("xColor", "x Color", tgt::vec4(1.0f, 0.0f, 0.0f, 1.0f))
    , yColor_("yColor", "y Color", tgt::vec4(0.0f, 1.0f, 0.0f, 1.0f))
    , zColor_("zColor", "z Color", tgt::vec4(0.0f, 0.0f, 1.0f, 1.0f))
    , clipEnabledLeftX_("enableLeftX", "Enable left clipping plane (x)", true)
    , clipUseSphereManipulatorLeftX_("useSphereManipulatorLeftX", "Use sphere manipulator for left clipping plane (x)", false)
    , clipLeftX_("leftClipPlane", "Left clipping plane (x)", 100000, 0, 100000)
    , clipEnabledRightX_("enableRightX", "Enable right clipping plane (x)", true)
    , clipUseSphereManipulatorRightX_("useSphereManipulatorRightX", "Use sphere manipulator for right clipping plane (x)", false)
    , clipRightX_("rightClipPlane", "Right clipping plane (x)", 0, 0, 100000)
    , clipEnabledFrontY_("enableBottomY", "Enable front clipping plane (y)", true)
    , clipUseSphereManipulatorFrontY_("useSphereManipulatorFrontY", "Use sphere manipulator for front clipping plane (y)", false)
    , clipFrontY_("bottomClipPlane", "Front clipping plane (y)", 100000, 0, 100000)
    , clipEnabledBackY_("enableTopY", "Enable back clipping plane (y)", true)
    , clipUseSphereManipulatorBackY_("useSphereManipulatorBackY", "Use sphere manipulator for back clipping plane (y)", false)
    , clipBackY_("topClipPlane", "Back clipping plane (y)", 0, 0, 100000)
    , clipEnabledBottomZ_("enableFrontZ", "Enable bottom clipping plane (z)", true)
    , clipUseSphereManipulatorBottomZ_("useSphereManipulatorBottomZ", "Use sphere manipulator for bottom clipping plane (z)", false)
    , clipBottomZ_("frontClipPlane", "Bottom clipping plane (z)", 0, 0, 100000)
    , clipEnabledTopZ_("enableBackZ", "Enable top clipping plane (z)", true)
    , clipUseSphereManipulatorTopZ_("useSphereManipulatorTopZ", "Use sphere manipulator for top clipping plane (z)", false)
    , clipTopZ_("backClipPlane", "Top clipping plane (z)", 100000, 0, 100000)
    , width_("lineWidth", "Line Width", 1.0f, 1.0f, 10.0f)
    , showInnerBB_("showInnerBB", "Show inner box", false)
    , innerColor_("innerColor", "Inner box color", tgt::vec4(0.0f, 0.0f, 0.0f, 1.0f))
    , inport_(Port::INPORT, "volume", "Volume Input")
{
    innerColor_.setViews(Property::COLOR);
    xColor_.setViews(Property::COLOR);
    yColor_.setViews(Property::COLOR);
    zColor_.setViews(Property::COLOR);
    addPort(inport_);

    addProperty(enable_);

    addProperty(showHandles_);
    //enable_.setGroupID("vis");

    moveEventProp_ = new EventProperty<PlaneWidgetProcessor>(
        "mouseEvent.clipplaneMovement", "Clipplane movement", this,
        &PlaneWidgetProcessor::planeMovement,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT,
        tgt::MouseEvent::MOTION | tgt::MouseEvent::PRESSED | tgt::MouseEvent::RELEASED,
        tgt::Event::MODIFIER_NONE, false);
    addEventProperty(moveEventProp_);
    syncMoveEventProp_ = new EventProperty<PlaneWidgetProcessor>(
        "mouseEvent.synchronizedMovement", "Synchronized movement", this,
        &PlaneWidgetProcessor::planeMovementSync,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT,
        tgt::MouseEvent::MOTION | tgt::MouseEvent::PRESSED | tgt::MouseEvent::RELEASED,
        tgt::Event::SHIFT, false);
    addEventProperty(syncMoveEventProp_);

    addProperty(width_);
        //width_.setGroupID("vis");
    addProperty(manipulatorScaling_);
        //manipulatorScaling_.setGroupID("vis");
    //setPropertyGroupGuiName("vis", "Visual Settings");

    addProperty(xColor_);
        xColor_.setGroupID("x");
    addProperty(clipEnabledRightX_);
        clipEnabledRightX_.setGroupID("x");
    addProperty(clipRightX_);
        clipRightX_.setGroupID("x");
    addProperty(clipUseSphereManipulatorRightX_);
        clipUseSphereManipulatorRightX_.setGroupID("x");
    addProperty(clipEnabledLeftX_);
        clipEnabledLeftX_.setGroupID("x");
    addProperty(clipLeftX_);
        clipLeftX_.setGroupID("x");
    addProperty(clipUseSphereManipulatorLeftX_);
        clipUseSphereManipulatorLeftX_.setGroupID("x");
    setPropertyGroupGuiName("x", "X Axis Clipping");

    addProperty(yColor_);
        yColor_.setGroupID("y");
    addProperty(clipEnabledFrontY_);
        clipEnabledFrontY_.setGroupID("y");
    addProperty(clipFrontY_);
        clipFrontY_.setGroupID("y");
    addProperty(clipUseSphereManipulatorFrontY_);
        clipUseSphereManipulatorFrontY_.setGroupID("y");
    addProperty(clipEnabledBackY_);
        clipEnabledBackY_.setGroupID("y");
    addProperty(clipBackY_);
        clipBackY_.setGroupID("y");
    addProperty(clipUseSphereManipulatorBackY_);
        clipUseSphereManipulatorBackY_.setGroupID("y");
    setPropertyGroupGuiName("y", "Y Axis Clipping");

    addProperty(zColor_);
        zColor_.setGroupID("z");
    addProperty(clipEnabledBottomZ_);
        clipEnabledBottomZ_.setGroupID("z");
    addProperty(clipBottomZ_);
        clipBottomZ_.setGroupID("z");
    addProperty(clipUseSphereManipulatorBottomZ_);
        clipUseSphereManipulatorBottomZ_.setGroupID("z");
    addProperty(clipEnabledTopZ_);
        clipEnabledTopZ_.setGroupID("z");
    addProperty(clipTopZ_);
        clipTopZ_.setGroupID("z");
    addProperty(clipUseSphereManipulatorTopZ_);
        clipUseSphereManipulatorTopZ_.setGroupID("z");
    setPropertyGroupGuiName("z", "Z Axis Clipping");

    addProperty(showInnerBB_);
        showInnerBB_.setGroupID("bb");
    addProperty(innerColor_);
        innerColor_.setGroupID("bb");
    setPropertyGroupGuiName("bb", "Inner Bounding Box");

    //X
    manipulators_.push_back(Manipulator(0, 0, &clipLeftX_, &clipRightX_, true));
    manipulators_.push_back(Manipulator(0, 2, &clipLeftX_, &clipRightX_, true));
    manipulators_.push_back(Manipulator(0, 0, &clipRightX_, &clipLeftX_, false));
    manipulators_.push_back(Manipulator(0, 2, &clipRightX_, &clipLeftX_, false));

    //Y
    manipulators_.push_back(Manipulator(1, 0, &clipFrontY_, &clipBackY_, false));
    manipulators_.push_back(Manipulator(1, 2, &clipFrontY_, &clipBackY_, false));
    manipulators_.push_back(Manipulator(1, 0, &clipBackY_, &clipFrontY_, true));
    manipulators_.push_back(Manipulator(1, 2, &clipBackY_, &clipFrontY_, true));

    //Z
    manipulators_.push_back(Manipulator(2, 0, &clipBottomZ_, &clipTopZ_, false));
    manipulators_.push_back(Manipulator(2, 2, &clipBottomZ_, &clipTopZ_, false));
    manipulators_.push_back(Manipulator(2, 0, &clipTopZ_, &clipBottomZ_, true));
    manipulators_.push_back(Manipulator(2, 2, &clipTopZ_, &clipBottomZ_, true));

    ON_PROPERTY_CHANGE(clipEnabledRightX_,PlaneWidgetProcessor,propertyVisibilityOnChange);
    ON_PROPERTY_CHANGE(clipEnabledLeftX_,PlaneWidgetProcessor,propertyVisibilityOnChange);
    ON_PROPERTY_CHANGE(clipEnabledFrontY_,PlaneWidgetProcessor,propertyVisibilityOnChange);
    ON_PROPERTY_CHANGE(clipEnabledBackY_,PlaneWidgetProcessor,propertyVisibilityOnChange);
    ON_PROPERTY_CHANGE(clipEnabledTopZ_,PlaneWidgetProcessor,propertyVisibilityOnChange);
    ON_PROPERTY_CHANGE(clipEnabledBottomZ_,PlaneWidgetProcessor,propertyVisibilityOnChange);
}

PlaneWidgetProcessor::~PlaneWidgetProcessor() {
    delete moveEventProp_;
    delete syncMoveEventProp_;
}

Processor* PlaneWidgetProcessor::create() const {
    return new PlaneWidgetProcessor();
}

void PlaneWidgetProcessor::initialize() throw (tgt::Exception) {
    GeometryRendererBase::initialize();

    if (!arrowDisplayList_) {
        GLUquadricObj* quadric = gluNewQuadric();
        arrowDisplayList_ = glGenLists(1);
        MatStack.pushMatrix();
        glNewList(arrowDisplayList_, GL_COMPILE);
        MatStack.translate(0.0f,0.0f,-0.05f);
        gluCylinder(quadric, 0.025f, 0.0f, 0.05f, 30, 10);
        MatStack.translate(0.0f,0.0f,-0.035f);
        gluCylinder(quadric, 0.01f, 0.01f, 0.06f, 20, 2);
        glEndList();
        MatStack.popMatrix();
        gluDeleteQuadric(quadric);
    }
    if (!arrowDisplayList_) {
        processorState_ = PROCESSOR_STATE_NOT_INITIALIZED;
        throw VoreenException("PlaneWidgetProcessor: Failed to create arrow display list");
    }

    if (!sphereDisplayList_) {
        GLUquadricObj* quadric = gluNewQuadric();
        sphereDisplayList_ = glGenLists(1);
        glNewList(sphereDisplayList_, GL_COMPILE);
        gluQuadricOrientation(quadric, GLU_OUTSIDE);
        gluSphere(quadric, 0.025f, 12, 12);
        glEndList();
        gluDeleteQuadric(quadric);
    }
    if (!sphereDisplayList_) {
        processorState_ = PROCESSOR_STATE_NOT_INITIALIZED;
        throw VoreenException("PlaneWidgetProcessor: Failed to create sphere display list");
    }
}

void PlaneWidgetProcessor::deinitialize() throw (tgt::Exception) {

    if (arrowDisplayList_) {
        glDeleteLists(arrowDisplayList_, 1);
        arrowDisplayList_ = 0;
    }

    if (sphereDisplayList_) {
        glDeleteLists(sphereDisplayList_, 1);
        sphereDisplayList_ = 0;
    }

    GeometryRendererBase::deinitialize();
}

void PlaneWidgetProcessor::invalidate(int inv) {
    if (inport_.isReady()) {
        tgt::ivec3 numSlices = inport_.getData()->getDimensions();

        clipRightX_.setMaxValue((float)numSlices.x-1);
        if(clipRightX_.get() > clipRightX_.getMaxValue())
            clipRightX_.set(std::max(clipRightX_.get(), std::min(clipRightX_.get(), clipRightX_.getMaxValue())));

        clipLeftX_.setMaxValue((float)numSlices.x-1);
        if(clipLeftX_.get() > clipLeftX_.getMaxValue())
            clipLeftX_.set(std::max(clipLeftX_.get(), std::min(clipLeftX_.get(), clipLeftX_.getMaxValue())));

        clipFrontY_.setMaxValue((float)numSlices.y-1);
        if(clipFrontY_.get() > clipFrontY_.getMaxValue())
            clipFrontY_.set(std::max(clipFrontY_.get(), std::min(clipFrontY_.get(), clipFrontY_.getMaxValue())));

        clipBackY_.setMaxValue((float)numSlices.y-1);
        if(clipBackY_.get() > clipBackY_.getMaxValue())
            clipBackY_.set(std::max(clipBackY_.get(), std::min(clipBackY_.get(), clipBackY_.getMaxValue())));

        clipTopZ_.setMaxValue((float)numSlices.z-1);
        if(clipTopZ_.get() > clipTopZ_.getMaxValue())
            clipTopZ_.set(std::max(clipTopZ_.get(), std::min(clipTopZ_.get(), clipTopZ_.getMaxValue())));

        clipBottomZ_.setMaxValue((float)numSlices.z-1);
        if(clipBottomZ_.get() > clipBottomZ_.getMaxValue())
            clipBottomZ_.set(std::max(clipBottomZ_.get(), std::min(clipBottomZ_.get(), clipBottomZ_.getMaxValue())));
    }
    else {
        clipRightX_.setMaxValue(100000);
        clipLeftX_.setMaxValue(100000);

        clipFrontY_.setMaxValue(100000);
        clipBackY_.setMaxValue(100000);

        clipTopZ_.setMaxValue(100000);
        clipBottomZ_.setMaxValue(100000);
    }

    /*clipLeftX_.setVisible(clipEnabledLeftX_.get());
    clipUseSphereManipulatorLeftX_.setVisible(clipEnabledLeftX_.get());

    clipRightX_.setVisible(clipEnabledRightX_.get());
    clipUseSphereManipulatorRightX_.setVisible(clipEnabledRightX_.get());

    clipFrontY_.setVisible(clipEnabledFrontY_.get());
    clipUseSphereManipulatorFrontY_.setVisible(clipEnabledFrontY_.get());

    clipBackY_.setVisible(clipEnabledBackY_.get());
    clipUseSphereManipulatorBackY_.setVisible(clipEnabledBackY_.get());

    clipTopZ_.setVisible(clipEnabledTopZ_.get());
    clipUseSphereManipulatorTopZ_.setVisible(clipEnabledTopZ_.get());

    clipBottomZ_.setVisible(clipEnabledBottomZ_.get());
    clipUseSphereManipulatorBottomZ_.setVisible(clipEnabledBottomZ_.get()); */

    GeometryRendererBase::invalidate(inv);
}

void PlaneWidgetProcessor::propertyVisibilityOnChange() {
    //x settings
    clipRightX_.setVisible(clipEnabledRightX_.get());
    clipUseSphereManipulatorRightX_.setVisible(clipEnabledRightX_.get());
    clipLeftX_.setVisible(clipEnabledLeftX_.get());
    clipUseSphereManipulatorLeftX_.setVisible(clipEnabledLeftX_.get());
    //y settings
    clipFrontY_.setVisible(clipEnabledFrontY_.get());
    clipUseSphereManipulatorFrontY_.setVisible(clipEnabledFrontY_.get());
    clipBackY_.setVisible(clipEnabledBackY_.get());
    clipUseSphereManipulatorBackY_.setVisible(clipEnabledBackY_.get());
    //z settings
    clipTopZ_.setVisible(clipEnabledTopZ_.get());
    clipUseSphereManipulatorTopZ_.setVisible(clipEnabledTopZ_.get());
    clipBottomZ_.setVisible(clipEnabledBottomZ_.get());
    clipUseSphereManipulatorBottomZ_.setVisible(clipEnabledBottomZ_.get());
}

void PlaneWidgetProcessor::setIDManager(IDManager* idm) {
    if(idm == idManager_)
        return;

    idManager_ = idm;

    if(idManager_) {
        for (size_t i = 0; i < manipulators_.size(); ++i)
            idManager_->registerObject(&manipulators_[i]);
    }
}

void PlaneWidgetProcessor::planeMovement(tgt::MouseEvent* e) {
    e->ignore();
    if (!idManager_)
        return;

    if (e->action() & tgt::MouseEvent::PRESSED) {
        const void* obj = idManager_->getObjectAtPos(tgt::ivec2(e->coord().x, e->viewport().y - e->coord().y));
        for (size_t i = 0; i < manipulators_.size(); ++i) {
            if(obj == &manipulators_[i]) {
                grabbed_ = static_cast<int>(i);
                break;
            }
        }
        if(grabbed_ != -1) {
            e->accept();
            manipulators_[grabbed_].prop_->toggleInteractionMode(true, this);
            invalidate();
        }
    }

    if (e->action() & tgt::MouseEvent::MOTION) {
        if(grabbed_ != -1) {
            e->accept();

            MatStack.pushMatrix();
            MatStack.multMatrix(inport_.getData()->getPhysicalToWorldMatrix());

            //adjacent vertices
            tgt::vec3 vertex1 = getCorner(manipulators_[grabbed_].axis_, manipulators_[grabbed_].corner_, 1.0f);
            tgt::vec3 vertex2 = getCorner(manipulators_[grabbed_].axis_, manipulators_[grabbed_].corner_, 0.0f);

            //convert coordinates of both points in windowcoordinates
            tgt::mat4 viewMat = inport_.getData()->getPhysicalToWorldMatrix();
            tgt::vec3 vertex1Projected = getWindowPos(vertex1, viewMat);
            tgt::vec3 vertex2Projected = getWindowPos(vertex2, viewMat);

            //calculate projection of mouseposition to line between both vertexs
            tgt::vec2 mousePos = tgt::vec2(static_cast<float>(e->coord().x), e->viewport().y-static_cast<float>(e->coord().y));
            tgt::vec3 direction = vertex2Projected-vertex1Projected;
            float t = tgt::dot(mousePos-vertex1Projected.xy(), direction.xy()) / tgt::lengthSq(direction.xy());
            if (t < 0.f)
                t = 0.f;
            if (t > 1.f)
                t = 1.f;

            //save difference before move for synced movement:
            float diff = manipulators_[grabbed_].prop_->get() - manipulators_[grabbed_].oppositeProp_->get();

            //update property value:
            manipulators_[grabbed_].prop_->set((1.0f-t)*manipulators_[grabbed_].prop_->getMaxValue());

            if(syncMovement_) {
                float temp = manipulators_[grabbed_].prop_->get() - diff;
                if(temp < 0)
                    manipulators_[grabbed_].oppositeProp_->set(0);
                else if (temp > manipulators_[grabbed_].oppositeProp_->getMaxValue())
                    manipulators_[grabbed_].oppositeProp_->set(manipulators_[grabbed_].oppositeProp_->getMaxValue());
                else
                    manipulators_[grabbed_].oppositeProp_->set(temp);
            }

            MatStack.popMatrix();

            invalidate();
        }
    }

    if (e->action() & tgt::MouseEvent::RELEASED) {
        if(grabbed_ != -1) {
            e->accept();
            manipulators_[grabbed_].prop_->toggleInteractionMode(false, this);
            invalidate();
            grabbed_ = -1;
        }
        syncMovement_ = false;
    }
}

void PlaneWidgetProcessor::planeMovementSync(tgt::MouseEvent* e) {
    e->ignore();
    if (!idManager_)
        return;

    if (e->action() & tgt::MouseEvent::PRESSED) {
        syncMovement_ = true;
        planeMovement(e);
        if(grabbed_ == -1)
            syncMovement_ = false;
    }
    if (e->action() & tgt::MouseEvent::MOTION) {
        planeMovement(e);
    }
    if (e->action() & tgt::MouseEvent::RELEASED) {
        planeMovement(e);
        syncMovement_ = false;
    }
}

void PlaneWidgetProcessor::renderPicking() {
    if (!showHandles_.get())
        return;

    if (!idManager_)
        return;

    MatStack.pushMatrix();
    MatStack.multMatrix(inport_.getData()->getPhysicalToWorldMatrix());

    if(clipEnabledLeftX_.get()) {
        idManager_->setGLColor(&manipulators_[0]);
        paintManipulator(manipulators_[0], clipUseSphereManipulatorLeftX_.get());
        idManager_->setGLColor(&manipulators_[1]);
        paintManipulator(manipulators_[1], clipUseSphereManipulatorLeftX_.get());
    }

    if(clipEnabledRightX_.get()) {
        idManager_->setGLColor(&manipulators_[2]);
        paintManipulator(manipulators_[2], clipUseSphereManipulatorRightX_.get());
        idManager_->setGLColor(&manipulators_[3]);
        paintManipulator(manipulators_[3], clipUseSphereManipulatorRightX_.get());
    }

    if(clipEnabledFrontY_.get()) {
        idManager_->setGLColor(&manipulators_[4]);
        paintManipulator(manipulators_[4], clipUseSphereManipulatorFrontY_.get());
        idManager_->setGLColor(&manipulators_[5]);
        paintManipulator(manipulators_[5], clipUseSphereManipulatorFrontY_.get());
    }

    if(clipEnabledBackY_.get()) {
        idManager_->setGLColor(&manipulators_[6]);
        paintManipulator(manipulators_[6], clipUseSphereManipulatorBackY_.get());
        idManager_->setGLColor(&manipulators_[7]);
        paintManipulator(manipulators_[7], clipUseSphereManipulatorBackY_.get());
    }

    if(clipEnabledTopZ_.get()) {
        idManager_->setGLColor(&manipulators_[8]);
        paintManipulator(manipulators_[8], clipUseSphereManipulatorTopZ_.get());
        idManager_->setGLColor(&manipulators_[9]);
        paintManipulator(manipulators_[9], clipUseSphereManipulatorTopZ_.get());
    }

    if(clipEnabledBottomZ_.get()) {
        idManager_->setGLColor(&manipulators_[10]);
        paintManipulator(manipulators_[10], clipUseSphereManipulatorBottomZ_.get());
        idManager_->setGLColor(&manipulators_[11]);
        paintManipulator(manipulators_[11], clipUseSphereManipulatorBottomZ_.get());
    }

    MatStack.popMatrix();

    LGL_ERROR;
}

void PlaneWidgetProcessor::render() {
    if (!enable_.get() || !showHandles_.get())
        return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    MatStack.pushMatrix();
    MatStack.multMatrix(inport_.getData()->getPhysicalToWorldMatrix());

    glLineWidth(width_.get());

    if ( ((grabbed_ == 0) || (grabbed_ == 1))
      || (syncMovement_ && ((grabbed_ == 2) || (grabbed_ == 3)))) {
        float t = clipLeftX_.get() / (float) clipLeftX_.getMaxValue();
        glColor4fv(xColor_.get().elem);
        glBegin(GL_LINE_LOOP);
        glVertex3fv(getCorner(0, 0, t).elem);
        glVertex3fv(getCorner(0, 1, t).elem);
        glVertex3fv(getCorner(0, 2, t).elem);
        glVertex3fv(getCorner(0, 3, t).elem);
        glEnd();
    }
    if ( ((grabbed_ == 2) || (grabbed_ == 3))
      || (syncMovement_ && ((grabbed_ == 0) || (grabbed_ == 1))) ) {
        float t = clipRightX_.get() / (float) clipRightX_.getMaxValue();
        glColor4fv(xColor_.get().elem);
        glBegin(GL_LINE_LOOP);
        glVertex3fv(getCorner(0, 0, t).elem);
        glVertex3fv(getCorner(0, 1, t).elem);
        glVertex3fv(getCorner(0, 2, t).elem);
        glVertex3fv(getCorner(0, 3, t).elem);
        glEnd();
    }
    if ( ((grabbed_ == 4) || (grabbed_ == 5))
      || (syncMovement_ && ((grabbed_ == 6) || (grabbed_ == 7))) ){
        float t = clipFrontY_.get() / (float) clipFrontY_.getMaxValue();
        glColor4fv(yColor_.get().elem);
        glBegin(GL_LINE_LOOP);
        glVertex3fv(getCorner(1, 0, t).elem);
        glVertex3fv(getCorner(1, 1, t).elem);
        glVertex3fv(getCorner(1, 2, t).elem);
        glVertex3fv(getCorner(1, 3, t).elem);
        glEnd();
    }
    if ( ((grabbed_ == 6) || (grabbed_ == 7))
      || (syncMovement_ && ((grabbed_ == 4) || (grabbed_ == 5))) ) {
        float t = clipBackY_.get() / (float) clipBackY_.getMaxValue();
        glColor4fv(yColor_.get().elem);
        glBegin(GL_LINE_LOOP);
        glVertex3fv(getCorner(1, 0, t).elem);
        glVertex3fv(getCorner(1, 1, t).elem);
        glVertex3fv(getCorner(1, 2, t).elem);
        glVertex3fv(getCorner(1, 3, t).elem);
        glEnd();
    }
    if ( ((grabbed_ == 8) || (grabbed_ == 9))
      || (syncMovement_ && ((grabbed_ == 10) || (grabbed_ == 11))) ) {
        float t = clipBottomZ_.get() / (float) clipBottomZ_.getMaxValue();
        glColor4fv(zColor_.get().elem);
        glBegin(GL_LINE_LOOP);
        glVertex3fv(getCorner(2, 0, t).elem);
        glVertex3fv(getCorner(2, 1, t).elem);
        glVertex3fv(getCorner(2, 2, t).elem);
        glVertex3fv(getCorner(2, 3, t).elem);
        glEnd();
    }
    if ( ((grabbed_ == 10) || (grabbed_ == 11))
      || (syncMovement_ && ((grabbed_ == 8) || (grabbed_ == 9))) ) {
        float t = clipTopZ_.get() / (float) clipTopZ_.getMaxValue();
        glColor4fv(zColor_.get().elem);
        glBegin(GL_LINE_LOOP);
        glVertex3fv(getCorner(2, 0, t).elem);
        glVertex3fv(getCorner(2, 1, t).elem);
        glVertex3fv(getCorner(2, 2, t).elem);
        glVertex3fv(getCorner(2, 3, t).elem);
        glEnd();
    }

    if (showInnerBB_.get()) {
        vec3 urb = inport_.getData()->getURB();
        vec3 llf = inport_.getData()->getLLF();
        //inner boundingbox:
        float xmin = clipLeftX_.get() / (float) clipLeftX_.getMaxValue();
        xmin = (xmin * urb.x) + ((1.0f - xmin) * llf.x);
        float xmax = clipRightX_.get() / (float) clipRightX_.getMaxValue();
        xmax = (xmax * urb.x) + ((1.0f - xmax) * llf.x);
        float ymin = clipFrontY_.get() / (float) clipFrontY_.getMaxValue();
        ymin = (ymin * urb.y) + ((1.0f - ymin) * llf.y);
        float ymax = clipBackY_.get() / (float) clipBackY_.getMaxValue();
        ymax = (ymax * urb.y) + ((1.0f - ymax) * llf.y);
        float zmin = clipBottomZ_.get() / (float) clipBottomZ_.getMaxValue();
        zmin = (zmin * urb.z) + ((1.0f - zmin) * llf.z);
        float zmax = clipTopZ_.get() / (float) clipTopZ_.getMaxValue();
        zmax = (zmax * urb.z) + ((1.0f - zmax) * llf.z);

        glColor4fv(innerColor_.get().elem);
        glBegin(GL_LINES);

        //---------------------------

        glVertex3f(xmin, ymin, zmin);
        glVertex3f(xmin, ymin, zmax);

        glVertex3f(xmin, ymax, zmin);
        glVertex3f(xmin, ymax, zmax);

        glVertex3f(xmax, ymin, zmin);
        glVertex3f(xmax, ymin, zmax);

        glVertex3f(xmax, ymax, zmin);
        glVertex3f(xmax, ymax, zmax);

        //---------------------------

        glVertex3f(xmin, ymin, zmin);
        glVertex3f(xmax, ymin, zmin);

        glVertex3f(xmin, ymax, zmin);
        glVertex3f(xmax, ymax, zmin);

        glVertex3f(xmin, ymin, zmax);
        glVertex3f(xmax, ymin, zmax);

        glVertex3f(xmin, ymax, zmax);
        glVertex3f(xmax, ymax, zmax);

        //---------------------------

        glVertex3f(xmin, ymin, zmin);
        glVertex3f(xmin, ymax, zmin);

        glVertex3f(xmax, ymin, zmin);
        glVertex3f(xmax, ymax, zmin);

        glVertex3f(xmin, ymin, zmax);
        glVertex3f(xmin, ymax, zmax);

        glVertex3f(xmax, ymin, zmax);
        glVertex3f(xmax, ymax, zmax);

        //---------------------------

        glEnd();

    }
    glLineWidth(1.0f);

    //---------------------------------------------------------------

    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHT0);

    // set light pos to camera pos
    MatStack.pushMatrix();
    MatStack.loadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, tgt::vec4(0.f, 0.f, 1.f, 0.f).elem);
    MatStack.popMatrix();

    /*vec3 spec(1.0f, 1.0f, 1.0f);
    glMaterialf( GL_FRONT_AND_BACK,    GL_SHININESS,    25.0f);
    glMaterialfv(GL_FRONT_AND_BACK,    GL_SPECULAR,    spec.elem); */

    //render manipulators:
    //X
    glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        xColor_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        xColor_.get().elem);
    if(clipEnabledLeftX_.get()) {
        paintManipulator(manipulators_[0], clipUseSphereManipulatorLeftX_.get());
        paintManipulator(manipulators_[1], clipUseSphereManipulatorLeftX_.get());
    }
    if(clipEnabledRightX_.get()) {
        paintManipulator(manipulators_[2], clipUseSphereManipulatorRightX_.get());
        paintManipulator(manipulators_[3], clipUseSphereManipulatorRightX_.get());
    }

    //Y
    glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        yColor_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        yColor_.get().elem);
    if(clipEnabledFrontY_.get()) {
        paintManipulator(manipulators_[4], clipUseSphereManipulatorFrontY_.get());
        paintManipulator(manipulators_[5], clipUseSphereManipulatorFrontY_.get());
    }
    if(clipEnabledBackY_.get()) {
        paintManipulator(manipulators_[6], clipUseSphereManipulatorBackY_.get());
        paintManipulator(manipulators_[7], clipUseSphereManipulatorBackY_.get());
    }

    //Z
    glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        zColor_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        zColor_.get().elem);
    if(clipEnabledTopZ_.get()) {
        paintManipulator(manipulators_[8], clipUseSphereManipulatorTopZ_.get());
        paintManipulator(manipulators_[9], clipUseSphereManipulatorTopZ_.get());
    }
    if(clipEnabledBottomZ_.get()) {
        paintManipulator(manipulators_[10], clipUseSphereManipulatorBottomZ_.get());
        paintManipulator(manipulators_[11], clipUseSphereManipulatorBottomZ_.get());
    }

    glPopAttrib();
    MatStack.popMatrix();

    LGL_ERROR;
}

void PlaneWidgetProcessor::paintManipulator(const Manipulator& a, bool useSphereManipulator) {
    vec3 rot;
    float angle = 0.f;
    switch(a.axis_) {
        case 0: rot = vec3(0.0f, 1.0f, 0.0f);
                angle = 90.0f;
                if(a.inverted_)
                    angle = -90.0f;
                break;
        case 1: rot = vec3(1.0f, 0.0f, 0.0f);
                angle = -90.0f;
                if(a.inverted_)
                    angle = 90.0f;
                break;
        case 2: rot = vec3(1.0f, 1.0f, 0.0f);
                angle = 180.0f;
                if(!a.inverted_)
                    angle = 0.0f;
                break;
    }

    float scalingFactor = 0.3f * tgt::length(getUrb() - getLlf()) * manipulatorScaling_.get();
    paintManipulator(getCorner(a.axis_, a.corner_, a.prop_->get() / (float) a.prop_->getMaxValue()), rot, angle, scalingFactor, useSphereManipulator);
}

void PlaneWidgetProcessor::paintManipulator(tgt::vec3 translation, tgt::vec3 rotationAxis, float rotationAngle, float scaleFactor, bool useSphereManipulator)
{
    MatStack.pushMatrix();
    MatStack.translate(translation.x, translation.y, translation.z);
    MatStack.rotate(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);
    MatStack.scale(scaleFactor, scaleFactor, scaleFactor);

    if(useSphereManipulator)
        glCallList(sphereDisplayList_);
    else
        glCallList(arrowDisplayList_);

    MatStack.popMatrix();
}

tgt::vec3 PlaneWidgetProcessor::getLlf() {
    return inport_.getData()->getLLF();
}

tgt::vec3 PlaneWidgetProcessor::getUrb() {
    return inport_.getData()->getURB();
}

vec3 PlaneWidgetProcessor::getCorner(int axis, int num, float t) {
    if(axis == 0) {
        float xSlice = (t * (getUrb().x - getLlf().x)) + getLlf().x;
        switch(num) {
            case 0:
                return vec3(xSlice, getUrb().y, getUrb().z);
            case 1:
                return vec3(xSlice, getLlf().y, getUrb().z);
            case 2:
                return vec3(xSlice, getLlf().y, getLlf().z);
            case 3:
                return vec3(xSlice, getUrb().y, getLlf().z);
        }
        return vec3(0.0f);
    }
    else if(axis == 1) {
        float ySlice = (t * (getUrb().y - getLlf().y)) + getLlf().y;
        switch(num) {
            case 0:
                return vec3(getLlf().x, ySlice, getLlf().z);
            case 1:
                return vec3(getLlf().x, ySlice, getUrb().z);
            case 2:
                return vec3(getUrb().x, ySlice, getUrb().z);
            case 3:
                return vec3(getUrb().x, ySlice, getLlf().z);
        }
        return vec3(0.0f);
    }
    else if(axis == 2) {
        float zSlice = (t * (getUrb().z - getLlf().z)) + getLlf().z;
        switch(num) {
            case 0:
                return vec3(getLlf().x, getUrb().y, zSlice);
            case 1:
                return vec3(getLlf().x, getLlf().y, zSlice);
            case 2:
                return vec3(getUrb().x, getLlf().y, zSlice);
            case 3:
                return vec3(getUrb().x, getUrb().y, zSlice);
        }
        return vec3(0.0f);
    }
    return vec3(0.0f);
}


} //namespace voreen
