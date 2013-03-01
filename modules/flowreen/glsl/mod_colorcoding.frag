/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#ifndef MOD_COLORTABLES_FRAG
#define MOD_COLORTABLES_FRAG

#define COLOR_TABLE_RAINBOW     0
#define COLOR_TABLE_HOT_METAL   1

#define COLOR_MODE_MAGNITUDE    0
#define COLOR_MODE_DIRECTION    1
#define COLOR_MODE_MONOCHROME   2

uniform float minValue_;
uniform float maxValue_;
uniform float maxMagnitude_;
uniform vec2 thresholds_;
uniform vec4 color_;

bool useThresholds_ = false;

vec4 getColor(const float m) {
#if COLOR_TABLE == COLOR_TABLE_HOT_METAL
    const int colorTableSize = 4;
    vec3 colorTable[colorTableSize];
    colorTable[0] = vec3(0.0, 0.0, 0.0);    // black
    colorTable[1] = vec3(1.0, 0.0, 0.0);    // red
    colorTable[2] = vec3(1.0, 1.0, 0.0);    // yellow
    colorTable[3] = vec3(1.0, 1.0, 1.0);    // white
#else
    const int colorTableSize = 6;
    vec3 colorTable[colorTableSize];
    colorTable[0] = vec3(0.0, 0.0, 0.0);    // black
    colorTable[1] = vec3(0.0, 0.0, 1.0);    // blue
    colorTable[2] = vec3(0.0, 1.0, 1.0);    // cyan
    colorTable[3] = vec3(0.0, 1.0, 0.0);    // green
    colorTable[4] = vec3(1.0, 1.0, 0.0);    // yellow
    colorTable[5] = vec3(1.0, 0.0, 0.0);    // red
#endif

    float numColors = float(colorTableSize - 1);
    float v = clamp(m * numColors, 0.0, numColors);
    ivec2 limits = clamp(ivec2(int(v), int(ceil(v))), 0, colorTableSize);
    vec3 color = mix(colorTable[limits.x], colorTable[limits.y], fract(v));
    return vec4(color, 1.0);
}

vec4 getColorFromFlowMagnitude(const float magnitude) {
    float maxMagnitude = maxMagnitude_;
    if (maxMagnitude <= 0.0) {
        float maxComponentValue = max(abs(maxValue_), abs(minValue_));
        maxMagnitude = sqrt(3.0 * (maxComponentValue * maxComponentValue));
    }
    useThresholds_ = (thresholds_ != vec2(0.0));
    if (useThresholds_ == true) {
        maxMagnitude = thresholds_.y;
        if ((magnitude < thresholds_.x) || (magnitude > thresholds_.y))
            return vec4(0.0, 0.0, 0.0, 1.0);
    }
    if  (magnitude > maxMagnitude)
        return vec4(1.0, 1.0, 1.0, 1.0);
    else
        return getColor(magnitude / maxMagnitude);
}

#endif
