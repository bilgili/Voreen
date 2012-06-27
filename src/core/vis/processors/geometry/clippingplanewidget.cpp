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

#include "voreen/core/vis/processors/geometry/clippingplanewidget.h"

using tgt::vec3;

namespace voreen {

ClippingPlaneWidget::ClippingPlaneWidget()
    : GeometryRenderer()
    , grabbed_(-1)
    , arrowDisplayList_(0)
    , lightPosition_("lightPosition", "Light source position", tgt::vec4(2.3f, 1.5f, 1.5f, 1.f),
                     tgt::vec4(-10), tgt::vec4(10))
    , xColor_("xColor", "x Color", tgt::vec4(1.0f, 0.0f, 0.0f, 1.0f))
    , yColor_("yColor", "y Color", tgt::vec4(0.0f, 1.0f, 0.0f, 1.0f))
    , zColor_("zColor", "z Color", tgt::vec4(0.0f, 0.0f, 1.0f, 1.0f))
    , clipLeftX_("leftClipPlane", "Left clipping plane", 0, 0, 100000, true)
    , clipRightX_("rightClipPlane", "Right clipping plane", 100000, 0, 100000, true)
    , clipUpY_("topClipPlane", "Top clipping plane", 0, 0, 100000, true)
    , clipDownY_("bottomClipPlane", "Bottom clipping plane", 100000, 0, 100000, true)
    , clipFrontZ_("frontClipPlane", "Front clipping plane", 0, 0, 100000, true)
    , clipBackZ_("backClipPlane", "Back clipping plane", 100000, 0, 100000, true)
    , width_("lineWidth", "Line Width", 1.0f, 1.0f, 10.0f, true)
    , inport_(Port::INPORT, "volume")
{

    moveEventProp_ = new TemplateMouseEventProperty<ClippingPlaneWidget>("Clipplane movement", new EventAction<ClippingPlaneWidget, tgt::MouseEvent>(this, &ClippingPlaneWidget::planeMovement), tgt::MouseEvent::MOTION | tgt::MouseEvent::PRESSED | tgt::MouseEvent::RELEASED, tgt::Event::NONE, tgt::MouseEvent::MOUSE_BUTTON_LEFT);
    addEventProperty(moveEventProp_);
    syncMoveEventProp_ = new TemplateMouseEventProperty<ClippingPlaneWidget>("Syncronized movement", new EventAction<ClippingPlaneWidget, tgt::MouseEvent>(this, &ClippingPlaneWidget::planeMovementSync), tgt::MouseEvent::MOTION | tgt::MouseEvent::PRESSED | tgt::MouseEvent::RELEASED, tgt::Event::SHIFT, tgt::MouseEvent::MOUSE_BUTTON_LEFT);
    addEventProperty(syncMoveEventProp_);

    addProperty(width_);

    addProperty(clipLeftX_);
    addProperty(clipRightX_);
    addProperty(xColor_);

    addProperty(clipDownY_);
    addProperty(clipUpY_);
    addProperty(yColor_);

    addProperty(clipFrontZ_);
    addProperty(clipBackZ_);
    addProperty(zColor_);

    addProperty(lightPosition_);

    addPort(inport_);

    //X
    arrows_.push_back(Arrow(0, 0, &clipLeftX_, &clipRightX_, false));
    arrows_.push_back(Arrow(0, 2, &clipLeftX_, &clipRightX_, false));
    arrows_.push_back(Arrow(0, 0, &clipRightX_, &clipLeftX_, true));
    arrows_.push_back(Arrow(0, 2, &clipRightX_, &clipLeftX_, true));

    //Y
    arrows_.push_back(Arrow(1, 0, &clipDownY_, &clipUpY_, false));
    arrows_.push_back(Arrow(1, 2, &clipDownY_, &clipUpY_, false));
    arrows_.push_back(Arrow(1, 0, &clipUpY_, &clipDownY_, true));
    arrows_.push_back(Arrow(1, 2, &clipUpY_, &clipDownY_, true));

    //Z
    arrows_.push_back(Arrow(2, 0, &clipFrontZ_, &clipBackZ_, false));
    arrows_.push_back(Arrow(2, 2, &clipFrontZ_, &clipBackZ_, false));
    arrows_.push_back(Arrow(2, 0, &clipBackZ_, &clipFrontZ_, true));
    arrows_.push_back(Arrow(2, 2, &clipBackZ_, &clipFrontZ_, true));
}

ClippingPlaneWidget::~ClippingPlaneWidget() {
    if (arrowDisplayList_)
        glDeleteLists(arrowDisplayList_, 1);
}

void ClippingPlaneWidget::initialize() throw (VoreenException) {
    GeometryRenderer::initialize();
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
    if (!arrowDisplayList_) 
        initialized_ = false;
    //TODO: throw exception
}

const std::string ClippingPlaneWidget::getProcessorInfo() const {
    return "3D-ControlElements to manipulate the clipping planes. Use linking to connect the properties to a CubeProxyGeometry.";
}

void ClippingPlaneWidget::invalidate(InvalidationLevel inv) {
	if(inport_.isReady()) {
		tgt::ivec3 numSlices = inport_.getData()->getVolume()->getDimensions();

		clipRightX_.setMaxValue(numSlices.x);
		clipLeftX_.setMaxValue(numSlices.x);

		clipDownY_.setMaxValue(numSlices.y);
		clipUpY_.setMaxValue(numSlices.y);

		clipBackZ_.setMaxValue(numSlices.z);
		clipFrontZ_.setMaxValue(numSlices.z);
	}
	else {
		clipRightX_.setMaxValue(100000);
		clipLeftX_.setMaxValue(100000);

		clipDownY_.setMaxValue(100000);
		clipUpY_.setMaxValue(100000);

		clipBackZ_.setMaxValue(100000);
		clipFrontZ_.setMaxValue(100000);
	}
	GeometryRenderer::invalidate(inv);
}

void ClippingPlaneWidget::setIDManager(IDManager* idm) {
    if(idm_ == idm)
        return;

    idm_ = idm;
    if(idm_) {
        for (size_t i = 0; i < arrows_.size(); ++i)
            idm->registerObject(&arrows_[i]);
    }
}

void ClippingPlaneWidget::planeMovement(tgt::MouseEvent* e) {
    e->ignore();
    if (e->action() & tgt::MouseEvent::PRESSED) {
        void* obj = idm_->getObjectAtPos(tgt::ivec2(e->coord().x, e->viewport().y - e->coord().y));
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
            //adjacent vertices
            tgt::vec3 vertex1 = getCorner(arrows_[grabbed_].axis_, arrows_[grabbed_].corner_, 1.0f);
            tgt::vec3 vertex2 = getCorner(arrows_[grabbed_].axis_, arrows_[grabbed_].corner_, 0.0f);

            //convert coordinates of both points in windowcoordinates
            tgt::vec3 vertex1Projected = getWindowPos(vertex1);
            tgt::vec3 vertex2Projected = getWindowPos(vertex2);

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
    if(!idm_)
        return;

    for (size_t i = 0; i < arrows_.size(); ++i) {
        idm_->setGLColor(&arrows_[i]);
        paintArrow(arrows_[i]);
    }
}

void ClippingPlaneWidget::render() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
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
        float t = clipDownY_.get() / (float) clipDownY_.getMaxValue();
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
        float t = clipUpY_.get() / (float) clipUpY_.getMaxValue();
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
        float t = clipFrontZ_.get() / (float) clipFrontZ_.getMaxValue();
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
        float t = clipBackZ_.get() / (float) clipBackZ_.getMaxValue();
        glColor4fv(zColor_.get().elem);
        glBegin(GL_LINE_LOOP);
        glVertex3fv(getCorner(2, 0, t).elem);
        glVertex3fv(getCorner(2, 1, t).elem);
        glVertex3fv(getCorner(2, 2, t).elem);
        glVertex3fv(getCorner(2, 3, t).elem);
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
