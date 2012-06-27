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

#ifndef VRN_PLOTPREDICATE_H
#define VRN_PLOTPREDICATE_H

#include "voreen/core/io/serialization/serialization.h"
//#include "voreen/core/plotting/plotcell.h"
#include "voreen/core/plotting/interval.h"

namespace voreen {

class PlotCellValue;

/**
 * \brief Abstract super class for predicate classes used for selecting subsets out of PlotData tables
 *
 * Subclasses have to override check function applying the predicate at all given PlotCells and
 * returning a boolean indicating whether all PlotCells match the predicate or not.
 */
class PlotPredicate : public AbstractSerializable  {
friend class PlotPredicateFactory;
public:

    enum PlotPredicateEnum {
        NONE = 0,
        LESS = 10,
        EQUAL = 20,
        EQUALVECTOR = 21,
        GREATER = 30,
        BETWEEN = 40,
        ISSUBSTR = 50,
        ALPHANUMERIC = 60,
        NOTALPHANUMERIC = 65,
        EMPTY = 70,
        NOTEMPTY = 75
    };

    /// return a vector of the stored thresholds
    virtual std::vector<PlotCellValue> getThresholdValues() const = 0;

    /// checks whether value stored in PlotCell \a value fulfills the predicate
    virtual bool check(const PlotCellValue& value) const = 0;

    /// creates a deep copy of the current PlotPredicate
    virtual PlotPredicate* clone() const = 0;

    /// returns a string representation of this predicate
    virtual std::string toString() const = 0;
};

/**
 * \brief  Predicate which checks if values in all given PlotCells are less than the threshold
 *         given at instantiation.
 */
class PlotPredicateLess : public PlotPredicate {
friend class PlotPredicateFactory;
public:

    /**
     * \brief      Constructor: creates a new PlotPredicate checking whether a given value is less than
     *             specified threshols \a threshold
     *
     * \param threshold    threshold againts the given PlotCells shall be checked
     */
    PlotPredicateLess(plot_t threshold);
    PlotPredicateLess(std::string threshold);
    PlotPredicateLess(const PlotCellValue& threshold);

    /// return a vector of the stored thresholds
    virtual std::vector<PlotCellValue> getThresholdValues() const;

    /// checks whether value stored in PlotCell \a value fulfills the predicate
    bool check(const PlotCellValue& value) const;

    /// creates a deep copy of the current PlotPredicate
    virtual PlotPredicate* clone() const;

    /// returns a string representation of this predicate
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);


private:
    PlotPredicateLess();

    PlotCellValue threshold_;      ///< threshold againts the given PlotCells shall be checked
};


/**
 * \brief  Predicate which checks if values in all given PlotCells are less than the threshold
 *         given at instantiation.
 */
class PlotPredicateEqual : public PlotPredicate {
friend class PlotPredicateFactory;
public:

    /**
     * \brief      Constructor: creates a new PlotPredicate checking whether a given value is less than
     *             specified threshols \a threshold
     *
     * \param threshold    threshold againts the given PlotCells shall be checked
     */
    PlotPredicateEqual(plot_t threshold);
    /**
     *
     * \param threshold    threshold againts given PlotCells shall be checked against.
     */
    PlotPredicateEqual(std::string threshold);
    PlotPredicateEqual(const PlotCellValue& threshold);

    /// return a vector of the stored thresholds
    virtual std::vector<PlotCellValue> getThresholdValues() const;

    /// checks whether value stored in PlotCell \a value fulfills the predicate
    bool check(const PlotCellValue& value) const;

    /// creates a deep copy of the current PlotPredicate
    virtual PlotPredicate* clone() const;

    /// returns a string representation of this predicate
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

private:
    PlotPredicateEqual();

    PlotCellValue threshold_;  ///< threshold againts the given PlotCells shall be checked
};

/**
 * \brief  Predicate which checks if values in all given PlotCells are less than the threshold
 *         given at instantiation.
 */
class PlotPredicateEqualVector : public PlotPredicate {
friend class PlotPredicateFactory;
public:

