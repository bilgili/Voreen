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

// slice/volume
#ifdef SLICE_TEXTURE_MODE_2D
uniform sampler2D sliceTex_;               // slice texture
uniform TextureParameters sliceTexParams_; // slice texture parameters
#else defined(SLICE_TEXTURE_MODE_3D)
uniform sampler3D volume_;                 // volume data set
uniform VolumeParameters volumeParams_;
#endif

// transfer functions
uniform TF_SAMPLER_TYPE transFuncTex_;
uniform TransFuncParameters transFuncParams_;
#if NUM_CHANNELS > 1
uniform TF_SAMPLER_TYPE transFuncTex2_;
uniform TransFuncParameters transFuncParams2_;
#endif
#if NUM_CHANNELS > 2
uniform TF_SAMPLER_TYPE transFuncTex3_;
uniform TransFuncParameters transFuncParams3_;
#endif
#if NUM_CHANNELS > 3
uniform TF_SAMPLER_TYPE transFuncTex4_;
uniform TransFuncParameters transFuncParams4_;
#endif

void main() {
    // fetch intensity
#ifdef SLICE_TEXTURE_MODE_2D
    vec4 intensity = textureLookup2Dnormalized(sliceTex_, sliceTexParams_, gl_TexCoord[0].xy);
    intensity *= sliceTexParams_.rwmScale_;
    intensity += sliceTexParams_.rwmOffset_;
#else if defined(SLICE_TEXTURE_MODE_3D)
    vec4 intensity = getVoxel(volume_, volumeParams_, gl_TexCoord[0].xyz);
    // NOTE: real-world-mapping is already applied by getVoxel()
    //intensity.a *= volumeParams_.rwmScale_;
    //intensity.a += volumeParams_.rwmOffset_;
#endif

    // compositing mode: add channels
    vec4 result;
#if NUM_CHANNELS == 1 // assuming alpha-texture
    result = applyTF(transFuncParams_, transFuncTex_, intensity.a);
#elif NUM_CHANNELS == 2 // assuming luminance-alpha texture
    vec4 channel1 = applyTF(transFuncParams_, transFuncTex_, intensity.r);
    vec4 channel2 = applyTF(transFuncParams2_, transFuncTex2_, intensity.g);
    result = clamp(channel1 + channel2, vec4(0.0), vec4(1.0));
#elif NUM_CHANNELS == 3 // assuming RGB texture
    vec4 channel1 = applyTF(transFuncParams_, transFuncTex_, intensity.r);
    vec4 channel2 = applyTF(transFuncParams2_, transFuncTex2_, intensity.g);
    vec4 channel3 = applyTF(transFuncParams3_, transFuncTex3_, intensity.b);
    result = clamp(channel1 + channel2 + channel3, vec4(0.0), vec4(1.0));
#elif NUM_CHANNELS == 4 // assuming RGBA texture
    vec4 channel1 = applyTF(transFuncParams_, transFuncTex_, intensity.r);
    vec4 channel2 = applyTF(transFuncParams2_, transFuncTex2_, intensity.g);
    vec4 channel3 = applyTF(transFuncParams3_, transFuncTex3_, intensity.b);
    vec4 channel4 = applyTF(transFuncParams4_, transFuncTex4_, intensity.a);
    result = channel1 + channel2 + channel3 + channel4;
#else // more than four channels => unknown texture type => only use alpha-value
    result = applyTF(transFuncParams_, transFuncTex_, intensity.a);
#endif

    vec4 fragColor = result;
    FragData0 = fragColor;

    if (result.a > 0.0)
        gl_FragDepth = gl_FragCoord.z;
    else
        gl_FragDepth = 1.0;
}

