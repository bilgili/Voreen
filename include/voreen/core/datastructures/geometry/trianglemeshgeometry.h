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

#ifndef VRN_TRIANGLEMESHGEOMETRY_H
#define VRN_TRIANGLEMESHGEOMETRY_H

#include <vector>

#include "tgt/tgt_gl.h"
#include "tgt/glmath.h"

#include "geometry.h"
#include "vertex.h"

#include "voreen/core/io/serialization/xmlserializer.h"
#include "voreen/core/io/serialization/xmldeserializer.h"

namespace voreen {

/// Base class for all TriangleMeshGeometries.
class VRN_CORE_API TriangleMeshGeometryBase : public Geometry {
public:
    TriangleMeshGeometryBase();

    virtual size_t getNumTriangles() const = 0;
    virtual bool isEmpty() const = 0;
    virtual void clear() = 0;
private:
};

/// Generic triangle, parameterized by the vertex-type. (@see VertexBase)
template <class V>
struct Triangle {
    V v_[3];

    typedef V VertexType;

    Triangle() {}
    Triangle(V v1, V v2, V v3)
    {
      v_[0] = v1;
      v_[1] = v2;
      v_[2] = v3;
    }
};

/*
 * Generic triangle-mesh, storing a vector of triangles.
 * Template argument is the vertex (not triangle!) type.
 */
template <class V>
class TriangleMeshGeometry : public TriangleMeshGeometryBase {
public:
    typedef Triangle<V> TriangleType;
    typedef V VertexType;

    TriangleMeshGeometry();

    /// Clears the vector and deletes the OpenGL buffer if necessary.
    virtual ~TriangleMeshGeometry();

    virtual size_t getNumTriangles() const;

    virtual bool isEmpty() const;

    virtual void clear();

    /// Adds a triangle to the mesh. Flags the bounding box and OpenGL buffer as invalid.
    void addTriangle(const TriangleType& t);

    const TriangleType& getTriangle(size_t i) const;

    const TriangleType& operator[] (size_t i) const;

    /// Modifies a triangle of the mesh. Flags the bounding box and OpenGL buffer as invalid.
    void setTriangle(const TriangleType& t, size_t i);

    /// Returns the bounding box in model or transformed coordinates. The BB is cached internally.
    virtual tgt::Bounds getBoundingBox(bool transformed = true) const;

    /// Serializes the triangles as binary blob.
    virtual void serialize(XmlSerializer& s) const;

    virtual void deserialize(XmlDeserializer& s);

    /// Flags the bounding box and OpenGL buffer as invalid.
    void invalidate();

    /*
     * Renders the mesh using OpenGL by binding the buffer, setting appropriate vertex attribute pointers and calling glDrawArrays.
     *
     * A shader with appropriate vertexattribute bindings has to be activated before calling render().
     */
    virtual void render() const;

    /**
     * Returns true, if the passed Geometry is a TriangleMeshGeometry of the same type and all its vertices are equal to this one's.
     */
    virtual bool equals(const Geometry* geometry, double epsilon = 1e-5) const;

    /// Triangulates a convex polygon
    void triangulate(const std::vector<VertexType>& poly);

    /// Triangulates a quad given by four vertices and add the two triangles to the mesh.
    void addQuad(const VertexType& v1, const VertexType& v2, const VertexType& v3, const VertexType& v4);

    /// Add the triangles of another mesh to this mesh. Vertices are transformed if necessary.
    void addMesh(const TriangleMeshGeometry<V>* mesh);

    /// Clips all triangles against the plane and closes the mesh. Works only for convex meshes.
    virtual void clip(const tgt::plane& clipPlane, double epsilon = 1e-5);

protected:
    virtual void updateBoundingBox() const;

    /// Updates the OpenGL buffer.
    virtual void updateBuffer() const;

    void clipTriangle(const TriangleType& in, std::vector<TriangleType>& out, std::vector< std::pair<VertexType, VertexType> >& edgeList, const tgt::plane& clipPlane, double epsilon = 1e-5);

    mutable tgt::Bounds boundingBox_;
    mutable bool boundingBoxValid_;

