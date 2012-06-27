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

#ifndef VRN_AGGREGATIONFUNCTION_H
#define VRN_AGGREGATIONFUNCTION_H

#include "voreen/core/plotting/plotbase.h"
#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

/**
 * \brief Abstract super class for aggregation functions which can be applied to the data.
 *
 * Subclasses have to override virtual evaluate function which applies the aggregation function
 * to a bunch of given plot_t.
 */
class AggregationFunction : public AbstractSerializable {
friend class AggregationFunctionFactory;
public:

    enum AggregationFunctionEnum {
        NONE = 0,
        COUNT = 1,
        COUNTHISTOGRAM = 2,
        MODE = 3,
        MIN = 20,
        MINHISTOGRAM = 21,
        MAX = 30,
        MAXHISTOGRAM = 31,
        SUM = 40,
        SUMHISTOGRAM = 41,
        MEDIAN = 50,
        MEDIANHISTOGRAM = 51,
        AVERAGE = 60,
        GEOMETRICMEAN = 61,
        HARMONICMEAN = 62,
        STANDARDDEVIATION = 70,
        VARIANCE = 75
    };

    /// Destructor
    virtual ~AggregationFunction() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const = 0;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const = 0;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const = 0;

};

/**
 * \brief Aggregation function which finds the count of given plot_t values in Histograms.
 */
class AggregationFunctionCountHistogram : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionCountHistogram() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which finds the count of given plot_t values.
 */
class AggregationFunctionCount : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionCount() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which finds the minimum value in given vector<plot_t>.
 */
class AggregationFunctionMinHistogram : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionMinHistogram() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which finds the minimum value in given vector<plot_t>.
 */
class AggregationFunctionMin : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionMin() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which finds the maximum value in given vector<plot_t>.
 */
class AggregationFunctionMaxHistogram : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionMaxHistogram() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which finds the maximum value in given vector<plot_t>.
 */
class AggregationFunctionMax : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionMax() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which calculates the sum of all values in given vector<plot_t>.
 */
class AggregationFunctionSumHistogram : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionSumHistogram() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which calculates the sum of all values in given vector<plot_t>.
 */
class AggregationFunctionSum : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionSum() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which calculates the arithmetic average (mean) of all values in given vector<plot_t>.
 */
class AggregationFunctionAverage : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionAverage() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};
/**
 * \brief Aggregation function which calculates the geomatric average (mean) of all values in given vector<plot_t>.
 */
class AggregationFunctionGeometricAverage : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionGeometricAverage() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};
/**
 * \brief Aggregation function which calculates the harmonic average (mean) of all values in given vector<plot_t>.
 */
class AggregationFunctionHarmonicAverage : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionHarmonicAverage() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which calculates the Median of all values in given vector<plot_t>.
 */
class AggregationFunctionMedian : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionMedian() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which calculates the Median of all values in given vector<plot_t>.
 */
class AggregationFunctionMedianHistogram : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionMedianHistogram() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which calculates the standard deviation of all values in given vector<plot_t>.
 */
class AggregationFunctionStandardDeviation : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionStandardDeviation() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which calculates the Variance of all values in given vector<plot_t>.
 */
class AggregationFunctionVariance : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionVariance() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

/**
 * \brief Aggregation function which calculates the Mode of all values in given vector<plot_t>.
 */
class AggregationFunctionMode : public AggregationFunction {
friend class AggregationFunctionFactory;
public:
    /// Destructor
    virtual ~AggregationFunctionMode() {};

    /// Applies the aggregation function to values \a values.
    virtual plot_t evaluate(std::vector<plot_t>& values) const;

    /// Create a copy of the actuell AggregationFunction.
    virtual AggregationFunction* clone() const;

    /// Returns a string representation of this AggregationFunction.
    virtual std::string toString() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

};

} // namespace voreen

#endif // VRN_AGGREGATIONFUNCTION_H
