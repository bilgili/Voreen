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

#include "voreen/core/cmdparser/command_float.h"

namespace voreen {

Command_Float::Command_Float(float* fp, const std::string& name, const std::string& parameterName, const std::string& shortName) {
    fp_ = fp;
    name_ = name;
    shortName_ = shortName;
    parameterList_ = parameterName;
}

bool Command_Float::execute(const std::vector<std::string>& parameters) {
    *fp_ = castFloat(parameters.at(1));
    return true;
}

bool Command_Float::checkParameters(const std::vector<std::string>& parameters) {
    return ((parameters.size() == 2) && isFloat(parameters.at(1)));
}

//-----------------------------------------------------------------------------

Command_TwoFloat::Command_TwoFloat(float* fp1, float* fp2, const std::string& name, const std::string& parameterList, const std::string& shortName) {
    fp1_ = fp1;
    fp2_ = fp2;
    name_ = name;
    shortName_ = shortName;
    parameterList_ = parameterList;
}

bool Command_TwoFloat::execute(const std::vector<std::string>& parameters) {
    *fp1_ = castFloat(parameters.at(1));
    *fp2_ = castFloat(parameters.at(2));
    return true;
}

bool Command_TwoFloat::checkParameters(const std::vector<std::string>& parameters) {
    return ((parameters.size() == 3) && castFloat(parameters.at(1)) && castFloat(parameters.at(2)));
}

} // namespace
