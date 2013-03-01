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
#include "modules/mod_sampler3d.frag"
#include "modules/mod_transfunc.frag"

#ifdef SLICE_TEXTURE_MODE_2D
    uniform sampler2D sliceTex_;               // slice texture
    uniform TextureParameters sliceTexParameters_; // slice texture parameters
    uniform float rwmScale_;                // RealWorldMapping slope
    uniform float rwmOffset_;               // RealWorldMapping intercept
#else defined(SLICE_TEXTURE_MODE_3D)
    uniform VolumeParameters volumeStruct_;
    uniform sampler3D volume_;    // volume data set
#endif

uniform TransFuncParameters transferFunc_;
uniform TF_SAMPLER_TYPE transferFuncTex_;

uniform int numChannels_;                       // number of channels in volume

vec3 rgb2hsv(vec3 colorRGB) {
    float minComponent = min(colorRGB.r, min(colorRGB.g, colorRGB.b));
    float maxComponent = max(colorRGB.r, max(colorRGB.g, colorRGB.b));
    float delta = maxComponent - minComponent;

    vec3 result;
    result.b = maxComponent;
    if (maxComponent != 0.0) result.g = delta / maxComponent;
    else result.g = 0.0;
    if (result.g == 0.0) result.r = 0.0; // no hue
    else {
        if (colorRGB.r == maxComponent) result.r = (colorRGB.g - colorRGB.b) / delta;
        else if (colorRGB.g == maxComponent) result.r = 2 + (colorRGB.b - colorRGB.r) / delta;
        else if (colorRGB.b == maxComponent) result.r = 4 + (colorRGB.r - colorRGB.g) / delta;
        result.r *= 60.0;
        if (result.r < 0.0) result.r += 360.0;
        result.r /= 360.0;
    }
    return result;
}

void main() {
    // fetch intensity
#ifdef SLICE_TEXTURE_MODE_2D
    vec4 intensity = textureLookup2Dnormalized(sliceTex_, sliceTexParameters_, gl_TexCoord[0].xy);
    intensity.a *= rwmScale_;
    intensity.a += rwmOffset_;
#else if defined(SLICE_TEXTURE_MODE_3D)
    vec4 intensity = getVoxel(volume_, volumeStruct_, gl_TexCoord[0].xyz);
#endif


    vec4 mat;

    if (numChannels_ == 1) {
        // Assuming Intensity only volume - get value via the transfer function.
        mat = applyTF(transferFunc_, transferFuncTex_, intensity.a);
    }
    else if (numChannels_ == 3) {
        // Assuming RGB volume - convert RGB to HSV and apply transfer function to hue.
        vec4 tfColor = applyTF(transferFunc_, transferFuncTex_, rgb2hsv(intensity.rgb).r);
        mat = vec4(intensity.rgb, tfColor.a);
    }
    else if (numChannels_ == 4) {
        // Assuming RGBA volume - no transfer function lookup necessary
        mat = intensity;
    }
    else {
        // something bad happened
        mat = vec4(1.0);
    }

    vec4 fragColor = mat;
    FragData0 = fragColor;

    if (mat.a > 0.0)
        gl_FragDepth = gl_FragCoord.z;
    else
        gl_FragDepth = 1.0;
}

