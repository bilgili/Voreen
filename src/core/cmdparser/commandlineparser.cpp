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

#include <vector>
#include <map>
#include <stdlib.h>

using std::vector;
using std::multimap;
using std::pair;
using std::string;

namespace {

// Macro for creating the for-loop to iterate over all commands
#define FORALLCOMMANDS \
    for (vector<Command*>::iterator iter = commands_.begin();\
        iter != commands_.end();\
        ++iter)

/**
 * Extracts multiple arguments from a single list. <br>
 * If <code>count</code> is <code>-2</code>, arguments will be extracted, as long as no new commands is found.
 * in[begin] itself will not be extracted if this is the command.<br>
 * If <code>count</code> is <code>-1</code>, the rest of the line will be extracted<br>
 * If <code>count</code> is > 0, that many arguments will be extracted and returned
 */
inline int extractArguments(const vector<string> in, vector<string>* out, const size_t begin, const int count) {
    int num = 0;
    if (count == -1) {
        for (size_t i = begin; i < in.size(); ++i, ++num)
            out->push_back(in[i]);
    }
    else
        if (count == -2) {
            // Extract arguments until a new command is found
            for (size_t i = begin; (i < in.size()) && (in[i][0] != '-'); ++i, num++) {
                out->push_back(in[i]);
            }
        }
    else {
        for (int i = 0; (i < count) && (static_cast<size_t>(begin+1+i) < in.size()) ; ++i, num++) {
            out->push_back(in[begin+1+i]);
        }
    }
    return num;
}

} // namespace

