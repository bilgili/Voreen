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

#include "voreen/core/datastructures/volume/volumegl.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumeoperator.h"
#include "tgt/gpucapabilities.h"

#include <algorithm>
#include <typeinfo>

#ifdef VRN_MODULE_FLOWREEN
#include "voreen/modules/flowreen/volumeflow3d.h"
#endif

using tgt::vec3;
using tgt::bvec3;
using tgt::ivec3;
using tgt::col4;
using tgt::mat4;

namespace {

// calculates next power of two if it is not already power of two
static int fitPowerOfTwo(int i) {
    int result = 1;

    while (result < i)
        result <<= 1;

    return result;
}

// same as above but for each component of a ivec3
static ivec3 fitPowerOfTwo(const ivec3& v) {
    ivec3 result;

    result.x = fitPowerOfTwo(v.x);
    result.y = fitPowerOfTwo(v.y);
    result.z = fitPowerOfTwo(v.z);

    return result;
}

// returns true, if dims.x, dims.y and dims.z are power of two
bool isPowerOfTwo(const ivec3& dims) {
    return dims == fitPowerOfTwo(dims);
}

} // namespace

namespace voreen {

const std::string VolumeGL::loggerCat_("voreen.VolumeGL");

VolumeGL::VolumeGL(Volume* volume,
                   tgt::Texture::Filter filter) throw (std::bad_alloc)
  : origVolume_(volume),
    volume_(volume),
    volumeType_( typeid(*volume) ),
    filter_(filter)
{
    /*
        check volume type and set appropiate format_, internalFormat_ and dataType_ values
    */

    tgtAssert(volume, "No volume");
    tgtAssert(tgt::hand(tgt::greaterThan(volume->getDimensions(), ivec3(1))),
        "Volume must have a size greater one in all dimensions");

    // VolumeUIntX
    if (volumeType_ == typeid(VolumeUInt8)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA8;
        dataType_ = GL_UNSIGNED_BYTE;
    }
    else if (volumeType_ == typeid(VolumeUInt16)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA16;
        dataType_ = GL_UNSIGNED_SHORT;
    }
    else if (volumeType_ == typeid(VolumeUInt32)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA;
//         format_ = GL_ALPHA_INTEGER_EXT;
//         internalFormat_ = GL_ALPHA32UI_EXT;
        dataType_ = GL_UNSIGNED_INT;
    }
    // VolumeIntX
    else if (volumeType_ == typeid(VolumeInt8)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA8;
        dataType_ = GL_BYTE;
    }
    else if (volumeType_ == typeid(VolumeInt16)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA16;
        dataType_ = GL_SHORT;
    }
    else if (volumeType_ == typeid(VolumeInt32)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA;
        dataType_ = GL_INT;
    }
    // VolumeFloat and VolumeDouble
    else if (volumeType_ == typeid(VolumeFloat)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA;
        dataType_ = GL_FLOAT;
    }
    else if (volumeType_ == typeid(VolumeDouble)) {
        LERROR("OpenGL does not support double textures directly");
    }
    // Volume2x with int16 types
    else if (volumeType_ == typeid(Volume2xUInt16)) {
        format_ = GL_LUMINANCE_ALPHA;
        internalFormat_ = GL_LUMINANCE16_ALPHA16;
        dataType_ = GL_UNSIGNED_SHORT;
    }
    else if (volumeType_ == typeid(Volume2xInt16)) {
        format_ = GL_LUMINANCE_ALPHA;
        internalFormat_ = GL_LUMINANCE16_ALPHA16;
        dataType_ = GL_SHORT;
    }
    // Volume3x with int8 types
    else if (volumeType_ == typeid(Volume3xUInt8)) {
        format_ = GL_RGB;
        internalFormat_ = GL_RGB8;
        dataType_ = GL_UNSIGNED_BYTE;
    }
    else if (volumeType_ == typeid(Volume3xInt8)) {
        format_ = GL_RGB;
        internalFormat_ = GL_RGB8;
        dataType_ = GL_BYTE;
    }
    // Volume4x with int8 types
    else if (volumeType_ == typeid(Volume4xUInt8)) {
        format_ = GL_RGBA;
        internalFormat_ = GL_RGBA8;
        dataType_ = GL_UNSIGNED_BYTE;
    }
    else if (volumeType_ == typeid(Volume4xInt8)) {
        format_ = GL_RGBA;
        internalFormat_ = GL_RGBA8;
        dataType_ = GL_BYTE;
    }
    // Volume3x with int16 types
    else if (volumeType_ == typeid(Volume3xUInt16)) {
        format_ = GL_RGB;
        internalFormat_ = GL_RGB16;
        dataType_ = GL_UNSIGNED_SHORT;
    }
    else if (volumeType_ == typeid(Volume3xInt16)) {
        format_ = GL_RGB;
        internalFormat_ = GL_RGB16;
        dataType_ = GL_SHORT;
    }
    // Volume4x with int16 types
    else if (volumeType_ == typeid(Volume4xUInt16)) {
        format_ = GL_RGBA;
        internalFormat_ = GL_RGBA16;
        dataType_ = GL_UNSIGNED_SHORT;
    }
    else if (volumeType_ == typeid(Volume4xInt16)) {
        format_ = GL_RGBA;
        internalFormat_ = GL_RGBA16;
        dataType_ = GL_SHORT;
    }
    // Volume3x with real types
    else if (volumeType_ == typeid(Volume3xFloat)) {
        format_ = GL_RGB;
        internalFormat_ = GL_RGB;
        dataType_ = GL_FLOAT;
    }
#ifdef VRN_MODULE_FLOWREEN
    else if (volumeType_ == typeid(VolumeFlow3D)) {
        format_ = GL_RGB;
        internalFormat_ = GL_RGB16;
        dataType_ = GL_FLOAT;
    }
#endif
    else if (volumeType_ == typeid(Volume3xDouble)) {
        tgtAssert(false, "OpenGL does not support this kind of volume directly");
    }
    // Volume4x with real types
    else if (volumeType_ == typeid(Volume4xFloat)) {
        format_ = GL_RGBA;
        internalFormat_ = GL_RGBA;
        dataType_ = GL_FLOAT;
    }
    else if (volumeType_ == typeid(Volume4xDouble)) {
        tgtAssert(false, "OpenGL does not support this kind of volume directly");
    }
    // -> not found
    else {
        tgtAssert(false, "unsupported volume format_");
    }

    try {
        generateTextures();
    }
    catch (std::bad_alloc) {
        // release all resources
        destroy();
        throw; // throw it to the caller
    }
}

VolumeGL::~VolumeGL() {
    destroy();
}

void VolumeGL::destroy() {
    // delete created VolumeTexture objects
    for (size_t i = 0; i < textures_.size(); ++i)
        delete textures_[i];

    // check whether a temporary scaled volume was created and delete it
    if (volume_ != origVolume_)
        delete volume_;
}

/*
 * getters and setters
 */

size_t VolumeGL::getNumTextures() const {
    return textures_.size();
}

const VolumeTexture* VolumeGL::getTexture(size_t i /*= 0*/) const {
    tgtAssert(i<getNumTextures(), "Index out of bounds!");
    return textures_[i];
}

VolumeTexture* VolumeGL::getTexture(size_t i /*= 0*/) {
    tgtAssert(i<getNumTextures(), "Index out of bounds!");
    return textures_[i];
}

Volume* VolumeGL::getVolume() {
    return volume_;
}

const Volume* VolumeGL::getVolume() const {
    return volume_;
}

tgt::Texture::Filter VolumeGL::getFilter() const {
    return filter_;
}

void VolumeGL::setFilter(tgt::Texture::Filter filter) {
    filter_ = filter;
}

void VolumeGL::generateTextures() throw (std::bad_alloc) {

    if (!GpuCaps.is3DTexturingSupported()) {
        LERROR("No 3D texture support");
        return;
    }

    ivec3 volumeDims = volume_->getDimensions();
    vec3 volumeSpacing = volume_->getSpacing();

    // set pixel store with no alignment when reading from main memory
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /// check if non-power-of-two textures are not supported
    if (!GpuCaps.isNpotSupported() && !isPowerOfTwo(volumeDims)) {

        // this stores the new POT-dataset
        Volume* potVolume = 0;

        // -> no non-power-of-two-support
        LINFO("No NPOT-support available, starting to bloat to POT...");

        try {
            VolumeOperatorCreateSubset voCreateSubset(ivec3(0, 0, 0), fitPowerOfTwo(volumeDims));
            potVolume = voCreateSubset.apply<Volume*>(volume_);
        }
        catch (std::bad_alloc) {
            throw; // throw it to the caller
        }

        ivec3 potVolumeDims = potVolume->getDimensions();

        // store transformation for texCoords
        vec3 scale = vec3(volumeDims)/vec3(potVolumeDims);

        // create and upload texture
        uploadTexture(potVolume, mat4::createScale(scale),
            volume_->getLLF(), volume_->getURB());

        // destroy the newly create potVolume
        delete potVolume;
    }
    else {
        // non-power-of-two-support available -> create and upload texture
        uploadTexture(volume_, mat4::identity, volume_->getLLF(), volume_->getURB());
    }

}

void VolumeGL::uploadTexture(Volume* v, const mat4& matrix, const vec3& llf, const vec3& urb) {

    tgt::vec3* temp2 = 0;
#ifdef VRN_MODULE_FLOWREEN
    // Textures containing flow data need to contain data
    // within range [0.0, 1.0], so the values have to be mapped
    //
    VolumeFlow3D* flowTex = dynamic_cast<VolumeFlow3D*>(v);
    if (flowTex) {
        const float minValue = flowTex->getMinValue();
        const float maxValue = flowTex->getMaxValue();
        const float range = (maxValue - minValue);

        const tgt::vec3* const voxels = flowTex->voxel();
        temp2 = new tgt::vec3[v->getNumVoxels()];
        for (size_t i = 0; i < v->getNumVoxels(); ++i) {
            if (voxels[i] != tgt::vec3::zero) {
                temp2[i].x = (voxels[i].x - minValue) / range;
                temp2[i].y = (voxels[i].y - minValue) / range;
                temp2[i].z = (voxels[i].z - minValue) / range;
            }
        }
    }
#endif

    // create texture
    VolumeTexture* vTex = 0;

    // use temp data if this was created
    if (temp2) {
        vTex = new VolumeTexture(reinterpret_cast<GLubyte*>(temp2),
            matrix, llf, urb, v->getDimensions(),
            format_, internalFormat_, dataType_, filter_);
    }
    else {
        vTex = new VolumeTexture(static_cast<GLubyte*>(v->getData()),
            matrix, llf, urb, v->getDimensions(),
            format_, internalFormat_, dataType_, filter_);
    }

    LGL_ERROR;

    vTex->bind();
    if (v->getData())
        vTex->uploadTexture();

    // set texture wrap to clamp
    vTex->setWrapping(tgt::Texture::CLAMP);

    LGL_ERROR;

    // delete temporary data that has eventually be created
    delete[] temp2;

    // prevent deleting twice
    vTex->setPixelData(0);

    // append to internal data structure
    textures_.push_back(vTex);

    LGL_ERROR;
}

} // namespace voreen
