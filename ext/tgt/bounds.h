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

#ifndef TGT_BOUNDS_H
#define TGT_BOUNDS_H

#include "tgt/types.h"
#include "tgt/vector.h"
#include "tgt/matrix.h"
#include "tgt/plane.h"

namespace tgt {

/**
*   Axis-aligned bounding box
*/
class TGT_API Bounds {
    unsigned int points_;   //points added to the box
    vec3 llf_; //lower left front
    vec3 urb_; //upper right back

    public:
    /**
    *   Constructs an undefined boundingbox
    */
    Bounds() { points_ = 0; }

    /**
    *   Constructs an undefined boundingbox containing v
    */
    Bounds(const vec3& v)
      : points_(1),
        llf_(v),
        urb_(v)
    {}

    /**
    *   Constructs a bounding box containing v1 and v2
    */
    Bounds(const vec3& v1, const vec3& v2)
      : points_(1),
        llf_(v1),
        urb_(v1) {

        addPoint(v2);
    }

    /**
    *   Enlarges the box (if necessary) to contain v
    */
    void addPoint(const vec3& v);
    void addPoint(const vec4& v);

    /**
    *   Enlarges the box (if necessary) to contain b
    */
    void addVolume(const Bounds& b) {
        addPoint(b.llf_);
        addPoint(b.urb_);
    }

    /**
    *   Returns the Lower Left Front point
    */
    Vector getLLF() const { return llf_; }

    /**
    *   Returns the Upper Right Back point
    */
    Vector getURB() const { return urb_; }

    /**
    *   Returns the center of this box
    */
    Vector center() const {
        return (diagonal() * 0.5f + llf_);
    }

    /**
    *   Returns the diagonal from llf to urb
    */
    Vector diagonal() const {
        return (urb_ - llf_);
    }

    /**
    *   Returns the volume of this box
    */
    float volume() const {
        return std::abs((llf_.x - urb_.x) * (llf_.y - urb_.y) * (llf_.z - urb_.z));
    }

    /**
    *   Returns true if box is defined (not only a point)
    */
    bool isDefined() const { return (points_ == 2); }

    /**
    *   Returns true if box is only a point
    */
    bool onlyPoint() const { return (points_ == 1); }

    /**
    *   Returns true if point is contained in this box
    */
    bool containsPoint(const Vector& p)
    {
        return ( (p.x >= llf_.x) && (p.y >= llf_.y) && (p.z >= llf_.z)
                    && (p.x <= urb_.x) && (p.y <= urb_.y) && (p.z <= urb_.z) );
    }

    /**
    *   Returns true if b is contained in this box
    *   Box has to be defined!
    */
    bool containsVolume(const Bounds& b) {
        return ( containsPoint(b.llf_) && containsPoint(b.urb_) );
    }

    /**
    *   Returns true if the boxes intersect
    *   Box has to be defined!
    */
    bool intersects(const Bounds& b) const {
        // Look for a separating axis on each box for each axis
        if ((llf_.x > b.urb_.x) || (b.llf_.x > urb_.x)) return false;
        if ((llf_.y > b.urb_.y) || (b.llf_.y > urb_.y)) return false;
        if ((llf_.z > b.urb_.z) || (b.llf_.z > urb_.z)) return false;

        // No separating axis ... they must intersect
        return true;
   }

    bool intersects(const plane& p) const;

    bool insideXZ(const Bounds& bounds) const;
    bool insideXZ(const vec3& v) const;

    ///Returns true if bounds is inside this.
    bool inside(const Bounds& bounds) const;
    ///Returns true if v is inside this.
    bool inside(const vec3& v) const;

    /**
     * Transform this BB using m.
     *
     * @return A BoundingBox containing the 8 vertices defined by this BB, transformed using m.
     */
    Bounds transform(const mat4& m) const;
};

class TGT_API HasBounds {
public:
    HasBounds(const Bounds& bounds)
      : boundingBox_(bounds)
    {}

    HasBounds()
      : boundingBox_(Bounds())
    {}

    /**
    *   Returns the boundingbox.
    */
    const Bounds& getBounds() const {
        return boundingBox_;
    }

protected:
    Bounds boundingBox_;
};

/// ostream-operator
TGT_API std::ostream& operator<< (std::ostream& o, const Bounds& b);

} // namespace

#endif //TGT_BOUNDS_H
