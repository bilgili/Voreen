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

#ifndef VRN_VOLUMEOPERATOR_H
#define VRN_VOLUMEOPERATOR_H

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/io/progressbar.h"
#include "voreen/core/utils/exception.h"

#ifdef VRN_MODULE_FLOWREEN
#include "voreen/modules/flowreen/volumeflow3d.h"
#endif

#include <vector>
#include <limits>

namespace voreen {

/**
 * Thrown by a VolumeOperator is the type of the passed Volume is not supported.
 */
class VolumeOperatorUnsupportedTypeException : public VoreenException {
public:
    VolumeOperatorUnsupportedTypeException(const std::string& type = "unknown")
        : VoreenException("Volume type '" + type + "'not supported by this VolumeOperator")
    {}
};

/**
 * Parent class of volume operators for one volume.
 */
template<class Derived>     // curiously recurring template pattern (CRTP) for static polymorphism
class VolumeOperatorUnary {
public:
    /**
     * Default constructor. Initializes the progressBar member with the null pointer.
     */
    VolumeOperatorUnary() :
      progressBar_(0) {}

    virtual ~VolumeOperatorUnary() = 0; // keeps the class abstract

    /**
     * Apply the operator on the given volume. The operator can modify the volume and also
     * return a value.
     */
    template<typename ReturnValue>
    ReturnValue apply(Volume* volume) const;

    /**
     * Assigns a progress bar that should be used by the
     * operator for indicating progress.
     */
    void setProgressBar(ProgressBar* progress) {
        progressBar_ = progress;
    }

protected:
    ProgressBar* progressBar_;  ///< to be used by concrete subclasses for indicating progress

};

/**
 * Parent class of volume operators for two volumes.
 * Both volumes must have the same type.
 */
template<class Derived>     // curiously recurring template pattern (CRTP) for static polymorphism
class VolumeOperatorBinary {
public:
    /**
     * Default constructor. Initializes the progressBar member with the null pointer.
     */
    VolumeOperatorBinary() :
      progressBar_(0) {}

    virtual ~VolumeOperatorBinary() = 0;

    /**
     * Apply the operator on the given volumes. The operator can modify the volumes and also
     * return a value.
     */
    template<typename ReturnValue>
    ReturnValue apply(Volume* v1, Volume* v2) const;

    /**
     * Assigns a progress bar that should be used by the
     * operator for indicating progress.
     */
    void setProgressBar(ProgressBar* progress) {
        progressBar_ = progress;
    }

protected:
    ProgressBar* progressBar_;  ///< to be used by concrete subclasses for indicating progress

};


/**
 * Variation of VRN_FOR_EACH_VOXEL that updates a progress bar
 * for each z-slice that has been processed. The PROGRESS parameter
 * may be null.
 */
#define VRN_FOR_EACH_VOXEL_WITH_PROGRESS(INDEX, POS, SIZE, PROGRESS) \
    for (tgt::ivec3 (INDEX) = (POS); (INDEX).z < (SIZE).z; ++(INDEX).z, PROGRESS ? PROGRESS->setProgress(static_cast<float>(INDEX.z) / static_cast<float>(SIZE.z)) : void(0)) \
        for ((INDEX).y = (POS).y; (INDEX).y < (SIZE).y; ++(INDEX).y)\
            for ((INDEX).x = (POS).x; (INDEX).x < (SIZE).x; ++(INDEX).x)

//
// Some common volume operators
//

/**
 * Returns a volume containing the subset [pos, pos+size[ of the passed input volume.
 */
class VolumeOperatorCreateSubset : public VolumeOperatorUnary<VolumeOperatorCreateSubset> {
    friend class VolumeOperatorUnary<VolumeOperatorCreateSubset>;
public:
    VolumeOperatorCreateSubset(tgt::ivec3 pos, tgt::ivec3 size);
    void setPos(tgt::ivec3 pos);
    void setSize(tgt::ivec3 size);
private:
    tgt::ivec3 pos_;
    tgt::ivec3 size_;

    template<typename T>
    VolumeAtomic<T>* apply_internal(VolumeAtomic<T>* inputVolume) const throw (std::bad_alloc);
};

/**
 * Returns a resized copy of the passed input volume by keeping its
 * remaining properties.
 *
 * @note The volume data is not copied by this operation.
 *
 * @see VolumeOperatorResample
 *
 * @return the resized volume
 */
class VolumeOperatorResize : public VolumeOperatorUnary<VolumeOperatorResize> {
    friend class VolumeOperatorUnary<VolumeOperatorResize>;
public:
    /**
     * @param newDims The target dimensions
     * @param allocMem If true, a new data buffer is allocated
     */
    VolumeOperatorResize(tgt::ivec3 newDims, bool allocMem = true);
    void setNewDims(tgt::ivec3 newDims);
    void setAllocMem(bool allocMem);
private:
    tgt::ivec3 newDims_;
    bool allocMem_;

    template<typename T>
    VolumeAtomic<T>* apply_internal(VolumeAtomic<T>* inputVolume) const throw (std::bad_alloc);
};

/**
 * Returns a copy of the input volume that has been resampled to the specified dimensions
 * by using the given filtering mode.
 *
 * @return the resampled volume
 */
class VolumeOperatorResample : public VolumeOperatorUnary<VolumeOperatorResample> {
    friend class VolumeOperatorUnary<VolumeOperatorResample>;
public:
    /**
     * @param newDims the target dimensions
     * @param filter The filtering mode to use for calculating the resampled values.
     */
    VolumeOperatorResample(tgt::ivec3 newDims, Volume::Filter filter);
    void setNewDims(tgt::ivec3 newDims);
    void setFilter(Volume::Filter filter);
private:
    tgt::ivec3 newDims_;
    Volume::Filter filter_;

