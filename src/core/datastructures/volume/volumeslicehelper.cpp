/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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
#include "tgt/logmanager.h"

namespace voreen {

using tgt::vec2;
using tgt::ivec2;
using tgt::vec3;
using tgt::mat4;

FaceGeometry getSliceGeometry(const VolumeBase* vh, SliceAlignment alignment, float sliceIndex, bool applyTransformation, const std::vector<const VolumeBase*> secondaryVolumes) {
    FaceGeometry slice;
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

                           slice.addVertex(VertexGeometry(tgt::vec3(xcoord, bb_urb.y, bb_urb.z), tgt::vec3(xcoord, bb_urb.y, bb_urb.z), tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
                           slice.addVertex(VertexGeometry(tgt::vec3(xcoord, bb_urb.y, bb_llf.z), tgt::vec3(xcoord, bb_urb.y, bb_llf.z), tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
                           slice.addVertex(VertexGeometry(tgt::vec3(xcoord, bb_llf.y, bb_llf.z), tgt::vec3(xcoord, bb_llf.y, bb_llf.z), tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
                           slice.addVertex(VertexGeometry(tgt::vec3(xcoord, bb_llf.y, bb_urb.z), tgt::vec3(xcoord, bb_llf.y, bb_urb.z), tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
                       }
                       break;
        case XZ_PLANE: {
                           float y = sliceIndex;
                           float ycoord = llf.y + (y+0.5f) * sp.y; // We want our slice to be in the center of voxels

                           slice.addVertex(VertexGeometry(tgt::vec3(bb_urb.x, ycoord, bb_urb.z), tgt::vec3(bb_urb.x, ycoord, bb_urb.z), tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
                           slice.addVertex(VertexGeometry(tgt::vec3(bb_urb.x, ycoord, bb_llf.z), tgt::vec3(bb_urb.x, ycoord, bb_llf.z), tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
                           slice.addVertex(VertexGeometry(tgt::vec3(bb_llf.x, ycoord, bb_llf.z), tgt::vec3(bb_llf.x, ycoord, bb_llf.z), tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
                           slice.addVertex(VertexGeometry(tgt::vec3(bb_llf.x, ycoord, bb_urb.z), tgt::vec3(bb_llf.x, ycoord, bb_urb.z), tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
                       }
                       break;
        case XY_PLANE: {
                           float z = sliceIndex;
                           float zcoord = llf.z + (z+0.5f) * sp.z; // We want our slice to be in the center of voxels

                           slice.addVertex(VertexGeometry(tgt::vec3(bb_urb.x, bb_urb.y, zcoord), tgt::vec3(bb_urb.x, bb_urb.y, zcoord), tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
                           slice.addVertex(VertexGeometry(tgt::vec3(bb_urb.x, bb_llf.y, zcoord), tgt::vec3(bb_urb.x, bb_llf.y, zcoord), tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
                           slice.addVertex(VertexGeometry(tgt::vec3(bb_llf.x, bb_llf.y, zcoord), tgt::vec3(bb_llf.x, bb_llf.y, zcoord), tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
                           slice.addVertex(VertexGeometry(tgt::vec3(bb_llf.x, bb_urb.y, zcoord), tgt::vec3(bb_llf.x, bb_urb.y, zcoord), tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
                       }
                       break;
        default: tgtAssert(false, "should not get here!");
    }

    if(applyTransformation) {
        slice.transform(vh->getPhysicalToWorldMatrix());

        //reset tex coords to coords after transformation:
        for(size_t k=0; k<slice.getVertexCount(); ++k) {
            VertexGeometry& vg = slice.getVertex(k);
            vg.setTexCoords(vg.getCoords());
        }
    }
    return slice;
}

Slice::Slice(tgt::vec3 origin, tgt::vec3 xVec, tgt::vec3 yVec, tgt::Texture* tex, RealWorldMapping rwm) : origin_(origin), xVec_(xVec), yVec_(yVec), tex_(tex), rwm_(rwm)
{
}

Slice::~Slice()
{
    delete tex_;
}

tgt::mat4 Slice::getTextureToWorldMatrix() const {
    vec3 zVec = normalize(cross(xVec_, yVec_));

    mat4 m(xVec_.x, yVec_.x, zVec.x, origin_.x,
           xVec_.y, yVec_.y, zVec.y, origin_.y,
           xVec_.z, yVec_.z, zVec.z, origin_.z,
           0.0f,    0.0f,    0.0f,   1.0f);
    return m;
}

tgt::mat4 Slice::getWorldToTextureMatrix() const {
    mat4 m = getTextureToWorldMatrix();
    mat4 inv;
    m.invert(inv);
    return inv;
}

//-------------------------------------------------------------------------------------------------

Slice* getVolumeSlice(const VolumeBase* vh, tgt::plane pl, float samplingRate) {
    const VolumeRAM* vol = vh->getRepresentation<VolumeRAM>();
    if(!vol)
        return 0;

    vec3 urb = vh->getURB();
    vec3 llf = vh->getLLF();
    vec3 center = (urb + llf) * 0.5f;

    vec3 xMax = center;
    xMax.x = urb.x;
    vec3 yMax = center;
    yMax.y = urb.y;
    vec3 zMax = center;
    zMax.z = urb.z;

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

    // transform bounds to temp system in order to construct new coordinate frame
    tgt::Bounds b(vh->getLLF(), vh->getURB());
    b = b.transform(m*pToW);

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
    return new Slice(origin, xVec, yVec, tex, vh->getRealWorldMapping());
}

} // namespace voreen
