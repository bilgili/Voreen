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

#include "voreen/core/datastructures/geometry/meshlistgeometry.h"
#include "voreen/core/io/serialization/xmlserializer.h"
#include "voreen/core/io/serialization/xmldeserializer.h"

namespace voreen {

MeshListGeometry::MeshListGeometry()
    : Geometry()
{}

Geometry* MeshListGeometry::create() const {
    return new MeshListGeometry();
}

size_t MeshListGeometry::getMeshCount() const {
    return meshes_.size();
}

size_t MeshListGeometry::getFaceCount() const {
    size_t result = 0;
    for (const_iterator it = begin(); it != end(); ++it)
        result += it->getFaceCount();
    return result;
}

size_t MeshListGeometry::getVertexCount() const {
    size_t result = 0;
    for (const_iterator it = begin(); it != end(); ++it)
        result += it->getVertexCount();
    return result;
}

bool MeshListGeometry::empty() const {
    return (getVertexCount() == 0);
}

void MeshListGeometry::addMesh(const MeshGeometry& mesh) {
    meshes_.push_back(mesh);
}

const MeshGeometry& MeshListGeometry::getMesh(size_t index) const {
    tgtAssert(index < meshes_.size(), "Invalid index");
    return meshes_.at(index);
}

MeshGeometry& MeshListGeometry::getMesh(size_t index) {
    tgtAssert(index < meshes_.size(), "Invalid index");
    return meshes_.at(index);
}

void MeshListGeometry::clear() {
    meshes_.clear();
}

MeshListGeometry::iterator MeshListGeometry::begin() {
    return meshes_.begin();
}

MeshListGeometry::const_iterator MeshListGeometry::begin() const {
    return meshes_.begin();
}

MeshListGeometry::iterator MeshListGeometry::end() {
    return meshes_.end();
}

MeshListGeometry::const_iterator MeshListGeometry::end() const {
    return meshes_.end();
}

MeshGeometry& MeshListGeometry::operator[](size_t index) {
    tgtAssert(index < meshes_.size(), "Invalid index");
    return meshes_[index];
}

const MeshGeometry& MeshListGeometry::operator[](size_t index) const {
    tgtAssert(index < meshes_.size(), "Invalid index");
    return meshes_[index];
}

tgt::Bounds MeshListGeometry::getBoundingBox() const {
    tgt::Bounds bounds;
    for (size_t i=0; i<meshes_.size(); i++)
        bounds.addVolume(meshes_[i].getBoundingBox());
    return bounds;
}

/*
void MeshListGeometry::getBoundingBox(tgt::vec3& llf, tgt::vec3& urb) const {
    llf = tgt::vec3(FLT_MAX);
    urb = tgt::vec3(-FLT_MAX);
    for (size_t i = 0; i < getMeshCount(); ++i) {
        const MeshGeometry& mesh = getMesh(i);
        for (size_t j = 0; j < mesh.getFaceCount(); ++j) {
            const FaceGeometry& face = mesh.getFace(j);
            for (size_t k = 0; k < face.getVertexCount(); ++k) {
                const VertexGeometry& vertex = face.getVertex(k);
                const tgt::vec3& coords = vertex.getCoords();

                llf.x = std::min(llf.x, coords.x);
                urb.x = std::max(urb.x, coords.x);
                llf.y = std::min(llf.y, coords.y);
                urb.y = std::max(urb.y, coords.y);
                llf.z = std::min(llf.z, coords.z);
                urb.z = std::max(urb.z, coords.z);
            }
        }
    }
}
*/

void MeshListGeometry::render() const {
    for (const_iterator it = begin(); it != end(); ++it)
        it->render();
}

void MeshListGeometry::transform(const tgt::mat4& transformation) {
    for (iterator it = begin(); it != end(); ++it)
        it->transform(transformation);
}

void MeshListGeometry::clip(const tgt::vec4& clipPlane, MeshListGeometry& closingFaces, double epsilon) {
    tgtAssert(epsilon >= 0.0, "negative epsilon");
    for (iterator it = begin(); it != end(); ++it) {
        MeshGeometry closingFace;
        it->clip(clipPlane, closingFace, epsilon);
        if (!closingFace.empty())
            closingFaces.addMesh(closingFace);
    }
}

void MeshListGeometry::clip(const tgt::vec4& clipPlane, double epsilon) {
    MeshListGeometry dummy;
    clip(clipPlane, dummy, epsilon);
}

bool MeshListGeometry::equals(const MeshListGeometry& meshList, double epsilon /*= 1e-6*/) const {
    if (getMeshCount() != meshList.getMeshCount())
        return false;
    for (size_t i=0; i<getMeshCount(); i++) {
        if (!meshes_[i].equals(meshList.meshes_[i], epsilon))
            return false;
    }
    return true;
}

bool MeshListGeometry::equals(const Geometry* geometry, double epsilon /*= 1e-6*/) const {
    const MeshListGeometry* meshListGeometry = dynamic_cast<const MeshListGeometry*>(geometry);
    if (!meshListGeometry)
        return false;
    else
        return equals(*meshListGeometry, epsilon);
}

void MeshListGeometry::serialize(XmlSerializer& s) const {
    s.serialize("meshes", meshes_);
}

void MeshListGeometry::deserialize(XmlDeserializer& s) {
    s.deserialize("meshes", meshes_);
    setHasChanged(true);
}

} // namespace
