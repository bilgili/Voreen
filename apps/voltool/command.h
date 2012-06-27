/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include "tgt/logmanager.h"

namespace voreen {

///This exception is caught by voltool and syntax is displayed to the user.
class SyntaxException {
public:
    virtual ~SyntaxException() {}
    SyntaxException(std::string msg) : message_(msg) {}
    virtual std::string getMessage() { return message_; }
private:
    std::string message_;
};

///Base command class, overwrite constructor and execute(..) to create custom commands
class Command {
protected:
    std::string help_;
    std::string info_;
    std::string syntax_;
    std::string name_;
    std::string loggerCat_;
    
public:
    Command() { loggerCat_ = "voreen.command"; }
    virtual ~Command() {}
    
    ///Returns name
    virtual std::string getName() { return name_; }
    ///Returns long help text
    virtual std::string getHelp() { return help_; }
    ///Returns short description
    virtual std::string getInfo() { return info_; }
    ///Returns syntax
    virtual std::string getSyntax()  { return syntax_; }
    
    ///Execute this command with the given parameters
    ///Can throw SyntaxException, IOException, etc.
    virtual bool execute(const std::vector<std::string> & parameters) = 0;
protected:
    ///Convert string to int, throws SyntaxException if conversion fails
    int asInt(const std::string& s) throw(SyntaxException);
    ///Convert string to float, throws SyntaxException if conversion fails
    float asFloat(const std::string& s) throw(SyntaxException);
    ///Throws SyntaxException with fitting message if c is false
    ///Example: checkParameters(parameters.size() == 2);
    void checkParameters(bool c) throw(SyntaxException);
    ///Throws SyntaxException with fitting message
    void illegalParameter() throw(SyntaxException);
};

///Manages registered commands
class CommandMap {
    std::map<std::string, Command*> commandMap_;
public:
    CommandMap();
    ~CommandMap();
    
    ///Add command to map. All commands will be deleted in destructor
    void addCommand(Command* cmd);
    ///Check if command is available
    bool available(std::string name);
    ///Get Command from map:
    Command* get(std::string name);
    ///List all available commands (using logging, level info)
    void listCommands();
};

}   //namespace voreen

#endif //VRN_COMMAND_H


