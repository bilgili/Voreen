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

#include "modules/mod_sampler3d.frag"
#include "modules/mod_transfunc.frag"
#include "mod_shadows.frag"

// declare volume
uniform VolumeParameters volumeStruct_;
uniform sampler3D volume_;                   // volume data set

uniform TransFuncParameters transferFunc_;
uniform TF_SAMPLER_TYPE transferFuncTex_;

uniform vec3 viewpoint_;         ///< point which determines the direction of texture creation (lightsource or camera)
uniform int sliceNumber_;        ///< the first and second slice must be handled differently
uniform ivec3 textureDimension_; ///< dimension of the shadow texture
uniform bool firstPass_;
uniform int scalingFactor_;

uniform sampler3D viewpointTexture_;
uniform int permutationIndexView_;
uniform float signView_;

uniform sampler3D blendTexture_;
uniform int permutationIndexBlend_;
uniform float signBlend_;
uniform vec2 angles_;


/**
 * Calculates the texture coordinates for accessing the shadowtexture.
 *
 * @return texture coordinates for accessing the shadowtexture
 */
vec3 calculateTextureCoordinates() {
    // get correct vertex position (in [-1, 1]) even for non uniform datasets
    vec3 vertexPosition =  texToPhysical(gl_TexCoord[0].xyz, volumeStruct_);

    // calculcate vector to lightsource position
    vec3 direction = normalize(viewpoint_ - vertexPosition);
    // copy vector to float array so that indexing with non constant values is possible
    float[3] directionArray = float[3](direction.x, direction.y, direction.z);

    float denominator = directionArray[permutationIndex_];

    // calculate t for linear equation
    float t = increment_ / denominator;
    vec3 texCoord = gl_TexCoord[0].xyz + t * direction;

    return texCoord;
}

/**
 * Calculates the shadow value for the current fragment with the given texture coordinates.
 * Depending on NEIGHBORS up to 8 voxels in the shadow texture are taken into account for
 * calculation.
 *
 * @param texCoords texture coordinates for access in shadow texture
 * @return shadow value
 */
vec4 calculateShadowValue(in vec3 texCoords) {
    vec4 shadow = vec4(0.0);

    texCoords = permuteTextureCoords(permutationIndex_, texCoords, increment_);
    //FIXME: z fighting problem not solved if volume is not empty in first slice
    //if (sliceNumber_ == 1)
    //    texCoords = clamp(texCoords, 0.0, 1.0);

    vec4 center = getValueLinear(shadowTexture_, texCoords, textureDimension_);
    if (NEIGHBORS == 0) {
        shadow = center;
    }
    else {
        vec2 delta = vec2(1.0) / (textureDimension_.xy - 1.0);

        vec3 northTexCoords, southTexCoords, westTexCoords, eastTexCoords;
        vec4 north, south, west, east;
        if (NEIGHBORS >= 4) {
            northTexCoords = texCoords + vec3(     0.0,  delta.y, 0.0);
            southTexCoords = texCoords + vec3(     0.0, -delta.y, 0.0);
            eastTexCoords  = texCoords + vec3(-delta.x,      0.0, 0.0);
            westTexCoords  = texCoords + vec3( delta.x,      0.0, 0.0);
            north = getValueLinear(shadowTexture_, northTexCoords, textureDimension_);
            south = getValueLinear(shadowTexture_, southTexCoords, textureDimension_);
            east  = getValueLinear(shadowTexture_, eastTexCoords, textureDimension_);
            west  = getValueLinear(shadowTexture_, westTexCoords, textureDimension_);

            shadow = applyFilter(center, north, south, west, east);
        }

        vec4 northeast, northwest, southeast, southwest;
        if (NEIGHBORS >= 8) {
            vec3 northwestTexCoords = northTexCoords + vec3( delta.x, 0.0, 0.0);
            vec3 northeastTexCoords = northTexCoords + vec3(-delta.x, 0.0, 0.0);
            vec3 southwestTexCoords = southTexCoords + vec3( delta.x, 0.0, 0.0);
            vec3 southeastTexCoords = southTexCoords + vec3(-delta.x, 0.0, 0.0);
            northwest = getValueLinear(shadowTexture_, northwestTexCoords, textureDimension_);
            northeast = getValueLinear(shadowTexture_, northeastTexCoords, textureDimension_);
            southwest = getValueLinear(shadowTexture_, southwestTexCoords, textureDimension_);
            southeast = getValueLinear(shadowTexture_, southeastTexCoords, textureDimension_);

            shadow = applyFilter(center, north, south, west, east,
                                 northeast, northwest, southeast, southwest);
        }
    }

    return shadow;
}

/**
 *
 * @param texCoord texture coordinates for access in volume texture
 * @return color for current fragment
 */
