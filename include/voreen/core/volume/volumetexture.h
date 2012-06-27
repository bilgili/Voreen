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

#ifndef VRN_VOLUMETEXTURE_H
#define VRN_VOLUMETEXTURE_H

#include "tgt/matrix.h"
#include "tgt/texturemanager.h"

namespace voreen {

/**
 * This class is basically a tgt::Texture with some Voreen specific extensions
 * used for splitting and NPOT support.
*/
class VolumeTexture : public tgt::Texture {
public:
    /**
     * With data and internalformat argument, type_ is set to GL_TEXTURE_3D
     * and \e NO new chunk of data will be allocated on the heap.
     */
    VolumeTexture(GLubyte* data,
                  const tgt::mat4& matrix,
                  const tgt::vec3& llf,
                  const tgt::vec3& urb,
                  const tgt::ivec3& dimensions,
                  GLint format,
                  GLint internalformat,
                  GLenum dataType,
                  tgt::Texture::Filter filter);

    /**
     * Returns the center position relativ the the corresponding volume data
     * in coordinates mapped to [-1, 1]
     */
    tgt::vec3 getCenter() const;

    /// Returns the texture matrix which should be used with this texture
    tgt::mat4 getMatrix() const;

    /// Get lower left front
    tgt::vec3 getLLF() const;

    /// Get upper right back
    tgt::vec3 getURB() const;

    /**
     * Get cube vertices
     *
     * @return An array of 8 tgt::vec3 which represent the cube vertices
     */
    const tgt::vec3* getCubeVertices() const;

    tgt::vec3 getCubeSize() const;

protected:
    tgt::mat4 matrix_;          ///< matrix which should be used to transform texture coordinates to the useful portion
    tgt::vec3 llf_;             ///< lower left front of the useful portion
    tgt::vec3 urb_;             ///< upper right back of the useful portion
    tgt::vec3 center_;          ///< center position of the useful portion relative to the original Volume mapped to [-1, 1]
    tgt::vec3 cubeVertices_[8]; ///< cube vertices of the useful portion
    tgt::vec3 cubeSize_;        ///< size of the cube of the useful portion in each direction
};

} // namespace voreen

#endif // VRN_VOLUMETEXTURE_H
