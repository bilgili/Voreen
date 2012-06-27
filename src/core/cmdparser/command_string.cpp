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

#include "voreen/core/cmdparser/command_string.h"

namespace voreen {

Command_String::Command_String(std::string* strp, const std::string& name, const std::string& shortName, const std::string& parameterList) {
    strp_ = strp;
    name_ = name;
    shortName_ = shortName;
    if (parameterList == "")
        parameterList_ = "<string value>";
    else
        parameterList_ = parameterList;
}

bool Command_String::execute(const std::vector<std::string>& parameters) {
    *strp_ = parameters.at(1);
    return true;
}

bool Command_String::checkParameters(const std::vector<std::string>& parameters) {
    return (parameters.size() == 2);
}

//-----------------------------------------------------------------------------

Command_LoadNetwork::Command_LoadNetwork(std::string* strp) : Command_String(strp, "--network", "-n", "<network file>") {}

Command_LoadDataset::Command_LoadDataset(std::string* strp) : Command_String(strp, "--dataset", "-d", "<dataset file>") {}

Command_LoadTransferFunction::Command_LoadTransferFunction(std::string* strp) : Command_String(strp, "--transferfunction", "-tf", "<transfer function>") {}

} // namespace
