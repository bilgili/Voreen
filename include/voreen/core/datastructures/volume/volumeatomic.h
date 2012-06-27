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

#ifndef VRN_VOLUMEATOMIC_H
#define VRN_VOLUMEATOMIC_H

#include <algorithm>
#include <cstring>
#include <fstream>

#include "tgt/assert.h"
#include "tgt/logmanager.h"

#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumeelement.h"

#include <typeinfo>

namespace voreen {

template<class T>
class VolumeAtomic : public Volume {
public:
    typedef T VoxelType;

    enum {
        BYTES_PER_VOXEL = sizeof(T),
        BITS_PER_VOXEL  = BYTES_PER_VOXEL * 8
    };

    /*
     * constructors and destructor
     */

    /**
     * While using this constructor the class will automatically allocate
     * an appropriate chunk of memory. This memory will be deleted by this class.
     * If allocMem is false, no memory will be allocated. This can be used to create
     * volumes without any data, in case you just want to store its dimensions, spacing etc.
     * This is used for bricking for example, because the entry-exit point volume would
     * otherwise allocate an enormous amount of memory, although only its dimensions are
     * required.
     */
    VolumeAtomic(const tgt::ivec3& dimensions,
                 const tgt::vec3& spacing = tgt::vec3(1.f),
                 int bitsStored = BITS_PER_VOXEL, bool allocMem=true) throw (std::bad_alloc);

    /**
     * While using this constructor the class will use an preallocated chunk
     * of memory given in \p data. This memory will be deleted by this class.
     */
    VolumeAtomic(T* data,
                 const tgt::ivec3& dimensions,
                 const tgt::vec3& spacing = tgt::vec3(1.f),
                 int bitsStored = BITS_PER_VOXEL);

    /// Deletes the \a data_ array
    virtual ~VolumeAtomic();

    virtual VolumeAtomic<T>* clone() const throw (std::bad_alloc);
    virtual VolumeAtomic<T>* clone(void* data) const throw (std::bad_alloc);

    /*
     * Static properties
     */

    virtual int getBitsAllocated() const;

    virtual int getNumChannels() const;

    virtual int getBytesPerVoxel() const;

    /**
     * Returns whether the volume's data type is signed.
     */
    static bool isSigned();

    /**
     * Returns the minimal and maximal element data values that can be contained
     * by the volume according to its data type.
     *
     * @note for unbounded types, (0.0, 1.0) is returned
     */
    virtual tgt::vec2 elementRange() const;

    /**
     * Returns whether the volume's data type is bounded,
     * which is true for integer types and false for
     * floating point types.
     */
    static bool isBounded();

    /**
     * Returns the minimum data value that is actually contained by the volume.
     *
     * @note Calculated min/max values are cached. Call invalidate() in order to
     *      enforce a recalculation.
     */
    T min() const;

    /**
     * Returns the maximal data value that is actually contained by the volume.
     *
     * @note Calculated min/max values are cached. Call invalidate() in order to
     *      enforce a recalculation.
     */
    T max() const;

    virtual size_t getNumBytes() const;

    /*
     * Helpers for calculating the position in 3d
     */
    inline static size_t calcPos(const tgt::ivec3& dimensions, size_t x, size_t y, size_t z);

    inline static size_t calcPos(const tgt::ivec3& dimensions, const tgt::ivec3& pos);

    /*
      Methods for accessing the voxels
    */

    /// just get a proper pointer for read and write access
    inline T* voxel();

    /// just get a proper pointer for read access only
    inline const T* voxel() const;

    /// get or set voxel
    inline T& voxel(size_t x, size_t y, size_t z);

    /// get voxel
    inline const T& voxel(size_t x, size_t y, size_t z) const;

    /// get or set voxel
    inline T& voxel(const tgt::ivec3& pos);

    /// get voxel
    inline const T& voxel(const tgt::ivec3& pos) const;

    /// get or set voxel
    inline T& voxel(size_t i);

    /// get voxel
    inline const T& voxel(size_t i) const;


    /*
     * getVoxelFloat and setVoxelFloat
     */
    virtual float getVoxelFloat(const tgt::ivec3& pos, size_t channel = 0) const;
    virtual float getVoxelFloat(size_t x, size_t y, size_t z, size_t channel = 0) const;
    virtual float getVoxelFloat(size_t index, size_t channel = 0) const;
    virtual void setVoxelFloat(float value, const tgt::ivec3& pos, size_t channel = 0);
    virtual void setVoxelFloat(float value, size_t x, size_t y, size_t z, size_t channel = 0);
    virtual void setVoxelFloat(float value, size_t index, size_t channel = 0);

