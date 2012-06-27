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

#include "voreen/modules/base/processors/volume/volumedistancetransform.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/volumeoperator.h"

namespace voreen {

VolumeDistanceTransform::VolumeDistanceTransform()
    : VolumeProcessor()
    , enableProcessingProp_("enableProcessing", "enable: ")
    , forceUpdate_(true)
    , volumeOwner_(false)
    , inport_(Port::INPORT, "volumehandle.input")
    , outport_(Port::OUTPORT, "volumehandle.output", 0)

{
    addPort(inport_);
    addPort(outport_);
    enableProcessingProp_.onChange(CallMemberAction<VolumeDistanceTransform>(this, &VolumeDistanceTransform::forceUpdate));
    addProperty(enableProcessingProp_);
}

VolumeDistanceTransform::~VolumeDistanceTransform() {
}

std::string VolumeDistanceTransform::getCategory() const {
    return "Volume Processing";
}

std::string VolumeDistanceTransform::getClassName() const {
    return "VolumeDistanceTransform";
}

Processor::CodeState VolumeDistanceTransform::getCodeState() const {
    return CODE_STATE_EXPERIMENTAL;
}

Processor* VolumeDistanceTransform::create() const {
    return new VolumeDistanceTransform();
}

std::string VolumeDistanceTransform::getProcessorInfo() const {
    return "Performs a 3D distance transform of the input volume. ";
}

void VolumeDistanceTransform::process() {
    if (!enableProcessingProp_.get()) {
        outport_.setData(inport_.getData(), volumeOwner_);
        volumeOwner_ = false;
    }
    else if (forceUpdate_ || inport_.hasChanged()) {
        distanceTransform();
    }
}

// private methods
//

void VolumeDistanceTransform::forceUpdate() {
    forceUpdate_ = true;
}

void VolumeDistanceTransform::distanceTransform() {
    tgtAssert(inport_.hasData(), "Inport has not data");

    forceUpdate_ = false;

    if (volumeOwner_) {
        outport_.deleteVolume();
        volumeOwner_ = false;
    }

    if (inport_.getData()->getVolume()) {
        if (!dynamic_cast<VolumeUInt16*>(inport_.getData()->getVolume())) {
            LWARNING("Currently only 16 bit volumes supported.");
            return;
        }

        VolumeUInt16* v = dynamic_cast<VolumeUInt16*>(inport_.getData()->getVolume()->clone());
        uint16_t val = 65535;
        uint16_t dist = 0;

        for (int z=1; z<v->getDimensions().z-1 ; z++){
            for (int x=1; x<v->getDimensions().x-1 ; x++)
                for (int y=1 ;y<v->getDimensions().y-1 ; y++)
                    if (v->voxel(x,y,z) > 0) {

                        dist = 0;
                        val =  255;

                        if (v->voxel(x-1,y,z) + 1 < val + dist)         {   val = v->voxel(x-1,y,z);  dist = 3; }
                        if (v->voxel(x,y-1,z) + 3 < val + dist)         {   val = v->voxel(x,y-1,z);  dist = 3; }
                        if (v->voxel(x-1,y-1,z) + 4 < val + dist )      {   val = v->voxel(x-1,y-1,z); dist = 4; }
                        if (v->voxel(x+1,y-1,z) + 4 < val + dist )      {   val = v->voxel(x+1,y-1,z); dist = 4; }
                        if (v->voxel(x,y,z-1) + 3 < val + dist)         {   val = v->voxel(x,y,z-1);  dist = 3; }

                        if (v->voxel(x-1,y,z-1) + 4  < val + dist){ val = v->voxel(x-1,y,z-1); dist = 4; }
                        if (v->voxel(x,y-1,z-1) + 4  < val + dist){ val = v->voxel(x,y-1,z-1); dist = 4; }

                        if (v->voxel(x-1,y,z+1) + 4  < val + dist){ val = v->voxel(x-1,y,z+1); dist = 4; }
                        if (v->voxel(x,y-1,z+1) + 4  < val + dist){ val = v->voxel(x,y-1,z+1); dist = 4; }


                        if (v->voxel(x-1,y-1,z+1) + 5 < val + dist){ val = v->voxel(x-1,y-1,z+1); dist = 5; }
                        if (v->voxel(x-1,y-1,z-1) + 5 < val + dist){ val = v->voxel(x-1,y-1,z-1); dist = 5; }

                        if (v->voxel(x+1,y-1,z-1) + 5 < val + dist){ val = v->voxel(x+1,y-1,z-1); dist = 5; }
                        if (v->voxel(x+1,y-1,z+1) + 5 < val + dist){ val = v->voxel(x+1,y-1,z+1); dist = 5; }

                        v->voxel(x,y,z) = val + dist;
                    }
        }

        for (int z=v->getDimensions().z-2; z>2; z--){
            for (int x=v->getDimensions().x-2; x>2; x--)
                for (int y=v->getDimensions().y-2; y>2; y--)
                    if (v->voxel(x,y,z) > 0) {
                        dist = 0;
                        val = v->voxel(x,y,z);

                        if (v->voxel(x+1,y,z) + 3 < val + dist)          { val = v->voxel(x+1,y,z); dist = 3; }
                        if (v->voxel(x,y+1,z) + 3 < val + dist)          { val = v->voxel(x,y+1,z); dist = 3; }
                        if (v->voxel(x+1,y+1,z) + 4 < val + dist)        { val = v->voxel(x+1,y+1,z); dist = 4; }
                        if (v->voxel(x,y,z+1) + 3  < val + dist)         { val = v->voxel(x,y,z+1); dist = 3; }

                        if (v->voxel(x-1,y+1,z) + 4 < val + dist){ val = v->voxel(x-1,y+1,z); dist = 4; }
                        if (v->voxel(x-1,y+1,z+1) + 5 < val + dist){ val = v->voxel(x-1,y+1,z+1); dist = 5; }
                        if (v->voxel(x-1,y+1,z-1) + 5 < val + dist){ val = v->voxel(x-1,y+1,z-1); dist = 5; }

                        if (v->voxel(x+1,y,z+1) + 4 < val + dist){ val = v->voxel(x+1,y,z+1); dist = 4; }
                        if (v->voxel(x,y+1,z+1) + 4 < val + dist){ val = v->voxel(x,y+1,z+1); dist = 4; }
                        if (v->voxel(x+1,y+1,z+1) + 5 < val + dist){ val = v->voxel(x+1,y+1,z+1); dist = 5; }

                        if (v->voxel(x+1,y,z-1) + 4 < val + dist){ val = v->voxel(x+1,y,z-1); dist = 4; }
                        if (v->voxel(x,y+1,z-1) + 4 < val + dist){ val = v->voxel(x,y,z-1); dist = 4; }
                        if (v->voxel(x+1,y+1,z-1)+ 5 < val + dist){ val = v->voxel(x+1,y+1,z-1); dist = 5; }

                        if ((val + dist) <  v->voxel(x,y,z)) v->voxel(x,y,z) =  val + dist;

                    }

        }

        //border cleaning, it was not transformed by distance mapping, property of the algorithm
        for (int x =0; x< v->getDimensions().x;x++)
            for (int y =0; y< v->getDimensions().y;y++)
                for (int z =0; z< v->getDimensions().z;z++){
                    if ( (x < 3) || (x > v->getDimensions().x-3) || (y < 3) || (y > v->getDimensions().y-3)
                        || (z < 3) || (z > v->getDimensions().z-3)) v->voxel(x,y,z) = 0;

                }
        outport_.setData(new VolumeHandle(v));
        volumeOwner_ = true;
    }
    else {
        outport_.setData(0);
        volumeOwner_ = false;
    }
}

void VolumeDistanceTransform::deinitialize() throw (VoreenException) {
    if (volumeOwner_) {
        outport_.deleteVolume();
        volumeOwner_ = false;
    }

    VolumeProcessor::deinitialize();
}

}   // namespace
