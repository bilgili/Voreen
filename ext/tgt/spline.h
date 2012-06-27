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

#ifndef TGT_SPLINE_H_
#define TGT_SPLINE_H_

#include <vector>

#include "tgt/config.h"
#include "tgt/types.h"
#include "tgt/vector.h"
#include "tgt/curve.h"


namespace tgt {

/**
 * Uniform B-Spline of arbitrary degree, which is evaluated by Cox-deBoor recursion.
 * 
 * In order to force the spline to interpolate the first and last control point, 
 * degree+1 knot values are repeated at the beginning and the end of 
 * the knot vector, respectively. 
 *
 */
class BSpline : public Curve { 

public:
    
    /**
     * Constructor expecting the spline's control points. The inner knot values are 
     * distributed uniformly and range from 0.0 till 1.0.
     *
     * @param ctrlPoints The spline's control points. Note: The number of control points must
     *          exceed the spline's degree.
     * @param degree The spline's degree, i.e. the degree of its basis functions. By default,
     *          a cubic B-Spline is generated. Note: The degree must be greater zero.
     * @param stepCount Only relevant, if the spline is to be rendered: It specifies the number
     *          of line segments the spline is divided into.
     */
    BSpline(const std::vector<vec3>& ctrlPoints, int degree = 3, GLuint stepCount = 50);

    /**
     * Constructor expecting the spline's control points and the corresponding knot values.
     * The passed knot-vector is rescaled uniformly and shifted to match the interval [0.0, 1.0].
     *
     * @param ctrlPoints The spline's control points. Note: The number of control points must
     *          exceed the spline's degree.
     * @param knotValues The spline's knot values. Note: The number of knot values must match 
     *          the number of control points and must be passed in non-decreasing order.
     * @param degree The spline's degree, i.e. the degree of its basis functions. By default,
     *          a cubic B-Spline is generated. Note: The degree must be greater zero.
     * @param stepCount Only relevant, if the spline is to be rendered: It specifies the number
     *          of line segments the spline is divided into.
     */
    BSpline(const std::vector<vec3>& ctrlPoints, const std::vector<float>& knotValues, int degree = 3, GLuint stepCount = 50);

    /**
     * Evaluates the B-Spline at an arbitrary position t within the interval [0.0,1.0].
     */
    vec3 getPoint(GLfloat t);

    /**
     * Calculates the B-Spline's first derivative (tangent) at an arbitrary position t 
     * within the interval [0.0,1.0]. Not implemented, yet.
     */
    vec3 getDerivative(GLfloat t);

private:
    
    // Cox-deBoor recursion
    float evalBasisFunction(int knotID, int deg, float u);

    // generates the spline's knot values
    void generateKnotVector();

    // degree and control points
    int degree_;
    std::vector<vec3> ctrlPoints_;
    
    // knot vector (internally generated)
    std::vector<float> knots_;

};



/**
    A Spline-Class, that implements Curve-Class. It has a user-defined number of controlpoints that build the
    B-Spline curve.  This curve can, for example, be used to run a camera or a light source along itself.
*/
class Spline : public Curve {

public:
    //Spline() : Curve (100, Bounds(), true, true) {}
    /// The constructor, by default, sets an empty controlpoint-vector and a tau-value of .5
    Spline(const std::vector<vec3>& points = std::vector<vec3>(), float t = .5f, int stepCount = 100)
        : Curve(stepCount, Bounds(), true, true),
		  controlPoints_(points),
          tau_(t)
    {}

    /// This function yields the interpolated position between the current- and the next-position,
    /// depending on the interpolation-varibale t
    vec3 interpolateCurrent(float t, const vec3& previous,
        const vec3& current, const vec3& next, const vec3& afternext) const;

    /// This function yields the interpolated first derivation between the derivation at the current-
    /// and that at the next-position, depending on the interpolation-varibale t
    vec3 firstDeriveCurrent(float t, const vec3& previous,
        const vec3& current, const vec3& next, const vec3& afternext) const;

    /// This can be used to simply interpolate between all controlpoints that are currently in the
    /// controlpoint-vector
    vec3 interpolate(float t, float total = 1.f) const;

    /// This can be used to simply interpolate between all first derivations at the controlpoints
    vec3 firstDerive(float t, float total = 1.f) const;

    /// setter / getter
    void setTau(float t) { tau_ = t; }
    float getTau() const { return tau_; }

    std::vector<vec3>& getControlPoints() { return controlPoints_; }
    const std::vector<vec3>& getControlPoints() const { return controlPoints_; }

    /// Use this to add controlpoints to the spline.
    void addControlPoint(const vec3& p) { controlPoints_.push_back(p); }


	/* These methods implement the virtuals of the Curve-Class */

	//! Returns the point on the spline at the parameter value t.
	vec3 getPoint(GLfloat t){
		return interpolate(t);
	}

	//! Returns the first derivation (tangent) of the spline at the parameter value t.
	vec3 getDerivative(GLfloat t){
		return firstDerive(t);
	}

private:

    /// This std::vector contains the controlpoints
    std::vector<vec3> controlPoints_;

    /// tau is a measure for how tightly or loosely the curve follows the controlpoints.
    float tau_;
};


} // namespace tgt

#endif //#ifndef _SPLINE_H_

