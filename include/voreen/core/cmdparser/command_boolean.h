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

#ifndef VRN_COMMAND_BOOLEAN_H
#define VRN_COMMAND_BOOLEAN_H

#include "voreen/core/cmdparser/command.h"

namespace voreen {

/**
 * This Command is able to set the boolean value given in the constructor to 'true' if the
 * name of the command (also given in the constructor) is called at commandline level
 *
 * Example:
 * In the original application "trial" we create a 
 * new Command_Boolean(&boolVar, "-testCommand") and add it to a CommandlineParser.
 * If the application is called with "trial -testCommand", the 'boolVar' will be set to true.
 * \sa Command, Command_Benchmark, CommandlineParser
 */
class Command_Boolean : public Command {
public:
    Command_Boolean(bool* bp, const std::string& name, const std::string& shortName = "");
    bool execute(const std::vector<std::string>& parameters);
    bool checkParameters(const std::vector<std::string>& parameters);

protected:
    bool* bp_;
};

//-----------------------------------------------------------------------------

/**
 * This command is used for toggling a benchmark in the application.
 * It simply calls the Command_String ctor and sets the values:
 * name          = --benchmark
 * shortName     = -b
 * \sa Command, Command_Boolean, CommandlineParser
 */
class Command_Benchmark : public Command_Boolean {
public:
    Command_Benchmark(bool* bp);
};

} // namespace

#endif // VRN_COMMAND_BOOLEAN_H
