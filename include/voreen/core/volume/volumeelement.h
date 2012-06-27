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

#ifndef VRN_VOLUMEELEMENT
#define VRN_VOLUMEELEMENT

#include <limits>

// Note: please ensure that no OpenGL dependencies are added into this file

namespace voreen {

namespace {
/**
 * Used internally by VolumeElement.
 */
template<class T>
T getUpperLimit() {
    return std::numeric_limits<T>::max();
}

template<>
float getUpperLimit() {
    return 1.f;
}

template<>
double getUpperLimit() {
    return 1.0;
}
}

/**
 * Helper class for getting sth like the maximum value
 */
template<class T>
class VolumeElement {
public:
    inline static T getZero();

    inline static T getChannel(const T& t, size_t channel);
    inline static void setChannel(T value, T& t, size_t channel);

    inline static int max();

    /**
    * Returns the maximum value an element can have. In contrast to max() no
    * vector is returned but the maximum value an element in the vector can have.
    */
    inline static int maxElement();

    inline static int getNumChannels();

    /**
    * Calculates the squared difference between val1 and val2.
    */
    static double calcSquaredDifference(T val1, T val2);

    typedef double DoubleType;
    typedef T BaseType;
};

template<class T>
class VolumeElement<tgt::Vector3<T> > {
public:
    inline static tgt::Vector3<T> getZero();

    inline static T getChannel(const tgt::Vector3<T>& t, size_t channel);
    inline static void setChannel(T value, tgt::Vector3<T>& t, size_t channel);

    inline static tgt::ivec3 max();

    /**
    * Returns the maximum value an element can have. In contrast to max() no
    * vector is returned but the maximum value an element in the vector can have.
    */
    inline static int maxElement();

    inline static int getNumChannels();

    /**
    * Calculates the squared difference between val1 and val2. Since val1 and val2 are 3-dimensional
    * vectors, each squared difference is multiplied by 1/3.
    */
    static double calcSquaredDifference(tgt::Vector3<T> val1, tgt::Vector3<T> val2);

    typedef tgt::Vector3<double> DoubleType;
    typedef T BaseType;
};

template<class T>
class VolumeElement<tgt::Vector4<T> > {
public:
    inline static tgt::Vector4<T> getZero();

    inline static T getChannel(const tgt::Vector4<T>& t, size_t channel);

    inline static void setChannel(T value, tgt::Vector4<T>& t, size_t channel);

    inline static tgt::ivec4 max();

    /**
    * Returns the maximum value an element can have. In contrast to max() no
    * vector is returned but the maximum value an element in the vector can have.
    */
    inline static int maxElement();

    inline static int getNumChannels();

    /**
    * Calculates the squared difference between val1 and val2. Since val1 and val2 are 4-dimensional
    * vectors, each squared difference is multiplied by 1/4.
    */
    static double calcSquaredDifference(tgt::Vector4<T> val1, tgt::Vector4<T> val2);

