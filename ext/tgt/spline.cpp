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

#include "tgt/spline.h"
#include "tgt/assert.h"

#include <algorithm>

namespace tgt {


BSpline::BSpline(const std::vector<vec3>& ctrlPoints, int degree, GLuint stepCount) :
    Curve(stepCount, Bounds(), true, true),
    ctrlPoints_(ctrlPoints),
    degree_(degree) 
{
    tgtAssert(degree_ > 0, "Degree of a B-Spline must be greater zero.");
    tgtAssert(static_cast<int>(ctrlPoints_.size()) > degree_, 
        "The number of control points must exceed the spline's degree.");
   
    generateKnotVector();

}

BSpline::BSpline(const std::vector<vec3>& ctrlPoints, const std::vector<float>& knotValues, 
                 int degree, GLuint stepCount) :
    Curve(stepCount, Bounds(), true, true),
    ctrlPoints_(ctrlPoints),
    degree_(degree) 
{

    tgtAssert(degree_ > 0, "Degree of a B-Spline must be greater zero.");
    tgtAssert(static_cast<int>(ctrlPoints_.size()) > degree_, 
        "The number of control points must exceed the spline's degree.");
    tgtAssert(knotValues.size() == ctrlPoints_.size(), 
        "The number of knot values must match the number of control points.");
    tgtAssert( std::adjacent_find(knotValues.begin(), knotValues.end(), std::greater<float>()) == knotValues.end(),
        "Knot values must be in non-decreasing order");

    knots_ = knotValues;
    generateKnotVector();

}


vec3 BSpline::getPoint(GLfloat t) {
    
    // evaluate spline by Cox-deBoor recursion (also known as deCasteljau recursion) 
    vec3 point(0.f);
    for (size_t i=0; i<ctrlPoints_.size(); ++i) {
        point += evalBasisFunction(i, degree_, t)*ctrlPoints_.at(i);
    }

    return point;
}

vec3 BSpline::getDerivative(GLfloat /*t*/) {
    // TODO
    return vec3(0.f);
}

// private
float BSpline::evalBasisFunction(int knotID, int deg, float u) {

    // Cox-deBoor recursion

    if (deg == 0) {
        if ( u >= knots_.at(knotID) && u < knots_.at(knotID+1) )
            return 1.f;
        else
            return 0.f;
    } 
    else {
        float factor1, factor2;

        if (knots_.at(knotID+deg) == knots_.at(knotID))
            factor1 = 0.f;
        else
            factor1 = (u - knots_.at(knotID)) / (knots_.at(knotID+deg) - knots_.at(knotID));

        if (knots_.at(knotID+deg+1) == knots_.at(knotID+1))
            factor2 = 0.f;
        else
            factor2 = (knots_.at(knotID+deg+1) - u) / (knots_.at(knotID+deg+1) - knots_.at(knotID+1));

        return factor1*evalBasisFunction(knotID, deg-1, u) + factor2*evalBasisFunction(knotID+1, deg-1, u);
    }

}

// private
void BSpline::generateKnotVector() {

    if (knots_.empty()) {

        // No knot values have been passed => generate uniform distribution.
        // In order to force the spline to interpolate the start and end control points,
        // (degree_+1) knots of value 0.0 or 1.0 are inserted at the beginning and the end 
        // of the knot vector, respectively.

        // total number of knot values and number of inner knots
        int numKnots = ctrlPoints_.size() + degree_ + 1;
        int innerKnots = numKnots - 2*(degree_+1);

        // start knots (for interpolation of the first control point)
        for (int i=0; i<=degree_; ++i) {
            knots_.push_back(0.f);
        }

        // inner knots
        if (innerKnots > 0) {
            float step = 1.f / static_cast<float>(innerKnots+1);
            for (int i=1; i<=innerKnots; ++i) {
                knots_.push_back(i*step);
            }
        }

        // end knots (for interpolation of the last control point)
        for (int i=0; i<=degree_; ++i) {
            knots_.push_back(1.f + 1e-6);
        }

    }
    else {

        // scale and shift passed knot values to interval [0.0, 1.0] and
        // add (degree_+1)/2 knots at the beginning and end of the knot vector,
        // respectively.
        
        std::vector<float> tempKnots = knots_;
        knots_.clear();

        int knotsToAdd = degree_;

        int firstKnot = tgt::iceil((degree_+1)/2.f) - 1;
        int lastKnot = static_cast<int>(tempKnots.size()) - tgt::ifloor((degree_+1)/2.f);

        float minKnot = tempKnots.at(firstKnot);
        float maxKnot = tempKnots.at(lastKnot);
        
        float shift = -minKnot;
        float scale = (1.f + 1e-6f) / (maxKnot - minKnot);

        for (int i=0; i<knotsToAdd; ++i) {
            knots_.push_back(0.f);
        }

        for (int i=firstKnot; i<=lastKnot; ++i) {
            knots_.push_back( (tempKnots.at(i)+shift)*scale );
        }

        for (int i=0; i<knotsToAdd; ++i) {
            knots_.push_back(1.f + 1e-6f);
        }

    }
}


//-----------------------------------------------------------------------------


/*
    The points that are actually interpolated in this function are the "current" and "next" vectors.
    The previous and afternext are simply necessary to calculate the steepness of the curve before and
    after the interval.  The interpolating variable t can range from 0 ( = current point) to 1 ( = next
    point).
*/
vec3 Spline::interpolateCurrent(float t, const vec3& previous,
            const vec3& current, const vec3& next, const vec3& afternext) const {

    if (t < 0.f || t > 1.f)
        return vec3();


    float fac1, fac2, fac3, fac4;

    fac1 = -tau_*t + 2.f*tau_*t*t - tau_*t*t*t;
    fac2 = 1.f + (tau_ - 3.f)*t*t + (2.f - tau_)*t*t*t;
    fac3 = tau_*t + (3.f - 2.f*tau_)*t*t + (tau_ - 2.f)*t*t*t;
    fac4 = -tau_*t*t + tau_*t*t*t;

    vec3 res = previous  * fac1;
    res     += current   * fac2;
    res     += next      * fac3;
    res     += afternext * fac4;

    return res;
}

/*
    The derivatives that are actually interpolated in this function are the derivatives of the "current"
    and "next" vectors. The interpolating variable t can range from 0 ( = derivative of current point) to 1
    ( = derivative of next point).
*/
vec3 Spline::firstDeriveCurrent(float t, const vec3& previous,
            const vec3& current, const vec3& next, const vec3& afternext) const
{
    if (t < 0.f || t > 1.f)
        return vec3();

    float fac1, fac2, fac3, fac4;

    fac1 = -tau_ + 4.f*tau_*t - 3.f*tau_*t*t;
    fac2 = 2.f*(tau_ - 3.f)*t + 3.f*(2.f - tau_)*t*t;
    fac3 = tau_ + 2.f*(3.f - 2.f*tau_)*t + 3.f*(tau_ - 2.f)*t*t;
    fac4 = -2.f*tau_*t + 3.f*tau_*t*t;

    vec3 res = previous  * fac1;
    res     += current   * fac2;
    res     += next      * fac3;
    res     += afternext * fac4;

    return res;
}

/*
    This is used to interpolate between all controlpoints.  The usual range from 0 to 1 that
    the interpolating variable can have is mapped to the interpolation between the current and
    the next controlpoint; thus, t = 0 yields the first controlpoint (or, to be exact, the second
    point since the first one is not included in the actual interpolation) and t = 1 yields the
    next to last point (since the last point is also not included in the interpolation).  The ending
    of the spline at t = 1.0 is just a standard, you can change it by specifiying a total variable
    to have more control about how fast the interpolation takes place (for example, if you want a very
    slow interpolation, you would set total to a bigger number like 5.0 which would make the interpolation
    5 times slower than normal when maintaining constant steps for the running variable t).
*/
vec3 Spline::interpolate(float t, float total) const {

    // with less than 4 control-Points, we don't have engough information to form a B-Spline;
    // also, the running variable must not be smaller than zero or greater than the total-run-variable
    if (controlPoints_.size() < 4 || t < 0.f || t > total)
        return vec3();

    // This is the part of the whole 0-1-range that represents a single interpolation between two points.
    float piece = total / (controlPoints_.size() - 3);

    // What is the current controlpoint? (+1 because we start a the second point in the controlPoint-Vector,
    // the first is not part of the curve)
    uint current = uint(std::floor(t / piece)) + 1;

    // In the current interpolation, how far have we gotten up to now?
    float interpol = fmodf(t, piece);

    if (current > 0 && current < (controlPoints_.size() - 1)) {
        vec3 res = interpolateCurrent(interpol / piece,  // this actually maps the piece of the "big" 0-1-range to the current 0-1-range
                               controlPoints_[current - 1],
                               controlPoints_[current],
                               controlPoints_[current + 1],
                               controlPoints_[std::min(static_cast<int>(current) + 2, static_cast<int>(controlPoints_.size())-1)]);

        return res;
    }

    tgtAssert( false, "The code should not run here.");
    return vec3();
}

/*
    This is used to interpolate the first derivatives of all controlpoints which is useful, for example, for a camera
    that is supposed to look in the direction in which the spline progresses.  See comments to the above function.
*/
vec3 Spline::firstDerive(float t, float total) const {

    // with less than 4 control-Points, we don't have engough information to form an interpolation;
    // also, the running variable must not be smaller than zero or greater than the total-run-variable
    if (controlPoints_.size() < 4  || t < 0.f || t > total)
        return vec3();

    float piece = total / (controlPoints_.size() - 3);

    uint current = uint(std::floor(t / piece)) + 1;
    float interpol = fmodf(t, piece);

    if (current > 0 && current < (controlPoints_.size() - 1)) {
        vec3 res = firstDeriveCurrent(interpol / piece,
                               controlPoints_[current - 1],
                               controlPoints_[current],
                               controlPoints_[current + 1],
                               controlPoints_[current + 2]);

        return res;
    }

    tgtAssert( false, "The code should not run here.");
    return vec3();
}



} // namespace

