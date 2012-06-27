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

#ifndef VRN_COMMAND_H
#define VRN_COMMAND_H 

#include <string>
#include <vector>

namespace voreen {

/** 
 * This exception will be thrown, if there is some kind of irreparable syntax error in a
 * commandline operation.
 */
class SyntaxException {
public:
    virtual ~SyntaxException() {}
    SyntaxException(std::string msg) : message_(msg) {}
    virtual std::string getMessage() { return message_; }
private:
    std::string message_;
};


/**
 * This class represents commands with can be called via commandline arguments. The constructor,
 * <code>execute</code>, <code>usage</code> and <code>checkParameters</code> must be overwritten
 * if you derive a new command from this one.
 * A newly created command must be added to a commandline parser in order to be used.
 * \sa CommandlineParser
 */
class Command {
public:
    Command();
    virtual ~Command() {}
    
    /// Returns the name of this command
    const std::string getName();

    /// Returns the short name of this command
    const std::string getShortName();
    
    /// Returns the parameter list necessary for the <code>usage</code> method
    const std::string getParameterList();

    /// Returns a short description used for the -help command
    const std::string getInfo();
    
    /**
     * Executes this command with the given parameters 
     * \param parameters The parameters needed for the execution of this command. The first entry
     * in this vector is the command itself
     * \return true, if the execution was successful, false otherwise
     */
    virtual bool execute(const std::vector<std::string>& parameters) = 0;

    ///Returns the usage-part for a command. Used in the usage()-method in the commandlineparser
    const std::string usage();

    /**
     * Checks the parameters for consistency and number
     * \return true, if the parameters are correct
     */
    virtual bool checkParameters(const std::vector<std::string>& parameters) = 0;

protected:
    /**
     * Convert string to int.
     * \param s Float in an integers pelt
     * \return The int value
     * \throws SyntaxException If the conversion fails
     */
    int castInt(const std::string& s) throw (SyntaxException);

    /**
     * Convert string to float.
     * \param s Float in a strings pelt
     * \return The float value
     * \throws SyntaxException If the conversion fails
     */
    float castFloat(const std::string& s) throw (SyntaxException);

    /**
     * Tests if the string contains an int value.
     * \param s The possible int disguised as a string
     * \return true, if s contains an int; false otherwise
     */
    bool isInt(const std::string& s);

    /**
     * Tests if the string contains a float value.
     * \param s The possible float disguised as a string
     * \return true, if s contains an float; false otherwise
     */
    bool isFloat(const std::string& s);

    /// Name of the command used on commandline level
    std::string name_;
    /// The short name of this command (usually an abbreviation
    std::string shortName_;
    /// The parameter list necessary for the <code>usage</code> method
    std::string parameterList_;
    /// Name used as a prefix for logging 
    std::string loggerCat_;
};

}   //namespace voreen

#endif //VRN_COMMAND_H
