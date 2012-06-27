/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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
 * This module contains all functions which can be used for masking
 * voxels during ray traversal. Currently a voxel can be masked by
 * applying a segmentation.
 * The according functions are referenced by RC_NOT_MASKED which is
 * used in the raycaster fragment shaders.
 */

#if defined(USE_SEGMENTATION)
    uniform float segment_;                                // id of the segment to be shown
    uniform sampler3D segmentation_;                    // segmentation volume
    uniform VOLUME_PARAMETERS segmentationParameters_;  // texture lookup parameters for segmentation_
#endif

/***
 * Returns true if the current sample lies in the current segment,
 * otherwise false.
 */
bool inSegmentation(vec3 sample) {
    #if defined(USE_SEGMENTATION)
        float seg = textureLookup3D(segmentation_, segmentationParameters_, sample).a;
        if (abs(seg - segment_) < 0.00001)
            return true;
        else
            return false;
    #else
        return true;
    #endif
}
