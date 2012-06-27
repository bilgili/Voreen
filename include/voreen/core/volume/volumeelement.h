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

/**
 * Used internally by VolumeElement.
 */
template<class T>
class UpperLimit {
public:
    inline static T get() {
        return std::numeric_limits<T>::max();
    }
};

template<>
class UpperLimit<float> {
public:
    inline static float get() {
        return 1.f;
    }
};

template<>
class UpperLimit<double> {
public:
    inline static double get() {
        return 1.0;
    }
};

/**
 * Helper class for getting sth like the maximum value
 */
template<class T>
class VolumeElement {
public:
    inline static T getZero() {
        return T(0);
    }

#ifdef VRN_DEBUG
    inline static T getChannel(const T& t, size_t channel) {
        tgtAssert(channel == 0, "channel must be zero");
#else
    inline static T getChannel(const T& t, size_t /*channel*/) {
#endif
        return t;
    }

#ifdef VRN_DEBUG
    inline static void setChannel(T value, T& t, size_t channel) {
        tgtAssert(channel == 0, "channel must be zero");
#else
    inline static void setChannel(T value, T& t, size_t /*channel*/) {
#endif
        t = value;
    }

    inline static int max() {
        return static_cast<int>( UpperLimit<T>::get() );
    }

    inline static int getNumChannels() {
        return 1;
    }

    typedef double DoubleType;
    typedef T BaseType;
};

template<class T>
class VolumeElement<tgt::Vector3<T> > {
public:
    inline static tgt::Vector3<T> getZero() {
        return tgt::Vector3<T>(T(0), T(0), T(0));
    }

    inline static T getChannel(const tgt::Vector3<T>& t, size_t channel) {
        tgtAssert(channel < 3, "channel must be less than 3");
        return t[channel];
    }

    inline static void setChannel(T value, tgt::Vector3<T>& t, size_t channel) {
        tgtAssert(channel < 3, "channel must be less than 3");
        t[channel] = value;
    }

    inline static tgt::ivec3 max() {
        return ivec3( UpperLimit<T>::get() );
    }

    inline static int getNumChannels() {
        return 3;
    }

    typedef tgt::Vector3<double> DoubleType;
    typedef T BaseType;
};

template<class T>
class VolumeElement<tgt::Vector4<T> > {
public:
    inline static tgt::Vector4<T> getZero() {
        return tgt::Vector4<T>(T(0), T(0), T(0), T(0));
    }

    inline static T getChannel(const tgt::Vector4<T>& t, size_t channel) {
        tgtAssert(channel < 4, "channel must be less than 4");
        return t[channel];
    }

    inline static void setChannel(T value, tgt::Vector4<T>& t, size_t channel) {
        tgtAssert(channel < 4, "channel must be less than 4");
        t[channel] = value;
    }

    inline static tgt::ivec4 max() {
        return ivec4( UpperLimit<T>::get() );
    }

    inline static int getNumChannels() {
        return 4;
    }

    typedef tgt::Vector4<double> DoubleType;
    typedef T BaseType;
};

} // namespace voreen

#endif // VRN_VOLUMEELEMENT
