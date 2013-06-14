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

__constant sampler_t gradSmp = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

/**
 * This module contains all functions which can be used for calculating
 * gradients on the fly within a OpenCL raycaster.
 * The functions below are referenced by CALC_GRADIENT which is
 * used in the raycaster kernel.
 */


/**
 * Calculates a voxel's gradient in volume object space based on the red
 * channel using forward differences.
 *
 * @param volume the voxel's volume
 * @param volumeStruct additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */
float4 calcGradientRFD(image3d_t volumeTex, VolumeParameters volumeStruct, float4 samplePos) {
    float4 offset = volumeStruct.datasetDimensionsRCP_;

    float v = read_imagef(volumeTex, gradSmp, samplePos).x;
    float v0 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(offset.x, 0.0f, 0.0f, 0.0f)).x;
    float v1 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, offset.y, 0.0f, 0.0f)).x;
    float v2 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, 0.0f, offset.z, 0.0f)).x;

    float4 gradient = (float4)(v - v0, v - v1, v - v2, 0.0f);
    gradient *= volumeStruct.datasetSpacingRCP_;
    gradient *= volumeStruct.rwmScale_;
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the green
 * channel using forward differences.
 *
 * @param volume the voxel's volume
 * @param volumeStruct additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */
float4 calcGradientGFD(image3d_t volumeTex, VolumeParameters volumeStruct, float4 samplePos) {
    float4 offset = volumeStruct.datasetDimensionsRCP_;

    float v = read_imagef(volumeTex, gradSmp, samplePos).y;
    float v0 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(offset.x, 0.0f, 0.0f, 0.0f)).y;
    float v1 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, offset.y, 0.0f, 0.0f)).y;
    float v2 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, 0.0f, offset.z, 0.0f)).y;

    float4 gradient = (float4)(v - v0, v - v1, v - v2, 0.0f);
    gradient *= volumeStruct.datasetSpacingRCP_;
    gradient *= volumeStruct.rwmScale_;
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the blue
 * channel using forward differences.
 *
 * @param volume the voxel's volume
 * @param volumeStruct additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */
float4 calcGradientBFD(image3d_t volumeTex, VolumeParameters volumeStruct, float4 samplePos) {
    float4 offset = volumeStruct.datasetDimensionsRCP_;

    float v = read_imagef(volumeTex, gradSmp, samplePos).z;
    float v0 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(offset.x, 0.0f, 0.0f, 0.0f)).z;
    float v1 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, offset.y, 0.0f, 0.0f)).z;
    float v2 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, 0.0f, offset.z, 0.0f)).z;

    float4 gradient = (float4)(v - v0, v - v1, v - v2, 0.0f);
    gradient *= volumeStruct.datasetSpacingRCP_;
    gradient *= volumeStruct.rwmScale_;
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the alpha
 * channel using forward differences.
 *
 * @param volume the voxel's volume
 * @param volumeStruct additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */
float4 calcGradientAFD(image3d_t volumeTex, VolumeParameters volumeStruct, float4 samplePos) {
    float4 offset = volumeStruct.datasetDimensionsRCP_;

    float v = read_imagef(volumeTex, gradSmp, samplePos).w;
    float v0 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(offset.x, 0.0f, 0.0f, 0.0f)).w;
    float v1 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, offset.y, 0.0f, 0.0f)).w;
    float v2 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, 0.0f, offset.z, 0.0f)).w;

    float4 gradient = (float4)(v - v0, v - v1, v - v2, 0.0f);
    gradient *= volumeStruct.datasetSpacingRCP_;
    gradient *= volumeStruct.rwmScale_;
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the red
 * channel using central differences.
 *
 * @param volume the voxel's volume
 * @param volumeStruct additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */
float4 calcGradientR(image3d_t volumeTex, VolumeParameters volumeStruct, float4 samplePos) {
    float4 offset = volumeStruct.datasetDimensionsRCP_;

    float v0 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(offset.x, 0.0f, 0.0f, 0.0f)).x;
    float v1 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, offset.y, 0.0f, 0.0f)).x;
    float v2 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, 0.0f, offset.z, 0.0f)).x;
    float v3 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x, 0.0f, 0.0f, 0.0f)).x;
    float v4 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, -offset.y, 0.0f, 0.0f)).x;
    float v5 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, 0.0f, -offset.z, 0.0f)).x;

    float4 gradient = (float4)(v3 - v0, v4 - v1, v5 - v2, 0.0f);
    gradient *= volumeStruct.datasetSpacingRCP_ * 0.5;
    gradient *= volumeStruct.rwmScale_;
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the green
 * channel using central differences.
 *
 * @param volume the voxel's volume
 * @param volumeStruct additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */
float4 calcGradientG(image3d_t volumeTex, VolumeParameters volumeStruct, float4 samplePos) {
    float4 offset = volumeStruct.datasetDimensionsRCP_;

    float v0 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(offset.x, 0.0f, 0.0f, 0.0f)).y;
    float v1 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, offset.y, 0.0f, 0.0f)).y;
    float v2 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, 0.0f, offset.z, 0.0f)).y;
    float v3 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x, 0.0f, 0.0f, 0.0f)).y;
    float v4 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, -offset.y, 0.0f, 0.0f)).y;
    float v5 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, 0.0f, -offset.z, 0.0f)).y;

    float4 gradient = (float4)(v3 - v0, v4 - v1, v5 - v2, 0.0f);
    gradient *= volumeStruct.datasetSpacingRCP_ * 0.5;
    gradient *= volumeStruct.rwmScale_;
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the blue
 * channel using central differences.
 *
 * @param volume the voxel's volume
 * @param volumeStruct additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */
float4 calcGradientB(image3d_t volumeTex, VolumeParameters volumeStruct, float4 samplePos) {
    float4 offset = volumeStruct.datasetDimensionsRCP_;

    float v0 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(offset.x, 0.0f, 0.0f, 0.0f)).z;
    float v1 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, offset.y, 0.0f, 0.0f)).z;
    float v2 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, 0.0f, offset.z, 0.0f)).z;
    float v3 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x, 0.0f, 0.0f, 0.0f)).z;
    float v4 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, -offset.y, 0.0f, 0.0f)).z;
    float v5 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, 0.0f, -offset.z, 0.0f)).z;

    float4 gradient = (float4)(v3 - v0, v4 - v1, v5 - v2, 0.0f);
    gradient *= volumeStruct.datasetSpacingRCP_ * 0.5;
    gradient *= volumeStruct.rwmScale_;
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the alpha
 * channel using central differences.
 *
 * @param volume the voxel's volume
 * @param volumeStruct additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */
float4 calcGradientA(image3d_t volumeTex, VolumeParameters volumeStruct, float4 samplePos) {
    float4 offset = volumeStruct.datasetDimensionsRCP_;

    float v0 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(offset.x, 0.0f, 0.0f, 0.0f)).w;
    float v1 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, offset.y, 0.0f, 0.0f)).w;
    float v2 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, 0.0f, offset.z, 0.0f)).w;
    float v3 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x, 0.0f, 0.0f, 0.0f)).w;
    float v4 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, -offset.y, 0.0f, 0.0f)).w;
    float v5 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(0.0f, 0.0f, -offset.z, 0.0f)).w;

    float4 gradient = (float4)(v3 - v0, v4 - v1, v5 - v2, 0.0f);
    gradient *= volumeStruct.datasetSpacingRCP_ * 0.5;
    gradient *= volumeStruct.rwmScale_;
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the alpha
 * channel using the sobel operator.
 *
 * @param volume the voxel's volume
 * @param volumeStruct additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */
float4 calcGradientSobel(image3d_t volumeTex, VolumeParameters volumeStruct, float4 samplePos) {
    float4 offset = volumeStruct.datasetDimensionsRCP_;

    /*
     * [ 1 2 3 ] [ 10 11 12 ] [ 19 20 21 ]
     * [ 4 5 6 ] [ 13 14 15 ] [ 22 23 24 ]
     * [ 7 8 9 ] [ 16 17 18 ] [ 25 26 27 ]
     */

    float v01 = read_imagef(volumeTex, gradSmp, samplePos + (float4)( offset.x, -offset.y,   offset.z,   0.0f)).w;
    float v02 = read_imagef(volumeTex, gradSmp, samplePos + (float4)( offset.x,      0.0f,   offset.z,   0.0f)).w;
    float v03 = read_imagef(volumeTex, gradSmp, samplePos + (float4)( offset.x,  offset.y,   offset.z,   0.0f)).w;
    float v04 = read_imagef(volumeTex, gradSmp, samplePos + (float4)( offset.x, -offset.y,       0.0f,   0.0f)).w;
    float v05 = read_imagef(volumeTex, gradSmp, samplePos + (float4)( offset.x,      0.0f,       0.0f,   0.0f)).w;
    float v06 = read_imagef(volumeTex, gradSmp, samplePos + (float4)( offset.x,  offset.y,       0.0f,   0.0f)).w;
    float v07 = read_imagef(volumeTex, gradSmp, samplePos + (float4)( offset.x, -offset.y,  -offset.z,   0.0f)).w;
    float v08 = read_imagef(volumeTex, gradSmp, samplePos + (float4)( offset.x,      0.0f,  -offset.z,   0.0f)).w;
    float v09 = read_imagef(volumeTex, gradSmp, samplePos + (float4)( offset.x,  offset.y,  -offset.z,   0.0f)).w;

    float v10 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(     0.0f, -offset.y,  offset.z,   0.0f)).w;
    float v11 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(     0.0f,      0.0f,  offset.z,   0.0f)).w;
    float v12 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(     0.0f,  offset.y,  offset.z,   0.0f)).w;
    float v13 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(     0.0f, -offset.y,      0.0f,   0.0f)).w;
    //float v14 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(     0.0f,      0.0f,      0.0f,   0.0f)).w;
    float v15 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(     0.0f,  offset.y,      0.0f,   0.0f)).w;
    float v16 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(     0.0f, -offset.y, -offset.z,   0.0f)).w;
    float v17 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(     0.0f,      0.0f, -offset.z,   0.0f)).w;
    float v18 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(     0.0f,  offset.y, -offset.z,   0.0f)).w;

    float v19 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x, -offset.y,  offset.z,   0.0f)).w;
    float v20 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x,      0.0f,  offset.z,   0.0f)).w;
    float v21 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x,  offset.y,  offset.z,   0.0f)).w;
    float v22 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x, -offset.y,      0.0f,   0.0f)).w;
    float v23 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x,      0.0f,      0.0f,   0.0f)).w;
    float v24 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x,  offset.y,      0.0f,   0.0f)).w;
    float v25 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x, -offset.y, -offset.z,   0.0f)).w;
    float v26 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x,      0.0f, -offset.z,   0.0f)).w;
    float v27 = read_imagef(volumeTex, gradSmp, samplePos + (float4)(-offset.x,  offset.y, -offset.z,   0.0f)).w;

    float gx =      v01 + 2.0*v02 +     v03    /* 0 0 0 */   -     v19 - 2.0*v20 -     v21
               +2.0*v04 + 4.0*v05 + 2.0*v06    /* 0 0 0 */   - 2.0*v22 - 4.0*v23 - 2.0*v24
                   +v07 + 2.0*v08 +     v09    /* 0 0 0 */   -     v25 - 2.0*v26 -     v27;

    float gy =     -v01 /* 0 */ +     v03    - 2.0*v10 /* 0 */ + 2.0*v12    -     v19 /* 0 */ +     v21
               -2.0*v04 /* 0 */ + 2.0*v06    - 4.0*v13 /* 0 */ + 4.0*v15    - 2.0*v22 /* 0 */ + 2.0*v24
                   -v07 /* 0 */ +     v09    - 2.0*v16 /* 0 */ + 2.0*v18    -     v25 /* 0 */ +     v27;

    float gz = v01 + 2.0*v02 + v03    + 2.0*v10 + 4.0*v11 + 2.0*v12    + v19 + 2.0*v20 + v21
                   /* 0 0 0 */                 /* 0 0 0 */                    /* 0 0 0 */
              -v07 - 2.0*v08 - v09    - 2.0*v16 - 4.0*v17 - 2.0*v18    - v25 - 2.0*v26 - v27;

    float4 gradient = (float4)(-gx, -gy, -gz, 0.0f);
    gradient *= volumeStruct.datasetSpacingRCP_ * 0.5;
    gradient /=  16.0; // sum of all positive elements
    gradient *= volumeStruct.rwmScale_;
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space by calculating the
 * gradients for all eight neighbours based on the alpha channel using
 * central differences and filtering between the reuslts.
 *
 * @param volume the voxel's volume
 * @param volumeStruct additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */
float4 calcGradientFiltered(image3d_t volumeTex, VolumeParameters volumeStruct, float4 samplePos) {
    float4 delta = volumeStruct.datasetDimensionsRCP_;

    float4 g0 = calcGradientA(volumeTex, volumeStruct, samplePos);
    float4 g1 = calcGradientA(volumeTex, volumeStruct, samplePos + (float4)(-delta.x, -delta.y, -delta.z, 0.0f));
    float4 g2 = calcGradientA(volumeTex, volumeStruct, samplePos + (float4)( delta.x,  delta.y,  delta.z, 0.0f));
    float4 g3 = calcGradientA(volumeTex, volumeStruct, samplePos + (float4)(-delta.x,  delta.y, -delta.z, 0.0f));
    float4 g4 = calcGradientA(volumeTex, volumeStruct, samplePos + (float4)( delta.x, -delta.y,  delta.z, 0.0f));
    float4 g5 = calcGradientA(volumeTex, volumeStruct, samplePos + (float4)(-delta.x, -delta.y,  delta.z, 0.0f));
    float4 g6 = calcGradientA(volumeTex, volumeStruct, samplePos + (float4)( delta.x,  delta.y, -delta.z, 0.0f));
    float4 g7 = calcGradientA(volumeTex, volumeStruct, samplePos + (float4)(-delta.x,  delta.y,  delta.z, 0.0f));
    float4 g8 = calcGradientA(volumeTex, volumeStruct, samplePos + (float4)( delta.x, -delta.y, -delta.z, 0.0f));
    float4 mix0 = mix(mix(g1, g2, 0.5), mix(g3, g4, 0.5), 0.5);
    float4 mix1 = mix(mix(g5, g6, 0.5), mix(g7, g8, 0.5), 0.5);
    return mix(g0, mix(mix0, mix1, 0.5), 0.75);
}

/**
 * Calculates a voxel's gradient in volume object space based on the alpha
 * channel using central differences.
 *
 * @param volume the voxel's volume
 * @param volumeStruct additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */

float4 calcGradient(image3d_t volumeTex, VolumeParameters volumeStruct, float4 samplePos) {
    return calcGradientA(volumeTex, volumeStruct, samplePos);
}