    template<typename T>
    VolumeAtomic<T>* apply_internal(VolumeAtomic<T>* inputVolume) const throw (std::bad_alloc);
};

/**
 * Reduces the Volumes resolution by half, by linearly downsampling 8 voxels
 * to 1 voxel. This does not necessarily happen when using the resample(..) function.
 *
 * @return the resampled volume
 */
class VolumeOperatorHalfsample : public VolumeOperatorUnary<VolumeOperatorHalfsample> {
    friend class VolumeOperatorUnary<VolumeOperatorHalfsample>;
private:
    template<typename T>
    VolumeAtomic<T>* apply_internal(VolumeAtomic<T>* volume) const throw (std::bad_alloc);
};

/**
 * Converts the source volume's data to the destination volume's data type
 * and writes the result to the dest volume.
 *
 * The source volume has to be passed to the constructor,
 * the dest volume has to be passed to apply().
 *
 * @note The dest volume has to be instantiated by the caller
 *       and must match the source volume in dimension and channel count.
 */
class VolumeOperatorConvert : public VolumeOperatorUnary<VolumeOperatorConvert> {
    friend class VolumeOperatorUnary<VolumeOperatorConvert>;

public:
    /**
     * @param srcVolume The volume to be converted. Must not be null.
     */
    VolumeOperatorConvert(const Volume* srcVolume) :
        srcVolume_(srcVolume)
    {}

private:
    /**
     * Performs the conversion.
     *
     * @param destVolume The conversion target. Must not be null
     *  and must match the source volume in dimensions and channel count.
     */
    template<typename T>
    void apply_internal(VolumeAtomic<T>* destVolume) const;

    const Volume* srcVolume_;
};

/**
 * Increase all voxel values in the volume by one.
 */
class VolumeOperatorIncrease : public VolumeOperatorUnary<VolumeOperatorIncrease> {
    friend class VolumeOperatorUnary<VolumeOperatorIncrease>;
private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;
};

/**
 * Decreases all voxel values in the volume by one.
 */
class VolumeOperatorDecrease : public VolumeOperatorUnary<VolumeOperatorDecrease> {
    friend class VolumeOperatorUnary<VolumeOperatorDecrease>;
private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;
};

/**
 * Add the value to all voxels in the volume.
 */
template<typename S = size_t>
class VolumeOperatorAddValue : public VolumeOperatorUnary<VolumeOperatorAddValue<S> > {
    friend class VolumeOperatorUnary<VolumeOperatorAddValue<S> >;
public:
    VolumeOperatorAddValue(S inc) : inc_(inc) {}

private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;

    S inc_;
};

/**
 * Subtracts the value from all voxels in the volume.
 */
template<typename S = size_t>
class VolumeOperatorSubtractValue : public VolumeOperatorUnary<VolumeOperatorSubtractValue<S> > {
    friend class VolumeOperatorUnary<VolumeOperatorSubtractValue<S> >;
public:
    VolumeOperatorSubtractValue(S dec) : dec_(dec) {}

private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;

    S dec_;
};

/**
 * Returns the minimum voxel value in the volume.
 */
class VolumeOperatorMinValue : public VolumeOperatorUnary<VolumeOperatorMinValue> {
    friend class VolumeOperatorUnary<VolumeOperatorMinValue>;
private:
    template<typename T>
    T apply_internal(VolumeAtomic<T>* volume) const;

    template<typename S>
    S apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const;

    template<typename S>
    S apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const;

    template<typename S>
    S apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const;
};

/**
 * Returns the maximum voxel value in the volume.
 */
class VolumeOperatorMaxValue : public VolumeOperatorUnary<VolumeOperatorMaxValue> {
    friend class VolumeOperatorUnary<VolumeOperatorMaxValue>;
private:
    template<typename T>
    T apply_internal(VolumeAtomic<T>* volume) const;

    template<typename S>
    S apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const;

    template<typename S>
    S apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const;

    template<typename S>
    S apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const;
};

/*
 * Inverts all voxel values by subtracting them from the (calculated) maximum value.
 */
class VolumeOperatorInvert : public VolumeOperatorUnary<VolumeOperatorInvert> {
    friend class VolumeOperatorUnary<VolumeOperatorInvert>;
private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;
};

/**
 * Mirrors the volume on the X axis.
 */
class VolumeOperatorMirrorX : public VolumeOperatorUnary<VolumeOperatorMirrorX> {
    friend class VolumeOperatorUnary<VolumeOperatorMirrorX>;
private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;

};

/**
 * Mirrors the volume on the Y axis.
 */
class VolumeOperatorMirrorY : public VolumeOperatorUnary<VolumeOperatorMirrorY> {
    friend class VolumeOperatorUnary<VolumeOperatorMirrorY>;
private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;

};

/**
 * Mirrors the volume on the Z axis.
 */
class VolumeOperatorMirrorZ : public VolumeOperatorUnary<VolumeOperatorMirrorZ> {
    friend class VolumeOperatorUnary<VolumeOperatorMirrorZ>;
private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;

};

/**
 * Checks if all voxels in the volume have the same value.
 */
class VolumeOperatorIsUniform : public VolumeOperatorUnary<VolumeOperatorIsUniform> {
    friend class VolumeOperatorUnary<VolumeOperatorIsUniform>;
private:
    template<typename T>
    bool apply_internal(VolumeAtomic<T>* volume) const;
};

/**
 * Calculates the root mean square error between the two volumes.
 */
class VolumeOperatorCalcError : public VolumeOperatorBinary<VolumeOperatorCalcError> {
    friend class VolumeOperatorBinary<VolumeOperatorCalcError>;
private:
    template<typename T>
    float apply_internal(VolumeAtomic<T>* v1, VolumeAtomic<T>* v2) const;
};

/**
 * Returns the number of significant voxels, i.e., the number of voxels with
 * a value greater than the smallest possible value that can be stored by the volume.
 */
class VolumeOperatorNumSignificant : public VolumeOperatorUnary<VolumeOperatorNumSignificant> {
    friend class VolumeOperatorUnary<VolumeOperatorNumSignificant>;
private:
    template<typename T>
    size_t apply_internal(VolumeAtomic<T>* volume) const;

    template<typename S>
    size_t apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const;

    template<typename S>
    size_t apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const;

    template<typename S>
    size_t apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const;
};

//
// Some more specialized volume operators
//

/*
 * The morphologic operator erosion.
 * TODO: document
 */
class VolumeOperatorErosion : public VolumeOperatorUnary<VolumeOperatorErosion> {
    friend class VolumeOperatorUnary<VolumeOperatorErosion>;
public:
    VolumeOperatorErosion(int kernelSize = 3)
        : kernelSize_(kernelSize)
    {}

private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;

    /// no-op
    template<typename S>
    void apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const;

    /// no-op
    template<typename S>
    void apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const;

    /// no-op
    template<typename S>
    void apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const;

    int kernelSize_;
};

/**
 * The morphologic operator dilation.
 * TODO: document
 */
class VolumeOperatorDilation : public VolumeOperatorUnary<VolumeOperatorDilation> {
    friend class VolumeOperatorUnary<VolumeOperatorDilation>;
public:
    VolumeOperatorDilation(int kernelSize = 3)
        : kernelSize_(kernelSize)
    {}

private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;

