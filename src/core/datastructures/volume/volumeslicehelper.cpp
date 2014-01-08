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

#include "voreen/core/datastructures/volume/volumeslicehelper.h"
#include "voreen/core/datastructures/geometry/trianglemeshgeometry.h"
#include "voreen/core/datastructures/roi/roibase.h"
#include "tgt/logmanager.h"

#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volumedisk.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/octree/volumeoctreebase.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorresample.h"
#include "voreen/core/utils/hashing.h"

#include <boost/thread/locks.hpp>

namespace voreen {

using tgt::vec2;
using tgt::ivec2;
using tgt::vec3;
using tgt::mat4;


//-------------------------------------------------------------------------------------------------

VolumeSliceGL::VolumeSliceGL(const tgt::svec2& sliceDim, SliceAlignment alignment, const std::string& format, const std::string& baseType,
    tgt::vec3 origin, tgt::vec3 xVec, tgt::vec3 yVec, RealWorldMapping rwm,
    void* data, GLint textureFormat, GLint internalFormat, GLenum textureDataType)
    : dimensions_(sliceDim)
    , alignment_(alignment)
    , format_(format)
    , baseType_(baseType)
    , origin_(origin)
    , xVec_(xVec)
    , yVec_(yVec)
    , rwm_(rwm)
    , data_(data)
    , textureFormat_(textureFormat)
    , internalFormat_(internalFormat)
    , textureDataType_(textureDataType)
    , tex_(0)
{
    tgtAssert(data, "no data passed");
}

VolumeSliceGL::VolumeSliceGL(const tgt::svec2& sliceDim, SliceAlignment alignment, const std::string& format, const std::string& baseType,
    tgt::vec3 origin, tgt::vec3 xVec, tgt::vec3 yVec, RealWorldMapping rwm,
    tgt::Texture* tex)
    : dimensions_(sliceDim)
    , alignment_(alignment)
    , format_(format)
    , baseType_(baseType)
    , origin_(origin)
    , xVec_(xVec)
    , yVec_(yVec)
    , rwm_(rwm)
    , data_(0)
    , tex_(tex)
    , textureFormat_(0)
    , internalFormat_(0)
    , textureDataType_(0)
{
    tgtAssert(tex, "no texture passed");
}

VolumeSliceGL::~VolumeSliceGL() {
    if (tex_) {
        tex_->setPixelData(0);
        delete tex_;
        tex_ = 0;
    }

    delete[] data_;
    data_ = 0;
}

std::string VolumeSliceGL::getFormat() const {
    return format_;
}

std::string VolumeSliceGL::getBaseType() const {
    return baseType_;
}

SliceAlignment VolumeSliceGL::getAlignment() const {
    return alignment_;
}

tgt::mat4 VolumeSliceGL::getTextureToWorldMatrix() const {
    vec3 zVec = normalize(cross(xVec_, yVec_));

    mat4 m(xVec_.x, yVec_.x, zVec.x, origin_.x,
        xVec_.y, yVec_.y, zVec.y, origin_.y,
        xVec_.z, yVec_.z, zVec.z, origin_.z,
        0.0f,    0.0f,    0.0f,   1.0f);
    return m;
}

tgt::mat4 VolumeSliceGL::getWorldToTextureMatrix() const {
    mat4 m = getTextureToWorldMatrix();
    mat4 inv;
    m.invert(inv);
    return inv;
}

voreen::RealWorldMapping VolumeSliceGL::getRealWorldMapping() const {
    return rwm_;
}

void VolumeSliceGL::bind() const {
    if (!tex_)
        createTexture();

    tgtAssert(tex_, "no texture");
    tex_->bind();
}

const tgt::Texture* VolumeSliceGL::getTexture() const {
    if (!tex_)
        createTexture();

    return tex_;
}

void VolumeSliceGL::createTexture() const {
    tgtAssert(data_, "no pixel data");
    tgtAssert(!tex_, "texture already created");

    tex_ = new tgt::Texture(reinterpret_cast<GLubyte*>(data_), tgt::ivec3((int)dimensions_.x, (int)dimensions_.y, 1),
        textureFormat_, internalFormat_, textureDataType_);
    LGL_ERROR;

    // map signed integer types from [-1.0:1.0] to [0.0:1.0] in order to avoid clamping of negative values
    // (see also setUniform() above)
    bool isSignedInteger = (textureDataType_ == GL_BYTE || textureDataType_ == GL_SHORT || textureDataType_ == GL_INT);
    if (isSignedInteger) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPixelTransferf(GL_RED_SCALE,   0.5f);
        glPixelTransferf(GL_GREEN_SCALE, 0.5f);
        glPixelTransferf(GL_BLUE_SCALE,  0.5f);
        glPixelTransferf(GL_ALPHA_SCALE, 0.5f);

        glPixelTransferf(GL_RED_BIAS,    0.5f);
        glPixelTransferf(GL_GREEN_BIAS,  0.5f);
        glPixelTransferf(GL_BLUE_BIAS,   0.5f);
        glPixelTransferf(GL_ALPHA_BIAS,  0.5f);
        LGL_ERROR;
    }

    // upload texture data
    tex_->uploadTexture();
    LGL_ERROR;

    if (isSignedInteger)
        glPopAttrib();
    LGL_ERROR;
}

//-------------------------------------------------------------------------------------------------

const std::string VolumeSliceHelper::loggerCat_("voreen.VolumeSliceHelper");

