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

#include "planemanipulation.h"

namespace voreen {

const float PlaneManipulation::EPS(0.000001f);
const tgt::vec4 PlaneManipulation::lightPosition_(2.3f, 1.5f, 1.5f, 1.f);

const std::string PlaneManipulation::loggerCat_("voreen.base.PlaneManipulation");

PlaneManipulation::PlaneManipulation()
    : GeometryRendererBase()
    , volumeInport_(Port::INPORT, "volume", "Volume Input")
    , geometryInport_(Port::INPORT, "geometry", "Geometry Input")
    , enable_("enable", "Enable", true)
    , renderPlane_("renderGeometry", "Show Plane", true)
    , renderManipulator_("renderManipulator", "Show Handle", true)
    , planeNormal_("planeNormal", "Plane Normal", tgt::vec3(0, 1, 0), tgt::vec3(-1), tgt::vec3(1))
    , planePosition_("planePosition", "Plane Position", 0.0f, -FLT_MAX, FLT_MAX, Processor::INVALID_RESULT, NumericProperty<float>::STATIC)
    , resetManipulatorPos_("resetmanipulatorpos", "Reset Handle on Plane")
    , resetPlane_("resetplane", "Reset Plane")
    , autoReset_("autoreset", "Auto Reset Handle", false, Processor::VALID)
    , invert_("invert", "Invert", false)
    , planeColor_("planeColor", "Plane Color", tgt::vec4(0.0f, 0.0f, 1.0f, 1.f))
    , grabbedElementColor_("grabbedElementColor", "Grabbed Handle Element Color", tgt::vec4(0.9f, 0.9f, 0.9f, 1.0f))
    , blockedElementColor_("blockedElementColor", "Blocked Handle Element Color", tgt::vec4(0.75f, 0.05f, 0.05f, 1.0f))
    , grabbedPlaneOpacity_("grabbedPlaneOpacity", "Grabbed Handle Opacity", 0.2f, 0.f, 1.f)
    , width_("lineWidth", "Line Width", 2.0f, 1.0f, 10.0f)
    , actualPlane_(-tgt::vec4(planeNormal_.get(), planePosition_.get()))
    , manipulatorScale_("ManipulatorScale","Handle Scale",1.0f,0.01f,1.0f)
    , manipulatorTopTipIsGrabbed_(false)
    , manipulatorBottomTipIsGrabbed_(false)
    , manipulatorCylinderIsGrabbed_(false)
    , wholeManipulatorIsGrabbed_(false)
    , invalidMovement_(false)
    , positionRange_("positionrange", "Position Range", tgt::vec2(-1.f, 1.f), tgt::vec2(-FLT_MAX), tgt::vec2(FLT_MAX), Processor::VALID)
    , manipulatorLength_(1.f)
    , manipulatorDiameter_(0.04f)
    , manipulatorTipRadius_(0.05f)
{
    addPort(volumeInport_);
    addPort(geometryInport_);

    planeColor_.setViews(Property::COLOR);
    grabbedElementColor_.setViews(Property::COLOR);
    blockedElementColor_.setViews(Property::COLOR);

    moveEventProp_ = new EventProperty<PlaneManipulation>(
        "mouseEvent.clipplaneManipulation", "Clipplane manipulation", this,
        &PlaneManipulation::planeManipulation,
        tgt::MouseEvent::MOUSE_BUTTON_ALL,
        tgt::MouseEvent::MOTION | tgt::MouseEvent::PRESSED | tgt::MouseEvent::RELEASED,
        tgt::Event::MODIFIER_NONE, false);
    addEventProperty(moveEventProp_);

    addProperty(enable_);

    addProperty(renderPlane_);
    addProperty(renderManipulator_);

    addProperty(invert_);

    addProperty(resetManipulatorPos_);
    addProperty(autoReset_);

    addProperty(resetPlane_);

    addProperty(planeNormal_);
    addProperty(planePosition_);
    addProperty(planeColor_);
    addProperty(grabbedElementColor_);
    addProperty(grabbedPlaneOpacity_);
    addProperty(blockedElementColor_);
    addProperty(width_);

    addProperty(manipulatorScale_);

    planePosition_.onChange(CallMemberAction<PlaneManipulation>(this, &PlaneManipulation::setManipulatorToStdPos));
    planeNormal_.onChange(CallMemberAction<PlaneManipulation>(this, &PlaneManipulation::setManipulatorToStdPos));
    resetManipulatorPos_.onClick(CallMemberAction<PlaneManipulation>(this, &PlaneManipulation::setManipulatorToStdPos));
    manipulatorScale_.onChange(CallMemberAction<PlaneManipulation>(this, &PlaneManipulation::updateManipulatorScale));
    resetPlane_.onClick(CallMemberAction<PlaneManipulation>(this, &PlaneManipulation::resetPlane));
}

PlaneManipulation::~PlaneManipulation() {
    delete moveEventProp_;
}

void PlaneManipulation::initialize() throw (tgt::Exception) {
    GeometryRendererBase::initialize();
}

Processor* PlaneManipulation::create() const {
    return new PlaneManipulation();
}

bool PlaneManipulation::isReady() const {
    return (volumeInport_.isReady() || geometryInport_.isReady());
}

void PlaneManipulation::setIDManager(IDManager* idm) {
    if (idManager_ == idm)
        return;

    idManager_ = idm;
    if (idManager_) {
        //set objects: plane manipulator
        idManager_->registerObject(&manipulatorCenter_);
        idManager_->registerObject(&topManipulatorPos_);
        idManager_->registerObject(&bottomManipulatorPos_);
    }
}

void PlaneManipulation::planeManipulation(tgt::MouseEvent* e) {

    e->ignore();

    //no plane manipulation if rendering of manipulator is not enabled
    if (!enable_.get() || (!renderManipulator_.get() && !renderPlane_.get()))
        return;

    //no plane manipulation / error message if no inport is connected
    if (!(volumeInport_.getData() || geometryInport_.getData())) {
        //LWARNING("volume or geometry inport has to be connected, ignoring events");
        return;
    }

    if (e->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT) {
        if (e->action() & tgt::MouseEvent::PRESSED) {
            const void* obj = idManager_->getObjectAtPos(tgt::ivec2(e->coord().x, e->viewport().y - e->coord().y));

            //check selection
            if (&topManipulatorPos_ == obj) {
                manipulatorTopTipIsGrabbed_ = true;
                planeNormal_.toggleInteractionMode(true, this);
                e->accept();

                //check if manipulating this tip is allowed by comparing direction of view vector to manipulator orientation via dot product
                float check = tgt::dot(manipulatorOrientation_, camera_.getPositionWithOffsets() - manipulatorCenter_);

                if (check < -EPS*1000.f)
                    invalidMovement_ = true;
                else
                    invalidMovement_ = false;

                invalidate();
            }
            else if (&bottomManipulatorPos_ == obj) {
                manipulatorBottomTipIsGrabbed_ = true;
                planeNormal_.toggleInteractionMode(true, this);
                e->accept();

                //check if manipulating this tip is allowed by comparing direction of view vector to manipulator orientation via dot product
                float check = tgt::dot(manipulatorOrientation_, camera_.getPositionWithOffsets() - manipulatorCenter_);

                if (check > EPS*1000.f)
                    invalidMovement_ = true;
                else
                    invalidMovement_ = false;

                invalidate();
            }
            else if (&manipulatorCenter_ == obj) {
                manipulatorCylinderIsGrabbed_ = true;
                planePosition_.toggleInteractionMode(true, this);
                e->accept();

                //compute scale factor to avoid numerical problems with small spacing
                scale_ = manipulatorLength_ / 200.f;

                //compute offset
                tgt::vec2 mousePos = tgt::vec2(static_cast<float>(e->coord().x), e->viewport().y-static_cast<float>(e->coord().y));

                tgt::vec3 begin = getWindowPos(manipulatorCenter_);
                tgt::vec3 end = getWindowPos(manipulatorCenter_ + manipulatorOrientation_ * scale_);

                tgt::vec3 point = begin;
                tgt::vec3 direction = end - begin;

                shiftOffset_ = getPointLineProjectionScalar(mousePos, point.xy(), direction.xy());

                invalidate();
            }
        }

        if (e->action() & tgt::MouseEvent::MOTION) {
            if (manipulatorTopTipIsGrabbed_ || manipulatorBottomTipIsGrabbed_ || manipulatorCylinderIsGrabbed_)
                e->accept();

            if (manipulatorCylinderIsGrabbed_) {
                // Determine mouse move offset...
                tgt::vec2 mousePos = tgt::vec2(static_cast<float>(e->coord().x), e->viewport().y-static_cast<float>(e->coord().y));

                tgt::vec3 begin = getWindowPos(manipulatorCenter_);
                tgt::vec3 end = getWindowPos(manipulatorCenter_ + manipulatorOrientation_ * scale_);

                tgt::vec3 point = begin;
                tgt::vec3 direction = end - begin;

                float t = getPointLineProjectionScalar(mousePos, point.xy(), direction.xy()) - shiftOffset_;

                //check if new manipulator position is valid (depending on the plane position)
                tgt::vec3 tmpCenter =  manipulatorCenter_ + t * manipulatorOrientation_ * scale_;
                float tmpPosition = tgt::dot(tmpCenter, manipulatorOrientation_);

                if ((positionRange_.get().x <= tmpPosition) && (tmpPosition <= positionRange_.get().y)) {
                    manipulatorCenter_ = tmpCenter;
                    planePosition_.set(tmpPosition);
                    updateManipulatorElements();
                    invalidMovement_ = false;
                }
                else {
                    invalidMovement_ = true;
                    invalidate();
                }

            }
            else if (manipulatorTopTipIsGrabbed_ || manipulatorBottomTipIsGrabbed_) {

                //check if manipulating this tip is allowed by comparing direction of view vector to manipulator orientation via dot product
                float check = tgt::dot(manipulatorOrientation_, camera_.getPositionWithOffsets() - manipulatorCenter_);

                if ((manipulatorTopTipIsGrabbed_ && !(check >= -EPS*1000.f)) || (manipulatorBottomTipIsGrabbed_ && !(check <= EPS*1000.f))) {
                    invalidMovement_ = true;
                    invalidate();
                    return;
                }
                else
                    invalidMovement_ = false;

                //get inverse view and projection matrices
                tgt::mat4 projectionMatrixInverse;
                camera_.getProjectionMatrix(e->viewport()).invert(projectionMatrixInverse);
                tgt::mat4 viewMatrixInverse = camera_.getViewMatrixInverse();

                //normalize mouse position
                tgt::vec2 pos = tgt::vec2(static_cast<float>(e->coord().x), e->viewport().y-static_cast<float>(e->coord().y)) / tgt::vec2(e->viewport());
                pos *= tgt::vec2(2.f);
                pos -= tgt::vec2(1.f);
                //get line of mouse position in world coordinates
                tgt::vec4 startWorld = viewMatrixInverse *  (projectionMatrixInverse *  tgt::vec4(pos, -1.f, 1.f));
                tgt::vec4 endWorld = viewMatrixInverse *  (projectionMatrixInverse *  tgt::vec4(pos, 1.f, 1.f));

                startWorld *= 1.f/startWorld.w;
                endWorld *= 1.f/endWorld.w;

                tgt::vec3 lineNormal = tgt::normalize(endWorld.xyz() - startWorld.xyz());

                //translate line so that sphere around manipulator is at origin
                tgt::vec3 startWorldTranslated = startWorld.xyz() - manipulatorCenter_;
                tgt::vec3 endWorldTranslated = endWorld.xyz() - manipulatorCenter_;

                float radius = manipulatorLength_ / 2.f;

                //check if line intersects sphere
                float dis = static_cast<float>(std::pow(tgt::dot(lineNormal, startWorldTranslated), 2.f))
                        - tgt::dot(startWorldTranslated, startWorldTranslated) + static_cast<float>(std::pow(radius, 2.f));

                tgt::vec3 tmpOrientation; // computed orientation

                if (dis >= 0.f) {
                    //hits sphere -> compute position on sphere
                    float t = -tgt::dot(lineNormal, startWorldTranslated) - std::sqrt(dis);
                    tgt::vec3 pos = (startWorld.xyz()) + t * lineNormal;
                    //compute orientation using the position
                    if (manipulatorTopTipIsGrabbed_)
                        tmpOrientation = tgt::normalize(pos - manipulatorCenter_);
                    else
                        tmpOrientation = tgt::normalize(manipulatorCenter_ - pos);
                }
                else {
                    //sphere has not been hit -> compute intersection with plane aligned with viewing plane through sphere center
                    tgt::vec3 planeNormal = tgt::normalize(camera_.getPositionWithOffsets() - manipulatorCenter_);
                    float t = tgt::dot(manipulatorCenter_ - startWorld.xyz(), planeNormal) / tgt::dot(lineNormal, planeNormal);
                    tgt::vec3 intersectionPoint = startWorld.xyz() + lineNormal * t;
                    //project to the intersection circle of sphere and plane
                    tgt::vec3 projectionPoint = tgt::normalize(intersectionPoint - manipulatorCenter_) * radius + manipulatorCenter_;
                    //compute orientation
                    if (manipulatorTopTipIsGrabbed_)
                        tmpOrientation = tgt::normalize(projectionPoint - manipulatorCenter_);
                    else
                        tmpOrientation = tgt::normalize(manipulatorCenter_ - projectionPoint);
                }

                //check if the orientation is valid for the current handle position (to ensure that the plane position is within the valid range)
                float tmpPosition = tgt::dot(manipulatorCenter_, tmpOrientation);

                if ((positionRange_.get().x <= tmpPosition) && (tmpPosition <= positionRange_.get().y)) {
                    manipulatorOrientation_ = tmpOrientation;
                    updateManipulatorElements();
                    planeNormal_.set(manipulatorOrientation_);
                    planePosition_.set(tmpPosition);
                    invalidate();
                }
                else {
                    invalidMovement_ = true;
                    invalidate();
                }
            }
        }

        if (e->action() & tgt::MouseEvent::RELEASED) {
            if(manipulatorTopTipIsGrabbed_ || manipulatorBottomTipIsGrabbed_ || manipulatorCylinderIsGrabbed_) {
                // Accept mouse event, turn off interaction mode, ungrab anchor and lines, and repaint...
                e->accept();
                planeNormal_.toggleInteractionMode(false, this);
                planePosition_.toggleInteractionMode(false, this);
                manipulatorTopTipIsGrabbed_ = false;
                manipulatorBottomTipIsGrabbed_ = false;
                manipulatorCylinderIsGrabbed_ = false;

                if (autoReset_.get() && !anchors_.empty())
                    setManipulatorToStdPos();

                invalidMovement_ = false;

                invalidate();
            }
        }
    }

    if (e->button() == tgt::MouseEvent::MOUSE_BUTTON_RIGHT) {

        if (e->action() & tgt::MouseEvent::PRESSED) {
            const void* obj = idManager_->getObjectAtPos(tgt::ivec2(e->coord().x, e->viewport().y - e->coord().y));

            //check selection
            if ((obj == &topManipulatorPos_) || (obj == &bottomManipulatorPos_) || (obj == &manipulatorCenter_)) {
                wholeManipulatorIsGrabbed_ = true;
                e->accept();

                //compute scale factor to avoid numerical problems with small spacing
                scale_ = manipulatorLength_ / 200.f;

                //compute offset
                tgt::vec2 mousePos = tgt::vec2(static_cast<float>(e->coord().x), e->viewport().y-static_cast<float>(e->coord().y));

                tgt::vec3 begin = getWindowPos(manipulatorCenter_);
                tgt::vec3 end = getWindowPos(manipulatorCenter_ + manipulatorOrientation_ * scale_);

                tgt::vec3 point = begin;
                tgt::vec3 direction = end - begin;

                shiftOffset_ = getPointLineProjectionScalar(mousePos, point.xy(), direction.xy());

                //check if the manipulator may be moved on the plane
                if (std::abs(tgt::dot(manipulatorOrientation_, tgt::normalize(camera_.getLook()))) < 0.2f)
                   invalidMovement_ = true;

                invalidate();
            }
        }

        if (e->action() & tgt::MouseEvent::MOTION) {

            if (wholeManipulatorIsGrabbed_) {

                //check if the manipulator may be moved on the plane
                float check = tgt::dot(manipulatorOrientation_, tgt::normalize(camera_.getLook()));

                if (std::abs(check) < 0.2f) {
                    invalidMovement_ = true;
                    invalidate();
                    return;
                }
                else
                    invalidMovement_ = false;

                e->accept();

                if (anchors_.empty())
                    return;

                //get line through mouse position:
                //1. get matrices
                tgt::mat4 projectionMatrixInverse;
                camera_.getProjectionMatrix(e->viewport()).invert(projectionMatrixInverse);
                tgt::mat4 viewMatrixInverse = camera_.getViewMatrixInverse();

                //2. normalize mouse position
                tgt::vec2 pos = tgt::vec2(static_cast<float>(e->coord().x), e->viewport().y-static_cast<float>(e->coord().y)) / tgt::vec2(e->viewport());
                pos *= tgt::vec2(2.f);
                pos -= tgt::vec2(1.f);
                //3. get line of mouse position in world coordinates
                tgt::vec4 startWorld = viewMatrixInverse *  (projectionMatrixInverse *  tgt::vec4(pos, -1.f, 1.f));
                tgt::vec4 endWorld = viewMatrixInverse *  (projectionMatrixInverse *  tgt::vec4(pos, 1.f, 1.f));

                startWorld *= 1.f/startWorld.w;
                endWorld *= 1.f/endWorld.w;

                tgt::vec3 lineNormal = tgt::normalize(endWorld.xyz() - startWorld.xyz());

                //get the plane and compute the intersection point
                tgt::vec3 planeNormal = tgt::normalize(planeNormal_.get());
                float t = tgt::dot(manipulatorCenter_ + manipulatorOrientation_ * scale_ * shiftOffset_ - startWorld.xyz(), planeNormal)
                                        / tgt::dot(lineNormal, planeNormal);

                tgt::vec3 intersectionPoint = startWorld.xyz() + lineNormal * t;

                //check if new position is valid
                tgt::vec3 tmpCenter = intersectionPoint - shiftOffset_ * scale_ * manipulatorOrientation_;

                if ((tgt::lessThanEqual(tmpCenter, manipulatorBounds_.getURB()) == tgt::bvec3(true))
                        && tgt::greaterThanEqual(tmpCenter, manipulatorBounds_.getLLF()) == tgt::bvec3(true))
                {
                    manipulatorCenter_ = tmpCenter;
                    updateManipulatorElements();
                }
                else
                    invalidMovement_ = true;

                invalidate();
            }
        }

        if (e->action() & tgt::MouseEvent::RELEASED) {

            if (wholeManipulatorIsGrabbed_) {
                // Accept mouse event, turn off interaction mode, ungrab anchor and lines, and repaint...
                e->accept();
                wholeManipulatorIsGrabbed_ = false;

                invalidMovement_ = false;

                invalidate();
            }
        }
    }
}

void PlaneManipulation::renderPicking() {
    if (!idManager_)
        return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    idManager_->setGLColor(&manipulatorCenter_);
    paintManipulatorCylinder(manipulatorCenter_, manipulatorOrientation_);

    idManager_->setGLColor(&topManipulatorPos_);
    paintManipulatorTip(topManipulatorPos_);

    idManager_->setGLColor(&bottomManipulatorPos_);
    paintManipulatorTip(bottomManipulatorPos_);


    glPopAttrib();
}

void PlaneManipulation::render() {

    if (!enable_.get())
        return;

    // check if exactly one input is connected
    tgtAssert(volumeInport_.isReady() || geometryInport_.isReady(), "neither inport is ready");

    // check if bounding box changed
    checkInportsForBoundingBox();

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // render clipping plane
    if (renderPlane_.get()) {

        // Update anchors and lines, if plane changed...
        if (tgt::equal(actualPlane_, tgt::vec4(planeNormal_.get(), planePosition_.get())) != tgt::bvec4(true, true, true, true))
            updateAnchorsAndLines();

        // Render lines...
        glColor4fv(planeColor_.get().elem);
        for (std::vector<Line>::iterator it = lines_.begin(); it != lines_.end(); ++it)
            paintLine(*it);

        //check if plane should be rendered
        if ((manipulatorTopTipIsGrabbed_ || manipulatorBottomTipIsGrabbed_ || manipulatorCylinderIsGrabbed_ || wholeManipulatorIsGrabbed_)
                && (grabbedPlaneOpacity_.get() > 0.f) && !invalidMovement_) {

            glColor4fv(tgt::vec4(planeColor_.get().xyz(), grabbedPlaneOpacity_.get()).elem);

            //prevent z fighting by using polygon offset
            glEnable(GL_POLYGON_OFFSET_FILL);

            float sign = invert_.get() ? -1.f : 1.f;
            float cos = sign * tgt::dot(manipulatorOrientation_, tgt::normalize(manipulatorCenter_ - camera_.getPositionWithOffsets()));
            if (cos < 0.f)
                glPolygonOffset(-2.f, -2.f);
            else
                glPolygonOffset(2.f, 2.f);

            //render plane polygon
            glBegin(GL_POLYGON);
            for (std::vector<Line>::iterator it = lines_.begin(); it != lines_.end(); ++it) {
                glVertex3fv((*it->first).elem);
                glVertex3fv((*it->second).elem);
            }
            glEnd();

            glDisable(GL_POLYGON_OFFSET_FILL);
        }

        // setup lighting...
        glEnable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);
        //transform light position with inverse view matrix to follow camera
        tgt::vec4 transformedLightPosition = camera_.getViewMatrixInverse() * lightPosition_;
        glLightfv(GL_LIGHT0, GL_POSITION, transformedLightPosition.elem);
        glEnable(GL_LIGHT0);

        tgt::vec3 spec(1.0f, 1.0f, 1.0f);
        glMaterialf( GL_FRONT_AND_BACK,    GL_SHININESS,    25.0f);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_SPECULAR,    spec.elem);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        tgt::vec4(1.0f, 0.0f, 0.0f, 1.0f).elem);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        tgt::vec4(1.0f, 0.0f, 0.0f, 1.0f).elem);

        //render anchors
        glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        planeColor_.get().elem);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        planeColor_.get().elem);
        for (std::vector<Anchor>::iterator it = anchors_.begin(); it != anchors_.end(); ++it) {
                paintAnchor(*it);
        }
    }

    // render manipulator
    if (renderManipulator_.get()) {
        paintManipulator();
    }

    glPopAttrib();

}

