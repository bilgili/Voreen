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

#ifndef VRN_VOLUMEGL_H
#define VRN_VOLUMEGL_H

#include <typeinfo>
#include <vector>

#include "tgt/tgt_gl.h"

#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumetexture.h"

namespace voreen {

/**
 * This class is the OpenGL interface for volume objects.
 * One or several 3D textures, which hold the complete data set, are created.
 */
class VolumeGL {
public:

    /**
     * Creates the VolumeTexture instances.
     *
     * @note The volume must have a size greater one in all dimensions
     */
    VolumeGL(Volume* volume, tgt::Texture::Filter filter = tgt::Texture::LINEAR)
        throw (std::bad_alloc);

    /// This class will not delete its \a volume_.
    virtual ~VolumeGL();

    /// Returns the number of textures which are held by this class
    size_t getNumTextures() const;

    /**
     * Returns a const pointer to the texture with index \p i.
     * The VolumeTexture pointers \e DO \e NOT have any special order. If you
     * want to sort them by distance use \a getSortedTextures
     *
     * @param i The index of the desired VolumeTexture
     * @return The desired VolumeTexture
     */
    const VolumeTexture* getTexture(size_t i = 0) const;

    /// @overload
    VolumeTexture* getTexture(size_t i = 0);

    /**
     * Returns a pointer to the Volume which was used to build an instance
     * of this class
     *
     * @return The Volume which was used in the contructor.
     */
    Volume* getVolume();

    /**
     * Returns a const pointer to the Volume which was used to build an instance
     * of this class
     *
     * @return The Volume which was used in the contructor.
     */
    const Volume* getVolume() const;

    /**
     * Returns the filter which is used during the creation of textures.
     *
     * @return The used filter.
     */
    tgt::Texture::Filter getFilter() const;

    /**
     * Sets the filter which should be used during the creation of textures.
     *
     * @param filter The filter which should be used.
     */
    void setFilter(tgt::Texture::Filter filter);

protected:
    /**
     * Used internally by the constructor.
     */
    void generateTextures() throw(std::bad_alloc);

    /**
     * Used internally to upload newly created textures.
     */
    void uploadTexture(Volume* v,
                       const tgt::mat4& matrix,
                       const tgt::vec3& llf,
                       const tgt::vec3& urb);

    Volume* origVolume_; ///< The original Volume which was specified when calling the constructor.
    Volume* volume_;     ///< All work is done on this Volume. Can be just a pointer to *origVolume_ or a resized Volume.

    const std::type_info& volumeType_;///< The type_info of the Volume which is used to create this class.

    tgt::Texture::Filter filter_; ///< Filter mode used when creating textures.

    GLint format_;        ///< The format of textures which will are created.
    GLint internalFormat_;///< The internal format of the textures which are created.
    GLenum dataType_;     ///< The data type of the textures which are created.

    typedef std::vector<VolumeTexture*> VolumeTextures;
    VolumeTextures textures_;

    static const std::string loggerCat_;

private:
    /// Used internally for destruction of the data.
    void destroy();

};

} // namespace voreen

#endif // VRN_VOLUMEGL_H
