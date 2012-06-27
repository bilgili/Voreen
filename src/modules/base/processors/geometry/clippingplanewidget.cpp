/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/modules/base/processors/geometry/clippingplanewidget.h"

#include "tgt/glmath.h"

using tgt::vec3;

namespace voreen {

ClippingPlaneWidget::ClippingPlaneWidget()
    : GeometryRendererBase()
    , grabbed_(-1)
    , syncMovement_(false)
    , arrowDisplayList_(0)
    , applyDatasetTransformationMatrix_("applyDatasetTrafoMatrix", "Apply data set trafo matrix", true)
    , xColor_("xColor", "x Color", tgt::vec4(1.0f, 0.0f, 0.0f, 1.0f))
    , yColor_("yColor", "y Color", tgt::vec4(0.0f, 1.0f, 0.0f, 1.0f))
    , zColor_("zColor", "z Color", tgt::vec4(0.0f, 0.0f, 1.0f, 1.0f))
    , clipLeftX_("leftClipPlane", "Left clipping plane (x)", 0, 0, 100000, true)
    , clipRightX_("rightClipPlane", "Right clipping plane (x)", 100000, 0, 100000, true)
    , clipFrontY_("bottomClipPlane", "Front clipping plane (y)", 100000, 0, 100000, true)
    , clipBackY_("topClipPlane", "Back clipping plane (y)", 0, 0, 100000, true)
    , clipBottomZ_("frontClipPlane", "Bottom clipping plane (z)", 0, 0, 100000, true)
    , clipTopZ_("backClipPlane", "Top clipping plane (z)", 100000, 0, 100000, true)
    , width_("lineWidth", "Line Width", 1.0f, 1.0f, 10.0f, true)
    , showInnerBB_("showInnerBB", "Show inner box", false)
    , innerColor_("innerColor", "Inner box color", tgt::vec4(0.0f, 0.0f, 0.0f, 1.0f))
    , lightPosition_("lightPosition", "Light source position", tgt::vec4(2.3f, 1.5f, 1.5f, 1.f),
        tgt::vec4(-10), tgt::vec4(10))
    , inport_(Port::INPORT, "volume")
{

    addPort(inport_);

    moveEventProp_ = new EventProperty<ClippingPlaneWidget>(
        "mouseEvent.clipplaneMovement", "Clipplane movement", this,
        &ClippingPlaneWidget::planeMovement,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT,
        tgt::MouseEvent::MOTION | tgt::MouseEvent::PRESSED | tgt::MouseEvent::RELEASED,
        tgt::Event::MODIFIER_NONE, false);
    addEventProperty(moveEventProp_);
    syncMoveEventProp_ = new EventProperty<ClippingPlaneWidget>(
        "mouseEvent.synchronizedMovement", "Synchronized movement", this,
        &ClippingPlaneWidget::planeMovementSync,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT,
        tgt::MouseEvent::MOTION | tgt::MouseEvent::PRESSED | tgt::MouseEvent::RELEASED,
        tgt::Event::SHIFT, false);
    addEventProperty(syncMoveEventProp_);

    addProperty(applyDatasetTransformationMatrix_);
    addProperty(width_);

    addProperty(clipRightX_);
    addProperty(clipLeftX_);
    addProperty(xColor_);

    addProperty(clipFrontY_);
    addProperty(clipBackY_);
    addProperty(yColor_);

    addProperty(clipBottomZ_);
    addProperty(clipTopZ_);
    addProperty(zColor_);

    addProperty(showInnerBB_);
    addProperty(innerColor_);

    addProperty(lightPosition_);


    //X
    arrows_.push_back(Arrow(0, 0, &clipLeftX_, &clipRightX_, true));
    arrows_.push_back(Arrow(0, 2, &clipLeftX_, &clipRightX_, true));
    arrows_.push_back(Arrow(0, 0, &clipRightX_, &clipLeftX_, false));
    arrows_.push_back(Arrow(0, 2, &clipRightX_, &clipLeftX_, false));

    //Y
    arrows_.push_back(Arrow(1, 0, &clipFrontY_, &clipBackY_, false));
    arrows_.push_back(Arrow(1, 2, &clipFrontY_, &clipBackY_, false));
    arrows_.push_back(Arrow(1, 0, &clipBackY_, &clipFrontY_, true));
    arrows_.push_back(Arrow(1, 2, &clipBackY_, &clipFrontY_, true));

    //Z
    arrows_.push_back(Arrow(2, 0, &clipBottomZ_, &clipTopZ_, false));
    arrows_.push_back(Arrow(2, 2, &clipBottomZ_, &clipTopZ_, false));
    arrows_.push_back(Arrow(2, 0, &clipTopZ_, &clipBottomZ_, true));
    arrows_.push_back(Arrow(2, 2, &clipTopZ_, &clipBottomZ_, true));
}

ClippingPlaneWidget::~ClippingPlaneWidget() {
    delete moveEventProp_;
    delete syncMoveEventProp_;
    if (arrowDisplayList_)
        glDeleteLists(arrowDisplayList_, 1);
}

Processor* ClippingPlaneWidget::create() const {
    return new ClippingPlaneWidget();
}

void ClippingPlaneWidget::initialize() throw (VoreenException) {
    GeometryRendererBase::initialize();
    if (!arrowDisplayList_) {
        GLUquadricObj* quadric = gluNewQuadric();
        arrowDisplayList_ = glGenLists(1);
        glNewList(arrowDisplayList_, GL_COMPILE);
        glTranslatef(0.0f,0.0f,-0.05f);
        gluCylinder(quadric, 0.025f, 0.0f, 0.05f, 30, 10);
        glTranslatef(0.0f,0.0f,-0.035f);
        gluCylinder(quadric, 0.01f, 0.01f, 0.06f, 20, 2);
        glEndList();
        gluDeleteQuadric(quadric);
    }
    if (!arrowDisplayList_) {
        initialized_ = false;
        throw VoreenException("ClippingPlaneWidget: Failed to create display list");
    }
}

std::string ClippingPlaneWidget::getProcessorInfo() const {
    return "3D control elements to manipulate axis-aligned clipping planes. Use linking to connect the properties to a CubeProxyGeometry."
           "<p>See MeshClipping and MeshClippingWidget for clipping against an arbitrarily oriented plane.</p>";
}

void ClippingPlaneWidget::invalidate(int inv) {
    if (inport_.isReady()) {
        tgt::ivec3 numSlices = inport_.getData()->getVolume()->getDimensions();

        clipRightX_.setMaxValue(numSlices.x-1);
        clipLeftX_.setMaxValue(numSlices.x-1);

        clipFrontY_.setMaxValue(numSlices.y-1);
        clipBackY_.setMaxValue(numSlices.y-1);

        clipTopZ_.setMaxValue(numSlices.z-1);
        clipBottomZ_.setMaxValue(numSlices.z-1);
    }
    else {
        clipRightX_.setMaxValue(100000);
        clipLeftX_.setMaxValue(100000);

        clipFrontY_.setMaxValue(100000);
        clipBackY_.setMaxValue(100000);

        clipTopZ_.setMaxValue(100000);
        clipBottomZ_.setMaxValue(100000);
    }
    GeometryRendererBase::invalidate(inv);
}

void ClippingPlaneWidget::setIDManager(IDManager* idm) {
    if (idManager_ == idm)
        return;

    idManager_ = idm;
    if (idManager_) {
        for (size_t i = 0; i < arrows_.size(); ++i)
            idm->registerObject(&arrows_[i]);
    }
}

void ClippingPlaneWidget::planeMovement(tgt::MouseEvent* e) {
    e->ignore();
    if (!idManager_)
        return;

    if (e->action() & tgt::MouseEvent::PRESSED) {
        void* obj = idManager_->getObjectAtPos(tgt::ivec2(e->coord().x, e->viewport().y - e->coord().y));
        for (size_t i = 0; i < arrows_.size(); ++i) {
            if(obj == &arrows_[i]) {
                grabbed_ = i;
                break;
            }
        }
        if(grabbed_ != -1) {
            e->accept();
            arrows_[grabbed_].prop_->toggleInteractionMode(true, this);
            invalidate();
        }
    }

    if (e->action() & tgt::MouseEvent::MOTION) {
        if(grabbed_ != -1) {
            e->accept();

            if (applyDatasetTransformationMatrix_.get()) {
                glPushMatrix();
                tgt::multMatrix(inport_.getData()->getVolume()->getTransformation());
            }

            //adjacent vertices
            tgt::vec3 vertex1 = getCorner(arrows_[grabbed_].axis_, arrows_[grabbed_].corner_, 1.0f);
            tgt::vec3 vertex2 = getCorner(arrows_[grabbed_].axis_, arrows_[grabbed_].corner_, 0.0f);

            //convert coordinates of both points in windowcoordinates
            tgt::mat4 viewMat = applyDatasetTransformationMatrix_.get() ? inport_.getData()->getVolume()->getTransformation() : tgt::mat4::identity;
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
            tgt::vec3 pOnLine = vertex1Projected+t*direction;

            //save difference before move for synced movement:
            int diff = arrows_[grabbed_].prop_->get() - arrows_[grabbed_].oppositeProp_->get();

            //update property value:
            arrows_[grabbed_].prop_->set(static_cast<int>((1.0f-t)*arrows_[grabbed_].prop_->getMaxValue()));

            if(syncMovement_) {
                int temp = arrows_[grabbed_].prop_->get() - diff;
                if(temp < 0)
                    arrows_[grabbed_].oppositeProp_->set(0);
                else if (temp > arrows_[grabbed_].oppositeProp_->getMaxValue())
                    arrows_[grabbed_].oppositeProp_->set(arrows_[grabbed_].oppositeProp_->getMaxValue());
                else
                    arrows_[grabbed_].oppositeProp_->set(temp);
            }

            if (applyDatasetTransformationMatrix_.get()) {
                glPopMatrix();
            }

            invalidate();
        }
    }

    if (e->action() & tgt::MouseEvent::RELEASED) {
        if(grabbed_ != -1) {
            e->accept();
            arrows_[grabbed_].prop_->toggleInteractionMode(false, this);
            invalidate();
            grabbed_ = -1;
        }
        syncMovement_ = false;
    }
}

void ClippingPlaneWidget::planeMovementSync(tgt::MouseEvent* e) {
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

void ClippingPlaneWidget::renderPicking() {
    if (!idManager_)
        return;

    if (applyDatasetTransformationMatrix_.get()) {
        glPushMatrix();
        tgt::multMatrix(inport_.getData()->getVolume()->getTransformation());
    }

    for (size_t i = 0; i < arrows_.size(); ++i) {
        idManager_->setGLColor(&arrows_[i]);
        paintArrow(arrows_[i]);
    }

    if (applyDatasetTransformationMatrix_.get())
        glPopMatrix();

    LGL_ERROR;
}

void ClippingPlaneWidget::render() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if (applyDatasetTransformationMatrix_.get()) {
        glPushMatrix();
        tgt::multMatrix(inport_.getData()->getVolume()->getTransformation());
    }

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
        vec3 urb = inport_.getData()->getVolume()->getURB();
        vec3 llf = inport_.getData()->getVolume()->getLLF();
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
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition_.get().elem);
    glEnable(GL_LIGHT0);