void PlaneManipulation::paintManipulator() {

    tgt::vec4 renderColor;          //color that is determined below and is used for rendering

    //get cosines for determining opacity etc.
    float topCos = tgt::dot(manipulatorOrientation_, tgt::normalize(camera_.getPositionWithOffsets() - manipulatorCenter_));
    float bottomCos = tgt::dot(-manipulatorOrientation_, tgt::normalize(camera_.getPositionWithOffsets() - manipulatorCenter_));

    //render cylinder
    if (manipulatorCylinderIsGrabbed_ || wholeManipulatorIsGrabbed_) {
        if (invalidMovement_)
            renderColor = blockedElementColor_.get();
        else
            renderColor = grabbedElementColor_.get();
        renderColor.a = 1.f;
        glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        renderColor.elem);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        renderColor.elem);
        paintManipulatorCylinder(manipulatorCenter_, manipulatorOrientation_);
    }
    else {
        //if not grabbed: the half of the cylinder behind the plane should be rendered transparent, depending on the angle with the camera
        renderColor = planeColor_.get();
        renderColor.a = 1.f;

        if (topCos < -EPS * 10.f)
            renderColor.a = 1.f + std::max(-(topCos * topCos) - 0.075f, -0.96f);

        glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        renderColor.elem);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        renderColor.elem);

        paintManipulatorCylinderTopHalf(manipulatorCenter_, manipulatorOrientation_);

        renderColor.a = 1.f;

        if (bottomCos < -EPS * 10.f)
            renderColor.a = 1.f + std::max(-(bottomCos * bottomCos) - 0.075f, -0.96f);

        glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        renderColor.elem);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        renderColor.elem);

        paintManipulatorCylinderBottomHalf(manipulatorCenter_, manipulatorOrientation_);
    }

    //render top tip
    if (manipulatorTopTipIsGrabbed_ || wholeManipulatorIsGrabbed_)
        if (invalidMovement_)
            renderColor = blockedElementColor_.get();
        else
            renderColor = grabbedElementColor_.get();
    else
        renderColor = planeColor_.get();

    //if not grabbed: render transparent if behind the plane (depending on angle with camera)
    renderColor.a = 1.f;
    if (topCos < -EPS * 10.f)
            renderColor.a = 1.f + std::max(-(topCos * topCos) - 0.075f, -0.96f);

    glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        renderColor.elem);
    glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        renderColor.elem);

    paintManipulatorTip(topManipulatorPos_);

    //render bottom tip
    if (manipulatorBottomTipIsGrabbed_ || wholeManipulatorIsGrabbed_)
        if (invalidMovement_)
            renderColor = blockedElementColor_.get();
        else
            renderColor = grabbedElementColor_.get();
    else
        renderColor = planeColor_.get();

    //if not grabbed: render transparent if behind the plane (depending on angle with camera)
    renderColor.a = 1.f;
    if (bottomCos < -EPS * 10.f)
            renderColor.a = 1.f + std::max(-(bottomCos * bottomCos) - 0.075f, -0.96f);

    glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        renderColor.elem);
    glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        renderColor.elem);

    paintManipulatorTip(bottomManipulatorPos_);
}

