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

#ifndef VRN_SEGMENTLISTGEOMETRY_H
#define VRN_SEGMENTLISTGEOMETRY_H

#include "voreen/core/datastructures/geometry/geometry.h"
#include "tgt/glmath.h"
#include <vector>

namespace voreen {

template<class T>
class PointSegmentListGeometry : public Geometry {

public:

    PointSegmentListGeometry() :
      Geometry(),
      numPoints_(0)
    { }

    virtual ~PointSegmentListGeometry() { }

    virtual void render() {
        glBegin(GL_POINTS);
        for (size_t i=0; i < segmentList_.size(); ++i){
            for (size_t j=0; j < segmentList_[i].size(); ++j){
                // assuming data type stored in the segment list is compatible to tgt::vertex
                // if not: template instantiation will fail (compile error)
                tgt::vertex(segmentList_[i][j]);
            }
        }
        glEnd();
    }

    void setData(std::vector< std::vector<T> > segmentList) {

        segmentList_ = segmentList;
        pointList_.clear();
        numPoints_ = 0;
        for (size_t i=0; i<segmentList_.size(); ++i) {
            numPoints_ += static_cast<int>(segmentList_[i].size());
        }

        setHasChanged(true);
    }

    void addSegment(std::vector<T> segment) {
        segmentList_.push_back(segment);
        pointList_.clear();
        numPoints_ += static_cast<int>(segment.size());
        setHasChanged(true);
    }

    const std::vector< std::vector<T> >& getData() const { return segmentList_; }

    const std::vector<T>& getPoints() {

        // generate point list, if not present
        if (pointList_.empty() && numPoints_ > 0) {
            for (size_t i=0; i<segmentList_.size(); ++i) {
                pointList_.insert(pointList_.end(), segmentList_[i].begin(), segmentList_[i].end());
            }
        }

        return pointList_;
    }

    int getNumSegments() const { return static_cast<int>(segmentList_.size()); }

    int getNumPoints() const { return numPoints_; }

protected:

    // contains a list of segments, each segment consists of points
    std::vector< std::vector<T> > segmentList_;

    // contains the flattened segment list: a list of all points
    // is generated on first access
    std::vector<T> pointList_;

    int numPoints_;

};

typedef PointSegmentListGeometry<tgt::vec3> PointSegmentListGeometryVec3;

} // namespace

#endif // VRN_SEGMENTLISTGEOMETRY_H
