/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/volume/volumetexture.h"

#include "tgt/plane.h"

using tgt::vec3;
using tgt::ivec3;
using tgt::mat4;
using tgt::plane;

namespace voreen {

/*
 * constructor
 */

VolumeTexture::VolumeTexture(GLubyte* data,
                             const mat4& matrix,
                             const vec3& llf,
                             const vec3& urb,
                             const ivec3& dimensions,
                             GLint format,
                             GLint internalformat,
                             GLenum dataType,
                             tgt::Texture::Filter filter)
  : tgt::Texture(data, dimensions, format, internalformat, dataType, filter),
    matrix_(matrix), llf_(llf), urb_(urb)
{
    // Always set to a 3d texture
    //FIXME: This causes opengl invalid operation when texture is only 2d
    setType(GL_TEXTURE_3D);

    // calculate cubeSize
    cubeSize_ = abs(urb - llf);

    // calculate position
    center_ = llf + cubeSize_ * vec3(0.5f, 0.5f, -0.5f); // keep right hand coordinate sytem in mind

    // calculate cube vertices
    plane::createCubeVertices(llf, urb, cubeVertices_);
}

/*
 * getters and setters
 */

mat4 VolumeTexture::getMatrix() const {
    return matrix_;
}

vec3 VolumeTexture::getCenter() const {
    return center_;
}

vec3 VolumeTexture::getLLF() const {
    return llf_;
}

tgt::vec3 VolumeTexture::getURB() const {
    return urb_;
}

const tgt::vec3* VolumeTexture::getCubeVertices() const {
    return cubeVertices_;
}

tgt::vec3 VolumeTexture::getCubeSize() const {
    return cubeSize_;
}

} // namespace voreen
