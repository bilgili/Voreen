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

#ifndef VRN_COMMANDS_MODIFY_H
#define VRN_COMMANDS_MODIFY_H

#include "voreen/core/cmdparser/command.h"

namespace voreen {

struct BrickingInformation;
class Volume;

class CommandCutToPieces : public Command {
public:
    CommandCutToPieces();
    bool checkParameters(const std::vector<std::string>& parameters);
    bool execute(const std::vector<std::string>& parameters);
};

class CommandScale : public Command {
public:
    CommandScale();
    bool checkParameters(const std::vector<std::string>& parameters);
    bool execute(const std::vector<std::string>& parameters);
};

class CommandMirrorZ : public Command {
public:
    CommandMirrorZ();
    bool execute(const std::vector<std::string>& parameters);
};

class CommandSubSet : public Command {
public:
    CommandSubSet();
    bool checkParameters(const std::vector<std::string>& parameters);
    bool execute(const std::vector<std::string>& parameters);
};

class CommandBrick : public Command {
public:
    CommandBrick();
    bool execute(const std::vector<std::string>& parameters);
    bool checkParameters(const std::vector<std::string>& parameters);
    /**
    * Gets information like spacing etc from a volume. This function is used to fill
    * the brickingInformation struct with the necessary information to brick the volume.
    */
    void getVolumeInformation(BrickingInformation& brickingInformation, Volume* volume);
};


class CommandScaleTexCoords : public Command {
public:
    CommandScaleTexCoords();
    bool execute(const std::vector<std::string>& parameters);
};


}   //namespace voreen

#endif //VRN_COMMANDS_CONVERT_H