    mutable GLuint bufferObject_;
    mutable bool bufferObjectValid_;

    std::vector<TriangleType> triangles_;
};

template <class V>
TriangleMeshGeometry<V>::TriangleMeshGeometry()
    : boundingBox_()
    , boundingBoxValid_(false)
    , bufferObject_(0)
    , bufferObjectValid_(false)
{}

template <class V>
TriangleMeshGeometry<V>::~TriangleMeshGeometry() {
    clear();

    if(bufferObject_ != 0) {
        glDeleteBuffers(1, &bufferObject_);
        bufferObject_ = 0;
        bufferObjectValid_ = false;
    }
}

template <class V>
size_t TriangleMeshGeometry<V>::getNumTriangles() const {
    return triangles_.size();
}

template <class V>
bool TriangleMeshGeometry<V>::isEmpty() const {
    return (triangles_.size() == 0);
}

template <class V>
void TriangleMeshGeometry<V>::clear() {
    triangles_.clear();

    invalidate();
}

template <class V>
void TriangleMeshGeometry<V>::addTriangle(const TriangleType& t) {
    triangles_.push_back(t);
    invalidate();
}

template <class V>
const typename TriangleMeshGeometry<V>::TriangleType& TriangleMeshGeometry<V>::getTriangle(size_t i) const {
    tgtAssert(i < triangles_.size(), "Invalid triangle index");
    return triangles_[i];
}

template <class V>
const typename TriangleMeshGeometry<V>::TriangleType& TriangleMeshGeometry<V>::operator[] (size_t i) const {
    tgtAssert(i < triangles_.size(), "Invalid triangle index");
    return triangles_[i];
}

template <class V>
void TriangleMeshGeometry<V>::setTriangle(const TriangleType& t, size_t i) {
    tgtAssert(i < triangles_.size(), "Invalid triangle index");
    triangles_[i] = t;

    invalidate();
}

template <class V>
tgt::Bounds TriangleMeshGeometry<V>::getBoundingBox(bool transformed) const {
    if(!boundingBoxValid_)
        updateBoundingBox();

    if(transformed)
        return boundingBox_.transform(getTransformationMatrix());
    else
        return boundingBox_;
}

template <class V>
void TriangleMeshGeometry<V>::serialize(XmlSerializer& s) const {
    TriangleMeshGeometryBase::serialize(s);
    s.serializeBinaryBlob("triangles", triangles_);
}

template <class V>
void TriangleMeshGeometry<V>::deserialize(XmlDeserializer& s) {
    TriangleMeshGeometryBase::deserialize(s);
    s.deserializeBinaryBlob("triangles", triangles_);
}

template <class V>
void TriangleMeshGeometry<V>::invalidate() {
    boundingBoxValid_ = false;
    bufferObjectValid_ = false;
}

template <class V>
void TriangleMeshGeometry<V>::render() const {
    updateBuffer();

    if(isEmpty())
        return;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    tgt::multMatrix(getTransformationMatrix());

    glBindBuffer(GL_ARRAY_BUFFER, bufferObject_);

    VertexType::setupVertexAttributePointers();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(triangles_.size() * 3));
    VertexType::disableVertexAttributePointers();

    glPopMatrix();
}

template <class V>
bool TriangleMeshGeometry<V>::equals(const Geometry* geometry, double epsilon) const {
    const TriangleMeshGeometry<V>* triMesh = dynamic_cast<const TriangleMeshGeometry<V>*>(geometry);

    if(triMesh) {
        if(getNumTriangles() != triMesh->getNumTriangles())
            return false;

        for(size_t i=0; i<triangles_.size(); i++) {
            if (!triangles_[i].v_[0].equals(triMesh->triangles_[i].v_[0], epsilon) ||
                !triangles_[i].v_[1].equals(triMesh->triangles_[i].v_[1], epsilon) ||
                !triangles_[i].v_[2].equals(triMesh->triangles_[i].v_[2], epsilon)  )
            {
                return false;
            }
        }
        return true;
    }
    else
        return false;
}