void PlaneManipulation::paintManipulatorCylinder(const tgt::vec3& center, const tgt::vec3& direction) {
    paintManipulatorCylinderTopHalf(center, direction);
    paintManipulatorCylinderBottomHalf(center, direction);
}

void PlaneManipulation::paintManipulatorCylinderTopHalf(const tgt::vec3& center, const tgt::vec3& direction) {

    GLUquadricObj* quadric = gluNewQuadric();

    MatStack.pushMatrix();

    // 3. translate to new center
    MatStack.translate(center.x, center.y, center.z);

    // 2. rotate to new direction
    float angle = static_cast<float>(std::acos(tgt::dot(tgt::normalize(direction), tgt::vec3(0.f, 0.f, 1.f))));
    tgt::vec3 rotationAxis = tgt::cross(tgt::normalize(direction), tgt::vec3(0.f, 0.f, 1.f));

    if (tgt::equal(rotationAxis, tgt::vec3(0.f)) != tgt::bvec3(true, true, true)) {
        tgt::mat4 matrix = tgt::mat4::createRotation(-angle, rotationAxis);
        glMultMatrixf(tgt::transpose(matrix).elem);
    }
    else {
        //if there can no rotation axis be found the direction lies in positive or negative z direction
        //for negative direction: rotate around x axis, else: do nothing
        if (direction.z < 0.f)
            MatStack.rotate(180.f, 1.f, 0.f, 0.f);
    }

    gluCylinder(quadric, manipulatorDiameter_, manipulatorDiameter_ / 2.f, manipulatorLength_ / 2.f, 20, 20);

    MatStack.popMatrix();
    gluDeleteQuadric(quadric);
}

