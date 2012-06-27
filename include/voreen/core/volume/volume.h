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

#ifndef VRN_VOLUME_H
#define VRN_VOLUME_H

// Note: please ensure that no OpenGL dependencies are added into this file

#include "voreen/core/vis/message.h"
#include "voreen/core/volume/volumemetadata.h"

namespace voreen {

class Volume {
public:

    /*
     * constructors and destructor
     */

    /**
     * @param dimensions dimesions of the new dataset
     * @param bitsStored stored bits of the new dataset
     * @param spacing spacing of the new dataset
    */
    Volume(const tgt::ivec3& dimensions,
           int bitsStored,
           const tgt::vec3& spacing = tgt::vec3(1.f));

    virtual ~Volume() {}

    /// Use this as a kind of a virtual constructor
    virtual Volume* clone() const throw (std::bad_alloc) = 0;

    /**
     * Use this as a kind of a virtual constructor that does _NOT_ copy over
     * the voxels but uses the given pointer instead as a voxel data. If this
     * pointer is 0 simply an chunk of data will be allocated that will not be
     * filled with any stuff.
     */
    virtual Volume* clone(void* data) const throw (std::bad_alloc) = 0;

    /*
     * getters and setters
     */

    tgt::ivec3 getDimensions() const;

    /// Returns the lower left front mapped to [-1, 1] with spacing kept in mind
    tgt::vec3 getLLF() const;

    /// Returns the upper right backmapped to [-1, 1] with spacing kept in mind
    tgt::vec3 getURB() const;

    /// Returns the cube vertices mapped to [-1, 1] with spacing kept in mind
    const tgt::vec3* getCubeVertices() const;

    /// Returns the size of the cube mapped to [-1, 1] with spacing kept in mind
    tgt::vec3 getCubeSize() const;

    size_t getNumVoxels() const;
    tgt::vec3 getSpacing() const;

    virtual int getBitsStored() const;

    virtual int getBitsAllocated() const = 0;
    virtual int getNumChannels() const = 0;
    virtual int getBytesPerVoxel() const = 0;

    /// Returns the number of bytes held in the \a data_ array
    virtual size_t getNumBytes() const = 0;

    VolumeMetaData& meta();
    const VolumeMetaData& meta() const;

    void setSpacing(const tgt::vec3 spacing);
    void setBitsStored(int bitsStored);

    /*
     * abstract access of the voxels
     */

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
     * Returns the trilinearly interpolated voxel of a given
     * postion and channel, converted to a float.
     *
     * BEWARE: Since this method is virtual it can be considered as slow.
     *
     * @param pos the position of the voxel
     * @param channel the channel of the voxel
     */
    float getVoxelFloatLinear(const tgt::vec3& pos, size_t channel = 0) const;

    /*
     * methods
     */

    /// Set all volume data to zero
    virtual void clear() = 0;

    /// Gets a void* to the data stored with this Volume
    virtual void* getData() = 0;
    virtual Volume* mirrorZ() const = 0;
    virtual Volume* createSubset(const tgt::ivec3& pos, const tgt::ivec3& size) const
        throw (std::bad_alloc) = 0;

    enum Filter {
        NEAREST,
        LINEAR
    };

    virtual Volume* scale(const tgt::ivec3& newDims, Filter filter) const
        throw (std::bad_alloc) = 0;

    /**
    * Reduces the Volumes resolution by half, by linearly downsampling 8 voxels
    * to 1 voxel. This does not necessarily happen when using the scale(..) function.
    */
    virtual Volume* downsample() const throw (std::bad_alloc);

    /**Calculates the root mean square error between this volume and
    * volume
    */
    virtual float calcError(Volume* volume) =0;

    /**
     * Use this method in order to copy over the data from \p v to this Volume
     * while converting the data to this Volume's VoxelType.
     *
     * @param v The source Volume.
     * @param smartConvert If true, some heuristcs are used in order to make a
     *      better output, if this type constellation is supported (currently not implemented).
     */
    void convert(const Volume* v, bool smartConvert = false);

    /**
     * Use this as type safe wrapper in order to get a proper typed pointer.
     */
    template<class T>
    inline static typename T::VoxelType* getData(T* v) {
        return (typename T::VoxelType*) v->getData();
    }

	/**
	* Use this to check if all voxels in the volume have the same value.
	*/
	virtual bool getAllVoxelsEqual() = 0;

protected:
    // protected default constructor
    Volume() {}

    tgt::ivec3  dimensions_;
    size_t      numVoxels_;
    int         bitsStored_;
    tgt::vec3   spacing_;
    tgt::vec3   llf_;
    tgt::vec3   urb_;
    tgt::vec3   cubeSize_;
    tgt::vec3   cubeVertices_[8];

    VolumeMetaData meta_;

    static const std::string loggerCat_;
};

typedef TemplateMessage<Volume*> VolumePtrMsg;

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
