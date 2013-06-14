/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#ifndef VRN_VERTEX_H
#define VRN_VERTEX_H

#include "tgt/vector.h"
#include "tgt/plane.h"
#include "tgt/tgt_gl.h"

namespace voreen {

/*
 * Base class for vertices stored in meshes.
 * Most of the methods have to be reimplemented for subclasses.
 * The methods are intentionally not made virtual for performance reasons.
 */
struct VertexBase {
    tgt::vec3 pos_;

    VertexBase() {}
    VertexBase(tgt::vec3 pos) : pos_(pos) {}

    /// Compares this vertex to another vertex. Reimplement in subclass.
    bool equals(const VertexBase& other, double epsilon = 1e-5) const;

    /// Sets up vertex attributs pointers for rendering. Reimplement in subclass.
    static void setupVertexAttributePointers(size_t stride = 0);

    /// De-initializes vertex attributes pointers. Reimplement in subclass.
    static void disableVertexAttributePointers();

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
    double getDistanceToPlane(const tgt::plane& plane, double epsilon = 1e-5) const;

    /// Interpolates two vertices of this type. Reimplement in subclass.
    static VertexBase interpolate(const VertexBase& v1, const VertexBase& v2, float t);

    /// Sets the normal of this vertex, used in clipping code. Default implementation does nothing.
    void setNormal(tgt::vec3 n);
};

//-------------------------------------------------------------------------------------------------

/// A vertex with an additional vec3 attribute (e.g., texture coordinate).
struct VertexVec3 : public VertexBase {
    tgt::vec3 attr1_;

    VertexVec3() {}
    VertexVec3(tgt::vec3 pos, tgt::vec3 attr1);

    bool equals(const VertexVec3& other, double epsilon = 1e-5) const;

    static void setupVertexAttributePointers();
    static void disableVertexAttributePointers();

    static VertexVec3 interpolate(const VertexVec3& v1, const VertexVec3& v2, float t);
};

//-------------------------------------------------------------------------------------------------

/// A vertex with an additional vec4 and vec3 attribute (e.g., RGBA color and texture coordinate).
struct VertexVec4Vec3 : public VertexBase {
    tgt::vec4 attr1_;
    tgt::vec3 attr2_;

    VertexVec4Vec3() {}
    VertexVec4Vec3(tgt::vec3 pos, tgt::vec4 attr1, tgt::vec3 attr2);

    bool equals(const VertexVec4Vec3& other, double epsilon = 1e-5) const;

    static void setupVertexAttributePointers();

    static void disableVertexAttributePointers();

    static VertexVec4Vec3 interpolate(const VertexVec4Vec3& v1, const VertexVec4Vec3& v2, float t);

    void setNormal(tgt::vec3 n);
};

} // namespace

#endif  //VRN_VERTEX_H