    virtual void clear();
    virtual void* getData();
    virtual void resize(tgt::ivec3 newDims, bool allocMem = true)
        throw (std::bad_alloc);
    virtual VolumeAtomic<T>* resample(tgt::ivec3 newDims, Filter filter) const
        throw (std::bad_alloc);
    virtual VolumeAtomic<T>* createSubset(tgt::ivec3 pos, tgt::ivec3 size) const
        throw (std::bad_alloc);

    /**
    * Reduces the Volume's resolution by half, by linearly downsampling 8 voxels
    * to 1 voxel. This does not necessarily happen when using the scale(..) function.
    */
    virtual VolumeAtomic<T>* halfsample() const throw (std::bad_alloc);

    /**
     * Invalidates cached values (e.g. min/max), should be called when the volume was modified.
     */
    virtual void invalidate() const;

    //-------------------------------------------------------------------
protected:
    // protected default constructor
    VolumeAtomic() {}

    T* data_;

    tgt::vec2 elementRange_;

    mutable T maxValue_;
    mutable T minValue_;
    mutable bool minMaxValid_;
};

/*
 * typedefs for easy usage
 */

typedef VolumeAtomic<uint8_t>   VolumeUInt8;
typedef VolumeAtomic<uint16_t>  VolumeUInt16;
typedef VolumeAtomic<uint32_t>  VolumeUInt32;

typedef VolumeAtomic<int8_t>    VolumeInt8;
typedef VolumeAtomic<int16_t>   VolumeInt16;
typedef VolumeAtomic<int32_t>   VolumeInt32;

typedef VolumeAtomic<float>     VolumeFloat;
typedef VolumeAtomic<double>    VolumeDouble;

typedef VolumeAtomic<tgt::col4>                 Volume4xUInt8;
typedef VolumeAtomic<tgt::Vector4<int8_t> >     Volume4xInt8;
typedef VolumeAtomic<tgt::Vector4<uint16_t> >   Volume4xUInt16;
typedef VolumeAtomic<tgt::Vector4<int16_t> >    Volume4xInt16;

typedef VolumeAtomic<tgt::col3>                 Volume3xUInt8;
typedef VolumeAtomic<tgt::Vector3<int8_t> >     Volume3xInt8;
typedef VolumeAtomic<tgt::Vector3<uint16_t> >   Volume3xUInt16;
typedef VolumeAtomic<tgt::Vector3<int16_t> >    Volume3xInt16;

typedef VolumeAtomic<tgt::Vector2<uint8_t> >    Volume2xUInt8;
typedef VolumeAtomic<tgt::Vector2< int8_t> >    Volume2xInt8;
typedef VolumeAtomic<tgt::Vector2<uint16_t> >   Volume2xUInt16;
typedef VolumeAtomic<tgt::Vector2< int16_t> >   Volume2xInt16;

typedef VolumeAtomic<tgt::vec3>  Volume3xFloat;
typedef VolumeAtomic<tgt::dvec3> Volume3xDouble;

typedef VolumeAtomic<tgt::vec4>  Volume4xFloat;
typedef VolumeAtomic<tgt::dvec4> Volume4xDouble;

//------------------------------------------------------------------------------
//  implementation
//------------------------------------------------------------------------------

/*
 * constructors and destructor
 */

template<class T>
VolumeAtomic<T>::VolumeAtomic(const tgt::ivec3& dimensions, const tgt::vec3& spacing,
                              int bitsStored, bool allocMem)
    throw (std::bad_alloc)
    : Volume(dimensions, bitsStored, spacing),
      data_(0),
      elementRange_(static_cast<float>(VolumeElement<T>::rangeMinElement()),
        static_cast<float>(VolumeElement<T>::rangeMaxElement())),
      minMaxValid_(false)
{

    // special treatment for 12 bit volumes stored in 16 bit
    if (typeid(T) == typeid(uint16_t) && bitsStored == 12)
        elementRange_.y = static_cast<float>((1 << 12) - 1);

    if (allocMem) {
        try {
            data_ = new T[numVoxels_];
        }
        catch (std::bad_alloc) {
            LERROR("Failed to allocate memory: bad allocation");
            throw; // throw it to the caller
        }
    }
}

template<class T>
VolumeAtomic<T>::VolumeAtomic(T* data,
                              const tgt::ivec3& dimensions,
                              const tgt::vec3& spacing,
                              int bitsStored)
    : Volume(dimensions, bitsStored, spacing),
      data_(data),
      elementRange_(static_cast<float>(VolumeElement<T>::rangeMinElement()),
      static_cast<float>(VolumeElement<T>::rangeMaxElement())),
      minMaxValid_(false)
{
    // special treatment for 12 bit volumes stored in 16 bit
    if (typeid(T) == typeid(uint16_t) && bitsStored == 12)
        elementRange_.y = static_cast<float>((1 << 12) - 1);
}

template<class T>
VolumeAtomic<T>* VolumeAtomic<T>::clone() const
    throw (std::bad_alloc)
{
    // create clone
    VolumeAtomic<T>* newVolume = 0;
    try {
        newVolume = new VolumeAtomic<T>(dimensions_, spacing_, bitsStored_); // allocate a chunk of data
    }
    catch (const std::bad_alloc&) {
        LERROR("Failed to clone volume: bad allocation");
        throw; // throw it to the caller
    }

    // copy over the voxel data
    memcpy(newVolume->data_, data_, getNumBytes());

    // copy over transformation matrix
    newVolume->setTransformation(getTransformation());

    // copy over meta data
    newVolume->meta() = meta();

    return newVolume;
}

template<class T>
VolumeAtomic<T>* VolumeAtomic<T>::clone(void* data) const
    throw (std::bad_alloc)
{
    // create clone
    VolumeAtomic<T>* newVolume = 0;
    if (data) {
        // use preallocated data
        newVolume = new VolumeAtomic<T>((T*) data, dimensions_, spacing_, bitsStored_);
    }
    else {
        // create volume without allocating memory
        newVolume = new VolumeAtomic<T>(dimensions_, spacing_, bitsStored_, false);
    }

    // copy over transformation matrix
    newVolume->setTransformation(getTransformation());

    // copy over meta data
    newVolume->meta() = meta();

    return newVolume;
}

template<class T>
VolumeAtomic<T>::~VolumeAtomic() {
    delete[] data_;
}


template<class T>
int VolumeAtomic<T>::getNumChannels() const {
    return VolumeElement<T>::getNumChannels();
}

template<class T>
bool VolumeAtomic<T>::isSigned() {
    return VolumeElement<T>::isSigned();
}

template<class T>
bool voreen::VolumeAtomic<T>::isBounded() {
    return VolumeElement<T>::isBounded();
}

/*
 * getVoxelFloat and setVoxelFloat
 */

template<class T>
float VolumeAtomic<T>::getVoxelFloat(const tgt::ivec3& pos, size_t channel) const {
    return getVoxelFloat(calcPos(dimensions_, pos), channel);
}

template<class T>
float VolumeAtomic<T>::getVoxelFloat(size_t x, size_t y, size_t z, size_t channel) const {
    return getVoxelFloat(tgt::ivec3(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)), channel);
}

