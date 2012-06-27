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

#ifndef VRN_GRADIENT_H
#define VRN_GRADIENT_H

#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/volume/volume.h"

#include <string>
#include <iostream>
#include <fstream>

using tgt::ivec3;
using tgt::vec3;

namespace voreen {

// TODO: these functions should be encapsulated somewhere

template<class U, class T>
VolumeAtomic<U>* calcGradients(VolumeAtomic<T> *input, int mapping) {
    
    VolumeAtomic<U>* result = new VolumeAtomic<U>(input->getDimensions(), input->getSpacing());
    
    tgt::vec3 gradient;

    tgt::ivec3 pos;
    tgt::ivec3 dim = input->getDimensions();
    T v0, v1, v2, v3, v4, v5;
    
    int bitsU = result->getBitsStored() / result->getNumChannels();

    //check for 4 channels...write intensity to this channel
    bool v4c = VolumeElement<U>::getNumChannels() == 4;

    for (pos.z = 0; pos.z < input->getDimensions().z; pos.z++) {
        for (pos.y = 0; pos.y < input->getDimensions().y; pos.y++) {
            for (pos.x = 0; pos.x < input->getDimensions().x; pos.x++) {
                if (pos.x != dim.x-1)
                    v0 = input->voxel(pos + tgt::ivec3(1, 0, 0));
                else
                    v0 = 0;
                if (pos.y != dim.y-1)
                    v1 = input->voxel(pos + tgt::ivec3(0, 1, 0));
                else
                    v1 = 0;
                if (pos.z != dim.z-1)
                    v2 = input->voxel(pos + tgt::ivec3(0, 0, 1));
                else
                    v2 = 0;
                
                if (pos.x != 0)
                    v3 = input->voxel(pos + tgt::ivec3(-1, 0, 0));
                else
                    v3 = 0;
                if (pos.y != 0)
                    v4 = input->voxel(pos + tgt::ivec3(0, -1, 0));
                else
                    v4 = 0;
                if (pos.z != 0)
                    v5 = input->voxel(pos + tgt::ivec3(0, 0, -1));
                else
                    v5 = 0;
                
                gradient = tgt::vec3(v3 - v0, v4 - v1, v5 - v2) / 2.f;
                gradient /= input->getSpacing();

                                
                if (v4c) {
                    if (bitsU == 8) {
                        reinterpret_cast<Volume4xUInt8*>(result)->voxel(pos).r = static_cast<uint8_t>((gradient.x/mapping)+128.f);
                        reinterpret_cast<Volume4xUInt8*>(result)->voxel(pos).g = static_cast<uint8_t>((gradient.y/mapping)+128.f);
                        reinterpret_cast<Volume4xUInt8*>(result)->voxel(pos).b = static_cast<uint8_t>((gradient.z/mapping)+128.f);
                        reinterpret_cast<Volume4xUInt8*>(result)->voxel(pos).a = static_cast<uint8_t>(input->getVoxelFloat(pos) * 255.0f);
                    }
                    else if (bitsU == 16) {
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).r = static_cast<uint16_t>((gradient.x/mapping * 256.f) + 32768.f);
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).g = static_cast<uint16_t>((gradient.y/mapping * 256.f) + 32768.f);
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).b = static_cast<uint16_t>((gradient.z/mapping * 256.f) + 32768.f);
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).a = static_cast<uint16_t>((input->getVoxelFloat(pos) * 65535.f));
                    }
                }
                else {
                    if (bitsU == 8) {
                        reinterpret_cast<Volume3xUInt8*>(result)->voxel(pos).r = static_cast<uint8_t>((gradient.x/mapping)+128.f);
                        reinterpret_cast<Volume3xUInt8*>(result)->voxel(pos).g = static_cast<uint8_t>((gradient.y/mapping)+128.f);
                        reinterpret_cast<Volume3xUInt8*>(result)->voxel(pos).b = static_cast<uint8_t>((gradient.z/mapping)+128.f);
                    }
                    else if (bitsU == 16) {
                        reinterpret_cast<Volume3xUInt16*>(result)->voxel(pos).r = static_cast<uint16_t>((gradient.x/mapping * 256.f) + 32768.f);
                        reinterpret_cast<Volume3xUInt16*>(result)->voxel(pos).g = static_cast<uint16_t>((gradient.y/mapping * 256.f) + 32768.f);
                        reinterpret_cast<Volume3xUInt16*>(result)->voxel(pos).b = static_cast<uint16_t>((gradient.z/mapping * 256.f) + 32768.f);
                    }
                }
               

            }
        }
    }
    return result;
}


