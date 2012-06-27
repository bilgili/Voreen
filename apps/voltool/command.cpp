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

#include "command.h"

namespace voreen {

int Command::asInt(const std::string& s) throw(SyntaxException) {
    std::istringstream iss(s);
    int t;
    bool success = !(iss >> std::dec >> t).fail();
    if(!success)
        throw SyntaxException("Failed to convert parameter to integer!");
    return t;
}

float Command::asFloat(const std::string& s) throw(SyntaxException) {
    std::istringstream iss(s);
    float t;
    bool success = !(iss >> std::dec >> t).fail();
    if(!success)
        throw SyntaxException("Failed to convert parameter to float!");
    return t;
}

void Command::checkParameters(bool c) throw(SyntaxException) {
    if(!c)
        throw SyntaxException("Illegal number of parameters!");
}

void Command::illegalParameter() throw(SyntaxException) {
    throw SyntaxException("Illegal parameter!");
}

//-----------------------------------------------------------------------------

CommandMap::CommandMap() {

}

CommandMap::~CommandMap() {
    //delete all commands:
    std::map<std::string, Command*>::iterator iter;   
    while(!commandMap_.empty()) {
        iter = commandMap_.begin();
        delete iter->second;
        commandMap_.erase(iter);
    }
}

void CommandMap::addCommand(Command* cmd) {
    commandMap_[cmd->getName()] = cmd;
}

bool CommandMap::available(std::string name) {
    if(commandMap_.find(name) != commandMap_.end())
        return true;
    else
        return false;
}

Command* CommandMap::get(std::string name) {
    return commandMap_[name];
}

void CommandMap::listCommands() {
    std::map<std::string, Command*>::iterator iter;   
    for( iter = commandMap_.begin(); iter != commandMap_.end(); iter++ ) {
        LINFOC("voreen.voltool", iter->first << ": " << iter->second->getInfo());
    }
}

}   //namespace voreen
