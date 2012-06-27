/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2007 Visualization and Computer Graphics Group, *
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
 * in the file "license.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * information about commercial licencing please contact the authors. *
 *                                                                    *
 **********************************************************************/

#include "voreen/core/volume/bricking/brick.h"
#include <cmath>

namespace voreen {

    Brick::Brick(tgt::ivec3 pos, tgt::ivec3 dimensions) {
        position_=pos;
        dimensions_=dimensions;
    }

    Brick::~Brick() {
    }

    tgt::ivec3 Brick::getDimensions() {
        return dimensions_;
    }

    tgt::ivec3 Brick::getPosition() {
        return position_;
    }

    /* The implemtation of all those virtual functions
    that are of no use in classes other than VolumeBrick */

    char* Brick::getVolume() {
        return 0;
    }

    void Brick::setCurrentLevelOfDetail(size_t) {
    }

    size_t Brick::getCurrentLevelOfDetail() {
        return 0;
    }

    tgt::vec3 Brick::getLlf() {
        return tgt::vec3(0.0f);
    }

    bool Brick::getLevelOfDetailChanged() {
        return true;
    }

    void Brick::setLevelOfDetailChanged(bool) {
    }

    size_t Brick::getOldLevelOfDetail() {
        return 0;
    }

    void Brick::setOldLevelOfDetail(size_t) {
    }

    void Brick::setAllVoxelsEqual(bool) {
    }

    bool Brick::getAllVoxelsEqual() {
        return true;
    }

    uint64_t Brick::getBvFilePosition() {
        return 0;
    }

    void Brick::setBvFilePosition(uint64_t) {

    }

    void Brick::addError(float) {
    }

    float Brick::getError(size_t) {
        return -1.0f;
    }
} //namespace voreen