/**
 * Calculates gradients by central differences.
 *
 * Use col3 or col4 as template argument in order to generate 3x8 or 4x8 bit datasets.
 * Use Vector3<uint16_t> or Vector4<uint16_t> as template argument in order 
 * to generate 3x16 or 4x16 bit datasets.
 */ 
template<class U>
VolumeAtomic<U>* calcGradients(voreen::Volume* vol) {
    
    if (vol->getBitsStored() == 8) {
        VolumeUInt8* input = (VolumeUInt8*) vol;
        return calcGradients<U, uint8_t>(input, 2);
    }
    else if (vol->getBitsStored() == 16) {
        VolumeUInt16* input = (VolumeUInt16*) vol;
        return calcGradients<U, uint16_t>(input, 512);
    }
    else if (vol->getBitsStored() == 12) {
        VolumeUInt16* input = (VolumeUInt16*) vol;
        return calcGradients<U, uint16_t>(input, 32);
    }
    LERRORC("calcGradients", "calcGradients needs a 8-, 12- or 16-bit dataset as input");
    return 0;
}



/**
 * Calculates gradients using linear regression according to Neumann et al.
 *
 \verbatim  
    L. Neumann, B. Csébfalvi, A. König, and M. E. Gröller.
    Gradient estimation in volume data using 4D linear regression.
    In Proceedings of Eurographics 2000, pages 351-358, 2000.
 \endverbatim
 *
 * The neighboring voxels are weighted by their reciprocal Euclidean distance.
 *
 * Use Vector3<uint8_t> / Vector4<uint8_t> or Vector3<uint16_t> / Vector4<uint16_t> 
 * as U template argument in order to generate 3x8 / 4x8 or 3x16 / 4x16 bit datasets. 
 * In case of 4-channel volume the alpha channel is filled with the input volume's intensities.
 *
 */