template <class V>
void TriangleMeshGeometry<V>::triangulate(const std::vector<VertexType>& poly) {
    if(poly.size() < 3)
        return;

    for(size_t i=2; i<poly.size(); i++)
        addTriangle(TriangleType(poly[0], poly[i-1], poly[i]));
}

template <class V>
void TriangleMeshGeometry<V>::addQuad(const VertexType& v1, const VertexType& v2, const VertexType& v3, const VertexType& v4) {
    addTriangle(TriangleType(v1, v2, v3));
    addTriangle(TriangleType(v1, v3, v4));
}

template <class V>
void TriangleMeshGeometry<V>::addMesh(const TriangleMeshGeometry<V>* mesh) {
    if(getTransformationMatrix() == mesh->getTransformationMatrix()) {
        //Just copy the triangles:
        for(size_t i=0; i<mesh->getNumTriangles(); i++)
            addTriangle(mesh->getTriangle(i));
    }
    else {
        tgt::mat4 m;
        getTransformationMatrix().invert(m);
        m = m * mesh->getTransformationMatrix();

        for(size_t i=0; i<mesh->getNumTriangles(); i++) {
            TriangleType t = mesh->getTriangle(i);
            t.v_[0].pos_ = m * t.v_[0].pos_;
            t.v_[1].pos_ = m * t.v_[1].pos_;
            t.v_[2].pos_ = m * t.v_[2].pos_;
            addTriangle(t);
        }
    }
}

template <class V>
void TriangleMeshGeometry<V>::clip(const tgt::plane& clipPlane, double epsilon) {
    tgtAssert(epsilon >= 0.0, "negative epsilon");

    tgt::plane pl = clipPlane.transform(getInvertedTransformationMatrix());

    tgt::Bounds b = getBoundingBox(false);
    if(!b.intersects(pl)) {
        // The plane isn't intersecting the boundingbox, so its either left as-is or removed:
        if(pl.distance(b.center()) < 0.0) {
            return;
        }
        else {
            clear();
            return;
        }
    }

    std::vector<TriangleType> clippedTriangles;
    std::vector< std::pair<VertexType, VertexType> > edgeList;

    // Clip all faces...
    for (size_t i=0; i<getNumTriangles(); i++) {
        clipTriangle(getTriangle(i), clippedTriangles, edgeList, pl, epsilon);
    }

    std::swap(triangles_, clippedTriangles);
    invalidate();

    // Is closing necessary?
    if (edgeList.size() > 1) {
        // Sort edges to produce contiguous vertex order...
        bool reverseLastEdge = false;
        for (size_t i = 0; i < edgeList.size() - 1; ++i) {
            for (size_t j = i + 1; j < edgeList.size(); ++j) {
                VertexType connectionVertex;
                if (reverseLastEdge)
                    connectionVertex = edgeList.at(i).first;
                else
                    connectionVertex = edgeList.at(i).second;

                if (distance(edgeList.at(j).first.pos_, connectionVertex.pos_) < epsilon) {
                    std::swap(edgeList.at(i + 1), edgeList.at(j));
                    reverseLastEdge = false;
                    break;
                }
                else if (distance(edgeList.at(j).second.pos_, connectionVertex.pos_) < epsilon) {
                    std::swap(edgeList.at(i + 1), edgeList.at(j));
                    reverseLastEdge = true;
                    break;
                }
            }
        }
        // Set normal of all vertices to plane normal:
        for (size_t i = 0; i < edgeList.size(); ++i) {
            edgeList.at(i).first.setNormal(pl.n);
            edgeList.at(i).second.setNormal(pl.n);
        }

        // Convert sorted edge list to sorted vertex list...
        std::vector<VertexType> closingFaceVertices;
        for (size_t i = 0; i < edgeList.size(); ++i) {
            bool reverseEdge = (i != 0) && !(distance(closingFaceVertices.at(closingFaceVertices.size() - 1).pos_, edgeList.at(i).first.pos_) < epsilon);

            VertexType first = (reverseEdge ? edgeList.at(i).second : edgeList.at(i).first);
            VertexType second = (reverseEdge ? edgeList.at(i).first : edgeList.at(i).second);

            if (i == 0)
                closingFaceVertices.push_back(first);
            else
                closingFaceVertices.at(closingFaceVertices.size() - 1) = VertexType::interpolate(closingFaceVertices.at(closingFaceVertices.size() - 1), first, 0.5f);

            if (i < (edgeList.size() - 1))
                closingFaceVertices.push_back(second);
            else
                closingFaceVertices[0] = VertexType::interpolate(closingFaceVertices[0], second, 0.5f);
        }

        // Convert vertex order to counter clockwise if necessary...
        tgt::vec3 closingFaceNormal(0, 0, 0);
        for (size_t i = 0; i < closingFaceVertices.size(); ++i)
            closingFaceNormal += tgt::cross(closingFaceVertices[i].pos_, closingFaceVertices[(i + 1) % closingFaceVertices.size()].pos_);

        closingFaceNormal = tgt::normalize(closingFaceNormal);

        if (tgt::dot(pl.n, closingFaceNormal) < 0)
            std::reverse(closingFaceVertices.begin(), closingFaceVertices.end());

        if(closingFaceVertices.size() > 2) {
            triangulate(closingFaceVertices);
        }
    }
}

