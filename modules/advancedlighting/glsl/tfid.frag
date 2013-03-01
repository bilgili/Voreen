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

#line 1

varying out vec4 FragData0;
varying out vec4 FragData1;

uniform sampler3D volume_;                      // volume data set
uniform sampler1D transferFunc_;
//uniform VolumeParameters volumeStruct_;
uniform ivec3 volSize_;
uniform int curSlice_;
//uniform float curSlice_;
uniform int pass_;

//const ivec3 delta = ivec3(1);

#ifdef SH_SUBSURFACE
vec3 calcNormal(in vec3 pos, in vec3 delta, in float level);
vec3 calcNormalFiltered(in vec3 samplePos, in vec3 delta, in float level);
#endif

/***
 * The main method.
 ***/
void main() {
    // Calculate the tex-coordinate of the current voxel
    //vec3 pos = vec3(gl_FragCoord.xy, curSlice_) / vec3(volSize_);
    //pos.z += 0.5 / float(volSize_.z);
    //ivec3 pos = ivec3(gl_FragCoord.xy, curSlice_);

    vec4 result = vec4(0.0);

    // During the first pass, we simply take the transfunc-value of the current voxel as the result
    if(pass_ == 0) {
        //vec3 pos = vec3(gl_FragCoord.xy, curSlice_) / vec3(volSize_);
        //result = texture1D(transferFunc_, texture3D(volume_, pos).a);

        ivec3 pos = ivec3(gl_FragCoord.xy, curSlice_);
        result = texture1D(transferFunc_, texelFetch(volume_, pos, 0).a);

        #ifdef SH_SUBSURFACE
        vec3 norm = calcNormalFiltered(pos, 1.0 / vec3(volSize_), 0.0);
        result.xyz = 0.5 * norm + 0.5;
        #endif
    } else {
        // In all the following passes, we consider 2x2x2-cubes of the previous level which are compressed
        // into one voxel in the current level.  This is done by calculating how much light passes through
        // each of the eight cubes from each other voxel in the 2x2x2-group (by simply multiplying the alpha-
        // values).  The sum of these eight transparency-values is the average amount of light that passes
        // trough this 2x2x2 neighbourhood, which we use as the alpha-value for the currently computed mipmap-level.

        //// look up the eight surrounding transfunc-values
        //ivec3 pos = ivec3(gl_FragCoord.xy, curSlice_);
        //vec4 v000 = texelFetch(volume_, pos, 0);
        //vec4 v001 = texelFetch(volume_, pos + ivec3(0, 0, 1), 0);
        //vec4 v010 = texelFetch(volume_, pos + ivec3(0, 1, 0), 0);
        //vec4 v011 = texelFetch(volume_, pos + ivec3(0, 1, 1), 0);
        //vec4 v100 = texelFetch(volume_, pos + ivec3(1, 0, 0), 0);
        //vec4 v101 = texelFetch(volume_, pos + ivec3(1, 0, 1), 0);
        //vec4 v110 = texelFetch(volume_, pos + ivec3(1, 1, 0), 0);
        //vec4 v111 = texelFetch(volume_, pos + ivec3(1, 1, 1), 0);

        // calculate the stepsize in this level to access a neighbourhood of eight voxels.  We have to use 2*volSize_,
        // since volSize_ holds the size of the current level, which is only 1/8 of the size of the previous level, in
        // which we perform the lookup
        vec3 delta = 1.0 / (2.0 * vec3(volSize_));
        vec3 pos = vec3(gl_FragCoord.xy, curSlice_) / vec3(volSize_);
        // look up the eight surrounding transfunc-values
        vec4 v000 = texture3D(volume_, pos);
        vec4 v001 = texture3D(volume_, pos + vec3(0., 0., delta.z));
        vec4 v010 = texture3D(volume_, pos + vec3(0., delta.y, 0.));
        vec4 v011 = texture3D(volume_, pos + vec3(0., delta.y, delta.z));
        vec4 v100 = texture3D(volume_, pos + vec3(delta.x, 0., 0.));
        vec4 v101 = texture3D(volume_, pos + vec3(delta.x, 0., delta.z));
        vec4 v110 = texture3D(volume_, pos + vec3(delta.x, delta.y, 0.));
        vec4 v111 = texture3D(volume_, pos + vec3(delta.x, delta.y, delta.z));

        // Calculate all the products. There are (8 over 2) = 28 unique pairs of voxels in the neighbourhood.
        // After those 28 products have been computed and summed up, we would have to duplicate that sum, since
        // we consider each voxel separately from each other, that is, every voxel has seven other voxel in the
        // neighbourhood that it interacts with - we don't only need VoxelA.a*VoxelB.a, but also VoxelB.a*VoxelA.a,
        // because the direction of the considered ray is important.
        // However, since we later have to divide by 8 to account for averaging the transparency for each voxel,
        // we might as well simply divide by 4 instead of multiplying by 2 and then dividing by 8.
        result.a =  v000.a*v001.a + v000.a*v010.a + v000.a*v011.a + v000.a*v100.a;
        result.a += v000.a*v101.a + v000.a*v110.a + v000.a*v111.a;
        result.a += v001.a*v010.a + v001.a*v011.a + v001.a*v100.a + v001.a*v101.a;
        result.a += v001.a*v110.a + v001.a*v111.a;
        result.a += v010.a*v011.a + v010.a*v100.a + v010.a*v101.a;
        result.a += v010.a*v110.a + v010.a*v111.a;
        result.a += v011.a*v100.a + v011.a*v101.a + v011.a*v110.a + v011.a*v111.a;
        result.a += v100.a*v101.a + v100.a*v110.a + v100.a*v111.a;
        result.a += v101.a*v110.a + v101.a*v111.a;
        result.a += v110.a*v111.a;

        result.a /= 4.0;

        // For the colors, we do a simple average-calculation
        result.xyz  = v000.xyz + v001.xyz + v010.xyz + v011.xyz;
        result.xyz += v100.xyz + v101.xyz + v110.xyz + v111.xyz;
        result.xyz /= 8.0;
        #ifdef SH_SUBSURFACE
        normalize(result.xyz);
        result.xyz = 0.5 * result.xyz + 0.5;
        #endif
    }

    // Save the result in the two render targets (see SHTransCoeff class for explanation as to why there are two targets)
    FragData0 = result;
    #ifdef SH_ERI
    FragData1 = result;
    #endif
    //gl_FragData[1] = vec4(1.0);
    //gl_FragData[1] = vec4(1.0 / pass_, 0.1 * pass_, 0.0, 1.0);
    //gl_FragData[1] = vec4(pos, 1.0);
    //gl_FragData[1] = vec4(pass_ * 0.2, 1.0 - pass_ * 0.2, 0.0, 1.0);
}