template<class T>
float voreen::VolumeAtomic<T>::getVoxelFloat(size_t index, size_t channel) const {

    typedef typename VolumeElement<T>::BaseType Base;
    tgt::vec2 elemRange = elementRange();

    Base value = VolumeElement<T>::getChannel(voxel(index), channel);
    return (static_cast<float>(value) - elemRange.x) / (elemRange.y - elemRange.x);
}

template<class T>
void VolumeAtomic<T>::setVoxelFloat(float value, const tgt::ivec3& pos, size_t channel) {

    setVoxelFloat(value, calcPos(dimensions_, pos), channel);
}

template<class T>
void VolumeAtomic<T>::setVoxelFloat(float value, size_t x, size_t y, size_t z, size_t channel) {
    setVoxelFloat(value, tgt::ivec3(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)), channel);
}

template<class T>
void voreen::VolumeAtomic<T>::setVoxelFloat(float value, size_t index, size_t channel) {

    typedef typename VolumeElement<T>::BaseType Base;
    tgt::vec2 elemRange = elementRange();

    VolumeElement<T>::setChannel(
        static_cast<Base>(elemRange.x + value*(elemRange.y-elemRange.x)),
        voxel(index), channel);
}


/*
 * getters and setters
 */

template<class T>
int VolumeAtomic<T>::getBitsAllocated() const {
    return BITS_PER_VOXEL;
}