VolumeSliceGL* VolumeSliceHelper::getVolumeSlice(const VolumeBase* vh, SliceAlignment alignment, size_t sliceIndex,
    size_t levelOfDetail /*= 0*/, clock_t timeLimit /*= 0*/, bool* complete /*= 0*/)
{
    tgtAssert(vh, "null pointer passed");
    tgtAssert(alignment != UNALIGNED_PLANE, "invalid alignment");
    tgtAssert(sliceIndex < vh->getDimensions()[alignment], "invalid slice index");

    vec3 urb = vh->getURB();
    vec3 llf = vh->getLLF();
    vec3 sp = vh->getSpacing();
    tgt::Bounds b(llf, urb);
    tgt::svec3 dims = vh->getDimensions();

    vec3 bb_urb = b.getURB();
    vec3 bb_llf = b.getLLF();

    vec3 origin(0.0f);
    vec3 xVec(0.0f);
    vec3 yVec(0.0f);

    tgt::svec2 sliceDim;

    void* dataBuffer = 0;
    GLint textureFormat, internalFormat;
    GLenum textureDataType;

    switch(alignment) {
        case YZ_PLANE: {
            // representation preference:
            // 1) use RAM volume, if present
            // 2) else: use octree, if present
            // 3) else: try to create RAM representation
            // note: disk representation is not usable for this alignment
            sliceDim = vh->getDimensions().yz();
            tgt::svec3 sliceDim3D(1, sliceDim);     //< expected dimension of the extracted slice
            const VolumeRAM* ramVolume3D = 0;       //< RAM representation
            const VolumeRAM* ramVolumeSlice = 0;    //< slice retrieved from octree (has to be deleted afterwards)
            if (vh->hasRepresentation<VolumeRAM>()) {
                ramVolume3D = vh->getRepresentation<VolumeRAM>();
            }
            else if (vh->hasRepresentation<VolumeOctreeBase>()) {
                try {
                    const VolumeOctreeBase* octree = vh->getRepresentation<VolumeOctreeBase>();
                    tgtAssert(octree, "no octree");
                    ramVolumeSlice = octree->createSlice(YZ_PLANE, sliceIndex, levelOfDetail, timeLimit, complete);
                    tgtAssert(ramVolumeSlice, "null pointer returned (exception expected)");

                    // resample slice of higher mipmap level to original volume resolution
                    if (levelOfDetail > 0) {
                        Volume* sliceHandle = new Volume(const_cast<VolumeRAM*>(ramVolumeSlice), vh);
                        Volume* sliceResampled = VolumeOperatorResample::APPLY_OP(sliceHandle, sliceDim3D, VolumeRAM::NEAREST);
                        tgtAssert(sliceResampled && sliceResampled->getWritableRepresentation<VolumeRAM>(), "no resampled slice");
                        ramVolumeSlice = sliceResampled->getWritableRepresentation<VolumeRAM>();
                        sliceResampled->releaseAllRepresentations();
                        delete sliceResampled;
                        delete sliceHandle;
                    }
                }
                catch (tgt::Exception& e) {
                    LERROR("Failed to create YZ slice from octree: " << e.what());
                    return 0;
                }
            }
            else {
                ramVolume3D = vh->getRepresentation<VolumeRAM>();
                if (!ramVolume3D) {
                    LERROR("Failed to create YZ slice: neither RAM nor octree representation available");
                    return 0;
                }
            }

            float x = static_cast<float>(sliceIndex);
            float xcoord = llf.x + (x+0.5f) * sp.x; // We want our slice to be in the center of voxels

            origin = vec3(xcoord, bb_llf.y, bb_llf.z);
            xVec = vec3(xcoord, bb_urb.y, bb_llf.z);
            yVec = vec3(xcoord, bb_llf.y, bb_urb.z);

            tgtAssert(ramVolume3D || ramVolumeSlice, "no volume");
            try {
                if (ramVolume3D)
                    VolumeSliceHelper::extractAlignedSlicePixelData(ramVolume3D, alignment, sliceIndex, dataBuffer, textureFormat, internalFormat, textureDataType);
                else
                    VolumeSliceHelper::extractAlignedSlicePixelData(ramVolumeSlice, alignment, 0, dataBuffer, textureFormat, internalFormat, textureDataType);
            }
            catch (VoreenException& e) {
                LERROR(e.what());
                delete ramVolumeSlice;
                return 0;
            }

            delete ramVolumeSlice;
            ramVolumeSlice = 0;
        }
        break;

        case XZ_PLANE: {
            // representation preference:
            // 1) use RAM volume, if present
            // 2) else: use octree, if present
            // 3) else: try to create RAM representation
            // note: disk representation is not usable for this alignment
            sliceDim = tgt::svec2(vh->getDimensions().x, vh->getDimensions().z);
            tgt::svec3 sliceDim3D(sliceDim.x, 1, sliceDim.y);   //< expected slice dimension
            const VolumeRAM* ramVolume3D = 0;                   //< RAM representation
            const VolumeRAM* ramVolumeSlice = 0;                //< slice retrieved from octree (has to be deleted afterwards)
            if (vh->hasRepresentation<VolumeRAM>()) {
                ramVolume3D = vh->getRepresentation<VolumeRAM>();
            }
            else if (vh->hasRepresentation<VolumeOctreeBase>()) {
                try {
                    const VolumeOctreeBase* octree = vh->getRepresentation<VolumeOctreeBase>();
                    tgtAssert(octree, "no octree");
                    ramVolumeSlice = octree->createSlice(XZ_PLANE, sliceIndex, levelOfDetail, timeLimit, complete);
                    tgtAssert(ramVolumeSlice, "null pointer returned (exception expected)");

                    // resample slice of higher mipmap level to original volume resolution
                    if (levelOfDetail > 0) {
                        Volume* sliceHandle = new Volume(const_cast<VolumeRAM*>(ramVolumeSlice), vh);
                        Volume* sliceResampled = VolumeOperatorResample::APPLY_OP(sliceHandle, sliceDim3D, VolumeRAM::NEAREST);
                        tgtAssert(sliceResampled && sliceResampled->getWritableRepresentation<VolumeRAM>(), "no resampled slice");
                        ramVolumeSlice = sliceResampled->getWritableRepresentation<VolumeRAM>();
                        sliceResampled->releaseAllRepresentations();
                        delete sliceResampled;
                        delete sliceHandle;
                    }
                }
                catch (tgt::Exception& e) {
                    LERROR("Failed to create XZ slice from octree: " << e.what());
                    return 0;
                }
            }
            else {
                ramVolume3D = vh->getRepresentation<VolumeRAM>();
                if (!ramVolume3D) {
                    LERROR("Failed to create XZ slice: neither RAM nor octree representation available");
                    return 0;
                }
            }

            float y = static_cast<float>(sliceIndex);
            float ycoord = llf.y + (y+0.5f) * sp.y; // We want our slice to be in the center of voxels

            origin = vec3(bb_llf.x, ycoord, bb_llf.z);
            xVec = vec3(bb_urb.x, ycoord, bb_llf.z);
            yVec = vec3(bb_llf.x, ycoord, bb_urb.z);

            tgtAssert(ramVolume3D || ramVolumeSlice, "no volume");
            try {
                if (ramVolume3D)
                    VolumeSliceHelper::extractAlignedSlicePixelData(ramVolume3D, alignment, sliceIndex, dataBuffer, textureFormat, internalFormat, textureDataType);
                else
                    VolumeSliceHelper::extractAlignedSlicePixelData(ramVolumeSlice, alignment, 0, dataBuffer, textureFormat, internalFormat, textureDataType);
            }
            catch (VoreenException& e) {
                LERROR(e.what());
                delete ramVolumeSlice;
                return 0;
            }

            delete ramVolumeSlice;
            ramVolumeSlice = 0;
        }
        break;

        case XY_PLANE: {
            // representation preference:
            // 1) use RAM volume, if present
            // 2) else: use octree, if present
            // 3) else: use disk volume, if present
            // 4) else: try to create RAM representation
            sliceDim = vh->getDimensions().xy();
            tgt::svec3 sliceDim3D(sliceDim, 1);    //< expected dimension of the extracted slice
            const VolumeRAM* ramVolume3D = 0;      //< RAM representation
            const VolumeRAM* ramVolumeSlice = 0;   //< slice retrieved from octree or disk volume (has to be deleted)
            if (vh->hasRepresentation<VolumeRAM>()) {
                ramVolume3D = vh->getRepresentation<VolumeRAM>();
            }
            else if (vh->hasRepresentation<VolumeOctreeBase>()) {
                try {
                    const VolumeOctreeBase* octree = vh->getRepresentation<VolumeOctreeBase>();
                    tgtAssert(octree, "no octree");
                    ramVolumeSlice = octree->createSlice(XY_PLANE, sliceIndex, levelOfDetail, timeLimit, complete);
                    tgtAssert(ramVolumeSlice, "null pointer returned (exception expected)");

                    // resample slice of higher mipmap level to original volume resolution
                    if (levelOfDetail > 0) {
                        Volume* sliceHandle = new Volume(const_cast<VolumeRAM*>(ramVolumeSlice), vh);
                        Volume* sliceResampled = VolumeOperatorResample::APPLY_OP(sliceHandle, sliceDim3D, VolumeRAM::NEAREST);
                        tgtAssert(sliceResampled && sliceResampled->getWritableRepresentation<VolumeRAM>(), "no resampled slice");
                        ramVolumeSlice = sliceResampled->getWritableRepresentation<VolumeRAM>();
                        sliceResampled->releaseAllRepresentations();
                        delete sliceResampled;
                        delete sliceHandle;
                    }
                }
                catch (tgt::Exception& e) {
                    LERROR("Failed to create XY slice from octeee: " << e.what());
                    return 0;
                }
            }
            else if (vh->hasRepresentation<VolumeDisk>()) {
                try {
                    ramVolumeSlice = vh->getRepresentation<VolumeDisk>()->loadSlices(sliceIndex, sliceIndex);
                    tgtAssert(ramVolumeSlice, "null pointer returned (exception expected)");
                }
                catch (tgt::Exception& e) {
                    LERROR("Failed to load XY slice from disk volume: " << e.what());
                    return 0;
                }
            }
            else {
                ramVolume3D = vh->getRepresentation<VolumeRAM>();
                if (!ramVolume3D) {
                    LERROR("Failed to create XY slice: neither RAM nor octree, nor disk representation available");
                    return 0;
                }
            }

            float z = static_cast<float>(sliceIndex);
            float zcoord = llf.z + (z+0.5f) * sp.z; // We want our slice to be in the center of voxels

            origin = vec3(bb_llf.x, bb_llf.y, zcoord);
            xVec = vec3(bb_urb.x, bb_llf.y, zcoord);
            yVec = vec3(bb_llf.x, bb_urb.y, zcoord);

            tgtAssert(ramVolume3D || ramVolumeSlice, "no volume");
            try {
                if (ramVolume3D)
                    VolumeSliceHelper::extractAlignedSlicePixelData(ramVolume3D, alignment, sliceIndex, dataBuffer, textureFormat, internalFormat, textureDataType);
                else
                    VolumeSliceHelper::extractAlignedSlicePixelData(ramVolumeSlice, alignment, 0, dataBuffer, textureFormat, internalFormat, textureDataType);
            }
            catch (VoreenException& e) {
                LERROR(e.what());
                delete ramVolumeSlice;
                return 0;
            }

            delete ramVolumeSlice;
            ramVolumeSlice = 0;
        }
        break;

        default:
            tgtAssert(false, "unknown/unsupported slice alignment");
            LERROR("Unknown/unsupported slice alignment: " << alignment);
            return 0;
    }

    tgtAssert(dataBuffer, "data buffer is empty");

    origin = vh->getPhysicalToWorldMatrix() * origin;
    xVec = vh->getPhysicalToWorldMatrix() * xVec;
    yVec = vh->getPhysicalToWorldMatrix() * yVec;
    xVec = xVec - origin;
    yVec = yVec - origin;

    return new VolumeSliceGL(sliceDim, alignment, vh->getFormat(), vh->getBaseType(), origin, xVec, yVec,
        vh->getRealWorldMapping(), dataBuffer, textureFormat, internalFormat, textureDataType);
}

