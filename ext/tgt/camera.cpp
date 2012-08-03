/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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
      windowRatio_(1.f),
      projectionMode_(pm)
{
    viewMatrix_ = mat4::createLookAt(position, focus, up);
}

Camera::~Camera() {
}

Camera* Camera::clone() const {
    Camera* cam = new Camera(position_, focus_, upVector_, frust_.getFovy(),
        frust_.getRatio(), frust_.getNearDist(), frust_.getFarDist());
    cam->setWindowRatio(windowRatio_);
    return cam;
}

bool Camera::operator==(const Camera& rhs) const {
    return (rhs.position_ == position_) && (rhs.focus_ == focus_) && (rhs.upVector_ == upVector_) && (rhs.frust_ == frust_) &&
        (rhs.windowRatio_ == windowRatio_) && (rhs.projectionMode_ == projectionMode_);
}

bool Camera::operator!=(const Camera& rhs) const {
    return !(*this == rhs);
}

// This is called to set up the Camera-View
void Camera::look() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    updateFrustum();
    loadMatrix(getFrustumMatrix());
    //getProjectionMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    updateVM();
    loadMatrix(viewMatrix_);
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

mat4 Camera::getFrustumMatrix() const {
    return mat4::createFrustum(frust_.getLeft() * windowRatio_, frust_.getRight() * windowRatio_,
                               frust_.getBottom(), frust_.getTop(),
                               frust_.getNearDist(), frust_.getFarDist());
}

mat4 Camera::getProjectionMatrix() const {
    if(projectionMode_ == ORTHOGRAPHIC) {
        if(windowRatio_ > 1.0f)
            return mat4::createOrtho(frust_.getLeft() * windowRatio_, frust_.getRight() * windowRatio_,
                                     frust_.getTop(), frust_.getBottom(),
                                    -frust_.getNearDist(), frust_.getFarDist());
        else
            return mat4::createOrtho(frust_.getLeft(), frust_.getRight(),
                                     frust_.getTop() * (1.0f/windowRatio_), frust_.getBottom() * (1.0f/windowRatio_),
                                    -frust_.getNearDist(), frust_.getFarDist());
    } else if(projectionMode_ == PERSPECTIVE) {
        float fovy = frust_.getFovy();
        if(fovy < 6.f)
            fovy = 6.f;
        if(fovy > 175.f)
            fovy = 175.f;

        if(windowRatio_ >= 1.0f)
            return mat4::createPerspective(deg2rad(fovy), frust_.getRatio() * windowRatio_ , frust_.getNearDist(), frust_.getFarDist());
        else
            return mat4::createPerspective(atan(tan(deg2rad(fovy/2.f))/(windowRatio_* frust_.getRatio()))*2, frust_.getRatio() * windowRatio_ , frust_.getNearDist(), frust_.getFarDist());
    }
    else
        return getFrustumMatrix();
}

line3 Camera::getViewRay(ivec2 vp, ivec2 pixel) const {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];

    tgt::mat4 projection_tgt = getProjectionMatrix();
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

    tgt::mat4 projection_tgt = getProjectionMatrix();
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

} // namespace tgt
