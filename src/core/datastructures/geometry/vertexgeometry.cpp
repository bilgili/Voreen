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

#include "tgt/glmath.h"

#include "voreen/core/datastructures/geometry/vertexgeometry.h"

namespace voreen {

VertexGeometry::VertexGeometry(const tgt::vec3& coords, const tgt::vec3& texcoords, const tgt::vec4& color)
    : coords_(coords)
    , texcoords_(texcoords)
    , color_(color)
{
}

tgt::vec3 VertexGeometry::getCoords() const {
    return coords_;
}

void VertexGeometry::setCoords(const tgt::vec3& coords) {
    coords_ = coords;

    setHasChanged(true);
}

tgt::vec3 VertexGeometry::getTexCoords() const {
    return texcoords_;
}

void VertexGeometry::setTexCoords(const tgt::vec3& texcoords) {
    texcoords_ = texcoords;

    setHasChanged(true);
}

tgt::vec4 VertexGeometry::getColor() const {
    return color_;
}

void VertexGeometry::setColor(const tgt::vec4& color) {
    color_ = color;

    setHasChanged(true);
}

void VertexGeometry::setColor(const tgt::vec3& color) {
    color_ = tgt::vec4(color, 1);

    setHasChanged(true);
}

double VertexGeometry::getLength() const {
    return tgt::length(coords_);
}

double VertexGeometry::getDistanceToPlane(const tgt::vec4& plane, double epsilon) const {
    double distance = tgt::dot(plane.xyz(), coords_) - plane.w;
    if (std::abs(distance) <= epsilon)
        return 0;
    else
        return distance;
}

double VertexGeometry::getDistance(const VertexGeometry& vertex) const {
    return tgt::length(vertex.coords_ - coords_);
}

void VertexGeometry::render() {
    glBegin(GL_POINTS);

    glColor4fv(color_.elem);
    tgt::texCoord(texcoords_);
    tgt::vertex(coords_);

    glEnd();
}

void VertexGeometry::combine(const VertexGeometry& vertex) {
    interpolate(vertex, 0.5);
}

VertexGeometry VertexGeometry::combine(const VertexGeometry& vertex1, const VertexGeometry& vertex2) {
    return interpolate(vertex1, vertex2, 0.5);
}

void VertexGeometry::interpolate(const VertexGeometry& vertex, double t) {
    coords_ += (vertex.coords_ - coords_) * static_cast<tgt::vec3::ElemType>(t);
    texcoords_ += (vertex.texcoords_ - texcoords_) * static_cast<tgt::vec3::ElemType>(t);
    color_ += (vertex.color_ - color_) * static_cast<tgt::vec4::ElemType>(t);

    setHasChanged(true);
}

VertexGeometry VertexGeometry::interpolate(const VertexGeometry& vertex1, const VertexGeometry& vertex2, double t) {
    VertexGeometry result = vertex1;
    result.interpolate(vertex2, t);
    return result;
}

void VertexGeometry::transform(const tgt::mat4& transformation) {
    coords_ = transformation * coords_;

    setHasChanged(true);
}

bool VertexGeometry::equals(const VertexGeometry& vertex, double epsilon) const {
    return std::abs(getDistance(vertex)) <= epsilon;
}

bool VertexGeometry::operator==(const VertexGeometry& vertex) const {
    return this->equals(vertex);
}

bool VertexGeometry::operator!=(const VertexGeometry& vertex) const {
    return !(*this == vertex);
}

} // namespace
