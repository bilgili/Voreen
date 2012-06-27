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

#ifndef TGT_CURVE_H
#define TGT_CURVE_H

#include "tgt/config.h"
#include "tgt/assert.h"
#include "tgt/matrix.h"
#include "tgt/renderable.h"
#include "tgt/tgt_gl.h"


namespace tgt {


/// \todo Bounds

/**
 This class is an abstract superclass for spacecurves. \n
 It provides the virtual methods getPoint() and getDerivative(), which
 must be implemented by its subclasses. These methods evaluate
 the curve for a given parameter from the interval [0,1]. \n
 Curve implements Renderable's render() method, so this method
 does normally not need to be reimplemented by the subclasses.
 \note Bounds are not implemented yet!
*/
class Curve : public Renderable {

public:

    /// This struct is used to specifie the curves draw style.
    enum DrawStyle {
        LINE,      /*!< curve is rendered as a set of lines. (default) */
        POINT          /*!< curve is rendered as a set of points.   */
    };


    //! Constructor.
    /// \param stepCount Specifies into how many parts the curve is divided for rendering
    /// \param bounds Spacecurve's boundary. You can pass an empty boundary.
    /// \param _static True if the spacecurve is fix and will not be moved
    /// \param visible curve is only rendered, if visible == true
    Curve(GLuint stepCount, const Bounds &bounds, bool _static, bool visible)
            : Renderable (bounds, _static, visible)
    {
        setStepCount(stepCount);
        setDrawStyle(LINE);
    }

    virtual ~Curve() {}

    //! Evaluate the spacecurve for parameter t. \n
    /// Only values inside the range [0,1] are accepted.
    virtual vec3 getPoint(GLfloat t) = 0;

    //! Evaluate the spacecurve's first derivative (tangent) for parameter t. \n
    /// Only values inside the range [0,1] are accepted.
    virtual vec3 getDerivative(GLfloat t) = 0;

    /// Renders the whole spacecurve using the specified draw style (Lines by default)
    /// if curve is visible
    void render() {
		if (visible_){
			render(0.0, 1.0);
		}
	}

    //! Renders a part of the curve: Starting at startParam and ending at endParam.
    /// if curve is visible. \n
    /// Expected: 0 <= startParam <= endParam <= 1
    virtual void render(GLfloat startParam, GLfloat endParam);

    //! Step count is the number of parts the curve is divided into for rendering. \n
    /// Default value is 100.
    void setStepCount(GLuint stepCount) {
        tgtAssert(stepCount > 0, "Step count is expected to be greater zero.");
        stepCount_ = stepCount;
    }

    //! Get step count.
    GLuint getStepCount() {
        return stepCount_;
    }

    //! Sets the draw style for rendering.
    /// \see DrawStyle
    void setDrawStyle(DrawStyle drawStyle) {
        drawStyle_ = drawStyle;
    }

    //! Gets the draw style for rendering.
    /// \see DrawStyle
    DrawStyle getDrawStyle() {
        return drawStyle_;
    }

    virtual void init() {}
    virtual void deinit() {}

protected:

    /// This is the number of parts the curve is divided into for rendering.
    GLuint stepCount_;
    /// \see DrawStyle
    DrawStyle drawStyle_;
};


/**
 This class represents a circle spacecurve. \n
 It is determined by a radius, a center point and a normal vector,
 which specifies its orientation in space. The normal is perpendicular to the circle's plane. \n
 The direction of rotation is counterclockwise.
*/
class Circle : public Curve {

public:

    //! Standard Constructor. \n
    /// Creates a circle with radius 1.0, lying in the x-y-Plane and centered at the origin.
    Circle() : Curve(100, Bounds(), false, true) {
        setRadius(1.0);
        setCenter(vec3(0.0, 0.0, 0.0));
        setNormal(vec3(0.0, 0.0, 1.0));
        setRotationMatrix();
    }

    //! Constructor.
    /// Creates a circle with the specified properties.
    Circle(GLfloat radius, const vec3& center, const vec3& normal,
           GLuint stepCount = 100, bool _static = false, bool visible = true)
            : Curve(stepCount, Bounds(), _static, visible) {
        setRadius(radius);
        setCenter(center);
        setNormal(normal);
        setRotationMatrix();
    }

    //! Evaluate the circle for parameter t. \n
    /// The direction of rotation is counterclockwise. \n
    /// Only values inside the range [0,1] are accepted.
    vec3 getPoint(GLfloat t);

    //! Evaluate the circle's first derivative (tangent) for parameter t. \n
    /// Only values inside the range [0,1] are accepted.
    vec3 getDerivative(GLfloat t);

    /// Set radius. Must be positive.
    void setRadius(GLfloat radius) {
        tgtAssert(radius > 0.f, "Radius must be positive." );
        radius_ = radius;
    }

    /// Get radius.
    GLfloat getRadius() {
        return radius_;
    }

    /// Set center of the circle.
    void setCenter(const vec3& center) {
        center_ = center;
    }

