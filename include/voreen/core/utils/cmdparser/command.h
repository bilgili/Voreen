/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#ifndef VRN_COMMAND_H
#define VRN_COMMAND_H

#include <exception>
#include <sstream>
#include <string>
#include <vector>
#include <set>

#include "voreen/core/utils/exception.h"

namespace voreen {

/**
 * A command is a functionality which can be called via commandline on program startup. As this
 * class is virtual, you have to derive from it and add it (in the program) to a commandline parser.
 * In the parser, the command's <code>name</code> and <code>shortName</code> must be unique.
 *
 * There are other convenience classes to use, for example \sa Command_Boolean , \sa Command_Float ,
 * \sa Command_TwoFloat , \sa Command_Integer , \sa Command_TwoInteger , \sa Command_String,
 * \sa Command_TwoString which accept one or two of the listed parameter-types.
 *
 * If -for example- you want to add a class which accepts one float value and one bool value,
 * you would create a new class Command_FloatBool which derives from Command and implement some methods:
 * <code>Command_FloatBool(float* fp, bool* bp, const std::string& name, const std::string& shortName = ""
 * , const std::string& infoText = "", const std::string& parameterList = "")</code> and call the Command
 * constructor with the arguments <code> (name, shortName, infoText, parameterList, 2, false) </code>
 * If you want to be able to put the command multiple times on the commandline it would look like this:
 * <code>Command_FloatBool(std::vector<float>* fp, std::vector<bool>* bp, const std::string& name,
 * const std::string& shortName = "" , const std::string& infoText = "", const std::string& parameterList = "")
 * </code> and the last argument for the super-ctor would be true.
 * If you don't provide a vector as return value, you'll (obviously) get only one value back. This value will be
 * the result of the latest call of the command.
 *
 * If the <code>argumentNum</code> is equal to -1, the remaining parameters of the commandline will be used.
 * Of course, only one of those commands can be used reasonably.
 *
 * At least you have to write the methode \sa execute in which you do something with the passed values. For
 * example just parse them and write them back to the member variables.
 * If you want to do something fancy with \sa checkParameters , you have to write this too, because the version
 * of this class only checks the parameters for the right count.
 */
class Command {
public:
    /**
     * The constructor which just saves the arguments to out own member variables.
     * \param name The (long) name of the parameter. e.g. --command1
     * \param shortName The abbriviated name of the parameter. e.g. -c
     * \param infoText A short text (preferably one line) explaining what the command does. Used in the
     * \sa displayHelp() method from the commandlineparser
     * \param parameterList A description which parameters are used. In the example with one float and
     * one bool value, this would be <float value> <bool value>. This is used in the \sa displayUsage()
     * and \sa displayHelp() methods.
     * \param argumentNum The number of arguments this command accepts
     * \param allowMultipleCalls If is possible to have more than one instance of this command in one
     * commandline? E.g. program1 -a dosomething -b somethingelse -a doitagain.
     */
    Command(const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string& parameterList = "", const int argumentNum = 1, const bool allowMultipleCalls = false);
    virtual ~Command();

    /// Returns the name of this command
    const std::string getName();

    /// Returns the short name of this command
    const std::string getShortName();

    /// Returns the parameter list necessary for the <code>usage</code> method
    const std::string getParameterList();

    /// Returns a short description used for the <code>--help</code> command
    const std::string getInfoText();

    /// Returns the number of accepted arguments for this command
    int getArgumentNumber();

    /// Returns if the command can be called more than once in a single command line
    bool getAllowMultipleCalls();

    /**
     * Executes this command with the given parameters
     * \param parameters The parameters needed for the execution of this command.
     * \return true, if the execution was successful, false otherwise
     */
    virtual bool execute(const std::vector<std::string>& parameters) = 0;

    /**
     * Checks the parameters for consistency and number. This version only tests for the number
     * of parameters. If you want to test for other conditions, overwrite this method in a derived
     * class
     * \param parameters The parameters which should be tested
     * \return true, if the parameters are correct
     */
    virtual bool checkParameters(const std::vector<std::string>& parameters);

    ///Returns the usage-part for a command. Used in the usage()-method from the commandlineparser
    virtual const std::string usage();

    /// Returns the help-part for a command. Used in the help()-method from the commandlineparser
    virtual const std::string help();

protected:
    /**
     * Trys to cast the string value s to the templated parameter T
     * if this fails, an std::exception will be thrown
     * The conversion is done via an std::istringstream so it can only
     * cast those types supported by the stream
     */
    template <class T>
    T cast(const std::string& s) throw (std::exception) {
        std::istringstream iss(s);
        T t;
        if ((iss >> std::dec >> t).fail())
            throw VoreenException("Cast failed in Command");
        else
            return t;
    }

    /**
     * Checks if there is a corresponding value from the string value s to the
     * template type T. As the method trys to convert the value, only those types
     * are possible, which are supported by the std::istringstream
     */
    template <class T>
    bool is(const std::string& s) {
        std::istringstream iss(s);
        T t;
        return (iss >> std::dec >> t);
    }

    /**
     * Tests, if the 'value' is present in the given array.
     */
    bool isValueInSet(const std::string& value, const std::set<std::string>& set);

    /// Name of the command used on commandline level
    std::string name_;
    /// The short name of this command (usually an abbreviation
    std::string shortName_;
     /// A description of the command; used in the <code>help</code> method
     std::string infoText_;
    /// The parameter list necessary for the <code>usage</code> method
    std::string parameterList_;
    /// Name used as a prefix for logging
    std::string loggerCat_;
    /// Stores the number of arguments this command accepts
    int argumentNum_;
    /// Stores, if the command can be called multiple times in a single commandline
    bool allowMultipleCalls_;
};

}   //namespace voreen

#endif //VRN_COMMAND_H