void PlaneManipulation::paintManipulatorCylinderBottomHalf(const tgt::vec3& center, const tgt::vec3& direction) {

    GLUquadricObj* quadric = gluNewQuadric();

    MatStack.pushMatrix();

    // 3. translate to new center
    MatStack.translate(center.x, center.y, center.z);

    // 2. rotate to new direction
    float angle = static_cast<float>(std::acos(tgt::dot(tgt::normalize(direction), tgt::vec3(0.f, 0.f, 1.f))));
    tgt::vec3 rotationAxis = tgt::cross(tgt::normalize(direction), tgt::vec3(0.f, 0.f, 1.f));

    if (tgt::equal(rotationAxis, tgt::vec3(0.f)) != tgt::bvec3(true, true, true)) {
        tgt::mat4 matrix = tgt::mat4::createRotation(-angle, rotationAxis);
        glMultMatrixf(tgt::transpose(matrix).elem);
    }
    else {
        //if there can no rotation axis be found the direction lies in positive or negative z direction
        //for negative direction: rotate around x axis, else: do nothing
        if (direction.z < 0.f)
            MatStack.rotate(180.f, 1.f, 0.f, 0.f);
    }

    // 1. cylinder: translate center to origin
    MatStack.translate(0.f, 0.f, -manipulatorLength_ / 2.f);
    gluCylinder(quadric, manipulatorDiameter_ / 2.f, manipulatorDiameter_, manipulatorLength_ / 2.f, 20, 20);

    MatStack.popMatrix();
    gluDeleteQuadric(quadric);
}