//-------------------------------------------------------------------------------------------------

VolumeSliceGL* VolumeSliceHelper::getVolumeSlice(const VolumeBase* vh, tgt::plane pl, float samplingRate) {
    vec3 urb = vh->getURB();
    vec3 llf = vh->getLLF();
    vec3 center = (urb + llf) * 0.5f;

    vec3 xMax = center;
    xMax.x = urb.x;
    vec3 yMax = center;
    yMax.y = urb.y;
    vec3 zMax = center;
    zMax.z = urb.z;

    // check whether the plane normal matches one of the main directions of the volume:
    tgt::plane plVoxel = pl.transform(vh->getWorldToVoxelMatrix());
    if(fabs(fabs(dot(vec3(1.0f, 0.0f, 0.0f), plVoxel.n)) - 1.0f) < 0.01f) {
        float sliceNumber = plVoxel.d * plVoxel.n.x;

        float integral = tgt::round(sliceNumber);
        if(fabs(sliceNumber - integral) < 0.1f)
            return VolumeSliceHelper::getVolumeSlice(vh, YZ_PLANE, static_cast<int>(sliceNumber));
        //else TODO
    }
    else if(fabs(fabs(dot(vec3(0.0f, 1.0f, 0.0f), plVoxel.n)) - 1.0f) < 0.01f) {
        float sliceNumber = plVoxel.d * plVoxel.n.y;

        float integral = tgt::round(sliceNumber);
        if(fabs(sliceNumber - integral) < 0.1f)
            return VolumeSliceHelper::getVolumeSlice(vh, XZ_PLANE, static_cast<int>(sliceNumber));
        //else TODO
    }
    else if(fabs(fabs(dot(vec3(0.0f, 0.0f, 1.0f), plVoxel.n)) - 1.0f) < 0.01f) {
        float sliceNumber = plVoxel.d * plVoxel.n.z;

        float integral = tgt::round(sliceNumber);
        if(fabs(sliceNumber - integral) < 0.1f)
            return VolumeSliceHelper::getVolumeSlice(vh, XY_PLANE, static_cast<int>(sliceNumber));
        //else TODO
    }

    const VolumeRAM* vol = vh->getRepresentation<VolumeRAM>();
    if(!vol)
        return 0;

    // transform to world coordinates:
    mat4 pToW = vh->getPhysicalToWorldMatrix();
    center = pToW * center;
    xMax = pToW * xMax;
    yMax = pToW * yMax;
    zMax = pToW * zMax;

    // project to plane:
    float d = pl.distance(center);
    center = center - (pl.n * d);
    d = pl.distance(xMax);
    xMax = xMax - (pl.n * d);
    d = pl.distance(yMax);
    yMax = yMax - (pl.n * d);
    d = pl.distance(zMax);
    zMax = zMax - (pl.n * d);

    // find max axis in plane:
    vec3 maxVec = xMax - center;
    if(distance(yMax, center) > length(maxVec))
        maxVec = yMax - center;
    if(distance(zMax, center) > length(maxVec))
        maxVec = zMax - center;

    maxVec = normalize(maxVec);
    vec3 temp = normalize(cross(maxVec, pl.n));

    // construct transformation to temporary system:
    mat4 m(maxVec.x, temp.x, pl.n.x, center.x,
           maxVec.y, temp.y, pl.n.y, center.y,
           maxVec.z, temp.z, pl.n.z, center.z,
           0.0f,     0.0f,   0.0f,   1.0f);
    tgt::mat4 mInv = tgt::mat4::identity;
    m.invert(mInv);

    // transform bounds to temp system in order to construct new coordinate frame
    tgt::Bounds b(vh->getLLF(), vh->getURB());
    b = b.transform(mInv*pToW);

    // construct new coordinate frame:
    vec3 origin = center;
    origin += b.getLLF().x * maxVec;
    origin += b.getLLF().y * temp;

    vec2 sp(tgt::min(vh->getSpacing()) / samplingRate);
    ivec2 res(tgt::iceil(b.diagonal().x / sp.x), tgt::iceil(b.diagonal().y / sp.y));

    vec3 xVec = maxVec * (sp.x * res.x);
    vec3 yVec = temp * (sp.y * res.y);

    LGL_ERROR;
    tgt::Texture* tex = new tgt::Texture(tgt::ivec3(res, 1), GL_ALPHA, GL_ALPHA32F_ARB, GL_FLOAT, tgt::Texture::LINEAR); //TODO: make dependent on input, add support for multiple channels
    LGL_ERROR;
    vec3 fetchX = normalize(xVec) * sp.x;
    vec3 fetchY = normalize(yVec) * sp.y;
    vec3 fetchOrigin = origin + (0.5f * fetchX) + (0.5f * fetchY);
    mat4 wToV = vh->getWorldToVoxelMatrix();
    vec3 dims = vh->getDimensions();
    for(int x=0; x<res.x; x++) {
        for(int y=0; y<res.y; y++) {
            vec3 pos = fetchOrigin + ((float)x * fetchX) + ((float)y * fetchY);
            pos = wToV * pos;
            float valueFloat = 0.0f;
            if(hand(greaterThanEqual(pos, vec3(0.0f))) && hand(lessThanEqual(pos, dims)))
                valueFloat = vol->getVoxelNormalizedLinear(pos);
            //uint8_t value = tgt::iround(valueFloat * 255.0f);

            tex->texel<float>(x, y) = valueFloat;
            //tex->texel<tgt::col4>(x, y) = tgt::col4(value);
            //tex->texel<tgt::col4>(x, y) = tgt::col4(128);
            //tex->texel<tgt::col4>(x, y) = tgt::col4(x);
        }
    }
    LGL_ERROR;
    tex->uploadTexture();
    LGL_ERROR;
    return new VolumeSliceGL(tex->getDimensions().xy(), UNALIGNED_PLANE, vh->getFormat(), vh->getBaseType(),
        origin, xVec, yVec, vh->getRealWorldMapping(),
        tex);
}

