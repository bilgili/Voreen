/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2005-2012 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "tgt/camera.h"

#include "tgt/assert.h"
#include "tgt/glmath.h"
#include "tgt/quaternion.h"
#include "tgt/spline.h"
#include "tgt/tgt_gl.h"

#include <cmath>
#include <iostream>

namespace tgt {

// Constructor
Camera::Camera(const vec3& position, const vec3& focus, const vec3& up,
               float fovy, float ratio, float distn, float distf, ProjectionMode pm)
    : position_(position),
      focus_(focus),
      upVector_(normalize(up)),
      frust_(Frustum(fovy, ratio, distn, distf)),
      projectionMode_(pm),
      eyeSeparation_(1.0f),
      eyeMode_(EYE_MIDDLE),
      axisMode_(ON_AXIS)
{
    viewMatrix_ = mat4::createLookAt(position, focus, up);
}

Camera::~Camera() {
}

Camera* Camera::clone() const {
    Camera* cam = new Camera(position_, focus_, upVector_, frust_.getFovy(),
        frust_.getRatio(), frust_.getNearDist(), frust_.getFarDist());
    cam->setStereoAxisMode(axisMode_);
    cam->setStereoEyeMode(eyeMode_, false);
    cam->setStereoEyeSeparation(eyeSeparation_, false);
    return cam;
}

bool Camera::operator==(const Camera& rhs) const {
    return (rhs.position_ == position_) && (rhs.focus_ == focus_) && (rhs.upVector_ == upVector_) && (rhs.frust_ == frust_) &&
        (rhs.projectionMode_ == projectionMode_);
}

bool Camera::operator!=(const Camera& rhs) const {
    return !(*this == rhs);
}

// This is called to set up the Camera-View
void Camera::look(float windowRatio) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    updateFrustum();
    loadMatrix(getFrustumMatrix(windowRatio));
    //getProjectionMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    updateVM();
    loadMatrix(viewMatrix_);
}

void Camera::look(ivec2 windowSize) {
    look(static_cast<float>(windowSize.x) / windowSize.y);
}

// Private method that updates the relevant frustum parameters
void Camera::updateFrustum() {
    frust_.update(this);
}

mat4 Camera::getViewMatrix() const {
    updateVM();
    return viewMatrix_;
}

void Camera::setViewMatrix(const mat4& mvMat) {
    mat4 inv;
    if (mvMat.invert(inv)) {
        // preserve the focallength
        float focallength = length(focus_ - position_);

        // calculate world-coordinates
        vec4 pos   = (inv * vec4(0.f, 0.f,  0.f, 1.f));
        vec4 look  = (inv * vec4(0.f, 0.f, -1.f, 0.f));
        vec4 focus = pos + focallength * look;
        vec4 up    = (inv * vec4(0.f, 1.f,  0.f, 0.f));

        positionCamera(pos.xyz(), focus.xyz(), up.xyz());

        viewMatrix_ = mvMat;
    }
}

mat4 Camera::getRotateMatrix() const {
    updateVM();
    return viewMatrix_.getRotationalPart();
}

mat4 Camera::getViewMatrixInverse() const {
    updateVM();
    mat4 inv;
    if (viewMatrix_.invert(inv))
        return inv;
    else
        return mat4::identity;
}

mat4 Camera::getFrustumMatrix(float windowRatio) const {
    return mat4::createFrustum(frust_.getLeft() * windowRatio, frust_.getRight() * windowRatio,
                               frust_.getBottom(), frust_.getTop(),
                               frust_.getNearDist(), frust_.getFarDist());
}

mat4 Camera::getFrustumMatrix(ivec2 windowSize) const {
    return getFrustumMatrix(static_cast<float>(windowSize.x) / windowSize.y);
}

mat4 Camera::getProjectionMatrix(ivec2 windowSize) const {
    return getProjectionMatrix(static_cast<float>(windowSize.x) / windowSize.y);
}

