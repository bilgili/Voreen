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

#ifndef VRN_VOLUME_H
#define VRN_VOLUME_H

// Note: please ensure that no OpenGL dependencies are added to this file

#include "voreen/core/datastructures/volume/volumemetadata.h"

namespace voreen {

/**
 * OpenGL-independent base class for volumetric data sets.
 *
 * This class stores the raw data as well as the
 * required meta information about it. It does,
 * however, neither perform any OpenGL operations nor
 * does it hold any OpenGL-related properties.
 *
 * \sa VolumeGL
 *
 */
class Volume {
public:
    enum Filter {
        NEAREST,
        LINEAR
    };

    /**
     * Constructor.
     *
     * @param dimensions dimensions of the new dataset
     * @param bitsStored bit depth of the new dataset
     * @param spacing dimensions of each voxel of the new dataset
     */
    Volume(const tgt::ivec3& dimensions,
           int bitsStored,
           const tgt::vec3& spacing = tgt::vec3(1.f));

    virtual ~Volume() {}

    /// Use this as a kind of a virtual constructor.
    virtual Volume* clone() const throw (std::bad_alloc) = 0;

    /**
     * Use this as a kind of a virtual constructor that does _NOT_ copy over
     * the voxels but uses the given pointer instead as a voxel data. If this
     * pointer is 0, an empty volume without voxel data is created.
     */
    virtual Volume* clone(void* data) const throw (std::bad_alloc) = 0;

    /// Returns the volume's dimensions in voxel coordinates.
    virtual tgt::ivec3 getDimensions() const;

    /// Returns the lower left front mapped to [-1, 1] with spacing kept in mind.
    virtual tgt::vec3 getLLF() const;

    /// Returns the upper right backmapped to [-1, 1] with spacing kept in mind.
    virtual tgt::vec3 getURB() const;

    /// Returns the cube vertices mapped to [-1, 1] with spacing kept in mind.
    virtual const tgt::vec3* getCubeVertices() const;

    /// Returns the size of the cube mapped to [-1, 1] with spacing kept in mind.
    virtual tgt::vec3 getCubeSize() const;

    /// Specifies the voxel dimensions of the volume.
    virtual void setSpacing(const tgt::vec3 spacing);

    /// Returns the voxel dimensions of the volume.
    virtual tgt::vec3 getSpacing() const;

    /// Returns the number of voxels contained by the volume.
    virtual size_t getNumVoxels() const;

    /// Returns the number of bytes held in the \a data_ array.
    virtual size_t getNumBytes() const = 0;

    /// Returns the number of channels of this volume.
    virtual int getNumChannels() const = 0;

    /// Set the volume's bit depth.
    virtual void setBitsStored(int bits);

    /// Returns the volume's bit depth.
    virtual int getBitsStored() const;

    /// Returns the number of bits that are allocated by each voxel.
    /// For technical reasons, it may exceed the volume's bit depth.
    virtual int getBitsAllocated() const = 0;

    /// Returns the number of bytes that are allocated for each voxel.
    virtual int getBytesPerVoxel() const = 0;

    /**
     * Assigns a transformation matrix to the volume.
     *
     * This matrix has no effect on the Volume data itself,
     * but is applied by the Processors during the rendering process.
     * It can, for instance, be used for the registration of volumes.
     */
    virtual void setTransformation(const tgt::mat4& transformationMatrix);

    /**
     * Returns the transformation matrix assigned to this volume.
     *
     * \sa setTransformation
     */
    virtual const tgt::mat4& getTransformation() const;

    /**
     * Returns the matrix mapping from voxel coordinates (i.e. [0; dim-1])
     * to world coordinates.
     *
     * @note The matrix is internally created on each call.
     */
    virtual tgt::mat4 getVoxelToWorldMatrix() const;

    /**
     * @deprecated meta data will be moved to volume handle
     */
    VolumeMetaData& meta();

    /**
     * @deprecated meta data will be moved to volume handle
     */
    const VolumeMetaData& meta() const;

    /**
     * Returns the data set's minimal and maximal possible element values
     * according to its data type converted to float.
     */
    virtual tgt::vec2 elementRange() const = 0;

    /**
     * Returns the voxel of a given postion and channel, converted to a float.
     *
     * BEWARE: Since this method is virtual it can be considered as slow.
     *
     * @param pos the position of the voxel
     * @param channel the channel of the voxel
     */
    virtual float getVoxelFloat(const tgt::ivec3& pos, size_t channel = 0) const = 0;

    /**
     * Returns the voxel of a given postion and channel, converted to a float.
     *
     * BEWARE: Since this method is virtual it can be considered as slow.
     *
     * @param x the x-coordinate of the voxel
     * @param y the y-coordinate of the voxel
     * @param z the z-coordinate of the voxel
     * @param channel the channel of the voxel
     */
    virtual float getVoxelFloat(size_t x, size_t y, size_t z, size_t channel = 0) const = 0;

    /**
     * Returns the voxel of a given position and channel, converted to a float.
     *
     * BEWARE: Since this method is virtual it can be considered as slow.
     *
     * @param index the index of the voxel
     * @param channel the channel of the voxel
     */
    virtual float getVoxelFloat(size_t index, size_t channel = 0) const = 0;