    /// unsupported volume format, throws exception
    template<typename S>
    void apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const;

    /// unsupported volume format, throws exception
    template<typename S>
    void apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const;

    /// unsupported volume format, throws exception
    template<typename S>
    void apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const;

    int kernelSize_;
};

/***
 * Median filter.
 * TODO: document
 */
class VolumeOperatorMedian : public VolumeOperatorUnary<VolumeOperatorMedian> {
    friend class VolumeOperatorUnary<VolumeOperatorMedian>;
public:
    VolumeOperatorMedian(int kernelSize = 3)
        : kernelSize_(kernelSize)
    {}

private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;

    /// unsupported volume format, throws exception
    template<typename S>
    void apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const;

    /// unsupported volume format, throws exception
    template<typename S>
    void apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const;

    /// unsupported volume format, throws exception
    template<typename S>
    void apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const;

    int kernelSize_;
};


///////////////////////////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////////////////////////

//
// Implementation for class VolumeOperatorBinary
//

template<class Derived> VolumeOperatorUnary<Derived>::~VolumeOperatorUnary() {}

template<class Derived> template<typename ReturnValue>
ReturnValue VolumeOperatorUnary<Derived>::apply(Volume* volume) const {
    const Derived* const d = static_cast<const Derived* const>(this);

    // This is really ugly but there seems to be no other possibility due
    // to lack of virtual template methods and the needs for a common
    // non-template base class for all kinds of volume classes... (df)
    //
    // Made even uglier by gcc 4.4.0 requiring VolumeAtomic<uint8_t> instead of VolumeUInt8
    // (yes, it is just a typedef!) when compiling with -O2, or linker errors will appear. joerg
    if (VolumeUInt8* v = dynamic_cast<VolumeUInt8*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeAtomic<uint8_t>* v = dynamic_cast<VolumeAtomic<uint8_t>*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeUInt16* v = dynamic_cast<VolumeUInt16*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeAtomic<uint16_t>* v = dynamic_cast<VolumeAtomic<uint16_t>*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeUInt32* v = dynamic_cast<VolumeUInt32*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeAtomic<uint32_t>* v = dynamic_cast<VolumeAtomic<uint32_t>*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeInt8* v = dynamic_cast<VolumeInt8*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeAtomic<int8_t>* v = dynamic_cast<VolumeAtomic<int8_t>*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeInt16* v = dynamic_cast<VolumeInt16*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeAtomic<int16_t>* v = dynamic_cast<VolumeAtomic<int16_t>*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeInt32* v = dynamic_cast<VolumeInt32*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeAtomic<int32_t>* v = dynamic_cast<VolumeAtomic<int32_t>*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeFloat* v = dynamic_cast<VolumeFloat*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeAtomic<float>* v = dynamic_cast<VolumeAtomic<float>*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeDouble* v = dynamic_cast<VolumeDouble*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeAtomic<double>* v = dynamic_cast<VolumeAtomic<double>*>(volume))
        return ReturnValue(d->apply_internal(v));

    /*else if (Volume2xUInt16* v = dynamic_cast<Volume2xUInt16*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (Volume2xInt16* v = dynamic_cast<Volume2xInt16*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (Volume3xUInt8* v = dynamic_cast<Volume3xUInt8*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (Volume3xInt8* v = dynamic_cast<Volume3xInt8*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (Volume3xUInt16* v = dynamic_cast<Volume3xUInt16*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (Volume3xInt16* v = dynamic_cast<Volume3xInt16*>(volume))
        return ReturnValue(d->apply_internal(v));*/

    else if (Volume3xFloat* v = dynamic_cast<Volume3xFloat*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeAtomic<tgt::Vector3<float> >* v = dynamic_cast<VolumeAtomic<tgt::Vector3<float> >*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (Volume3xDouble* v = dynamic_cast<Volume3xDouble*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeAtomic<tgt::Vector3<double> >* v = dynamic_cast<VolumeAtomic<tgt::Vector3<double> >*>(volume))
        return ReturnValue(d->apply_internal(v));

    /*else if (Volume4xUInt8* v = dynamic_cast<Volume4xUInt8*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (Volume4xInt8* v = dynamic_cast<Volume4xInt8*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (Volume4xUInt16* v = dynamic_cast<Volume4xUInt16*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (Volume4xInt16* v = dynamic_cast<Volume4xInt16*>(volume))
        return ReturnValue(d->apply_internal(v));*/

    else if (Volume4xFloat* v = dynamic_cast<Volume4xFloat*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeAtomic<tgt::Vector4<float> >* v = dynamic_cast<VolumeAtomic<tgt::Vector4<float> >*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (Volume4xDouble* v = dynamic_cast<Volume4xDouble*>(volume))
        return ReturnValue(d->apply_internal(v));
    else if (VolumeAtomic<tgt::Vector4<double> >* v = dynamic_cast<VolumeAtomic<tgt::Vector4<double> >*>(volume))
        return ReturnValue(d->apply_internal(v));
#ifdef VRN_MODULE_FLOWREEN
    else if (VolumeFlow3D* v = dynamic_cast<VolumeFlow3D*>(volume))
        return ReturnValue(d->apply_internal(v));
#endif
    else
        throw VolumeOperatorUnsupportedTypeException(typeid(volume).name());

    return ReturnValue(0);
}

//
// Implementation for class VolumeOperatorBinary
//

template<class Derived> VolumeOperatorBinary<Derived>::~VolumeOperatorBinary() {}

template<class Derived> template<typename ReturnValue>
ReturnValue VolumeOperatorBinary<Derived>::apply(Volume* v1, Volume* v2) const {
    if (typeid(*v1) != typeid(*v2))
        throw VoreenException("VolumeOperatorBinary expects identical volume types, got: '"
                              + std::string(typeid(*v1).name()) + "' and '"
                              + std::string(typeid(*v2).name()) + "'.");

    const Derived* const d = static_cast<const Derived* const>(this);

    // This is just an adaptation of VolumeOperatorUnary<Derived>::apply().
    if (VolumeUInt8* v = dynamic_cast<VolumeUInt8*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeUInt8*>(v2)));
    else if (VolumeAtomic<uint8_t>* v = dynamic_cast<VolumeAtomic<uint8_t>*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeAtomic<uint8_t>*>(v2)));
    else if (VolumeUInt16* v = dynamic_cast<VolumeUInt16*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeUInt16*>(v2)));
    else if (VolumeAtomic<uint16_t>* v = dynamic_cast<VolumeAtomic<uint16_t>*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeAtomic<uint16_t>*>(v2)));
    else if (VolumeUInt32* v = dynamic_cast<VolumeUInt32*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeUInt32*>(v2)));
    else if (VolumeAtomic<uint32_t>* v = dynamic_cast<VolumeAtomic<uint32_t>*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeAtomic<uint32_t>*>(v2)));
    else if (VolumeInt8* v = dynamic_cast<VolumeInt8*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeInt8*>(v2)));
    else if (VolumeAtomic<int8_t>* v = dynamic_cast<VolumeAtomic<int8_t>*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeAtomic<int8_t>*>(v2)));
    else if (VolumeInt16* v = dynamic_cast<VolumeInt16*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeInt16*>(v2)));
    else if (VolumeAtomic<int16_t>* v = dynamic_cast<VolumeAtomic<int16_t>*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeAtomic<int16_t>*>(v2)));
    else if (VolumeInt32* v = dynamic_cast<VolumeInt32*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeInt32*>(v2)));
    else if (VolumeAtomic<int32_t>* v = dynamic_cast<VolumeAtomic<int32_t>*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeAtomic<int32_t>*>(v2)));
    else if (VolumeFloat* v = dynamic_cast<VolumeFloat*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeFloat*>(v2)));
    else if (VolumeAtomic<float>* v = dynamic_cast<VolumeAtomic<float>*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeAtomic<float>*>(v2)));
    else if (VolumeDouble* v = dynamic_cast<VolumeDouble*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeDouble*>(v2)));
    else if (VolumeAtomic<double>* v = dynamic_cast<VolumeAtomic<double>*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeAtomic<double>*>(v2)));

    else if (Volume3xFloat* v = dynamic_cast<Volume3xFloat*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<Volume3xFloat*>(v2)));
    else if (VolumeAtomic<tgt::Vector3<float> >* v = dynamic_cast<VolumeAtomic<tgt::Vector3<float> >*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeAtomic<tgt::Vector3<float> >*>(v2)));
    else if (Volume3xDouble* v = dynamic_cast<Volume3xDouble*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<Volume3xDouble*>(v2)));
    else if (VolumeAtomic<tgt::Vector3<double> >* v = dynamic_cast<VolumeAtomic<tgt::Vector3<double> >*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeAtomic<tgt::Vector3<double> >*>(v2)));

    else if (Volume4xFloat* v = dynamic_cast<Volume4xFloat*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<Volume4xFloat*>(v2)));
    else if (VolumeAtomic<tgt::Vector4<float> >* v = dynamic_cast<VolumeAtomic<tgt::Vector4<float> >*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeAtomic<tgt::Vector4<float> >*>(v2)));
    else if (Volume4xDouble* v = dynamic_cast<Volume4xDouble*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<Volume4xDouble*>(v2)));
    else if (VolumeAtomic<tgt::Vector4<double> >* v = dynamic_cast<VolumeAtomic<tgt::Vector4<double> >*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeAtomic<tgt::Vector4<double> >*>(v2)));
