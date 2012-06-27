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

CircleAnimation::CircleAnimation(float totalAngle, int speed, int fps, tgt::Trackball* trackBall) :
    AbstractAnimation(fps, totalAngle/360.0f * static_cast<float>(fps * speed)),
	trackBall_(trackBall) {
		// compute step angle in radian
		stepAngle_ = (totalAngle / numFrames_) * (PIf / 180.0f);
}

void CircleAnimation::moveToFrame(int /*frame*/) {
    trackBall_->rotate(tgt::vec3(0.f, 1.f, 0.f), stepAngle_);
}

vec3 CircleAnimation::getUp() {
	return trackBall_->getCamera()->getUpVector();
}

vec3 CircleAnimation::getCenter() {
	return trackBall_->getCamera()->getFocus();
}

vec3 CircleAnimation::getEye() {
	return trackBall_->getCamera()->getPosition();
}

} // namespace tgt

