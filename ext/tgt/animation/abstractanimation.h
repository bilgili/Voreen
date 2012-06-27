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


#ifndef ABSTRACTANIMATION_H_
#define ABSTRACTANIMATION_H_

#include "tgt/vector.h"
#include <QPixmap>

namespace tgt {

/**
 * <ul>
 * <li>given some <emph>reference points</emph> with their position in <emph>time index</emph> and some step <emph>speed</emph> (fps/totalFps)</li>
 * <li>any <emph>AbstractAnimation</emph> should return a suitable triple (up,center,eye) of vectors for the camera</li>
 * <li>for selected "current" position in time index specified by frame ({@link #moveToFrame(int)})</li>
 * </ul>
 */
class AbstractAnimation {
public:
    /**
     * <tex>\frac{fps}{totalFps}\hat{=}frameInSecond</tex>
     * @param fps
     * @param totalFps
     */
	AbstractAnimation(int fps, int numFrames) :
        fps_(fps),
        numFrames_(numFrames)
	{
    }

    AbstractAnimation() {
    }

    virtual ~AbstractAnimation() {
    }

    /**
     * setup the getters
     */
    virtual void moveToFrame(int frame) = 0;

    virtual vec3 getUp() = 0;
    virtual vec3 getCenter() = 0;
    virtual vec3 getEye() = 0;

protected:
    int fps_;
    int numFrames_;

};

/**
 * stolen member type of AnimationPlugin
 */
struct KeyFrame {
    vec3 position_;
    vec3 focus_;
    vec3 up_;
    double timeToNextFrame_;
    QPixmap pic_;
};

} // namespace tgt

#endif /* ABSTRACTANIMATION_H_ */
