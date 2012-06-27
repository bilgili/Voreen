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

#include "voreen/core/utils/cmdparser/command.h"

#include <sstream>

namespace voreen {

Command::Command(const std::string& name, const std::string& shortName, const std::string& infoText,
                 const std::string& parameterList, const int argumentNum, const bool allowMultipleCalls)
    : name_(name)
    , shortName_(shortName)
    , infoText_(infoText)
    , parameterList_(parameterList)
    , argumentNum_(argumentNum)
    , allowMultipleCalls_(allowMultipleCalls)
{}

Command::~Command() {}

const std::string Command::getName() {
    return name_;
}

const std::string Command::getShortName() {
    return shortName_;
}

const std::string Command::getParameterList() {
    return parameterList_;
}

const std::string Command::getInfoText() {
    return infoText_;
}

int Command::getArgumentNumber() {
    return argumentNum_;
}

bool Command::getAllowMultipleCalls() {
    return allowMultipleCalls_;
}

const std::string Command::usage() {
    std::string result = "[";
    if (getShortName() != "")
        result = result + "<" + getShortName() + "|" + getName() + ">";
    else
        result += getName();

    if (getParameterList() != "")
        result = result + " " + getParameterList();

    result += "]";

    return result;
}

const std::string Command::help() {
    std::string result;
    if (getShortName() != "")
        result = getShortName() + "|" + getName() + ": \t" + getInfoText();
    else
        result = getName() + ": \t" + getInfoText();

    return result;
}

bool Command::checkParameters(const std::vector<std::string>& parameters) {
    return (parameters.size() == static_cast<size_t>(getArgumentNumber()));
}

bool Command::isValueInSet(const std::string& value, const std::set<std::string>& set) {
    return (set.find(value) != set.end());
}

}   //namespace voreen
