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

#include "voreen/core/datastructures/geometry/vertex.h"

namespace voreen {

bool VertexBase::equals(const VertexBase& other, double epsilon) const {
    if (distance(pos_, other.pos_) > epsilon)
        return false;
    else
        return true;
}

void VertexBase::setupVertexAttributePointers(size_t stride) {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), 0);
}

void VertexBase::disableVertexAttributePointers() {
    glDisableVertexAttribArray(0);
}

double VertexBase::getDistanceToPlane(const tgt::plane& plane, double epsilon) const {
    double distance = plane.distance(pos_);
    if (std::abs(distance) <= epsilon)
        return 0;
    else
        return distance;
}

VertexBase VertexBase::interpolate(const VertexBase& v1, const VertexBase& v2, float t) {
    return VertexBase((v1.pos_ * (1.0f - t)) + (v2.pos_ * t));
}

void VertexBase::setNormal(tgt::vec3 /*n*/) {
}

//-------------------------------------------------------------------------------------------------

VertexVec3::VertexVec3(tgt::vec3 pos, tgt::vec3 attr1)
    : VertexBase(pos)
    , attr1_(attr1)
{}

bool VertexVec3::equals(const VertexVec3& other, double epsilon) const {
    if (distance(pos_, other.pos_) > epsilon)
        return false;

    if (distance(attr1_, other.attr1_) > epsilon)
        return false;

    return true;
}

void VertexVec3::setupVertexAttributePointers() {
    VertexBase::setupVertexAttributePointers(sizeof(VertexVec3));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexVec3), (void*)sizeof(tgt::vec3));
}

void VertexVec3::disableVertexAttributePointers() {
    VertexBase::disableVertexAttributePointers();

    glDisableVertexAttribArray(1);
}

VertexVec3 VertexVec3::interpolate(const VertexVec3& v1, const VertexVec3& v2, float t) {
    return VertexVec3((v1.pos_ * (1.0f - t)) + (v2.pos_ * t), (v1.attr1_* (1.0f - t)) + (v2.attr1_* t));
}

//-------------------------------------------------------------------------------------------------

VertexVec4Vec3::VertexVec4Vec3(tgt::vec3 pos, tgt::vec4 attr1, tgt::vec3 attr2)
    : VertexBase(pos)
    , attr1_(attr1)
    , attr2_(attr2)
{}

bool VertexVec4Vec3::equals(const VertexVec4Vec3& other, double epsilon) const {
    if (distance(pos_, other.pos_) > epsilon)
        return false;

    if (distance(attr1_, other.attr1_) > epsilon)
        return false;

    if (distance(attr2_, other.attr2_) > epsilon)
        return false;

    return true;
}

void VertexVec4Vec3::setupVertexAttributePointers() {
    VertexBase::setupVertexAttributePointers(sizeof(VertexVec4Vec3));

    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexVec4Vec3), (void*)sizeof(tgt::vec3));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexVec4Vec3), (void*)(sizeof(tgt::vec3) + sizeof(tgt::vec4)));
}

void VertexVec4Vec3::disableVertexAttributePointers() {
    VertexBase::disableVertexAttributePointers();

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

VertexVec4Vec3 VertexVec4Vec3::interpolate(const VertexVec4Vec3& v1, const VertexVec4Vec3& v2, float t) {
    return VertexVec4Vec3((v1.pos_ * (1.0f - t)) + (v2.pos_ * t), (v1.attr1_* (1.0f - t)) + (v2.attr1_* t), (v1.attr2_* (1.0f - t)) + (v2.attr2_* t));
    //return VertexVec4Vec3((v1.pos_ * (1.0f - t)) + (v2.pos_ * t), (v1.attr1_* (1.0f - t)) + (v2.attr1_* t), tgt::vec3(0.0f));
}

void VertexVec4Vec3::setNormal(tgt::vec3 n) {
    attr2_ = n;
}

} // namespace
