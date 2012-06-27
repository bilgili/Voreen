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

/**
 * This module contains all functions which are used for the raysetup
 * as well as the ray traversal within a raycaster.
 * RC_BEGIN_LOOP and RC_END_LOOP are used within the raycaster fragment
 * shaders to controll the ray traversal.
 */

uniform float raycastingQualityFactorRCP_;

/***
 * Calculates the direction of the ray and returns the number
 * of steps and the direction.
 ***/
void raySetup(in vec3 first, in vec3 last, out vec3 rayDirection, out float tIncr, out float tEnd) {
    // calculate ray parameters
    tIncr = 0.005 * raycastingQualityFactorRCP_;

    rayDirection = last - first;
    tEnd = length(rayDirection);
    rayDirection = normalize(rayDirection);
    #ifdef ADAPTIVE_SAMPLING
        directionWithStepSize = rayDirection * tIncr;
    #endif
}

/***
 * Applies early ray termination. The current opacity is compared to
 * the maximum opacity. In case it is greater, the opacity is set to
 * 1.0 and true is returned, otherwise false is returned.
 ***/
bool earlyRayTermination(inout float opacity, in float maxOpacity) {
    if (opacity >= maxOpacity) {
        opacity = 1.0;
        return true;
    } else {
        return false;
    }
}

/***
 * The beginning of a typical raycasting loop.
 */
#define RC_BEGIN_LOOP \
    float tDepth = -1.0;                                    \
    bool finished = false;                                    \
    for (int loop0=0; !finished && loop0<255; loop0++) {    \
        for (int loop1=0; !finished && loop1<255; loop1++) {

/***
 * The end of a typical raycasting loop. If adaptive sampling
 * is used for rendering bricked volumes, t is increased by a
 * multiple of tIncr, thereby skipping several samples. 
 */
#ifdef ADAPTIVE_SAMPLING
#define RC_END_LOOP(result)									\
			finished = earlyRayTermination(result.a, 0.9);	\
			t += (tIncr * float(numberOfSkippedSamples));				\
    		finished = finished || (t > tEnd);				\
		}													\
	}														\
	gl_FragDepth = getDepthValue(tDepth, tEnd,				\
					entryPointsDepth_, exitPointsDepth_);
#else
#define RC_END_LOOP(result)									\
			finished = earlyRayTermination(result.a, 0.9);	\
			t += tIncr;				                        \
    		finished = finished || (t > tEnd);				\
		}													\
	}														\
	gl_FragDepth = getDepthValue(tDepth, tEnd,				\
					entryPointsDepth_, exitPointsDepth_);
#endif

/**
* In order to keep the shaders as free as possible from dealing
* with bricking and adaptive sampling, these defines can be placed
* before and after the compositing function calls to enable adaptive
* sampling when bricking is used. For normal datasets these defines
* will have no impact at all. 
*/ 
#ifdef ADAPTIVE_SAMPLING
#define RC_BEGIN_COMPOSITING                                \
    for (int i=0; i<numberOfSkippedSamples; i++) {
#else 
    #define RC_BEGIN_COMPOSITING                                                                            
#endif

#ifdef ADAPTIVE_SAMPLING
#define RC_END_COMPOSITING                                  \
    }
#else 
#define RC_END_COMPOSITING                                                                               
#endif

