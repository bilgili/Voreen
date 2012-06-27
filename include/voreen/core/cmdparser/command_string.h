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

#ifndef VRN_COMMAND_STRING_H
#define VRN_COMMAND_STRING_H

#include "voreen/core/cmdparser/command.h"

namespace voreen {

/**
 * This Command is able to set the string value given in the constructor to the second parameter
 * if the name of the command (also given in the constructor) is called at commandline level
 *
 * Example:
 * In the original application "trial" we create a 
 * new Command_String(&StringVar, "-testCommand", "-t", "<string value>") and add it to a CommandlineParser.
 * If the application is called with "trial -testCommand foo", the 'StringVar' will be set to "foo".
 *
 * The Usage entry will be: [<-t|-testCommand> <string value>]
 * \sa Command, Command_LoadNetwork, Command_LoadDataset, Command_LoadTransferFunction, CommandlineParser
 */
class Command_String : public Command {
public:
    Command_String(std::string* strp, const std::string& name, const std::string& shortName = "", const std::string& parameterList = "");
    bool execute(const std::vector<std::string>& parameters);
    bool checkParameters(const std::vector<std::string>& parameters);

protected:
    std::string* strp_;
};

//-----------------------------------------------------------------------------

/**
 * This command is used for loading a network file.
 * It simply calls the Command_String ctor and sets the values:
 * name          = --network
 * shortName     = -n
 * parameterList = <network file>
 * \sa Command, Command_String, CommandlineParser
 */
class Command_LoadNetwork : public Command_String {
public:
    Command_LoadNetwork(std::string* strp);
};

//-----------------------------------------------------------------------------

/**
 * This command is used for loading a dataset file.
 * It simply calls the Command_String ctor and sets the values:
 * name          = --dataset
 * shortName     = -d
 * parameterList = <dataset file>
 * \sa Command, Command_String, CommandlineParser
 */
class Command_LoadDataset : public Command_String {
public:
    Command_LoadDataset(std::string* strp);
};

//-----------------------------------------------------------------------------

/**
 * This command is used for loading a transfer function.
 * It simply calls the Command_String ctor and sets the values:
 * name          = --transferfunction
 * shortName     = -tf
 * parameterList = <transfer function>
 * \sa Command, Command_String, CommandlineParser
 */
class Command_LoadTransferFunction : public Command_String {
public:
    Command_LoadTransferFunction(std::string* strp);
};

} // namespace

#endif // VRN_COMMAND_STRING_H