template<class U, class T>
VolumeAtomic<U>* calcGradientsLinearRegression(VolumeAtomic<T> *input) {
    
    VolumeAtomic<U>* result = new VolumeAtomic<U>(input->getDimensions(), input->getSpacing());
    
    // check input volume type
    float maxValueT;
    if ( typeid(*input) == typeid(VolumeUInt8)  || 
         typeid(*input) == typeid(VolumeUInt16) || 
         typeid(*input) == typeid(VolumeUInt32) )
        maxValueT = static_cast<float>( (1 << input->getBitsStored()) - 1);
    else if ( typeid(*input) == typeid(VolumeFloat) || typeid(*input) == typeid(VolumeDouble))
        maxValueT = 1.f;
    else { 
        LERRORC("calcGradientsLinearRegression", "Unknown or unsupported input volume type");
        tgtAssert(false, "Unknown or unsupported input volume type");
        return result;
    }

    // check output volume type
    int bitsU;
    int numChannelsU;
    float maxValueU;
    if ( typeid(*result) == typeid(Volume3xUInt8)  || 
         typeid(*result) == typeid(Volume3xUInt16) ||
         typeid(*result) == typeid(Volume4xUInt8)  || 
         typeid(*result) == typeid(Volume4xUInt16)   ) {
        numChannelsU = result->getNumChannels();
        bitsU = result->getBitsStored() / numChannelsU;
        maxValueU = static_cast<float>( (1 << bitsU) - 1);
    }
    else { 
        LERRORC("calcGradientsLinearRegression", "Unknown or unsupported output volume type");
        tgtAssert(false, "Unknown or unsupported output volume type");
        return result;
    }
    
    // Euclidean weights for voxels with Manhattan distances of 1/2/3
    float w_1 = 1.f;
    float w_2 = 0.5f;
    float w_3 = 1.f/3.f;
    
    float w_A = 1.f / (8.f + 2.f/3.f);
    float w_B = w_A;
    float w_C = w_A;

    vec3 gradient;
    ivec3 pos;
    ivec3 dim = input->getDimensions();
    for (pos.z = 0; pos.z < dim.z; pos.z++) {
        for (pos.y = 0; pos.y < dim.y; pos.y++) {
            for (pos.x = 0; pos.x < dim.x; pos.x++) {

                if (pos.x >= 1 && pos.x < dim.x-1 &&
                    pos.y >= 1 && pos.y < dim.y-1 &&
                    pos.z >= 1 && pos.z < dim.z-1)
                {
                    //left plane
                    T v000 = input->voxel(pos + ivec3(-1,-1,-1));
                    T v001 = input->voxel(pos + ivec3(-1, -1, 0));
                    T v002 = input->voxel(pos + ivec3(-1, -1, 1));
                    T v010 = input->voxel(pos + ivec3(-1, 0, -1));
                    T v011 = input->voxel(pos + ivec3(-1, 0, 0));
                    T v012 = input->voxel(pos + ivec3(-1, 0, 1));
                    T v020 = input->voxel(pos + ivec3(-1, 1, -1));
                    T v021 = input->voxel(pos + ivec3(-1, 1, 0));
                    T v022 = input->voxel(pos + ivec3(-1, 1, 1));
                    
                    //mid plane
                    T v100 = input->voxel(pos + ivec3(0, -1, -1));
                    T v101 = input->voxel(pos + ivec3(0, -1, 0));
                    T v102 = input->voxel(pos + ivec3(0, -1, 1));
                    T v110 = input->voxel(pos + ivec3(0, 0, -1));
                    //T v111 = input->voxel(pos + ivec3(0, 0, 0)); 
                    T v112 = input->voxel(pos + ivec3(0, 0, 1));
                    T v120 = input->voxel(pos + ivec3(0, 1, -1));
                    T v121 = input->voxel(pos + ivec3(0, 1, 0));
                    T v122 = input->voxel(pos + ivec3(0, 1, 1));
                    
                    //right plane
                    T v200 = input->voxel(pos + ivec3(1, -1, -1));
                    T v201 = input->voxel(pos + ivec3(1, -1, 0));
                    T v202 = input->voxel(pos + ivec3(1, -1, 1));
                    T v210 = input->voxel(pos + ivec3(1, 0, -1));
                    T v211 = input->voxel(pos + ivec3(1, 0, 0));
                    T v212 = input->voxel(pos + ivec3(1, 0, 1));
                    T v220 = input->voxel(pos + ivec3(1, 1, -1));
                    T v221 = input->voxel(pos + ivec3(1, 1, 0));
                    T v222 = input->voxel(pos + ivec3(1, 1, 1));

                    gradient.x = static_cast<float>( w_1 * ( v211 - v011 )               +
                                                     w_2 * ( v201 + v210 + v212 + v221 
                                                            -v001 - v010 - v012 - v021 ) + 
                                                     w_3 * ( v200 + v202 + v220 + v222
                                                            -v000 - v002 - v020 - v022 )   );
                    
                    gradient.y = static_cast<float>( w_1 * ( v121 - v101 )               +
                                                     w_2 * ( v021 + v120 + v122 + v221 
                                                            -v001 - v100 - v102 - v201 ) + 
                                                     w_3 * ( v020 + v022 + v220 + v222
                                                            -v000 - v002 - v200 - v202 )   );

                    gradient.z = static_cast<float>( w_1 * ( v112 - v110 )               +
                                                     w_2 * ( v012 + v102 + v122 + v212 
                                                            -v010 - v100 - v120 - v210 ) + 
                                                     w_3 * ( v002 + v022 + v202 + v222
                                                            -v000 - v020 - v200 - v220 )   );

                    gradient.x *= w_A;
                    gradient.y *= w_B;
                    gradient.z *= w_C;

                    gradient /= input->getSpacing();
                    gradient *= -1.f;

                   
                }
                else {
                    gradient = vec3(0.f);
                }

                // map vector from [-maxT:maxT] to [0:maxU] since we expect an unsigned volume as input/output type
                gradient = ( (gradient / maxValueT) / 2.f + 0.5f ) * maxValueU;

                if (numChannelsU == 3) {
                    if (bitsU == 8) {
                        reinterpret_cast<Volume3xUInt8*>(result)->voxel(pos).r = static_cast<uint8_t>(gradient.x);
                        reinterpret_cast<Volume3xUInt8*>(result)->voxel(pos).g = static_cast<uint8_t>(gradient.y);
                        reinterpret_cast<Volume3xUInt8*>(result)->voxel(pos).b = static_cast<uint8_t>(gradient.z);
                    }
                    else if (bitsU == 16) {
                        reinterpret_cast<Volume3xUInt16*>(result)->voxel(pos).r = static_cast<uint16_t>(gradient.x);
                        reinterpret_cast<Volume3xUInt16*>(result)->voxel(pos).g = static_cast<uint16_t>(gradient.y);
                        reinterpret_cast<Volume3xUInt16*>(result)->voxel(pos).b = static_cast<uint16_t>(gradient.z);
                    }
                }
                else if (numChannelsU == 4) {
                    if (bitsU == 8) {
                        reinterpret_cast<Volume4xUInt8*>(result)->voxel(pos).r = static_cast<uint8_t>(gradient.x);
                        reinterpret_cast<Volume4xUInt8*>(result)->voxel(pos).g = static_cast<uint8_t>(gradient.y);
                        reinterpret_cast<Volume4xUInt8*>(result)->voxel(pos).b = static_cast<uint8_t>(gradient.z);
                        reinterpret_cast<Volume4xUInt8*>(result)->voxel(pos).a = static_cast<uint8_t>(input->getVoxelFloat(pos) * 255.f);
                    }
                    else if (bitsU == 16) {
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).r = static_cast<uint16_t>(gradient.x);
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).g = static_cast<uint16_t>(gradient.y);
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).b = static_cast<uint16_t>(gradient.z);
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).a = static_cast<uint16_t>(input->getVoxelFloat(pos) * 65535.f);
                    }
                }

            }
        }
    }
    

    return result;
  
}