void PlaneManipulation::paintManipulatorTip(const tgt::vec3& center) {
    GLUquadricObj* quadric = gluNewQuadric();
    MatStack.pushMatrix();
    MatStack.translate(center.x, center.y, center.z);
    gluSphere(quadric, manipulatorTipRadius_, 32, 32);
    MatStack.popMatrix();
    gluDeleteQuadric(quadric);
}

void PlaneManipulation::paintAnchor(const Anchor& anchor) {
    GLUquadricObj* quadric = gluNewQuadric();
    MatStack.pushMatrix();
    MatStack.translate(anchor.x, anchor.y, anchor.z);
    gluSphere(quadric, manipulatorTipRadius_ / 2.5f, 20, 20);
    MatStack.popMatrix();
    gluDeleteQuadric(quadric);
}

void PlaneManipulation::paintLine(const Line& line) {
    glLineWidth(width_.get());
    glBegin(GL_LINES);
    tgt::vec3 begin = *(line.first);
    tgt::vec3 end = *(line.second);
    glVertex3fv(begin.elem);
    glVertex3fv(end.elem);
    glEnd();
}

/*inline float PlaneManipulation::getPointLineDistance(const tgt::vec3& point, const tgt::vec3& linePositionVector, const tgt::vec3& lineDirectionVector) const {
    return tgt::length(tgt::cross(lineDirectionVector, point - linePositionVector)) / tgt::length(lineDirectionVector);
}*/