template<typename T>
void copySliceData(const voreen::VolumeAtomic<T>* volume, T*& dataBuffer,
                   voreen::SliceAlignment sliceAlign, size_t sliceID, bool flipX, bool flipY) {

    tgtAssert(volume, "volume is null");

    tgt::ivec3 volDim = volume->getDimensions();
    switch (sliceAlign) {
    case voreen::YZ_PLANE:
        {
            tgt::ivec2 sliceDim = volDim.yz();
            tgtAssert(static_cast<int>(sliceID) < volDim.x, "invalid slice id");
            dataBuffer = new T[tgt::hmul(sliceDim)];
            for (size_t y=0; y<static_cast<size_t>(sliceDim.y); y++) {
                for (size_t x=0; x<static_cast<size_t>(sliceDim.x); x++) {
                    dataBuffer[y*sliceDim.x + x] = volume->voxel(sliceID, !flipX ? x : (sliceDim.x-1) - x,
                                                                          !flipY ? y : (sliceDim.y-1) - y);
                }
            }
        }
        break;
    case voreen::XZ_PLANE:
        {
            tgt::ivec2 sliceDim(volDim.x, volDim.z);
            tgtAssert(static_cast<int>(sliceID) < volDim.y, "invalid slice id");
            dataBuffer = new T[tgt::hmul(sliceDim)];
            for (size_t y=0; y<static_cast<size_t>(sliceDim.y); y++) {
                for (size_t x=0; x<static_cast<size_t>(sliceDim.x); x++) {
                    dataBuffer[y*sliceDim.x + x] = volume->voxel(!flipX ? x : (sliceDim.x-1) - x, sliceID,
                                                                 !flipY ? y : (sliceDim.y-1) - y);
                }
            }
        }
        break;
    case voreen::XY_PLANE:
        {
            tgt::ivec2 sliceDim(volDim.x, volDim.y);
            tgtAssert(static_cast<int>(sliceID) < volDim.z, "invalid slice id");
            dataBuffer = new T[tgt::hmul(sliceDim)];
            for (size_t y=0; y<static_cast<size_t>(sliceDim.y); y++) {
                for (size_t x=0; x<static_cast<size_t>(sliceDim.x); x++) {
                    dataBuffer[y*sliceDim.x + x] = volume->voxel(!flipX ? x : (sliceDim.x-1) - x,
                                                                 !flipY ? y : (sliceDim.y-1) - y, sliceID);
                }
            }
        }
        break;
    default:
        tgtAssert(false, "invalid slice alignment");

    }
}

