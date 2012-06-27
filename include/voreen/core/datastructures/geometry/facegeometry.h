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

#ifndef VRN_FACEGEOMETRY_H
#define VRN_FACEGEOMETRY_H

#include <vector>

#include "voreen/core/datastructures/geometry/geometry.h"
#include "voreen/core/datastructures/geometry/vertexgeometry.h"

namespace voreen {

/**
 * Represents a face geometry containing a list of vertex geometries.
 *
 * @note The following assumptions has to be fulfilled:
 *       (1) the vertex list represents a convex polygon, which is automatically closed;
 *       (2) each vertex lies in the same plane;
 *       (3) counter clockwise vertex order determines the front face of the face geometry.
 *
 * @attention Each function which possibly change the face geometry sets the @c hasChanged flag
 *            to @c true, even if nothing has changed at all.
 *
 * @attention If the face geometry is changed by using the @c operator[]
 *            or the @c getVertex function then @c setHasChanged(true)
 *            has to be called manually.
 *
 * @par
 * Here is a short example of using the @c FaceGeometry:
 * @code
 * FaceGeometry face;
 * face.addVertex(VertexGeometry(tgt::vec3( 1,  1,  0)));
 * face.addVertex(VertexGeometry(tgt::vec3(-1,  1,  0)));
 * face.addVertex(VertexGeometry(tgt::vec3(-1, -1,  0)));
 * face.addVertex(VertexGeometry(tgt::vec3( 1, -1,  0)));
 *
 * face.clip(tgt::vec4(0, 1, 0, 0));
 * face.render();
 * @endcode
 *
 * @see Geometry
 * @see VertexGeometry
 * @see MeshGeometry
 */
class FaceGeometry : public Geometry {
public:
    /**
     * Type of the vertex geometry list.
     */
    typedef std::vector<VertexGeometry> VertexListType;

    /**
     * Type of the vertex geometry list iterator.
     */
    typedef VertexListType::iterator iterator;

    /**
     * Type of the const vertex geometry list iterator.
     */
    typedef VertexListType::const_iterator const_iterator;

    /**
     * Default constructor.
     */
    FaceGeometry();

    /**
     * Returns the number of vertex geometries contained by this face geometry.
     *
     * @returns vertex geometry count
     */
    size_t getVertexCount() const;

    /**
     * Returns true, if the face's vertex count is zero.
     *
     * @see getVertexCount
     */
    bool empty() const;

    /**
     * Adds the given vertex geometry to this face geometry.
     *
     * @param vertex the vertex geometry
     */
    void addVertex(const VertexGeometry& vertex);

    /**
     * Returns the vertex geometry at the given @c index.
     *
     * @attention If the face geometry is changed by using the @c operator[]
     *            or the @c getVertex function then @c setHasChanged(true)
     *            has to be called manually.
     *
     * @param index the vertex geometry index
     *
     * @returns the vertex geometry at the given @c index
     */
    VertexGeometry& getVertex(size_t index);

    /**
     * Removes all vertex geometries form this face geometry.
     */
    void clear();

    /**
     * Returns an iterator pointing to the first vertex geometry of this face geometry.
     *
     * @returns an iterator pointing to the first vertex geometry
     */
    iterator begin();

    /**
     * Returns a const iterator pointing to the first vertex geometry of this face geometry.
     *
     * @returns a const iterator pointing to the first vertex geometry
     */
    const_iterator begin() const;

    /**
     * Returns an iterator pointing behind the last vertex geometry of this face geometry.
     *
     * @returns an iterator pointing behind the last vertex geometry
     */
    iterator end();

    /**
     * Returns a const iterator pointing behind the last vertex geometry of this face geometry.
     *
     * @returns a const iterator pointing behind the last vertex geometry
     */
    const_iterator end() const;

    /**
     * Returns the vertex geometry at the given @c index.
     *
     * @attention If the face geometry is changed by using the @c operator[]
     *            or the @c getVertex function then @c setHasChanged(true)
     *            has to be called manually.
     *
     * @param index the index
     *
     * @returns the vertex geometry at the given @c index
     */
    VertexGeometry& operator[](size_t index);

    /**
     * @see Geometry::render
     */
    virtual void render();

    /**
     * Transforms the face geometry using the given transformation matrix.
     *
     * @param transformation the transformation matrix
     */
    void transform(const tgt::mat4& transformation);

    /**
     * Clips the face geometry by the given arbitrary clipping plane.
     *
     * @par
     * The arbitrary clipping plane is represented by the equation
     * NX - c = 0 where N is the normal vector of the clipping plane
     * and corresponds to the x-, y-, and z-coordinate of the given vector,
     * and c is a scalar corresponding to the w-coordinate of the given
     * vector and X is any point on the plane.
     *
     * @note It is not necessary that the clipping plane normal vector
     *       is normalized, but it may improve computational accuracy.
     *
     * @note Use the @c epsilon parameter to change the accuracy of
     *       vertex- / vertex geometry and vertex geometry / clipping plane comparison.
     *
     * @note The clipping algorithm is based on the paper "Clipping a Mesh Against a Plane"
     *       by David Eberly. For further information see:
     *       http://www.geometrictools.com/Documentation/ClipMesh.pdf
     *
     * @param clipplane the arbitrary clipping plane
     * @param epsilon the accuracy for vertex geometry comparison
     */
    void clip(const tgt::vec4& clipplane, double epsilon = 1e-6);

private:
    /**
     * Vertex geometry list.
     */
    VertexListType vertices_;
};

}    // namespace

#endif  //VRN_FACEGEOMETRY_H
