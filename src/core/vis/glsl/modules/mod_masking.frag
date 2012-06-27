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
 * This module contains all functions which can be used for masking
 * voxels during ray traversal. Currently a voxel can be masked based
 * on its intensity value through thresholding, or alternatively by
 * applying a segmentation.
 * The according functions are referenced by RC_NOT_MASKED which is
 * used in the raycaster fragment shaders.
 */

// THRESHOLDING

uniform float lowerThreshold_;
uniform float upperThreshold_;

/***
 * Returns true if the given intensity lies in the threshold interval.
 */
bool inThresholdInterval(in float intensity) {
	return (intensity >= lowerThreshold_ && intensity <= upperThreshold_);
}


// SEGMENTATION

#if defined(USE_SEGMENTATION)
	uniform float segment_;								// id of the segment to be shown
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
		if (step(abs(seg - segment_/255.0), (1.0 / 255.0) / 2.0) >= 0.0) return true;
		else return false;
	#else
		return true;
	#endif
}



// THE FOLLOWING FUNCTION IS OBSOLETE
// it can be removed when rc_simple is removed

/***
 * Returns 1 if the current sample belongs to the current segment
 * else 0.
 */
float applySegmentation(vec3 sample) {
	return 0.0;
	/*
    float seg = textureLookup3D(segmentation_, segmentationParameters_, sample).a;
    return step(abs(seg - segment_/255.0), (1.0 / 255.0) / 2.0);
	*/
}
