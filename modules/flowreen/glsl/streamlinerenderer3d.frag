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

#include "mod_phong.frag"
#include "mod_colorcoding.frag"

varying float depth;

uniform sampler3D vectorField3D_;
uniform vec2 clippingPlanes_;

float getFlowMagnitude(const vec3 r) {
    vec3 v = texture3D(vectorField3D_, r).xyz;
    if (v != vec3(0.0)) {
        v = (v * (maxValue_ - minValue_)) + minValue_;
        return length(v);
    }
    return 0.0;
}

void main()
{
    float magnitude = getFlowMagnitude(gl_TexCoord[0].xyz);
    vec3 color = getColorFromFlowMagnitude(magnitude).xyz;
    if (color == vec3(0.0))
        discard;

    float alpha = 1.0;

    // if near and far clipping planes are specified, fade of distance objects using alpha
    // values and the function
    //
    // f(d) = 1/far * (1/x - 1.0f), d in [0.0, 1.0]
    //
    if (clippingPlanes_ != vec2(0.0)) {
        float d = ((depth + clippingPlanes_.x) / (clippingPlanes_.x + clippingPlanes_.y));
        float c = 1.0 / clippingPlanes_.y;
        alpha = (d < c) ? 1.0 : ((c / d) - c);
    }

    if (usePhongShading_ == true) {
        vec3 phongTerms = (useForLines_ == false) ? phongShading() : phongShadingForLines();
        FragData0 = vec4((color * ka_) + (color * kd_ * phongTerms.y) + (color * ks_ * phongTerms.z), alpha);
    } else
        FragData0 = vec4(color, alpha);
}
