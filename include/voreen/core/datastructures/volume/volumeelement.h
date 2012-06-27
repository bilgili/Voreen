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
T getTypeUpperLimit() {
    return std::numeric_limits<T>::max();
}

template<class T>
T getTypeLowerLimit() {
    return std::numeric_limits<T>::min();
}

template<class T>
bool isTypeSigned() {
    return std::numeric_limits<T>::is_signed;
}

template<class T>
bool isTypeBounded() {
    return std::numeric_limits<T>::is_bounded;
}

template<>
float getTypeUpperLimit() {
    return 1.f;
}

template<>
float getTypeLowerLimit() {
    return 0.f;
}

template<>
double getTypeUpperLimit() {
    return 1.0;
}

template<>
double getTypeLowerLimit() {
    return 0.0;
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

    /**
     * Returns whether the data type is signed.
     */
    inline bool static isSigned();

    /**
     * Returns whether the data type is bounded,
     * which is true for integer types and false for
     * floating point types.
     */
    inline bool static isBounded();

    /**
     * Returns the maximal possible value of T
     */
    inline static T rangeMax();

    /**
     * Returns the minimal possible value of T
     */
    inline static T rangeMin();

    /**
    * Returns the maximum value an element can have. In contrast to max() no
    * vector is returned but the maximum value an element in the vector can have.
    * Since we cannot make any assumption about the element's data type,
    * we have to use float as fall back.
    */
    inline static float rangeMaxElement();

    /**
     * Returns the minimum value an element can have.
     */
    inline static float rangeMinElement();

    /**
     * Extracts the minimal element from the passed value
     * and converts it to float.
     */
    inline static float getMinElement(const T& value);

    /**
     * Extracts the maximal element from the passed value
     * and converts it to float.
     */
    inline static float getMaxElement(const T& value);

    inline static int getNumChannels();

    /**
    * Calculates the squared difference between val1 and val2.
    */
    static double calcSquaredDifference(T val1, T val2);

    typedef double DoubleType;
    typedef T BaseType;
};

template<class T>
class VolumeElement<tgt::Vector2<T> > {
public:
    inline static tgt::Vector2<T> getZero();

    inline static T getChannel(const tgt::Vector2<T>& t, size_t channel);
    inline static void setChannel(T value, tgt::Vector2<T>& t, size_t channel);

    inline static bool isSigned();

    inline bool static isBounded();

    inline static tgt::Vector2<T> rangeMax();
    inline static tgt::Vector2<T> rangeMin();

    /**
    * Returns the maximum value an element can have. In contrast to max() no
    * vector is returned but the maximum value an element in the vector can have.
    */
    inline static float rangeMaxElement();
    inline static float rangeMinElement();

    inline static float getMaxElement(const tgt::Vector2<T>& value);
    inline static float getMinElement(const tgt::Vector2<T>& value);

    inline static int getNumChannels();

    /**
    * Calculates the squared difference between val1 and val2. Since val1 and val2 are 2-dimensional
    * vectors, each squared difference is multiplied by 1/2.
    */
    static double calcSquaredDifference(tgt::Vector2<T> val1, tgt::Vector2<T> val2);

    typedef tgt::Vector2<double> DoubleType;
    typedef T BaseType;
};

template<class T>
class VolumeElement<tgt::Vector3<T> > {
public:
    inline static tgt::Vector3<T> getZero();

    inline static T getChannel(const tgt::Vector3<T>& t, size_t channel);
    inline static void setChannel(T value, tgt::Vector3<T>& t, size_t channel);

    inline static bool isSigned();

    inline bool static isBounded();

    inline static tgt::Vector3<T> rangeMax();
    inline static tgt::Vector3<T> rangeMin();

    /**
    * Returns the maximum value an element can have. In contrast to max() no
    * vector is returned but the maximum value an element in the vector can have.
    */
    inline static float rangeMaxElement();
    inline static float rangeMinElement();