void VolumeSliceHelper::extractAlignedSlicePixelData(const VolumeRAM* volumeRAM, SliceAlignment sliceAlign, size_t sliceID,
    void*& dataBuffer, GLint& textureFormat, GLint& internalFormat, GLenum& textureDataType)
    throw (VoreenException)
{
    tgtAssert(volumeRAM, "null pointer passed");
    tgtAssert(sliceID < volumeRAM->getDimensions()[sliceAlign], "invalid slice index");

    tgt::svec3 volDim = volumeRAM->getDimensions();

    bool flipX = false;
    bool flipY = false;
    tgt::svec3 sliceTexDim = volDim;
    if (sliceAlign == XY_PLANE) {
        sliceTexDim = tgt::svec3(volDim.xy(), 1);
    }
    else if (sliceAlign == XZ_PLANE) {
        sliceTexDim = tgt::svec3(volDim.x, volDim.z, 1);
    }
    else if (sliceAlign == YZ_PLANE) {
        sliceTexDim = tgt::svec3(volDim.y, volDim.z, 1);
    }
    else {
        tgtAssert(false, "unknown/unsupported slice alignment");
        LERROR("Unknown/unsupported slice alignment: " << sliceAlign);
        return;
    }

    //
    // allocate pixel data buffer and copy over pixel data from volume
    //

    try {
        // scalar
        if (dynamic_cast<const VolumeAtomic<uint8_t>*>(volumeRAM)) {
            textureFormat = GL_ALPHA;
            internalFormat = GL_ALPHA8;
            textureDataType = GL_UNSIGNED_BYTE;
            copySliceData<uint8_t>(static_cast<const VolumeAtomic<uint8_t>*>(volumeRAM), reinterpret_cast<uint8_t*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<int8_t>*>(volumeRAM)) {
            textureFormat = GL_ALPHA;
            internalFormat = GL_ALPHA8;
            textureDataType = GL_BYTE;
            copySliceData<int8_t>(static_cast<const VolumeAtomic<int8_t>*>(volumeRAM), reinterpret_cast<int8_t*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<uint16_t>*>(volumeRAM)) {
            textureFormat = GL_ALPHA;
            internalFormat = GL_ALPHA16;
            textureDataType = GL_UNSIGNED_SHORT;
            copySliceData<uint16_t>(static_cast<const VolumeAtomic<uint16_t>*>(volumeRAM), reinterpret_cast<uint16_t*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<int16_t>*>(volumeRAM)) {
            textureFormat = GL_ALPHA;
            internalFormat = GL_ALPHA16;
            textureDataType = GL_SHORT;
            copySliceData<int16_t>(static_cast<const VolumeAtomic<int16_t>*>(volumeRAM), reinterpret_cast<int16_t*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<uint32_t>*>(volumeRAM)) {
            textureFormat = GL_ALPHA;
            internalFormat = GL_ALPHA;
            textureDataType = GL_UNSIGNED_INT;
            copySliceData<uint32_t>(static_cast<const VolumeAtomic<uint32_t>*>(volumeRAM), reinterpret_cast<uint32_t*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<int32_t>*>(volumeRAM)) {
            textureFormat = GL_ALPHA;
            internalFormat = GL_ALPHA;
            textureDataType = GL_INT;
            copySliceData<int32_t>(static_cast<const VolumeAtomic<int32_t>*>(volumeRAM), reinterpret_cast<int32_t*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<uint64_t>*>(volumeRAM)) {
            throw VoreenException("Texture data type 'uint64' not supported by OpenGL");
        }
        else if (dynamic_cast<const VolumeAtomic<int64_t>*>(volumeRAM)) {
            throw VoreenException("Texture data type 'int64' not supported by OpenGL");
        }
        else if (dynamic_cast<const VolumeAtomic<float>*>(volumeRAM)) {
            textureFormat = GL_ALPHA;
            internalFormat = GL_ALPHA32F_ARB;
            textureDataType = GL_FLOAT;
            copySliceData<float>(static_cast<const VolumeAtomic<float>*>(volumeRAM), reinterpret_cast<float*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<double>*>(volumeRAM)) {
            throw VoreenException("Texture data type 'double' not supported by OpenGL");
        }
        // vec2
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<uint8_t> >*>(volumeRAM)) {
            textureFormat = GL_RG;
            internalFormat = GL_RG8;
            textureDataType = GL_UNSIGNED_BYTE;
            copySliceData<tgt::Vector2<uint8_t> >(static_cast<const VolumeAtomic<tgt::Vector2<uint8_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector2<uint8_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<int8_t> >*>(volumeRAM)) {
            textureFormat = GL_RG;
            internalFormat = GL_RG8;
            textureDataType = GL_BYTE;
            copySliceData<tgt::Vector2<int8_t> >(static_cast<const VolumeAtomic<tgt::Vector2<int8_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector2<int8_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<uint16_t> >*>(volumeRAM)) {
            textureFormat = GL_RG;
            internalFormat = GL_RG16;
            textureDataType = GL_UNSIGNED_SHORT;
            copySliceData<tgt::Vector2<uint16_t> >(static_cast<const VolumeAtomic<tgt::Vector2<uint16_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector2<uint16_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<int16_t> >*>(volumeRAM)) {
            textureFormat = GL_RG;
            internalFormat = GL_RG16;
            textureDataType = GL_SHORT;
            copySliceData<tgt::Vector2<int16_t> >(static_cast<const VolumeAtomic<tgt::Vector2<int16_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector2<int16_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<uint32_t> >*>(volumeRAM)) {
            textureFormat = GL_RG;
            internalFormat = GL_RG;
            textureDataType = GL_UNSIGNED_INT;
            copySliceData<tgt::Vector2<uint32_t> >(static_cast<const VolumeAtomic<tgt::Vector2<uint32_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector2<uint32_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<int32_t> >*>(volumeRAM)) {
            textureFormat = GL_RG;
            internalFormat = GL_RG;
            textureDataType = GL_INT;
            copySliceData<tgt::Vector2<int32_t> >(static_cast<const VolumeAtomic<tgt::Vector2<int32_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector2<int32_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<uint64_t> >*>(volumeRAM)) {
            throw VoreenException("Texture data type 'uint64' not supported by OpenGL");
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<int64_t> >*>(volumeRAM)) {
            throw VoreenException("Texture data type 'int64' not supported by OpenGL");
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<float> >*>(volumeRAM)) {
            textureFormat = GL_RG;
            internalFormat = GL_RG32F;
            textureDataType = GL_FLOAT;
            copySliceData<tgt::Vector2<float> >(static_cast<const VolumeAtomic<tgt::Vector2<float> >*>(volumeRAM), reinterpret_cast<tgt::Vector2<float>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<double> >*>(volumeRAM)) {
            throw VoreenException("Texture data type 'double' not supported by OpenGL");
        }
        // vec3
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<uint8_t> >*>(volumeRAM)) {
            textureFormat = GL_RGB;
            internalFormat = GL_RGB8;
            textureDataType = GL_UNSIGNED_BYTE;
            copySliceData<tgt::Vector3<uint8_t> >(static_cast<const VolumeAtomic<tgt::Vector3<uint8_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector3<uint8_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<int8_t> >*>(volumeRAM)) {
            textureFormat = GL_RGB;
            internalFormat = GL_RGB8;
            textureDataType = GL_BYTE;
            copySliceData<tgt::Vector3<int8_t> >(static_cast<const VolumeAtomic<tgt::Vector3<int8_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector3<int8_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<uint16_t> >*>(volumeRAM)) {
            textureFormat = GL_RGB;
            internalFormat = GL_RGB16;
            textureDataType = GL_UNSIGNED_SHORT;
            copySliceData<tgt::Vector3<uint16_t> >(static_cast<const VolumeAtomic<tgt::Vector3<uint16_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector3<uint16_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<int16_t> >*>(volumeRAM)) {
            textureFormat = GL_RGB;
            internalFormat = GL_RGB16;
            textureDataType = GL_SHORT;
            copySliceData<tgt::Vector3<int16_t> >(static_cast<const VolumeAtomic<tgt::Vector3<int16_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector3<int16_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<uint32_t> >*>(volumeRAM)) {
            textureFormat = GL_RGB;
            internalFormat = GL_RGB;
            textureDataType = GL_UNSIGNED_INT;
            copySliceData<tgt::Vector3<uint32_t> >(static_cast<const VolumeAtomic<tgt::Vector3<uint32_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector3<uint32_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<int32_t> >*>(volumeRAM)) {
            textureFormat = GL_RGB;
            internalFormat = GL_RGB;
            textureDataType = GL_INT;
            copySliceData<tgt::Vector3<int32_t> >(static_cast<const VolumeAtomic<tgt::Vector3<int32_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector3<int32_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<uint64_t> >*>(volumeRAM)) {
            throw VoreenException("Texture data type 'uint64' not supported by OpenGL");
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<int64_t> >*>(volumeRAM)) {
            throw VoreenException("Texture data type 'int64' not supported by OpenGL");
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<float> >*>(volumeRAM)) {
            textureFormat = GL_RGB;
            internalFormat = GL_RGB32F;
            textureDataType = GL_FLOAT;
            copySliceData<tgt::Vector3<float> >(static_cast<const VolumeAtomic<tgt::Vector3<float> >*>(volumeRAM), reinterpret_cast<tgt::Vector3<float>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<double> >*>(volumeRAM)) {
            throw VoreenException("Texture data type 'double' not supported by OpenGL");
        }
        // vec4
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<uint8_t> >*>(volumeRAM)) {
            textureFormat = GL_RGBA;
            internalFormat = GL_RGBA8;
            textureDataType = GL_UNSIGNED_BYTE;
            copySliceData<tgt::Vector4<uint8_t> >(static_cast<const VolumeAtomic<tgt::Vector4<uint8_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector4<uint8_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<int8_t> >*>(volumeRAM)) {
            textureFormat = GL_RGBA;
            internalFormat = GL_RGBA8;
            textureDataType = GL_BYTE;
            copySliceData<tgt::Vector4<int8_t> >(static_cast<const VolumeAtomic<tgt::Vector4<int8_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector4<int8_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<uint16_t> >*>(volumeRAM)) {
            textureFormat = GL_RGBA;
            internalFormat = GL_RGBA16;
            textureDataType = GL_UNSIGNED_SHORT;
            copySliceData<tgt::Vector4<uint16_t> >(static_cast<const VolumeAtomic<tgt::Vector4<uint16_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector4<uint16_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<int16_t> >*>(volumeRAM)) {
            textureFormat = GL_RGBA;
            internalFormat = GL_RGBA16;
            textureDataType = GL_SHORT;
            copySliceData<tgt::Vector4<int16_t> >(static_cast<const VolumeAtomic<tgt::Vector4<int16_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector4<int16_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<uint32_t> >*>(volumeRAM)) {
            textureFormat = GL_RGBA;
            internalFormat = GL_RGBA;
            textureDataType = GL_UNSIGNED_INT;
            copySliceData<tgt::Vector4<uint32_t> >(static_cast<const VolumeAtomic<tgt::Vector4<uint32_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector4<uint32_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<int32_t> >*>(volumeRAM)) {
            textureFormat = GL_RGBA;
            internalFormat = GL_RGBA;
            textureDataType = GL_INT;
            copySliceData<tgt::Vector4<int32_t> >(static_cast<const VolumeAtomic<tgt::Vector4<int32_t> >*>(volumeRAM), reinterpret_cast<tgt::Vector4<int32_t>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<uint64_t> >*>(volumeRAM)) {
            throw VoreenException("Texture data type 'uint64' not supported by OpenGL");
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<int64_t> >*>(volumeRAM)) {
            throw VoreenException("Texture data type 'int64' not supported by OpenGL");
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<float> >*>(volumeRAM)) {
            textureFormat = GL_RGBA;
            internalFormat = GL_RGBA32F;
            textureDataType = GL_FLOAT;
            copySliceData<tgt::Vector4<float> >(static_cast<const VolumeAtomic<tgt::Vector4<float> >*>(volumeRAM), reinterpret_cast<tgt::Vector4<float>*&>(dataBuffer),
                sliceAlign, sliceID, flipX, flipY);
        }
        else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<double> >*>(volumeRAM)) {
            throw VoreenException("Texture data type 'double' not supported by OpenGL");
        }
        else {
            throw VoreenException("unknown or unsupported volume type");
        }
    }
    catch (std::exception& e) {
        throw VoreenException("Failed to extract slice pixel data: " + std::string(e.what()));
    }

}

TriangleMeshGeometryVec3* VolumeSliceHelper::getSliceGeometry(const VolumeBase* vh, SliceAlignment alignment, float sliceIndex, bool applyTransformation, const std::vector<const VolumeBase*> secondaryVolumes) {
    TriangleMeshGeometryVec3* slice = new TriangleMeshGeometryVec3();
    vec3 urb = vh->getURB();
    vec3 llf = vh->getLLF();
    vec3 sp = vh->getSpacing();
    tgt::Bounds b(llf, urb);

    mat4 wToP = vh->getWorldToPhysicalMatrix();
    for(size_t i=0; i<secondaryVolumes.size(); i++) {
        tgt::Bounds sb(secondaryVolumes[i]->getLLF(), secondaryVolumes[i]->getURB());
        tgt::Bounds sbTf = sb.transform(wToP * secondaryVolumes[i]->getPhysicalToWorldMatrix());

        b.addPoint(sbTf.getLLF());
        b.addPoint(sbTf.getURB());
    }

    vec3 bb_urb = b.getURB();
    vec3 bb_llf = b.getLLF();

    switch(alignment) {
        case YZ_PLANE: {
                           float x = sliceIndex;
                           float xcoord = llf.x + (x+0.5f) * sp.x; // We want our slice to be in the center of voxels

                           slice->addQuad(
                           VertexVec3(tgt::vec3(xcoord, bb_urb.y, bb_urb.z), tgt::vec3(xcoord, bb_urb.y, bb_urb.z)),
                           VertexVec3(tgt::vec3(xcoord, bb_urb.y, bb_llf.z), tgt::vec3(xcoord, bb_urb.y, bb_llf.z)),
                           VertexVec3(tgt::vec3(xcoord, bb_llf.y, bb_llf.z), tgt::vec3(xcoord, bb_llf.y, bb_llf.z)),
                           VertexVec3(tgt::vec3(xcoord, bb_llf.y, bb_urb.z), tgt::vec3(xcoord, bb_llf.y, bb_urb.z)));
                       }
                       break;
        case XZ_PLANE: {
                           float y = sliceIndex;
                           float ycoord = llf.y + (y+0.5f) * sp.y; // We want our slice to be in the center of voxels

                           slice->addQuad(
                           VertexVec3(tgt::vec3(bb_urb.x, ycoord, bb_urb.z), tgt::vec3(bb_urb.x, ycoord, bb_urb.z)),
                           VertexVec3(tgt::vec3(bb_urb.x, ycoord, bb_llf.z), tgt::vec3(bb_urb.x, ycoord, bb_llf.z)),
                           VertexVec3(tgt::vec3(bb_llf.x, ycoord, bb_llf.z), tgt::vec3(bb_llf.x, ycoord, bb_llf.z)),
                           VertexVec3(tgt::vec3(bb_llf.x, ycoord, bb_urb.z), tgt::vec3(bb_llf.x, ycoord, bb_urb.z)));
                       }
                       break;
        case XY_PLANE: {
                           float z = sliceIndex;
                           float zcoord = llf.z + (z+0.5f) * sp.z; // We want our slice to be in the center of voxels

                           slice->addQuad(
                           VertexVec3(tgt::vec3(bb_urb.x, bb_urb.y, zcoord), tgt::vec3(bb_urb.x, bb_urb.y, zcoord)),
                           VertexVec3(tgt::vec3(bb_urb.x, bb_llf.y, zcoord), tgt::vec3(bb_urb.x, bb_llf.y, zcoord)),
                           VertexVec3(tgt::vec3(bb_llf.x, bb_llf.y, zcoord), tgt::vec3(bb_llf.x, bb_llf.y, zcoord)),
                           VertexVec3(tgt::vec3(bb_llf.x, bb_urb.y, zcoord), tgt::vec3(bb_llf.x, bb_urb.y, zcoord)));
                       }
                       break;
        default: tgtAssert(false, "should not get here!");
    }
    tgt::mat4 m = vh->getPhysicalToTextureMatrix();

    // set coords to texture coordinates:
    for(size_t j=0; j<slice->getNumTriangles(); ++j) {
        Triangle<VertexVec3> t = slice->getTriangle(j);
        t.v_[0].pos_ = m * t.v_[0].pos_;
        t.v_[1].pos_ = m * t.v_[1].pos_;
        t.v_[2].pos_ = m * t.v_[2].pos_;
        t.v_[0].attr1_ = t.v_[0].pos_;
        t.v_[1].attr1_ = t.v_[1].pos_;
        t.v_[2].attr1_ = t.v_[2].pos_;
        slice->setTriangle(t, j);
    }

    slice->setTransformationMatrix(vh->getTextureToWorldMatrix());
    return slice;
}

//------------------------------------------------------------------------------------------------
// Slice Cache

class SliceCreationBackgroundThread : public BackgroundThread {
public:
    SliceCreationBackgroundThread(Processor* processor, const VolumeSliceCache* sliceCache,
           const VolumeBase* volume, SliceAlignment alignment, size_t sliceIndex,
            size_t levelOfDetail, const std::string& hash)
        //: ProcessorBackgroundThread(processor)
        : BackgroundThread()
        , sliceCache_(sliceCache)
        , volume_(volume)
        , alignment_(alignment)
        , sliceIndex_(sliceIndex)
        , levelOfDetail_(levelOfDetail)
        , hash_(hash)
    {
        //tgtAssert(processor_, "null pointer passed as processor");
        tgtAssert(volume_, "null pointer passed as volume");
        tgtAssert(!hash_.empty(), "passed hash is empty");
    }

    virtual void threadMain() {
        VolumeSliceGL* slice = VolumeSliceHelper::getVolumeSlice(volume_, alignment_, sliceIndex_, levelOfDetail_);

        if (slice) {
            sliceCache_->addSliceToCache(slice, hash_);
        }
    }

    const VolumeSliceCache* sliceCache_;

    const VolumeBase* volume_;
    SliceAlignment alignment_;
    size_t sliceIndex_;
    size_t levelOfDetail_;
    std::string hash_;

}; // SliceCreationBackgroundThread

VolumeSliceCache::VolumeSliceCache(Processor* owner, size_t cacheSize)
    : owner_(owner)
    , cacheSize_(cacheSize)
    , currentBackgroundThread_(0)
{
    tgtAssert(owner_, "null pointer passed");
}

VolumeSliceCache::~VolumeSliceCache() {
    clear();
}

size_t VolumeSliceCache::getCacheSize() const {
    return cacheSize_;
}

void VolumeSliceCache::setCacheSize(size_t cacheSize) {
    cacheSize_ = cacheSize;
    while (slices_.size() > cacheSize_) {
        delete slices_.back().slice_;
        slices_.pop_back();
    }
}

void VolumeSliceCache::clear() {
    boost::lock_guard<boost::mutex> lock(cacheAccessMutex_);

    if (currentBackgroundThread_ && currentBackgroundThread_->isRunning())
        currentBackgroundThread_->interruptAndJoin();
    delete currentBackgroundThread_;
    currentBackgroundThread_ = 0;

    for (std::list<CacheEntry>::iterator it = slices_.begin(); it != slices_.end(); ++it) {
        tgtAssert(it->slice_, "cache entry does not store slice");
        delete it->slice_;
    }
    slices_.clear();
}

VolumeSliceGL* VolumeSliceCache::getVolumeSlice(const VolumeBase* volume, SliceAlignment alignment, size_t sliceIndex,
    size_t levelOfDetail /*= 0*/, clock_t timeLimit /*= 0*/, bool* complete /*= 0*/, bool asynchronous /*= false*/) const
{
    tgtAssert(volume, "null pointer passed");

    std::string hash = getHash(volume, alignment, sliceIndex, levelOfDetail);

    // check if slice is present in cache, otherwise create it and add it to cache (if complete)
    VolumeSliceGL* slice = findSliceInCache(hash, true);
    bool sliceComplete = true;

    if (!slice) {

        if (!asynchronous) { // create slice synchronously
            slice = VolumeSliceHelper::getVolumeSlice(volume, alignment, sliceIndex, levelOfDetail, timeLimit, &sliceComplete);
            if (slice && sliceComplete) {
                addSliceToCache(slice, hash);
                cleanupCache();
                tgtAssert(slices_.size() <= cacheSize_, "invalid cache size");
            }
        }
        else { // create slice in background thread
            bool sliceCurrentlyCreated = currentBackgroundThread_ && currentBackgroundThread_->hash_ == hash;
            if (!sliceCurrentlyCreated) {

                // interrupt/delete current thread
                if (currentBackgroundThread_) {
                    if (currentBackgroundThread_->isRunning()) {
                        currentBackgroundThread_->interruptAndJoin();
                    }
                    delete currentBackgroundThread_;
                    currentBackgroundThread_ = 0;
                }

                cleanupCache();

                // start new thread
                currentBackgroundThread_ = new SliceCreationBackgroundThread(owner_, this, volume, alignment, sliceIndex, levelOfDetail, hash);
                currentBackgroundThread_->run();
            }

            sliceComplete = false;
            slice = 0;
        }
    }

    if (complete)
        *complete = sliceComplete;

    return slice;
}

VolumeSliceGL* VolumeSliceCache::getVolumeSlice(const VolumeBase* volume, tgt::plane pl, float samplingRate, bool asynchronous) const {
    tgtAssert(volume, "null pointer passed");

    std::string hash = getHash(volume, pl, samplingRate);

    // check if slice is present in cache, otherwise create it and add it to cache
    VolumeSliceGL* slice = findSliceInCache(hash, true);

    if (!slice) {
        slice = VolumeSliceHelper::getVolumeSlice(volume, pl, samplingRate);
        if (slice) {
            addSliceToCache(slice, hash);
            cleanupCache();
            tgtAssert(slices_.size() <= cacheSize_, "invalid cache size");
        }
    }

    return slice;
}

// private

void VolumeSliceCache::addSliceToCache(VolumeSliceGL* slice, const std::string& hash) const {
    boost::lock_guard<boost::mutex> lock(cacheAccessMutex_);

    tgtAssert(slice, "null pointer passed");
    tgtAssert(!hash.empty(), "hash is empty");

    if (cacheSize_ == 0)
        return;

    CacheEntry entry;
    entry.hash_ = hash;
    entry.slice_ = slice;
    slices_.push_front(entry);
}


void VolumeSliceCache::cleanupCache() const {
    boost::lock_guard<boost::mutex> lock(cacheAccessMutex_);

    while (slices_.size() > cacheSize_) {
        delete slices_.back().slice_;
        slices_.pop_back();
    }
    tgtAssert(slices_.size() <= cacheSize_, "invalid cache size");
}

VolumeSliceGL* VolumeSliceCache::findSliceInCache(const std::string& hash, bool updateUsage) const {
    boost::lock_guard<boost::mutex> lock(cacheAccessMutex_);

    tgtAssert(!hash.empty(), "hash is empty");
    for (std::list<CacheEntry>::iterator it = slices_.begin(); it != slices_.end(); ++it) {
        if (it->hash_ == hash) {
            CacheEntry entry = *it;
            if (updateUsage) { // move found entry to front
                slices_.erase(it);
                slices_.push_front(entry);
            }
            tgtAssert(entry.slice_, "cache entry does not contain slice");
            return entry.slice_;
        }
    }
    return 0;
}

bool VolumeSliceCache::hasSliceInCache(const VolumeBase* volume, SliceAlignment alignment, size_t sliceIndex, size_t levelOfDetail /*= 0*/) const {
    return (findSliceInCache(getHash(volume, alignment, sliceIndex, levelOfDetail), false) != 0);
}

bool VolumeSliceCache::hasSliceInCache(const VolumeBase* volume, tgt::plane pl, float samplingRate) const {
    return (findSliceInCache(getHash(volume, pl, samplingRate), false) != 0);
}

std::string VolumeSliceCache::getHash(const VolumeBase* volume, SliceAlignment alignment, size_t sliceIndex, size_t levelOfDetail) const {
    std::ostringstream configStr;
    configStr << volume->getHash();
    configStr << alignment;
    configStr << sliceIndex;
    configStr << levelOfDetail;
    return VoreenHash::getHash(configStr.str());
}

std::string VolumeSliceCache::getHash(const VolumeBase* volume, tgt::plane pl, float samplingRate) const {
    std::ostringstream configStr;
    configStr << volume->getHash();
    configStr << pl;
    configStr << samplingRate;
    return VoreenHash::getHash(configStr.str());

}

} // namespace voreen
