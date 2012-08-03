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

#include "aggregationfunctionfactory.h"

#include "../datastructures/aggregationfunction.h"

namespace voreen {

const std::string AggregationFunctionFactory::getTypeString(const std::type_info& type) const {
    if (type == typeid(AggregationFunctionMin))
        return "AggregationFunctionMin";
    if (type == typeid(AggregationFunctionMinHistogram))
        return "AggregationFunctionMinHistogram";
    else if (type == typeid(AggregationFunctionMax))
        return "AggregationFunctionMax";
    else if (type == typeid(AggregationFunctionMaxHistogram))
        return "AggregationFunctionMaxHistogram";
    else if (type == typeid(AggregationFunctionSum))
        return "AggregationFunctionSum";
    else if (type == typeid(AggregationFunctionSumHistogram))
        return "AggregationFunctionSumHistogram";
    else if (type == typeid(AggregationFunctionCount))
        return "AggregationFunctionCount";
    else if (type == typeid(AggregationFunctionCountHistogram))
        return "AggregationFunctionCountHistogram";
    else if (type == typeid(AggregationFunctionMedian))
        return "AggregationFunctionMedian";
    else if (type == typeid(AggregationFunctionMedianHistogram))
        return "AggregationFunctionMedianHistogram";
    else if (type == typeid(AggregationFunctionAverage))
        return "AggregationFunctionAverage";
    else if (type == typeid(AggregationFunctionGeometricAverage))
        return "AggregationFunctionGeometricAverage";
    else if (type == typeid(AggregationFunctionHarmonicAverage))
        return "AggregationFunctionHarmonicAverage";
    else if (type == typeid(AggregationFunctionStandardDeviation))
        return "AggregationFunctionStandardDeviation";
    else if (type == typeid(AggregationFunctionVariance))
        return "AggregationFunctionVariance";
    else if (type == typeid(AggregationFunctionMode))
        return "AggregationFunctionMode";
    else
        return "";
}

Serializable* AggregationFunctionFactory::createType(const std::string& typeString) {
    if (typeString == "AggregationFunctionMin")
        return new AggregationFunctionMin();
    if (typeString == "AggregationFunctionMinHistogram")
        return new AggregationFunctionMinHistogram();
    else if (typeString == "AggregationFunctionMax")
        return new AggregationFunctionMax();
    else if (typeString == "AggregationFunctionMaxHistogram")
        return new AggregationFunctionMaxHistogram();
    else if (typeString == "AggregationFunctionSum")
        return new AggregationFunctionSum();
    else if (typeString == "AggregationFunctionSumHistogram")
        return new AggregationFunctionSumHistogram();
    else if (typeString == "AggregationFunctionCount")
        return new AggregationFunctionCount();
    else if (typeString == "AggregationFunctionCountHistogram")
        return new AggregationFunctionCountHistogram();
    else if (typeString == "AggregationFunctionMedian")
        return new AggregationFunctionMedian();
    else if (typeString == "AggregationFunctionMedianHistogram")
        return new AggregationFunctionMedianHistogram();
    else if (typeString == "AggregationFunctionAverage")
        return new AggregationFunctionAverage();
    else if (typeString == "AggregationFunctionGeometricAverage")
        return new AggregationFunctionGeometricAverage();
    else if (typeString == "AggregationFunctionHarmonicAverage")
        return new AggregationFunctionHarmonicAverage();
    else if (typeString == "AggregationFunctionStandardDeviation")
        return new AggregationFunctionStandardDeviation();
    else if (typeString == "AggregationFunctionVariance")
        return new AggregationFunctionVariance();
    else if (typeString == "AggregationFunctionMode")
        return new AggregationFunctionMode();
    else
        return 0;
}

} // namespace voreen
