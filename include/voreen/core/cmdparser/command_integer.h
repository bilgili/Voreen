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

#ifndef VRN_COMMAND_INTEGER_H
#define VRN_COMMAND_INTEGER_H

#include "voreen/core/cmdparser/command.h"

namespace voreen {

/**
 * This Command is able to set the integer value given in the constructor to a certain value x
 * if the name of the command (also given in the constructor) is called at commandline level with
 * the parameter x.
 *
 * Example:
 * In the original application "trial" we create a 
 * new Command_Integer(&intVar, "-testCommand") and add it to a CommandlineParser.
 * If the application is called with "trial -testCommand 42", the 'intVar' will be set to 42.
 * \sa Command, Command_TwoInteger, Command_Width, Command_Height, CommandlineParser
 */
class Command_Integer : public Command {
public:
    Command_Integer(int* ip, const std::string& name, const std::string& parameterList, const std::string& shortName = "");
    bool execute(const std::vector<std::string>& parameters);
    bool checkParameters(const std::vector<std::string>& parameters);

protected:
    int* ip_;
};

//-----------------------------------------------------------------------------

/**
 * This Command is able to set two integer values given in the constructor to certain values x and y
 * if the name of the command (also given in the constructor) is called at commandline level with
 * the parameters x and y.
 *
 * Example:
 * In the original application "trial" we create a 
 * new Command_TwoInteger(&intVar1, &intVar2, "-testCommand") and add it to a CommandlineParser.
 * If the application is called with "trial -testCommand 42 1337", the 'intVar1' will be set to 42
 * and 'intVar2' will be 1337 afterwards
 * \sa Command, Command_Integer, CommandlineParser
 */
class Command_TwoInteger : public Command {
public:
    Command_TwoInteger(int* ip1, int* ip2, const std::string& name, const std::string& parameterList, const std::string& shortName = "");
    bool execute(const std::vector<std::string>& parameters);
    bool checkParameters(const std::vector<std::string>& parameters);

protected:
    int* ip1_;
    int* ip2_;
};

//-----------------------------------------------------------------------------

/**
 * This command is used for setting a width.
 * It simply calls the Command_Integer ctor and sets the values:
 * name          = --width
 * shortName     = -w
 * parameterList = <value>
 * \sa Command, Command_Integer, CommandlineParser
 */
class Command_Width : public Command_Integer {
public:
    Command_Width(int* ip);
};

//-----------------------------------------------------------------------------

/**
 * This command is used for setting a height.
 * It simply calls the Command_Integer ctor and sets the values:
 * name          = --height
 * shortName     = -h
 * parameterList = <value>
 * \sa Command, Command_Integer, CommandlineParser
 */
class Command_Height : public Command_Integer {
public:
    Command_Height(int* ip);
};

//-----------------------------------------------------------------------------

/**
 * This command is used for setting a size.
 * It simply calls the Command_TwoInteger ctor and sets the values:
 * name          = --size
 * shortName     = -s
 * parameterList = <value1 value2>
 * \sa Command, Command_Integer, CommandlineParser
 */
class Command_Size : public Command_TwoInteger {
public:
    Command_Size(int* ip1, int* ip2);
};

} // namespace

#endif // VRN_COMMAND_INTEGER_H