template<class T>
int VolumeAtomic<T>::getBytesPerVoxel() const {
    return BYTES_PER_VOXEL;
}

template<class T>
size_t VolumeAtomic<T>::getNumBytes() const {
    return sizeof(T) * numVoxels_;
}

/*
 * Helpers for calculating the position in 3d
 */

template<class T>
inline size_t VolumeAtomic<T>::calcPos(const tgt::ivec3& dimensions, size_t x, size_t y, size_t z) {
    //we need to work with size_t to avoid problems when working with very large datasets
    return z*(size_t)dimensions.x*(size_t)dimensions.y + y*(size_t)dimensions.x + x;
}

template<class T>
inline size_t VolumeAtomic<T>::calcPos(const tgt::ivec3& dimensions, const tgt::ivec3& pos) {
    //we need to work with size_t to avoid problems when working with very large datasets
    return (size_t)pos.z*(size_t)dimensions.x*(size_t)dimensions.y + (size_t)pos.y*(size_t)dimensions.x + (size_t)pos.x;
}

/*
 * further methods
 */

template<class T>
tgt::vec2 VolumeAtomic<T>::elementRange() const {
    return elementRange_;
}

template<class T>
T VolumeAtomic<T>::min() const {
    if (!minMaxValid_) {
        minValue_ = *std::min_element(data_, data_ + getNumVoxels());
        maxValue_ = *std::max_element(data_, data_ + getNumVoxels());
        minMaxValid_ = true;
    }

    return minValue_;

}

template<class T>
T VolumeAtomic<T>::max() const {
    if (!minMaxValid_) {
        minValue_ = *std::min_element(data_, data_ + getNumVoxels());
        maxValue_ = *std::max_element(data_, data_ + getNumVoxels());
        minMaxValid_ = true;
    }

    return maxValue_;
}

template<class T>
void VolumeAtomic<T>::clear() {
    memset(data_, 0, getNumBytes());
    invalidate();
}

template<class T>
void VolumeAtomic<T>::invalidate() const {
    minMaxValid_ = false;
}

template<class T>
void* VolumeAtomic<T>::getData() {
    return reinterpret_cast<void*>(data_);
}

template<class T>
void voreen::VolumeAtomic<T>::resize(tgt::ivec3 newDims, bool allocMem)
    throw (std::bad_alloc)
{
    if (newDims == getDimensions() && (data_ || !allocMem))
        return;

    LDEBUG("Resizing from dimensions " << dimensions_ << " to " << newDims);

    delete[] data_;
    data_ = 0;

    dimensions_ = newDims;
    calculateProperties();

    if (allocMem) {
        try {
            data_ = new T[numVoxels_];
        }
        catch (std::bad_alloc) {
            LERROR("Failed to resize volume: bad allocation");
            throw; // throw it to the caller
        }
    }
}

