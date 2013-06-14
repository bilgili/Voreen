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

#ifndef VRN_VOLUMEGL_H
#define VRN_VOLUMEGL_H

#include <typeinfo>
#include <vector>

#include "tgt/tgt_gl.h"

#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volumetexture.h"
#include "voreen/core/datastructures/meta/realworldmappingmetadata.h"
#include "voreen/core/utils/exception.h"

namespace voreen {

/**
 * This class is the OpenGL interface for volume objects.
 * One or several 3D textures, which hold the complete data set, are created.
 */
class VRN_CORE_API VolumeGL : public VolumeRepresentation {
public:

    /**
     * Creates the VolumeTexture instance.
     *
     * @note The volume must have a size greater one in all dimensions
     *
     * @throw VoreenException if the volume type is not supported by OpenGL
     * @throw std::bad_alloc
     */
    VolumeGL(const VolumeRAM* volume)
        throw (VoreenException, std::bad_alloc);

    virtual ~VolumeGL();

    /**
     * Returns a const pointer to the texture with index \p i.
     * The VolumeTexture pointers \e DO \e NOT have any special order. If you
     * want to sort them by distance use \a getSortedTextures
     *
     * @param i The index of the desired VolumeTexture
     * @return The desired VolumeTexture
     */
    const VolumeTexture* getTexture() const;

    /// @overload
    VolumeTexture* getTexture();

    virtual std::string getFormat() const;
    virtual std::string getBaseType() const;
    virtual size_t getNumChannels() const;
    virtual size_t getBytesPerVoxel() const;

    /**
     * Returns the real-world mapping that has been applied during texture upload
     * (GL_*_SCALE, GL_*_BIAS).
     *
     * Signed integer types are mapped from [-1.0:1.0] to [0.0:1.0] in order to avoid
     * clamping of negative values. For all other data types, no mapping is applied
     * (except the standard OpenGL normalization of integer types).
     */
    virtual RealWorldMapping getPixelTransferMapping() const;

protected:
    /**
     * Determines the volume type and creates the according OpenGL texture.
     * Internally called by the constructor.
     *
     * @throw VoreenException if the volume type is not supported by OpenGL
     * @throw std::bad_alloc
     */
    void generateTexture(const VolumeRAM* vol)
        throw (VoreenException, std::bad_alloc);

    VolumeTexture* texture_;
    std::string format_;
    std::string baseType_;

    /// scale factor and offset used during texture upload (GL_*_SCALE, GL_*_BIAS)
    RealWorldMapping pixelTransferMapping_;

    static const std::string loggerCat_;

private:
    /// Used internally for destruction of the data.
    void destroy();
};

class RepresentationConverterUploadGL : public RepresentationConverter<VolumeGL> {
public:
    virtual bool canConvert(const VolumeRepresentation* source) const;
    virtual VolumeRepresentation* convert(const VolumeRepresentation* source) const;
};

class RepresentationConverterDownloadGL : public RepresentationConverter<VolumeRAM> {
public:
    virtual bool canConvert(const VolumeRepresentation* source) const;
    virtual VolumeRepresentation* convert(const VolumeRepresentation* source) const;
};

} // namespace voreen

#endif // VRN_VOLUMEGL_H
