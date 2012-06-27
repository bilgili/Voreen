/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#ifndef VRN_PORTCONDITION_VOLUMETYPE_H
#define VRN_PORTCONDITION_VOLUMETYPE_H

#include "voreen/core/ports/conditions/portcondition.h"
#include "voreen/core/ports/volumeport.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"

namespace voreen {

/**
 * Generic base class for port conditions that check for a specific volume type T.
 */
template<typename T>
class PortConditionVolumeType : public PortCondition {
public:
    PortConditionVolumeType(const std::string& volTypeName);

    virtual ~PortConditionVolumeType();

    /**
     * Returns true, if the inport has data and
     * the encapsulated volume is of type T.
     */
    virtual bool acceptsPortData() const;

protected:
    virtual void setCheckedPort(const Port* checkedPort);

    const VolumePort* volumePort_;
};

// ------------------------------------------------------------------------------------------------

/**
 * Port condition that allows OpenGL-compatible volume types,
 * i.e., volumes that can be converted into an OpenGL texture.
 * These are all standard volume types except 64 integer and double:
 * - VolumeRAM_UInt8, VolumeRAM_Int8
 * - VolumeRAM_UInt16, VolumeRAM_Int16
 * - VolumeRAM_UInt32, VolumeRAM_Int32
 * - VolumeRAM_Float
 * - VolumeRAM_2xUInt8, VolumeRAM_2xInt8
 * - VolumeRAM_2xUInt16, VolumeRAM_2xInt16
 * - VolumeRAM_2xUInt32, VolumeRAM_2xInt32
 * - VolumeRAM_2xFloat
 * - VolumeRAM_3xUInt8, VolumeRAM_3xInt8
 * - VolumeRAM_3xUInt16, VolumeRAM_3xInt16
 * - VolumeRAM_3xUInt32, VolumeRAM_3xInt32
 * - VolumeRAM_3xFloat
 * - VolumeRAM_4xUInt8, VolumeRAM_4xInt8
 * - VolumeRAM_4xUInt16, VolumeRAM_4xInt16
 * - VolumeRAM_4xUInt32, VolumeRAM_4xInt32
 * - VolumeRAM_4xFloat
 */
class PortConditionVolumeTypeGL : public PortCondition {
public:
    PortConditionVolumeTypeGL();

    virtual bool acceptsPortData() const;

protected:
    virtual void setCheckedPort(const Port* checkedPort);
    PortConditionLogicalOr orCondition_;
};

// ------------------------------------------------------------------------------------------------

/**
 * Port condition that allows float and double volume types:
 * - VolumeRAM_Float, VolumeRAM_Double
 * - VolumeRAM_2xFloat, VolumeRAM_2xDouble
 * - VolumeRAM_3xFloat, VolumeRAM_3xDouble
 * - VolumeRAM_4xFloat, VolumeRAM_4xDouble
 */
class PortConditionVolumeTypeReal : public PortCondition {
public:
    PortConditionVolumeTypeReal();

    virtual bool acceptsPortData() const;

protected:
    virtual void setCheckedPort(const Port* checkedPort);
    PortConditionLogicalOr orCondition_;
};

// ------------------------------------------------------------------------------------------------

/**
 * Port condition that allows integer volume types:
 * - VolumeRAM_UInt8, VolumeRAM_Int8
 * - VolumeRAM_UInt16, VolumeRAM_Int16
 * - VolumeRAM_UInt32, VolumeRAM_Int32
 * - VolumeRAM_UInt64, VolumeRAM_Int64
 * - VolumeRAM_2xUInt8, VolumeRAM_2xInt8
 * - VolumeRAM_2xUInt16, VolumeRAM_2xInt16
 * - VolumeRAM_2xUInt32, VolumeRAM_2xInt32
 * - VolumeRAM_2xUInt64, VolumeRAM_2xInt64
 * - VolumeRAM_3xUInt8, VolumeRAM_3xInt8
 * - VolumeRAM_3xUInt16, VolumeRAM_3xInt16
 * - VolumeRAM_3xUInt32, VolumeRAM_3xInt32
 * - VolumeRAM_3xUInt64, VolumeRAM_3xInt64
 * - VolumeRAM_4xUInt8, VolumeRAM_4xInt8
 * - VolumeRAM_4xUInt16, VolumeRAM_4xInt16
 * - VolumeRAM_4xUInt32, VolumeRAM_4xInt32
 * - VolumeRAM_4xUInt64, VolumeRAM_4xInt64
 */
class PortConditionVolumeTypeInteger : public PortCondition {
public:
    PortConditionVolumeTypeInteger();

