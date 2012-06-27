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

#ifndef VRN_COMMAND_FLOAT_H
#define VRN_COMMAND_FLOAT_H

#include "voreen/core/cmdparser/command.h"

namespace voreen {

/**
 * This Command is able to set the float value given in the constructor to a certain value x
 * if the name of the command (also given in the constructor) is called at commandline level with
 * the parameter x.
 *
 * Example:
 * In the original application "trial" we create a 
 * new Command_Float(&floatVar, "-testCommand") and add it to a CommandlineParser.
 * If the application is called with "trial -testCommand 42.11", the 'intVar' will be set to 42.11 .
 * \sa Command, CommandlineParser
 */
class Command_Float : public Command {
public:
    Command_Float(float* fp, const std::string& name, const std::string& parameterName, const std::string& shortName = "");
    bool execute(const std::vector<std::string>& parameters);
    bool checkParameters(const std::vector<std::string>& parameters);

protected:
    float* fp_;
};

//-----------------------------------------------------------------------------

/**
 * This Command is able to set two float values given in the constructor to certain values x and y
 * if the name of the command (also given in the constructor) is called at commandline level with
 * the parameters x and y.
 *
 * Example:
 * In the original application "trial" we create a 
 * new Command_TwoInteger(&floatVar1, &floatVar2, "-testCommand") and add it to a CommandlineParser.
 * If the application is called with "trial -testCommand 12.3 45.6", the 'floatVar1' will be set to 12.3
 * and 'floatVar2' will be 45.6 afterwards
 * \sa Command, Command_Integer, CommandlineParser
 */
class Command_TwoFloat : public Command {
public:
    Command_TwoFloat(float* fp1, float* fp2, const std::string& name, const std::string& parameterList, const std::string& shortName = "");
    bool execute(const std::vector<std::string>& parameters);
    bool checkParameters(const std::vector<std::string>& parameters);

protected:
    float* fp1_;
    float* fp2_;
};

} // namespace

#endif // VRN_COMMAND_FLOAT_H