inline float PlaneManipulation::getPointLineProjectionScalar(const tgt::vec2& point, const tgt::vec2& linePositionVector, const tgt::vec2& lineDirectionVector) const {
    return tgt::dot(point - linePositionVector, lineDirectionVector) / tgt::lengthSq(lineDirectionVector);
}

bool PlaneManipulation::getIntersectionPointInBB(tgt::vec3& intersectionPoint, const tgt::vec3& startVertex,
        const tgt::vec3& endVertex, const tgt::vec4& plane)
{
    tgt::vec3 lineDirectionVector = tgt::normalize(endVertex - startVertex);

    float denominator = tgt::dot(lineDirectionVector, plane.xyz());
    if (std::abs(denominator) < EPS)
        return false;

    float numerator = tgt::dot((plane.xyz() * plane.w) - startVertex, plane.xyz());
    float t = numerator / denominator;

    tgt::vec3 intersection = startVertex + t * lineDirectionVector;

    // Is intersection point within AABB?
    float tEnd = tgt::length(endVertex - startVertex);
    if (t >= 0 && t <= tEnd)
    {
        intersectionPoint = intersection;
        return true;
    }

    return false;
}

void PlaneManipulation::addAnchor(const tgt::vec3& startVertex, const tgt::vec3& endVertex, const tgt::vec4& plane) {
    tgt::vec3 intersectionPoint;
    // No intersection point withing the bounding box?
    if (!getIntersectionPointInBB(intersectionPoint, startVertex, endVertex, plane))
        return;

    // Omit anchor, if it already exist...
    for (std::vector<Anchor>::iterator it = anchors_.begin(); it != anchors_.end(); ++it)
        if (std::abs(tgt::length(*it - intersectionPoint)) < EPS)
            return;

    anchors_.push_back(intersectionPoint);
}

void PlaneManipulation::addLine(const tgt::vec4& plane) {

    Anchor* begin = 0;
    Anchor* end = 0;

    for (std::vector<Anchor>::iterator it = anchors_.begin(); it != anchors_.end(); ++it) {

        // Is anchor on given plane? TODO: epsilon should not be multiplied, find a better solution for this to account for numerical problems
        if (std::abs(tgt::dot(plane.xyz(), *it) - plane.w) < EPS * 100.f) {
            if (!begin)
                begin = &(*it);
            else {
                end = &(*it);
                break;
            }
        }
    }

    // Was line start and end point found?
    if (begin && end)
        lines_.push_back(std::make_pair(begin, end));
}

