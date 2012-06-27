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

uniform vec2 screenDim_;
uniform vec2 screenDimRCP_;

// definitions for textures of type GL_TEXTURE_2D
#if defined(VRN_TEXTURE_2D)

    #define SAMPLER2D_TYPE sampler2D

    // texture lookup function for 2D textures
    // texture coordinates have to be passed as fragment coordinates!
    vec4 textureLookup2D(in sampler2D texture, in vec2 texCoords) {
        vec2 texCoordsNormalized = texCoords * screenDimRCP_;
        return texture2D(texture, texCoordsNormalized);
    }
    
// definitions for textures of type GL_TEXTURE_RECTANGLE_ARB
#elif defined(VRN_TEXTURE_RECTANGLE)

    #extension GL_ARB_texture_rectangle : enable    

    #define SAMPLER2D_TYPE sampler2DRect

    // texture lookup function for 2D textures
    // texture coordinates have to be passed as fragment coordinates!
    vec4 textureLookup2D(in sampler2DRect texture, in vec2 texCoords) {
        return texture2DRect(texture, texCoords);
    }

#endif