#ifdef VRN_MODULE_FLOWREEN
    else if (VolumeFlow3D* v = dynamic_cast<VolumeFlow3D*>(v1))
        return ReturnValue(d->apply_internal(v, dynamic_cast<VolumeFlow3D*>(v1)));
#endif
    else
        throw VolumeOperatorUnsupportedTypeException(typeid(v1).name());

    // we have a problem here, no cast was successfull
    throw VolumeOperatorUnsupportedTypeException(typeid(v1).name());

    return ReturnValue(0);
}



///////////////////////////////////////////////////////////////////////////////
// Implementation of the operators
///////////////////////////////////////////////////////////////////////////////

// ============================================================================

template<typename T>
VolumeAtomic<T>* VolumeOperatorCreateSubset::apply_internal(VolumeAtomic<T>* volume) const throw (std::bad_alloc) {
        VolumeAtomic<T>* subset;
    try {
        subset = new VolumeAtomic<T>(size_, volume->getSpacing(), volume->getTransformation(), volume->getBitsStored());
    }
    catch (std::bad_alloc) {
        LERRORC("voreen.VolumeOperatorCreateSubset", "Failed to create subset: bad allocation");
        throw; // throw it to the caller
    }

    subset->meta() = volume->meta();

    // calculate new imageposition
    if (pos_.z != 0.f)
        subset->meta().setImagePositionZ(volume->meta().getImagePositionZ() - volume->getSpacing().z * pos_.z);

    LINFOC("voreen.VolumeOperatorCreateSubset", "Creating subset " << size_ << " from position " << pos_);

    // create values for ranges less than zero and greater equal dimensions_
    subset->clear(); // TODO: This can be optomized by avoiding to clear the values in range

    // now the rest
    tgt::ivec3 start = tgt::max(pos_, tgt::ivec3::zero);// clamp values
    tgt::ivec3 end   = tgt::min(pos_ + size_, volume->getDimensions());    // clamp values

    VRN_FOR_EACH_VOXEL(i, tgt::ivec3(0), end - start)
        subset->voxel(i) = volume->voxel(i+start);

    return subset;
}

template<typename T>
VolumeAtomic<T>* VolumeOperatorResize::apply_internal(VolumeAtomic<T>* volume) const throw (std::bad_alloc) {

    LINFOC("voreen.VolumeOperatorResize", "Resizing from dimensions " << volume->getDimensions() << " to " << newDims_);

    // build target volume
    VolumeAtomic<T>* result;
    try {
        result = new VolumeAtomic<T>(newDims_, volume->getSpacing(), volume->getTransformation(),
            volume->getBitsStored(), allocMem_);
    }
    catch (std::bad_alloc) {
        throw; // throw it to the caller
    }
    result->meta() = volume->meta();

    return result;
}

