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

#include "circleanimation.h"

#include "tgt/camera.h"

namespace tgt {

CircleAnimation::CircleAnimation(float totalAngle, int duration, int fps, Camera* const camera)
    : AbstractAnimation(fps, static_cast<int>((totalAngle / 360.0f) * fps * duration)),
    stepAngle_(0.0f),
    currentAngle_(0.0f),
    rotation_(mat3::identity),
    initialCamPos_(camera->getPosition()),
    initialCamUpVector_(camera->getUpVector()),
    initialCamFocus_(camera->getFocus()),
    camPos_(initialCamPos_),
    camFocus_(initialCamFocus_)
{
    stepAngle_ = (totalAngle / numFrames_);
    rotation_ = mat3::createRotation(deg2rad(stepAngle_), initialCamUpVector_);
}

void CircleAnimation::moveToFrame(int frame) {
    currentAngle_ = (stepAngle_ * frame);
}

vec3 CircleAnimation::getUp() {
    return initialCamUpVector_;
}

vec3 CircleAnimation::getCenter() {
    camFocus_ = (rotation_ * (camFocus_ - initialCamFocus_));
    camFocus_ += initialCamFocus_;
    return camFocus_;
}

vec3 CircleAnimation::getEye() {
    camPos_ = (rotation_ * (camPos_ - initialCamFocus_));
    camPos_ += initialCamFocus_;
    return camPos_;
}

} // namespace tgt

