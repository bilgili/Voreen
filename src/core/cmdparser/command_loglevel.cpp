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

#include "voreen/core/cmdparser/command_loglevel.h"

#include "tgt/logmanager.h"

namespace voreen {

Command_LogLevel::Command_LogLevel(tgt::LogLevel* dbgLevel) {
    name_ = "--loglevel";
    shortName_ = "-l";
    parameterList_ = "<debug|warning|info|error|fatal>";
    dbgLevel_ = dbgLevel;
}

bool Command_LogLevel::execute(const std::vector<std::string>& parameters) {
    std::string argument = parameters.at(1);
    if (argument == "debug")
        *dbgLevel_ = tgt::Debug;
    else if (argument == "warning")
        *dbgLevel_ = tgt::Warning;
    else if (argument == "info")
        *dbgLevel_ = tgt::Info;
    else if (argument == "error")
        *dbgLevel_ = tgt::Error;
    else if (argument == "fatal")
        *dbgLevel_ = tgt::Fatal;
    else // This case should not happen
        return false;

    return true;
}

bool Command_LogLevel::checkParameters(const std::vector<std::string>& parameters) {
    if (parameters.size() != 2)
        return false;

    std::string argument = parameters.at(1);

    bool argumentCorrect =  ((argument == "debug")  || 
                             (argument == "warning") ||
                             (argument == "info")    ||
                             (argument == "error")   ||
                             (argument == "fatal")
                             );

    return argumentCorrect;
}

} // namespace