template<typename T>
VolumeAtomic<T>* VolumeOperatorResample::apply_internal(VolumeAtomic<T>* volume) const throw (std::bad_alloc) {
    using tgt::vec3;
    using tgt::ivec3;

    LINFOC("voreen.VolumeOperatorResample", "Resampling from dimensions " << volume->getDimensions() << " to " << newDims_);

    vec3 ratio = vec3(volume->getDimensions()) / vec3(newDims_);
    vec3 invDims = 1.f / vec3(volume->getDimensions());

    ivec3 pos = ivec3::zero; // iteration variable
    vec3 nearest; // knows the new position of the target volume

    // build target volume
    VolumeAtomic<T>* v;
    try {
         v = new VolumeAtomic<T>(newDims_, volume->getSpacing()*ratio, volume->getTransformation(), volume->getBitsStored());
    }
    catch (std::bad_alloc) {
        throw; // throw it to the caller
    }

    v->setTransformation(volume->getTransformation());
    v->meta() = volume->meta();

    if (progressBar_)
        progressBar_->setProgress(0.f);

    /*
        Filter from the source volume to the target volume.
    */
    switch (filter_) {
    case Volume::NEAREST:
        for (pos.z = 0; pos.z < newDims_.z; ++pos.z) {

            if (progressBar_)
                progressBar_->setProgress(static_cast<float>(pos.z) / static_cast<float>(newDims_.z));

            nearest.z = static_cast<float>(pos.z) * ratio.z;

            for (pos.y = 0; pos.y < newDims_.y; ++pos.y) {
                nearest.y = static_cast<float>(pos.y) * ratio.y;

                for (pos.x = 0; pos.x < newDims_.x; ++pos.x) {
                    nearest.x = static_cast<float>(pos.x) * ratio.x;

                    ivec3 index = tgt::clamp(ivec3(nearest + 0.5f), ivec3(0), volume->getDimensions() - 1);
                    v->voxel(pos) = volume->voxel(index); // round and do the lookup
                }
            }
        }
        break;

    case Volume::LINEAR:
        for (pos.z = 0; pos.z < newDims_.z; ++pos.z) {

            if (progressBar_)
                progressBar_->setProgress(static_cast<float>(pos.z) / static_cast<float>(newDims_.z));

            nearest.z = static_cast<float>(pos.z) * ratio.z;

            for (pos.y = 0; pos.y < newDims_.y; ++pos.y) {
                nearest.y = static_cast<float>(pos.y) * ratio.y;

                for (pos.x = 0; pos.x < newDims_.x; ++pos.x) {
                    nearest.x = static_cast<float>(pos.x) * ratio.x;
                    vec3 p = nearest - floor(nearest); // get decimal part
                    ivec3 llb = ivec3(nearest);
                    ivec3 urf = ivec3(ceil(nearest));
                    urf = tgt::min(urf, volume->getDimensions() - 1); // clamp so the lookups do not exceed the dimensions

                    /*
                      interpolate linearly
                    */
                    typedef typename VolumeElement<T>::DoubleType Double;
                    v->voxel(pos) =
                        T(  Double(volume->voxel(llb.x, llb.y, llb.z)) * static_cast<double>((1.f-p.x)*(1.f-p.y)*(1.f-p.z))  // llB
                          + Double(volume->voxel(urf.x, llb.y, llb.z)) * static_cast<double>((    p.x)*(1.f-p.y)*(1.f-p.z))  // lrB
                          + Double(volume->voxel(urf.x, urf.y, llb.z)) * static_cast<double>((    p.x)*(    p.y)*(1.f-p.z))  // urB
                          + Double(volume->voxel(llb.x, urf.y, llb.z)) * static_cast<double>((1.f-p.x)*(    p.y)*(1.f-p.z))  // ulB
                          + Double(volume->voxel(llb.x, llb.y, urf.z)) * static_cast<double>((1.f-p.x)*(1.f-p.y)*(    p.z))  // llF
                          + Double(volume->voxel(urf.x, llb.y, urf.z)) * static_cast<double>((    p.x)*(1.f-p.y)*(    p.z))  // lrF
                          + Double(volume->voxel(urf.x, urf.y, urf.z)) * static_cast<double>((    p.x)*(    p.y)*(    p.z))  // urF
                          + Double(volume->voxel(llb.x, urf.y, urf.z)) * static_cast<double>((1.f-p.x)*(    p.y)*(    p.z)));// ulF
                }
            }
        }
        break;
    }

    if (progressBar_)
        progressBar_->setProgress(1.f);

    return v;
}

// ============================================================================

template<typename T>
VolumeAtomic<T>* VolumeOperatorHalfsample::apply_internal(VolumeAtomic<T>* volume) const throw (std::bad_alloc) {
    tgt::ivec3 dims = volume->getDimensions();
    tgt::ivec3 halfDims = volume->getDimensions() / 2;

    VolumeAtomic<T>* newVolume = new VolumeAtomic<T>(halfDims, volume->getSpacing()*2.f,
        volume->getTransformation(), volume->getBitsStored());

    typedef typename VolumeElement<T>::DoubleType Double;
    VRN_FOR_EACH_VOXEL_WITH_PROGRESS(index, tgt::ivec3(0), halfDims, progressBar_) {
        tgt::ivec3 pos = index*2; // tgt::ivec3(2*x,2*y,2*z);
        newVolume->voxel(index) =
            T(  Double(volume->voxel(pos.x, pos.y, pos.z))          * (1.0/8.0) //LLF
              + Double(volume->voxel(pos.x, pos.y, pos.z+1))        * (1.0/8.0) //LLB
              + Double(volume->voxel(pos.x, pos.y+1, pos.z))        * (1.0/8.0) //ULF
              + Double(volume->voxel(pos.x, pos.y+1, pos.z+1))      * (1.0/8.0) //ULB
              + Double(volume->voxel(pos.x+1, pos.y, pos.z))        * (1.0/8.0) //LRF
              + Double(volume->voxel(pos.x+1, pos.y, pos.z+1))      * (1.0/8.0) //LRB
              + Double(volume->voxel(pos.x+1, pos.y+1, pos.z))      * (1.0/8.0) //URF
              + Double(volume->voxel(pos.x+1, pos.y+1, pos.z+1))    * (1.0/8.0)); //URB
    }
    if (progressBar_)
        progressBar_->setProgress(1.f);

    return newVolume;
}

// ============================================================================

