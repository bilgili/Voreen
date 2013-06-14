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

#include "voreen/core/datastructures/geometry/trianglemeshgeometry.h"

namespace voreen {

using tgt::vec3;
using tgt::vec4;

TriangleMeshGeometryBase::TriangleMeshGeometryBase()
    : Geometry()
{}

TriangleMeshGeometryVec3* TriangleMeshGeometryVec3::createCube(VertexType llfVertex, VertexType urbVertex) {
    TriangleMeshGeometryVec3* ret = new TriangleMeshGeometryVec3();
    ret->addCube(llfVertex, urbVertex);
    return ret;
}

void TriangleMeshGeometryVec3::addCube(VertexType llfVertex, VertexType urbVertex) {
    // expecting llfVertex.pos_ < urbVertex.pos_
    if (llfVertex.pos_.x > urbVertex.pos_.x) {
        std::swap(llfVertex.pos_.x, urbVertex.pos_.x);
        std::swap(llfVertex.attr1_.x, urbVertex.attr1_.x);
    }
    if (llfVertex.pos_.y > urbVertex.pos_.y) {
        std::swap(llfVertex.pos_.y, urbVertex.pos_.y);
        std::swap(llfVertex.attr1_.y, urbVertex.attr1_.y);
    }
    if (llfVertex.pos_.z > urbVertex.pos_.z) {
        std::swap(llfVertex.pos_.z, urbVertex.pos_.z);
        std::swap(llfVertex.attr1_.z, urbVertex.attr1_.z);
    }

    VertexVec3 llf(tgt::vec3(llfVertex.pos_.x, llfVertex.pos_.y, llfVertex.pos_.z), tgt::vec3(llfVertex.attr1_.x, llfVertex.attr1_.y, llfVertex.attr1_.z));
    VertexVec3 lrf(tgt::vec3(urbVertex.pos_.x, llfVertex.pos_.y, llfVertex.pos_.z), tgt::vec3(urbVertex.attr1_.x, llfVertex.attr1_.y, llfVertex.attr1_.z));
    VertexVec3 lrb(tgt::vec3(urbVertex.pos_.x, llfVertex.pos_.y, urbVertex.pos_.z), tgt::vec3(urbVertex.attr1_.x, llfVertex.attr1_.y, urbVertex.attr1_.z));
    VertexVec3 llb(tgt::vec3(llfVertex.pos_.x, llfVertex.pos_.y, urbVertex.pos_.z), tgt::vec3(llfVertex.attr1_.x, llfVertex.attr1_.y, urbVertex.attr1_.z));

    VertexVec3 ulb(tgt::vec3(llfVertex.pos_.x, urbVertex.pos_.y, urbVertex.pos_.z), tgt::vec3(llfVertex.attr1_.x, urbVertex.attr1_.y, urbVertex.attr1_.z));
    VertexVec3 ulf(tgt::vec3(llfVertex.pos_.x, urbVertex.pos_.y, llfVertex.pos_.z), tgt::vec3(llfVertex.attr1_.x, urbVertex.attr1_.y, llfVertex.attr1_.z));
    VertexVec3 urf(tgt::vec3(urbVertex.pos_.x, urbVertex.pos_.y, llfVertex.pos_.z), tgt::vec3(urbVertex.attr1_.x, urbVertex.attr1_.y, llfVertex.attr1_.z));
    VertexVec3 urb(tgt::vec3(urbVertex.pos_.x, urbVertex.pos_.y, urbVertex.pos_.z), tgt::vec3(urbVertex.attr1_.x, urbVertex.attr1_.y, urbVertex.attr1_.z));

    addTriangle(TriangleType(urb, urf, ulf));
    addTriangle(TriangleType(urb, ulf, ulb));

    addTriangle(TriangleType(llf, ulf, urf));
    addTriangle(TriangleType(llf, urf, lrf));

    addTriangle(TriangleType(llf, llb, ulb));
    addTriangle(TriangleType(llf, ulb, ulf));

    addTriangle(TriangleType(urb, ulb, llb));
    addTriangle(TriangleType(urb, llb, lrb));

    addTriangle(TriangleType(urb, lrb, lrf));
    addTriangle(TriangleType(urb, lrf, urf));

    addTriangle(TriangleType(llf, lrf, lrb));
    addTriangle(TriangleType(llf, lrb, llb));
}

//-------------------------------------------------------------------------------------------------

TriangleMeshGeometryVec4Vec3* TriangleMeshGeometryVec4Vec3::createCube(tgt::vec3 coordLlf, tgt::vec3 coordUrb, tgt::vec3 colorLlf, tgt::vec3 colorUrb, float alpha, tgt::vec3 texLlf, tgt::vec3 texUrb) {
    // expecting coordLlf < coordUrb
    if (coordLlf.x > coordUrb.x) {
        std::swap(coordLlf.x, coordUrb.x);
        std::swap(texLlf.x, texUrb.x);
        std::swap(colorLlf.x, colorUrb.x);
    }
    if (coordLlf.y > coordUrb.y) {
        std::swap(coordLlf.y, coordUrb.y);
        std::swap(texLlf.y, texUrb.y);
        std::swap(colorLlf.y, colorUrb.y);
    }
    if (coordLlf.z > coordUrb.z) {
        std::swap(coordLlf.z, coordUrb.z);
        std::swap(texLlf.z, texUrb.z);
        std::swap(colorLlf.z, colorUrb.z);
    }

    TriangleMeshGeometryVec4Vec3* ret = new TriangleMeshGeometryVec4Vec3();

    VertexVec4Vec3 llf(tgt::vec3(coordLlf.x, coordLlf.y, coordLlf.z), tgt::vec4(colorLlf.x, colorLlf.y, colorLlf.z, alpha), tgt::vec3(texLlf.x, texLlf.y, texLlf.z));
    VertexVec4Vec3 lrf(tgt::vec3(coordUrb.x, coordLlf.y, coordLlf.z), tgt::vec4(colorUrb.x, colorLlf.y, colorLlf.z, alpha), tgt::vec3(texUrb.x, texLlf.y, texLlf.z));
    VertexVec4Vec3 lrb(tgt::vec3(coordUrb.x, coordLlf.y, coordUrb.z), tgt::vec4(colorUrb.x, colorLlf.y, colorUrb.z, alpha), tgt::vec3(texUrb.x, texLlf.y, texUrb.z));
    VertexVec4Vec3 llb(tgt::vec3(coordLlf.x, coordLlf.y, coordUrb.z), tgt::vec4(colorLlf.x, colorLlf.y, colorUrb.z, alpha), tgt::vec3(texLlf.x, texLlf.y, texUrb.z));

    VertexVec4Vec3 ulb(tgt::vec3(coordLlf.x, coordUrb.y, coordUrb.z), tgt::vec4(colorLlf.x, colorUrb.y, colorUrb.z, alpha), tgt::vec3(texLlf.x, texUrb.y, texUrb.z));
    VertexVec4Vec3 ulf(tgt::vec3(coordLlf.x, coordUrb.y, coordLlf.z), tgt::vec4(colorLlf.x, colorUrb.y, colorLlf.z, alpha), tgt::vec3(texLlf.x, texUrb.y, texLlf.z));
    VertexVec4Vec3 urf(tgt::vec3(coordUrb.x, coordUrb.y, coordLlf.z), tgt::vec4(colorUrb.x, colorUrb.y, colorLlf.z, alpha), tgt::vec3(texUrb.x, texUrb.y, texLlf.z));
    VertexVec4Vec3 urb(tgt::vec3(coordUrb.x, coordUrb.y, coordUrb.z), tgt::vec4(colorUrb.x, colorUrb.y, colorUrb.z, alpha), tgt::vec3(texUrb.x, texUrb.y, texUrb.z));

    ret->addTriangle(TriangleType(urb, urf, ulf));
    ret->addTriangle(TriangleType(urb, ulf, ulb));

    ret->addTriangle(TriangleType(llf, ulf, urf));
    ret->addTriangle(TriangleType(llf, urf, lrf));

    ret->addTriangle(TriangleType(llf, llb, ulb));
    ret->addTriangle(TriangleType(llf, ulb, ulf));

    ret->addTriangle(TriangleType(urb, ulb, llb));
    ret->addTriangle(TriangleType(urb, llb, lrb));

    ret->addTriangle(TriangleType(urb, lrb, lrf));
    ret->addTriangle(TriangleType(urb, lrf, urf));

    ret->addTriangle(TriangleType(llf, lrf, lrb));
    ret->addTriangle(TriangleType(llf, lrb, llb));

    return ret;
}

TriangleMeshGeometryVec4Vec3* TriangleMeshGeometryVec4Vec3::createCube(tgt::vec3 coordLlf, tgt::vec3 coordUrb, tgt::vec3 colorLlf, tgt::vec3 colorUrb, float alpha) {
    // expecting coordLlf < coordUrb
    if (coordLlf.x > coordUrb.x) {
        std::swap(coordLlf.x, coordUrb.x);
        std::swap(colorLlf.x, colorUrb.x);
    }
    if (coordLlf.y > coordUrb.y) {
        std::swap(coordLlf.y, coordUrb.y);
        std::swap(colorLlf.y, colorUrb.y);
    }
    if (coordLlf.z > coordUrb.z) {
        std::swap(coordLlf.z, coordUrb.z);
        std::swap(colorLlf.z, colorUrb.z);
    }

    TriangleMeshGeometryVec4Vec3* ret = new TriangleMeshGeometryVec4Vec3();

    VertexVec4Vec3 llf(tgt::vec3(coordLlf.x, coordLlf.y, coordLlf.z), tgt::vec4(colorLlf.x, colorLlf.y, colorLlf.z, alpha), tgt::vec3(0.0f));
    VertexVec4Vec3 lrf(tgt::vec3(coordUrb.x, coordLlf.y, coordLlf.z), tgt::vec4(colorUrb.x, colorLlf.y, colorLlf.z, alpha), tgt::vec3(0.0f));
    VertexVec4Vec3 lrb(tgt::vec3(coordUrb.x, coordLlf.y, coordUrb.z), tgt::vec4(colorUrb.x, colorLlf.y, colorUrb.z, alpha), tgt::vec3(0.0f));
    VertexVec4Vec3 llb(tgt::vec3(coordLlf.x, coordLlf.y, coordUrb.z), tgt::vec4(colorLlf.x, colorLlf.y, colorUrb.z, alpha), tgt::vec3(0.0f));

    VertexVec4Vec3 ulb(tgt::vec3(coordLlf.x, coordUrb.y, coordUrb.z), tgt::vec4(colorLlf.x, colorUrb.y, colorUrb.z, alpha), tgt::vec3(0.0f));
    VertexVec4Vec3 ulf(tgt::vec3(coordLlf.x, coordUrb.y, coordLlf.z), tgt::vec4(colorLlf.x, colorUrb.y, colorLlf.z, alpha), tgt::vec3(0.0f));
    VertexVec4Vec3 urf(tgt::vec3(coordUrb.x, coordUrb.y, coordLlf.z), tgt::vec4(colorUrb.x, colorUrb.y, colorLlf.z, alpha), tgt::vec3(0.0f));
    VertexVec4Vec3 urb(tgt::vec3(coordUrb.x, coordUrb.y, coordUrb.z), tgt::vec4(colorUrb.x, colorUrb.y, colorUrb.z, alpha), tgt::vec3(0.0f));

    llf.attr2_ = lrf.attr2_ = lrb.attr2_ = llb.attr2_ = ulb.attr2_ = ulf.attr2_ = urf.attr2_ = urb.attr2_ = tgt::vec3(0.0f, 1.0f, 0.0f);
    ret->addTriangle(TriangleType(urb, urf, ulf));
    ret->addTriangle(TriangleType(urb, ulf, ulb));

    llf.attr2_ = lrf.attr2_ = lrb.attr2_ = llb.attr2_ = ulb.attr2_ = ulf.attr2_ = urf.attr2_ = urb.attr2_ = tgt::vec3(0.0f, 0.0f, -1.0f);
    ret->addTriangle(TriangleType(llf, ulf, urf));
    ret->addTriangle(TriangleType(llf, urf, lrf));

    llf.attr2_ = lrf.attr2_ = lrb.attr2_ = llb.attr2_ = ulb.attr2_ = ulf.attr2_ = urf.attr2_ = urb.attr2_ = tgt::vec3(-1.0f, 0.0f, 0.0f);
    ret->addTriangle(TriangleType(llf, llb, ulb));
    ret->addTriangle(TriangleType(llf, ulb, ulf));

    llf.attr2_ = lrf.attr2_ = lrb.attr2_ = llb.attr2_ = ulb.attr2_ = ulf.attr2_ = urf.attr2_ = urb.attr2_ = tgt::vec3(0.0f, 0.0f, 1.0f);
    ret->addTriangle(TriangleType(urb, ulb, llb));
    ret->addTriangle(TriangleType(urb, llb, lrb));

    llf.attr2_ = lrf.attr2_ = lrb.attr2_ = llb.attr2_ = ulb.attr2_ = ulf.attr2_ = urf.attr2_ = urb.attr2_ = tgt::vec3(1.0f, 0.0f, 0.0f);
    ret->addTriangle(TriangleType(urb, lrb, lrf));
    ret->addTriangle(TriangleType(urb, lrf, urf));

    llf.attr2_ = lrf.attr2_ = lrb.attr2_ = llb.attr2_ = ulb.attr2_ = ulf.attr2_ = urf.attr2_ = urb.attr2_ = tgt::vec3(0.0f, -1.0f, 0.0f);
    ret->addTriangle(TriangleType(llf, lrf, lrb));
    ret->addTriangle(TriangleType(llf, lrb, llb));

    return ret;
}

void TriangleMeshGeometryVec4Vec3::addCube(VertexVec3 llfVertex, VertexVec3 urbVertex) {
    // expecting llfVertex.pos_ < urbVertex.pos_
    if (llfVertex.pos_.x > urbVertex.pos_.x) {
        std::swap(llfVertex.pos_.x, urbVertex.pos_.x);
        std::swap(llfVertex.attr1_.x, urbVertex.attr1_.x);
    }
    if (llfVertex.pos_.y > urbVertex.pos_.y) {
        std::swap(llfVertex.pos_.y, urbVertex.pos_.y);
        std::swap(llfVertex.attr1_.y, urbVertex.attr1_.y);
    }
    if (llfVertex.pos_.z > urbVertex.pos_.z) {
        std::swap(llfVertex.pos_.z, urbVertex.pos_.z);
        std::swap(llfVertex.attr1_.z, urbVertex.attr1_.z);
    }

    float alpha = 1.0f;

    VertexVec4Vec3 llf(tgt::vec3(llfVertex.pos_.x, llfVertex.pos_.y, llfVertex.pos_.z), tgt::vec4(llfVertex.attr1_.x, llfVertex.attr1_.y, llfVertex.attr1_.z, alpha), tgt::vec3(0.0f));
    VertexVec4Vec3 lrf(tgt::vec3(urbVertex.pos_.x, llfVertex.pos_.y, llfVertex.pos_.z), tgt::vec4(urbVertex.attr1_.x, llfVertex.attr1_.y, llfVertex.attr1_.z, alpha), tgt::vec3(0.0f));
    VertexVec4Vec3 lrb(tgt::vec3(urbVertex.pos_.x, llfVertex.pos_.y, urbVertex.pos_.z), tgt::vec4(urbVertex.attr1_.x, llfVertex.attr1_.y, urbVertex.attr1_.z, alpha), tgt::vec3(0.0f));
    VertexVec4Vec3 llb(tgt::vec3(llfVertex.pos_.x, llfVertex.pos_.y, urbVertex.pos_.z), tgt::vec4(llfVertex.attr1_.x, llfVertex.attr1_.y, urbVertex.attr1_.z, alpha), tgt::vec3(0.0f));

    VertexVec4Vec3 ulb(tgt::vec3(llfVertex.pos_.x, urbVertex.pos_.y, urbVertex.pos_.z), tgt::vec4(llfVertex.attr1_.x, urbVertex.attr1_.y, urbVertex.attr1_.z, alpha), tgt::vec3(0.0f));
    VertexVec4Vec3 ulf(tgt::vec3(llfVertex.pos_.x, urbVertex.pos_.y, llfVertex.pos_.z), tgt::vec4(llfVertex.attr1_.x, urbVertex.attr1_.y, llfVertex.attr1_.z, alpha), tgt::vec3(0.0f));
    VertexVec4Vec3 urf(tgt::vec3(urbVertex.pos_.x, urbVertex.pos_.y, llfVertex.pos_.z), tgt::vec4(urbVertex.attr1_.x, urbVertex.attr1_.y, llfVertex.attr1_.z, alpha), tgt::vec3(0.0f));
    VertexVec4Vec3 urb(tgt::vec3(urbVertex.pos_.x, urbVertex.pos_.y, urbVertex.pos_.z), tgt::vec4(urbVertex.attr1_.x, urbVertex.attr1_.y, urbVertex.attr1_.z, alpha), tgt::vec3(0.0f));

    llf.attr2_ = lrf.attr2_ = lrb.attr2_ = llb.attr2_ = ulb.attr2_ = ulf.attr2_ = urf.attr2_ = urb.attr2_ = tgt::vec3(0.0f, 1.0f, 0.0f);
    addTriangle(TriangleType(urb, urf, ulf));
    addTriangle(TriangleType(urb, ulf, ulb));

    llf.attr2_ = lrf.attr2_ = lrb.attr2_ = llb.attr2_ = ulb.attr2_ = ulf.attr2_ = urf.attr2_ = urb.attr2_ = tgt::vec3(0.0f, 0.0f, -1.0f);
    addTriangle(TriangleType(llf, ulf, urf));
    addTriangle(TriangleType(llf, urf, lrf));

    llf.attr2_ = lrf.attr2_ = lrb.attr2_ = llb.attr2_ = ulb.attr2_ = ulf.attr2_ = urf.attr2_ = urb.attr2_ = tgt::vec3(-1.0f, 0.0f, 0.0f);
    addTriangle(TriangleType(llf, llb, ulb));
    addTriangle(TriangleType(llf, ulb, ulf));

    llf.attr2_ = lrf.attr2_ = lrb.attr2_ = llb.attr2_ = ulb.attr2_ = ulf.attr2_ = urf.attr2_ = urb.attr2_ = tgt::vec3(0.0f, 0.0f, 1.0f);
    addTriangle(TriangleType(urb, ulb, llb));
    addTriangle(TriangleType(urb, llb, lrb));

    llf.attr2_ = lrf.attr2_ = lrb.attr2_ = llb.attr2_ = ulb.attr2_ = ulf.attr2_ = urf.attr2_ = urb.attr2_ = tgt::vec3(1.0f, 0.0f, 0.0f);
    addTriangle(TriangleType(urb, lrb, lrf));
    addTriangle(TriangleType(urb, lrf, urf));

    llf.attr2_ = lrf.attr2_ = lrb.attr2_ = llb.attr2_ = ulb.attr2_ = ulf.attr2_ = urf.attr2_ = urb.attr2_ = tgt::vec3(0.0f, -1.0f, 0.0f);
    addTriangle(TriangleType(llf, lrf, lrb));
    addTriangle(TriangleType(llf, lrb, llb));
}

void TriangleMeshGeometryVec4Vec3::addQuad(const VertexVec3& v1, const VertexVec3& v2, const VertexVec3& v3, const VertexVec3& v4) {
    vec3 a = v2.pos_ - v1.pos_;
    vec3 b = v3.pos_ - v1.pos_;
    vec3 n = normalize(cross(a, b));
    float alpha = 1.0f;

    addTriangle(TriangleType(VertexVec4Vec3(v1.pos_, vec4(v1.attr1_, alpha), n), VertexVec4Vec3(v2.pos_, vec4(v2.attr1_, alpha), n), VertexVec4Vec3(v3.pos_, vec4(v3.attr1_, alpha), n)));
    addTriangle(TriangleType(VertexVec4Vec3(v1.pos_, vec4(v1.attr1_, alpha), n), VertexVec4Vec3(v3.pos_, vec4(v3.attr1_, alpha), n), VertexVec4Vec3(v4.pos_, vec4(v4.attr1_, alpha), n)));
}

//-------------------------------------------------------------------------------------------------

TriangleMeshGeometrySimple* TriangleMeshGeometrySimple::createCube(tgt::vec3 coordLlf, tgt::vec3 coordUrb) {
    // expecting coordLlf < coordUrb
    if (coordLlf.x > coordUrb.x) {
        std::swap(coordLlf.x, coordUrb.x);
    }
    if (coordLlf.y > coordUrb.y) {
        std::swap(coordLlf.y, coordUrb.y);
    }
    if (coordLlf.z > coordUrb.z) {
        std::swap(coordLlf.z, coordUrb.z);
    }

    TriangleMeshGeometrySimple* ret = new TriangleMeshGeometrySimple();

    VertexBase llf(tgt::vec3(coordLlf.x, coordLlf.y, coordLlf.z));
    VertexBase lrf(tgt::vec3(coordUrb.x, coordLlf.y, coordLlf.z));
    VertexBase lrb(tgt::vec3(coordUrb.x, coordLlf.y, coordUrb.z));
    VertexBase llb(tgt::vec3(coordLlf.x, coordLlf.y, coordUrb.z));

    VertexBase ulb(tgt::vec3(coordLlf.x, coordUrb.y, coordUrb.z));
    VertexBase ulf(tgt::vec3(coordLlf.x, coordUrb.y, coordLlf.z));
    VertexBase urf(tgt::vec3(coordUrb.x, coordUrb.y, coordLlf.z));
    VertexBase urb(tgt::vec3(coordUrb.x, coordUrb.y, coordUrb.z));

    ret->addTriangle(TriangleType(urb, urf, ulf));
    ret->addTriangle(TriangleType(urb, ulf, ulb));

    ret->addTriangle(TriangleType(llf, ulf, urf));
    ret->addTriangle(TriangleType(llf, urf, lrf));

    ret->addTriangle(TriangleType(llf, llb, ulb));
    ret->addTriangle(TriangleType(llf, ulb, ulf));

    ret->addTriangle(TriangleType(urb, ulb, llb));
    ret->addTriangle(TriangleType(urb, llb, lrb));

    ret->addTriangle(TriangleType(urb, lrb, lrf));
    ret->addTriangle(TriangleType(urb, lrf, urf));

    ret->addTriangle(TriangleType(llf, lrf, lrb));
    ret->addTriangle(TriangleType(llf, lrb, llb));

    return ret;
}

} // namespace
