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

#ifndef VRN_CMDLINEPARSER_H
#define VRN_CMDLINEPARSER_H

// These includes in the header file is intentional
// The programmers should only has to include "commandlineparser.h" to use the functionality
#include "voreen/core/cmdparser/command.h"
#include "voreen/core/cmdparser/command_boolean.h"
#include "voreen/core/cmdparser/command_float.h"
#include "voreen/core/cmdparser/command_integer.h"
#include "voreen/core/cmdparser/command_loglevel.h"
#include "voreen/core/cmdparser/command_string.h"

namespace voreen {

class Command;

class CommandNameAssignedTwiceException : public std::exception {};

/**
 * This class manages commandline arguments and is able to execute them. This is done by adding
 * \sa Commands to this class. In these Commands the functionality for an explicit commandline argument
 * is codified.
 * To use this class you have to create a CommandlineParser and then use the \sa addCommand method.
 * The commands are tested and executed in the \sa execute method. Each command will be checked for the
 * correct amount of parameters and then will be executed. If either of those goes wrong, a unix-like
 * "usage" message will be displayed to <code>std::cout</code>.
 *
 * Showcase use:
 * We have three <code>Command</code>s called "-command1" "-load <name>" "-maximize" with 0,1 and 0
 * parameters respectively.
 * In the main method of the programm we create a CommandlineParser, call \sa setCommandLine
 * with the argc and argv arguments from the c main method and consecutively \sa addCommand
 * each of the three commands. Afterwards we can use the \sa execute method and pass the args
 * parameter from the c(++) main method. The rest will be done by this class.
 * \sa Command
 */
class CommandlineParser {
public:
    /**
     * Initializes the CommandlineParser.
     * \param programName The Name of the Program. Necessary for the "usage" output
     */
    CommandlineParser(std::string programName);

    /**
     * Deletes all the added Commands aswell
     */
    ~CommandlineParser();

    /**
     * Stores the commandline arguments into a vector. This is done by seperating the string
     * along the dashes and grouping the arguments. The first argument (the program name) is
     * omitted.
     * e.g. "voreen.exe -flag1 -command2 argument3 -flag4"
     * will be converted to a vector:
     * ("flag1"), ("-command2 argument3"), ("flag4")
     * \param argc The count of arguments
     * \param argv The arguments themselves
     */
    void setCommandLine(int argc, char** argv);

    /** 
     * Parses and executes all commands. This method first checks, if all the given commands
     * exist in this commandline parser and have the correct (i.e. right amount and right type)
     * parameters. If that is so, the commands will be executed consecutively.
     * If either this or the testing before is unsuccessful, the program will be terminated
     * and the \sa usage method will be called.
     * \sa usage
     */
    void execute();

    /**
     * Prints the "usage" lines to <code>std::cout</code> containing all the accessible commands and
     * the associated info-lines. This method simply puts a header containing the information
     * about the program name above a list of usage-infos of the commands being administrated
     * in this commandline parser.
     */
    void usage();

    /**
     * Adds \sa command to the parser.
     * It will be checked if either name (short or long) is already exists in the commandline parser.
     * \param cmd The command you want to add
     * \throws CommandNameAssignedTwiceException If a name would be assigned twice.
     */
    void addCommand(Command* cmd) throw(CommandNameAssignedTwiceException);

private:
    /**
     * Fetches a command from the vector identifies by a (short) name.
     * \param shortOrLongName The name of the command. This can either be the short or the long name.
     * \return The appropriate command
     */
    Command* getCommand(std::string shortOrLongName);

    /**
     * Terminates the program and displays the usage message to std::cout
     * \param msg The message which should be displayed
     */
    void exitWithError(const std::string& msg);

    /// This vector contains all the added commands
    std::vector<Command*> commands_;
    /// The name of the creating program. Necessary for the \sa usage command
    std::string programName_;
    /// The commandline being stored from the \sa setCommandLine method
    std::vector<std::string> commandlineArguments_;
    /// If the first argument of the argument list didn't start with a dash,we have
    /// to exit the program at a different place
    bool argumentListWasWrong;
};

} // namespace

#endif // VRN_CMDLINEPARSER_H