    inline static float getMaxElement(const tgt::Vector3<T>& value);
    inline static float getMinElement(const tgt::Vector3<T>& value);

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

    inline static bool isSigned();

    inline bool static isBounded();

    inline static tgt::Vector4<T> rangeMax();
    inline static tgt::Vector4<T> rangeMin();

    /**
    * Returns the maximum value an element can have. In contrast to max() no
    * vector is returned but the maximum value an element in the vector can have.
    */
    inline static float rangeMaxElement();
    inline static float rangeMinElement();

    inline static float getMaxElement(const tgt::Vector4<T>& value);
    inline static float getMinElement(const tgt::Vector4<T>& value);

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
inline bool VolumeElement<T>::isSigned() {
    return isTypeSigned<T>();
}

template<class T>
inline bool VolumeElement<T>::isBounded() {
    return isTypeBounded<T>();
}

template<class T>
inline T VolumeElement<T>::rangeMax() {
    return static_cast<T>( getTypeUpperLimit<T>() );
}

template<class T>
inline T VolumeElement<T>::rangeMin() {
    return static_cast<T>( getTypeLowerLimit<T>() );
}

/**
* Returns the maximum value an element can have. In contrast to max() no
* vector is returned but the maximum value an element in the vector can have.
*/
template<class T>
inline float VolumeElement<T>::rangeMaxElement() {
    return static_cast<float>( getTypeUpperLimit<T>() );
}

template<class T>
float voreen::VolumeElement<T>::getMaxElement(const T& value) {
    return static_cast<float>(value);
}

template<class T>
float voreen::VolumeElement<T>::getMinElement(const T& value) {
    return static_cast<float>(value);
}

template<class T>
inline float VolumeElement<T>::rangeMinElement() {
    return static_cast<float>( getTypeLowerLimit<T>() );
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

// --- Vector2 ----

template<class T>
inline tgt::Vector2<T> VolumeElement<tgt::Vector2<T> >::getZero() {
    return tgt::Vector2<T>(T(0), T(0));
}

template<class T>
inline T VolumeElement<tgt::Vector2<T> >::getChannel(const tgt::Vector2<T>& t, size_t channel) {
    tgtAssert(channel < 2, "channel must be less than 2");
    if (channel < 2)
        return t[channel];
    else
        return 0;
}

template<class T>
inline void VolumeElement<tgt::Vector2<T> >::setChannel(T value, tgt::Vector2<T>& t, size_t channel) {
    tgtAssert(channel < 2, "channel must be less than 2");
    t[channel] = value;
}

template<class T>
inline bool VolumeElement<tgt::Vector2<T> >::isSigned() {
    return isTypeSigned<T>();
}

template<class T>
inline bool VolumeElement<tgt::Vector2<T> >::isBounded() {
    return isTypeBounded<T>();
}

template<class T>
inline tgt::Vector2<T> VolumeElement<tgt::Vector2<T> >::rangeMax() {
    return tgt::Vector2<T>(getTypeUpperLimit<T>());
}

template<class T>
inline tgt::Vector2<T> VolumeElement<tgt::Vector2<T> >::rangeMin() {
    return tgt::Vector2<T>(getTypeLowerLimit<T>());
}

/**
* Returns the maximum value an element can have. In contrast to max() no
* vector is returned but the maximum value an element in the vector can have.
*/
template<class T>
inline float VolumeElement<tgt::Vector2<T> >::rangeMaxElement() {
    return static_cast<float>(getTypeUpperLimit<T>());
}

template<class T>
inline float VolumeElement<tgt::Vector2<T> >::rangeMinElement() {
    return static_cast<float>(getTypeLowerLimit<T>());
}

template<class T>
float voreen::VolumeElement<tgt::Vector2<T> >::getMaxElement(const tgt::Vector2<T>& value) {
    return static_cast<float>(value.x);
}

template<class T>
float voreen::VolumeElement<tgt::Vector2<T> >::getMinElement(const tgt::Vector2<T>& value) {
    return static_cast<float>(value.x);
}

template<class T>
inline int VolumeElement<tgt::Vector2<T> >::getNumChannels() {
    return 2;
}

/**
* Calculates the squared difference between val1 and val2. Since val1 and val2 are 3-dimensional
* vectors, each squared difference is multiplied by 1/3.
*/
template<class T>
double VolumeElement<tgt::Vector2<T> >::calcSquaredDifference(tgt::Vector2<T> val1, tgt::Vector2<T> val2) {
    double result;
    tgt::Vector2d difference;

    difference.x = (double)val1.x - (double)val2.x;
    difference.y = (double)val1.y - (double)val2.y;

    difference.x = difference.x * difference.x;
    difference.y = difference.y * difference.y;

    result = difference.x + difference.y;

    return result;
}

// --- Vector3 ----

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
inline bool VolumeElement<tgt::Vector3<T> >::isSigned() {
    return isTypeSigned<T>();
}

template<class T>
inline bool VolumeElement<tgt::Vector3<T> >::isBounded() {
    return isTypeBounded<T>();
}

template<class T>
inline tgt::Vector3<T> VolumeElement<tgt::Vector3<T> >::rangeMax() {
    return tgt::Vector3<T>(getTypeUpperLimit<T>());
}

template<class T>
inline tgt::Vector3<T> VolumeElement<tgt::Vector3<T> >::rangeMin() {
    return tgt::Vector3<T>(getTypeLowerLimit<T>());
}

/**
* Returns the maximum value an element can have. In contrast to max() no
* vector is returned but the maximum value an element in the vector can have.
*/
template<class T>
inline float VolumeElement<tgt::Vector3<T> >::rangeMaxElement() {
    return static_cast<float>(getTypeUpperLimit<T>());
}

template<class T>
inline float VolumeElement<tgt::Vector3<T> >::rangeMinElement() {
    return static_cast<float>(getTypeLowerLimit<T>());
}

template<class T>
float voreen::VolumeElement<tgt::Vector3<T> >::getMaxElement(const tgt::Vector3<T>& value) {
    return static_cast<float>(value.x);
}

template<class T>
float voreen::VolumeElement<tgt::Vector3<T> >::getMinElement(const tgt::Vector3<T>& value) {
    return static_cast<float>(value.y);
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

// --- Vector4 ----

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
inline bool VolumeElement<tgt::Vector4<T> >::isSigned() {
    return isTypeSigned<T>();
}

template<class T>
inline bool VolumeElement<tgt::Vector4<T> >::isBounded() {
    return isTypeBounded<T>();
}

template<class T>
inline tgt::Vector4<T> VolumeElement<tgt::Vector4<T> >::rangeMax() {
    return tgt::Vector4<T>(getTypeUpperLimit<T>());
}

template<class T>
inline tgt::Vector4<T> VolumeElement<tgt::Vector4<T> >::rangeMin() {
    return tgt::Vector4<T>(getTypeLowerLimit<T>());
}

/**
* Returns the maximum value an element can have. In contrast to max() no
* vector is returned but the maximum value an element in the vector can have.
*/
template<class T>
inline float VolumeElement<tgt::Vector4<T> >::rangeMaxElement() {
    return static_cast<float>(getTypeUpperLimit<T>());
}

template<class T>
inline float VolumeElement<tgt::Vector4<T> >::rangeMinElement() {
    return static_cast<float>(getTypeLowerLimit<T>());
}

template<class T>
float voreen::VolumeElement<tgt::Vector4<T> >::getMaxElement(const tgt::Vector4<T>& value) {
    return static_cast<float>(value.x);
}

template<class T>
float voreen::VolumeElement<tgt::Vector4<T> >::getMinElement(const tgt::Vector4<T>& value) {
    return static_cast<float>(value.x);
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
