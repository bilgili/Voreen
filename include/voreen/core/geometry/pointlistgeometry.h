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

#ifndef VRN_POINTLISTGEOMETRY_H
#define VRN_POINTLISTGEOMETRY_H

#include "voreen/core/geometry/geometry.h"
#include "tgt/glmath.h"
#include <vector>

namespace voreen {

template<class T>
class PointListGeometry : public Geometry
{

public:
    PointListGeometry() : Geometry() { points_ = new std::vector<T>(); }
    virtual ~PointListGeometry() { delete points_; points_ = 0; }
    virtual void draw() {

        if (!points_)
            return;

        glBegin(GL_POINTS);
        for (size_t i=0; i < points_->size(); ++i){
            // assuming data type stored in the point list is compatible to tgt::vertex
            // if not: template instantiation will fail (compile error)
            tgt::vertex((*points_)[i]);
        }
        glEnd();
    }

    void addPoint(T point) {points_->push_back(point);}
    const std::vector<T>& getData() const { return *points_; }
    void setData(std::vector<T> points) { *points_ = points; }
    size_t getNumPoints() const { return points_->size(); }

protected:
    std::vector<T>* points_;

};

// -------------------------------------------------------------------------

class PointListGeometryVec3 : public PointListGeometry<tgt::vec3> {
public:
    PointListGeometryVec3() { }
    ~PointListGeometryVec3() { }

    void draw()
    {
        //vector<tgt::vec3>& points = static_cast< vector<tgt::vec3> >(points_);
        glBegin(GL_POINTS);
        for ( size_t i = 0; i < points_->size(); i++ )
        {
            //tgt::vec3& v = static_cast<tgt::vec3>(points[i]);
            tgt::vec3& v = this->points_->at(i);
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }
};

} // namespace

#endif // VRN_POINTLISTGEOMETRY_H
