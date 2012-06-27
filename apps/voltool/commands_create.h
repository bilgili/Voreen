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

#ifndef VRN_COMMANDS_CREATE_H
#define VRN_COMMANDS_CREATE_H

#include "voreen/core/cmdparser/command.h"
#include "voreen/core/volume/volumeatomic.h"

namespace voreen {

class CommandGenerateMask : public Command {
public:
    CommandGenerateMask();
    bool checkParameters(const std::vector<std::string>& parameters);
    bool execute(const std::vector<std::string>& parameters);
};

class CommandCreate : public Command {
public:
    CommandCreate();
    bool checkParameters(const std::vector<std::string>& parameters);
    bool execute(const std::vector<std::string>& parameters);

protected:
    void fillPlane(VolumeUInt8* vds, tgt::vec3 center, tgt::vec3 normal, uint8_t value);
    void fillCircle(VolumeUInt8* vds, tgt::vec3 center, float radius, uint8_t value);
    void fillOrientedCircle(VolumeUInt8* vds, tgt::vec3 center, tgt::vec3 normal, float radius, uint8_t value);
    void fillSphere(VolumeUInt8* vds, tgt::vec3 center, float radius, uint8_t value);
    void fillEllipsoid(VolumeUInt8* vds, tgt::vec3 center, tgt::vec3 radius, uint8_t value);
    void fillBox(VolumeUInt8* vds, tgt::ivec3 start, tgt::ivec3 end, uint8_t value);
    void fillOrientedBox(VolumeUInt8* vds, tgt::vec3 center, tgt::vec3 dir, float lengthA, float lengthB, float yStart, float yEnd, uint8_t value);
    void applyPerturbation(Volume* vds, tgt::ivec3 dimensions, tgt::vec3 frequency, tgt::vec3 amplitude);
};

}   //namespace voreen

#endif //VRN_COMMANDS_CREATE_H
