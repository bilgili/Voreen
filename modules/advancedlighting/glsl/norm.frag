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

uniform sampler3D volume_;                      // volume data set
uniform ivec3 volSize_;
uniform int curSlice_;

vec3 delta;
varying out vec4 FragData0;

vec3 calcNormal(in vec3 pos) {
    vec3 gradient = vec3(0.0);

    //left plane
    float v000 = texture3D(volume_, pos + vec3(-delta.x,-delta.y,-delta.z)).a;
    float v001 = texture3D(volume_, pos + vec3(-delta.x, -delta.y, 0.)).a;
    float v002 = texture3D(volume_, pos + vec3(-delta.x, -delta.y, delta.z)).a;
    float v010 = texture3D(volume_, pos + vec3(-delta.x, 0., -delta.z)).a;
    float v011 = texture3D(volume_, pos + vec3(-delta.x, 0., 0.)).a;
    float v012 = texture3D(volume_, pos + vec3(-delta.x, 0., delta.z)).a;
    float v020 = texture3D(volume_, pos + vec3(-delta.x, delta.y, -delta.z)).a;
    float v021 = texture3D(volume_, pos + vec3(-delta.x, delta.y, 0.)).a;
    float v022 = texture3D(volume_, pos + vec3(-delta.x, delta.y, delta.z)).a;

    //mid plane
    float v100 = texture3D(volume_, pos + vec3(0., -delta.y, -delta.z)).a;
    float v101 = texture3D(volume_, pos + vec3(0., -delta.y, 0.)).a;
    float v102 = texture3D(volume_, pos + vec3(0., -delta.y, delta.z)).a;
    float v110 = texture3D(volume_, pos + vec3(0., 0., -delta.z)).a;
    float v112 = texture3D(volume_, pos + vec3(0., 0., delta.z)).a;
    float v120 = texture3D(volume_, pos + vec3(0., delta.y, -delta.z)).a;
    float v121 = texture3D(volume_, pos + vec3(0., delta.y, 0.)).a;
    float v122 = texture3D(volume_, pos + vec3(0., delta.y, delta.z)).a;

    //right plane
    float v200 = texture3D(volume_, pos + vec3(delta.x, -delta.y, -delta.z)).a;
    float v201 = texture3D(volume_, pos + vec3(delta.x, -delta.y, 0.)).a;
    float v202 = texture3D(volume_, pos + vec3(delta.x, -delta.y, delta.z)).a;
    float v210 = texture3D(volume_, pos + vec3(delta.x, 0., -delta.z)).a;
    float v211 = texture3D(volume_, pos + vec3(delta.x, 0., 0.)).a;
    float v212 = texture3D(volume_, pos + vec3(delta.x, 0., delta.z)).a;
    float v220 = texture3D(volume_, pos + vec3(delta.x, delta.y, -delta.z)).a;
    float v221 = texture3D(volume_, pos + vec3(delta.x, delta.y, 0.)).a;
    float v222 = texture3D(volume_, pos + vec3(delta.x, delta.y, delta.z)).a;

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

vec3 calcNormalFiltered(in vec3 samplePos) {
    vec3 g0 = calcNormal(samplePos);
    vec3 g1 = calcNormal(samplePos+vec3(-delta.x, -delta.y, -delta.z));
    vec3 g2 = calcNormal(samplePos+vec3( delta.x,  delta.y,  delta.z));
    vec3 g3 = calcNormal(samplePos+vec3(-delta.x,  delta.y, -delta.z));
    vec3 g4 = calcNormal(samplePos+vec3( delta.x, -delta.y,  delta.z));
    vec3 g5 = calcNormal(samplePos+vec3(-delta.x, -delta.y,  delta.z));
    vec3 g6 = calcNormal(samplePos+vec3( delta.x,  delta.y, -delta.z));
    vec3 g7 = calcNormal(samplePos+vec3(-delta.x,  delta.y,  delta.z));
    vec3 g8 = calcNormal(samplePos+vec3( delta.x, -delta.y, -delta.z));

    vec3 mix0 = mix(mix(g1, g2, 0.5), mix(g3, g4, 0.5), 0.5);
    vec3 mix1 = mix(mix(g5, g6, 0.5), mix(g7, g8, 0.5), 0.5);
    return normalize(mix(g0, mix(mix0, mix1, 0.5), 0.75));
}

/***
 * The main method.
 ***/
void main() {
    // Calculate the tex-coordinate of the current voxel
    vec3 pos = vec3(gl_FragCoord.xy, curSlice_) / vec3(volSize_);
    pos.z += 0.5 / float(volSize_.z);

    delta = 1.0 / vec3(volSize_);

    vec3 result = calcNormalFiltered(pos);

    FragData0 = vec4(0.5 * result + 0.5, 1.0);
}

