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

#ifndef VRN_TRIANGLEMESHGEOMETRYINDEXED_H
#define VRN_TRIANGLEMESHGEOMETRYINDEXED_H

#include <vector>

#include "tgt/tgt_gl.h"
#include "tgt/glmath.h"

#include "trianglemeshgeometry.h"
#include "vertex.h"

#include "voreen/core/io/serialization/xmlserializer.h"
#include "voreen/core/io/serialization/xmldeserializer.h"

namespace voreen {

/*
 * Use this class to render indexed vertex meshes. It is faster to render large amounts of trianlges sharing
 * vertices with this class, but clipping is not implemented for now.
 */
template <class I, class V>
class TriangleMeshGeometryIndexed : public TriangleMeshGeometryBase {
public:
    typedef Triangle<I> TriangleType;
    typedef V VertexType;

    TriangleMeshGeometryIndexed();

    /// Clears the vector and deletes the OpenGL buffers if necessary.
    virtual ~TriangleMeshGeometryIndexed();

    virtual size_t getNumTriangles() const;
    virtual bool isEmpty() const;
    virtual void clear();

    /// Adds a triangle to the mesh by passing three indices. Flags the bounding box and OpenGL buffer as invalid.
    void addTriangle(const tgt::Vector3<I>& t);
    void addTriangle(const TriangleType& t);

    const TriangleType& getTriangle(size_t i) const;

    const TriangleType& operator[] (size_t i) const;

    /// Modifies a triangle of the mesh. Flags the bounding box and OpenGL buffer as invalid.
    void setTriangle(const TriangleType& t, size_t i);

    /// Returns the bounding box in model or transformed coordinates. The BB is cached internally.
    virtual tgt::Bounds getBoundingBox(bool transformed = true) const;

    /// Serializes the triangles and indices as binary blob.
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
    void triangulate(const std::vector<I>& poly);

    /// Triangulates a quad given by four vertices and add the two triangles to the mesh.
    void addQuad(const I& v1, const I& v2, const I& v3, const I& v4);

    /// Add the triangles of another mesh to this mesh. Vertices are transformed if necessary.
    void addMesh(const TriangleMeshGeometryIndexed<I, V>* mesh);

    virtual Geometry* clone() const;

    /// Used to pass the vertex set for this geometry. For now, this method _must_ be called before any triangles are added.
    void setVertices(const std::vector<VertexType>& vertices) {
        vertices_ = vertices;
    }

protected:

    virtual void updateBoundingBox() const;

    /// Updates the OpenGL buffer.
    virtual void updateBuffer() const;

    mutable tgt::Bounds boundingBox_;
    mutable bool boundingBoxValid_;

    mutable GLuint bufferObject_;
    mutable bool bufferObjectValid_;

    mutable GLuint indexObject_;
    mutable bool indexObjectValid_;

    std::vector<TriangleType> triangles_;
    std::vector<VertexType> vertices_;
    GLuint indexDataType_;

