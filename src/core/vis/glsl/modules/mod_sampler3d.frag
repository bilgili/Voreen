/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

/*
 * This struct contains information about a volume, like
 * its dimensions and spacing. Additionally, the reciprocal
 * values of all parameters are available (suffix RCP) .
 * The values are set automatically by
 * VolumeRenderer::bindVolumes() if necessary.
 */
struct VOLUME_PARAMETERS {
    vec3 datasetDimensions_;        // the dataset's resolution, e.g. [ 256.0, 128.0, 128.0]
    vec3 datasetDimensionsRCP_;
    vec3 datasetSpacing_;           // set dataset's voxel size, e.g. [ 2.0, 0.5, 1.0]
    vec3 datasetSpacingRCP_;
    vec3 volumeCubeSize_;           // the volume's size in its object coordinates, e.g. [ 1.0, 0.5, 0.5]
    vec3 volumeCubeSizeRCP_;
    vec3 texCoordScaleFactor_;      // scale factor for tex coords, if VRN_TEXTURE_3D_SCALED is used
    vec3 texCoordScaleFactorRCP_;
    int bitDepth_;                  // the volume's bit depth
    float bitDepthScale_;           // scaling factor that must be applied for normalizing the fetched texture value.
                                    // currently just used for 12 bit volumes, which actually use only 12 out of 16 bits.
};

/*
 * Function for volume texture lookup. In addition to the volume and the texture coordinates
 * the corresponding VOLUME_PARAMETERS struct has to be passed.
 * Before returning the fetched value it is normalized to the interval [0,1], in order to deal
 * with 12 bit data sets.
 */
vec4 textureLookup3D(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 texCoords) {
    vec4 result;
    #if defined(VRN_TEXTURE_3D)
        result = texture3D(volume, texCoords);
    #elif defined(VRN_TEXTURE_3D_SCALED)
        result = texture3D(volume, texCoords*volumeParameters.texCoordScaleFactor_);
    #endif
    result *= volumeParameters.bitDepthScale_;
    return result;
}


/*
 * Function for volume texture lookup. In addition to the volume and the texture coordinates
 * the corresponding VOLUME_PARAMETERS struct has to be passed .
 * In contrast to textureLookup3D() this function does not normalize the intensity values,
 * in order to deal with 12 bit data sets.
 */
vec4 textureLookup3DUnnormalized(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 texCoords) {
    #if defined(VRN_TEXTURE_3D)
        return texture3D(volume, texCoords);
    #elif defined(VRN_TEXTURE_3D_SCALED)
        return texture3D(volume, texCoords*volumeParameters.texCoordScaleFactor_);
    #endif
}


/*
 * This function should be called by all raycasters in order to get the intensity from the volume.
 * In cases where volumeParameters indicates, that gradients are stored in the volume, these are
 * also fetched from the volume. Therefore, in addition to the volume and the texture coordinates
 * the corresponding VOLUME_PARAMETERS struct has to be passed.
 * Before returning the intensity value it is normalized to the interval [0,1], in order to deal
 * with 12 bit data sets.
 *
 * @return The result vec4 stores the intensity in the a channel and the gradient (if available)
 *         in the rgb channels.
 *
 */
vec4 getVoxel(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 sample) {
    return textureLookup3D(volume, volumeParameters, sample);
}

/*
 * This function should be called by all raycasters in order to get the intensity from the volume.
 * In cases where volumeParameters indicates, that gradients are stored in the volume, these are
 * also fetched from the volume. Therefore, in addition to the volume and the texture coordinates
 * the corresponding VOLUME_PARAMETERS struct has to be passed.
 * In contrast to getVoxel() this function does not normalize the intensity values, in order to deal
 * with 12 bit data sets.
 *
 * @return The result vec4 stores the intensity in the a channel and the gradient (if available)
 *         in the rgb channels.
 */
vec4 getVoxelUnnormalized(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 sample) {
    return textureLookup3DUnnormalized(volume, volumeParameters, sample);
}