void PlaneManipulation::updateAnchorsAndLines() {
    tgtAssert((volumeInport_.getData() || geometryInport_.getData()), "no input volume or geometry");

    anchors_.clear();
    lines_.clear();

    //use physical bounding box and transform its vertices to world coordinates to compute the anchors and lines
    tgt::Bounds bBox = physicalSceneBounds_;

    tgt::vec3 physicalLLF = bBox.getLLF();
    tgt::vec3 physicalURB = bBox.getURB();

    tgt::vec3 worldLLF = (physicalToWorld_ * tgt::vec4(physicalLLF, 1.f)).xyz();
    tgt::vec3 worldURB = (physicalToWorld_ * tgt::vec4(physicalURB, 1.f)).xyz();
    tgt::vec3 worldLRF = (physicalToWorld_ * tgt::vec4(physicalURB.x, physicalLLF.y, physicalLLF.z, 1.f)).xyz();
    tgt::vec3 worldULF = (physicalToWorld_ * tgt::vec4(physicalLLF.x, physicalURB.y, physicalLLF.z, 1.f)).xyz();
    tgt::vec3 worldLLB = (physicalToWorld_ * tgt::vec4(physicalLLF.x, physicalLLF.y, physicalURB.z, 1.f)).xyz();
    tgt::vec3 worldURF = (physicalToWorld_ * tgt::vec4(physicalURB.x, physicalURB.y, physicalLLF.z, 1.f)).xyz();
    tgt::vec3 worldULB = (physicalToWorld_ * tgt::vec4(physicalLLF.x, physicalURB.y, physicalURB.z, 1.f)).xyz();
    tgt::vec3 worldLRB = (physicalToWorld_ * tgt::vec4(physicalURB.x, physicalLLF.y, physicalURB.z, 1.f)).xyz();

    //get plane
    tgt::vec4 plane = tgt::vec4(tgt::normalize(planeNormal_.get()), planePosition_.get());

    // Determine and add all anchors...
    addAnchor(worldLLF, worldLRF, plane);
    addAnchor(worldLLF, worldULF, plane);
    addAnchor(worldLLF, worldLLB, plane);
    addAnchor(worldURB, worldULB, plane);
    addAnchor(worldURB, worldLRB, plane);
    addAnchor(worldURB, worldURF, plane);
    addAnchor(worldLLB, worldLRB, plane);
    addAnchor(worldLLB, worldULB, plane);
    addAnchor(worldULF, worldURF, plane);
    addAnchor(worldULF, worldULB, plane);
    addAnchor(worldLRF, worldURF, plane);
    addAnchor(worldLRF, worldLRB, plane);

    // Less than 3 anchors (indicates that the clipping plane only touches the AABB)?
    if (anchors_.size() < 3)
        anchors_.clear();

    tgt::mat4 t;
    physicalToWorld_.getRotationalPart().invert(t);

    //determine the normal vectors for the planes
    //tgt::vec3 nx = tgt::normalize(worldLRF - worldLLF);
    //tgt::vec3 ny = tgt::normalize(worldULF - worldLLF);
    //tgt::vec3 nz = tgt::normalize(worldLLB - worldLLF);
    tgt::vec3 nx = tgt::normalize((tgt::transpose(t) * tgt::vec4(1.f, 0.f, 0.f, 1.f)).xyz());
    tgt::vec3 ny = tgt::normalize((tgt::transpose(t) * tgt::vec4(0.f, 1.f, 0.f, 1.f)).xyz());
    tgt::vec3 nz = tgt::normalize((tgt::transpose(t) * tgt::vec4(0.f, 0.f, 1.f, 1.f)).xyz());
    addLine(tgt::vec4( nx ,  tgt::dot(nx, worldURB)));
    addLine(tgt::vec4( nx ,  tgt::dot(nx, worldLLF)));
    addLine(tgt::vec4( ny ,  tgt::dot(ny, worldURB)));
    addLine(tgt::vec4( ny ,  tgt::dot(ny, worldLLF)));
    addLine(tgt::vec4( nz ,  tgt::dot(nz, worldURB)));
    addLine(tgt::vec4( nz ,  tgt::dot(nz, worldLLF)));


    // Update actual plane equation to ensure that this update function is only called when necessary...
    actualPlane_ = tgt::vec4(planeNormal_.get(), planePosition_.get());

    // No lines found?
    if (lines_.size() == 0)
        return;

    //
    // Order lines and their anchors to CCW polygon vertex order which is expected by OpenGL...
    //

    // Sort lines to produce contiguous vertex order...
    bool reverseLastLine = false;
    for (std::vector<Line>::size_type i = 0; i < lines_.size() - 1; ++i) {
        for (std::vector<Line>::size_type j = i + 1; j < lines_.size(); ++j) {
            Anchor* connectionAnchor;
            if (reverseLastLine)
                connectionAnchor = lines_.at(i).first;
            else
                connectionAnchor = lines_.at(i).second;

            if (std::abs(tgt::length(*connectionAnchor - *(lines_.at(j).first))) < EPS) {
                std::swap(lines_.at(i + 1), lines_.at(j));
                reverseLastLine = false;
                break;
            }
            else if (std::abs(tgt::length(*connectionAnchor - *(lines_.at(j).second))) < EPS) {
                std::swap(lines_.at(i + 1), lines_.at(j));
                reverseLastLine = true;
                break;
            }
        }
    }

    std::vector<Anchor*> vertices;
    // Convert sorted line list to sorted vertex list...
    for (std::vector<Line>::size_type i = 0; i < lines_.size(); ++i) {
        bool reverseLine = i != 0 && std::abs(tgt::length(*vertices.at(vertices.size() - 1) - *(lines_.at(i).first))) >= EPS;

        Anchor* first = (reverseLine ? lines_.at(i).second : lines_.at(i).first);
        Anchor* second = (reverseLine ? lines_.at(i).first : lines_.at(i).second);

        if (i == 0)
            vertices.push_back(first);

        if (i < (vertices.size() - 1))
            vertices.push_back(second);
    }

    // Convert vertex order to counter clockwise if necessary...
    tgt::vec3 normal(0, 0, 0);
    for (std::vector<Anchor*>::size_type i = 0; i < vertices.size(); ++i)
        normal += tgt::cross(*vertices[i], *vertices[(i + 1) % vertices.size()]);
    normal = tgt::normalize(normal);

    if (tgt::dot(plane.xyz(), normal) < 0) {
        std::reverse(lines_.begin(), lines_.end());
        for (std::vector<Line>::iterator it = lines_.begin(); it != lines_.end(); ++it)
            std::swap(it->first, it->second);
    }
}