template<typename T>
void VolumeOperatorConvert::apply_internal(VolumeAtomic<T>* destVolume) const {

    if (!srcVolume_)
        throw VoreenException("VolumeOperatorConvert: source volume is null pointer");
    if (!destVolume)
        throw VoreenException("VolumeOperatorConvert: dest volume is null pointer");

    if (destVolume->getDimensions() != srcVolume_->getDimensions())
        throw VoreenException("VolumeOperatorConvert: volume dimensions must match");
    if (destVolume->getNumChannels() != srcVolume_->getNumChannels())
        throw VoreenException("VolumeOperatorConvert: number of channels must match");

    // check the source volume's type
    const VolumeUInt8* src8 = dynamic_cast<const VolumeUInt8*>(srcVolume_);
    const VolumeUInt16* src16 = dynamic_cast<const VolumeUInt16*>(srcVolume_);
    const VolumeFloat* srcFloat = dynamic_cast<const VolumeFloat*>(srcVolume_);
    const VolumeDouble* srcDouble = dynamic_cast<const VolumeDouble*>(srcVolume_);

    // check the dest volume's type
    VolumeUInt8* dest8 = dynamic_cast<VolumeUInt8*>(destVolume);
    VolumeUInt16* dest16 = dynamic_cast<VolumeUInt16*>(destVolume);

    if (src8 && dest8) {
        LINFOC("voreen.VolumeOperatorConvert" ,"No conversion necessary: source and dest type equal (VolumeUInt8)");
        VRN_FOR_EACH_VOXEL(i, tgt::ivec3(0), src8->getDimensions())
            dest8->voxel(i) = src8->voxel(i);
    }
    else if (src16 && dest16) {
        LINFOC("voreen.VolumeOperatorConvert" ,"No conversion necessary: source and dest type equal (VolumeUInt16)");
        VRN_FOR_EACH_VOXEL(i, tgt::ivec3(0), src16->getDimensions())
            dest16->voxel(i) = src16->voxel(i);
    }
    else if (src16 && dest8) {
        LINFOC("voreen.VolumeOperatorConvert", "Using accelerated conversion from VolumeUInt16 -> VolumeUInt8");
        // because the number of shifting bits varies by the number of bits used it must be calculated
        int shift = src16->getBitsStored() - dest8->getBitsStored();
        VRN_FOR_EACH_VOXEL(i, tgt::ivec3(0), src16->getDimensions())
            dest8->voxel(i) = src16->voxel(i) >> shift;
    }
    else if (src8 && dest16) {
        LINFOC("voreen.VolumeOperatorConvert", "Using accelerated conversion from VolumeUInt8 -> VolumeUInt16");
        // because the number of shifting bits varies by the number of bits used it must be calculated
        int shift = dest16->getBitsStored() - src8->getBitsStored();
        VRN_FOR_EACH_VOXEL(i, tgt::ivec3(0), src8->getDimensions())
            dest16->voxel(i) = src8->voxel(i) << shift;
    }
    else if (srcFloat) {
        float min = srcFloat->min();
        float max = srcFloat->max();
        float range = (max - min);

        LINFOC("voreen.VolumeOperatorConvert", "Converting float volume with data range [" << min << "; " << max << "] to "
            << destVolume->getBitsAllocated() << " bit (normalized).");

        VRN_FOR_EACH_VOXEL(i, tgt::ivec3(0), srcFloat->getDimensions())
            destVolume->setVoxelFloat((srcFloat->voxel(i) - min) / range, i);
    }
    else if (srcDouble) {
        double min = srcDouble->min();
        double max = srcDouble->max();
        double range = (max - min);

        LINFOC("voreen.VolumeOperatorConvert", "Converting double volume with data range [" << min << "; " << max << "] to "
            << destVolume->getBitsAllocated() << " bit (normalized).");

        VRN_FOR_EACH_VOXEL(i, tgt::ivec3(0), srcDouble->getDimensions())
            destVolume->setVoxelFloat(static_cast<float>((srcDouble->voxel(i) - min) / range), i);
    }
    else {
        LINFOC("voreen.VolumeOperatorConvert", "Using fallback with setVoxelFloat and getVoxelFloat");
        VRN_FOR_EACH_VOXEL(i, tgt::ivec3(0), srcVolume_->getDimensions())
            destVolume->setVoxelFloat(srcVolume_->getVoxelFloat(i), i);
    }

    if (progressBar_)
        progressBar_->setProgress(1.f);
}

// ============================================================================

template<typename S> template<typename T>
void VolumeOperatorSubtractValue<S>::apply_internal(VolumeAtomic<T>* volume) const {
    T* voxel = volume->voxel();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i)
        voxel[i] -= T(dec_);
}

// ============================================================================

template<typename S> template<typename T>
void VolumeOperatorAddValue<S>::apply_internal(VolumeAtomic<T>* volume) const {
    T* voxel = volume->voxel();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i)
        voxel[i] += T(inc_);
}

// ============================================================================

template<typename T>
void VolumeOperatorIncrease::apply_internal(VolumeAtomic<T>* volume) const {
    T* voxel = volume->voxel();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i)
        voxel[i] += T(1);
}

// ============================================================================

template<typename T>
void VolumeOperatorDecrease::apply_internal(VolumeAtomic<T>* volume) const {
    T* voxel = volume->voxel();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i)
        voxel[i] -= T(1);
}

// ============================================================================

template<typename T>
T VolumeOperatorMinValue::apply_internal(VolumeAtomic<T>* volume) const {
    T* voxel = volume->voxel();
    T min = std::numeric_limits<T>::max();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        if (voxel[i] < min)
            min = voxel[i];
    }
    return min;
}

// specialized template version for tgt::VectorX clases which do no implement operator>().
//

template<typename S>
S VolumeOperatorMinValue::apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const {
    tgt::Vector2<S>* voxel = volume->voxel();
    S min = std::numeric_limits<S>::max();
    S voxelMin = min;

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        voxelMin = tgt::min(voxel[i]);
        if (voxelMin < min)
            min = voxelMin;
    }
    return min;
}

template<typename S>
S VolumeOperatorMinValue::apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const {
    tgt::Vector3<S>* voxel = volume->voxel();
    S min = std::numeric_limits<S>::max();
    S voxelMin = min;

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        voxelMin = tgt::min(voxel[i]);
        if (voxelMin < min)
            min = voxelMin;
    }
    return min;
}

template<typename S>
S VolumeOperatorMinValue::apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const {
    tgt::Vector4<S>* voxel = volume->voxel();
    S min = std::numeric_limits<S>::max();
    S voxelMin = min;

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        voxelMin = tgt::min(voxel[i]);
        if (voxelMin < min)
            min = voxelMin;
    }
    return min;
}
// ============================================================================

template<typename T>
T VolumeOperatorMaxValue::apply_internal(VolumeAtomic<T>* volume) const {
    T* voxel = volume->voxel();
    T max = std::numeric_limits<T>::min();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        if (voxel[i] > max)
            max = voxel[i];
    }
    return max;
}

// specialized template version for tgt::VectorX classes which do no implement operator>().

template<typename S>
S VolumeOperatorMaxValue::apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const {
    tgt::Vector2<S>* voxel = volume->voxel();
    S max = std::numeric_limits<S>::min();
    S voxelMax = max;

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        voxelMax = tgt::max(voxel[i]);
        if (voxelMax > max)
            max = voxelMax;
    }
    return max;
}