/**
 * Calculates gradients with neighborhood of 26, using the Sobel filter.
 *
 */
template<class U, class T>
VolumeAtomic<U>* calcGradientsSobel(VolumeAtomic<T> *input, int mapping, bool intensityCheck) {
    VolumeAtomic<U>* result = new VolumeAtomic<U>(input->getDimensions(), input->getSpacing());

    using tgt::vec3;
    using tgt::ivec3;

    int bitsU = result->getBitsStored() / result->getNumChannels();

    vec3 gradient;

    ivec3 pos;
    ivec3 dim = input->getDimensions();

    //check for 4 channels...write intensity to this channel
    bool v4 = VolumeElement<U>::getNumChannels() == 4;

    for (pos.z = 0; pos.z < input->getDimensions().z; pos.z++) {
        for (pos.y = 0; pos.y < input->getDimensions().y; pos.y++) {
            for (pos.x = 0; pos.x < input->getDimensions().x; pos.x++) {

                gradient = vec3(0.f);

                if (pos.x >= 1 && pos.x < dim.x-1 &&
                    pos.y >= 1 && pos.y < dim.y-1 &&
                    pos.z >= 1 && pos.z < dim.z-1)
                {
                   
                    //left plane
                    T v000 = input->voxel(pos + ivec3(-1, -1, -1));
                    T v001 = input->voxel(pos + ivec3(-1, -1, 0));
                    T v002 = input->voxel(pos + ivec3(-1, -1, 1));
                    T v010 = input->voxel(pos + ivec3(-1, 0, -1));
                    T v011 = input->voxel(pos + ivec3(-1, 0, 0));
                    T v012 = input->voxel(pos + ivec3(-1, 0, 1));
                    T v020 = input->voxel(pos + ivec3(-1, 1, -1));
                    T v021 = input->voxel(pos + ivec3(-1, 1, 0));
                    T v022 = input->voxel(pos + ivec3(-1, 1, 1));
                    //mid plane
                    T v100 = input->voxel(pos + ivec3(0, -1, -1));
                    T v101 = input->voxel(pos + ivec3(0, -1, 0));
                    T v102 = input->voxel(pos + ivec3(0, -1, 1));
                    T v110 = input->voxel(pos + ivec3(0, 0, -1));
                    T v111 = input->voxel(pos + ivec3(0, 0, 0)); //not needed for calculation
                    T v112 = input->voxel(pos + ivec3(0, 0, 1));
                    T v120 = input->voxel(pos + ivec3(0, -1, -1));
                    T v121 = input->voxel(pos + ivec3(0, -1, 0));
                    T v122 = input->voxel(pos + ivec3(0, -1, 1));
                    //right plane
                    T v200 = input->voxel(pos + ivec3(1, -1, -1));
                    T v201 = input->voxel(pos + ivec3(1, -1, 0));
                    T v202 = input->voxel(pos + ivec3(1, -1, 1));
                    T v210 = input->voxel(pos + ivec3(1, 0, -1));
                    T v211 = input->voxel(pos + ivec3(1, 0, 0));
                    T v212 = input->voxel(pos + ivec3(1, 0, 1));
                    T v220 = input->voxel(pos + ivec3(1, 1, -1));
                    T v221 = input->voxel(pos + ivec3(1, 1, 0));
                    T v222 = input->voxel(pos + ivec3(1, 1, 1));

                    if (intensityCheck){
                        v000 = ((v000 == v111) ? v000 : 0);
                        v001 = ((v001 == v111) ? v001 : 0);
                        v002 = ((v002 == v111) ? v002 : 0);
                        v010 = ((v010 == v111) ? v010 : 0);
                        v011 = ((v011 == v111) ? v011 : 0);
                        v012 = ((v012 == v111) ? v012 : 0);
                        v020 = ((v020 == v111) ? v020 : 0);
                        v021 = ((v021 == v111) ? v021 : 0);
                        v022 = ((v022 == v111) ? v022 : 0);

                        v100 = ((v100 == v111) ? v100 : 0);
                        v101 = ((v101 == v111) ? v101 : 0);
                        v102 = ((v102 == v111) ? v102 : 0);
                        v110 = ((v110 == v111) ? v110 : 0);
                        v112 = ((v112 == v111) ? v112 : 0);
                        v120 = ((v120 == v111) ? v120 : 0);
                        v121 = ((v121 == v111) ? v121 : 0);
                        v122 = ((v122 == v111) ? v122 : 0);

                        v200 = ((v200 == v111) ? v200 : 0);
                        v201 = ((v201 == v111) ? v201 : 0);
                        v202 = ((v202 == v111) ? v202 : 0);
                        v210 = ((v210 == v111) ? v210 : 0);
                        v211 = ((v211 == v111) ? v211 : 0);
                        v212 = ((v212 == v111) ? v212 : 0);
                        v220 = ((v220 == v111) ? v220 : 0);
                        v221 = ((v221 == v111) ? v221 : 0);
                        v222 = ((v222 == v111) ? v222 : 0);
                    }


                    //filter x-direction
                    gradient.x += -1 * v000;
                    gradient.x += -3 * v010;
                    gradient.x += -1 * v020; 
                    gradient.x += 1 * v200;
                    gradient.x += 3 * v210;
                    gradient.x += 1 * v220;
                    gradient.x += -3 * v001;
                    gradient.x += -6 * v011;
                    gradient.x += -3 * v021;
                    gradient.x += +3 * v201;
                    gradient.x += +6 * v211;
                    gradient.x += +3 * v221;
                    gradient.x += -1 * v002;
                    gradient.x += -3 * v012;
                    gradient.x += -1 * v022;							
                    gradient.x += +1 * v202;
                    gradient.x += +3 * v212;
                    gradient.x += +1 * v222;

                    //filter y-direction
                    gradient.y += -1 * v000;
                    gradient.y += -3 * v100;
                    gradient.y += -1 * v200; 
                    gradient.y += +1 * v020;
                    gradient.y += +3 * v120;
                    gradient.y += +1 * v220;
                    gradient.y += -3 * v001;
                    gradient.y += -6 * v101;
                    gradient.y += -3 * v201;
                    gradient.y += +3 * v021;
                    gradient.y += +6 * v121;
                    gradient.y += +3 * v221;
                    gradient.y += -1 * v002;
                    gradient.y += -3 * v102;
                    gradient.y += -1 * v202;
                    gradient.y += +1 * v022;
                    gradient.y += +3 * v122;
                    gradient.y += +1 * v222;

                    //filter z-direction
                    gradient.z += -1 * v000;
                    gradient.z += -3 * v100;
                    gradient.z += -1 * v200; 
                    gradient.z += +1 * v002;
                    gradient.z += +3 * v102;
                    gradient.z += +1 * v202;
                    gradient.z += -3 * v010;
                    gradient.z += -6 * v110;
                    gradient.z += -3 * v210;
                    gradient.z += +3 * v012;
                    gradient.z += +6 * v112;
                    gradient.z += +3 * v212;
                    gradient.z += -1 * v020;
                    gradient.z += -3 * v120;
                    gradient.z += -1 * v220;
                    gradient.z += +1 * v022;
                    gradient.z += +3 * v122;
                    gradient.z += +1 * v222;


                    gradient /= 22.f;   // sum of all positive weights
                    gradient /= 2.f;    // this mask has a step length of 2 voxels
                    gradient /= input->getSpacing();
                    gradient *= -1.f;

                }

                if (v4) {
                    if (bitsU == 8) {
                        reinterpret_cast<Volume4xUInt8*>(result)->voxel(pos).r = static_cast<uint8_t>((gradient.x/mapping)+128.f);
                        reinterpret_cast<Volume4xUInt8*>(result)->voxel(pos).g = static_cast<uint8_t>((gradient.y/mapping)+128.f);
                        reinterpret_cast<Volume4xUInt8*>(result)->voxel(pos).b = static_cast<uint8_t>((gradient.z/mapping)+128.f);
                        reinterpret_cast<Volume4xUInt8*>(result)->voxel(pos).a = static_cast<uint8_t>(input->getVoxelFloat(pos) * 255.0f);
                    }
                    else if (bitsU == 16) {
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).r = static_cast<uint16_t>((gradient.x/mapping * 256.f) + 32768.f);
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).g = static_cast<uint16_t>((gradient.y/mapping * 256.f) + 32768.f);
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).b = static_cast<uint16_t>((gradient.z/mapping * 256.f) + 32768.f);
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).a = static_cast<uint16_t>((input->getVoxelFloat(pos) * 65535.f));
                    }
                    else if (bitsU == 12) {
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).r = static_cast<uint16_t>((gradient.x/mapping * 16.f) + 2048.f);
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).g = static_cast<uint16_t>((gradient.y/mapping * 16.f) + 2048.f);
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).b = static_cast<uint16_t>((gradient.z/mapping * 16.f) + 2048.f);
                        reinterpret_cast<Volume4xUInt16*>(result)->voxel(pos).a = static_cast<uint16_t>((input->getVoxelFloat(pos) * 4095.f));
                    }
                }
                else {
                    if (bitsU == 8) {
                        reinterpret_cast<Volume3xUInt8*>(result)->voxel(pos).r = static_cast<uint8_t>((gradient.x/mapping)+128.f);
                        reinterpret_cast<Volume3xUInt8*>(result)->voxel(pos).g = static_cast<uint8_t>((gradient.y/mapping)+128.f);
                        reinterpret_cast<Volume3xUInt8*>(result)->voxel(pos).b = static_cast<uint8_t>((gradient.z/mapping)+128.f);
                    }
                    else if (bitsU == 16) {
                        reinterpret_cast<Volume3xUInt16*>(result)->voxel(pos).r = static_cast<uint16_t>((gradient.x/mapping * 256.f) + 32768.f);
                        reinterpret_cast<Volume3xUInt16*>(result)->voxel(pos).g = static_cast<uint16_t>((gradient.y/mapping * 256.f) + 32768.f);
                        reinterpret_cast<Volume3xUInt16*>(result)->voxel(pos).b = static_cast<uint16_t>((gradient.z/mapping * 256.f) + 32768.f);
                    }
                     else if (bitsU == 12) {
                        reinterpret_cast<Volume3xUInt16*>(result)->voxel(pos).r = static_cast<uint16_t>((gradient.x/mapping * 16.f) + 2048.f);
                        reinterpret_cast<Volume3xUInt16*>(result)->voxel(pos).g = static_cast<uint16_t>((gradient.y/mapping * 16.f) + 2048.f);
                        reinterpret_cast<Volume3xUInt16*>(result)->voxel(pos).b = static_cast<uint16_t>((gradient.z/mapping * 16.f) + 2048.f);
                    }
                }

            }
        }
    }
    return result;
}