    virtual bool acceptsPortData() const;

protected:
    virtual void setCheckedPort(const Port* checkedPort);
    PortConditionLogicalOr orCondition_;
};

// ------------------------------------------------------------------------------------------------

/**
 * Port condition that allows volumes with a specific number of channels.
 */
class PortConditionVolumeChannelCount : public PortCondition {
public:
    PortConditionVolumeChannelCount(size_t numChannels);

    virtual bool acceptsPortData() const;

    virtual std::string getDescription() const;

protected:
    virtual void setCheckedPort(const Port* checkedPort);

    const VolumePort* volumePort_;
    size_t numChannels_;
};


// ------------------------------------------------------------------------------------------------
// concrete subtypes of PortConditionVolumeType

// single channel

class PortConditionVolumeTypeUInt8 : public PortConditionVolumeType<VolumeRAM_UInt8> {
public:
    PortConditionVolumeTypeUInt8() :
      PortConditionVolumeType<VolumeRAM_UInt8>("VolumeRAM_UInt8") {};
};

class PortConditionVolumeTypeInt8 : public PortConditionVolumeType<VolumeRAM_Int8> {
public:
    PortConditionVolumeTypeInt8() :
      PortConditionVolumeType<VolumeRAM_Int8>("VolumeRAM_Int8") {};
};

class PortConditionVolumeTypeUInt16 : public PortConditionVolumeType<VolumeRAM_UInt16> {
public:
    PortConditionVolumeTypeUInt16() :
      PortConditionVolumeType<VolumeRAM_UInt16>("VolumeRAM_UInt16") {};
};

class PortConditionVolumeTypeInt16 : public PortConditionVolumeType<VolumeRAM_Int16> {
public:
    PortConditionVolumeTypeInt16() :
      PortConditionVolumeType<VolumeRAM_Int16>("VolumeRAM_Int16") {};
};

class PortConditionVolumeTypeUInt32 : public PortConditionVolumeType<VolumeRAM_UInt32> {
public:
    PortConditionVolumeTypeUInt32() :
      PortConditionVolumeType<VolumeRAM_UInt32>("VolumeRAM_UInt32") {};
};

class PortConditionVolumeTypeInt32 : public PortConditionVolumeType<VolumeRAM_Int32> {
public:
    PortConditionVolumeTypeInt32() :
      PortConditionVolumeType<VolumeRAM_Int32>("VolumeRAM_Int32") {};
};

class PortConditionVolumeTypeUInt64 : public PortConditionVolumeType<VolumeRAM_UInt64> {
public:
    PortConditionVolumeTypeUInt64() :
      PortConditionVolumeType<VolumeRAM_UInt64>("VolumeRAM_UInt64") {};
};

class PortConditionVolumeTypeInt64 : public PortConditionVolumeType<VolumeRAM_Int64> {
public:
    PortConditionVolumeTypeInt64() :
      PortConditionVolumeType<VolumeRAM_Int64>("VolumeRAM_Int64") {};
};

class PortConditionVolumeTypeFloat : public PortConditionVolumeType<VolumeRAM_Float> {
public:
    PortConditionVolumeTypeFloat() :
      PortConditionVolumeType<VolumeRAM_Float>("VolumeRAM_Float") {};
};

class PortConditionVolumeTypeDouble : public PortConditionVolumeType<VolumeRAM_Double> {
public:
    PortConditionVolumeTypeDouble() :
      PortConditionVolumeType<VolumeRAM_Double>("VolumeRAM_Double") {};
};

// two channels

class PortConditionVolumeType2xUInt8 : public PortConditionVolumeType<VolumeRAM_2xUInt8> {
public:
    PortConditionVolumeType2xUInt8() :
      PortConditionVolumeType<VolumeRAM_2xUInt8>("VolumeRAM_2xUInt8") {};
};

class PortConditionVolumeType2xInt8 : public PortConditionVolumeType<VolumeRAM_2xInt8> {
public:
    PortConditionVolumeType2xInt8() :
      PortConditionVolumeType<VolumeRAM_2xInt8>("VolumeRAM_2xInt8") {};
};

class PortConditionVolumeType2xUInt16 : public PortConditionVolumeType<VolumeRAM_2xUInt16> {
public:
    PortConditionVolumeType2xUInt16() :
      PortConditionVolumeType<VolumeRAM_2xUInt16>("VolumeRAM_2xUInt16") {};
};

class PortConditionVolumeType2xInt16 : public PortConditionVolumeType<VolumeRAM_2xInt16> {
public:
    PortConditionVolumeType2xInt16() :
      PortConditionVolumeType<VolumeRAM_2xInt16>("VolumeRAM_2xInt16") {};
};

class PortConditionVolumeType2xUInt32 : public PortConditionVolumeType<VolumeRAM_2xUInt32> {
public:
    PortConditionVolumeType2xUInt32() :
      PortConditionVolumeType<VolumeRAM_2xUInt32>("VolumeRAM_2xUInt32") {};
};

class PortConditionVolumeType2xInt32 : public PortConditionVolumeType<VolumeRAM_2xInt32> {
public:
    PortConditionVolumeType2xInt32() :
      PortConditionVolumeType<VolumeRAM_2xInt32>("VolumeRAM_2xInt32") {};
};

class PortConditionVolumeType2xUInt64 : public PortConditionVolumeType<VolumeRAM_2xUInt64> {
public:
    PortConditionVolumeType2xUInt64() :
      PortConditionVolumeType<VolumeRAM_2xUInt64>("VolumeRAM_2xUInt64") {};
};

class PortConditionVolumeType2xInt64 : public PortConditionVolumeType<VolumeRAM_2xInt64> {
public:
    PortConditionVolumeType2xInt64() :
      PortConditionVolumeType<VolumeRAM_2xInt64>("VolumeRAM_2xInt64") {};
};

class PortConditionVolumeType2xFloat : public PortConditionVolumeType<VolumeRAM_2xFloat> {
public:
    PortConditionVolumeType2xFloat() :
      PortConditionVolumeType<VolumeRAM_2xFloat>("VolumeRAM_2xFloat") {};
};

class PortConditionVolumeType2xDouble : public PortConditionVolumeType<VolumeRAM_2xDouble> {
public:
    PortConditionVolumeType2xDouble() :
      PortConditionVolumeType<VolumeRAM_2xDouble>("VolumeRAM_2xDouble") {};
};

// three channels

class PortConditionVolumeType3xUInt8 : public PortConditionVolumeType<VolumeRAM_3xUInt8> {
public:
    PortConditionVolumeType3xUInt8() :
      PortConditionVolumeType<VolumeRAM_3xUInt8>("VolumeRAM_3xUInt8") {};
};

class PortConditionVolumeType3xInt8 : public PortConditionVolumeType<VolumeRAM_3xInt8> {
public:
    PortConditionVolumeType3xInt8() :
      PortConditionVolumeType<VolumeRAM_3xInt8>("VolumeRAM_3xInt8") {};
};

class PortConditionVolumeType3xUInt16 : public PortConditionVolumeType<VolumeRAM_3xUInt16> {
public:
    PortConditionVolumeType3xUInt16() :
      PortConditionVolumeType<VolumeRAM_3xUInt16>("VolumeRAM_3xUInt16") {};
};

class PortConditionVolumeType3xInt16 : public PortConditionVolumeType<VolumeRAM_3xInt16> {
public:
    PortConditionVolumeType3xInt16() :
      PortConditionVolumeType<VolumeRAM_3xInt16>("VolumeRAM_3xInt16") {};
};

class PortConditionVolumeType3xUInt32 : public PortConditionVolumeType<VolumeRAM_3xUInt32> {
public:
    PortConditionVolumeType3xUInt32() :
      PortConditionVolumeType<VolumeRAM_3xUInt32>("VolumeRAM_3xUInt32") {};
};

class PortConditionVolumeType3xInt32 : public PortConditionVolumeType<VolumeRAM_3xInt32> {
public:
    PortConditionVolumeType3xInt32() :
      PortConditionVolumeType<VolumeRAM_3xInt32>("VolumeRAM_3xInt32") {};
};

class PortConditionVolumeType3xUInt64 : public PortConditionVolumeType<VolumeRAM_3xUInt64> {
public:
    PortConditionVolumeType3xUInt64() :
      PortConditionVolumeType<VolumeRAM_3xUInt64>("VolumeRAM_3xUInt64") {};
};

class PortConditionVolumeType3xInt64 : public PortConditionVolumeType<VolumeRAM_3xInt64> {
public:
    PortConditionVolumeType3xInt64() :
      PortConditionVolumeType<VolumeRAM_3xInt64>("VolumeRAM_3xInt64") {};
};

class PortConditionVolumeType3xFloat : public PortConditionVolumeType<VolumeRAM_3xFloat> {
public:
    PortConditionVolumeType3xFloat() :
      PortConditionVolumeType<VolumeRAM_3xFloat>("VolumeRAM_3xFloat") {};
};

class PortConditionVolumeType3xDouble : public PortConditionVolumeType<VolumeRAM_3xDouble> {
public:
    PortConditionVolumeType3xDouble() :
      PortConditionVolumeType<VolumeRAM_3xDouble>("VolumeRAM_3xDouble") {};
};

// four channels

class PortConditionVolumeType4xUInt8 : public PortConditionVolumeType<VolumeRAM_4xUInt8> {
public:
    PortConditionVolumeType4xUInt8() :
      PortConditionVolumeType<VolumeRAM_4xUInt8>("VolumeRAM_4xUInt8") {};
};

class PortConditionVolumeType4xInt8 : public PortConditionVolumeType<VolumeRAM_4xInt8> {
public:
    PortConditionVolumeType4xInt8() :
      PortConditionVolumeType<VolumeRAM_4xInt8>("VolumeRAM_4xInt8") {};
};

class PortConditionVolumeType4xUInt16 : public PortConditionVolumeType<VolumeRAM_4xUInt16> {
public:
    PortConditionVolumeType4xUInt16() :
      PortConditionVolumeType<VolumeRAM_4xUInt16>("VolumeRAM_4xUInt16") {};
};

class PortConditionVolumeType4xInt16 : public PortConditionVolumeType<VolumeRAM_4xInt16> {
public:
    PortConditionVolumeType4xInt16() :
      PortConditionVolumeType<VolumeRAM_4xInt16>("VolumeRAM_4xInt16") {};
};

class PortConditionVolumeType4xUInt32 : public PortConditionVolumeType<VolumeRAM_4xUInt32> {
public:
    PortConditionVolumeType4xUInt32() :
      PortConditionVolumeType<VolumeRAM_4xUInt32>("VolumeRAM_4xUInt32") {};
};

class PortConditionVolumeType4xInt32 : public PortConditionVolumeType<VolumeRAM_4xInt32> {
public:
    PortConditionVolumeType4xInt32() :
      PortConditionVolumeType<VolumeRAM_4xInt32>("VolumeRAM_4xInt32") {};
};

class PortConditionVolumeType4xUInt64 : public PortConditionVolumeType<VolumeRAM_4xUInt64> {
public:
    PortConditionVolumeType4xUInt64() :
      PortConditionVolumeType<VolumeRAM_4xUInt64>("VolumeRAM_4xUInt64") {};
};

class PortConditionVolumeType4xInt64 : public PortConditionVolumeType<VolumeRAM_4xInt64> {
public:
    PortConditionVolumeType4xInt64() :
      PortConditionVolumeType<VolumeRAM_4xInt64>("VolumeRAM_4xInt64") {};
};

class PortConditionVolumeType4xFloat : public PortConditionVolumeType<VolumeRAM_4xFloat> {
public:
    PortConditionVolumeType4xFloat() :
      PortConditionVolumeType<VolumeRAM_4xFloat>("VolumeRAM_4xFloat") {};
};

class PortConditionVolumeType4xDouble : public PortConditionVolumeType<VolumeRAM_4xDouble> {
public:
    PortConditionVolumeType4xDouble() :
      PortConditionVolumeType<VolumeRAM_4xDouble>("VolumeRAM_4xDouble") {};
};


// -----------------------------------------------------------------------------------------------
// template definitions

template<typename T>
voreen::PortConditionVolumeType<T>::PortConditionVolumeType(const std::string& volTypeName)
    : PortCondition("Volume of type " + volTypeName + " expected")
{}

template<typename T>
voreen::PortConditionVolumeType<T>::~PortConditionVolumeType()
{}

template<typename T>
bool voreen::PortConditionVolumeType<T>::acceptsPortData() const  {
    if (!volumePort_ || !volumePort_->hasData())
        return false;

    const VolumeRAM* volume = volumePort_->getData()->getRepresentation<VolumeRAM>();
    return (dynamic_cast<const T*>(volume));
}

template<typename T>
void voreen::PortConditionVolumeType<T>::setCheckedPort(const Port* checkedPort) {
    if (!dynamic_cast<const VolumePort*>(checkedPort)) {
        LERRORC("voreen.PortConditionVolumeType", "Assigned port is not a volume port");
    }
    else {
        volumePort_ = static_cast<const VolumePort*>(checkedPort);
    }
}

} // namespace

#endif // VRN_PORTCONDITION_VOLUMETYPE_H
