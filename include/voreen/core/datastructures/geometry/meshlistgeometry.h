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

#ifndef VRN_MESHLISTGEOMETRY_H
#define VRN_MESHLISTGEOMETRY_H

#include <vector>

#include "voreen/core/datastructures/geometry/geometry.h"
#include "voreen/core/datastructures/geometry/meshgeometry.h"

namespace voreen {

/**
 * Represents a mesh geometry containing a list of mesh geometries.
 *
 * @attention Each function which possibly change the face geometry sets the @c hasChanged flag
 *            to @c true, even if nothing has changed at all.
 *
 * @attention If the mesh list geometry is changed by using the @c operator[]
 *            or the @c getMesh function then @c setHasChanged(true)
 *            has to be called manually.
 *
 * @par
 * Here is a short example of using the @c FaceGeometry:
 * @code
 * MeshListGeometry meshes;
 * meshes.addMesh(MeshGeometry::createCube());
 * meshes.addMesh(MeshGeometry::createCube(tgt::vec3(1, -1, 1), tgt::vec3(3, 1, -1)));
 *
 * meshes.clip(tgt::vec4(0, 1, 0, 0));
 * meshes.render();
 * @endcode
 *
 * @see Geometry
 * @see MeshGeometry
 * @see FaceGeometry
 * @see VertexGeometry
 */
class MeshListGeometry : public Geometry {
public:
    /**
     * Type of the mesh geometry list.
     */
    typedef std::vector<MeshGeometry> MeshListType;

    /**
     * Type of the mesh geometry list iterator.
     */
    typedef MeshListType::iterator iterator;

    /**
     * Type of the const mesh geometry list iterator.
     */
    typedef MeshListType::const_iterator const_iterator;

    /**
     * Default constructor.
     */
    MeshListGeometry();

    /**
     * Returns the number of mesh geometries contained by this mesh list geometry.
     *
     * @returns mesh geometry count
     */
    size_t getMeshCount() const;

    /**
     * Returns the number of mesh faces contained by this mesh list geometry,
     * i.e. the sum of its meshes' face counts.
     *
     * @see MeshGeometry::getFaceCount
     */
    size_t getFaceCount() const;

    /**
     * Returns the number of mesh vertices contained by this mesh list geometry,
     * i.e. the sum of its meshes' vertex counts.
     *
     * @see MeshGeometry::getVertexCount
     */
    size_t getVertexCount() const;

    /**
     * Returns true, if the geometry's vertex count is zero.
     *
     * @see getVertexCount
     */
    bool empty() const;

    /**
     * Adds the given mesh geometry to this mesh list geometry.
     *
     * @param mesh the mesh geometry
     */
    void addMesh(const MeshGeometry& mesh);

    /**
     * Returns the mesh geometry at the given @c index.
     *
     * @attention If the mesh list geometry is changed by using the @c operator[]
     *            or the @c getMesh function then @c setHasChanged(true)
     *            has to be called manually.
     *
     * @param index the mesh geometry index
     *
     * @returns the mesh geometry at the given @c index
     */
    MeshGeometry& getMesh(size_t index);

    /**
     * Removes all mesh geometries from this mesh list geometry.
     */
    void clear();

    /**
     * Returns an iterator pointing to the first mesh geometry of this mesh list geometry.
     *
     * @returns an iterator pointing to the first mesh geometry
     */
    iterator begin();

    /**
     * Returns a const iterator pointing to the first mesh geometry of this mesh list geometry.
     *
     * @returns a const iterator pointing to the first mesh geometry
     */
    const_iterator begin() const;

    /**
     * Returns an iterator pointing behind the last mesh geometry of this mesh list geometry.
     *
     * @returns an iterator pointing behind the last mesh geometry
     */
    iterator end();

    /**
     * Returns a const iterator pointing behind the last mesh geometry of this mesh list geometry.
     *
     * @returns a const iterator pointing behind the last mesh geometry
     */
    const_iterator end() const;

    /**
     * Returns the mesh geometry at the given @c index.
     *
     * @attention If the mesh list geometry is changed by using the @c operator[]
     *            or the @c getMesh function then @c setHasChanged(true)
     *            has to be called manually.
     *
     * @param index the mesh geometry index
     *
     * @returns the mesh geometry at the given @c index
     */
    MeshGeometry& operator[](size_t index);

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
     * Calls the @c clip function of each contained mesh geometry.
     *
     * @see MeshGeometry::clip
     *
     * @param clipplane an arbitrary clipping plane
     * @param epsilon the accuracy for vertex geometry comparison
     *
     * @return The clipping faces generated for closing the clipped meshes.
     */
    MeshListGeometry clip(const tgt::vec4& clipplane, double epsilon = 1e-6);

private:
    /**
     * Mesh geometry list.
     */
    MeshListType meshes_;
};

}    // namespace

#endif  //VRN_MESHLISTGEOMETRY_H