    vec3 spec(1.0f, 1.0f, 1.0f);
    glMaterialf( GL_FRONT_AND_BACK,    GL_SHININESS,    25.0f);
    glMaterialfv(GL_FRONT_AND_BACK,    GL_SPECULAR,    spec.elem);

    //render arrows:
    //X
    glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        xColor_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        xColor_.get().elem);
    paintArrow(arrows_[0]);
    paintArrow(arrows_[1]);
    paintArrow(arrows_[2]);
    paintArrow(arrows_[3]);

    //Y
    glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        yColor_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        yColor_.get().elem);
    paintArrow(arrows_[4]);
    paintArrow(arrows_[5]);
    paintArrow(arrows_[6]);
    paintArrow(arrows_[7]);

    //Z
    glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        zColor_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        zColor_.get().elem);
    paintArrow(arrows_[8]);
    paintArrow(arrows_[9]);
    paintArrow(arrows_[10]);
    paintArrow(arrows_[11]);

    glPopAttrib();

    if (applyDatasetTransformationMatrix_.get())
        glPopMatrix();

    LGL_ERROR;
}

void ClippingPlaneWidget::paintArrow(const Arrow& a) {
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

    paintArrow(getCorner(a.axis_, a.corner_, a.prop_->get() / (float) a.prop_->getMaxValue()), rot, angle);
}

void ClippingPlaneWidget::paintArrow(tgt::vec3 translation, tgt::vec3 rotationAxis, float rotationAngle)
{
    glPushMatrix();
    glTranslatef(translation.x, translation.y, translation.z);
    glRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

    glCallList(arrowDisplayList_);
    glPopMatrix();
}

tgt::vec3 ClippingPlaneWidget::getLlf() {
    return inport_.getData()->getVolume()->getCubeSize() / -2.f;
}

tgt::vec3 ClippingPlaneWidget::getUrb() {
    return inport_.getData()->getVolume()->getCubeSize() / 2.f;
}

vec3 ClippingPlaneWidget::getCorner(int axis, int num, float t) {
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
