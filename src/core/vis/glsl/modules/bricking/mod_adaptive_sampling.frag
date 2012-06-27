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
* This file supplies the function to determine the samplings
* that can be saved by adaptive sampling. Adaptive Sampling
* means skipping samplings in bricks with lower resolutions.
* By skipping a certain number of samples (instead of simply 
* increasing the sampling distance) it is guaranteed that
* one does not "jump" into the next brick and does not skip
* potentially high resolution samples. For more information 
* see the paper by Patrick Ljung ("Adaptive Sampling in
* Single Pass, GPU-based Raycasting of Multiresolution Volumes")
*/ 

//Declare the two variables globally since they are needed
//at multiple places in each shader using adaptive sampling. 

/**
* The number of samples that can be skipped.
*/
int numberOfSkippedSamples;

/**
* The ray direction multiplied by tIncr (aka step size between
* two samples). This multiplication is done after normalization 
* of the the ray direction. 
*/
vec3 directionWithStepSize;

/**
* Calculates the number of samples that can be skipped in
* low resolution bricks. 
*/
int getNumberOfSkippedSamples(vec3 sample, vec3 direction) {

    //All calculations are done in block coordinates, so convert the
    //sample position.
	vec3 blockCoordinates = convertToBlockCoordinates(sample); 
	
    //Convert the ray direction as well. 
    direction = convertToBlockCoordinates(direction);

	vec3 intraBlockCoordinates = fract(blockCoordinates);

    //For more information about the calculation, see
    //Patrick Ljung's paper.
	vec3 temp;
	if (direction.x < 0.0)
		temp.x = 0.0;
	else temp.x = 1.0;

	if (direction.y < 0.0)
		temp.y = 0.0;
	else temp.y = 1.0;

	if (direction.z < 0.0)
		temp.z = 0.0;
	else temp.z = 1.0;

	float term1 = (temp.x - intraBlockCoordinates.x) / direction.x;
	float term2 = (temp.y - intraBlockCoordinates.y) / direction.y;
	float term3 = (temp.z - intraBlockCoordinates.z) / direction.z;

	float steps = min(term1 , term2);
	steps = min(steps , term3);
	
	float numberOfSkippedSamples = min (1.0 + floor(steps), currentScaleFactor);
	int result = int(numberOfSkippedSamples);

	return result;
}
