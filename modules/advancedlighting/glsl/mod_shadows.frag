/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

uniform sampler3D shadowTexture_;
uniform int permutationIndex_;
uniform float increment_;

/**
 * Adjusts texture coordinates according to the coordinate we are looping about.
 *
 * @param texCoords texture coordinates before permutation
 * @return texture coordinates after permutation
 */
vec3 permuteTextureCoords(in int permutation, in vec3 texCoords, in float sign) {
    vec3 texNew;
    if (permutation == 0) {
        texNew.xyz = texCoords.zyx;
    }
    else if (permutation == 1) {
        texNew.xyz = texCoords.xzy;
    }
    else {
        texNew = texCoords;
        texNew.x = 1.0-texNew.x;
    }

    if (sign > 0.0) {
        texNew.xz = 1.0 - texNew.xz;
    }

    return texNew;
}


vec4 getShadowValues(in vec3 samplePos) {
    vec3 texCoords = permuteTextureCoords(permutationIndex_, samplePos, increment_);
    return texture3D(shadowTexture_, texCoords);
}

/**
 * Applies a mean or gauss filter to the given parameter.
 *
 * @param center center value
 * @param north north value
 * @param south south value
 * @param west west value
 * @param east east value
 * @param northwest northwest value
 * @param northeast northeast value
 * @param southwest southwest value
 * @param southeast southeast value
 * @return filtered value
 */
vec4 applyFilter(in vec4 center, in vec4 north, in vec4 south, in vec4 west, in vec4 east,
                 in vec4 northwest, in vec4 northeast, in vec4 southwest, in vec4 southeast)
{
    vec4 filtered = vec4(0.0);

#ifdef USE_MEAN_FILTER
    filtered = (center + north     + south     + west      + east
                       + northeast + northwest + southeast + southwest);
    filtered /= 9.0;
#endif

#ifdef USE_GAUSSIAN_FILTER
    filtered = (1.0 * northwest  + 2.0 * north  + 1.0 * northeast +
                2.0 * west       + 4.0 * center + 2.0 * east      +
                1.0 * southwest  + 2.0 * south  + 1.0 * southeast);

    filtered /= 16.0;
#endif

    return filtered;
}

/**
 * Applies a mean or gauss filter to the given parameter.
 *
 * @param center center value
 * @param north north value
 * @param south south value
 * @param west west value
 * @param east east value
 * @return filtered value
 */
vec4 applyFilter(in vec4 center, in vec4 north, in vec4 south, in vec4 west, in vec4 east) {
    vec4 filtered = vec4(0.0);

#ifdef USE_MEAN_FILTER
    filtered = (center + north + south + west + east);
    filtered /= 5.0;
#endif

#ifdef USE_GAUSSIAN_FILTER
    filtered = (             2.0 * north               +
                2.0 * west + 4.0 * center + 2.0 * east +
                           + 2.0 * south               );
    filtered /= 12.0;
#endif

    return filtered;
}


vec4 getValueLinear(in sampler3D texture, in vec3 texCoord, in vec3 dimension) {
    //TODO: we can avoid manual bilinear filtering if we use texturearrays (cdoer)
    vec2 delta = vec2(1.0) / (dimension.xy - 1.0);

    vec2 temp = floor(texCoord.xy / delta)  * delta;
    vec3 lowerleft  = vec3(temp.x,                temp.y,                texCoord.z);
    vec3 lowerright = vec3(lowerleft.x + delta.x, lowerleft.y,           texCoord.z);
    vec3 upperleft  = vec3(lowerleft.x,           lowerleft.y + delta.y, texCoord.z);
    vec3 upperright = vec3(lowerleft.x + delta.x, lowerleft.y + delta.y, texCoord.z);

    vec4 valuelowerleft  = texture3D(texture, lowerleft);
    vec4 valuelowerright = texture3D(texture, lowerright);
    vec4 valueupperleft  = texture3D(texture, upperleft);
    vec4 valueupperright = texture3D(texture, upperright);

    float coefficientLeft  = (lowerright.x - texCoord.x) / (lowerright.x - lowerleft.x);
    float coefficientRight = (texCoord.x  - lowerleft.x) / (lowerright.x - lowerleft.x);

    vec4 t1 = coefficientLeft * valuelowerleft + coefficientRight * valuelowerright;
    vec4 t2 = coefficientLeft * valueupperleft + coefficientRight * valueupperright;
    t1 *= (upperleft.y - texCoord.y) / (upperleft.y  - lowerleft.y);
    t2 *= (texCoord.y - lowerleft.y) / (upperleft.y  - lowerleft.y);

    return t1 + t2;
}