    /**
     * Sets the voxel of a given postion and channel, converted from a float.
     *
     * BEWARE: Since this method is virtual it can be considered as slow.
     *
     * @param value The float value to be set.
     * @param pos the position of the voxel
     * @param channel the channel of the voxel
     */
    virtual void setVoxelFloat(float value, const tgt::ivec3& pos, size_t channel = 0) = 0;

    /**
     * Sets the voxel of a given postion and channel, converted from a float.
     *
     * BEWARE: Since this method is virtual it can be considered as slow.
     *
     * @param value The float value to be set.
     * @param x the x-coordinate of the voxel
     * @param y the y-coordinate of the voxel
     * @param z the z-coordinate of the voxel
     * @param channel the channel of the voxel
     */
    virtual void setVoxelFloat(float value, size_t x, size_t y, size_t z, size_t channel = 0) = 0;

    /**
     * Sets the voxel of a given postion and channel, converted from a float.
     *
     * BEWARE: Since this method is virtual it can be considered as slow.
     *
     * @param value The float value to be set.
     * @param index the index of the voxel
     * @param channel the channel of the voxel
     */
    virtual void setVoxelFloat(float value, size_t index, size_t channel = 0) = 0;

    /**
     * Returns the trilinearly interpolated voxel of a given
     * postion and channel, converted to a float.
     *
     * BEWARE: Since this method is virtual it can be considered as slow.
     *
     * @param pos the position of the voxel
     * @param channel the channel of the voxel
     */
    virtual float getVoxelFloatLinear(const tgt::vec3& pos, size_t channel = 0) const;

    /// Set all volume data to zero
    virtual void clear() = 0;

    /// Gets a void* to the data stored with this Volume
    virtual void* getData() = 0;

    virtual Volume* createSubset(tgt::ivec3 pos, tgt::ivec3 size) const
        throw (std::bad_alloc) = 0;

    /**
     * Resizes the volume to the specified dimensions by keeping its
     * remaining properties.
     *
     * @note The volume data is is cleared by this operation.
     *
     * @param newDims the volume's new dimensions
     * @param allocMem if true, a new data buffer is allocated
     *
     */
    virtual void resize(tgt::ivec3 newDims, bool allocMem = true)
        throw (std::bad_alloc) = 0;

    /**
     * Returns a volume that has been resampled to the specified dimensions
     * by using the given filtering mode.
     *
     * @return the resampled volume
     */
    virtual Volume* resample(tgt::ivec3 newDims, Filter filter) const
        throw (std::bad_alloc) = 0;

    /**
     * Reduces the Volumes resolution by half, by linearly downsampling 8 voxels
     * to 1 voxel. This does not necessarily happen when using the resample(..) function.
     *
     * @return the resampled volume
     */
    virtual Volume* halfsample() const
        throw (std::bad_alloc);

    /**
     * Use this method in order to copy over the data from \p v to this Volume
     * while converting the data to this volume's voxel type.
     *
     * @param v The source volume. Must match this volume in dimension and channel count!
     */
    void convert(const Volume* v);

    /**
     * Use this as type safe wrapper in order to get a proper typed pointer.
     */
    template<class T>
    inline static typename T::VoxelType* getData(T* v);

    //-------------------------------------------------------------------
protected:
    // protected default constructor
    Volume() {}
    void calculateProperties();

    tgt::ivec3  dimensions_;
    size_t      numVoxels_;
    int         bitsStored_;
    tgt::vec3   spacing_;
    tgt::vec3   llf_;
    tgt::vec3   urb_;
    tgt::vec3   cubeSize_;
    tgt::vec3   cubeVertices_[8];

    tgt::mat4 transformationMatrix_;

    VolumeMetaData meta_;

    static const std::string loggerCat_;
};


/**
 * You can use this macro if you want to iterate over all voxels and it is
 * important for you that you do it dimensionwise, for instance: <br>
\code
VRN_FOR_EACH_VOXEL(i, ivec3(0, 0, 0), ivec(15, 20, 30))
    voxel(i) = i.x * i.y + i.z;
\endcode
 * or like this:
\code
VRN_FOR_EACH_VOXEL(i, ivec3(0, 0, 0), ivec(15, 20, 30)) {
    voxel(i) = i.x * i.y + i.z;
    foo();
}
\endcode
 * If you want to do sth just with each voxel use this simple for-loop
\code
for (size_t i = 0; i \< numVoxels_; ++i)
     voxel(i) = i;
\endcode
*/
#define VRN_FOR_EACH_VOXEL(INDEX, POS, SIZE) \
    for (tgt::ivec3 (INDEX) = (POS); (INDEX).z < (SIZE).z; ++(INDEX).z)\
        for ((INDEX).y = (POS).y; (INDEX).y < (SIZE).y; ++(INDEX).y)\
            for ((INDEX).x = (POS).x; (INDEX).x < (SIZE).x; ++(INDEX).x)

} // namespace voreen

#endif // VRN_VOLUME_H
