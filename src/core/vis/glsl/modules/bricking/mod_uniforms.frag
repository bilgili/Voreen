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
* This file contains all the uniforms passed to the shader that have
* anything to do with bricking. Some of these uniforms are absolutely
* necessary, others are "just" for reducing rendering time.
*/

//The packed texture.
uniform sampler3D packedVolume_;                             
uniform VOLUME_PARAMETERS packedVolumeParameters_;          

//The index texture.
uniform sampler3D indexVolume_;                            
uniform VOLUME_PARAMETERS indexVolumeParameters_;

//The number of bricks in each dimension.
uniform float numbricksX_;
uniform float numbricksY_;
uniform float numbricksZ_;

// The bricksize in each dimension in normal texture coordinates € [0,1].
// Even though a brick has the same number of voxels in each dimensions,
// its size isn't necessarily the same in each dimensions. If the packed
// texture is very flat, all bricks might fit next to each other in one long
// row. Then brickSizeZ_ and brickSizeY_ would be 1.0, while brickSizeX_ would
// be much smaller.
uniform float brickSizeX_;
uniform float brickSizeY_;
uniform float brickSizeZ_;

// The factor necessary to retrieve the original values written to the index texture.
// For example, if a brick starts at (96,32,64) in the packed texture and you write these
// values to the index texture, the values you will read from it in the shader will all be
// € [0, 1]^3. Multiplying with this factor will retrieve the original value.
uniform float indexVolumeFactorX_;
uniform float indexVolumeFactorY_;
uniform float indexVolumeFactorZ_;

// The factor necessary to calculate a position in the packed texture. The packed texture
// and the original volume don't have the same dimensions, thus the same sample coordinates
// would have different meaning. These factors take care of that. 
uniform float offsetFactorX_;
uniform float offsetFactorY_;
uniform float offsetFactorZ_;

// These values are only passed to save rendering time by reducing the number of 
// computations in the shader. If someone knows better names for them, don't hesitate
// to change them. 
uniform float temp1;    //1.0f / (2.0f * numbricksX_);
uniform float temp2;    //1.0f / (2.0f * numbricksY_);
uniform float temp3;    //1.0f / (2.0f * numbricksZ_);
uniform float temp4;    //1.0f / (2.0f * maxbricksize_);
uniform float temp5;    //1.0f / numbricksX_;
uniform float temp6;    //1.0f / numbricksY_;
uniform float temp7;    //1.0f / numbricksZ_;
uniform float boundaryX_;   //1.0 - (1.0 / numbricksX)
uniform float boundaryY_;   //1.0 - (1.0 / numbricksY)
uniform float boundaryZ_;   //1.0 - (1.0 / numbricksZ)

// If interpolation coarseness is enabled (when normal coarseness is turned on,
// intrablock interpolation will be used instead of interblock interpolation),
// this uniform indicates whether or not coarseness is enabled at the moment. 
#ifdef INTERPOLATION_COARSENESS
 uniform bool coarsenessOn_;
#endif