mat4 Camera::getProjectionMatrix(float windowRatio) const {
    if(projectionMode_ == ORTHOGRAPHIC) {
        if(windowRatio > 1.0f)
            return mat4::createOrtho(frust_.getLeft() * windowRatio, frust_.getRight() * windowRatio,
                                     frust_.getTop(), frust_.getBottom(),
                                    -frust_.getNearDist(), frust_.getFarDist());
        else
            return mat4::createOrtho(frust_.getLeft(), frust_.getRight(),
                                     frust_.getTop() * (1.0f/windowRatio), frust_.getBottom() * (1.0f/windowRatio),
                                    -frust_.getNearDist(), frust_.getFarDist());
    } else if(projectionMode_ == PERSPECTIVE) {
        float fovy = frust_.getFovy();
        if(fovy < 6.f)
            fovy = 6.f;
        if(fovy > 175.f)
            fovy = 175.f;

        if(windowRatio >= 1.0f)
            return mat4::createPerspective(deg2rad(fovy), frust_.getRatio() * windowRatio , frust_.getNearDist(), frust_.getFarDist());
        else
            return mat4::createPerspective(atan(tan(deg2rad(fovy/2.f))/(windowRatio* frust_.getRatio()))*2, frust_.getRatio() * windowRatio , frust_.getNearDist(), frust_.getFarDist());
    }
    else
        return getFrustumMatrix(windowRatio);
}

line3 Camera::getViewRay(ivec2 vp, ivec2 pixel) const {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];

    tgt::mat4 projection_tgt = getProjectionMatrix(vp);
    tgt::mat4 modelview_tgt = getViewMatrix();
    for (int i = 0; i < 4; ++i) {
        modelview[i+0]   = modelview_tgt[i].x;
        modelview[i+4]   = modelview_tgt[i].y;
        modelview[i+8]   = modelview_tgt[i].z;
        modelview[i+12]  = modelview_tgt[i].w;
        projection[i+0]  = projection_tgt[i].x;
        projection[i+4]  = projection_tgt[i].y;
        projection[i+8]  = projection_tgt[i].z;
        projection[i+12] = projection_tgt[i].w;
    }
    viewport[0] = 0;
    viewport[1] = 0;
    viewport[2] = static_cast<GLint>(vp.x);
    viewport[3] = static_cast<GLint>(vp.y);

    GLdouble winX, winY, winZ;
    winX = pixel.x;
    winY = vp.y - pixel.y;
    winZ = 1.0f;

    tgt::dvec3 posFar;
    gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posFar.x, &posFar.y, &posFar.z);
    LGL_ERROR;

    winZ = 0.0f;
    tgt::dvec3 posNear;
    gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posNear.x, &posNear.y, &posNear.z);
    LGL_ERROR;

    tgt::line3 l(posNear, posFar);
    return l;
}

vec3 Camera::project(ivec2 vp, vec3 point) const {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];

    tgt::mat4 projection_tgt = getProjectionMatrix(vp);
    tgt::mat4 modelview_tgt = getViewMatrix();
    for (int i = 0; i < 4; ++i) {
        modelview[i+0]   = modelview_tgt[i].x;
        modelview[i+4]   = modelview_tgt[i].y;
        modelview[i+8]   = modelview_tgt[i].z;
        modelview[i+12]  = modelview_tgt[i].w;
        projection[i+0]  = projection_tgt[i].x;
        projection[i+4]  = projection_tgt[i].y;
        projection[i+8]  = projection_tgt[i].z;
        projection[i+12] = projection_tgt[i].w;
    }
    viewport[0] = 0;
    viewport[1] = 0;
    viewport[2] = vp.x;
    viewport[3] = vp.y;

    GLdouble pointProjectedGL[3];
    gluProject(point.x, point.y, point.z, modelview, projection, viewport,
               &pointProjectedGL[0], &pointProjectedGL[1], &pointProjectedGL[2]);

    return tgt::vec3(static_cast<float>(pointProjectedGL[0]),
                     static_cast<float>(pointProjectedGL[1]),
                     static_cast<float>(pointProjectedGL[2]));
}