/**
 * Calculates gradients with Sobel operator.
 */ 
template<class U>
VolumeAtomic<U>* calcGradientsSobel(voreen::Volume* vol, bool intensityCheck) {
    if (vol->getBitsStored() == 8) {
        VolumeUInt8* input = (VolumeUInt8*) vol;
        return calcGradientsSobel<U, uint8_t>(input, 2, intensityCheck);
    }
    else if (vol->getBitsStored() == 16) {
        VolumeUInt16* input = (VolumeUInt16*) vol;
        return calcGradientsSobel<U, uint16_t>(input, 512, intensityCheck);
    }
    else if (vol->getBitsStored() == 12) {
        VolumeUInt16* input = (VolumeUInt16*) vol;
        return calcGradientsSobel<U, uint16_t>(input, 32, intensityCheck);
    }
    std::cout << "ERROR: calcGradients needs a 8-, 12- or 16-bit dataset as input" << std::endl;
    return 0;
}


/**
 * Calculates gradient magnitudes from a gradient volume.
 *
 * Use uint8_t or uint16_t as U template argument in order to generate 8 or 16 bit datasets.
 */
template<class U, class T>
VolumeAtomic<U>* calcGradientMagnitudes(VolumeAtomic<T> *input) {
    
    VolumeAtomic<U>* result = new VolumeAtomic<U>(input->getDimensions(), input->getSpacing());
    
    float maxValueT;
    if ( typeid(*input) == typeid(Volume3xUInt8)  || 
         typeid(*input) == typeid(Volume3xUInt16) || 
         typeid(*input) == typeid(Volume4xUInt8)  || 
         typeid(*input) == typeid(Volume4xUInt16)    ) {
         int bitsT = input->getBitsStored() / input->getNumChannels();
        maxValueT = static_cast<float>( (1 << bitsT) - 1);
    }
    else if ( typeid(*input) == typeid(Volume3xFloat) || typeid(*input) == typeid(Volume3xDouble) ||
        typeid(*input) == typeid(Volume4xFloat) || typeid(*input) == typeid(Volume4xDouble) ) {
        maxValueT = 1.f;
    }
    else { 
        LERRORC("calcGradientMagnitudes", "Unknown or unsupported input volume type");
        tgtAssert(false, "Unknown or unsupported input volume type");
        return result;
    }

    float maxValueU;
    if ( typeid(*result) == typeid(VolumeUInt8)  || 
         typeid(*result) == typeid(VolumeUInt16) || 
         typeid(*result) == typeid(VolumeUInt32) )
        maxValueU = static_cast<float>( (1 << result->getBitsStored()) - 1);
    else if ( typeid(*result) == typeid(VolumeFloat) || typeid(*result) == typeid(VolumeDouble))
        maxValueU = 1.f;
    else { 
        LERRORC("calc2ndDerivatives", "Unknown or unsupported output volume type");
        tgtAssert(false, "Unknown or unsupported output volume type");
        return result;
    }
    

    ivec3 pos;
    ivec3 dim = input->getDimensions();
    for (pos.z = 0; pos.z < dim.z; pos.z++) {
        for (pos.y = 0; pos.y < dim.y; pos.y++) {
            for (pos.x = 0; pos.x < dim.x; pos.x++) {

                vec3 gradient;
                gradient.x = input->getVoxelFloat(pos, 0);
                gradient.y = input->getVoxelFloat(pos, 1);
                gradient.z = input->getVoxelFloat(pos, 2);
    
                // input value range is [0:maxValue] with (maxValue/2.f) corrensponding to zero
                gradient = (gradient*2.f)-1.f;

                float gradientMagnitude = tgt::length(gradient);

                //result->voxel(pos) = static_cast<U>( ( (derivative / maxValueT) / 2.f + 0.5f ) * maxValueU );
                result->voxel(pos) = static_cast<U>( gradientMagnitude * maxValueU );

            }
        }
    }
    


    return result;
  
}