template <class V>
void TriangleMeshGeometry<V>::updateBoundingBox() const {
    tgt::Bounds bb;
    for(size_t i=0; i<triangles_.size(); i++) {
        bb.addPoint(triangles_[i].v_[0].pos_);
        bb.addPoint(triangles_[i].v_[1].pos_);
        bb.addPoint(triangles_[i].v_[2].pos_);
    }
    boundingBox_ = bb;
    boundingBoxValid_ = true;
}

template <class V>
void TriangleMeshGeometry<V>::updateBuffer() const {
    if(bufferObjectValid_)
        return;
    else if(bufferObject_ != 0) {
        glDeleteBuffers(1, &bufferObject_);
        bufferObject_ = 0;
        bufferObjectValid_ = false;
    }

    if(isEmpty())
        return;

    glGenBuffers(1, &bufferObject_);

    glBindBuffer(GL_ARRAY_BUFFER, bufferObject_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleType) * triangles_.size(), &triangles_[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    bufferObjectValid_ = true;
}

template <class V>
void TriangleMeshGeometry<V>::clipTriangle(const TriangleType& in, std::vector<TriangleType>& out, std::vector< std::pair<VertexType, VertexType> >& edgeList, const tgt::plane& clipPlane, double epsilon) {
    tgtAssert(epsilon >= 0.0, "negative epsilon");

    double lastDistance = in.v_[0].getDistanceToPlane(clipPlane, epsilon);

    std::vector<VertexType> outVertices;
    std::vector<VertexType> onPlaneVertices;

    // Process face edges...
    for (size_t i = 0; i < 3; ++i) {
        double distance = in.v_[(i + 1) % 3].getDistanceToPlane(clipPlane, epsilon);

        // Keep both vertices?
        if (lastDistance <= 0 && distance <= 0) {
            // If processing the first edge, insert first vertex...
            if (i == 0)
                outVertices.push_back(in.v_[i]);

            // If NOT processing the last edge, insert second vertex...
            if (i < 2)
                outVertices.push_back(in.v_[i + 1]);
        }
        // Discard first vertex, but keep second vertex?
        else if (lastDistance > 0 && distance <= 0) {
            // If NOT clipplane intersection vertex and second vertex are equal, insert clipplane intersection vertex...
            VertexType intersectionVertex = VertexType::interpolate(in.v_[i], in.v_[(i + 1) % 3], static_cast<float>(lastDistance / (lastDistance - distance)));
            if (!in.v_[(i + 1) % 3].equals(intersectionVertex, epsilon)) {
                outVertices.push_back(intersectionVertex);
                onPlaneVertices.push_back(intersectionVertex);
            }
            else
                onPlaneVertices.push_back(in.v_[(i + 1) % 3]);


            // If NOT processing the last edge, insert second vertex...
            if (i < 2)
                outVertices.push_back(in.v_[i + 1]);
        }
        // Keep first vertex, but discard second vertex?
        else if (lastDistance <= 0 && distance > 0) {
            // If processing the first edge, insert first vertex...
            if (i == 0)
                outVertices.push_back(in.v_[i]);

            //// If NOT clipplane intersection vertex and first vertex are equal, insert clipplane intersection vertex...
            VertexType intersectionVertex = VertexType::interpolate(in.v_[i], in.v_[(i + 1) % 3], static_cast<float>(lastDistance / (lastDistance - distance)));
            if (!in.v_[i].equals(intersectionVertex, epsilon)) {
                outVertices.push_back(intersectionVertex);
                onPlaneVertices.push_back(intersectionVertex);
            }
            else
                onPlaneVertices.push_back(in.v_[i]);
        }

        lastDistance = distance;
    }

    // Create triangles from output vertices:
    if(outVertices.size() == 3) {
        out.push_back(TriangleType(outVertices[0], outVertices[1], outVertices[2]));
    }
    else if(outVertices.size() == 4) {
        out.push_back(TriangleType(outVertices[0], outVertices[1], outVertices[2]));
        out.push_back(TriangleType(outVertices[0], outVertices[2], outVertices[3]));
    }

    if(onPlaneVertices.size() == 2) {
        edgeList.push_back(std::pair<VertexType, VertexType>(onPlaneVertices[0], onPlaneVertices[1]));
    }
    else if(onPlaneVertices.size() == 0) {
    }
    else {
        tgtAssert(false, "Should not come here.");
    }
}

//-------------------------------------------------------------------------------------------------

/// A triangle mesh with vertex type VertexVec3.
class VRN_CORE_API TriangleMeshGeometryVec3 : public TriangleMeshGeometry<VertexVec3> {
public:
    virtual Geometry* create() const { return new TriangleMeshGeometryVec3(); }
    virtual std::string getClassName() const { return "TriangleMeshGeometryVec3"; }

    /// Creates a mesh containing a cube specified by two vertices.
    static TriangleMeshGeometryVec3* createCube(VertexType llfVertex, VertexType urbVertex);

    /// Adds a cube to this mesh.
    void addCube(VertexType llfVertex, VertexType urbVertex);
protected:
};

//-------------------------------------------------------------------------------------------------

/// A triangle mesh with vertex type VertexVec4Vec3
class VRN_CORE_API TriangleMeshGeometryVec4Vec3 : public TriangleMeshGeometry<VertexVec4Vec3> {
public:
    virtual Geometry* create() const { return new TriangleMeshGeometryVec4Vec3(); }
    virtual std::string getClassName() const { return "TriangleMeshGeometryVec4Vec3"; }

    static TriangleMeshGeometryVec4Vec3* createCube(tgt::vec3 coordLlf, tgt::vec3 coordUrb, tgt::vec3 colorLlf, tgt::vec3 colorUrb, float alpha, tgt::vec3 texLlf, tgt::vec3 texUrb);

    /// Adds a cube to this mesh.
    void addCube(VertexVec3 llfVertex, VertexVec3 urbVertex);

    void addQuad(const VertexVec3& v1, const VertexVec3& v2, const VertexVec3& v3, const VertexVec3& v4);

    /// Creates a cube with color and normals:
    static TriangleMeshGeometryVec4Vec3* createCube(tgt::vec3 coordLlf, tgt::vec3 coordUrb, tgt::vec3 colorLlf, tgt::vec3 colorUrb, float alpha);
};

//-------------------------------------------------------------------------------------------------

/// A triangle mesh with vertex type VertexBase (i.e., only position)
class VRN_CORE_API TriangleMeshGeometrySimple : public TriangleMeshGeometry<VertexBase> {
public:
    virtual Geometry* create() const { return new TriangleMeshGeometrySimple(); }
    virtual std::string getClassName() const { return "TriangleMeshGeometrySimple"; }

    static TriangleMeshGeometrySimple* createCube(tgt::vec3 coordLlf, tgt::vec3 coordUrb);
};

} // namespace

#endif  //VRN_TRIANGLEMESHGEOMETRY_H
