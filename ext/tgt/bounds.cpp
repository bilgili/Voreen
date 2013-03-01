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

#include "tgt/bounds.h"

#include <algorithm>

#include "tgt/assert.h"

namespace tgt {

using std::min;
using std::max;

void Bounds::addPoint(const vec3& v) {
    if (points_ < 2) {
        ++points_;
        if (points_ == 1) {
            llf_ = v; urb_ = v;
            return;
        }
    }

    llf_ = min(llf_, v);
    urb_ = max(urb_, v);
}

void Bounds::addPoint(const vec4& v) {
    addPoint(v.xyz());
}

bool Bounds::insideXZ(const Bounds& bounds) const {
    tgtAssert(       isDefined(), "This Box ist not defined.");
    tgtAssert(bounds.isDefined(), "Box b ist not defined.");

    vec3 llfb = bounds.getLLF();
    vec3 urbb = bounds.getURB();

    float r0x = min(llf_[0], urb_[0]);
    float r1x = max(llf_[0], urb_[0]);
    float r0y = min(llf_[2], urb_[2]);
    float r1y = max(llf_[2], urb_[2]);
    float r2x = min(llfb[0], urbb[0]);
    float r3x = max(llfb[0], urbb[0]);
    float r2y = min(llfb[2], urbb[2]);
    float r3y = max(llfb[2], urbb[2]);

    return (r0x >= r2x) && (r0y >= r2y)
        && (r1x <= r3x) && (r1y <= r3y);
}

bool Bounds::insideXZ(const vec3& v) const {
    tgtAssert(  isDefined(), "This Box ist not defined.");

    return (llf_[0] <= v[0]) && (v[0] <= urb_[0])
        && (llf_[2] <= v[2]) && (v[2] <= urb_[2]);
}

bool Bounds::inside(const Bounds& bounds) const {
    tgtAssert(       isDefined(), "This Box ist not defined.");
    tgtAssert(bounds.isDefined(), "Box b ist not defined.");

    vec3 llfb = bounds.getLLF();
    vec3 urbb = bounds.getURB();

    float r0x = min(llf_[0], urb_[0]);
    float r1x = max(llf_[0], urb_[0]);
    float r0y = min(llf_[1], urb_[1]);
    float r1y = max(llf_[1], urb_[1]);
    float r0z = min(llf_[2], urb_[2]);
    float r1z = max(llf_[2], urb_[2]);

    float r2x = min(llfb[0], urbb[0]);
    float r3x = max(llfb[0], urbb[0]);
    float r2y = min(llfb[1], urbb[1]);
    float r3y = max(llfb[1], urbb[1]);
    float r2z = min(llfb[2], urbb[2]);
    float r3z = max(llfb[2], urbb[2]);

    return (r0x >= r2x) && (r1x <= r3x)
        && (r0y >= r2y) && (r1y <= r3y)
        && (r0z >= r2z) && (r1z <= r3z);
}

bool Bounds::inside(const vec3& v) const {
    tgtAssert(  isDefined(), "This Box ist not defined.");

    return (llf_[0] <= v[0]) && (v[0] <= urb_[0])
        && (llf_[1] <= v[1]) && (v[1] <= urb_[1])
        && (llf_[2] <= v[2]) && (v[2] <= urb_[2]);
}

Bounds Bounds::transform(const mat4& m) const {
    Bounds b;
    b.addPoint(m * vec3(llf_.x, llf_.y, llf_.z));
    b.addPoint(m * vec3(urb_.x, llf_.y, llf_.z));
    b.addPoint(m * vec3(llf_.x, urb_.y, llf_.z));
    b.addPoint(m * vec3(llf_.x, llf_.y, urb_.z));
    b.addPoint(m * vec3(urb_.x, urb_.y, llf_.z));
    b.addPoint(m * vec3(llf_.x, urb_.y, urb_.z));
    b.addPoint(m * vec3(urb_.x, llf_.y, urb_.z));
    b.addPoint(m * vec3(urb_.x, urb_.y, urb_.z));
    return b;
}

bool Bounds::intersects(const plane& p) const {
    bool pointsNeg = false;
    bool pointsPos = false;

    float d = p.distance(vec3(llf_.x, llf_.y, llf_.z));
    if(d < 0.0f) pointsNeg = true; else if(d > 0.0f) pointsPos = true;
    d = p.distance(vec3(urb_.x, llf_.y, llf_.z));
    if(d < 0.0f) pointsNeg = true; else if(d > 0.0f) pointsPos = true;
    d = p.distance(vec3(llf_.x, urb_.y, llf_.z));
    if(d < 0.0f) pointsNeg = true; else if(d > 0.0f) pointsPos = true;
    d = p.distance(vec3(llf_.x, llf_.y, urb_.z));
    if(d < 0.0f) pointsNeg = true; else if(d > 0.0f) pointsPos = true;
    d = p.distance(vec3(urb_.x, urb_.y, llf_.z));
    if(d < 0.0f) pointsNeg = true; else if(d > 0.0f) pointsPos = true;
    d = p.distance(vec3(llf_.x, urb_.y, urb_.z));
    if(d < 0.0f) pointsNeg = true; else if(d > 0.0f) pointsPos = true;
    d = p.distance(vec3(urb_.x, llf_.y, urb_.z));
    if(d < 0.0f) pointsNeg = true; else if(d > 0.0f) pointsPos = true;
    d = p.distance(vec3(urb_.x, urb_.y, urb_.z));
    if(d < 0.0f) pointsNeg = true; else if(d > 0.0f) pointsPos = true;

    return (pointsNeg && pointsPos);
}

/// ostream-operator
std::ostream& operator<< (std::ostream& o, const Bounds& b) {
    return (o << "(llf: " << b.getLLF() << " urb: " << b.getURB() << ")");
}

} // namespace