/**
 * Computes an simple approximation of the second directional derivative along the gradient direction at each voxel.
 * The calculation is done by applying the Laplacian operator.
 *
 * Use uint8_t or uint16_t as U template argument in order to generate 8 or 16 bit datasets.
 */
template<class U, class T>
VolumeAtomic<U>* calc2ndDerivatives(VolumeAtomic<T> *input) {
    
    VolumeAtomic<U>* result = new VolumeAtomic<U>(input->getDimensions(), input->getSpacing());
    
    float maxValueT;
    if ( typeid(*input) == typeid(VolumeUInt8)  || 
         typeid(*input) == typeid(VolumeUInt16) || 
         typeid(*input) == typeid(VolumeUInt32) )
        maxValueT = static_cast<float>( (1 << input->getBitsStored()) - 1);
    else if ( typeid(*input) == typeid(VolumeFloat) || typeid(*input) == typeid(VolumeDouble))
        maxValueT = 1.f;
    else { 
        LERRORC("calc2ndDerivatives", "Unknown or unsupported input volume type");
        tgtAssert(false, "Unknown or unsupported input volume type");
        return result;
    }

    float maxValueU;
    if ( typeid(*result) == typeid(VolumeUInt8)  || 
         typeid(*result) == typeid(VolumeUInt16) || 
         typeid(*result) == typeid(VolumeUInt32) )
        maxValueU = static_cast<float>( (1 << result->getBitsStored()) - 1);
    else if ( typeid(*result) == typeid(VolumeFloat) || typeid(*result) == typeid(VolumeDouble))
        maxValueU = 1.f;
    else { 
        LERRORC("calc2ndDerivatives", "Unknown or unsupported output volume type");
        tgtAssert(false, "Unknown or unsupported output volume type");
        return result;
    }
    

    float derivative;
    ivec3 pos;
    ivec3 dim = input->getDimensions();
    for (pos.z = 0; pos.z < dim.z; pos.z++) {
        for (pos.y = 0; pos.y < dim.y; pos.y++) {
            for (pos.x = 0; pos.x < dim.x; pos.x++) {

                if (pos.x >= 1 && pos.x < dim.x-1 &&
                    pos.y >= 1 && pos.y < dim.y-1 &&
                    pos.z >= 1 && pos.z < dim.z-1)
                {
                    //left plane
                    //T v000 = input->voxel(pos + ivec3(-1,-1,-1));
                    //T v001 = input->voxel(pos + ivec3(-1, -1, 0));
                    //T v002 = input->voxel(pos + ivec3(-1, -1, 1));
                    //T v010 = input->voxel(pos + ivec3(-1, 0, -1));
                    T v011 = input->voxel(pos + ivec3(-1, 0, 0));
                    //T v012 = input->voxel(pos + ivec3(-1, 0, 1));
                    //T v020 = input->voxel(pos + ivec3(-1, 1, -1));
                    //T v021 = input->voxel(pos + ivec3(-1, 1, 0));
                    //T v022 = input->voxel(pos + ivec3(-1, 1, 1));
                    
                    //mid plane
                    //T v100 = input->voxel(pos + ivec3(0, -1, -1));
                    T v101 = input->voxel(pos + ivec3(0, -1, 0));
                    //T v102 = input->voxel(pos + ivec3(0, -1, 1));
                    T v110 = input->voxel(pos + ivec3(0, 0, -1));
                    T v111 = input->voxel(pos + ivec3(0, 0, 0)); 
                    T v112 = input->voxel(pos + ivec3(0, 0, 1));
                    //T v120 = input->voxel(pos + ivec3(0, 1, -1));
                    T v121 = input->voxel(pos + ivec3(0, 1, 0));
                    //T v122 = input->voxel(pos + ivec3(0, 1, 1));
                    
                    //right plane
                    //T v200 = input->voxel(pos + ivec3(1, -1, -1));
                    //T v201 = input->voxel(pos + ivec3(1, -1, 0));
                    //T v202 = input->voxel(pos + ivec3(1, -1, 1));
                    //T v210 = input->voxel(pos + ivec3(1, 0, -1));
                    T v211 = input->voxel(pos + ivec3(1, 0, 0));
                    //T v212 = input->voxel(pos + ivec3(1, 0, 1));
                    //T v220 = input->voxel(pos + ivec3(1, 1, -1));
                    //T v221 = input->voxel(pos + ivec3(1, 1, 0));
                    //T v222 = input->voxel(pos + ivec3(1, 1, 1));

                    // Original Laplace operator 
                    derivative = static_cast<float>( -6.0*v111                  + 
                                                      v101 + v110 + v112 + v121  + 
                                                      v011 + v211     );  
                   
                    // Simple Laplacian of Gaussian
                    /*derivative = static_cast<double>( -36.0*v111 + 
                                                      4.0*v101 + 4.0*v110 + 4.0*v112 + 4.0*v121  + 
                                                      4.0*v011 + 4.0*v211 +
                                                      v100 + v102 + v120 + v122 +
                                                      v001 + v010 + v012 + v021 +
                                                      v201 + v210 + v212 + v221      ) / 8.0;  */

                }
                else {
                    derivative = 0.f;
                }

                // map value from [-maxT:maxT] to [0:maxU] since we expect an unsigned volume as input/output type
                result->voxel(pos) = static_cast<U>( ( (derivative / maxValueT) / 2.f + 0.5f ) * maxValueU );

            }
        }
    }
    


    return result;
  
}


/**
 * Calculates gradients with neighborhood of 26.
 * Slower, but should result in better gradients.
 */
Volume4xUInt8* calcGradients26(voreen::Volume* vol);

/**
 * Filters gradients stored in a 32 bit volume data set. A very simple interpolation
 * adapted to binary data sets is performed, which replaces zero gradients by
 * non-zero gradients in the direct vicinity.
 *
 * @param vol 32 bit volume data set with stored gradients.
 */
Volume4xUInt8* filterGradients(voreen::Volume* vol);

/**
 * Filters gradients stored in a 32 bit volume data set.
 * The mid value of gradients in vicinity is saved.
 *
 * @param vol 32 bit volume data set with stored gradients.
 */
Volume4xUInt8* filterGradientsMid(voreen::Volume* vol);

/**
 * Filters gradients stored in a 32 bit volume data set.
 * gradient values in vicinity are weighted.
 *
 * @param vol 32 bit volume data set with stored gradients.
 */
Volume4xUInt8* filterGradientsWeighted(Volume* vol, bool intensityCheck);

} // namespace

#endif //VRN_GRADIENT_H
