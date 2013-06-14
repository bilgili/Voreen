/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2005-2013 Visualization and Computer Graphics Group, *
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
#include "tgt/logmanager.h"
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
      axisMode_(ON_AXIS),
      useOffset_(false),
      offset_(tgt::vec3(0.f))
{
    viewMatrix_ = mat4::createLookAt(position, focus, up);
}

Camera::Camera(const Camera& cam)
    : position_(cam.getPosition()),
      focus_(cam.getFocus()),
      upVector_(cam.getUpVector()),
      frust_(cam.getFrustum()),
      projectionMode_(cam.getProjectionMode()),
      eyeSeparation_(cam.getStereoEyeSeparation()),
      eyeMode_(cam.getStereoEyeMode()),
      axisMode_(cam.getStereoAxisMode()),
      useOffset_(cam.isOffsetEnabled()),
      offset_(cam.getOffset())
{
    viewMatrix_ = mat4::createLookAt(getPositionWithOffsets(), getFocusWithOffsets(), upVector_);
}

Camera::~Camera() {
}

Camera* Camera::clone() const {
    Camera* cam = new Camera(position_, focus_, upVector_, frust_.getFovy(),
        frust_.getRatio(), frust_.getNearDist(), frust_.getFarDist());
    cam->setStereoAxisMode(axisMode_);
    cam->setStereoEyeMode(eyeMode_, false);
    cam->setStereoEyeSeparation(eyeSeparation_, false);
    cam->enableOffset(useOffset_);
    cam->setOffset(offset_);
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

        updateVM();
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

tgt::Frustum Camera::getFrustumWithOffsets() const {
    if(eyeMode_ != EYE_MIDDLE && axisMode_ == ON_AXIS)
        return stereoFrustumShift();
    else
        return frust_;
}

mat4 Camera::getFrustumMatrix(float windowRatio) const {
    Frustum f = getFrustumWithOffsets();
    return mat4::createFrustum(f.getLeft() * windowRatio, f.getRight() * windowRatio,
                               f.getBottom(), f.getTop(),
                               f.getNearDist(), f.getFarDist());
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
    eyeSeparation_ = separation;
    return updateCam;;
}

bool Camera::setStereoEyeMode(StereoEyeMode mode, bool updateCam) {
    if(eyeMode_ == mode)
        return false;
    eyeMode_ = mode;
    return updateCam;
}

tgt::vec3 Camera::getStereoShift() const {
    if(eyeMode_ == EYE_MIDDLE)
        return tgt::vec3(0.f);
    else if(eyeMode_ == EYE_LEFT)
        return getStrafeWithOffsets() * (-eyeSeparation_ * 0.5f);
    else
        return getStrafeWithOffsets() * (eyeSeparation_ * 0.5f);

    //should not get here, but removes a warning
    return tgt::vec3(0.f);
}

tgt::vec3 Camera::getStrafeWithOffsets() const {
    tgt::vec3 pos = position_;
    if(useOffset_)
        pos += offset_;
    return normalize(cross(focus_ - pos, upVector_));
}

tgt::vec3 Camera::getFocusWithOffsets() const {
    tgt::vec3 foc = focus_;
    if(eyeMode_ != EYE_MIDDLE) {
        tgt::vec3 shift = getStereoShift();
        tgt::vec3 strafe = getStrafeWithOffsets();
        float moveXScalar = tgt::dot(shift, strafe);
        float moveYScalar = tgt::dot(shift, upVector_);
        foc += moveXScalar * strafe + moveYScalar * upVector_;
    }
    return foc;
}

tgt::vec3 Camera::getPositionWithOffsets() const {
    tgt::vec3 pos = position_;
    if(eyeMode_ != EYE_MIDDLE)
        pos += getStereoShift();
    if(useOffset_);
        pos += offset_;
    return pos;
}

tgt::Frustum Camera::stereoFrustumShift() const {
    Frustum f = frust_;
    tgt::vec3 shift = getStereoShift();

    float moveXScalar = tgt::dot(shift, getStrafe());
    float moveYScalar = tgt::dot(shift, getUpVector());

    float oDist = getFocalLength();
    //float nDist = length(getPosition() - getFocus() + shift);
    float nDist = getFocalLength();

    f.setRight((getFrustRight() * oDist - moveXScalar * getNearDist())/nDist);
    //should be +dis... but left frustum value is < 0 so --=+
    f.setLeft((getFrustLeft() * oDist - moveXScalar * getNearDist())/nDist);
    f.setTop((getFrustTop() * oDist - moveYScalar * getNearDist())/nDist);
    //should be +dis... but left frustum value is < 0 so --=+
    f.setBottom((getFrustBottom() * oDist - moveYScalar * getNearDist())/nDist);
    return f;
}

} // namespace tgt
