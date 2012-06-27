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

#include "modules/mod_sampler3d.frag"           // contains struct VOLUME_PARAMETERS 

uniform VOLUME_PARAMETERS volumeParameters_;    // additional information about the volume the eep are generated for

varying vec4 eep_;

/**
 * Simply get the provided proxy geometry's vertex position, transform it to texture space
 * and put it into eep_. OpenGL will interpolate the values for us.
 */
void main()
{
    // transform eep vertex from object space to texture space
    eep_.xyz = (gl_Vertex.xyz * volumeParameters_.volumeCubeSizeRCP_) + vec3(0.5);
    eep_ = vec4(eep_.xyz, 1.0);

    // transform vertex position into world coordinates
    vec4 vertexPos = gl_ModelViewMatrix * gl_Vertex;
    gl_Position = gl_ProjectionMatrix * vertexPos;
    
    #ifdef VRN_USE_CLIP_PLANE
		gl_ClipVertex = gl_Vertex;
    #endif
}
