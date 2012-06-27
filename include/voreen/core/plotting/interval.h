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

#ifndef VRN_INTERVAL_H
#define VRN_INTERVAL_H

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

class VoreenException;
/**
 * \brief templated class for handling intervals of numeric types
 *
 * Offers support for common interval arithmetic.
 **/
template<typename T>
class Interval : public Serializable  {
public:
    /// default constructor (creates the empty interval (0,0))
    Interval();

    /// copy constructor
    Interval(const Interval<T>& src);

    /// constructor
    Interval(T left, T right, bool leftOpen=false, bool rightOpen=false);

    /// destructor
    virtual ~Interval();

    /// returns left interval border
    inline const T& getLeft() const;
    /// returns right interval border
    inline const T& getRight() const;

    /// returns whether left interval border ist open
    inline bool getLeftOpen() const;
    /// returns whether right interval border ist open
    inline bool getRightOpen() const;

    /// returns whether interval is empty
    bool empty() const;

    /// returns the size of the interval (right - left)
    T size() const;

    /// returns whether the interval contains the value \a value
    bool contains(const T& value) const;
    /// returns whether the interval contains the interval \a interval
    bool contains(const Interval<T>& interval) const;


    /// returns the intersection of this interval with \a rhs
    Interval<T> intersectWith(const Interval<T>& rhs) const;

    /// returns the smallest interval containing both intervals *this and \a rhs
    /// if one of these intervals is empty, the other is returned
    Interval<T> unionWith(const Interval<T>& rhs) const;


    /// returns a new interval which is the current interval enlarged by factor
    Interval<T> enlarge(const T& factor) const;

    /// returns a new interval which is the current interval enlarged as much as it contains \a value
    Interval<T> nibble(const T& value) const;

    /// returns \a value clamped to current interval
    T clampValue(const T& value) const;


    /**
    * @see Property::serialize
    */
    virtual void serialize(XmlSerializer& s) const;

    /**
    * @see Property::deserialize
    */
    virtual void deserialize(XmlDeserializer& s);


    /// returns an interval whose borders are incremented by \a rhs
    Interval<T> operator+(const T& rhs) const;
    /// returns an interval whose borders are decremented by \a rhs
    Interval<T> operator-(const T& rhs) const;
    /// returns an interval whose borders are multiplied by \a rhs
    Interval<T> operator*(const T& rhs) const;
    /// returns an interval whose borders are divided by \a rhs
    Interval<T> operator/(const T& rhs) const throw (VoreenException);


    bool operator==(const Interval<T>& rhs) const;
    bool operator!=(const Interval<T>& rhs) const;

private:
    T left_;                ///< left interval border
    T right_;               ///< right interval border

    bool leftOpen_;         ///< flag whether left interval border is open
    bool rightOpen_;        ///< flag whether right interval border is open

};



// ---------------------------------------- implementation ----------------------------------------

template<typename T>
Interval<T>::Interval()
    : Serializable()
    , left_(0)
    , right_(0)
    , leftOpen_(true)
    , rightOpen_(true)
{}

template<typename T>
Interval<T>::Interval(const Interval<T>& src)
    : Serializable()
    , left_(src.left_)
    , right_(src.right_)
    , leftOpen_(src.leftOpen_)
    , rightOpen_(src.rightOpen_)
{}


template<typename T>
Interval<T>::Interval(T left, T right, bool leftOpen, bool rightOpen)
    : left_(left)
    , right_(right)
    , leftOpen_(leftOpen)
    , rightOpen_(rightOpen)
{}

template<typename T>
Interval<T>::~Interval() {
}

template<typename T>
const T& Interval<T>::getLeft() const {
    return left_;
}

template<typename T>
const T& Interval<T>::getRight() const {
    return right_;
}

template<typename T>
bool Interval<T>::getLeftOpen() const {
    return leftOpen_;
}

template<typename T>
bool Interval<T>::getRightOpen() const {
    return rightOpen_;
}

template<typename T>
T Interval<T>::size() const {
    return right_ - left_;
}

template<typename T>
bool Interval<T>::empty() const {
    if (left_ > right_)
        return true;
    if (left_ == right_)
        return (leftOpen_ || rightOpen_);
    return false;
}

template<typename T>
bool Interval<T>::contains(const T& value) const {
    if ((value < left_) || (leftOpen_ && value == left_))
        return false;
    if ((value > right_) || (rightOpen_ && value == right_))
        return false;
    return true;
}

template<typename T>
bool Interval<T>::contains(const Interval<T>& rhs) const {
    if ((left_ > rhs.left_) || ((leftOpen_ || rhs.leftOpen_) && left_ == rhs.left_))
        return false;
    if ((right_ < rhs.right_) || ((rightOpen_ || rhs.rightOpen_) && right_ == rhs.right_))
        return false;
    return true;
}

