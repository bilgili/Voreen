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
    virtual ~VolumeOperatorUnary() = 0; // keeps the class abstract

    /**
     * Apply the operator on the given volume. The operator can modify the volume and also
     * return a value.
     */
    template<typename ReturnValue>
    ReturnValue apply(Volume* volume) const;

    /**
     * Alternative way for calling apply() when its return type is void.
     */
    inline void operator()(Volume* volume) const {
        apply<void>(volume);
    }
};

/**
 * Parent class of volume operators for two volumes.
 * Both volumes must have the same type.
 */
template<class Derived>     // curiously recurring template pattern (CRTP) for static polymorphism
class VolumeOperatorBinary {
public:
    virtual ~VolumeOperatorBinary() = 0;

    /**
     * Apply the operator on the given volumes. The operator can modify the volumes and also
     * return a value.
     */
    template<typename ReturnValue>
    ReturnValue apply(Volume* v1, Volume* v2) const;

    /**
     * Alternative way for calling apply() when its return type is void.
     */
    inline void operator()(Volume* v1, Volume* v2) const {
        apply<void>(v1, v2);
    }
};

//
// Some common volume operators
//

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
    if (typeid(v1) != typeid(v2))
        throw VoreenException("VolumeOperatorBinary expects identical volume types, got: '"
                              + std::string(typeid(v1).name()) + "' and '"
                              + std::string(typeid(v2).name()) + "'.");

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
void VolumeOperatorMirrorZ::apply_internal(VolumeAtomic<T>* volume) const {
    // This could also be implemented without an additional copy...
    VolumeAtomic<T>* mirror = volume->clone();

    VRN_FOR_EACH_VOXEL(i, tgt::ivec3(0), mirror->getDimensions())
        volume->voxel(i) = mirror->voxel(i.x, i.y, mirror->getDimensions().z - i.z - 1);

    delete mirror;

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
                tgt::ivec3 smallVolumePos = calcPosInSmallerVolume(currentPos, factor);
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
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volDim) {
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
