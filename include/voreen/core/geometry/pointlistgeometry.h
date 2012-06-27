/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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
#include <vector>

namespace voreen {

template<class T>
class PointListGeometry : public Geometry
{
    protected:
        std::vector<T>* points_;

    public:
        PointListGeometry() { points_ = new std::vector<T>(); }
        virtual ~PointListGeometry() { delete points_; points_ = 0; }
        virtual void draw() = 0;

        const std::vector<T>& getData() const { return *points_; }
        void setData(const std::vector<T>& points) { *points_ = points; }
        size_t getNumPoints() const { return points_.size(); }
};

} // namespace

#endif // VRN_POINTLISTGEOMETRY_H