template<typename T>
Interval<T> Interval<T>::intersectWith(const Interval<T>& rhs) const {
    T nl, nr;
    bool nlo, nro;

    if (left_ < rhs.left_) {
        nl = rhs.left_;
        nlo = rhs.leftOpen_;
    }
    else if (left_ == rhs.left_) {
        nl = left_;
        nlo = leftOpen_ || rhs.leftOpen_;
    }
    else {
        nl = left_;
        nlo = leftOpen_;
    }

    if (right_ > rhs.right_) {
        nr = rhs.right_;
        nro = rhs.rightOpen_;
    }
    else if (right_ == rhs.right_) {
        nr = right_;
        nro = rightOpen_ || rhs.rightOpen_;
    }
    else {
        nr = right_;
        nro = rightOpen_;
    }

    return Interval<T>(nl, nr, nlo, nro);
}


template<typename T>
Interval<T> Interval<T>::unionWith(const Interval<T>& rhs) const{
    // check if one of the intervals is empty
    if (empty())
        return Interval<T>(rhs);
    if (rhs.empty())
        return Interval<T>(*this);

    T nl, nr;
    bool nlo, nro;

    if (left_ < rhs.left_) {
        nl = left_;
        nlo = leftOpen_;
    }
    else if (left_ == rhs.left_) {
        nl = left_;
        nlo = leftOpen_ && rhs.leftOpen_;
    }
    else {
        nl = rhs.left_;
        nlo = rhs.leftOpen_;
    }

    if (right_ > rhs.right_) {
        nr = right_;
        nro = rightOpen_;
    }
    else if (right_ == rhs.right_) {
        nr = right_;
        nro = rightOpen_ && rhs.rightOpen_;
    }
    else {
        nr = rhs.right_;
        nro = rhs.rightOpen_;
    }

    return Interval<T>(nl, nr, nlo, nro);
}

template<typename T>
void Interval<T>::serialize(XmlSerializer& s) const {
    s.serialize("left", left_);
    s.serialize("right", right_);
    s.serialize("leftOpen", leftOpen_);
    s.serialize("rightOpen", rightOpen_);
}

template<typename T>
void Interval<T>::deserialize(XmlDeserializer& s) {
    s.deserialize("left", left_);
    s.deserialize("right", right_);
    s.deserialize("leftOpen", leftOpen_);
    s.deserialize("rightOpen", rightOpen_);
}

template<typename T>
Interval<T> Interval<T>::operator+(const T& rhs) const {
    return Interval<T>(left_+rhs, right_+rhs, leftOpen_, rightOpen_);
}

template<typename T>
Interval<T> Interval<T>::operator-(const T& rhs) const {
    return Interval<T>(left_-rhs, right_-rhs, leftOpen_, rightOpen_);
}

template<typename T>
Interval<T> Interval<T>::operator*(const T& rhs) const {
    return Interval<T>(left_*rhs, right_*rhs, leftOpen_, rightOpen_);
}

template<typename T>
Interval<T> Interval<T>::operator/(const T& rhs) const throw (VoreenException) {
    if (rhs == 0)
        throw VoreenException("division with zero");

    return Interval<T>(left_/rhs, right_/rhs, leftOpen_, rightOpen_);
}

template<typename T>
Interval<T> Interval<T>::enlarge(const T& factor) const {
    T length = right_-left_;
    T offset = length*(factor-1.0)/2.0;
    return Interval(left_-offset,right_+offset,leftOpen_,rightOpen_);
}

template<typename T>
Interval<T> Interval<T>::nibble(const T& value) const {
    if (empty())
        return Interval(value, value, false, false);
    else if (contains(value))
        return *this;
    else if (leftOpen_ && left_ == value)
        return Interval(left_, right_, false, rightOpen_);
    else if (rightOpen_ && right_ == value)
        return Interval(left_, right_, leftOpen_, false);
    else if (left_ > value)
        return Interval(value, right_, false,rightOpen_);
    else // (right_ < value)
        return Interval(left_, value, leftOpen_,false);
}

template<typename T>
T Interval<T>::clampValue(const T& value) const {
    if (value < left_)
        return left_;
    else if (value > right_)
        return right_;
    else // contains(value)
        return value;
}

template<typename T>
bool Interval<T>::operator==(const Interval<T>& rhs) const {
    return (left_ == rhs.left_
         && right_ == rhs.right_
         && leftOpen_ == rhs.leftOpen_
         && rightOpen_ == rhs.rightOpen_);
}

template<typename T>
bool Interval<T>::operator!=(const Interval<T>& rhs) const {
    return (left_ != rhs.left_
         || right_ != rhs.right_
         || leftOpen_ != rhs.leftOpen_
         || rightOpen_ != rhs.rightOpen_);
}

}// namespace voreen


#endif // VRN_INTERVAL_H
