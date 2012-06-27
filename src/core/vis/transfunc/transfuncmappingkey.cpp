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

#include "voreen/core/vis/transfunc/transfuncmappingkey.h"

namespace voreen {

TransFuncMappingKey::TransFuncMappingKey(float i, const tgt::col4& color) : intensity_(i), 
    colorL_(color), colorR_(color), split_(false)
{}

TransFuncMappingKey::~TransFuncMappingKey()
{}

void TransFuncMappingKey::setColorL(const tgt::col4& color) {
    colorL_ = color;
    if(!split_)
        colorR_ = color;
}

void TransFuncMappingKey::setColorL(const tgt::ivec4& color) {
    colorL_ = tgt::col4(color);
    if(!split_)
        colorR_ = tgt::col4(color);
}

tgt::col4& TransFuncMappingKey::getColorL() {
    return colorL_;
}

void TransFuncMappingKey::setColorR(const tgt::col4& color) {
    colorR_ = color;
    if(!split_)
        colorL_ = color;
}

void TransFuncMappingKey::setColorR(const tgt::ivec4& color) {
    colorR_ = tgt::col4(color);
    if(!split_)
        colorL_ = tgt::col4(color);
}

tgt::col4& TransFuncMappingKey::getColorR() {
    return colorR_;
}

void TransFuncMappingKey::setAlphaR(float a) {
    colorR_.a = (uint8_t) (a*255.0);
    if(!split_)
        colorL_.a = (uint8_t) (a*255.0);
}

void TransFuncMappingKey::setAlphaL(float a) {
    colorL_.a = (uint8_t) (a*255.0);
    if(!split_)
        colorR_.a = (uint8_t) (a*255.0);
}

float TransFuncMappingKey::getAlphaR() {
    return colorR_.a / 255.0f; 
}

float TransFuncMappingKey::getAlphaL() {
    return colorL_.a / 255.0f; 
}

bool TransFuncMappingKey::isSplit() {
    return split_; 
}

void TransFuncMappingKey::setSplit(bool split, bool useLeft/*=true*/) {
    if(split_ == split)
        return;
    if(!split) {
        //join colors:
        if(useLeft)
            colorR_ = colorL_;
        else
            colorL_ = colorR_;
    }
    split_ = split; 
}

float TransFuncMappingKey::getIntensity() {
    return intensity_; 
}

void TransFuncMappingKey::setIntensity(float i) {
    intensity_ = i; 
}


} // namespace