void PlaneManipulation::checkInportsForBoundingBox() {

    if (!volumeInport_.getData() && !geometryInport_.getData())
        return;

    //get bounding box, first try to get from volume, else get from geometry
    tgt::Bounds bBoxWorld;
    tgt::Bounds bBoxPhysical;
    if (volumeInport_.getData()) {
        bBoxWorld = volumeInport_.getData()->getBoundingBox().getBoundingBox();
        bBoxPhysical = volumeInport_.getData()->getBoundingBox(false).getBoundingBox(false);
    }
    else {
        bBoxWorld = geometryInport_.getData()->getBoundingBox();
        bBoxPhysical = geometryInport_.getData()->getBoundingBox(false);
    }

    //check if scene bounds have changed -> update range for plane position and size of manipulator according to current scene
    if ((!worldSceneBounds_.isDefined() ||
                (bBoxWorld.getLLF() != worldSceneBounds_.getLLF())
                || (bBoxWorld.getURB() != worldSceneBounds_.getURB()))
        || (!physicalSceneBounds_.isDefined() ||
                (bBoxPhysical.getLLF() != physicalSceneBounds_.getLLF())
                || (bBoxPhysical.getURB() != physicalSceneBounds_.getURB())))
    {

        //set new bounds
        worldSceneBounds_ = bBoxWorld;
        manipulatorBounds_ = tgt::Bounds(bBoxWorld.center() + (bBoxWorld.getLLF() - bBoxWorld.center()) * 6.f,
                bBoxWorld.center() + (bBoxWorld.getURB() - bBoxWorld.center()) * 6.f);

        physicalSceneBounds_ = bBoxPhysical;

        //get transformations
        if (volumeInport_.getData()) {
            physicalToWorld_ = volumeInport_.getData()->getPhysicalToWorldMatrix();
            worldToPhysical_ = volumeInport_.getData()->getWorldToPhysicalMatrix();
        }
        else {
            physicalToWorld_ = geometryInport_.getData()->getTransformationMatrix();
            worldToPhysical_ = geometryInport_.getData()->getInvertedTransformationMatrix();
        }

        //check every corner of world scene bounds and compute position range
        tgt::vec3 llf = bBoxWorld.getLLF();
        tgt::vec3 urb = bBoxWorld.getURB();
        float length = std::max(tgt::length(llf), tgt::length(urb));

        tgt::vec3 llb = tgt::vec3(llf.x, llf.y, urb.z);
        tgt::vec3 urf = tgt::vec3(urb.x, urb.y, llf.z);
        length = std::max(length, std::max(tgt::length(llb), tgt::length(urf)));

        tgt::vec3 lrf = tgt::vec3(llf.x, urb.y, llf.z);
        tgt::vec3 ulb = tgt::vec3(urb.x, llf.y, urb.z);
        length = std::max(length, std::max(tgt::length(lrf), tgt::length(ulb)));

        tgt::vec3 lrb = tgt::vec3(llf.x, urb.y, urb.z);
        tgt::vec3 ulf = tgt::vec3(urb.x, llf.y, llf.z);
        length = std::max(length, std::max(tgt::length(lrb), tgt::length(ulf)));

        if (length == 0.f)
            length = 1.f;

        positionRange_.set(tgt::vec2(-length, length));

        //check if plane position is valid and adjust if necessary
        if (planePosition_.get() < -length)
            planePosition_.set(-length);
        else if (planePosition_.get() > length)
            planePosition_.set(length);

        //if the bounding box changed: reset the manipulator
        setManipulatorToStdPos();

        updateManipulatorScale();

        updateAnchorsAndLines();
    }
}

void PlaneManipulation::updateManipulatorScale(){
    //compute length and diameter of manipulator handle according to scene size
    manipulatorLength_ = 0.25f * tgt::length(worldSceneBounds_.diagonal()) * manipulatorScale_.get();
    manipulatorDiameter_ = 0.05f * manipulatorLength_;
    manipulatorTipRadius_ = 0.05f * manipulatorLength_;
    updateManipulatorElements();
    //invalidate();
}

void PlaneManipulation::setManipulatorToStdPos() {

    //do not change position if the manipulator is currently in use
    if (manipulatorTopTipIsGrabbed_ || manipulatorBottomTipIsGrabbed_ || manipulatorCylinderIsGrabbed_)
        return;

    tgt::vec3 normal = tgt::normalize(planeNormal_.get());
    tgt::vec3 center = worldSceneBounds_.center();
    tgt::vec3 pointOnPlane = normal * planePosition_.get();
    float c = tgt::dot(normal, center - pointOnPlane);

    manipulatorCenter_ = center - normal * c;
    manipulatorOrientation_ = normal;
    updateManipulatorElements();

    invalidate();
}

void PlaneManipulation::resetPlane() {
    //set normal to -z in physical coordinates
    planeNormal_.set(tgt::normalize((physicalToWorld_.getRotationalPart() * tgt::vec4(0.f,0.f,-1.f, 1.f)).xyz()));

    //set position to center of bounding box in world coordinates
    float position = tgt::dot(worldSceneBounds_.center(), planeNormal_.get());
    planePosition_.set(position);
}

void PlaneManipulation::updateManipulatorElements() {
    topManipulatorPos_ = manipulatorCenter_ + tgt::normalize(manipulatorOrientation_) * (manipulatorLength_ / 2.f);
    bottomManipulatorPos_ = manipulatorCenter_  - tgt::normalize(manipulatorOrientation_) * (manipulatorLength_ / 2.f);
}

} //namespace voreen
