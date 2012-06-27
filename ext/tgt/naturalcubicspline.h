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

#ifndef TGT_NATURALCUBICSPLINE_H
#define TGT_NATURALCUBICSPLINE_H

#include <vector>
#include "tgt/curve.h"
#include "tgt/vector.h"
#include "tgt/catmullromspline.h"

namespace tgt {

TGT_API std::vector<vec3> generateKnotPoints(const std::vector<vec2>& ctrlPoints);
TGT_API std::vector<vec4> generateKnotPoints(const std::vector<vec3>& ctrlPoints);

class TGT_API NaturalCubicSpline : public SplineTmp {
public:
    NaturalCubicSpline();
    NaturalCubicSpline(const std::vector<vec2>& points);

    void setPoints(const std::vector<vec2>& points);

    float getPoint(float t) const;

private:
    void computeCoefficients();
    int lookup(float x) const;

    std::vector<vec2> points_;
    std::vector<vec4> coefficients_;
};

} // namespace

#endif // TGT_NATURALCUBICSPLINE_H