    typedef tgt::Vector4<double> DoubleType;
    typedef T BaseType;
};


// ==============
// implementation
// ==============

template<class T>
inline T VolumeElement<T>::getZero() {
    return T(0);
}

template<class T>
#ifdef VRN_DEBUG
inline T VolumeElement<T>::getChannel(const T& t, size_t channel) {
    tgtAssert(channel == 0, "channel must be zero");
#else
inline  T VolumeElement<T>::getChannel(const T& t, size_t /*channel*/) {
#endif
    return t;
}

template<class T>
#ifdef VRN_DEBUG
inline void VolumeElement<T>::setChannel(T value, T& t, size_t channel) {
    tgtAssert(channel == 0, "channel must be zero");
#else
inline void VolumeElement<T>::setChannel(T value, T& t, size_t /*channel*/) {
#endif
    t = value;
}

template<class T>
inline int VolumeElement<T>::max() {
    return static_cast<int>( getUpperLimit<T>() );
}

/**
* Returns the maximum value an element can have. In contrast to max() no
* vector is returned but the maximum value an element in the vector can have.
*/
template<class T>
inline int VolumeElement<T>::maxElement() {
    return static_cast<int>( getUpperLimit<T>() );
}

template<class T>
inline int VolumeElement<T>::getNumChannels() {
    return 1;
}

/**
* Calculates the squared difference between val1 and val2.
*/
template<class T>
double VolumeElement<T>::calcSquaredDifference(T val1, T val2) {
    double result;

    result = static_cast<double>(val1) - static_cast<double>(val2);
    result = result * result;

    return result;
}

template<class T>
inline tgt::Vector3<T> VolumeElement<tgt::Vector3<T> >::getZero() {
    return tgt::Vector3<T>(T(0), T(0), T(0));
}

template<class T>
inline T VolumeElement<tgt::Vector3<T> >::getChannel(const tgt::Vector3<T>& t, size_t channel) {
    tgtAssert(channel < 3, "channel must be less than 3");
    if (channel < 3)
        return t[channel];
    else
        return 0;
}

template<class T>
inline void VolumeElement<tgt::Vector3<T> >::setChannel(T value, tgt::Vector3<T>& t, size_t channel) {
    tgtAssert(channel < 3, "channel must be less than 3");
    t[channel] = value;
}

template<class T>
inline tgt::ivec3 VolumeElement<tgt::Vector3<T> >::max() {
    return tgt::ivec3(getUpperLimit<T>());
}

/**
* Returns the maximum value an element can have. In contrast to max() no
* vector is returned but the maximum value an element in the vector can have.
*/
template<class T>
inline int VolumeElement<tgt::Vector3<T> >::maxElement() {
    tgt::ivec3 temp = tgt::ivec3(static_cast<int>(getUpperLimit<T>()));
    return temp.x;
}

template<class T>
inline int VolumeElement<tgt::Vector3<T> >::getNumChannels() {
    return 3;
}

/**
* Calculates the squared difference between val1 and val2. Since val1 and val2 are 3-dimensional
* vectors, each squared difference is multiplied by 1/3.
*/
template<class T>
double VolumeElement<tgt::Vector3<T> >::calcSquaredDifference(tgt::Vector3<T> val1, tgt::Vector3<T> val2) {
    double result;
    tgt::Vector3d difference;

    difference.x = (double)val1.x - (double)val2.x;
    difference.y = (double)val1.y - (double)val2.y;
    difference.z = (double)val1.z - (double)val2.z;

    difference.x = difference.x * difference.x;
    difference.y = difference.y * difference.y;
    difference.z = difference.z * difference.z;

    result = difference.x + difference.y + difference.z;

    return result;
}

template<class T>
inline tgt::Vector4<T> VolumeElement<tgt::Vector4<T> >::getZero() {
    return tgt::Vector4<T>(T(0), T(0), T(0), T(0));
}

template<class T>
inline T VolumeElement<tgt::Vector4<T> >::getChannel(const tgt::Vector4<T>& t, size_t channel) {
    tgtAssert(channel < 4, "channel must be less than 4");
    if (channel < 4)
        return t[channel];
    else
        return 0;
}

template<class T>
inline void VolumeElement<tgt::Vector4<T> >::setChannel(T value, tgt::Vector4<T>& t, size_t channel) {
    tgtAssert(channel < 4, "channel must be less than 4");
    t[channel] = value;
}

template<class T>
inline tgt::ivec4 VolumeElement<tgt::Vector4<T> >::max() {
    return ivec4(getUpperLimit<T>());
}

/**
* Returns the maximum value an element can have. In contrast to max() no
* vector is returned but the maximum value an element in the vector can have.
*/
template<class T>
inline int VolumeElement<tgt::Vector4<T> >::maxElement() {
    tgt::ivec4 temp = tgt::ivec4( static_cast<int>(getUpperLimit<T>() ) );
    return temp.x;
}

template<class T>
inline int VolumeElement<tgt::Vector4<T> >::getNumChannels() {
    return 4;
}

/**
* Calculates the squared difference between val1 and val2. Since val1 and val2 are 4-dimensional
* vectors, each squared difference is multiplied by 1/4.
*/
template<class T>
double VolumeElement<tgt::Vector4<T> >::calcSquaredDifference(tgt::Vector4<T> val1, tgt::Vector4<T> val2) {
    double result;
    tgt::Vector4d difference;

    difference.x = static_cast<double>(val1.x) - static_cast<double>(val2.x);
    difference.y = static_cast<double>(val1.y) - static_cast<double>(val2.y);
    difference.z = static_cast<double>(val1.z) - static_cast<double>(val2.z);
    difference.w = static_cast<double>(val1.w) - static_cast<double>(val2.w);

    difference.x = difference.x*difference.x;
    difference.y = difference.y*difference.y;
    difference.z = difference.z*difference.z;
    difference.w = difference.w*difference.w;

    result = difference.x + difference.y + difference.z + difference.w;

    return result;
}


} // namespace voreen

#endif // VRN_VOLUMEELEMENT
