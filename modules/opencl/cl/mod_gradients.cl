/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

__constant sampler_t gradSmp = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

float4 calcGradient(float4 sample, image3d_t volumeTex) {

    float4 offset = (float4)(1.f) / convert_float4(get_image_dim(volumeTex));
    float4 gradient;

    float v0  = read_imagef(volumeTex, gradSmp, sample - (float4)(offset.x, 0.0, 0.0, 0.0)).x;
    float v1  = read_imagef(volumeTex, gradSmp, sample - (float4)(0.0, offset.y, 0.0, 0.0)).x;
    float v2  = read_imagef(volumeTex, gradSmp, sample - (float4)(0.0, 0.0, offset.z, 0.0)).x;
    float v3  = read_imagef(volumeTex, gradSmp, sample + (float4)(offset.x, 0.0, 0.0, 0.0)).x;
    float v4  = read_imagef(volumeTex, gradSmp, sample + (float4)(0.0, offset.y, 0.0, 0.0)).x;
    float v5  = read_imagef(volumeTex, gradSmp, sample + (float4)(0.0, 0.0, offset.z, 0.0)).x;
    gradient.x = v0 - v3;
    gradient.y = v1 - v4;
    gradient.z = v2 - v5;
    gradient.w = 0.0;

    return gradient;
}

float4 calcFilteredGradient(float4 sample, image3d_t volumeTex) {
    float4 offset = (float4)(1.f) / convert_float4(get_image_dim(volumeTex));

    float4 g0 = calcGradient(sample, volumeTex);
    float4 g1 = calcGradient(sample + (float4)(-offset.x, -offset.y, -offset.z, 0.0), volumeTex);
    float4 g2 = calcGradient(sample + (float4)( offset.x,  offset.y,  offset.z, 0.0), volumeTex);
    float4 g3 = calcGradient(sample + (float4)(-offset.x,  offset.y, -offset.z, 0.0), volumeTex);
    float4 g4 = calcGradient(sample + (float4)( offset.x, -offset.y,  offset.z, 0.0), volumeTex);
    float4 g5 = calcGradient(sample + (float4)(-offset.x, -offset.y,  offset.z, 0.0), volumeTex);
    float4 g6 = calcGradient(sample + (float4)( offset.x,  offset.y, -offset.z, 0.0), volumeTex);
    float4 g7 = calcGradient(sample + (float4)(-offset.x,  offset.y,  offset.z, 0.0), volumeTex);
    float4 g8 = calcGradient(sample + (float4)( offset.x, -offset.y, -offset.z, 0.0), volumeTex);

    float4 mix0 = mix(mix(g1, g2, 0.5f), mix(g3, g4, 0.5f), 0.5f);
    float4 mix1 = mix(mix(g5, g6, 0.5f), mix(g7, g8, 0.5f), 0.5f);
    return mix(g0, mix(mix0, mix1, 0.5f), 0.75f);
}
