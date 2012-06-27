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

#include "voreen/core/vis/exception.h"

// These includes in the header file is intentional
// The user should only has to include "commandlineparser.h" to use the full functionality
// The 'basic' classes
#include "voreen/core/cmdparser/command.h"
#include "voreen/core/cmdparser/singlecommand.h"
#include "voreen/core/cmdparser/multiplecommand.h"

// The directly usable ones
#include "voreen/core/cmdparser/command_loglevel.h"
#include "voreen/core/cmdparser/predefinedcommands.h"

namespace voreen {

class CommandlineParser {
public:
    /**
     * Default constructor which does nothing fancy at all
     * \param programName The name of the program. Used in the \sa displayUsage and \sa displayHelp methods
     */
    CommandlineParser(const std::string programName = "");

    /**
     * The destructor will also delete all the contained classes within
     */
    ~CommandlineParser();

    /**
     * Sets the commandline given from the main method.
     * \param argc The number of arguments
     * \param argv The arguments themself
     */
    void setCommandLine(int argc, char** argv);

    /**
     * Sets the commandline given from the main method.
     * \param arguments The arguments
     */
    void setCommandLine(std::vector<std::string> arguments);

    /**
     * Parses the commandline, evaluates all the commands and executes them. The nameless command
     * will be checked last, but executed first. Other executions might be somewhat random
     */
    void execute();

    /**
     * Add a new command to the parser.
     * \param cmd The new command
     * \throws CommandNameAssignedTwiceException self explanatory
     */
    void addCommand(Command* cmd) throw (VoreenException);

    /**
     * Add a new command to take care of the nameless arguments
     * \param cmd The command responsible for those arguments
     * \throws CommandAlreadyAssignedException self explanatory again
     */
    void addCommandForNamelessArguments(Command* cmd) throw (VoreenException);

    /// Returns the first commandline argument containing the path and the executable name
    std::string getProgramPath() const;

    /// Sets the verbosity of the commandline parser. When set, each (command,argument) pair will be
    /// printed to std::cout
    void setVerbosity(const bool verbosity);

    /**
     * Prints the usage information to the std::cout.
     * \param displayHelp Should the <code>help</code> section be printed as well?
     */
    void displayUsage();

    /// Print the full help text to the std::cout
    void displayHelp();


protected:
    /// Returns the command with a specific <code>shortName</code> or <code>name</code>. If no such
    /// command exists, 0 is returned
    Command* getCommand(std::string shortOrLongName);

    /// Bail out, display the message <code>msg</code> and display the usage
    void exitWithError(const std::string& msg);

    /// The stored commands
    std::vector<Command*> commands_;

    /// The command we want to use for nameless arguments    
    Command* commandForNamelessArguments_;

    /// All the arguments passed onto this parser
    std::vector<std::string> arguments_;

    /// The path to the program + filename
    std::string programPath_;

    /// The name of the program used in the \sa usage method
    std::string programName_;

    /// Controls the verbosity of the commandline parser in regard to execution
    bool verbosity_;
};

} // namespace

#endif // VRN_CMDLINEPARSER_H
