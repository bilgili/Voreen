/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
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

#include <cmath>

#include "tgt/assert.h"
#include "tgt/glmath.h"
#include "tgt/quaternion.h"
#include "tgt/spline.h"
#include "tgt/tgt_gl.h"

#include <iostream>
namespace tgt {

// Constructor
Camera::Camera(const vec3& position, const vec3& focus, const vec3& up,
                    float fovy, float ratio, float distn, float distf)
    : position_      (position)
    , focus_         (focus)
    , upVector_      (normalize(up))
    , frust_         (Frustum(fovy, ratio, distn, distf))
    , eyesep_        (0.01f)
{
    viewMatrix_ = mat4::createLookAt(position, focus, up);
}

Camera::~Camera() {
}

// This is called to set up the Camera-View
void Camera::look(Eye eye) {
    if (eye == EYE_MIDDLE) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        updateFrustum();
        loadMatrix(getProjectionMatrix());
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        updateVM();
        loadMatrix(viewMatrix_);

    } else {
        // Stereo
// FIXME: call to update routines "updateViewMatrix", "updateFrustum"
// FIXME: stereo was not adjusted to new camera class yet
        const vec3 vp = getPosition();
        const vec3 vf = getFocus();
        const vec3 vu = getUpVector();
        vec3 vs = getStrafe();

        Frustum frustum = getFrustum();

        float focallength = length(vf-vp);
//         float ratio       = frustum.ratio_;
        float ndfl        = frustum.getNearDist() / focallength;
        double cnear      = frustum.getNearDist();

        if (cnear<0.1)
            cnear = 0.1; // avoid roundoff errors
        double cfar = frustum.getFarDist();

        // Derive the two eye positions
        vs.x *= eyesep_ / 2.f;
        vs.y *= eyesep_ / 2.f;
        vs.z *= eyesep_ / 2.f;
        if (eye == EYE_LEFT)
            vs = -vs;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        // off-axis projection
        float left, right;
        if (eye == EYE_RIGHT) {
            left  = frustum.getLeft() - 0.5f * eyesep_ * ndfl;
            right = frustum.getRight() - 0.5f * eyesep_ * ndfl;
        } else { // EYE_LEFT
            left  = frustum.getLeft() + 0.5f * eyesep_ * ndfl;
            right = frustum.getRight() + 0.5f * eyesep_ * ndfl;
        }
        float top    =  frustum.getTop();
        float bottom =  frustum.getBottom();

        glFrustum(left, right, bottom, top, cnear, cfar);

        // translation of both eyes
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(vp.x + vs.x, vp.y + vs.y, vp.z + vs.z,
                  vf.x + vs.x, vf.y + vs.y, vf.z + vs.z,
                  vu.x, vu.y, vu.z);
    }
}

// Private method that updates the relevant frustum parameters
void Camera::updateFrustum() {
    frust_.update(this);
}

mat4 Camera::getViewMatrix() const {
// TODO: remodel this for new stereo-support, FL
//     if (!stereo_)
    updateVM();
    return viewMatrix_;
//     else {
//         vec3 shift = getStrafe() * eyesep_ * 0.5f;
//         if (eye_ == EYE_LEFT)
//             shift = -shift;
//         mat4 m = viewMatrix_.getRotationalPart();
//         vec3 stereoPos = getPosition() + shift;
//         m.elem[3]  = -stereoPos.x;
//         m.elem[7]  = -stereoPos.y;
//         m.elem[11] = -stereoPos.z;
//         return m;
//     }
}

void Camera::setViewMatrix(mat4 mvMat) {
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

/*  near and far are defined by windef.h ==> renamed parameters: near -> pnear, far -> pfar      */

mat4 Camera::getStereoPerspectiveMatrix(Eye eye, float fov, float aspect, float pnear, float pfar) const {

    // This is taken from the OpenGL-Faq of the OpenGL-homepage
    float top    = tanf(fov * tgt::PIf / 360.0f) * pnear;
    float bottom = -top;

    float left = 0.f;
    float right = 0.f;

    float ndfl = pnear / getFocalLength();

    if (pnear < 0.1f)
        pnear = 0.1f; // avoid roundoff errors

    // off-axis projection
    if (eye == EYE_RIGHT) {
        left  = (-aspect*top) - 0.5f * eyesep_ * ndfl;
        right = ( aspect*top) - 0.5f * eyesep_ * ndfl;
    } else { // EYE_LEFT
        left  = (-aspect*top) + 0.5f * eyesep_ * ndfl;
        right = ( aspect*top) + 0.5f * eyesep_ * ndfl;
    }

    return mat4::createFrustum(left, right, top, bottom, pnear, pfar);
}

mat4 Camera::getFrustumMatrix() const {
	return mat4::createFrustum(frust_.getLeft(), frust_.getRight(),
							   frust_.getTop(), frust_.getBottom(),
							   frust_.getNearDist(), frust_.getFarDist());
}

mat4 Camera::getProjectionMatrix() const {
    return mat4::createPerspective( deg2rad(frust_.getFovy()), frust_.getRatio(), frust_.getNearDist(), frust_.getFarDist());
}

//------------------------------------------------------------------------------

OrthographicCamera::OrthographicCamera(const tgt::vec3 &position,
                                       const tgt::vec3 &focus,
                                       const tgt::vec3 &up,
                                       float left, float right, 
                                       float bottom, float top,
                                       float distn, float distf)

{
    focus_ = focus;
    position_ = position;
    upVector_ = normalize(up);
    frust_ = Frustum(left, right, bottom, top, distn, distf);
    eyesep_ = 0.01f;
    viewMatrix_ = mat4::createLookAt(position, focus, up);
}


mat4 OrthographicCamera::getProjectionMatrix() const {
    return mat4::createOrtho(frust_.getLeft(),     frust_.getRight(),
							 frust_.getTop(),      frust_.getBottom(),
							 frust_.getNearDist(), frust_.getFarDist());
}

} // namespace tgt