#ifdef SH_SUBSURFACE
vec3 calcNormal(in vec3 pos, in vec3 delta, in float level) {

    vec3 gradient = vec3(0.0);

    //left plane
    float v000 = texture3DLod(volume_, pos + vec3(-delta.x,-delta.y,-delta.z), level).a;
    float v001 = texture3DLod(volume_, pos + vec3(-delta.x, -delta.y, 0.), level).a;
    float v002 = texture3DLod(volume_, pos + vec3(-delta.x, -delta.y, delta.z), level).a;
    float v010 = texture3DLod(volume_, pos + vec3(-delta.x, 0., -delta.z), level).a;
    float v011 = texture3DLod(volume_, pos + vec3(-delta.x, 0., 0.), level).a;
    float v012 = texture3DLod(volume_, pos + vec3(-delta.x, 0., delta.z), level).a;
    float v020 = texture3DLod(volume_, pos + vec3(-delta.x, delta.y, -delta.z), level).a;
    float v021 = texture3DLod(volume_, pos + vec3(-delta.x, delta.y, 0.), level).a;
    float v022 = texture3DLod(volume_, pos + vec3(-delta.x, delta.y, delta.z), level).a;

    //mid plane
    float v100 = texture3DLod(volume_, pos + vec3(0., -delta.y, -delta.z), level).a;
    float v101 = texture3DLod(volume_, pos + vec3(0., -delta.y, 0.), level).a;
    float v102 = texture3DLod(volume_, pos + vec3(0., -delta.y, delta.z), level).a;
    float v110 = texture3DLod(volume_, pos + vec3(0., 0., -delta.z), level).a;
    float v112 = texture3DLod(volume_, pos + vec3(0., 0., delta.z), level).a;
    float v120 = texture3DLod(volume_, pos + vec3(0., delta.y, -delta.z), level).a;
    float v121 = texture3DLod(volume_, pos + vec3(0., delta.y, 0.), level).a;
    float v122 = texture3DLod(volume_, pos + vec3(0., delta.y, delta.z), level).a;

    //right plane
    float v200 = texture3DLod(volume_, pos + vec3(delta.x, -delta.y, -delta.z), level).a;
    float v201 = texture3DLod(volume_, pos + vec3(delta.x, -delta.y, 0.), level).a;
    float v202 = texture3DLod(volume_, pos + vec3(delta.x, -delta.y, delta.z), level).a;
    float v210 = texture3DLod(volume_, pos + vec3(delta.x, 0., -delta.z), level).a;
    float v211 = texture3DLod(volume_, pos + vec3(delta.x, 0., 0.), level).a;
    float v212 = texture3DLod(volume_, pos + vec3(delta.x, 0., delta.z), level).a;
    float v220 = texture3DLod(volume_, pos + vec3(delta.x, delta.y, -delta.z), level).a;
    float v221 = texture3DLod(volume_, pos + vec3(delta.x, delta.y, 0.), level).a;
    float v222 = texture3DLod(volume_, pos + vec3(delta.x, delta.y, delta.z), level).a;

    //filter x-direction
    gradient.x += -1. * v000;
    gradient.x += -3. * v010;
    gradient.x += -1. * v020;
    gradient.x += +1. * v200;
    gradient.x += +3. * v210;
    gradient.x += +1. * v220;
    gradient.x += -3. * v001;
    gradient.x += -6. * v011;
    gradient.x += -3. * v021;
    gradient.x += +3. * v201;
    gradient.x += +6. * v211;
    gradient.x += +3. * v221;
    gradient.x += -1. * v002;
    gradient.x += -3. * v012;
    gradient.x += -1. * v022;
    gradient.x += +1. * v202;
    gradient.x += +3. * v212;
    gradient.x += +1. * v222;

    //filter y-direction
    gradient.y += -1. * v000;
    gradient.y += -3. * v100;
    gradient.y += -1. * v200;
    gradient.y += +1. * v020;
    gradient.y += +3. * v120;
    gradient.y += +1. * v220;
    gradient.y += -3. * v001;
    gradient.y += -6. * v101;
    gradient.y += -3. * v201;
    gradient.y += +3. * v021;
    gradient.y += +6. * v121;
    gradient.y += +3. * v221;
    gradient.y += -1. * v002;
    gradient.y += -3. * v102;
    gradient.y += -1. * v202;
    gradient.y += +1. * v022;
    gradient.y += +3. * v122;
    gradient.y += +1. * v222;

    //filter z-direction
    gradient.z += -1. * v000;
    gradient.z += -3. * v100;
    gradient.z += -1. * v200;
    gradient.z += +1. * v002;
    gradient.z += +3. * v102;
    gradient.z += +1. * v202;
    gradient.z += -3. * v010;
    gradient.z += -6. * v110;
    gradient.z += -3. * v210;
    gradient.z += +3. * v012;
    gradient.z += +6. * v112;
    gradient.z += +3. * v212;
    gradient.z += -1. * v020;
    gradient.z += -3. * v120;
    gradient.z += -1. * v220;
    gradient.z += +1. * v022;
    gradient.z += +3. * v122;
    gradient.z += +1. * v222;

    gradient *= -1.0;

    return normalize(gradient);
}