    static const std::string loggerCat_;
};


template<class I, class V>
const std::string TriangleMeshGeometryIndexed<I,V>::loggerCat_ = "voreen.TriangleMeshGeometryIndexed";

template <class I, class V>
TriangleMeshGeometryIndexed<I, V>::TriangleMeshGeometryIndexed()
    : boundingBox_()
    , boundingBoxValid_(false)
    , bufferObject_(0)
    , bufferObjectValid_(false)
    , indexObject_(0)
    , indexObjectValid_(false)
{
    switch(sizeof(I)) {
        case 1:
            indexDataType_ = GL_UNSIGNED_BYTE;
            break;
        case 2:
            indexDataType_ = GL_UNSIGNED_SHORT;
            break;
        case 4:
            indexDataType_ = GL_UNSIGNED_INT;
            break;
        default:
            tgtAssert(false, "Unsupported index datatype in TriangleMeshGeometryIndexed!");
    }
}

template <class I, class V>
TriangleMeshGeometryIndexed<I, V>::~TriangleMeshGeometryIndexed() {
    clear();

    if(bufferObject_ != 0) {
        glDeleteBuffers(1, &bufferObject_);
        bufferObject_ = 0;
        bufferObjectValid_ = false;
    }

    if(indexObject_ != 0) {
        glDeleteBuffers(1, &indexObject_);
        indexObject_ = 0;
        indexObjectValid_ = false;
    }
}

template <class I, class V>
size_t TriangleMeshGeometryIndexed<I, V>::getNumTriangles() const {
    return triangles_.size();
}

template <class I, class V>
bool TriangleMeshGeometryIndexed<I, V>::isEmpty() const {
    return (triangles_.size() == 0);
}

template <class I, class V>
void TriangleMeshGeometryIndexed<I, V>::clear() {
    triangles_.clear();
    vertices_.clear();

    invalidate();
}

template <class I, class V>
void TriangleMeshGeometryIndexed<I, V>::addTriangle(const tgt::Vector3<I>& t) {
    if (vertices_.empty()) {
        LERROR("No vertices have been set; cannot add triangle.");
        return;
    }
    tgtAssert(t.x < vertices_.size() && t.y < vertices_.size() && t.z < vertices_.size(), "Invalid triangle indices");
    triangles_.push_back(TriangleType(t.x, t.y, t.z));
    invalidate();
}

template <class I, class V>
void voreen::TriangleMeshGeometryIndexed<I, V>::addTriangle(const TriangleType& t) {
    if (vertices_.empty()) {
        LERROR("No vertices have been set; cannot add triangle.");
        return;
    }
    tgtAssert(t.v_[0] < vertices_.size() && t.v_[1] < vertices_.size() && t.v_[2] < vertices_.size(), "Invalid triangle indices");
    triangles_.push_back(t);
    invalidate();
}

template <class I, class V>
const typename TriangleMeshGeometryIndexed<I, V>::TriangleType& TriangleMeshGeometryIndexed<I, V>::getTriangle(size_t i) const {
    tgtAssert(i < triangles_.size(), "Invalid triangle index");
    return triangles_[i];
}

template <class I, class V>
const typename TriangleMeshGeometryIndexed<I, V>::TriangleType& TriangleMeshGeometryIndexed<I, V>::operator[] (size_t i) const {
    tgtAssert(i < triangles_.size(), "Invalid triangle index");
    return triangles_[i];
}

template <class I, class V>
void TriangleMeshGeometryIndexed<I, V>::setTriangle(const TriangleType& t, size_t i) {
    tgtAssert(i < triangles_.size(), "Invalid triangle index");
    triangles_[i] = t;

    invalidate();
}

template <class I, class V>
tgt::Bounds TriangleMeshGeometryIndexed<I, V>::getBoundingBox(bool transformed) const {
    if(!boundingBoxValid_)
        updateBoundingBox();

    if(transformed)
        return boundingBox_.transform(getTransformationMatrix());
    else
        return boundingBox_;
}

template <class I, class V>
void TriangleMeshGeometryIndexed<I, V>::serialize(XmlSerializer& s) const {
    TriangleMeshGeometryBase::serialize(s);
    s.serializeBinaryBlob("triangles", triangles_);
    s.serializeBinaryBlob("vertices", vertices_);
}

template <class I, class V>
void TriangleMeshGeometryIndexed<I, V>::deserialize(XmlDeserializer& s) {
    TriangleMeshGeometryBase::deserialize(s);
    s.deserializeBinaryBlob("triangles", triangles_);
    s.deserializeBinaryBlob("vertices", vertices_);
}

template <class I, class V>
void TriangleMeshGeometryIndexed<I, V>::invalidate() {
    boundingBoxValid_ = false;
    bufferObjectValid_ = false;
    indexObjectValid_ = false;
}

template <class I, class V>
void TriangleMeshGeometryIndexed<I, V>::render() const {
    updateBuffer();

    if(isEmpty())
        return;

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.multMatrix(getTransformationMatrix());

    glBindBuffer(GL_ARRAY_BUFFER, bufferObject_);
    VertexType::setupVertexAttributePointers();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexObject_);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(triangles_.size() * 3), indexDataType_, NULL);

    LGL_ERROR;

    VertexType::disableVertexAttributePointers();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    MatStack.popMatrix();
}

template <class I, class V>
bool TriangleMeshGeometryIndexed<I, V>::equals(const Geometry* geometry, double epsilon) const {
    const TriangleMeshGeometryIndexed<I, V>* triMesh = dynamic_cast<const TriangleMeshGeometryIndexed<I, V>*>(geometry);

    if(triMesh) {
        if(getNumTriangles() != triMesh->getNumTriangles())
            return false;

        for(size_t i=0; i<triangles_.size(); i++) {
            if (!triangles_[i].v_[0] == triMesh->triangles_[i].v_[0] ||
                !triangles_[i].v_[1] == triMesh->triangles_[i].v_[1] ||
                !triangles_[i].v_[2] == triMesh->triangles_[i].v_[2]  )
            {
                return false;
            }
        }

        for(size_t i=0; i<vertices_.size(); i++) {
            if (!vertices_[i].equals(triMesh->vertices_[i]))
                return false;
        }
        return true;
    }
    else
        return false;
}

