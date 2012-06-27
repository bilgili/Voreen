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

#ifndef CIRCLEANIMATION_H_
#define CIRCLEANIMATION_H_

#include "abstractanimation.h"
#include "tgt/camera.h"
#include "tgt/navigation/trackball.h"

namespace tgt {

/**
 * Performs a rotation around the focus point.
 */
class CircleAnimation: public AbstractAnimation {
public:
	CircleAnimation(float totalAngle, int speed, int fps, tgt::Trackball* trackBall);

    virtual ~CircleAnimation() {}

public:
    virtual void moveToFrame(int /*frame*/);
    virtual vec3 getUp();
    virtual vec3 getCenter();
    virtual vec3 getEye();

private:
    double stepAngle_;
    tgt::Trackball* trackBall_;
};

} // namespace tgt

#endif // CIRCLEANIMATION_H