    /**
     * PlotPredicateEqual(plot_t threshold)
     *
     * \brief      Constructor: creates a new PlotPredicate checking whether a given value is less than
     *             specified threshols \a threshold
     *
     * \param threshold    threshold againts the given PlotCells shall be checked
     */
    PlotPredicateEqualVector(const std::vector<plot_t>& threshold);
    /**
     *
     * \param threshold    threshold againts given PlotCells shall be checked against.
     */
    PlotPredicateEqualVector(const std::vector<std::string>& threshold);
    PlotPredicateEqualVector(const std::vector<PlotCellValue>& threshold);

    /// return a vector of the stored thresholds
    virtual std::vector<PlotCellValue> getThresholdValues() const;

    /// checks whether value stored in PlotCell \a value fulfills the predicate
    bool check(const PlotCellValue& value) const;

    /// creates a deep copy of the current PlotPredicate
    virtual PlotPredicate* clone() const;

    /// returns a string representation of this predicate
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

private:
    PlotPredicateEqualVector();

    std::vector<PlotCellValue> threshold_;  ///< threshold againts the given PlotCells shall be checked
};


/**
 * \brief  Predicate which checks if values in all given PlotCells are greater than the threshold
 *         given at instantiation.
 */
class PlotPredicateGreater : public PlotPredicate {
friend class PlotPredicateFactory;
public:

    /**
     * \brief      Constructor: creates a new PlotPredicate checking whether a given value is greater than
     *             specified threshols \a threshold
     *
     * \param threshold    threshold againts the given PlotCells shall be checked
     */
    PlotPredicateGreater(plot_t threshold);
    PlotPredicateGreater(std::string threshold);
    PlotPredicateGreater(const PlotCellValue& threshold);

    /// return a vector of the stored thresholds
    virtual std::vector<PlotCellValue> getThresholdValues() const;

    /// checks whether value stored in PlotCell \a value fulfills the predicate
    bool check(const PlotCellValue& value) const;

    /// creates a deep copy of the current PlotPredicate
    virtual PlotPredicate* clone() const;

    /// returns a string representation of this predicate
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

private:
    PlotPredicateGreater();

    PlotCellValue threshold_;      ///< threshold againts the given PlotCells shall be checked
};


class PlotPredicateContains : public PlotPredicate {
friend class PlotPredicateFactory;
public:


    PlotPredicateContains(const Interval<PlotCellValue>& interval);
    PlotPredicateContains(const Interval<plot_t>& interval);
    PlotPredicateContains(const Interval<std::string>& interval);
    /// return a vector of the stored thresholds
    virtual std::vector<PlotCellValue> getThresholdValues() const;

    /// checks whether value stored in PlotCellValue \a value fulfills the predicate
    bool check(const PlotCellValue& value) const;

    /// creates a deep copy of the current PlotPredicate
    virtual PlotPredicate* clone() const;

    /// returns a string representation of this predicate
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

private:
    PlotPredicateContains();

    Interval<PlotCellValue> interval_; ///< interval againts the given PlotCellValues shall be checked
};


/**
 * \brief   Predicate which checks if values in all given PlotCells are in between
 *          a lower and an upper threshold.
 */
class PlotPredicateBetween : public PlotPredicate {
friend class PlotPredicateFactory;
public:

    /**
     * \brief   Constructor: creates a new PlotPredicate checking whether a given value is
     *          in between \a smallThreshold and \a greatThreshold
     *
     * \param smallThreshold    lower threshold againts the given PlotCells shall be checked
     * \param greatThreshold    upper threshold againts the given PlotCells shall be checked
     */
    PlotPredicateBetween(plot_t smallThreshold, plot_t greatThreshold);
    PlotPredicateBetween(std::string smallThreshold, std::string greatThreshold);
    PlotPredicateBetween(const PlotCellValue& smallThreshold, const PlotCellValue& greatThreshold);

    /// return a vector of the stored thresholds
    virtual std::vector<PlotCellValue> getThresholdValues() const;

