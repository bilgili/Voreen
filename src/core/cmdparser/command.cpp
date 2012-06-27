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

#include "voreen/core/cmdparser/command.h"

namespace voreen {

Command::Command() : loggerCat_("voreen.Command") {
}

const std::string Command::getName() {
    return name_;
}

const std::string Command::getShortName() {
    return shortName_;
}

const std::string Command::getParameterList() {
    return parameterList_;
}

const std::string Command::getInfo() {
    return "";
}

const std::string Command::usage() {
    bool shortNameExists = (getShortName() != "");
    bool parameterListExists = (getParameterList() != "");

    std::string result = "[";
    if (shortNameExists)
        result = result + "<" + getShortName() + "|" + getName() + ">";
    else
        result += getName();

    if (parameterListExists)
        result = result + " " + getParameterList();

    result += "]";

    return result;
}

int Command::castInt(const std::string& s) throw (SyntaxException) {
    std::istringstream iss(s);
    int t;
    bool conversionFailed = (iss >> std::dec >> t).fail();
    if (conversionFailed)
        throw SyntaxException("Failed to convert parameter to integer");
    else
        return t;
}
    
bool Command::isInt(const std::string& s) {
    std::istringstream iss(s);
    int t;
    return (iss >> std::dec >> t);
}

float Command::castFloat(const std::string& s) throw (SyntaxException) {
    std::istringstream iss(s);
    float t;
    bool conversionFailed = (iss >> std::dec >> t).fail();
    if (conversionFailed)
        throw SyntaxException("Failed to convert parameter to float");
    else
        return t;
}

bool Command::isFloat(const std::string& s) {
    std::istringstream iss(s);
    float t;
    return (iss >> std::dec >> t);
}

}   //namespace voreen
