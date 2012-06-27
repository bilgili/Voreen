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

#include "voreen/core/plotting/plotpredicate.h"


#include <sstream>
#include <string>


namespace voreen {

// PlotPredicateLess methods -------------------------------------------------------

PlotPredicateLess::PlotPredicateLess()
    : threshold_(PlotCellValue())
    {}

PlotPredicateLess::PlotPredicateLess(plot_t threshold)
    : threshold_(PlotCellValue(threshold))
    {}
PlotPredicateLess::PlotPredicateLess(std::string threshold)
    : threshold_(PlotCellValue(threshold))
    {}

PlotPredicateLess::PlotPredicateLess(const PlotCellValue& threshold)
    : threshold_(threshold.isTag() ? PlotCellValue(threshold.getTag()) : PlotCellValue(threshold.getValue()))
    {}

std::vector<PlotCellValue> PlotPredicateLess::getThresholdValues() const {
    std::vector<PlotCellValue> erg;
    erg.push_back(threshold_);
    return erg;
}


bool PlotPredicateLess::check(const PlotCellValue& value) const {
    return ((value.isValue() && threshold_.isValue() && value.getValue() <  threshold_.getValue())
        || (value.isTag() && threshold_.isTag() && value.getTag() < threshold_.getTag()));
}

PlotPredicate* PlotPredicateLess::clone() const {
    return new PlotPredicateLess(threshold_);
}

std::string PlotPredicateLess::toString() const {
    std::stringstream ss;
    ss << "Less Predicate, Threshold=";
    if (threshold_.isNull())
        ss << "NULL";
    else if (threshold_.isValue())
        ss << threshold_.getValue();
    else
        ss << "\"" << threshold_.getTag() << "\"";
    return ss.str();
}

void PlotPredicateLess::serialize(XmlSerializer& s) const {
    s.serialize("Threshold",threshold_);
}

void PlotPredicateLess::deserialize(XmlDeserializer& s) {
    s.deserialize("Threshold",threshold_);
}



// PlotPredicateEqual methods -------------------------------------------------------

PlotPredicateEqual::PlotPredicateEqual()
    : threshold_(PlotCellValue())
    {}

PlotPredicateEqual::PlotPredicateEqual(plot_t threshold)
    : threshold_(PlotCellValue(threshold))
    {}

PlotPredicateEqual::PlotPredicateEqual(std::string threshold)
    : threshold_(PlotCellValue(threshold))
    {}

PlotPredicateEqual::PlotPredicateEqual(const PlotCellValue& threshold)
    : threshold_(threshold.isTag() ? PlotCellValue(threshold.getTag()) : PlotCellValue(threshold.getValue()))
    {}

std::vector<PlotCellValue> PlotPredicateEqual::getThresholdValues() const {
    std::vector<PlotCellValue> erg;
    erg.push_back(threshold_);
    return erg;
}

bool PlotPredicateEqual::check(const PlotCellValue& value) const {
    return ((value.isValue() && threshold_.isValue() && value.getValue() == threshold_.getValue()) ||
        (value.isTag() && threshold_.isTag() && value.getTag() == threshold_.getTag()));
}

PlotPredicate* PlotPredicateEqual::clone() const {
    return new PlotPredicateEqual(threshold_);
}

std::string PlotPredicateEqual::toString() const {
    std::stringstream ss;
    ss << "Equals Predicate, Value=";
    if (threshold_.isNull())
        ss << "NULL";
    else if (threshold_.isValue())
        ss << threshold_.getValue();
    else
        ss << "\"" << threshold_.getTag() << "\"";
    return ss.str();
}

void PlotPredicateEqual::serialize(XmlSerializer& s) const {
    s.serialize("Threshold",threshold_);
}

void PlotPredicateEqual::deserialize(XmlDeserializer& s) {
    s.deserialize("Threshold",threshold_);
}

// PlotPredicateEqual methods -------------------------------------------------------

PlotPredicateEqualVector::PlotPredicateEqualVector()
    : threshold_(std::vector<PlotCellValue>(0))
    {}

PlotPredicateEqualVector::PlotPredicateEqualVector(const std::vector<plot_t>& threshold) {
    threshold_.clear();
    for (size_t i = 0; i < threshold.size(); ++i) {
        threshold_.push_back(PlotCellValue(threshold.at(i)));
    }
}

PlotPredicateEqualVector::PlotPredicateEqualVector(const std::vector<std::string>& threshold) {
    threshold_.clear();
    for (size_t i = 0; i < threshold.size(); ++i) {
        threshold_.push_back(PlotCellValue(threshold.at(i)));
    }
}

PlotPredicateEqualVector::PlotPredicateEqualVector(const std::vector<PlotCellValue>& threshold) {
    threshold_.clear();
    for (size_t i = 0; i < threshold.size(); ++i) {
        threshold_.push_back(threshold.at(i));
    }
}

std::vector<PlotCellValue> PlotPredicateEqualVector::getThresholdValues() const {
    std::vector<PlotCellValue> erg;
    for (size_t i = 0; i < threshold_.size(); ++i) {
        erg.push_back(threshold_.at(i));
    }
    return erg;
}


bool PlotPredicateEqualVector::check(const PlotCellValue& value) const {
    bool erg = false;
    for (size_t i = 0; i < threshold_.size(); ++i) {
        erg = erg || ((value.isValue() && threshold_.at(i).isValue() && value.getValue() == threshold_.at(i).getValue()) ||
        (value.isTag() && threshold_.at(i).isTag() && value.getTag() == threshold_.at(i).getTag()));
    }
     return erg;
}

PlotPredicate* PlotPredicateEqualVector::clone() const {
    return new PlotPredicateEqualVector(threshold_);
}


std::string PlotPredicateEqualVector::toString() const {
    std::stringstream ss;
    ss << "Equals Predicate, ";
    for (size_t i = 0; i < threshold_.size(); ++i) {
        ss << "Value" << i << " ";
        if (threshold_.at(i).isNull())
          ss << "NULL";
        else if (threshold_.at(i).isValue())
            ss << threshold_.at(i).getValue();
        else
            ss << "\"" << threshold_.at(i).getTag() << "\"";
    }
        return ss.str();
}

void PlotPredicateEqualVector::serialize(XmlSerializer& s) const {
    s.serialize("Threshold",threshold_);
}

void PlotPredicateEqualVector::deserialize(XmlDeserializer& s) {
    s.deserialize("Threshold",threshold_);
}

// PlotPredicateGreater methods -------------------------------------------------------

PlotPredicateGreater::PlotPredicateGreater()
    : threshold_(PlotCellValue())
    {}

PlotPredicateGreater::PlotPredicateGreater(plot_t threshold)
    : threshold_(PlotCellValue(threshold))
    {}

PlotPredicateGreater::PlotPredicateGreater(std::string threshold)
    : threshold_(PlotCellValue(threshold))
    {}

PlotPredicateGreater::PlotPredicateGreater(const PlotCellValue& threshold)
    : threshold_(threshold.isTag() ? PlotCellValue(threshold.getTag()) : PlotCellValue(threshold.getValue()))
    {}

std::vector<PlotCellValue> PlotPredicateGreater::getThresholdValues() const {
    std::vector<PlotCellValue> erg;
    erg.push_back(threshold_);
    return erg;
}


bool PlotPredicateGreater::check(const PlotCellValue& value) const {
    return ((value.isValue() && threshold_.isValue() && value.getValue() > threshold_.getValue()) ||
        (value.isTag() && threshold_.isTag() && value.getTag() > threshold_.getTag()));
}

PlotPredicate* PlotPredicateGreater::clone() const {
    return new PlotPredicateGreater(threshold_);
}


std::string PlotPredicateGreater::toString() const {
    std::stringstream ss;
    ss << "Greater Predicate, Threshold=";
    if (threshold_.isNull())
        ss << "NULL";
    else if (threshold_.isValue())
        ss << threshold_.getValue();
    else
        ss << "\"" << threshold_.getTag() << "\"";
    return ss.str();
}

void PlotPredicateGreater::serialize(XmlSerializer& s) const {
    s.serialize("Threshold",threshold_);
}

void PlotPredicateGreater::deserialize(XmlDeserializer& s) {
    s.deserialize("Threshold",threshold_);
}


// PlotPredicateInterval methods -------------------------------------------------------

PlotPredicateContains::PlotPredicateContains()
    : interval_(Interval<PlotCellValue>())
{}

PlotPredicateContains::PlotPredicateContains(const Interval<PlotCellValue>& interval)
    : interval_(interval)
{}

PlotPredicateContains::PlotPredicateContains(const Interval<plot_t>& interval)
    : interval_(Interval<PlotCellValue>(PlotCellValue(interval.getLeft()),
    PlotCellValue(interval.getRight()),interval.getLeftOpen(),interval.getRightOpen()))
{
}

PlotPredicateContains::PlotPredicateContains(const Interval<std::string>& interval)
    : interval_(Interval<PlotCellValue>(PlotCellValue(interval.getLeft()),
    PlotCellValue(interval.getRight()),interval.getLeftOpen(),interval.getRightOpen()))
    {}

std::vector<PlotCellValue> PlotPredicateContains::getThresholdValues() const {
    std::vector<PlotCellValue> erg;
    erg.push_back(interval_.getLeft());
    erg.push_back(interval_.getRight());
    erg.push_back(PlotCellValue(interval_.getLeftOpen()));
    erg.push_back(PlotCellValue(interval_.getRightOpen()));
    return erg;
}

bool PlotPredicateContains::check(const PlotCellValue& value) const {
    return interval_.contains(value);
}

PlotPredicate* PlotPredicateContains::clone() const {
    return new PlotPredicateContains(interval_);
}


std::string PlotPredicateContains::toString() const {
    std::stringstream ss;
    ss << "Interval Predicate ";
    if (interval_.getLeftOpen())
        ss << "(";
    else
        ss << "[";
    ss << interval_.getLeft() << " ; " << interval_.getRight();
    if (interval_.getRightOpen())
        ss << ")";
    else
        ss << "]";
    return ss.str();
}

void PlotPredicateContains::serialize(XmlSerializer& s) const {
    s.serialize("Interval",interval_);
}

void PlotPredicateContains::deserialize(XmlDeserializer& s) {
    s.deserialize("Interval",interval_);
}



// PlotPredicateBetween methods -------------------------------------------------------

PlotPredicateBetween::PlotPredicateBetween()
    : smallThreshold_(PlotCellValue())
    , greatThreshold_(PlotCellValue())
    {}

PlotPredicateBetween::PlotPredicateBetween(plot_t smallThreshold, plot_t greatThreshold)
    : smallThreshold_(PlotCellValue(smallThreshold))
    , greatThreshold_(PlotCellValue(greatThreshold))
    {}

PlotPredicateBetween::PlotPredicateBetween(std::string smallThreshold, std::string greatThreshold)
    : smallThreshold_(PlotCellValue(smallThreshold))
    , greatThreshold_(PlotCellValue(greatThreshold))
    {}

PlotPredicateBetween::PlotPredicateBetween(const PlotCellValue& smallThreshold, const PlotCellValue& greatThreshold)
    : smallThreshold_(smallThreshold.isTag() ? PlotCellValue(smallThreshold.getTag()) : PlotCellValue(smallThreshold.getValue()))
    , greatThreshold_(greatThreshold.isTag() ? PlotCellValue(greatThreshold.getTag()) : PlotCellValue(greatThreshold.getValue()))
    {}


std::vector<PlotCellValue> PlotPredicateBetween::getThresholdValues() const {
    std::vector<PlotCellValue> erg;
    erg.push_back(smallThreshold_);
    erg.push_back(greatThreshold_);
    return erg;
}


bool PlotPredicateBetween::check(const PlotCellValue& value) const {
    return ((value.isValue() && smallThreshold_.isValue() && greatThreshold_.isValue() &&
        value.getValue() > smallThreshold_.getValue() && value.getValue() < greatThreshold_.getValue()) ||
        (value.isTag() && smallThreshold_.isTag() && greatThreshold_.isTag() &&
        value.getTag() > smallThreshold_.getTag() && value.getTag() < greatThreshold_.getTag()));
}

PlotPredicate* PlotPredicateBetween::clone() const {
    return new PlotPredicateBetween(smallThreshold_,greatThreshold_);
}


std::string PlotPredicateBetween::toString() const {
    std::stringstream ss;
    ss << "Between Predicate, lower Threshold=";
    if (smallThreshold_.isNull())
        ss << "NULL";
    else if (smallThreshold_.isValue())
        ss << smallThreshold_.getValue();
    else
        ss << "\"" << smallThreshold_.getTag() << "\"";
    ss << ", upper Threshold=";
    if (greatThreshold_.isNull())
        ss << "NULL";
    else if (greatThreshold_.isValue())
        ss << greatThreshold_.getValue();
    else
        ss << "\"" << greatThreshold_.getTag() << "\"";
    return ss.str();
}

void PlotPredicateBetween::serialize(XmlSerializer& s) const {
    s.serialize("smallThreshold",smallThreshold_);
    s.serialize("greatThreshold",greatThreshold_);
}

void PlotPredicateBetween::deserialize(XmlDeserializer& s) {
    s.deserialize("smallThreshold",smallThreshold_);
    s.deserialize("greatThreshold",greatThreshold_);
}

// PlotPredicateIsSubStr methods -------------------------------------------------------

PlotPredicateIsSubStr::PlotPredicateIsSubStr()
    : threshold_("")
    {}

PlotPredicateIsSubStr::PlotPredicateIsSubStr(std::string threshold)
    : threshold_(threshold)
    {}

bool PlotPredicateIsSubStr::check(const PlotCellValue& value) const {
    return (value.isTag() && (value.getTag().find(threshold_,0) != std::string::npos));
}


std::vector<PlotCellValue> PlotPredicateIsSubStr::getThresholdValues() const {
    std::vector<PlotCellValue> erg;
    erg.push_back(PlotCellValue(threshold_));
    return erg;
}

PlotPredicate* PlotPredicateIsSubStr::clone() const {
    return new PlotPredicateIsSubStr(threshold_);
}



std::string PlotPredicateIsSubStr::toString() const {
    std::stringstream ss;
    ss << "Substring Predicate, Threshold=";
    ss << "\"" << threshold_ << "\"";
    return ss.str();
}

void PlotPredicateIsSubStr::serialize(XmlSerializer& s) const {
    s.serialize("threshold",threshold_);
}

void PlotPredicateIsSubStr::deserialize(XmlDeserializer& s) {
    s.deserialize("threshold",threshold_);
}




// PlotPredicateNotAlphaNumeric methods -------------------------------------------------------

bool PlotPredicateNotAlphaNumeric::check(const PlotCellValue& value) const {
    if (value.isValue())
        return false;
    if (value.isNull())
        return false;
    if (value.isTag()) {
        std::string s = value.getTag();
        std::stringstream Str(s);
        double num;
        if (Str >> num)
            return false;
        for (size_t i = 0; i < s.length(); ++i) {
            int c = s[i];
            if (c <= 0 || !isalnum(s[i])) {
                return true;
            }
        }
        return false;
    }

    return true;
}

PlotPredicate* PlotPredicateNotAlphaNumeric::clone() const {
    return new PlotPredicateNotAlphaNumeric();
}



std::vector<PlotCellValue> PlotPredicateNotAlphaNumeric::getThresholdValues() const {
    std::vector<PlotCellValue> erg;
    return erg;
}


std::string PlotPredicateNotAlphaNumeric::toString() const {
    std::stringstream ss;
    ss << "Not Alpha-Numeric Predicate";
    //ss << "\"" << threshold_ << "\"";
    return ss.str();
}

void PlotPredicateNotAlphaNumeric::serialize(XmlSerializer& /*s*/) const {
}

void PlotPredicateNotAlphaNumeric::deserialize(XmlDeserializer& /*s*/) {
}





// PlotPredicateAlphaNumeric methods -------------------------------------------------------

bool PlotPredicateAlphaNumeric::check(const PlotCellValue& value) const {
    if (value.isValue())
        return true;
    if (value.isNull())
        return false;
    if (value.isTag()) {
        std::string s = value.getTag();
        std::stringstream Str(s);
        double num;
        if (Str >> num)
            return true;
        for (size_t i = 0; i < s.length(); ++i) {
            int c = s[i];
            if (c <= 0 || !isalnum(s[i])) {
                return false;
            }
        }
        return true;
    }

    return false;
}

PlotPredicate* PlotPredicateAlphaNumeric::clone() const {
    return new PlotPredicateAlphaNumeric();
}



std::vector<PlotCellValue> PlotPredicateAlphaNumeric::getThresholdValues() const {
    std::vector<PlotCellValue> erg;
    return erg;
}


std::string PlotPredicateAlphaNumeric::toString() const {
    std::stringstream ss;
    ss << "Alpha Numeric Predicate";
    //ss << "\"" << threshold_ << "\"";
    return ss.str();
}

void PlotPredicateAlphaNumeric::serialize(XmlSerializer& /*s*/) const {
}

void PlotPredicateAlphaNumeric::deserialize(XmlDeserializer& /*s*/) {
}





// PlotPredicateEmpty methods -------------------------------------------------------

bool PlotPredicateEmpty::check(const PlotCellValue& value) const {
    return (value.isNull());
}

PlotPredicate* PlotPredicateEmpty::clone() const {
    return new PlotPredicateEmpty();
}



std::vector<PlotCellValue> PlotPredicateEmpty::getThresholdValues() const {
    std::vector<PlotCellValue> erg;
    return erg;
}


std::string PlotPredicateEmpty::toString() const {
    std::stringstream ss;
    ss << "Empty Predicate";
    //ss << "\"" << threshold_ << "\"";
    return ss.str();
}

void PlotPredicateEmpty::serialize(XmlSerializer& /*s*/) const {
}

void PlotPredicateEmpty::deserialize(XmlDeserializer& /*s*/) {
}





// PlotPredicateNotEmpty methods -------------------------------------------------------

bool PlotPredicateNotEmpty::check(const PlotCellValue& value) const {
    return (!value.isNull());
}

PlotPredicate* PlotPredicateNotEmpty::clone() const {
    return new PlotPredicateNotEmpty();
}



std::vector<PlotCellValue> PlotPredicateNotEmpty::getThresholdValues() const {
    std::vector<PlotCellValue> erg;
    return erg;
}


std::string PlotPredicateNotEmpty::toString() const {
    std::stringstream ss;
    ss << "Not Empty Predicate";
    //ss << "\"" << threshold_ << "\"";
    return ss.str();
}

void PlotPredicateNotEmpty::serialize(XmlSerializer& /*s*/) const {
}

void PlotPredicateNotEmpty::deserialize(XmlDeserializer& /*s*/) {
}

}

