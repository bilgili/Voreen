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

#ifndef VRN_POINTGEOMETRY_H
#define VRN_POINTGEOMETRY_H

#include "voreen/core/datastructures/geometry/geometry.h"

namespace voreen {

/**
 * Used to store a single point in the geometry container.
 */
template<class T>
class PointGeometry : public Geometry {
public:
    PointGeometry() {}
    virtual ~PointGeometry() {}
    virtual void render() = 0;

    const T& getData() const { return point_; }
    void setData(const T& point) { point_ = point; }

protected:
    T point_;
};


//----------------------------------------------------------------------------


/**
 * Used to store a single tgt::vec3 in the geometry container.
 */
class TGTVec3PointGeometry : public PointGeometry<tgt::vec3> {
public:
    TGTVec3PointGeometry() {}
    virtual ~TGTVec3PointGeometry() {}
    virtual void render() {};
};


//----------------------------------------------------------------------------


/**
 * Used to store a single tgt::vec4 in the geometry container.
 */
class TGTVec4PointGeometry : public PointGeometry<tgt::vec4> {
public:
    TGTVec4PointGeometry() {}
    virtual ~TGTVec4PointGeometry() {}
    virtual void render() {};
};

} // namespace

#endif // VRN_POINTGEOMETRY_H
