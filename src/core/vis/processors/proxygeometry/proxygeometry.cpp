/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"

#include "tgt/plane.h"
#include "tgt/quadric.h"
#include "tgt/glmath.h"

namespace voreen {

using tgt::vec3;
using tgt::vec4;

const std::string ProxyGeometry::setUseClipping_("useClipping");
const std::string ProxyGeometry::setLeftClipPlane_("leftClipPlane");
const std::string ProxyGeometry::setRightClipPlane_("rightClipPlane");
const std::string ProxyGeometry::setTopClipPlane_("topClipPlane");
const std::string ProxyGeometry::setBottomClipPlane_("bottomClipPlane");
const std::string ProxyGeometry::setFrontClipPlane_("frontClipPlane");
const std::string ProxyGeometry::setBackClipPlane_("backClipPlane");
const std::string ProxyGeometry::resetClipPlanes_("clipPlanes");

ProxyGeometry::ProxyGeometry()
    : VolumeRenderer()
    , needsBuild_(true)
    , volumeSize_(tgt::vec3::zero)
    , volumeCenter_(tgt::vec3::zero)
    , volume_(0)
    , applyDatasetTransformationMatrix_("useDatasetTrafoMatrix", "Apply data set trafo matrix", true, Processor::INVALID_PARAMETERS)
    , inport_(Port::INPORT, "volumehandle.volumehandle")
    , cpPort_(Port::OUTPORT, "coprocessor.proxygeometry", true)
{
    addPort(inport_);
    addPort(cpPort_);
}

void ProxyGeometry::process() {
    if (inport_.isReady()) {
        volume_ = inport_.getData()->getVolume();
        if (volume_) {
            needsBuild_ = true;
            // getCubeSize() returns the size mapped to [-1,1] and we want [-0.5,0.5] here
            volumeSize_ = volume_->getCubeSize();
        }
    } 
}

vec3 ProxyGeometry::getVolumeSize() {
    return volumeSize_;
}

inline tgt::vec3 ProxyGeometry::vertexToTexCoord(const tgt::vec3& vertex) const {
    // Texture coordinates need to be within [0.0, 1.0] for all three
    // dimensions, but the vertex positions are probably in [-1.0, 1.0],
    // so find a stretching factor for each dimension. Example:
    //
    // getVolumeSize() = [2.0, 2.0, 0.25];
    // volumeSize_ = getVolumeSize() / 2 = [1.0, 1.0, 0.125]
    // => vertices ranging from [-1.0, -1.0, -0.125] - [1.0, 1.0, 0.125]
    // => stretch componentwise by stretch = [1.0, 1.0, 8.0] to get
    // vertex * stretch in range from [-1.0, -1.0, -1.0] - [1.0, 1.0, 1.0]
    // => add componentwise 1.0 and divide by 2 to get texture coordinates
    // within [0.0, 1.0]
    //
    // Annoying, I know, but needs to be done... (df)
    //
    return (((vertex / volumeSize_) + tgt::vec3(1.0f)) / 2.0f);
}

} // namespace voreen