bool Camera::setStereoEyeSeparation(float separation, bool updateCam) {
    if(!updateCam) {
        eyeSeparation_ = separation;
        return false;
    }

    if(eyeMode_ == EYE_MIDDLE)
        return false;

    float dif = separation - eyeSeparation_;
    if(dif == 0.f)
        return false;
    else {
        stereoShift(getStereoShift(eyeMode_, EYE_MIDDLE));
        eyeSeparation_ = separation;
        stereoShift(getStereoShift(EYE_MIDDLE, eyeMode_));
        //tgt::vec3 dir = getStrafe() * dif / 2.f;
        //stereoShift(dir);
        return true;
    }
}

bool Camera::setStereoEyeMode(StereoEyeMode mode, bool updateCam) {
    if(!updateCam) {
        eyeMode_ = mode;
        return false;
    }

    if(mode == eyeMode_)
        return false;

    stereoShift(getStereoShift(eyeMode_, mode));
    eyeMode_ = mode;
    return true;
}

void Camera::stereoShift(tgt::vec3 shift) {
    if(length(shift) == 0.f)
        return;
    stereoCameraShift(shift);
    stereoFrustumShift(shift);
}

tgt::vec3 Camera::getStereoShift(StereoEyeMode from, StereoEyeMode to) const {
    if(from == to)
        return tgt::vec3(0.f);

    switch(to) {
        case EYE_LEFT:
            switch(from){
                case EYE_MIDDLE:
                    return getStrafe() * (-eyeSeparation_ / 2.f);
                case EYE_RIGHT:
                    return getStrafe() * (-eyeSeparation_);
            }
            break;

        case EYE_MIDDLE:
            switch(from){
                case EYE_LEFT:
                    return getStrafe() * (eyeSeparation_ / 2.0f);
                case EYE_RIGHT:
                    return getStrafe() * (-eyeSeparation_/ 2.0f);
            }
            break;

        case EYE_RIGHT:
            switch(from){
                case EYE_MIDDLE:
                    return getStrafe() * (eyeSeparation_ / 2.0f);
                case EYE_LEFT:
                    return getStrafe() * eyeSeparation_;
            }
            break;
    }
    //should not get here, but removes a warning
    return tgt::vec3(0.f);
}

void Camera::stereoCameraShift(tgt::vec3 shift) {
    //calculate new focus
    float moveXScalar = tgt::dot(shift, getStrafe());
    float moveYScalar = tgt::dot(shift, getUpVector());
    //tgt::vec3 moveZ = tgt::dot(shift, getLook())*getLook();
    tgt::vec3 focusProj = moveXScalar * getStrafe() + moveYScalar * getUpVector();

    //set new position/focus
    position_ = getPosition() + shift;
    focus_ = getFocus() + focusProj;
    invalidateVM();
}

void Camera::stereoFrustumShift(tgt::vec3 shift) {
    float moveXScalar = tgt::dot(shift, getStrafe());
    float moveYScalar = tgt::dot(shift, getUpVector());

    // TODO: find out if oldDist always == newDist (off/on-axis?)
    float oDist = getFocalLength();
    //float nDist = length(getPosition() - getFocus() + shift);
    float nDist = getFocalLength();

    setFrustRight((getFrustRight() * oDist - moveXScalar * getNearDist())/nDist);
    //should be -dis... but left frustum value is < 0 so --=+
    setFrustLeft((getFrustLeft() * oDist - moveXScalar * getNearDist())/nDist);
    setFrustTop((getFrustTop() * oDist - moveYScalar * getNearDist())/nDist);
    //should be -dis... but left frustum value is < 0 so --=+
    setFrustBottom((getFrustBottom() * oDist - moveYScalar * getNearDist())/nDist);        
}

} // namespace tgt
