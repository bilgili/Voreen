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

#ifdef MOD_APPLY_SEGMENTATION

uniform sampler2D segmentationTransferFunc_;

vec4 applySegmentationClassification(vec3 sample, vec4 voxel, sampler3D segmentation, VOLUME_PARAMETERS segmentationParameters) {

    // Determine segment id and perform transfer function lookup within corresponding segmentation transfer function.
    // The 1D transfer function of a segment is stored in the 2D segmentation tf texture as a 3-row wide stripe which is centered around the row 3*i+1.

    float segmentScaleFactor = 255.0;
    if (segmentationParameters.bitDepth_ == 12)
        segmentScaleFactor = 4095.0;
    else if (segmentationParameters.bitDepth_ == 16)
        segmentScaleFactor = 65535.0;

    float segValue = getVoxel(segmentation, segmentationParameters, sample).a;
    float segment = segValue * segmentScaleFactor;

    return texture2D(segmentationTransferFunc_, vec2(voxel.a, (segment*3.0+1.0)/float(SEGMENTATION_TRANSFUNC_HEIGHT)) );

}

#endif