vec4 calculateColor(in vec3 texCoord) {
    vec4 color = vec4(0.0);

    vec4 center = textureLookup3D(volume_, volumeStruct_, texCoord);
    center = applyTF(transferFunc_, transferFuncTex_, center.a);
    if (NEIGHBORS == 0) {
        color = center;
    }
    else {
        vec3 deltax, deltay;
        vec3 delta = vec3(1.0) / (volumeStruct_.datasetDimensions_ - 1.0);

        if (permutationIndex_ == 0) {
            deltax = vec3(0.0, 0.0, delta.z);
            deltay = vec3(0.0, delta.y, 0.0);
        }
        else if (permutationIndex_ == 1) {
            deltax = vec3(delta.x, 0.0, 0.0);
            deltay = vec3(0.0, 0.0, delta.z);
        }
        else {
            deltax = vec3(delta.x, 0.0, 0.0);
            deltay = vec3(0.0, delta.y, 0.0);
        }

        vec4 north, south, west, east;
        vec3 northTexCoords, southTexCoords, westTexCoords, eastTexCoords;
        if (NEIGHBORS >= 4) {
            northTexCoords = texCoord + deltay;
            southTexCoords = texCoord - deltay;
            westTexCoords  = texCoord + deltax;
            eastTexCoords  = texCoord - deltax;
            north = textureLookup3D(volume_, volumeStruct_, northTexCoords);
            north = applyTF(transferFunc_, transferFuncTex_, north.a);
            south = textureLookup3D(volume_, volumeStruct_, southTexCoords);
            south = applyTF(transferFunc_, transferFuncTex_, south.a);
            west  = textureLookup3D(volume_, volumeStruct_, westTexCoords);
            west  = applyTF(transferFunc_, transferFuncTex_, west.a);
            east  = textureLookup3D(volume_, volumeStruct_, eastTexCoords);
            east  = applyTF(transferFunc_, transferFuncTex_, east.a);

            color = applyFilter(center, north, south, west, east);
        }

        vec4 northeast, northwest, southeast, southwest;
        if (NEIGHBORS >= 8) {
            vec3 northwestTexCoords = northTexCoords + deltax;
            vec3 northeastTexCoords = northTexCoords - deltax;
            vec3 southwestTexCoords = southTexCoords + deltay;
            vec3 southeastTexCoords = southTexCoords - deltay;
            northeast = textureLookup3D(volume_, volumeStruct_, northeastTexCoords);
            northeast = applyTF(transferFunc_, transferFuncTex_, northeast.a);
            northwest = textureLookup3D(volume_, volumeStruct_, northwestTexCoords);
            northwest = applyTF(transferFunc_, transferFuncTex_, northwest.a);
            southeast  = textureLookup3D(volume_, volumeStruct_, southeastTexCoords);
            southeast  = applyTF(transferFunc_, transferFuncTex_, southeast.a);
            southwest  = textureLookup3D(volume_, volumeStruct_, southwestTexCoords);
            southwest  = applyTF(transferFunc_, transferFuncTex_, southwest.a);

            color = applyFilter(center, north, south, west, east,
                                northeast, northwest, southeast, southwest);
        }
    }

    return color;
}


/**
 * The main method.
 */
void main() {
    vec4 result;

    if (sliceNumber_ == 0) {
        // fetch intensity value from volume dataset (texture ccordinates in [0, 1])
        float intensity = textureLookup3D(volume_, volumeStruct_, gl_TexCoord[0].xyz).a;
        // apply transfer function
        result = applyTF(transferFunc_, transferFuncTex_, intensity);
        //TODO: incorporate light parameters
    }
    else {
        // calculate texture coordinates
        vec3 texCoord = calculateTextureCoordinates();

        // calculate shadow and color according to selected filter and neighborhood
        vec4 shadowValue = calculateShadowValue(texCoord);
        vec4 color = calculateColor(texCoord);

        // obey size scaling of texture
        color.a *= scalingFactor_;
        // back to front compositing
        result.rgb = (1.0 - color.a) * shadowValue.rgb + color.a * color.rgb;
        result.a =   (1.0 - color.a) * shadowValue.a   + color.a;

        if (!firstPass_) {
            if ((abs(angles_.y - angles_.x)) < 0.05) {
                vec3 coords = permuteTextureCoords(permutationIndexBlend_, gl_TexCoord[0].xyz, signBlend_);
                vec4 value = texture3D(blendTexture_, coords);
                //TODO: think about it (sum of angles_.x and angles_y is not 1!!!!)
                vec4 temp = (1.0-angles_.x)*result + angles_.y*value;
                result = temp;
            }

            //vec3 coords = permuteTextureCoords(permutationIndexView_, gl_TexCoord[0].xyz, signView_);
            //result = texture3D(viewpointTexture_, coords);

        }

    }

    // write value
    FragData0 = result;
}