    /// checks whether value stored in PlotCell \a value fulfills the predicate
    bool check(const PlotCellValue& value) const;

    /// creates a deep copy of the current PlotPredicate
    virtual PlotPredicate* clone() const;

    /// returns a string representation of this predicate
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

private:
    PlotPredicateBetween();

    PlotCellValue smallThreshold_; ///< lower threshold againts the given PlotCells shall be checked
    PlotCellValue greatThreshold_; ///< upper threshold againts the given PlotCells shall be checked
};



/**
 * \brief  Predicate which checks if the threshold ist a substring of values in all given PlotCells
 *         given at instantiation.
 */
class PlotPredicateIsSubStr : public PlotPredicate {
friend class PlotPredicateFactory;
public:

    /**
     * PlotPredicateIsSubStr(std::string threshold)
     *
     * \brief      Constructor: creates a new PlotPredicate checking whether a given value is greater than
     *             specified threshols \a threshold
     *
     * \param threshold    threshold againts the given PlotCells shall be checked
     */
    PlotPredicateIsSubStr(std::string threshold);

    /// return a vector of the stored thresholds
    virtual std::vector<PlotCellValue> getThresholdValues() const;

    /// checks whether value stored in PlotCell \a value fulfills the predicate
    bool check(const PlotCellValue& value) const;

    /// creates a deep copy of the current PlotPredicate
    virtual PlotPredicate* clone() const;

    /// returns a string representation of this predicate
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

private:
    PlotPredicateIsSubStr();

    std::string threshold_;      ///< threshold againts the given PlotCells shall be checked
};



/**
 * \brief  Predicate which checks if values in all given PlotCells are greater than the threshold
 *         given at instantiation.
 */
class PlotPredicateNotAlphaNumeric : public PlotPredicate {
friend class PlotPredicateFactory;
public:

    /// return a vector of the stored thresholds
    virtual std::vector<PlotCellValue> getThresholdValues() const;

    /// checks whether value stored in PlotCell \a value fulfills the predicate
    bool check(const PlotCellValue& value) const;

    /// creates a deep copy of the current PlotPredicate
    virtual PlotPredicate* clone() const;

    /// returns a string representation of this predicate
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};



/**
 * \brief  Predicate which checks if values in all given PlotCells are greater than the threshold
 *         given at instantiation.
 */
class PlotPredicateAlphaNumeric : public PlotPredicate {
friend class PlotPredicateFactory;
public:

    /// return a vector of the stored thresholds
    virtual std::vector<PlotCellValue> getThresholdValues() const;

    /// checks whether value stored in PlotCell \a value fulfills the predicate
    bool check(const PlotCellValue& value) const;

    /// creates a deep copy of the current PlotPredicate
    virtual PlotPredicate* clone() const;

    /// returns a string representation of this predicate
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};



/**
 * \brief  Predicate which checks if values in all given PlotCells are greater than the threshold
 *         given at instantiation.
 */
class PlotPredicateEmpty : public PlotPredicate {
friend class PlotPredicateFactory;
public:

    /// return a vector of the stored thresholds
    virtual std::vector<PlotCellValue> getThresholdValues() const;

    /// checks whether value stored in PlotCell \a value fulfills the predicate
    bool check(const PlotCellValue& value) const;

    /// creates a deep copy of the current PlotPredicate
    virtual PlotPredicate* clone() const;

    /// returns a string representation of this predicate
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};



/**
 * \brief  Predicate which checks if values in all given PlotCells are greater than the threshold
 *         given at instantiation.
 */
class PlotPredicateNotEmpty : public PlotPredicate {
friend class PlotPredicateFactory;
public:

    /// return a vector of the stored thresholds
    virtual std::vector<PlotCellValue> getThresholdValues() const;

    /// checks whether value stored in PlotCell \a value fulfills the predicate
    bool check(const PlotCellValue& value) const;

    /// creates a deep copy of the current PlotPredicate
    virtual PlotPredicate* clone() const;

    /// returns a string representation of this predicate
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};


} // namespace voreen

#endif // VRN_PLOTPREDICATE_H
