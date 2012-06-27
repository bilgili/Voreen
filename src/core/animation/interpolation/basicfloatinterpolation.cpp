/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/core/animation/interpolation/basicfloatinterpolation.h"
#include <math.h>

namespace voreen {

float BasicFloatInterpolation::linearInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    return (1-time)*startvalue+time*endvalue;
}

float BasicFloatInterpolation::inQuadInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = static_cast<float>(pow(time,2));
    return startvalue+multiplikator*(endvalue-startvalue);
}
float BasicFloatInterpolation::inCubicInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = static_cast<float>(pow(time,3));
    return startvalue+multiplikator*(endvalue-startvalue);
}
float BasicFloatInterpolation::inQuartInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = static_cast<float>(pow(time,4));
    return startvalue+multiplikator*(endvalue-startvalue);
}
float BasicFloatInterpolation::inQuintInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = static_cast<float>(pow(time,5));
    return startvalue+multiplikator*(endvalue-startvalue);
}
float BasicFloatInterpolation::inSineInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = 1-(static_cast<float>(sin((1-time)/2*3.14159265)));
    return startvalue+multiplikator*(endvalue-startvalue);
}
float BasicFloatInterpolation::inExponentInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = static_cast<float>(( pow(2,10*(time-1)) -0.0001));
    return startvalue+multiplikator*(endvalue-startvalue);
}
float BasicFloatInterpolation::inCircInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = static_cast<float>((1-(sqrt(1-pow(time,2)))));
    return startvalue+multiplikator*(endvalue-startvalue);
}


float BasicFloatInterpolation::outQuadInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = 1-(static_cast<float>(pow((1-time),2)));
    return startvalue+multiplikator*(endvalue-startvalue);
}
float BasicFloatInterpolation::outCubicInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = 1-(static_cast<float>(pow((1-time),3)));
    return startvalue+multiplikator*(endvalue-startvalue);
}
float BasicFloatInterpolation::outQuartInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = 1-(static_cast<float>(pow((1-time),4)));
    return startvalue+multiplikator*(endvalue-startvalue);
}
float BasicFloatInterpolation::outQuintInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = 1-(static_cast<float>(pow((1-time),5)));
    return startvalue+multiplikator*(endvalue-startvalue);
}
float BasicFloatInterpolation::outSineInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = static_cast<float>(sin(time/2*3.14159265));
    return startvalue+multiplikator*(endvalue-startvalue);
}
float BasicFloatInterpolation::outExponentInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = static_cast<float>((1.001* (1-pow(2,0-10*time))));
    return startvalue+multiplikator*(endvalue-startvalue);
}
float BasicFloatInterpolation::outCircInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    float multiplikator = static_cast<float>((sqrt(1-pow((1-time),2))));
    return startvalue+multiplikator*(endvalue-startvalue);
}


float BasicFloatInterpolation::inOutQuadInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return inQuadInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return outQuadInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}
float BasicFloatInterpolation::inOutCubicInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return inCubicInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return outCubicInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}
float BasicFloatInterpolation::inOutQuartInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return inQuartInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return outQuartInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}
float BasicFloatInterpolation::inOutQuintInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return inQuintInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return outQuintInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}
float BasicFloatInterpolation::inOutSineInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return inSineInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return outSineInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}
float BasicFloatInterpolation::inOutExponentInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return inExponentInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return outExponentInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}
float BasicFloatInterpolation::inOutCircInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return inCircInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return outCircInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}


float BasicFloatInterpolation::outInQuadInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return outQuadInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return inQuadInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}
float BasicFloatInterpolation::outInCubicInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return outCubicInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return inCubicInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}
float BasicFloatInterpolation::outInQuartInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return outQuartInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return inQuartInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}
float BasicFloatInterpolation::outInQuintInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return outQuintInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return inQuintInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}
float BasicFloatInterpolation::outInSineInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return outSineInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return inSineInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}
float BasicFloatInterpolation::outInExponentInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return outExponentInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return inExponentInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}
float BasicFloatInterpolation::outInCircInterpolation(float startvalue, float endvalue, float time){
    if (time<0) return startvalue;
    if (time>1) return endvalue;
    if (time<0.5)
        return outCircInterpolation(startvalue, (startvalue+endvalue)/2, time*2);
    else
        return inCircInterpolation((startvalue+endvalue)/2, endvalue , time*2-1);
}

} // namespace voreen
