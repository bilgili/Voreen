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

#include "voreen/core/cmdparser/command_integer.h"

namespace voreen {

Command_Integer::Command_Integer(int* ip, const std::string& name, const std::string& parameterList, const std::string& shortName) {
    ip_ = ip;
    name_ = name;
    shortName_ = shortName;
    parameterList_ = parameterList;
}

bool Command_Integer::execute(const std::vector<std::string>& parameters) {
    *ip_ = castInt(parameters.at(1));
    return true;
}

bool Command_Integer::checkParameters(const std::vector<std::string>& parameters) {
    return ((parameters.size() == 2) && isInt(parameters.at(1)));
}

//-----------------------------------------------------------------------------

Command_TwoInteger::Command_TwoInteger(int* ip1, int* ip2, const std::string& name, const std::string& parameterList, const std::string& shortName) {
    ip1_ = ip1;
    ip2_ = ip2;
    name_ = name;
    shortName_ = shortName;
    parameterList_ = parameterList;
}

bool Command_TwoInteger::execute(const std::vector<std::string>& parameters) {
    *ip1_ = castInt(parameters.at(1));
    *ip2_ = castInt(parameters.at(2));
    return true;
}

bool Command_TwoInteger::checkParameters(const std::vector<std::string>& parameters) {
    return ((parameters.size() == 3) && isInt(parameters.at(1)) && isInt(parameters.at(2)));
}

//-----------------------------------------------------------------------------

Command_Width::Command_Width(int* ip) : Command_Integer(ip, "--width", "-w", "<value>") {}

Command_Height::Command_Height(int* ip) : Command_Integer(ip, "--height", "-h", "<value>") {}

Command_Size::Command_Size(int* ip1, int* ip2) : Command_TwoInteger(ip1, ip2, "--size", "-s", "<value1 value2>") {}

} // namespace
