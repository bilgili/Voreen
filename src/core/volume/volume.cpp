/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/volume/volume.h"

#include "voreen/core/volume/volumeatomic.h"

#include "tgt/plane.h"

using tgt::vec3;
using tgt::ivec3;

namespace voreen {

const std::string Volume::loggerCat_("Voreen.Volume");

Volume::Volume(const ivec3& dimensions,
               int bitsStored,
               const vec3& spacing)
  : dimensions_(dimensions),
    numVoxels_(hmul(dimensions)),
    bitsStored_(bitsStored),
    spacing_(spacing)
{
    cubeSize_ = vec3(dimensions) * spacing;
    cubeSize_ = cubeSize_ * 2.f / max(cubeSize_);
    urb_ = cubeSize_ / 2.f;
    llf_ = -urb_;

    // swap z of llf and urb since we use a right handed coordinate system
    std::swap(llf_.z, urb_.z);

    // calculate cube vertices
    tgt::plane::createCubeVertices(llf_, urb_, cubeVertices_);
}

void Volume::convert(const Volume* v, bool /*smartConvert*/ /*= false*/) {
    //TODO: implement smartConvert (see description in .h) (what does this actually mean??? joerg)
    //TODO: this should better placed somewhere else and not introduce dependencies to VolumeAtomic here

    //FIXME: do a real error check with exceptions here
    tgtAssert(dimensions_ == v->getDimensions(), "dimensions must match here");
    tgtAssert(getNumChannels() == v->getNumChannels(), "number of channels must match here");

    /*
        check what we have
    */
    VolumeUInt8*  t_ui8  = dynamic_cast<VolumeUInt8*>(this);
    VolumeUInt16* t_ui16 = dynamic_cast<VolumeUInt16*>(this);

    const VolumeUInt8*  v_ui8  = dynamic_cast<const VolumeUInt8*>(v);
    const VolumeUInt16* v_ui16 = dynamic_cast<const VolumeUInt16*>(v);

    if (t_ui8 && v_ui16) {
        LINFO("using accelerated conversion from VolumeUInt16 -> VolumeUInt8 without smart convert");

        // because the number of shifting bits varies by the number of bits used it must be calculated
        int shift = v_ui16->getBitsStored() - t_ui8->getBitsStored();

        for (size_t i = 0; i < t_ui8->getNumVoxels(); ++i)
            t_ui8->voxel(i) = v_ui16->voxel(i) >> shift;
    }
    else if (t_ui16 && v_ui8) {
        LINFO("using accelerated conversion from VolumeUInt8 -> VolumeUInt16 without smart convert");

        // because the number of shifting bits varies by the number of bits used it must be calculated
        int shift = t_ui16->getBitsStored() - v_ui8->getBitsStored();

        for (size_t i = 0; i < t_ui16->getNumVoxels(); ++i)
            t_ui16->voxel(i) = uint16_t( v_ui8->voxel(i) ) << shift;
    }
    else {
        LINFO("using fallback with setVoxelFloat and getVoxelFloat and without smart convert");

        VRN_FOR_EACH_VOXEL(i, tgt::ivec3(0), dimensions_)
            setVoxelFloat(v->getVoxelFloat(i), i);
    }
}

/*
 * getters and setters
 */

ivec3 Volume::getDimensions() const {
    return dimensions_;
}

size_t Volume::getNumVoxels() const {
    return numVoxels_;
}

int Volume::getBitsStored() const {
    return bitsStored_;
}

vec3 Volume::getSpacing() const {
    return spacing_;
}

vec3 Volume::getLLF() const {
    return llf_;
}

vec3 Volume::getURB() const {
    return urb_;
}

const vec3* Volume::getCubeVertices() const {
    return cubeVertices_;
}

vec3 Volume::getCubeSize() const {
    return cubeSize_;
}

const VolumeMetaData& Volume::meta() const {
    return meta_;
}

VolumeMetaData& Volume::meta() {
    return meta_;
}

void Volume::setSpacing(const tgt::vec3 spacing) {
    spacing_ = spacing;
}

void Volume::setBitsStored(int bitsStored) {
    bitsStored_ = bitsStored;
}

/*
 * abstract access of the voxels
 */

float Volume::getVoxelFloat(size_t x, size_t y, size_t z, size_t channel /*= 0*/) const {
    return getVoxelFloat(ivec3(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)), channel);
}

float Volume::getVoxelFloatLinear(const vec3& pos, size_t channel /*= 0*/) const {
    vec3 p = pos - floor(pos); // get decimal part
    ivec3 llb = ivec3( pos );
    ivec3 urf = ivec3( ceil(pos) );
    urf = max(min(urf, dimensions_ - 1), ivec3(1)); // clamp so the lookups do not exceed the dimensions
    llb = max(min(llb, dimensions_ - 1), ivec3(1)); // dito

    /*
        interpolate linearly
    */
    return  getVoxelFloat(llb.x, llb.y, llb.z, channel) * (1.f-p.x)*(1.f-p.y)*(1.f-p.z) // llB
          + getVoxelFloat(urf.x, llb.y, llb.z, channel) * (    p.x)*(1.f-p.y)*(1.f-p.z) // lrB
          + getVoxelFloat(urf.x, urf.y, llb.z, channel) * (    p.x)*(    p.y)*(1.f-p.z) // urB
          + getVoxelFloat(llb.x, urf.y, llb.z, channel) * (1.f-p.x)*(    p.y)*(1.f-p.z) // ulB
          + getVoxelFloat(llb.x, llb.y, urf.z, channel) * (1.f-p.x)*(1.f-p.y)*(    p.z) // llF
          + getVoxelFloat(urf.x, llb.y, urf.z, channel) * (    p.x)*(1.f-p.y)*(    p.z) // lrF
          + getVoxelFloat(urf.x, urf.y, urf.z, channel) * (    p.x)*(    p.y)*(    p.z) // urF
          + getVoxelFloat(llb.x, urf.y, urf.z, channel) * (1.f-p.x)*(    p.y)*(    p.z);// ulF
}

} // namespace voreen
