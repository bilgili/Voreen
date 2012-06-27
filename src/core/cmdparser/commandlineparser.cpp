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

#include "voreen/core/cmdparser/commandlineparser.h"

#include "voreen/core/cmdparser/command.h"

namespace {

// Macro for creating the for-loop to iterate over all commands
#define FORALLCOMMANDS \
    for (std::vector<Command*>::iterator iter = commands_.begin();\
        iter != commands_.end();\
        ++iter)

/**
 * This procedure gets a line of the form "abc def ghi jkl" and creates a vector containing
 * ((abc),(def),(ghi),(jkl)) using a single space as a delimiter
 */
inline void stringToVector(const std::string& line, std::vector<std::string>& vec) {
    std::string::size_type space = line.find(" ");
    if (space != std::string::npos) {
        std::string subString = line.substr(0,space);
        vec.push_back(subString);

        stringToVector(line.substr(space+1), vec);
    } else
        vec.push_back(line);
}

} // namespace

namespace voreen {

CommandlineParser::CommandlineParser(std::string programName = "") :
        programName_(programName)
{
    argumentListWasWrong = false;
}

CommandlineParser::~CommandlineParser() {
    //delete all commands
    FORALLCOMMANDS {
        delete *iter;
    }
    commands_.clear();
}

void CommandlineParser::setCommandLine(int argc, char** argv) {
    std::string line = argv[0];
    for (int iter = 1; iter < argc; ++iter) {
        if (argv[iter][0] == '-') {
            // We have encountered a new command, so save the old one
            commandlineArguments_.push_back(line);
            line = argv[iter];
        } else {
            if (iter == 1) {
                // If the first argument doesn't start with a dash, we have a wrong argument list
                argumentListWasWrong = true;
                return;
            }
            // No new command, so append this one to the other
            line += " ";
            line += argv[iter];
        }
    }
    // We still have a command in the pipeline, so flush it
    commandlineArguments_.push_back(line);
}

void CommandlineParser::execute() {
    if (argumentListWasWrong)
        exitWithError("The first argument didn't start with a dash");

    bool parametersCorrect = true;
    for (size_t iter = 1; iter < commandlineArguments_.size(); ++iter) {
        std::string completeLine = commandlineArguments_.at(iter);
        std::vector<std::string> lineInAVector;
        stringToVector(completeLine, lineInAVector);

        Command* command = getCommand(lineInAVector.at(0));
        
        if (command != 0)
            parametersCorrect &= command->checkParameters(lineInAVector);
        else
            // command == means this command did not exist
            parametersCorrect = false;

        if (!parametersCorrect) {
            // If any of the parameters is not correct, we'll exit and display the usage message
            exitWithError("One of the parameters was not correct");
            usage();
            exit(EXIT_FAILURE);
        }
    }

    bool commandsExecutedWithoutError = true;
    for (size_t iter = 1; iter < commandlineArguments_.size(); ++iter) {
        std::string completeLine = commandlineArguments_.at(iter);
        std::vector<std::string> lineInAVector;
        stringToVector(completeLine, lineInAVector);

        Command* command = getCommand(lineInAVector.at(0));
        
        // All the commands exist, otherwise the first part of the procedure would've found out
        commandsExecutedWithoutError &= command->execute(lineInAVector);

        if (!commandsExecutedWithoutError) {
            // If any of the commands was not successful, we'll exit and display the usage message
            exitWithError("One of the executions was unsuccessful");
        }
    }
}

void CommandlineParser::addCommand(Command* cmd) throw(CommandNameAssignedTwiceException){
    // Check, if either the name or the shortname is already assigned in the parser
    if ((getCommand(cmd->getName()) == 0 && getCommand(cmd->getShortName())  == 0))
        commands_.push_back(cmd);
    else
        // One of the names existed, so throw an exception
        throw CommandNameAssignedTwiceException();
}

void CommandlineParser::usage() {
    std::string usageString = "Usage: " + programName_+ " ";
    FORALLCOMMANDS {
        if (*iter)
            usageString += (*iter)->usage() + " ";
    }
    // Display via the std-out because no Logger-Prefix is wanted with the output
    std::cout << usageString << std::endl;
    
}

Command* CommandlineParser::getCommand(std::string shortOrLongName) {
    FORALLCOMMANDS {
        if (((*iter)->getName() == shortOrLongName) || ((*iter)->getShortName() == shortOrLongName))
            return (*iter);
    }
    return 0;
}

inline void CommandlineParser::exitWithError(const std::string& msg) {
    std::cout << msg << std::endl;
    usage();
    exit(EXIT_FAILURE);
}

} // namespace