template<class T>
VolumeAtomic<T>* VolumeAtomic<T>::resample(tgt::ivec3 newDims, Filter filter) const
    throw (std::bad_alloc)
{
    using tgt::vec3;
    using tgt::ivec3;

    LINFO("Resampling from dimensions " << dimensions_ << " to " << newDims);

    vec3 ratio = vec3(dimensions_) / vec3(newDims);
    vec3 invDims = 1.f / vec3(dimensions_);

    ivec3 pos = ivec3::zero; // iteration variable
    vec3 nearest; // knows the new position of the target volume

    // build target volume
    VolumeAtomic<T>* v;
    try {
         v = new VolumeAtomic(newDims, spacing_*ratio, bitsStored_);
    }
    catch (std::bad_alloc) {
        throw; // throw it to the caller
    }

    v->setTransformation(getTransformation());
    v->meta() = meta();

    /*
        Filter from the source volume to the target volume.
    */
    switch (filter) {
    case NEAREST:
        for (pos.z = 0; pos.z < newDims.z; ++pos.z) {
            nearest.z = static_cast<float>(pos.z) * ratio.z;

            for (pos.y = 0; pos.y < newDims.y; ++pos.y) {
                nearest.y = static_cast<float>(pos.y) * ratio.y;

                for (pos.x = 0; pos.x < newDims.x; ++pos.x) {
                    nearest.x = static_cast<float>(pos.x) * ratio.x;

                    ivec3 index = tgt::clamp(ivec3(nearest + 0.5f), ivec3(0), dimensions_ - 1);
                    v->voxel(pos) = voxel(index); // round and do the lookup
                }
            }
        }
        break;

    case LINEAR:
        for (pos.z = 0; pos.z < newDims.z; ++pos.z) {
            nearest.z = static_cast<float>(pos.z) * ratio.z;

            for (pos.y = 0; pos.y < newDims.y; ++pos.y) {
                nearest.y = static_cast<float>(pos.y) * ratio.y;

                for (pos.x = 0; pos.x < newDims.x; ++pos.x) {
                    nearest.x = static_cast<float>(pos.x) * ratio.x;
                    vec3 p = nearest - floor(nearest); // get decimal part
                    ivec3 llb = ivec3(nearest);
                    ivec3 urf = ivec3(ceil(nearest));
                    urf = tgt::min(urf, dimensions_ - 1); // clamp so the lookups do not exceed the dimensions

                    /*
                      interpolate linearly
                    */
                    typedef typename VolumeElement<T>::DoubleType Double;
                    v->voxel(pos) =
                        T(  Double(voxel(llb.x, llb.y, llb.z)) * static_cast<double>((1.f-p.x)*(1.f-p.y)*(1.f-p.z))  // llB
                          + Double(voxel(urf.x, llb.y, llb.z)) * static_cast<double>((    p.x)*(1.f-p.y)*(1.f-p.z))  // lrB
                          + Double(voxel(urf.x, urf.y, llb.z)) * static_cast<double>((    p.x)*(    p.y)*(1.f-p.z))  // urB
                          + Double(voxel(llb.x, urf.y, llb.z)) * static_cast<double>((1.f-p.x)*(    p.y)*(1.f-p.z))  // ulB
                          + Double(voxel(llb.x, llb.y, urf.z)) * static_cast<double>((1.f-p.x)*(1.f-p.y)*(    p.z))  // llF
                          + Double(voxel(urf.x, llb.y, urf.z)) * static_cast<double>((    p.x)*(1.f-p.y)*(    p.z))  // lrF
                          + Double(voxel(urf.x, urf.y, urf.z)) * static_cast<double>((    p.x)*(    p.y)*(    p.z))  // urF
                          + Double(voxel(llb.x, urf.y, urf.z)) * static_cast<double>((1.f-p.x)*(    p.y)*(    p.z)));// ulF
                }
            }
        }
        break;
    }

    return v;
}

template<class T>
VolumeAtomic<T>* VolumeAtomic<T>::createSubset(tgt::ivec3 pos, tgt::ivec3 size) const
    throw (std::bad_alloc)
{

    VolumeAtomic<T>* subset;
    try {
        subset = new VolumeAtomic<T>(size, spacing_, bitsStored_);
    }
    catch (std::bad_alloc) {
        LERROR("Failed to create subset: bad allocation");
        throw; // throw it to the caller
    }

    subset->meta() = meta();

    // calculate new imageposition
    if (pos.z != 0.f)
        subset->meta().setImagePositionZ(meta().getImagePositionZ() - spacing_.z * pos.z);

    LINFO("Creating subset " << size << " from position " << pos);

    // create values for ranges less than zero and greater equal dimensions_
    subset->clear(); // TODO: This can be optomized by avoiding to clear the values in range

    // now the rest
    tgt::ivec3 start = tgt::max(pos, tgt::ivec3::zero);// clamp values
    tgt::ivec3 end   = tgt::min(pos + size, dimensions_);    // clamp values

    VRN_FOR_EACH_VOXEL(i, tgt::ivec3(0), end - start)
        subset->voxel(i) = voxel(i+start);

    return subset;
}

/**
* Calculates to which position the given position relates in a smaller volume. This
* is used in the calcError function to determine to which position in the smaller volume
* a voxel position corresponds, in order to compute the difference between the voxels.
*/
inline static tgt::ivec3 calcPosInSmallerVolume(tgt::ivec3 pos, tgt::ivec3 factor) {
    tgt::ivec3 result;
    result.x = static_cast<int>( floor(pos.x / (float)factor.x));
    result.y = static_cast<int>( floor(pos.y / (float)factor.y));
    result.z = static_cast<int>( floor(pos.z / (float)factor.z));
    return result;
}

