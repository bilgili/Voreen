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

uniform vec2 screenDim_;
uniform vec2 screenDimRCP_;

/*
 * This struct contains information about a texture, like
 * its dimensions. Additionally, the reciprocal
 * values of all parameters are available (suffix RCP) .
 */
struct TEXTURE_PARAMETERS {
    vec2 dimensions_;        // the texture's resolution, e.g. [256.0, 128.0]
    vec2 dimensionsRCP_;
    mat4 matrix_;            // texture coordinate transformation
};

// definitions for textures of type GL_TEXTURE_2D
#if defined(VRN_TEXTURE_2D)
    #define SAMPLER2D_TYPE sampler2D

    // Texture lookup function for 2D textures,
    // expecting texture coordinates as pixel coordinates, i.e, [(0,0) , textureSize].
    vec4 textureLookup2D(in sampler2D texture, in TEXTURE_PARAMETERS texParams, in vec2 texCoords) {
        vec2 texCoordsNormalized = texCoords * texParams.dimensionsRCP_;
        vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoordsNormalized, 0.0, 1.0)).xy;
        return texture2D(texture, texCoordsTransformed);
    }

    // Texture lookup function for 2D textures,
    // expecting normalized texture coordinates, i.e., [0,1].
    vec4 textureLookup2Dnormalized(in sampler2D texture, in TEXTURE_PARAMETERS texParams, in vec2 texCoords) {
        vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoords, 0.0, 1.0)).xy;
        return texture2D(texture, texCoordsTransformed);
    }

// definitions for textures of type GL_TEXTURE_RECTANGLE_ARB
#elif defined(VRN_TEXTURE_RECTANGLE)

    #extension GL_ARB_texture_rectangle : enable

    #define SAMPLER2D_TYPE sampler2DRect

    // texture lookup function for 2D textures
    // texture coordinates have to be passed as fragment coordinates!
    vec4 textureLookup2Dnormalized(in sampler2DRect texture, in TEXTURE_PARAMETERS texParams, in vec2 texCoords) {
        vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoords, 0.0, 1.0)).xy;
        return texture2DRect(texture, texCoordsTransformed * texParams.dimensions_);
    }

    // texture lookup function for 2D textures
    // texture coordinates have to be passed as pixel coordinates.
    vec4 textureLookup2D(in sampler2DRect texture, in TEXTURE_PARAMETERS texParams, in vec2 texCoords) {
        vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoords, 0.0, 1.0)).xy;
        return texture2DRect(texture, texCoordsTransformed);
    }

#endif

// Standard texture lookup function for RenderPort images.
// Texture coordinates are expected in fragment coordinates, i.e, [(0,0) , viewportSize].
//
// @note This function may only be used, if the processor's input and output images (RenderPorts)
//       have the same dimensions.
//
vec4 textureLookup2Dscreen(in SAMPLER2D_TYPE texture, in TEXTURE_PARAMETERS texParams, in vec2 texCoords) {
    vec2 texCoordsNormalized = texCoords * screenDimRCP_;
    return textureLookup2Dnormalized(texture, texParams, texCoordsNormalized);
}