template<typename S>
S VolumeOperatorMaxValue::apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const {
    tgt::Vector3<S>* voxel = volume->voxel();
    S max = std::numeric_limits<S>::min();
    S voxelMax = max;

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        voxelMax = tgt::max(voxel[i]);
        if (voxelMax > max)
            max = voxelMax;
    }
    return max;
}

template<typename S>
S VolumeOperatorMaxValue::apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const {
    tgt::Vector4<S>* voxel = volume->voxel();
    S max = std::numeric_limits<S>::min();
    S voxelMax = max;

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        voxelMax = tgt::max(voxel[i]);
        if (voxelMax > max)
            max = voxelMax;
    }
    return max;
}

// ============================================================================

template<typename T>
void VolumeOperatorInvert::apply_internal(VolumeAtomic<T>* volume) const {
    VolumeOperatorMaxValue maxOp;
    T max = T(maxOp.apply<float>(volume));
    T* voxel = volume->voxel();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i)
        voxel[i] = max - voxel[i];
}

// ============================================================================

template<typename T>
void VolumeOperatorMirrorX::apply_internal(VolumeAtomic<T>* volume) const {
    // This could also be implemented without an additional copy...
    VolumeAtomic<T>* mirror = volume->clone();

    VRN_FOR_EACH_VOXEL_WITH_PROGRESS(i, tgt::ivec3(0), mirror->getDimensions(), progressBar_)
        volume->voxel(i) = mirror->voxel(mirror->getDimensions().x - i.x - 1, i.y, i.z);

    delete mirror;

    if (progressBar_)
        progressBar_->setProgress(1.f);

    volume->invalidate();
}

// ============================================================================

template<typename T>
void VolumeOperatorMirrorY::apply_internal(VolumeAtomic<T>* volume) const {
    // This could also be implemented without an additional copy...
    VolumeAtomic<T>* mirror = volume->clone();

    VRN_FOR_EACH_VOXEL_WITH_PROGRESS(i, tgt::ivec3(0), mirror->getDimensions(), progressBar_)
        volume->voxel(i) = mirror->voxel(i.x, mirror->getDimensions().y - i.y - 1, i.z);

    delete mirror;

    if (progressBar_)
        progressBar_->setProgress(1.f);

    volume->invalidate();
}

// ============================================================================

template<typename T>
void VolumeOperatorMirrorZ::apply_internal(VolumeAtomic<T>* volume) const {
    // This could also be implemented without an additional copy...
    VolumeAtomic<T>* mirror = volume->clone();

    VRN_FOR_EACH_VOXEL_WITH_PROGRESS(i, tgt::ivec3(0), mirror->getDimensions(), progressBar_)
        volume->voxel(i) = mirror->voxel(i.x, i.y, mirror->getDimensions().z - i.z - 1);

    delete mirror;

    if (progressBar_)
        progressBar_->setProgress(1.f);

    volume->invalidate();
}

// ============================================================================

template<typename T>
bool VolumeOperatorIsUniform::apply_internal(VolumeAtomic<T>* volume) const {
    T firstVoxel = volume->voxel(0);
    bool allVoxelsEqual=true;
    for (size_t i=1; i < volume->getNumVoxels(); i++) {
        T currentVoxel = volume->voxel(i);
        if (firstVoxel != currentVoxel) {
            allVoxelsEqual = false;
            break;
        }
    }
    return allVoxelsEqual;
}

// ============================================================================

template<typename T>
float VolumeOperatorCalcError::apply_internal(VolumeAtomic<T>* v1, VolumeAtomic<T>* v2) const {
    double errorSum = 0.0;
    tgt::ivec3 factor = v1->getDimensions() / v2->getDimensions();

    for (int i=0; i<v1->getDimensions().z; i++) {
        for (int j=0; j<v1->getDimensions().y; j++) {
            for (int k=0; k<v1->getDimensions().x; k++) {
                tgt::ivec3 currentPos = tgt::ivec3(k,j,i);
                tgt::ivec3 smallVolumePos;
                smallVolumePos.x = static_cast<int>( floor(currentPos.x / (float)factor.x));
                smallVolumePos.y = static_cast<int>( floor(currentPos.y / (float)factor.y));
                smallVolumePos.z = static_cast<int>( floor(currentPos.z / (float)factor.z));
                T origVoxel = v1->voxel(currentPos);
                T errVoxel = v2->voxel(smallVolumePos);

                errorSum = errorSum + VolumeElement<T>::calcSquaredDifference(origVoxel, errVoxel);
            }
        }
    }

    errorSum = errorSum / (float)(v1->getNumVoxels());
    errorSum = sqrt(errorSum);
    errorSum = errorSum / (double)VolumeElement<T>::rangeMaxElement();

    return (float)errorSum;
}

// ============================================================================

template<typename T>
size_t VolumeOperatorNumSignificant::apply_internal(VolumeAtomic<T>* volume) const {
    size_t result = 0;
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volume->getDimensions()) {
        if (volume->voxel(pos) > VolumeElement<T>::rangeMin())
            result++;
    }
    return result;
}

// specialized template version for tgt::VectorX classes which do no implement operator>().
// do nothing in these cases

template<typename S>
size_t VolumeOperatorNumSignificant::apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const {
    size_t result = 0;
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volume->getDimensions()) {
        if (tgt::hadd(volume->voxel(pos)) > VolumeElement<S>::rangeMin())
            result++;
    }
    return result;
}

template<typename S>
size_t VolumeOperatorNumSignificant::apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const {
    size_t result = 0;
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volume->getDimensions()) {
        if (tgt::hadd(volume->voxel(pos)) > VolumeElement<S>::rangeMin())
            result++;
    }
    return result;
}

template<typename S>
size_t VolumeOperatorNumSignificant::apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const {
    size_t result = 0;
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volume->getDimensions()) {
        if (tgt::hadd(volume->voxel(pos)) > VolumeElement<S>::rangeMin())
            result++;
    }
    return result;
}

// ============================================================================