vec3 calcNormalFiltered(in vec3 samplePos, in vec3 delta, in float level) {
    vec3 g0 = calcNormal(samplePos, delta, level);
    vec3 g1 = calcNormal(samplePos+vec3(-delta.x, -delta.y, -delta.z), delta, level);
    vec3 g2 = calcNormal(samplePos+vec3( delta.x,  delta.y,  delta.z), delta, level);
    vec3 g3 = calcNormal(samplePos+vec3(-delta.x,  delta.y, -delta.z), delta, level);
    vec3 g4 = calcNormal(samplePos+vec3( delta.x, -delta.y,  delta.z), delta, level);
    vec3 g5 = calcNormal(samplePos+vec3(-delta.x, -delta.y,  delta.z), delta, level);
    vec3 g6 = calcNormal(samplePos+vec3( delta.x,  delta.y, -delta.z), delta, level);
    vec3 g7 = calcNormal(samplePos+vec3(-delta.x,  delta.y,  delta.z), delta, level);
    vec3 g8 = calcNormal(samplePos+vec3( delta.x, -delta.y, -delta.z), delta, level);

    vec3 mix0 = mix(mix(g1, g2, 0.5), mix(g3, g4, 0.5), 0.5);
    vec3 mix1 = mix(mix(g5, g6, 0.5), mix(g7, g8, 0.5), 0.5);
    return normalize(mix(g0, mix(mix0, mix1, 0.5), 0.75));
}
#endif