template<class T>
VolumeAtomic<T>* VolumeAtomic<T>::halfsample() const
    throw (std::bad_alloc) {
    tgt::ivec3 dims = dimensions_;
    tgt::ivec3 halfDims = dimensions_ / 2;

    VolumeAtomic<T>* newVolume = new VolumeAtomic<T>(halfDims,spacing_*2.f,bitsStored_);

    for (int z=0; z<halfDims.z; z++) {
        for (int y=0; y<halfDims.y; y++) {
            for (int x=0; x<halfDims.x; x++) {
                tgt::ivec3 pos = tgt::ivec3(2*x,2*y,2*z);
                typedef typename VolumeElement<T>::DoubleType Double;
                newVolume->voxel(x,y,z) =
                        T(  Double(voxel(pos.x, pos.y, pos.z))          * (1.0/8.0) //LLF
                          + Double(voxel(pos.x, pos.y, pos.z+1))        * (1.0/8.0) //LLB
                          + Double(voxel(pos.x, pos.y+1, pos.z))        * (1.0/8.0) //ULF
                          + Double(voxel(pos.x, pos.y+1, pos.z+1))      * (1.0/8.0) //ULB
                          + Double(voxel(pos.x+1, pos.y, pos.z))        * (1.0/8.0) //LRF
                          + Double(voxel(pos.x+1, pos.y, pos.z+1))      * (1.0/8.0) //LRB
                          + Double(voxel(pos.x+1, pos.y+1, pos.z))      * (1.0/8.0) //URF
                          + Double(voxel(pos.x+1, pos.y+1, pos.z+1))    * (1.0/8.0)); //URB

            }
        }
    }

    return newVolume;
}

/*
  Methods for accessing the voxels
*/

/// just get a proper pointer for read and write access
template<class T>
inline T* VolumeAtomic<T>::voxel() {
    return data_;
}

/// just get a proper pointer for read access only
template<class T>
inline const T* VolumeAtomic<T>::voxel() const {
    return data_;
}

/// get or set voxel
template<class T>
inline T& VolumeAtomic<T>::voxel(size_t x, size_t y, size_t z) {
    tgtAssert(x < size_t(dimensions_.x), "x index out of bounds");
    tgtAssert(y < size_t(dimensions_.y), "y index out of bounds");
    tgtAssert(z < size_t(dimensions_.z), "z index out of bounds");

    return data_[calcPos(dimensions_, x, y, z)];
}

/// get voxel
template<class T>
inline const T& VolumeAtomic<T>::voxel(size_t x, size_t y, size_t z) const {
    tgtAssert(x < size_t(dimensions_.x), "x index out of bounds");
    tgtAssert(y < size_t(dimensions_.y), "y index out of bounds");
    tgtAssert(z < size_t(dimensions_.z), "z index out of bounds");

    return data_[calcPos(dimensions_, x, y, z)];
}

/// get or set voxel
template<class T>
inline T& VolumeAtomic<T>::voxel(const tgt::ivec3& pos) {
    tgtAssert(pos.x < dimensions_.x, "x index out of bounds");
    tgtAssert(pos.y < dimensions_.y, "y index out of bounds");
    tgtAssert(pos.z < dimensions_.z, "z index out of bounds");
    tgtAssert(pos.x >= 0, "x index out of bounds");
    tgtAssert(pos.y >= 0, "y index out of bounds");
    tgtAssert(pos.z >= 0, "z index out of bounds");

    return data_[calcPos(dimensions_, pos)];
}

/// get voxel
template<class T>
inline const T& VolumeAtomic<T>::voxel(const tgt::ivec3& pos) const {
    tgtAssert(pos.x < dimensions_.x, "x index out of bounds");
    tgtAssert(pos.y < dimensions_.y, "y index out of bounds");
    tgtAssert(pos.z < dimensions_.z, "z index out of bounds");
    tgtAssert(pos.x >= 0, "x index out of bounds");
    tgtAssert(pos.y >= 0, "y index out of bounds");
    tgtAssert(pos.z >= 0, "z index out of bounds");

    return data_[calcPos(dimensions_, pos)];
}

/// get or set voxel
template<class T>
inline T& VolumeAtomic<T>::voxel(size_t i) {
    tgtAssert(i < numVoxels_, "index out of bounds");

    return data_[i];
}

/// get voxel
template<class T>
inline const T& VolumeAtomic<T>::voxel(size_t i) const {
    tgtAssert(i < numVoxels_, "index out of bounds");

    return data_[i];
}


} // namespace voreen

#endif // VRN_VOLUMEATOMIC_H
