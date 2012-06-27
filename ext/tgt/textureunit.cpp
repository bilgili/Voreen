/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "tgt/textureunit.h"
#include "tgt/gpucapabilities.h"
#include "tgt/assert.h"

namespace tgt {

bool TextureUnit::initialized_ = false;
unsigned short TextureUnit::totalActive_ = 0;
unsigned short TextureUnit::maxTexUnits_ = 0;
unsigned short TextureUnit::numKeptUnits_ = 0;
std::vector<bool> TextureUnit::busyUnits_ = std::vector<bool>();

TextureUnit::TextureUnit(bool keep)
    : number_(0)
    , glEnum_(0)
    , assigned_(false)
    , keep_(keep)
{
    if (!initialized_)
        init();
}

TextureUnit::~TextureUnit() {
    if (assigned_ && !keep_) {
        busyUnits_.at(number_) = false;
        totalActive_--;
    }
}

void TextureUnit::activate() const {
    if (!assigned_)
        assignUnit();
    glActiveTexture(getEnum());
}

GLint TextureUnit::getEnum() const {
    if (!assigned_)
        assignUnit();
    return glEnum_;
}

GLint TextureUnit::getUnitNumber() const {
    if (!assigned_)
        assignUnit();
    return number_;
}

void TextureUnit::setZeroUnit() {
    glActiveTexture(GL_TEXTURE0);
}

void TextureUnit::cleanup() {
    for (size_t i = 0; i < maxTexUnits_; i++) {
        if (busyUnits_.at(i))
            busyUnits_.at(i) = false;
    }
    totalActive_ = 0;
    numKeptUnits_ = 0;
    setZeroUnit();
}

bool TextureUnit::unused() {
    return (totalActive_ == 0);
}

unsigned short TextureUnit::numLocalActive() {
    return (totalActive_ - numKeptUnits_);
}

void TextureUnit::assignUnit() const {
    tgtAssert(totalActive_ <= maxTexUnits_, "No more texture units available");

    assigned_ = true;

    for (size_t i = 0; i < maxTexUnits_; i++) {
        if (!busyUnits_.at(i)) {
            number_ = (GLint)i;
            busyUnits_.at(i) = true;
            totalActive_++;
            if(keep_)
                numKeptUnits_++;
            break;
        }
    }

    glEnum_ = GL_TEXTURE0 + number_;
}

void TextureUnit::init() {
    maxTexUnits_ = GpuCaps.getNumTextureUnits();
    busyUnits_ = std::vector<bool>(maxTexUnits_, false);
    initialized_ = true;
    numKeptUnits_ = 0;
}

} // namespace
