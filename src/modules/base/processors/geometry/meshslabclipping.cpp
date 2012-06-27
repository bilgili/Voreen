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

#include "voreen/modules/base/processors/geometry/meshslabclipping.h"

namespace voreen {

MeshSlabClipping::MeshSlabClipping()
    : Processor()
    , inport_(Port::INPORT, "geometry.geometry")
    , outport_(Port::OUTPORT, "geometry.clippedgeometry")
    , normal_("slabNormal", "Slab Normal", tgt::vec3(0, 1, 0), tgt::vec3(-1), tgt::vec3(1))
    , position_("slabPosition", "Slab Position", 0.0f, -10.0f, 10.0f)
    , thickness_("slabThickness", "Slab Thickness", 0.5f, 0.0f, 3.0f)
{
    addPort(inport_);
    addPort(outport_);

    addProperty(normal_);
    addProperty(position_);
    addProperty(thickness_);
}

MeshSlabClipping::~MeshSlabClipping()
{ }

Processor* MeshSlabClipping::create() const {
    return new MeshSlabClipping();
}

std::string MeshSlabClipping::getProcessorInfo() const {
    return "Cuts a slab of arbitrary orientation and thickness out of the input MeshListGeometry "
           "by clipping it against two parallel planes.";
}

void MeshSlabClipping::process() {
    tgtAssert(inport_.getData(), "no geometry");

    MeshListGeometry* inportGeometry = dynamic_cast<MeshListGeometry*>(inport_.getData());
    if (!inportGeometry) {
        LWARNING("Input geometry of type MeshListGeometry expected.");
        outport_.setData(0);
        return;
    }

    geometry_ = *inportGeometry;

    geometry_.clip(tgt::vec4(tgt::normalize( normal_.get()), position_.get()));
    geometry_.clip(tgt::vec4(tgt::normalize(-normal_.get()), -position_.get()+thickness_.get()));

    outport_.setData(&geometry_);
}

}  //namespace
