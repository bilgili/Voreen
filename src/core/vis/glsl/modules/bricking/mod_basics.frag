/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

/**
* As mod_sampler3d.frag is usually not included if bricking is used,
* these functions have to be defined elsewhere to be usable for bricking.
* Simply including it here won't work, as some aspects of the functions have
* to be different.
*/

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
    float bitDepthScale_;
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

