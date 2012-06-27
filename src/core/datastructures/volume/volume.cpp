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

#include "voreen/core/datastructures/volume/volume.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"

#include "tgt/plane.h"

using tgt::vec3;
using tgt::ivec3;

namespace voreen {

const std::string Volume::loggerCat_("voreen.Volume");

Volume::Volume(const ivec3& dimensions, int bitsStored, const vec3& spacing, const tgt::mat4& transformation)
    : dimensions_(dimensions)
    , bitsStored_(bitsStored)
    , spacing_(spacing)
    , transformationMatrix_(transformation)
{
    calculateProperties();
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

void Volume::setBitsStored(int bits) {
    bitsStored_ = bits;
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

void Volume::setTransformation(const tgt::mat4& transformationMatrix) {
    transformationMatrix_ = transformationMatrix;
}

const tgt::mat4& Volume::getTransformation() const {
    return transformationMatrix_;
}

MeshGeometry Volume::getBoundingBox(bool applyTransformation) const {
    MeshGeometry boundingBox = MeshGeometry::createCube(getLLF(), getURB(), tgt::vec3(0.f), tgt::vec3(1.f));
    if (applyTransformation)
        boundingBox.transform(getTransformation());
    return boundingBox;
}

tgt::mat4 Volume::getVoxelToWorldMatrix(bool applyTransformation) const {
    tgt::mat4 result;
    // 1. transform voxel to texture coordinates
    result = tgt::mat4::createScale(1.f / tgt::vec3(getDimensions()-1));

    // 2. apply texture to world transformation
    tgt::mat4 textureToWorld = getTextureToWorldMatrix(applyTransformation);
    return textureToWorld * result;
}

tgt::mat4 Volume::getWorldToVoxelMatrix(bool applyTransformation) const {
    tgt::mat4 result = tgt::mat4::identity;
    getVoxelToWorldMatrix(applyTransformation).invert(result);
    return result;
}

tgt::mat4 Volume::getTextureToWorldMatrix(bool applyTransformation) const {
    tgt::mat4 result;
    // 1. translate cube's center to the origin
    result = tgt::mat4::createTranslation(tgt::vec3(-0.5f));
    // 2. scale by the volume's cube size
    result = tgt::mat4::createScale(getCubeSize()) * result;
    if(applyTransformation) {
        // 3. finally apply additional transformation matrix
        result = getTransformation() * result;
    }

    return result;
}

tgt::mat4 Volume::getWorldToTextureMatrix(bool applyTransformation) const {
    tgt::mat4 result = tgt::mat4::identity;
    getTextureToWorldMatrix(applyTransformation).invert(result);
    return result;
}

const VolumeMetaData& Volume::meta() const {
    return meta_;
}

VolumeMetaData& Volume::meta() {
    return meta_;
}

void Volume::setSpacing(const tgt::vec3 spacing) {
    spacing_ = spacing;
    calculateProperties();
}

template<class T>
inline typename T::VoxelType* Volume::getData(T* v) {
    return (typename T::VoxelType*) v->getData();
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
    urf = max(min(urf, dimensions_ - 1), ivec3(0)); // clamp so the lookups do not exceed the dimensions
    llb = max(min(llb, dimensions_ - 1), ivec3(0)); // dito

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

void Volume::calculateProperties() {
    //numVoxels_ = hmul(dimensions_); << does not work for very large volumes!
    numVoxels_ = (size_t)dimensions_.x * (size_t)dimensions_.y * (size_t)dimensions_.z;

    cubeSize_ = vec3(dimensions_) * spacing_;
    cubeSize_ = cubeSize_ * 2.f / max(cubeSize_);
    urb_ = cubeSize_ / 2.f;
    llf_ = -urb_;

    // calculate cube vertices
    tgt::plane::createCubeVertices(llf_, urb_, cubeVertices_);
}

} // namespace voreen
