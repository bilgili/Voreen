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

#include "sliceproxygeometry.h"

#include "voreen/core/datastructures/geometry/trianglemeshgeometry.h"
#include "tgt/glmath.h"

using tgt::ivec2;
using tgt::vec3;
using tgt::mat4;
using tgt::Texture;

namespace voreen {

const std::string SliceProxyGeometry::loggerCat_("voreen.SliceProxyGeometry");

SliceProxyGeometry::SliceProxyGeometry()
    : RenderProcessor()
    , normal_("planeNormal", "Plane Normal", tgt::vec3(0, 1, 0), tgt::vec3(-1), tgt::vec3(1))
    , position_("planePosition", "Plane Position", 0.0f, -10.0f, 10.0f)
    , size_("size", "Quad size", 10.0f, 0.1f, 100.0f)
    , mwheelCycleHandler_("mouseWheelHandler", "Slice Cycling", &position_)
    , geomPort_(Port::OUTPORT, "geometry", "Geometry Output")
{
    addInteractionHandler(mwheelCycleHandler_);

    addProperty(normal_);
    addProperty(position_);
    addProperty(size_);

    addPort(geomPort_);
}

SliceProxyGeometry::~SliceProxyGeometry() {
}

std::string SliceProxyGeometry::getProcessorInfo() const {
    return "Produces a quad in the specified plane to use as proxygeometry for multimodal slice rendering.";
}

void SliceProxyGeometry::process() {
    LGL_ERROR;

    vec3 n = normalize(normal_.get());

    vec3 temp = vec3(1.0, 0.0, 0.0);
    if(abs(dot(temp, n) > 0.9))
        temp = vec3(0.0, 1.0, 0.0);

    vec3 inPlaneA = normalize(cross(n, temp)) * 0.5f * size_.get();
    vec3 inPlaneB = normalize(cross(n, inPlaneA)) * 0.5f * size_.get();

    vec3 base = n * -position_.get();

    TriangleMeshGeometryVec3* slice = new TriangleMeshGeometryVec3();
    slice->addQuad(VertexVec3(base + inPlaneA + inPlaneB, base + inPlaneA + inPlaneB),
                   VertexVec3(base - inPlaneA + inPlaneB, base - inPlaneA + inPlaneB),
                   VertexVec3(base - inPlaneA - inPlaneB, base - inPlaneA - inPlaneB),
                   VertexVec3(base + inPlaneA - inPlaneB, base + inPlaneA - inPlaneB));

    LGL_ERROR;

    geomPort_.setData(slice);
}

} // namespace voreen
