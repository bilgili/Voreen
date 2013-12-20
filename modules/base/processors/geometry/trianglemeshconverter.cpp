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

#include "trianglemeshconverter.h"
#include "voreen/core/datastructures/geometry/trianglemeshgeometry.h"

namespace voreen {

const std::string TriangleMeshConverter::loggerCat_("voreen.base.TriangleMeshConverter");

TriangleMeshConverter::TriangleMeshConverter()
    : Processor()
    , inport_(Port::INPORT, "geometry.geometry", "Geometry Input")
    , outport_(Port::OUTPORT, "geometry.clippedgeometry", "Converted Geometry Output")
    , enabled_("enabled", "Enabled", true)
    , targetMesh_("targetmesh", "Convert into...", Processor::INVALID_RESULT)
{
    addPort(inport_);
    addPort(outport_);

    addProperty(enabled_);
    targetMesh_.addOption("simple", "Simple");
    targetMesh_.addOption("vertexvec3", "VertexVec3");
    targetMesh_.addOption("vertexvec4vec3", "VertexVec4Vec3");
    addProperty(targetMesh_);
}

TriangleMeshConverter::~TriangleMeshConverter()
{}

Processor* TriangleMeshConverter::create() const {
    return new TriangleMeshConverter();
}

void TriangleMeshConverter::process() {
    const Geometry* inputGeometry = inport_.getData();
    tgtAssert(inport_.getData(), "no geometry");

    if (!enabled_.get()) {
        outport_.setData(inputGeometry, false);
        return;
    }
    const TriangleMeshGeometryBase* tm = dynamic_cast<const TriangleMeshGeometryBase*>(inputGeometry);
    if (!tm) {
        LWARNING("This processor can only be used with triangle meshes; skipping execution.");
        outport_.setData(inputGeometry, false);
        return;
    }

    Geometry* outputGeometry = 0;

    switch(tm->getVertexLayout()) {
        case TriangleMeshGeometryBase::SIMPLE: {
            const TriangleMeshGeometrySimple* tmgs = static_cast<const TriangleMeshGeometrySimple*>(tm);
            if(targetMesh_.get() == "simple") {
                outport_.setData(inputGeometry, false);
                return;
            } else if(targetMesh_.get() == "vertexvec3") {
                TriangleMeshGeometryVec3* tmgv3 = new TriangleMeshGeometryVec3();
                for(size_t i = 0; i < tmgs->getNumTriangles(); i++) {
                    Triangle<VertexBase> tb = tmgs->getTriangle(i);
                    // TODO optional normalization calculation
                    Triangle<VertexVec3> tv3(VertexVec3(tb.v_[0].pos_, tgt::vec3(1.f, 0.f, 0.f)),
                                             VertexVec3(tb.v_[1].pos_, tgt::vec3(1.f, 0.f, 0.f)),
                                             VertexVec3(tb.v_[2].pos_, tgt::vec3(1.f, 0.f, 0.f)));
                    tmgv3->addTriangle(tv3);
                }
                outputGeometry = tmgv3;
            } else if(targetMesh_.get() == "vertexvec4vec3") {
                TriangleMeshGeometryVec4Vec3* tmgv4v3 = new TriangleMeshGeometryVec4Vec3();
                for(size_t i = 0; i < tmgs->getNumTriangles(); i++) {
                    Triangle<VertexBase> tb = tmgs->getTriangle(i);
                    // TODO optional normalization calculation
                    Triangle<VertexVec4Vec3> tv4v3(VertexVec4Vec3(tb.v_[0].pos_, tgt::vec4(1.f), tgt::vec3(1.f, 0.f, 0.f)),
                                                   VertexVec4Vec3(tb.v_[1].pos_, tgt::vec4(1.f), tgt::vec3(1.f, 0.f, 0.f)),
                                                   VertexVec4Vec3(tb.v_[2].pos_, tgt::vec4(1.f), tgt::vec3(1.f, 0.f, 0.f)));
                    tmgv4v3->addTriangle(tv4v3);
                }
                outputGeometry = tmgv4v3;
            }
            break;
        }
        case TriangleMeshGeometryBase::VEC3: {
            const TriangleMeshGeometryVec3* tmgv3 = static_cast<const TriangleMeshGeometryVec3*>(tm);
            if(targetMesh_.get() == "simple") {
                    TriangleMeshGeometrySimple* tmgs = new TriangleMeshGeometrySimple();
                    for(size_t i = 0; i < tmgv3->getNumTriangles(); i++) {
                        Triangle<VertexVec3> tv3 = tmgv3->getTriangle(i);
                        // TODO optional normalization calculation
                        Triangle<VertexBase> tvb(VertexBase(tv3.v_[0].pos_), VertexBase(tv3.v_[1].pos_), VertexBase(tv3.v_[2].pos_));
                        tmgs->addTriangle(tvb);
                    }
                    outputGeometry = tmgs;
            } else if(targetMesh_.get() == "vertexvec3") {
                    // TODO optional normal calc
                    outport_.setData(inputGeometry, false);
                    return;
            } else if(targetMesh_.get() == "vertexvec4vec3") {
                    TriangleMeshGeometryVec4Vec3* tmgv4v3 = new TriangleMeshGeometryVec4Vec3();
                    for(size_t i = 0; i < tmgv3->getNumTriangles(); i++) {
                        Triangle<VertexVec3> tv3 = tmgv3->getTriangle(i);
                        // TODO optional color prop
                        Triangle<VertexVec4Vec3> tv4v3(VertexVec4Vec3(tv3.v_[0].pos_, tgt::vec4(1.f), tv3.v_[0].attr1_),
                                                       VertexVec4Vec3(tv3.v_[1].pos_, tgt::vec4(1.f), tv3.v_[1].attr1_),
                                                       VertexVec4Vec3(tv3.v_[2].pos_, tgt::vec4(1.f), tv3.v_[2].attr1_));
                        tmgv4v3->addTriangle(tv4v3);
                    }
                    outputGeometry = tmgv4v3;
            }
            break;
        }
        case TriangleMeshGeometryBase::VEC4VEC3: {
            const TriangleMeshGeometryVec4Vec3* tmgv4v3 = static_cast<const TriangleMeshGeometryVec4Vec3*>(tm);
            if(targetMesh_.get() == "simple") {
                TriangleMeshGeometrySimple* tmgs = new TriangleMeshGeometrySimple();
                for(size_t i = 0; i < tmgv4v3->getNumTriangles(); i++) {
                    Triangle<VertexVec4Vec3> tv4v3 = tmgv4v3->getTriangle(i);
                    // TODO optional normalization calculation
                    Triangle<VertexBase> tvb(VertexBase(tv4v3.v_[0].pos_), VertexBase(tv4v3.v_[1].pos_), VertexBase(tv4v3.v_[2].pos_));
                    tmgs->addTriangle(tvb);
                }
                outputGeometry = tmgs;
            } else if(targetMesh_.get() == "vertexvec3") {
                TriangleMeshGeometryVec3* tmgv3 = new TriangleMeshGeometryVec3();
                for(size_t i = 0; i < tmgv4v3->getNumTriangles(); i++) {
                    Triangle<VertexVec4Vec3> tv4v3 = tmgv4v3->getTriangle(i);
                    Triangle<VertexVec3> tv3(VertexVec3(tv4v3.v_[0].pos_, tv4v3.v_[0].attr2_),
                                             VertexVec3(tv4v3.v_[1].pos_, tv4v3.v_[1].attr2_),
                                             VertexVec3(tv4v3.v_[2].pos_, tv4v3.v_[2].attr2_));
                    tmgv3->addTriangle(tv3);
                }
                outputGeometry = tmgv3;
            } else if(targetMesh_.get() == "vertexvec4vec3") {
                // TODO optional normal calc / color conversion
                outport_.setData(inputGeometry, false);
                return;
            }
            break;
        }
        default:
            LWARNING("Unknown source triangle mesh layout; could not convert.");
            break;
    }

    if(outputGeometry)
        outputGeometry->setTransformationMatrix(tm->getTransformationMatrix());
    outport_.setData(outputGeometry);
}

}  //namespace
