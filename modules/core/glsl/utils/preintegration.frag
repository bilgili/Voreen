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

#include "modules/mod_sampler2d.frag"

uniform sampler1D tfTex_;
uniform TextureParameters texParams_;

//uniform int resolution_;

uniform float samplingStepSize_;

void main() {
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 fragCoordNorm = gl_FragCoord.xy * texParams_.dimensionsRCP_;

    vec4 fragColor = vec4(0.0);

    //if intensity values at start and end of the segment are equal: lookup transfer function
    if (fragCoord.x == fragCoord.y)
#if defined(GLSL_VERSION_130)
        fragColor = texture(tfTex_, fragCoordNorm.x);
#else
        fragColor = texture1D(tfTex_, fragCoordNorm.x);
#endif
    else {
        //else: perform compositing in transfer function space
        int sf = int(fragCoord.x);
        int sb = int(fragCoord.y);
        float scale = abs(float(sf - sb));

        int incr = 1;
        if (sf < sb) incr = -1;

        //fragColor = vec4(0.0);

        for (int s = sb; (incr == 1 ? s <= sf : s >= sf); s += incr) {
#if defined(GLSL_VERSION_130)
            vec4 curColor = texture(tfTex_, s * texParams_.dimensionsRCP_.x);
#else
            vec4 curColor = texture1D(tfTex_, s * texParams_.dimensionsRCP_.x);
#endif

            if (curColor.a > 0.0f) {
                // apply opacity correction to accomodate for variable sampling intervals
                curColor.a = 1.f - pow(1.f - curColor.a, samplingStepSize_ * 200.0f * scale);

                //actual compositing
                vec3 result_rgb = fragColor.xyz + (1.0f - fragColor.a) * curColor.a * curColor.xyz;
                fragColor.a = fragColor.a + (1.0f - fragColor.a) * curColor.a;

                fragColor.xyz = result_rgb;
            }
        }
    }

    FragData0 = fragColor;
}
