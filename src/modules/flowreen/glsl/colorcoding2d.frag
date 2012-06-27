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

#include "mod_colorcoding.frag"

uniform sampler2D vectorField2D_;

float getFlowMagnitude(const vec2 r) {
    vec2 v = texture2D(vectorField2D_, r).xy;
    if (v != vec2(0.0)) {
        v = (v * (maxValue_ - minValue_)) + minValue_;
        return length(v);
    }
    return 0.0;
}

vec4 getColorFromFlowDirection(const vec2 r) {
    vec2 v = texture2D(vectorField2D_, r).xy;
    if (v != vec2(0.0))
        v = normalize((v * (maxValue_ - minValue_)) + minValue_);
    return vec4(abs(v.x), 0.0, abs(v.y), 1.0);
}

void main() {
    // look up the flow at the position and calculate the color
    //
    float magnitude = getFlowMagnitude(gl_TexCoord[0].st);

#if COLOR_MODE == COLOR_MODE_DIRECTION
    gl_FragColor = getColorFromFlowDirection(gl_TexCoord[0].st);
#elif COLOR_MODE == COLOR_MODE_MONOCHROME
    gl_FragColor = clamp(color_, vec4(0.0), vec4(1.0));
#else
    gl_FragColor = getColorFromFlowMagnitude(magnitude);
#endif
}