    /// Get center of the circle.
    vec3 getCenter() {
        return center_;
    }

    //! Set the circle's normal vector in order to specifie its orientation. \n
    /// The circle lies in the plane, which is perpendicular to the normal. \n
    /// Normal vector must not be the nullvector.
    void setNormal(const vec3& normal);

    /// Get circle's normal vector.
    vec3 getNormal() {
        return normal_;
    }

private:

    // The circle's properties.
    GLfloat radius_;
    vec3 center_;
    vec3 normal_;

    // Circle points are calculated in x-y-plane and then rotated
    // according to the specified orientation.
    mat3 rotationMatrix_;
    void setRotationMatrix();

};


/**
 This class represents an ellipse spacecurve. \n
 It is determined by a center point, the semi major axis length, the semi minor axis length,
 a normal vector and the direction of one of the two semi major axis. Normal vector and
 semi major axis direction determine the ellipse's orientation in space. The normal vector is
 perpendicular to the plane the ellipse lies in. \n
 The direction of rotation is counterclockwise.

 \attention{ Class Ellipse also exists in Windows GDI. Declare objects of this class always as
             tgt::Ellipse to avoid namespace conflicts ! }
*/
class Ellipse : public Curve {

public:

    //! Standard constructor. \n
    /// Creates a circle with radius 1.0, lying in the x-y-plane and centered at the origin.
    /// Semi major axis direction is parallel to the x-axis.
    Ellipse() : Curve(100, Bounds(), false, true) {
        setCenter(vec3(0.0, 0.0, 0.0));
        setSemiMajorAxisLength(1.0);
        setSemiMinorAxisLength(1.0);
        setNormal(vec3(0.0, 0.0, 1.0));
        setSemiMajorAxisDirection(vec3(1.0, 0.0, 0.0));
    }

    /// Constructor. Creates an ellipse with the specified properties.
    Ellipse(const vec3& center, GLfloat semiMajorAxisLength, GLfloat semiMinorAxisLength,
               vec3 normal, vec3 semiMajorAxisDirection,
               GLuint stepCount = 100, bool _static = false, bool visible = true)
            : Curve(stepCount, Bounds(), _static, visible) {

        setCenter(center);
        setSemiMajorAxisLength(semiMajorAxisLength);
        setSemiMinorAxisLength(semiMinorAxisLength);
        setNormal(normal);
        setSemiMajorAxisDirection(semiMajorAxisDirection);
    }

    //! Evaluate the ellipse for parameter t. \n
    /// The direction of rotation is counterclockwise. \n
    /// Only values inside the range [0,1] are accepted.
    vec3 getPoint(GLfloat t);

    //! Evaluate the ellipse's first derivative (tangent) for parameter t. \n
    /// Only values inside the range [0,1] are accepted.
    vec3 getDerivative(GLfloat t);

    //! Set the ellipse's center.
    void setCenter(const vec3& center) {
        center_ = center;
    }

    //! Get the ellipse's center;
    vec3 getCenter() {
        return center_;
    }

    //! Sets the length of the semi major axis. Must be positive.
    void setSemiMajorAxisLength(GLfloat a) {
        tgtAssert(a > 0, "Semi major axis length must be positive.");
        a_ = a;
    }

    //! Gets the length of the semi major axis.
    GLfloat getSemiMajorAxisLength() {
        return a_;
    }


    //! Sets the length of the semi minor axis. Must be positive.
    void setSemiMinorAxisLength(GLfloat b) {
        tgtAssert(b > 0, "Semi minor axis length must be positive.");
        b_ = b;
    }

    //! Gets the length of the semi minor axis.
    GLfloat getSemiMinorAxisLength() {
        return b_;
    }

    //! Sets the ellipse's normal vector. \n
    /// The ellipse lies in the plane, which is perpendicular to the normal.
    void setNormal(const vec3& normal);

    //! Gets the ellipse's normal vector.
    vec3 getNormal() {
        return normal_;
    }

    //! Sets the direction of one of the semi major axis.
    /// If it is not perpendicular to the normal vector, it is
    /// projected into the plane which is perpendicular to the normal. \n
    /// The direction of the semi minor axis does not have
    /// to be set, as it is calculated automatically.
    void setSemiMajorAxisDirection(const vec3& semiMajor);

    //! Gets the semi major axis direction.
    vec3 getSemiMajorAxisDirection() {
        return semiMajor_;
    }

    //! Gets the semi minor axis direction.
    /// Cannot be set, as it is calculated automatically.
    vec3 getSemiMinorAxisDirection() {
        return semiMinor_;
    }

private:

    // Center and radii of the ellipse
    vec3 center_;
    GLfloat a_;
    GLfloat b_;

    // vectors determining its orientation in space
    vec3 normal_;
    vec3 semiMajor_;
    vec3 semiMinor_;

    // Ellipse points are calculated in x-y-plane and then rotated
    // according to the specified orientation.
    mat3 rotationMatrix_;
    void setRotationMatrix();

};

} //namespace tgt

#endif //TGT_CURVE_H
