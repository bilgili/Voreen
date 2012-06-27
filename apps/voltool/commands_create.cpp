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

#include "commands_create.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/volume/volumecontainer.h"
 
namespace voreen {

CommandGenerateMask::CommandGenerateMask() {
    help_ = "\n";
    
    info_ = "";
    name_ = "generatemask";
    syntax_ = name_ + " DX DY DZ RADIUS CX CY CZ OUT";
    loggerCat_ += "." + name_;
}
    
bool CommandGenerateMask::execute(const std::vector<std::string>& parameters) {
    checkParameters(parameters.size() == 8);
    
    tgt::vec3 center;
    tgt::ivec3 dimensions;
    dimensions.x = asInt(parameters[0]);
    dimensions.y = asInt(parameters[1]);
    dimensions.z = asInt(parameters[2]);
    float radius = asFloat(parameters[3]);
    center.x = asFloat(parameters[4]);
    center.y = asFloat(parameters[5]);
    center.z = asFloat(parameters[6]);

    LINFO("Generating mask dataset with dimensions: " << dimensions);

    VolumeUInt8* target = new VolumeUInt8(dimensions);

    for (int voxel_z=0; voxel_z<dimensions.z; voxel_z++) {
        for (int voxel_y=0; voxel_y<dimensions.y; voxel_y++) {
            for (int voxel_x=0; voxel_x<dimensions.x; voxel_x++) {
                tgt::vec3 diff = center - tgt::vec3((float)voxel_x, (float)voxel_y, (float)voxel_z);
                if(length(diff) <= (float)radius)
                    target->voxel(voxel_x, voxel_y, voxel_z) = (uint8_t)255;
                else
                    target->voxel(voxel_x, voxel_y, voxel_z)= (uint8_t)0;
            }
        }
    }
    
    if(target) {
        VolumeSerializerPopulator volLoadPop;
        VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();
        serializer->save(parameters.back(), target);
        delete target;
        delete serializer;
        return true;
    }
    else
        return false;
}

//-----------------------------------------------------------------------------

CommandCreate::CommandCreate() {
    help_ = "Writes 8-bit dataset with dimensions SIZExSIZExSIZE\n";
    help_ += "Possible datasets:\n";
    help_ += "cornell: creates a cornell box\n";
    help_ += "cube:\n";
    help_ += "synth: \n";
    help_ += "cloud: \n";
    help_ += "aotestbox: \n";
    help_ += "shadowtestvol: \n";
    
    info_ = "Create datasets.";
    name_ = "create";
    syntax_ = name_ + " [cornell|cube|synth|cloud|aotestbox|shadowtestvol] SIZE OUT8";
    loggerCat_ += "." + name_;
}
    
bool CommandCreate::execute(const std::vector<std::string>& parameters) {
    checkParameters(parameters.size() == 3);
    
    std::string operation = parameters[0];
    
    tgt::ivec3 dimensions;
    dimensions.x = asInt(parameters[1]);
    dimensions.y = dimensions.x;
    dimensions.z = dimensions.x;
    
    VolumeUInt8* target = new VolumeUInt8(dimensions);
    
    if (operation == "cornell") {
        tgt::vec3 center = dimensions / 2;

        int s = dimensions.x;
        int thickness = s / 15;
        int border = 3;
        int is = s - ((thickness+border)*2); //the inner size of the box

        uint8_t white = 200;
        uint8_t red = 70;
        uint8_t green = 150;

        uint8_t box0 = 100;
        uint8_t box1 = 50;
        uint8_t sphere = 120;

//         uint8_t light = 240;
//         uint8_t fog = 20;

//         float radius = dimensions.x / 4;

        LINFO("Generating cornell dataset with dimensions: " << dimensions);

        //clear dataset:
        fillBox(target, tgt::ivec3(0,0,0), dimensions, 0);


        //fill with fog
        /*
        fillBox(target, tgt::ivec3(border+thickness, border+thickness, border+thickness),
                tgt::ivec3(border+thickness+is, border+thickness+is, border+thickness+is), fog);
                */


        //white floor
        fillBox(target, tgt::ivec3(border, border, border), tgt::ivec3(s-border, border+thickness, s-border), white);
        //white back wall:
        fillBox(target, tgt::ivec3(border, border, border), tgt::ivec3(s-border, s-border, border+thickness), white);
        //white ceiling:
        fillBox(target, tgt::ivec3(border, s-border-thickness, border), tgt::ivec3(s-border, s-border, s-border), white);

        //THE NEXT 2 ARE OPTIONAL:
        //white front wall:
//         fillBox(target, tgt::ivec3(border, border, border+thickness+is), tgt::ivec3(s-border, s-border, s-border), white);
        //cut hole in ceiling (lightsource)
//         fillBox(target, tgt::ivec3(center.x-thickness*2, s-border-thickness, center.x-thickness*2), tgt::ivec3(center.x+thickness*2, s-border, center.x+thickness*2), light);
        fillBox(target, tgt::ivec3(center.x-thickness*2, s-border-thickness, center.x-thickness*2), tgt::ivec3(center.x+thickness*2, s-border, center.x+thickness*2), 0);

        //green right wall:
        fillBox(target, tgt::ivec3(s-thickness-border,thickness+border,thickness+border), tgt::ivec3(s-border,s-thickness-border,s-border), green);

        //red right wall:
        fillBox(target, tgt::ivec3(border,thickness+border,thickness+border), tgt::ivec3(border+thickness,s-thickness-border,s-border), red);

        //box0: oriented box, left, back
        fillOrientedBox(target, tgt::vec3(border+thickness+(int)(is*0.3f), 0.0f, border+thickness+(int)(is*0.3f)), tgt::vec3(1.0f, 0.0f, 2.0f), (int)(is*0.15), (int)(is*0.2), border+thickness, border+thickness+(int)is*0.6f, box0);

        //box1: oriented box, right, front
        fillOrientedBox(target, tgt::vec3(border+thickness+(int)(is*0.7f), 0.0f, border+thickness+(int)(is*0.8f)), tgt::vec3(2.0f, 0.0f, 1.0f), (int)(is*0.15), (int)(is*0.15), border+thickness, border+thickness+(int)is*0.3f, box1);

        //sphere, left, front
        fillSphere(target, tgt::vec3(border+thickness+(int)(is*0.3f), border+thickness+is/6, border+thickness+(int)(is*0.8f)), is/6, sphere);
//         fillBox(targetDataset_, tgt::ivec3(), tgt::ivec3(), );

//         void fillSphere(VolumeDataset* vds, tgt::vec3 center, float radius, uint8_t value);
//         void fillBox(VolumeDataset* vds, tgt::ivec3 start, tgt::ivec3 end, uint8_t value);

    }
    else if (operation == "cube") {
        tgt::vec3 center;
        center = dimensions / 2;

        int s = dimensions.x;
        int thickness = s / 15;
        int border = 3;
//         int is = s - ((thickness+border)*2); //the inner size of the box

        uint8_t white = 200;
//         uint8_t red = 70;
//         uint8_t green = 150;

//         uint8_t box0 = 100;
//         uint8_t box1 = 50;
//         uint8_t sphere = 120;

//         uint8_t light = 240;
//         uint8_t fog = 20;

//         float radius = (float)dimensions.x / 4.0f;

        LINFO("Generating cornell dataset with dimensions: " << dimensions);

        //clear dataset:
        fillBox(target, tgt::ivec3(0,0,0), dimensions, 0);

        //fill with fog
        /*
         fillBox(targetDataset_, tgt::ivec3(border+thickness, border+thickness, border+thickness),
                tgt::ivec3(border+thickness+is, border+thickness+is, border+thickness+is), fog);
*/
        //white floor
        fillBox(target, tgt::ivec3(border, border, border), tgt::ivec3(s-border, border+thickness, s-border), white);
        //white back wall:
        /*
        fillBox(targetDataset_, tgt::ivec3(border, border, border), tgt::ivec3(s-border, s-border, border+thickness), white);
        //white ceiling:
        fillBox(targetDataset_, tgt::ivec3(border, s-border-thickness, border), tgt::ivec3(s-border, s-border, s-border), white);

        //THE NEXT 2 ARE OPTIONAL:
        //white front wall:
//         fillBox(targetDataset_, tgt::ivec3(border, border, border+thickness+is), tgt::ivec3(s-border, s-border, s-border), white);
        //cut hole in ceiling (lightsource)
        fillBox(targetDataset_, tgt::ivec3(center.x-thickness*2, s-border-thickness, center.x-thickness*2), tgt::ivec3(center.x+thickness*2, s-border, center.x+thickness*2), light);

        //green right wall:
        fillBox(targetDataset_, tgt::ivec3(s-thickness-border,thickness+border,thickness+border), tgt::ivec3(s-border,s-thickness-border,s-border), green);

        //red right wall:
        fillBox(targetDataset_, tgt::ivec3(border,thickness+border,thickness+border), tgt::ivec3(border+thickness,s-thickness-border,s-border), red);

        //box0: oriented box, left, back
        fillOrientedBox(targetDataset_, tgt::vec3(border+thickness+(int)(is*0.3f), 0.0f, border+thickness+(int)(is*0.3f)), tgt::vec3(1.0f, 0.0f, 2.0f), (int)(is*0.15), (int)(is*0.2), border+thickness, border+thickness+(int)is*0.6f, box0);

        //box1: oriented box, right, front
        fillOrientedBox(targetDataset_, tgt::vec3(border+thickness+(int)(is*0.7f), 0.0f, border+thickness+(int)(is*0.8f)), tgt::vec3(2.0f, 0.0f, 1.0f), (int)(is*0.15), (int)(is*0.15), border+thickness, border+thickness+(int)is*0.3f, box1);

        //sphere, left, front
        fillSphere(targetDataset_, tgt::vec3(border+thickness+(int)(is*0.3f), border+thickness+is/6, border+thickness+(int)(is*0.8f)), is/6, sphere);
//         fillBox(targetDataset_, tgt::ivec3(), tgt::ivec3(), );

//         void fillSphere(VolumeDataset* vds, tgt::vec3 center, float radius, uint8_t value);
//         void fillBox(VolumeDataset* vds, tgt::ivec3 start, tgt::ivec3 end, uint8_t value);
    */
    }
     else if (operation == "synth") {
        tgt::vec3 center;
        center = dimensions / 2;

        float radius = (float)dimensions.x / 4.0f;

        LINFO("Generating synth dataset with dimensions: " << dimensions);

        fillBox(target, tgt::ivec3(0,0,0), dimensions, 0);

        fillSphere(target, center, radius, 96);

        /*
        for (int voxel_z=5; voxel_z<dimensions.z-5; voxel_z++) {
            for (int voxel_y=5; voxel_y<dimensions.y-5; voxel_y++) {
                for (int voxel_x=5; voxel_x<(5+(dimensions.x/10)); voxel_x++) {
                   targetDataset_->setVoxel(voxel_x, voxel_y, voxel_z, (uint8_t)192);
                }
            }
        }
        */
        for (int voxel_x=5; voxel_x<dimensions.x-5; voxel_x++) {
            for (int voxel_y=5; voxel_y<dimensions.y-5; voxel_y++) {
                for (int voxel_z=5; voxel_z<(5+(dimensions.z/10)); voxel_z++) {
                   target->voxel(voxel_x, voxel_y, voxel_z) = (uint8_t)128;
                }
            }
        }
    }
    else if (operation == "cloud") {
        dimensions.x *= 2;
        tgt::vec3 center;
        center = dimensions / 2;

        LINFO("Generating cloud dataset with dimensions: " << dimensions);

        fillBox(target, tgt::ivec3(0,0,0), dimensions, 0);

        // generate cloud pattern
        fillEllipsoid(target, tgt::vec3(dimensions.x*0.5, dimensions.y*0.5, dimensions.z*0.5), tgt::vec3(dimensions.x*0.3, dimensions.y*0.35, dimensions.z*0.3), 128);
        fillEllipsoid(target, tgt::vec3(dimensions.x*0.2, dimensions.y*0.3, dimensions.z*0.45), tgt::vec3(dimensions.x*0.2, dimensions.y*0.2, dimensions.z*0.15), 128);
        fillEllipsoid(target, tgt::vec3(dimensions.x*0.8, dimensions.y*0.4, dimensions.z*0.6), tgt::vec3(dimensions.x*0.15, dimensions.y*0.15, dimensions.z*0.15), 128);
        fillEllipsoid(target, tgt::vec3(dimensions.x*0.6, dimensions.y*0.3, dimensions.z*0.8), tgt::vec3(dimensions.x*0.1, dimensions.y*0.15, dimensions.z*0.12), 128);
        fillEllipsoid(target, tgt::vec3(dimensions.x*0.3, dimensions.y*0.6, dimensions.z*0.55), tgt::vec3(dimensions.x*0.15, dimensions.y*0.15, dimensions.z*0.2), 128);

        // perform noise perturbation
        tgt::vec3 lowFrequency = tgt::vec3(10, 10, 10);
        tgt::vec3 highAmplitude = tgt::vec3((float)dimensions.x/10.0, (float)dimensions.y/10.0, (float)dimensions.z/10.0);
        LINFO("applying low frequency high amplitude perturbation...");
        applyPerturbation(target, dimensions, lowFrequency, highAmplitude);

        tgt::vec3 highFrequency = tgt::vec3(1, 1, 1);
        tgt::vec3 lowAmplitude = tgt::vec3((float)dimensions.x/20.0, (float)dimensions.y/20.0, (float)dimensions.z/20.0);
        LINFO("applying high frequency low amplitude perturbation...");
        applyPerturbation(target, dimensions, highFrequency, lowAmplitude);
    }
    else if (operation == "aotestbox"){
        LINFO("Generating aotest");
        
        fillBox(target, tgt::ivec3(0,0,0), dimensions, 0);

        for (int i=0 ; i<dimensions.z; ++i){
            LINFO("Generating aotest slice: "<< i);
            fillBox(target, tgt::ivec3(0,0,i), tgt::ivec3(dimensions.x,dimensions.y,i+1), i);
        }
    }
    else if (operation == "shadowtestvol"){
        LINFO("Generating shadowTestVolume");
        delete target;
        target = new VolumeUInt8(tgt::ivec3(64,64,64));

        fillBox(target, tgt::ivec3(0,0,0), tgt::ivec3(64,64,64), 0);

        fillBox(target, tgt::ivec3(0,0,0), tgt::ivec3(64,64,1), 25);
        fillBox(target, tgt::ivec3(0,0,1), tgt::ivec3(64,64,2), 25);

        fillBox(target, tgt::ivec3(25,25,45), tgt::ivec3(39,39,46), 25);
        fillBox(target, tgt::ivec3(25,25,46), tgt::ivec3(39,39,47), 25);
    }
    else
        throw SyntaxException("Unknown type!");
    
    if(target) {
        VolumeSerializerPopulator volLoadPop;
        VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();
        serializer->save(parameters.back(), target);
//         delete serializer; //deleted by VolumeSerializerPopulator
        delete target;
    }
    return true;
}

void CommandCreate::fillSphere(VolumeUInt8* vds, tgt::vec3 center, float radius, uint8_t value) {
    for (int voxel_z=0; voxel_z<vds->getDimensions().z; voxel_z++) {
        for (int voxel_y=0; voxel_y<vds->getDimensions().y; voxel_y++) {
            for (int voxel_x=0; voxel_x<vds->getDimensions().x; voxel_x++) {
            tgt::vec3 diff = center - tgt::vec3((float)voxel_x, (float)voxel_y, (float)voxel_z);
            if(length(diff) <= (float)radius)
                vds->voxel(voxel_x, voxel_y, voxel_z) = value;
            }
        }
    }
}

void CommandCreate::fillEllipsoid(VolumeUInt8* vds, tgt::vec3 center, tgt::vec3 radius, uint8_t value) {
    // ugly code - don't do this at home
    for (int voxel_z=0; voxel_z<vds->getDimensions().z; voxel_z++) {
        for (int voxel_y=0; voxel_y<vds->getDimensions().y; voxel_y++) {
            for (int voxel_x=0; voxel_x<vds->getDimensions().x; voxel_x++) {
                if ((((voxel_x-center.x)*(voxel_x-center.x))/((radius.x*radius.x))) +
                    (((voxel_y-center.y)*(voxel_y-center.y))/((radius.y*radius.y))) +
                    (((voxel_z-center.z)*(voxel_z-center.z))/((radius.z*radius.z))) <= 1) {
                    vds->voxel(voxel_x, voxel_y, voxel_z) = value;
                }
            }
        }
    }
}

void CommandCreate::fillBox(VolumeUInt8* vds, tgt::ivec3 start, tgt::ivec3 end, uint8_t value) {
    tgt::ivec3 i;
    for(i.x = start.x; i.x < end.x; i.x++) {
        for(i.y = start.y; i.y < end.y; i.y++) {
            for(i.z = start.z; i.z < end.z; i.z++) {
                vds->voxel(i.x, i.y, i.z) = value;
            }
        }
    }
}

void CommandCreate::fillOrientedBox(VolumeUInt8* vds, tgt::vec3 center, tgt::vec3 dir, float lengthA, float lengthB, float yStart, float yEnd, uint8_t value) {
    dir = normalize(dir);
    center.y = 0.0f;
    tgt::vec3 dir2 = cross(dir, tgt::vec3(0.0f, 1.0f, 0.0f));
    for (int voxel_z=0; voxel_z<vds->getDimensions().z; voxel_z++) {
       for (int voxel_x=0; voxel_x<vds->getDimensions().x; voxel_x++) {
            tgt::vec3 diff = tgt::vec3((float)voxel_x, 0.0f, (float)voxel_z) - center;
            float l = dot(dir, diff);
            float l2 = dot(dir2, diff);
            if((fabs(l) < lengthA) && (fabs(l2) < lengthB))
                for (int voxel_y=yStart; voxel_y<yEnd; voxel_y++) {
                    vds->voxel(voxel_x, voxel_y, voxel_z) = value;
                }
        }
    }
}



void CommandCreate::applyPerturbation(Volume* /*vds*/, tgt::ivec3 /*dimensions*/, tgt::vec3 /*frequency*/, tgt::vec3 /*amplitude*/) {
//     srand( static_cast<unsigned int>(42) );
//     LWARNING("ATTENTION: set random seed to fixed value.");
//
//     tgt::vec3 perturbVec;
//     for (int voxel_z=5; voxel_z<dimensions.z-5; voxel_z+=frequency.z) {
//         for (int voxel_y=5; voxel_y<dimensions.y-5; voxel_y+=frequency.y) {
//             for (int voxel_x=5; voxel_x<dimensions.x-5; voxel_x+=frequency.x) {
//                 perturbVec = tgt::vec3((rand()/(float)RAND_MAX)*amplitude.x,
//                                        (rand()/(float)RAND_MAX)*amplitude.y,
//                                        (rand()/(float)RAND_MAX)*amplitude.z);
//                 for (int offset_z=0; offset_z<frequency.z; offset_z++) {
//                     for (int offset_y=0; offset_y<frequency.y; offset_y++) {
//                         for (int offset_x=0; offset_x<frequency.x; offset_x++) {
//                             if ((voxel_x+offset_x + perturbVec.x < dimensions.x) &&
//                                 (voxel_y+offset_y + perturbVec.y < dimensions.y) &&
//                                 (voxel_z+offset_z + perturbVec.z < dimensions.z)) {
//                                     uint8_t* oldIntensity = targetDataset_->voxel(voxel_x+offset_x, voxel_y+offset_y, voxel_z+offset_z);
//                                     targetDataset_->setVoxel(voxel_x+offset_x, voxel_y+offset_y, voxel_z+offset_z, targetDataset_->voxel(voxel_x+offset_x+perturbVec.x, voxel_y+offset_y+perturbVec.y, voxel_z+offset_z+perturbVec.z));
//                                     //targetDataset_->setVoxel(voxel_x+offset_x+perturbVec.x, voxel_y+offset_y+perturbVec.y, voxel_z+offset_z+perturbVec.z, oldIntensity);
//                                 }
//                         }
//                     }
//                 }
//             }
//         }
//     }
}

}   //namespace voreen