template<typename T>
void VolumeOperatorDilation::apply_internal(VolumeAtomic<T>* volume) const {
    VolumeAtomic<T>* input = volume->clone();

    int halfKernelDim = kernelSize_ / 2;
    tgt::ivec3 volDim = input->getDimensions();

    // kernel is separable => consecutively apply 1D kernel along each axis instead of a 3D kernel

    // x-direction (inputVol -> outputVol)
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volDim) {
        int xmin = std::max(pos.x-halfKernelDim, 0);
        int xmax = std::min(pos.x+halfKernelDim, volDim.x-1);

        T val = input->voxel(pos);
        tgt::ivec3 npos = pos;
        for (npos.x=xmin; npos.x<=xmax; npos.x++) {
            val = std::max(val, input->voxel(npos));
        }
        volume->voxel(pos) = val;
    }

    // y-direction (outputVol -> inputVol)
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volDim) {
        int ymin = std::max(pos.y-halfKernelDim, 0);
        int ymax = std::min(pos.y+halfKernelDim, volDim.y-1);

        T val = volume->voxel(pos);
        tgt::ivec3 npos = pos;
        for (npos.y=ymin; npos.y<=ymax; npos.y++) {
            val = std::max(val, volume->voxel(npos));
        }
        input->voxel(pos) = val;
    }

    // z-direction (inputVol -> outputVol)
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volDim) {
        int zmin = std::max(pos.z-halfKernelDim, 0);
        int zmax = std::min(pos.z+halfKernelDim, volDim.z-1);

        T val = input->voxel(pos);
        tgt::ivec3 npos = pos;
        for (npos.z=zmin; npos.z<=zmax; npos.z++) {
            val = std::max(val, input->voxel(npos));
        }
        volume->voxel(pos) = val;
    }

    delete input;

    if (progressBar_)
        progressBar_->setProgress(1.f);

    volume->invalidate();
}

// specialized template version for tgt::VectorX classes which do no implement operator>().

template<typename S>
void VolumeOperatorDilation::apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const {
    throw VolumeOperatorUnsupportedTypeException(typeid(*volume).name());
}

template<typename S>
void VolumeOperatorDilation::apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const {
    throw VolumeOperatorUnsupportedTypeException(typeid(*volume).name());
}

template<typename S>
void VolumeOperatorDilation::apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const {
    throw VolumeOperatorUnsupportedTypeException(typeid(*volume).name());
}

// ============================================================================

template<typename T>
void VolumeOperatorErosion::apply_internal(VolumeAtomic<T>* volume) const {
    VolumeAtomic<T>* input = volume->clone();

    int halfKernelDim = kernelSize_ / 2;
    tgt::ivec3 volDim = input->getDimensions();

    // kernel is separable => consecutively apply 1D kernel along each axis instead of a 3D kernel

    // x-direction (inputVol -> outputVol)
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volDim) {
        int xmin = std::max(pos.x-halfKernelDim, 0);
        int xmax = std::min(pos.x+halfKernelDim, volDim.x-1);

        T val = input->voxel(pos);
        tgt::ivec3 npos = pos;
        for (npos.x=xmin; npos.x<=xmax; npos.x++) {
            val = std::min(val, input->voxel(npos));
        }
        volume->voxel(pos) = val;
    }

    // y-direction (outputVol -> inputVol)
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volDim) {
        int ymin = std::max(pos.y-halfKernelDim, 0);
        int ymax = std::min(pos.y+halfKernelDim, volDim.y-1);

        T val = volume->voxel(pos);
        tgt::ivec3 npos = pos;
        for (npos.y=ymin; npos.y<=ymax; npos.y++) {
            val = std::min(val, volume->voxel(npos));
        }
        input->voxel(pos) = val;
    }

    // z-direction (inputVol -> outputVol)
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volDim) {
        int zmin = std::max(pos.z-halfKernelDim, 0);
        int zmax = std::min(pos.z+halfKernelDim, volDim.z-1);

        T val = input->voxel(pos);
        tgt::ivec3 npos = pos;
        for (npos.z=zmin; npos.z<=zmax; npos.z++) {
            val = std::min(val, input->voxel(npos));
        }
        volume->voxel(pos) = val;
    }

    delete input;

    if (progressBar_)
        progressBar_->setProgress(1.f);

    volume->invalidate();
}

// specialized template version for tgt::VectorX classes which do no implement operator>().

template<typename S>
void VolumeOperatorErosion::apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const {
    throw VolumeOperatorUnsupportedTypeException(typeid(*volume).name());
}

template<typename S>
void VolumeOperatorErosion::apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const {
    throw VolumeOperatorUnsupportedTypeException(typeid(*volume).name());
}

template<typename S>
void VolumeOperatorErosion::apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const {
    throw VolumeOperatorUnsupportedTypeException(typeid(*volume).name());
}

// ============================================================================

template<typename T>
void VolumeOperatorMedian::apply_internal(VolumeAtomic<T>* volume) const {
    VolumeAtomic<T>* input = volume->clone();

    int halfKernelDim = kernelSize_ / 2;
    tgt::ivec3 volDim = input->getDimensions();
    VRN_FOR_EACH_VOXEL_WITH_PROGRESS(pos, tgt::ivec3(0), volDim, progressBar_) {
        int zmin = std::max(pos.z-halfKernelDim, 0);
        int zmax = std::min(pos.z+halfKernelDim, volDim.z-1);
        int ymin = std::max(pos.y-halfKernelDim, 0);
        int ymax = std::min(pos.y+halfKernelDim, volDim.y-1);
        int xmin = std::max(pos.x-halfKernelDim, 0);
        int xmax = std::min(pos.x+halfKernelDim, volDim.x-1);

        tgt::ivec3 npos;
        std::vector<T> values;
        for (npos.z=zmin; npos.z<=zmax; npos.z++) {
            for (npos.y=ymin; npos.y<=ymax; npos.y++) {
                for (npos.x=xmin; npos.x<=xmax; npos.x++) {
                    values.push_back(input->voxel(npos));
                }
            }
        }
        size_t len = values.size();
        nth_element(values.begin(), values.begin()+(len/2), values.end());
        volume->voxel(pos) = values[len / 2];
    }
    delete input;

    if (progressBar_)
        progressBar_->setProgress(1.f);

    volume->invalidate();
}

// specialized template version for tgt::VectorX classes which do no implement operator>().

template<typename S>
void VolumeOperatorMedian::apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const {
    throw VolumeOperatorUnsupportedTypeException(typeid(*volume).name());
}

template<typename S>
void VolumeOperatorMedian::apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const {
    throw VolumeOperatorUnsupportedTypeException(typeid(*volume).name());
}

template<typename S>
void VolumeOperatorMedian::apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const {
    throw VolumeOperatorUnsupportedTypeException(typeid(*volume).name());
}

} // namespace

#endif // VRN_VOLUMEOPERATOR_H
