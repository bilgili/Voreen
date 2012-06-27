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

#include "voreen/core/datastructures/volume/gradient.h"

#include "tgt/logmanager.h"

using namespace tgt;

namespace voreen {

Volume4xUInt8* calcGradients26(Volume* vol) {
    // generate 32 bit data set to store results
    Volume4xUInt8* result = new Volume4xUInt8(vol->getDimensions(), vol->getSpacing(), vol->getTransformation(), 32);
    if (vol->getBitsStored() == 8) {
        VolumeUInt8* input = static_cast<VolumeUInt8*>(vol);

        vec3 offset = vec3(1.0, 1.0, 1.0);
        ivec3 gradient;

        ivec3 pos;
        ivec3 dim = vol->getDimensions();
        uint8_t gX,gY,gZ;

        for (pos.z = 0; pos.z < vol->getDimensions().z; ++pos.z) {
            for (pos.y = 0; pos.y < vol->getDimensions().y; ++pos.y) {
                for (pos.x = 0; pos.x < vol->getDimensions().x; ++pos.x) {

                    if (pos.x >= 1 && pos.x < dim.x-1 &&
                        pos.y >= 1 && pos.y < dim.y-1 &&
                        pos.z >= 1 && pos.z < dim.z-1)
                    {
                        //left plane
                        uint8_t v000 = input->voxel(vec3(pos) + vec3(-offset.x, -offset.y, -offset.z));
                        uint8_t v001 = input->voxel(vec3(pos) + vec3(-offset.x, -offset.y, 0.0));
                        uint8_t v002 = input->voxel(vec3(pos) + vec3(-offset.x, -offset.y, offset.z));
                        uint8_t v010 = input->voxel(vec3(pos) + vec3(-offset.x, 0.0, -offset.z));
                        uint8_t v011 = input->voxel(vec3(pos) + vec3(-offset.x, 0.0, 0.0));
                        uint8_t v012 = input->voxel(vec3(pos) + vec3(-offset.x, 0.0, offset.z));
                        uint8_t v020 = input->voxel(vec3(pos) + vec3(-offset.x, offset.y, -offset.z));
                        uint8_t v021 = input->voxel(vec3(pos) + vec3(-offset.x, offset.y, 0.0));
                        uint8_t v022 = input->voxel(vec3(pos) + vec3(-offset.x, offset.y, offset.z));
                        //mid plane
                        uint8_t v100 = input->voxel(vec3(pos) + vec3(0.0, -offset.y, -offset.z));
                        uint8_t v101 = input->voxel(vec3(pos) + vec3(0.0, -offset.y, 0.0));
                        uint8_t v102 = input->voxel(vec3(pos) + vec3(0.0, -offset.y, offset.z));
                        uint8_t v110 = input->voxel(vec3(pos) + vec3(0.0, 0.0, -offset.z));
                        uint8_t v111 = input->voxel(vec3(pos) + vec3(0.0, 0.0, 0.0)); //not needed for calculation
                        uint8_t v112 = input->voxel(vec3(pos) + vec3(0.0, 0.0, offset.z));
                        uint8_t v120 = input->voxel(vec3(pos) + vec3(0.0, offset.y, -offset.z));
                        uint8_t v121 = input->voxel(vec3(pos) + vec3(0.0, offset.y, 0.0));
                        uint8_t v122 = input->voxel(vec3(pos) + vec3(0.0, offset.y, offset.z));
                        //right plane
                        uint8_t v200 = input->voxel(vec3(pos) + vec3(offset.x, -offset.y, -offset.z));
                        uint8_t v201 = input->voxel(vec3(pos) + vec3(offset.x, -offset.y, 0.0));
                        uint8_t v202 = input->voxel(vec3(pos) + vec3(offset.x, -offset.y, offset.z));
                        uint8_t v210 = input->voxel(vec3(pos) + vec3(offset.x, 0.0, -offset.z));
                        uint8_t v211 = input->voxel(vec3(pos) + vec3(offset.x, 0.0, 0.0));
                        uint8_t v212 = input->voxel(vec3(pos) + vec3(offset.x, 0.0, offset.z));
                        uint8_t v220 = input->voxel(vec3(pos) + vec3(offset.x, offset.y, -offset.z));
                        uint8_t v221 = input->voxel(vec3(pos) + vec3(offset.x, offset.y, 0.0));
                        uint8_t v222 = input->voxel(vec3(pos) + vec3(offset.x, offset.y, offset.z));

                        gradient = ivec3(0,0,0);
                        gradient += -1*(v211 - v011) * ivec3(1,0,0);
                        gradient += -1*(v121 - v101) * ivec3(0,1,0);
                        gradient += -1*(v112 - v110) * ivec3(0,0,1);

                        gradient += -1*(v221 - v001) * ivec3(1,1,0);
                        gradient += -1*(v201 - v021) * ivec3(1,-1,0);
                        gradient += -1*(v122 - v100) * ivec3(0,1,1);
                        gradient += -1*(v102 - v120) * ivec3(0,-1,1);
                        gradient += -1*(v012 - v210) * ivec3(-1,0,1);
                        gradient += -1*(v212 - v010) * ivec3(1,0,1);

                        gradient += -1*(v222 - v000) * ivec3(1,1,1);
                        gradient += -1*(v022 - v200) * ivec3(-1,1,1);
                        gradient += -1*(v202 - v020) * ivec3(1,-1,1);
                        gradient += -1*(v220 - v002) * ivec3(1,1,-1);

                        gradient /= 9;

                        gX = static_cast<uint8_t>(gradient.x/2)+128;
                        gY = static_cast<uint8_t>(gradient.y/2)+128;
                        gZ = static_cast<uint8_t>(gradient.z/2)+128;
                        result->voxel(pos) = col4(gX, gY, gZ, v111);
                    }
                    else
                        result->voxel(pos) = col4(128, 128, 128, input->voxel(pos));
                }
            }
        }
    }
    else {
        LERRORC("gradient", "calcGradients26 does currently only work with a 8-bit dataset as input");
        delete result;
        return 0;
    }
    return result;

}

Volume4xUInt8* filterGradientsMid(Volume* vol) {
   if (vol->getBitsStored() == 32) {
       Volume4xUInt8* input = static_cast<Volume4xUInt8*>(vol);

       Volume4xUInt8* resultFiltered = new Volume4xUInt8(vol->getDimensions(), vol->getSpacing(), vol->getTransformation(), 32);
       vec3 delta = vec3(1.0);

       ivec3 pos;
       ivec3 dim = vol->getDimensions();
       for (pos.z = 0; pos.z < vol->getDimensions().z; ++pos.z) {
           for (pos.y = 0; pos.y < vol->getDimensions().y; ++pos.y) {
               for (pos.x = 0; pos.x < vol->getDimensions().x; ++pos.x) {

                   if (pos.x >= 1 && pos.x < dim.x-1 &&
                       pos.y >= 1 && pos.y < dim.y-1 &&
                       pos.z >= 1 && pos.z < dim.z-1)
                   {
                       vec4 g0 = input->voxel(vec3(pos));

                       vec4 g1 = input->voxel(vec3(pos)+vec3(-delta.x, -delta.y, -delta.z));
                       vec4 g2 = input->voxel(vec3(pos)+vec3( delta.x,  delta.y,  delta.z));
                       vec4 g3 = input->voxel(vec3(pos)+vec3(-delta.x,  delta.y, -delta.z));
                       vec4 g4 = input->voxel(vec3(pos)+vec3( delta.x, -delta.y,  delta.z));
                       vec4 g5 = input->voxel(vec3(pos)+vec3(-delta.x, -delta.y,  delta.z));
                       vec4 g6 = input->voxel(vec3(pos)+vec3( delta.x,  delta.y, -delta.z));
                       vec4 g7 = input->voxel(vec3(pos)+vec3(-delta.x,  delta.y,  delta.z));
                       vec4 g8 = input->voxel(vec3(pos)+vec3( delta.x, -delta.y, -delta.z));

                       vec4 mix0 = vec4(0.5)*(vec4(0.5)*g1+vec4(0.5)*g2) + vec4(0.5)*(vec4(0.5)*g3+vec4(0.5)*g4);
                       vec4 mix1 = vec4(0.5)*(vec4(0.5)*g5+vec4(0.5)*g6) + vec4(0.5)*(vec4(0.5)*g7+vec4(0.5)*g8);

                       vec4 filteredGrad = vec4(0.25)*(vec4(0.5)*mix0+vec4(0.5)*mix1) + vec4(0.75)*g0;
                       resultFiltered->voxel(pos) = vec4(filteredGrad.x, filteredGrad.y, filteredGrad.z, g0.a);
                   }
                   else
                       resultFiltered->voxel(pos) = vec4(0.f,0.f,0.f,input->voxel(vec3(pos)).a);
               }
           }
       }
       return resultFiltered;
   }
   else {
       LERRORC("gradient", "filterGradients needs a 32-bit dataset as input");
       return 0;
   }
}

Volume4xUInt8* filterGradientsWeighted(Volume* vol, bool intensityCheck) {
    if (vol->getBitsStored() == 32) {
        Volume4xUInt8* input = static_cast<Volume4xUInt8*>(vol);

        Volume4xUInt8* resultFiltered = new Volume4xUInt8(vol->getDimensions(), vol->getSpacing(), vol->getTransformation(), 32);
        vec3 delta = vec3(1.0);

        ivec3 pos;
        ivec3 dim = vol->getDimensions();
        for (pos.z = 0; pos.z < vol->getDimensions().z; ++pos.z) {
            for (pos.y = 0; pos.y < vol->getDimensions().y; ++pos.y) {
                for (pos.x = 0; pos.x < vol->getDimensions().x; ++pos.x) {

                    if (pos.x >= 1 && pos.x < dim.x-1 &&
                        pos.y >= 1 && pos.y < dim.y-1 &&
                        pos.z >= 1 && pos.z < dim.z-1)
                    {
                        vec4 g000 = input->voxel(vec3(pos) + vec3(-delta.x, -delta.y, -delta.z)) ;
                        vec4 g001 = input->voxel(vec3(pos) + vec3(-delta.x, -delta.y, 0.0)) ;
                        vec4 g002 = input->voxel(vec3(pos) + vec3(-delta.x, -delta.y, delta.z)) ;
                        vec4 g010 = input->voxel(vec3(pos) + vec3(-delta.x, 0.0, -delta.z)) ;
                        vec4 g011 = input->voxel(vec3(pos) + vec3(-delta.x, 0.0, 0.0)) ;
                        vec4 g012 = input->voxel(vec3(pos) + vec3(-delta.x, 0.0, delta.z)) ;
                        vec4 g020 = input->voxel(vec3(pos) + vec3(-delta.x, delta.y, -delta.z)) ;
                        vec4 g021 = input->voxel(vec3(pos) + vec3(-delta.x, delta.y, 0.0)) ;
                        vec4 g022 = input->voxel(vec3(pos) + vec3(-delta.x, delta.y, delta.z)) ;
                        //mid plane
                        vec4 g100 = input->voxel(vec3(pos) + vec3(0.0, -delta.y, -delta.z)) ;
                        vec4 g101 = input->voxel(vec3(pos) + vec3(0.0, -delta.y, 0.0)) ;
                        vec4 g102 = input->voxel(vec3(pos) + vec3(0.0, -delta.y, delta.z)) ;
                        vec4 g110 = input->voxel(vec3(pos) + vec3(0.0, 0.0, -delta.z)) ;
                        vec4 g111 = input->voxel(vec3(pos) + vec3(0.0, 0.0, 0.0)) ; //not needed for calculation
                        vec4 g112 = input->voxel(vec3(pos) + vec3(0.0, 0.0, delta.z)) ;
                        vec4 g120 = input->voxel(vec3(pos) + vec3(0.0, delta.y, -delta.z)) ;
                        vec4 g121 = input->voxel(vec3(pos) + vec3(0.0, delta.y, 0.0)) ;
                        vec4 g122 = input->voxel(vec3(pos) + vec3(0.0, delta.y, delta.z)) ;
                        //right plane
                        vec4 g200 = input->voxel(vec3(pos) + vec3(delta.x, -delta.y, -delta.z)) ;
                        vec4 g201 = input->voxel(vec3(pos) + vec3(delta.x, -delta.y, 0.0)) ;
                        vec4 g202 = input->voxel(vec3(pos) + vec3(delta.x, -delta.y, delta.z)) ;
                        vec4 g210 = input->voxel(vec3(pos) + vec3(delta.x, 0.0, -delta.z)) ;
                        vec4 g211 = input->voxel(vec3(pos) + vec3(delta.x, 0.0, 0.0)) ;
                        vec4 g212 = input->voxel(vec3(pos) + vec3(delta.x, 0.0, delta.z)) ;
                        vec4 g220 = input->voxel(vec3(pos) + vec3(delta.x, delta.y, -delta.z)) ;
                        vec4 g221 = input->voxel(vec3(pos) + vec3(delta.x, delta.y, 0.0)) ;
                        vec4 g222 = input->voxel(vec3(pos) + vec3(delta.x, delta.y, delta.z)) ;

                        if (intensityCheck) {
                            g000 = ((g000.a == g111.a) ? g000 : vec4(128,128,128,0));
                            g001 = ((g001.a == g111.a) ? g001 : vec4(128,128,128,0));
                            g002 = ((g002.a == g111.a) ? g002 : vec4(128,128,128,0));
                            g010 = ((g010.a == g111.a) ? g010 : vec4(128,128,128,0));
                            g011 = ((g011.a == g111.a) ? g011 : vec4(128,128,128,0));
                            g012 = ((g012.a == g111.a) ? g012 : vec4(128,128,128,0));
                            g020 = ((g020.a == g111.a) ? g020 : vec4(128,128,128,0));
                            g021 = ((g021.a == g111.a) ? g021 : vec4(128,128,128,0));
                            g022 = ((g022.a == g111.a) ? g022 : vec4(128,128,128,0));

                            g100 = ((g100.a == g111.a) ? g100 : vec4(128,128,128,0));
                            g101 = ((g101.a == g111.a) ? g101 : vec4(128,128,128,0));
                            g102 = ((g102.a == g111.a) ? g102 : vec4(128,128,128,0));
                            g110 = ((g110.a == g111.a) ? g110 : vec4(128,128,128,0));
                            g112 = ((g112.a == g111.a) ? g112 : vec4(128,128,128,0));
                            g120 = ((g120.a == g111.a) ? g120 : vec4(128,128,128,0));
                            g121 = ((g121.a == g111.a) ? g121 : vec4(128,128,128,0));
                            g122 = ((g122.a == g111.a) ? g122 : vec4(128,128,128,0));

                            g200 = ((g200.a == g111.a) ? g200 : vec4(128,128,128,0));
                            g201 = ((g201.a == g111.a) ? g201 : vec4(128,128,128,0));
                            g202 = ((g202.a == g111.a) ? g202 : vec4(128,128,128,0));
                            g210 = ((g210.a == g111.a) ? g210 : vec4(128,128,128,0));
                            g211 = ((g211.a == g111.a) ? g211 : vec4(128,128,128,0));
                            g212 = ((g212.a == g111.a) ? g212 : vec4(128,128,128,0));
                            g220 = ((g220.a == g111.a) ? g220 : vec4(128,128,128,0));
                            g221 = ((g221.a == g111.a) ? g221 : vec4(128,128,128,0));
                            g222 = ((g222.a == g111.a) ? g222 : vec4(128,128,128,0));
                        }

                        float weightDiagDiag = 1;
                        float weightDiag = 3;
                        float weightDirect = 6;
                        float weightMain = 9;

                        float sumWeight = 8 * weightDiagDiag + 12 * weightDiag + 6 * weightDirect + weightMain;

                        vec4 mix1 = weightDiagDiag*g000 + weightDiag*g001 + weightDiagDiag*g002
                            + weightDiag*g010 + weightDirect*g011 + weightDiag*g012 + weightDiagDiag*g020
                            + weightDiag*g021 + weightDiagDiag*g022;

                        vec4 mix2 = weightDiag*g100 + weightDirect*g101 + weightDiag*g102 + weightDirect*g110
                            + weightMain*g111 + weightDirect*g112 + weightDiag*g120 + weightDirect*g121 + weightDiag*g122;
                        vec4 mix3 = weightDiagDiag*g200 + weightDiag*g201 + weightDiagDiag*g202 + weightDiag*g210
                            + weightDirect*g211 + weightDiag*g212 + weightDiagDiag*g220 + weightDiag*g221 + weightDiagDiag*g222;

                        vec4 filteredGrad = (mix1+mix2+mix3)/(sumWeight);
                        resultFiltered->voxel(pos) = vec4(filteredGrad.x, filteredGrad.y, filteredGrad.z,
                            input->voxel(vec3(pos)).a);
                    }
                    else
                        resultFiltered->voxel(pos) = vec4(0.f,0.f,0.f,input->voxel(vec3(pos)).a);
                }
            }
        }
        return resultFiltered;
    }
    else {
        LERRORC("gradient", "filterGradients needs a 32-bit dataset as input");
        return 0;
    }
}

Volume4xUInt8* filterGradients(Volume* vol) {
    if (vol->getBitsStored() == 32) {
        Volume4xUInt8* input = static_cast<Volume4xUInt8*>(vol);

        Volume4xUInt8* resultFiltered = new Volume4xUInt8(vol->getDimensions(), vol->getSpacing(), vol->getTransformation(), 32);
        vec3 offset = vec3(1.0);
        // gradients are stored in the interval [0..256], with (128,128,128) being the zero gradient
        vec3 zeroGrad = vec3(128,128,128);

        ivec3 pos;
        vec4 curGrad;
        vec4 filteredGrad;
        ivec3 dim = vol->getDimensions();
        for (pos.z = 0; pos.z < vol->getDimensions().z; ++pos.z) {
            for (pos.y = 0; pos.y < vol->getDimensions().y; ++pos.y) {
                for (pos.x = 0; pos.x < vol->getDimensions().x; ++pos.x) {

                    if (pos.x >= 1 && pos.x < dim.x-1 &&
                        pos.y >= 1 && pos.y < dim.y-1 &&
                        pos.z >= 1 && pos.z < dim.z-1)
                    {
                        curGrad = input->voxel(vec3(pos));
                        filteredGrad = curGrad;

                        // check if the current gradient is a zero gradient
                        if (curGrad.xyz() == zeroGrad) {
                            vec4 v0 = input->voxel(vec3(pos) + vec3(offset.x, 0.0, 0.0));
                            if (v0.xyz() != zeroGrad)
                                filteredGrad = v0;
                            else {
                                vec4 v1 = input->voxel(vec3(pos) + vec3(0, offset.y, 0));
                                if (v1.xyz() != zeroGrad)
                                    filteredGrad = v1;
                                else {
                                    vec4 v2 = input->voxel(vec3(pos) + vec3(0, 0, offset.z));
                                    if (v2.xyz() != zeroGrad)
                                        filteredGrad = v2;
                                    else {
                                        vec4 v3 = input->voxel(vec3(pos) + vec3(-offset.x, 0, 0));
                                        if (v3.xyz() != zeroGrad)
                                            filteredGrad = v3;
                                        else {
                                            vec4 v4 = input->voxel(vec3(pos) + vec3(0, -offset.y, 0));
                                            if (v4.xyz() != zeroGrad)
                                                filteredGrad = v4;
                                            else {
                                                vec4 v5 = input->voxel(vec3(pos) + vec3(0, 0, -offset.z));
                                                if (v5.xyz() != zeroGrad)
                                                    filteredGrad = v5;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    resultFiltered->voxel(pos) = vec4(filteredGrad.x, filteredGrad.y, filteredGrad.z, curGrad.a);
                }
            }
        }
        return resultFiltered;
    }
    else {
        LERRORC("gradient", "filterGradients needs a 32-bit dataset as input");
        return 0;
    }
}

} // namespace voreen
