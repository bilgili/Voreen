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

#ifndef VRN_VERTEXGEOMETRY_H
#define VRN_VERTEXGEOMETRY_H

#include "voreen/core/datastructures/geometry/geometry.h"

#include "tgt/vector.h"
#include "tgt/matrix.h"

namespace voreen {

/**
 * Represents a vertex geometry containing vertex coordinates, texture coordinates and vertex color.
 *
 * @attention Each function which possibly change the vertex geometry sets the @c hasChanged flag
 *            to @c true, even if nothing has changed at all.
 *
 * @see FaceGeometry
 */
class VertexGeometry : public Geometry {
public:
    /**
     * Instantiates a vertex geometry with the given vertex coordinates, texture coordinates
     * and vertex color.
     *
     * @param coords the vertex coordinates
     * @param texcoords the texture coordinates
     * @param color the vertex color
     */
    VertexGeometry(
        const tgt::vec3& coords = tgt::vec3(0, 0, 0),
        const tgt::vec3& texcoords = tgt::vec3(0, 0, 0),
        const tgt::vec4& color = tgt::vec4(0, 0, 0, 0));

    /**
     * Returns the vertex coordinates of the vertex geometry.
     *
     * @returns the vertex coordinates
     */
    tgt::vec3 getCoords() const;

    /**
     * Sets the vertex coordinates of the vertex geometry.
     *
     * @param coords the vertex coordinates
     */
    void setCoords(const tgt::vec3& coords);

    /**
     * Returns the texture coordinates of the vertex geometry.
     *
     * @returns the texture coordinates
     */
    tgt::vec3 getTexCoords() const;

    /**
     * Sets the texture coordinates of the vertex geometry.
     *
     * @param texcoords the texture coordinates
     */
    void setTexCoords(const tgt::vec3& texcoords);

    /**
     * Returns the vertex color of the vertex geometry.
     *
     * @returns the vertex color
     */
    tgt::vec4 getColor() const;

    /**
     * Sets the vertex color of the vertex geometry.
     *
     * @param color the vertex color
     */
    void setColor(const tgt::vec4& color);

    /**
     * Sets the vertex color of the vertex geometry.
     *
     * @param color the vertex color
     */
    void setColor(const tgt::vec3& color);

    /**
     * Returns the euclidean length of the vertex coordinates vector.
     *
     * @returns the euclidean vertex coordinates vector length
     */
    double getLength() const;

    /**
     * Returns the distance between the vertex geometry and the given plane.
     *
     * @note Use the @c epsilon parameter to change the accuracy at which
     *       the vertex geometry lies on the given plane.
     *
     * @param plane the plane
     * @param epsilon the accuracy at which the vertex geometry lies on the given plane
     *
     * @returns distance between vertex geometry and given plane
     */
    double getDistanceToPlane(const tgt::vec4& plane, double epsilon = 1e-6) const;

    /**
     * Returns the distance between this and the given vertex geometry.
     *
     * @param vertex a vertex geometry
     *
     * @returns distance between this and the given vertex geometry
     */
    double getDistance(const VertexGeometry& vertex) const;

    /**
     * @see Geometry::render
     */
    virtual void render();

    /**
     * Combines this and the given vertex geometry.
     *
     * @par
     * Combining two vertex geometries means combining every contained vector like
     * vertex coordinates vector or texture coordinates vector.
     *
     * @note Combining two vertex geometries is equal to an interpolation with a factor of 0.5.
     *
     * @param vertex a vertex geometry
     */
    void combine(const VertexGeometry& vertex);

    /**
     * Combines two vertex geometries.
     *
     * @par
     * Combining two vertex geometries means combining each contained vector like
     * vertex coordinates vector or texture coordinates vector.
     *
     * @note Combining two vertex geometries is equal to an interpolation with a factor of 0.5.
     *
     * @param vertex1 the first vertex geometry
     * @param vertex2 the second vertex geometry
     *
     * @returns the combined vertex geometry
     */
    static VertexGeometry combine(const VertexGeometry& vertex1, const VertexGeometry& vertex2);

    /**
     * Interpolates between this and the given vertex geometry.
     *
     * @par
     * Interpolation between two vertex geometries means interpolation between each
     * contained vector like vertex coordinates vector or texture coordinates vector.
     *
     * @param vertex a vertex geometry
     * @param t interpolation factor
     */
    void interpolate(const VertexGeometry& vertex, double t);

    /**
     * Interpolates between two vertex geometries.
     *
     * @par
     * Interpolation between two vertex geometries means interpolation between each
     * contained vector like vertex coordinates vector or texture coordinates vector.
     *
     * @param vertex1 the first vertex geometry
     * @param vertex2 the second vertex geometry
     * @param t interpolation factor
     *
     * @returns the interpolated vertex geometry.
     */
    static VertexGeometry interpolate(const VertexGeometry& vertex1, const VertexGeometry& vertex2, double t);

    /**
     * Transforms the vertex geometry using the given transformation matrix.
     *
     * @param transformation the transformation matrix
     */
    void transform(const tgt::mat4& transformation);

    /**
     * Returns whether this and the given vertex geometry are equal.
     *
     * @note Use the @c epsilon parameter to change the accuracy at which
     *       two vertex geometries are equal.
     *
     * @param vertex a vertex geometry
     * @param epsilon the accuracy at which two vertex geometries are equal
     *
     * @returns @c true if this and the given vertex geometry are equal, otherwise @c false
     */
    bool equals(const VertexGeometry& vertex, double epsilon = 1e-6) const;

    /**
     * Returns whether this and the given vertex geometry are equal.
     *
     * @note The equals function with default accuracy is used for the comparison.
     *
     * @param vertex a vertex geometry
     *
     * @returns @c true if this and the given vertex geometry are equal, otherwise @c false
     */
    bool operator==(const VertexGeometry& vertex) const;

    /**
     * Returns whether this and the given vertex geometry are unequal.
     *
     * @note The equals function with default accuracy is used for the comparison.
     *
     * @param vertex a vertex geometry
     *
     * @returns @c true if this and the given vertex geometry are unequal, otherwise @c false
     */
    bool operator!=(const VertexGeometry& vertex) const;

private:
    /**
     * Vertex coordinates of this vertex geometry.
     */
    tgt::vec3 coords_;

    /**
     * Texture coordinates of this vertex geometry.
     */
    tgt::vec3 texcoords_;

    /**
     * Vertex color of this vertex geometry.
     */
    tgt::vec4 color_;
};

}    // namespace

#endif  //VRN_VERTEXGEOMETRY_H
