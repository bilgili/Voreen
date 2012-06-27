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

#include "voreen/modules/base/processors/geometry/meshclipping.h"

namespace voreen {

const std::string MeshClipping::loggerCat_("voreen.MeshClipping");

MeshClipping::MeshClipping()
    : Processor()
    , inport_(Port::INPORT, "geometry.geometry")
    , outport_(Port::OUTPORT, "geometry.clippedgeometry")
    , normal_("planeNormal", "Plane Normal", tgt::vec3(0, 1, 0), tgt::vec3(-1), tgt::vec3(1))
    , position_("planePosition", "Plane Position", 0.0f, -10.0f, 10.0f)
{
    addPort(inport_);
    addPort(outport_);

    addProperty(normal_);
    addProperty(position_);
}

MeshClipping::~MeshClipping()
{}

std::string MeshClipping::getProcessorInfo() const {
    return "Clips a list of mesh geometries (MeshListGeometry) against an arbitrarily oriented clipping plane.<br/>"
           "See MeshClippingWidget for a convenient manipulation of the plane.";
}

Processor* MeshClipping::create() const {
    return new MeshClipping();
}

void MeshClipping::process() {
    tgtAssert(inport_.getData(), "no geometry");

    MeshListGeometry* inportGeometry = dynamic_cast<MeshListGeometry*>(inport_.getData());
    if (!inportGeometry) {
        LWARNING("Input geometry type not supported, expecting MeshListGeometry.");
        outport_.setData(0);
        return;
    }

    geometry_ = *inportGeometry;

    geometry_.clip(tgt::vec4(tgt::normalize(normal_.get()), position_.get()));

    outport_.setData(&geometry_);
}

}  //namespace
