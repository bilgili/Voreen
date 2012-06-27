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

#ifndef VRN_VOLUMEOPERATOR_H
#define VRN_VOLUMEOPERATOR_H

#include "voreen/core/volume/volumeatomic.h"
#include <vector>

namespace voreen {

// ============================================================================

template<class Derived>     // CRTP for static polymorphism
class VolumeOperatorUnary {
public:
    virtual ~VolumeOperatorUnary() = 0; // keeps the class abstract

    template<typename ReturnValue>
    ReturnValue apply(Volume* volume) const;

    inline void operator()(Volume* volume) const {
        apply<void>(volume);
    }
};

// ----------------------------------------------------------------------------

// implementations for class VolumeOperatorBinary

template<class Derived> VolumeOperatorUnary<Derived>::~VolumeOperatorUnary() {}

template<class Derived> template<typename ReturnValue>
ReturnValue VolumeOperatorUnary<Derived>::apply(Volume* volume) const {
    const Derived* const d = static_cast<const Derived* const>(this);

    // This is REALLY ugly but there seems to be no other possibility due
    // to lack of virtual template methods and the needs for a common
    // non-template base class for all kinds of volume classes... (df)
    //
    // Made even uglier by gcc 4.4.0 requiring VolumeAtomic<uint8_t> instead of VolumeUInt8
    // (yes, it is just a typedef!) when compiling with -O2, or linker array will appear. joerg
    if (VolumeUInt8* v = dynamic_cast<VolumeUInt8*>(volume))
        return ReturnValue(d->apply_internal(v));
    if (VolumeAtomic<uint8_t>* v = dynamic_cast<VolumeAtomic<uint8_t>*>(volume))
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
    else if (Volume3xDouble* v = dynamic_cast<Volume3xDouble*>(volume))
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
    else if (Volume4xDouble* v = dynamic_cast<Volume4xDouble*>(volume))
        return ReturnValue(d->apply_internal(v));
    else {
        std::cout << "serious problem:\n\tcannot apply unary volume operator to volume of type '"
            << typeid(volume).name() << "'!\n";
    }
    return ReturnValue(0);
}

// ============================================================================

// Currently unused and causing gcc compile errors (don't know why) (df)

/*
template<class Derived>     // CRTP for static polymorphism
class VolumeOperatorBinary {
public:
    virtual ~VolumeOperatorBinary() = 0;

    template<typename ReturnValue>
    ReturnValue apply(Volume* v1, Volume* v2) const;

    inline void operator()(Volume* v1, Volume* v2) const {
        apply<void>(v1, v2);
    }
};

// ----------------------------------------------------------------------------

// implementations for class VolumeOperatorBinary

template<class Derived> VolumeOperatorBinary<Derived>::~VolumeOperatorBinary() {}

template<class Derived> template<typename ReturnValue>
ReturnValue VolumeOperatorBinary<Derived>::apply(Volume* v1, Volume* v2) const {
    const Derived* const d = static_cast<const Derived* const>(this);

    // This is REALLY ugly but there seems to be no other possibility due
    // to lack of virtual template methods and the needs for a common
    // non-template base class for all kinds of volume classes... (df)
    //
    if ((VolumeFloat* _v1 = dynamic_cast<VolumeFloat*>(v1))
        && (VolumeFloat* _v2 = dynamic_cast<VolumeFloat*>(v2))) {
        return ReturnValue(d->apply_internal(_v1, _v2));
    }
    // TODO: implement castings for other volume types like above...
    else {
        std::cout << "serious problem:\n\tcannot apply biary volume operator to volumes of type '"
            << typeid(v1).name() << "' and '" << typeid(v2).name() << "'!\n";
    }
}
*/
// ============================================================================

class VolumeOperatorIncrease : public VolumeOperatorUnary<VolumeOperatorIncrease> {

friend class VolumeOperatorUnary<VolumeOperatorIncrease>;

private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;
};

// ============================================================================

class VolumeOperatorDecrease : public VolumeOperatorUnary<VolumeOperatorDecrease> {

friend class VolumeOperatorUnary<VolumeOperatorDecrease>;

private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;
};

// ============================================================================

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

// ----------------------------------------------------------------------------

template<typename S> template<typename T>
void VolumeOperatorAddValue<S>::apply_internal(VolumeAtomic<T>* volume) const {
    T* voxel = volume->voxel();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i)
        voxel[i] += T(inc_);
}

// ============================================================================

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

// ----------------------------------------------------------------------------

template<typename S> template<typename T>
void VolumeOperatorSubtractValue<S>::apply_internal(VolumeAtomic<T>* volume) const {
    T* voxel = volume->voxel();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i)
        voxel[i] -= T(dec_);
}

// ============================================================================

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

// ============================================================================

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

// ============================================================================

class VolumeOperatorInvert : public VolumeOperatorUnary<VolumeOperatorInvert> {
friend class VolumeOperatorUnary<VolumeOperatorInvert>;
private:
    template<typename T>
    void apply_internal(VolumeAtomic<T>* volume) const;
};

}   // namespace

#endif
