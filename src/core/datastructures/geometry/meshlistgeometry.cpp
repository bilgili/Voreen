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

#include "voreen/core/datastructures/geometry/meshlistgeometry.h"

namespace voreen {

MeshListGeometry::MeshListGeometry()
    : Geometry()
{
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

void MeshListGeometry::render() {
    for (iterator it = begin(); it != end(); ++it)
        it->render();
}

void MeshListGeometry::transform(const tgt::mat4& transformation) {
    for (iterator it = begin(); it != end(); ++it)
        it->transform(transformation);
}

MeshListGeometry MeshListGeometry::clip(const tgt::vec4& clipplane, double epsilon) {
    MeshListGeometry closingFaces;
    for (iterator it = begin(); it != end(); ++it) {
        MeshGeometry closingFace = it->clip(clipplane, epsilon);
        if (!closingFace.empty())
            closingFaces.addMesh(closingFace);
    }
    return closingFaces;
}

} // namespace
