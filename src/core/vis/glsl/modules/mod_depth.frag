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

/*
uniform float const_to_z_w_1;
uniform float const_to_z_w_2;
uniform float const_to_z_e_1;
uniform float const_to_z_w_2;
*/

/**
 * Calculates the depth value for the current sample specified by the parameter t.
 **/
float calculateDepthValue(float t, SAMPLER2D_TYPE entryPointsDepth, SAMPLER2D_TYPE exitPointsDepth) {
	/*
	   Converting eye coordinate depth values to windows coordinate depth values:
	   (see http://www.opengl.org/resources/faq/technical/depthbuffer.htm 12.050, assuming w_e = 1)
	   
	   z_w = (1.0/z_e)*((f*n)/(f-n)) + 0.5*((f+n)/(f-n))+0.5; (f=far plane, n=near plane)
	   
	   We calculate constant terms outside:
	   const_to_z_w_1 = ((f*n)/(f-n)) 
	   const_to_z_w_2 = 0.5*((f+n)/(f-n))+0.5
	   
	   Converting windows coordinates to eye coordinates:
	   
	   z_e = 1.0/([z_w - 0.5 - 0.5*((f+n)/(f-n))]*((f-n)/(f*n)));
	   
	   with constant terms
	   const_to_z_e_1 = 0.5 + 0.5*((f+n)/(f-n))
	   const_to_z_e_2 = ((f-n)/(f*n))
	 */
	
	// temp
	float n = 0.1;
	float f = 50.0;
    
    float const_to_z_e_1 = 0.5 + 0.5*((f+n)/(f-n));
    float const_to_z_e_2 = ((f-n)/(f*n));
    float const_to_z_w_1 = ((f*n)/(f-n));
    float const_to_z_w_2 = 0.5*((f+n)/(f-n))+0.5;
    // end temp
    
    // fetch front value in windows coordinates
	float zw_front = textureLookup2D(entryPointsDepth, gl_FragCoord.xy).z;
	// and convert it into eye coordinates
	float ze_front = 1.0/((zw_front - const_to_z_e_1)*const_to_z_e_2);
	
	// fetch back value in windows coordinates
	float zw_back = textureLookup2D(exitPointsDepth, gl_FragCoord.xy).z;
	// and convert it into eye coordinates
	float ze_back = 1.0/((zw_back - const_to_z_e_1)*const_to_z_e_2);
	
	// interpolate in eye coordinates
	float ze_current = ze_front + t*(ze_back-ze_front);
	
	// convert back to window coordinates
	float zw_current = (1.0/ze_current)*const_to_z_w_1 + const_to_z_w_2;
	
	return zw_current;
}

/**
 * Returns the depth value for a given t by considering the ray
 * length as well as the depth of the entry and exit points.
 */
float getDepthValue(float t, float tEnd, SAMPLER2D_TYPE entryPointsDepth, SAMPLER2D_TYPE exitPointsDepth) {
	if (t >= 0.0)
		return calculateDepthValue(t/tEnd, entryPointsDepth, exitPointsDepth);
	else
		return 1.0;
}