template <class I, class V>
void TriangleMeshGeometryIndexed<I, V>::triangulate(const std::vector<I>& poly) {
    if(poly.size() < 3)
        return;

    for(size_t i=2; i<poly.size(); i++)
        addTriangle(TriangleType(poly[0], poly[i-1], poly[i]));
}

template <class I, class V>
void TriangleMeshGeometryIndexed<I, V>::addQuad(const I& v1, const I& v2, const I& v3, const I& v4) {
    addTriangle(TriangleType(v1, v2, v3));
    addTriangle(TriangleType(v1, v3, v4));
}

template <class I, class V>
void TriangleMeshGeometryIndexed<I, V>::addMesh(const TriangleMeshGeometryIndexed<I, V>* mesh) {
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
            vertices_.at(t.v_[0]).pos_ = m * vertices_.at(t.v_[0]).pos_;
            vertices_.at(t.v_[1]).pos_ = m * vertices_.at(t.v_[1]).pos_;
            vertices_.at(t.v_[2]).pos_ = m * vertices_.at(t.v_[2]).pos_;
            addTriangle(t);
        }
    }
}

template <class I, class V>
void TriangleMeshGeometryIndexed<I, V>::updateBoundingBox() const {
    tgt::Bounds bb;
    for(size_t i=0; i<triangles_.size(); i++) {
        bb.addPoint(vertices_.at(triangles_[i].v_[0]).pos_);
        bb.addPoint(vertices_.at(triangles_[i].v_[1]).pos_);
        bb.addPoint(vertices_.at(triangles_[i].v_[2]).pos_);
    }
    boundingBox_ = bb;
    boundingBoxValid_ = true;
}

template <class I, class V>
void TriangleMeshGeometryIndexed<I, V>::updateBuffer() const {
    if(bufferObjectValid_ && indexObjectValid_)
        return;
    if(!bufferObjectValid_ && bufferObject_ != 0) {
        glDeleteBuffers(1, &bufferObject_);
        bufferObject_ = 0;
        bufferObjectValid_ = false;
    }

    if(!indexObjectValid_ && indexObject_ != 0) {
        glDeleteBuffers(1, &bufferObject_);
        bufferObject_ = 0;
        bufferObjectValid_ = false;
    }

    if(isEmpty())
        return;

    glGenBuffers(1, &bufferObject_);
    glGenBuffers(1, &indexObject_);

    glBindBuffer(GL_ARRAY_BUFFER, bufferObject_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexType) * vertices_.size(), &vertices_[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexObject_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TriangleType) * triangles_.size(), &triangles_[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    bufferObjectValid_ = true;
    indexObjectValid_ = true;
}

template<class I, class V>
Geometry* TriangleMeshGeometryIndexed<I, V>::clone() const {
    TriangleMeshGeometryIndexed<I, V>* newGeom = static_cast<TriangleMeshGeometryIndexed<I, V>*>(create());
    newGeom->vertices_ = vertices_;
    newGeom->triangles_ = triangles_;
    newGeom->setTransformationMatrix(getTransformationMatrix());
    return newGeom;
}

//-------------------------------------------------------------------------------------------------

/// A triangle mesh with vertex type VertexVec3 with indices of type uint16_t <=> up to 2^16 triangles.
class VRN_CORE_API TriangleMeshGeometryUInt16IndexedVec3 : public TriangleMeshGeometryIndexed<uint16_t, VertexVec3> {
public:
    virtual Geometry* create() const { return new TriangleMeshGeometryUInt16IndexedVec3(); }
    virtual std::string getClassName() const { return "TriangleMeshGeometryUInt16IndexedVec3"; }
    virtual TriangleMeshGeometryBase::VertexLayout getVertexLayout() const { return VEC3; }
protected:
};

/// A triangle mesh with vertex type VertexVec3 with indices of type uint32_t <=> up to 2^32 triangles.
class VRN_CORE_API TriangleMeshGeometryUInt32IndexedVec3 : public TriangleMeshGeometryIndexed<uint32_t, VertexVec3> {
public:
    virtual Geometry* create() const { return new TriangleMeshGeometryUInt32IndexedVec3(); }
    virtual std::string getClassName() const { return "TriangleMeshGeometryUInt32IndexedVec3"; }
    virtual TriangleMeshGeometryBase::VertexLayout getVertexLayout() const { return VEC3; }
protected:
};

} // namespace

#endif  //VRN_TRIANGLEMESHGEOMETRY_H
