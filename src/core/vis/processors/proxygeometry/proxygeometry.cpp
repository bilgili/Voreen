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

const Identifier ProxyGeometry::setUseClipping_("set.UseClipping");
const Identifier ProxyGeometry::setLeftClipPlane_("set.LeftClipPlane");
const Identifier ProxyGeometry::setRightClipPlane_("set.RightClipPlane");
const Identifier ProxyGeometry::setTopClipPlane_("set.TopClipPlane");
const Identifier ProxyGeometry::setBottomClipPlane_("set.BottomClipPlane");
const Identifier ProxyGeometry::setFrontClipPlane_("set.FrontClipPlane");
const Identifier ProxyGeometry::setBackClipPlane_("set.BackClipPlane");
const Identifier ProxyGeometry::resetClipPlanes_("reset.clipPlanes");
const Identifier ProxyGeometry::getVolumeSize_("get.volumeSize");

ProxyGeometry::ProxyGeometry()
    : VolumeRenderer()
    , needsBuild_(true)
    , volumeSize_(tgt::vec3::zero)
    , volumeCenter_(tgt::vec3::zero)
    , volume_(0)
{}

int ProxyGeometry::initializeGL() {
    return VRN_OK;
}

void ProxyGeometry::process(LocalPortMapping* portMapping) {
    bool changed = false;
    const bool res = VolumeHandleValidator::checkVolumeHandle(currentVolumeHandle_,
        portMapping->getVolumeHandle("volumehandle.volumehandle"), &changed);

    if ((res == true) && (changed == true)) {
        volume_ = currentVolumeHandle_->getVolumeGL()->getVolume();
        if (volume_ != 0) {
            needsBuild_ = true;
            // getCubeSize() returns the size mapped to [-1,1] and we want [-0.5,0.5] here
            volumeSize_ = volume_->getCubeSize();
        }
    }
}

Message* ProxyGeometry::call(Identifier ident, LocalPortMapping* /*portMapping*/) {
    if (ident == "render") {
        render();
        return 0;
    }
    else if (ident == getVolumeSize_) {
        return new Vec3Msg("", getVolumeSize()/2.f);
    }
    else
        return 0;
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