namespace voreen {

CommandlineParser::CommandlineParser(const std::string programName)
: commandForNamelessArguments_(0)
, programName_(programName)
, verbosity_(false)
{}

CommandlineParser::~CommandlineParser() {
    FORALLCOMMANDS {
        delete *iter;
    }
    commands_.clear();
}

std::string CommandlineParser::getProgramPath() const {
    return programPath_;
}

void CommandlineParser::setVerbosity(const bool verbosity) {
    verbosity_ = verbosity;
}

void CommandlineParser::setCommandLine(int argc, char** argv) {
    // argv[0] = program name
    // argv[i] = i-th argument
    programPath_ = argv[0];

    // Might be possible that someone calls us multiple times
    arguments_.clear();

    // Just add the arguments to the vector
    for (int i = 1; i < argc; ++i)
        arguments_.push_back(argv[i]);
}

void CommandlineParser::setCommandLine(std::vector<std::string> arguments) {
    // argv[0] = program name
    // argv[i] = i-th argument
    programPath_ = arguments[0];

    // Might be possible that someone calls us multiple times
    arguments_.clear();

    // Just add the arguments to the vector
    for (size_t i = 1; i < arguments.size(); ++i)
        arguments_.push_back(arguments[i]);
}

void CommandlineParser::execute() {
    // There is only one argument and this is either "-h" or "--help" ; so display the help
    if ((arguments_.size() == 1) && ( (arguments_[0] == "-h") || (arguments_[0] == "--help")) ) {
        displayHelp();
    }

    vector<string> argumentsForNameless;

    // We store the commands and parameters in a map to be able to execute them without parsing the
    // commandline again
    multimap<Command*, vector<string> > parameterMap;

    for (size_t i = 0 ; i < arguments_.size(); ++i) {
        // In the beginning we assume that we just started the loop or finished reading parameters for
        // one command

        // So test if the next argument is a command or a parameter for a nameless argument
        if (arguments_[i][0] != '-') {
            int number = extractArguments(arguments_, &argumentsForNameless, i, -2);
            i += (number - 1);
        }
        else {
            // We have found a command
            Command* currentCommand = getCommand(arguments_[i]);

            // currentCommand = 0, if there wasn't a command with that specific name or shortName
            if (currentCommand == 0)
                exitWithError(arguments_[i] + " is not a valid command");

            vector<string> parameters;
            int number = extractArguments(arguments_, &parameters, i, currentCommand->getArgumentNumber());
            i += number;

            // don't insert if the command doesn't allow multiple calls and already is in the map
            if (!currentCommand->getAllowMultipleCalls() && parameterMap.find(currentCommand) != parameterMap.end())
                exitWithError(currentCommand->getName() + " doesn't allow multiple calls in a single line");

            parameterMap.insert(pair<Command*, vector<string> >(currentCommand, parameters));
        }
    }
    // We now have all the commands with the respective parameters stored in the map and all the parameters for
    // the nameless command is avaiable as well.

    // First step: Test, if we have nameless arguments even if we don't have a nameless command. Otherwise bail out
    if ((argumentsForNameless.size() != 0) && (commandForNamelessArguments_ == 0))
        exitWithError("No appropriate command avaiable for nameless parameters");

    // Second step: Check if every command is happy with the parameters assigned to it
    for (multimap<Command*, vector<string> >::iterator it = parameterMap.begin(); it != parameterMap.end(); ++it) {
        bool correct = (*it).first->checkParameters((*it).second);
        if (!correct)
            exitWithError("One of the parameters for " + (*it).first->getName() + " was not correct");
    }

    // Second-and-a-halfs step: Display pairs for (command,argument) if verbosity is wanted
    if (verbosity_) {
        std::cout << "Verbosity output:" << std::endl;
        // First the nameless command
        std::cout << "(Nameless command,";
        for (vector<string>::iterator iter = argumentsForNameless.begin();
            iter != argumentsForNameless.end();
            ++iter)
            std::cout << " " << (*iter);
        std::cout << ")" << std::endl;

        // Then the rest
        for (multimap<Command*, vector<string> >::iterator it = parameterMap.begin(); it != parameterMap.end(); ++it) {
            std::cout << "(" << (*it).first->getName() << ",";

            for (vector<string>::iterator iter = (*it).second.begin();
                iter != (*it).second.end();
                ++iter)
                std::cout << " " << (*iter);
            std::cout << ")" << std::endl;
        }
        std::cout << std::endl;
    }

    // Third step: Execute the nameless command if there are any arguments available
    if (argumentsForNameless.size() != 0) {
        bool correct = commandForNamelessArguments_->checkParameters(argumentsForNameless);

        if (correct)
            commandForNamelessArguments_->execute(argumentsForNameless);
        else
            exitWithError("One of the parameters for the nameless command was not correct");
    }

    // Fourth step: Execute the commands (this step is only done if everyone is happy up until now)
    for (multimap<Command*, vector<string> >::iterator it = parameterMap.begin(); it != parameterMap.end(); ++it) {
        bool correct = (*it).first->execute((*it).second);
        if (!correct)
            exitWithError("The execution for " + (*it).first->getName() + " failed");
    }
}

void CommandlineParser::addCommand(Command* cmd) throw (VoreenException) {
    // Check, if either the name or the shortname is already assigned in the parser
    if (getCommand(cmd->getName()) == 0 && ((getCommand(cmd->getShortName()) == 0) || (cmd->getShortName() == "")))
        commands_.push_back(cmd);
    else {
        // One of the names existed, so throw an exception
        throw VoreenException(cmd->getName() + " or " + cmd->getShortName() + " was already assigned in this parser");
    }
}

void CommandlineParser::addCommandForNamelessArguments(Command* cmd) throw (VoreenException) {
    if (commandForNamelessArguments_ == 0) {
        // The command for nameless arguments wasn't already set
        if (cmd->getAllowMultipleCalls())
            throw VoreenException("Nameless command mustn't be allowed to be called multiple times");
        else
            commandForNamelessArguments_ = cmd;
    }
    else
        throw VoreenException("There was already a nameless command assigned to this parser");
}

void CommandlineParser::displayUsage() {
    string usageString = "Usage: ";

    if (programName_ != "")
        usageString += programName_ + " ";

    if (commandForNamelessArguments_ != 0)
        usageString += commandForNamelessArguments_->usage() + " ";

    FORALLCOMMANDS {
        if (*iter)
            usageString += "\n" + (*iter)->usage() + " ";
    }

    // Display via the std-out because no Logger-Prefix is wanted with the output
    std::cout << usageString << std::endl;
}

void CommandlineParser::displayHelp() {
    displayUsage();
    std::cout << std::endl << std::endl << "Help:" << std::endl << "-----" << std::endl;

    FORALLCOMMANDS {
        std::cout << (*iter)->help() << std::endl;
    }

    exit(EXIT_FAILURE);
}

Command* CommandlineParser::getCommand(std::string shortOrLongName) {
    FORALLCOMMANDS {
        if (((*iter)->getName() == shortOrLongName) || ((*iter)->getShortName() == shortOrLongName))
            return (*iter);
    }
    return 0;
}

inline void CommandlineParser::exitWithError(const std::string& msg) {
    std::cout << msg << std::endl << std::endl;
    displayUsage();
    exit(EXIT_FAILURE);
}

} // namespace
