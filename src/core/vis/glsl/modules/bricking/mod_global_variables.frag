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
* This file contains a lot of variables that are needed at a lot of different
* places and functions during the interpolation. Instead of passing them to each 
* function as an argument (which wouldn't look very nice), they are declared 
* globally (which is not elegant either, but better). 
*/

/**
* The coordinates of each brick in normal coordinates (i.e. € [0,1[^3). The 
* coordinates indicate the lower-left-front corner of the bricks. (That's why
* bricks can't start at 1.0, they would be outside the volume). 
* Bricks are numbered as shown in the "picture" below. 
*
*   7 ---------8
*  /|         /|
* 3 ---------4 |
* | |        | |
* | |        | |
* | |        | |
* | 5--------|-6
* |/         |/
* 1 ---------2
*
*/
vec3 llfBlock,llbBlock,ulfBlock,ulbBlock,lrfBlock,lrbBlock,urfBlock,urbBlock;

/**
* The weight of each edge in interblock interpolation. "edge12" means
* the edge between brick 1 and 2. 
*/
float edge12,edge13,edge15,edge24,edge26,edge34,edge37;
float edge48,edge56,edge57,edge68,edge78;


/**
* The index volume sample for each brick. These values are needed often.
*/
vec4 llfBlockIndexVolSample,llbBlockIndexVolSample,ulfBlockIndexVolSample,ulbBlockIndexVolSample;
vec4 lrfBlockIndexVolSample,lrbBlockIndexVolSample,urfBlockIndexVolSample,urbBlockIndexVolSample;

/**
* The boundary distance of each block, i.e. the distance between the brick boundary and the bounding
* box encompassing all the samples in the brick. This distance is needed several times, and storing
* it globally saves recalculating it from the scalefactor every time.
*/
float llfBlockBoundaryDistance,llbBlockBoundaryDistance,ulfBlockBoundaryDistance,ulbBlockBoundaryDistance;
float lrfBlockBoundaryDistance,lrbBlockBoundaryDistance,urfBlockBoundaryDistance,urbBlockBoundaryDistance;

/**
* The fact whether or not a brick is inside the volume. During interblock interpolation the eight brick
* neighborhood is established, and some of them might not be inside the volume. If inside the volume,
* the value will be 1.0, otherwise 0.0.
*/
float llfBlockValid,llbBlockValid,ulfBlockValid,ulbBlockValid,lrfBlockValid,lrbBlockValid;
float urfBlockValid,urbBlockValid;

/**This is the scalefactor of the brick holding the current sample. It is needed when using adaptive 
* sampling, as the number of samples that can be skipped depends on it. 
*/
float currentScaleFactor;