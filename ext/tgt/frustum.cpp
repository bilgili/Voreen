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

#include "tgt/frustum.h"

#include "tgt/camera.h"
#include "tgt/assert.h"

namespace tgt {

Frustum::Frustum(float fovy, float ratio, float nearDist, float farDist)
  : nearDist_(nearDist),
    farDist_ (farDist )
{
    setFovy(fovy);
    setRatio(ratio);
}

Frustum::Frustum(float left, float right, float bottom, float top, float nearDist, float farDist)
  : lnear_(left  ),
    rnear_(right ),
    bnear_(bottom),
    tnear_(top   ),
    nearDist_(nearDist),
    farDist_ (farDist )
{}

void Frustum::update(Camera* c) {
    campos_ = c->getPosition();

    normals_[FARN]  = normalize(c->getLook()); // FIXME: we do not need to normalize -- Camera::getLook() always returns a normalized vector anyway
    normals_[NEARN] = -1.f*normals_[FARN];

    vec3 cam2near = normals_[FARN] * nearDist_;

    nearp_ = campos_ + cam2near;
    farp_  = campos_ + normals_[FARN] * farDist_;

    vec3 up = normalize(c->getUpVector()); // FIXME: we do not need to normalize -- Camera::getUpVector() always returns a normalized vector anyway
    vec3 strafe = normalize(c->getStrafe());// FIXME: we do not need to normalize -- Camera::getStrafe() always returns a normalized vector anyway
    vec3 tmp;
	
    tmp = cam2near + tnear_ * cross(strafe, c->getLook());
    normals_[TOPN]    = normalize( cross(strafe, tmp) );

    tmp = cam2near + bnear_ * cross(strafe, c->getLook());
    normals_[BOTTOMN] = normalize( cross(tmp, strafe) );

    tmp = cam2near + lnear_ * strafe;
    normals_[LEFTN]   = normalize( cross(cross(strafe, c->getLook()), tmp) );

    tmp = cam2near + rnear_ * strafe;
    normals_[RIGHTN]  = normalize( cross(tmp, cross(strafe, c->getLook())) );
}

// Is this bounding-box culled?
bool Frustum::isCulledXZ(const Bounds& bounds) const {
    vec3 urb = bounds.getURB();
    vec3 llf = bounds.getLLF();

    vec3 points[4];
    points[0] = vec3(llf.x, 0.f, llf.z);
    points[1] = vec3(urb.x, 0.f, llf.z);
    points[2] = vec3(urb.x, 0.f, urb.z);
    points[3] = vec3(llf.x, 0.f, urb.z);

    // all 6 sides of the frustum are tested against all four corners
    // of the bounding box. If all four points of the box are outside of
    // a frustum-plane, then the bounding box is not visible and we return
    // true.  If the box survives all tests then it is visible.
    for(size_t i = 0; i < 6; i++) {
        int outside = 0;
        for(size_t j = 0; j < 4; j++) {
            vec3 pos = (i < 4) ? campos_
                : ((i == 4) ? nearp() : farp());
            if (dot(getNormal(i), points[j] - pos) >= 0.f)
                ++outside;
        }
        if (outside == 4)
            return true;
    }
    return false;
}

// TODO: Seems to work, but not really tested.
bool Frustum::isCulled(const Bounds& bounds) const {
    vec3 urb = bounds.getURB();
    vec3 llf = bounds.getLLF();

    vec3 points[8];
    points[0] = vec3(llf.x, llf.y, llf.z);
    points[1] = vec3(urb.x, llf.y, llf.z);
    points[2] = vec3(urb.x, llf.y, urb.z);
    points[3] = vec3(llf.x, llf.y, urb.z);
    points[4] = vec3(llf.x, urb.y, llf.z);
    points[5] = vec3(urb.x, urb.y, llf.z);
    points[6] = vec3(urb.x, urb.y, urb.z);
    points[7] = vec3(llf.x, urb.y, urb.z);

    for(size_t i = 0; i < 6; i++) {
        int outside = 0;
        for(size_t j = 0; j < 8; j++) {
            vec3 pos = (i < 4) ? campos_
                : ((i == 4) ? nearp() : farp());
            if (dot(getNormal(i), points[j] - pos) >= 0.f)
                ++outside;
        }
        if (outside == 8)
            return true;
    }

    return false;

}

// Is this point culled?
bool Frustum::isCulled(const vec3& v) const {

    // the position of the camera is lying on the top, bottom, left, and right
    // plane, so we might as well use it as the reference point for those planes
    vec3 dist = v - campos_;
    // test the point against all 6 planes of the frustum (there are more
    // efficient algorithms for this, if you deem this to slow knock yourself out and
    // code something fancy)
    if (dot(leftn(),   dist) >= 0.f) return true;
    if (dot(rightn(),  dist) >= 0.f) return true;
    if (dot(bottomn(), dist) >= 0.f) return true;
    if (dot(topn(),    dist) >= 0.f) return true;
    if (dot(farn(),    v - farp()) >= 0.f) return true;
    if (dot(nearn(),   v - nearp() ) >= 0.f) return true;

    // all tests survived? then the point is visible
    return false;
}

}; // namespace tgt
