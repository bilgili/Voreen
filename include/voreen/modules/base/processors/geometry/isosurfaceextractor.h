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

#ifndef VRN_ISOSURFACEEXTRACTOR_H
#define VRN_ISOSURFACEEXTRACTOR_H

#include "voreen/core/processors/processor.h"
#include "voreen/core/ports/allports.h"

#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/vectorproperty.h"

#include "voreen/core/datastructures/geometry/meshlistgeometry.h"

namespace voreen {

class IsosurfaceExtractor : public Processor {
public:

    IsosurfaceExtractor();
    virtual ~IsosurfaceExtractor();
    virtual Processor* create() const;

    virtual std::string getClassName() const    { return "IsosurfaceExtractor"; }
    virtual std::string getCategory() const     { return "Geometry"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_EXPERIMENTAL; }

    virtual std::string getProcessorInfo() const;

protected:
    virtual void process();

    VolumePort inport_;
    GeometryPort outport_;

    IntProperty isoValue_;
    FloatVec4Property isoColor_;

    MeshListGeometry geometry_;

    /// category used in logging
    static const std::string loggerCat_;
};

template <typename T>
class MarchingCubes {
public:
    enum VoxelOrder { XYZ, XZY };   // TODO: add support for every possible order

    typedef T VoxelType;

    struct Triangle {
        Triangle() : marker_(false) {}
        Triangle(const tgt::vec3& v0, const tgt::vec3& v1, const tgt::vec3& v2) : marker_(false) {
            vertices_[0] = v0;
            vertices_[1] = v1;
            vertices_[2] = v2;
        }
        tgt::vec3 vertices_[3];
        bool marker_;
    };

public:
    static std::vector<Triangle> march(const T* const texture, const tgt::ivec3& texDim,
        const T isovalue, const VoxelOrder& voxelOrder = XYZ);

private:
    class GridCell {
    public:
        GridCell(const unsigned int number, const T* const texture,
            const tgt::ivec3& texDim, const VoxelOrder& voxelOrder);

        tgt::vec3 vertices_[8];
        T values_[8];

    private:
        const VoxelOrder voxelOrder_;
    };

private:
    static std::vector<Triangle> polygonise(const GridCell& grid, const T isovalue);

    /**
     * Performs linear interpolation of the two given vertices using the given
     * values at those vertices of the cube / grid cell.
     */
    static tgt::vec3 linIntVertex(const int isolevel, const tgt::vec3& p1, const tgt::vec3& p2,
        const int value1, const int value2);

    /**
     * Converts the given position (cartesian coordinates >= 0 in the volume texture) to
     * the corresponding voxel number. The texture is supposed to contain the data in
     * XZY-order.
     */
    static unsigned int posToVoxelNumberXYZ(const tgt::ivec3& pos, const tgt::ivec3& dimensions)
    {
        return ((pos.z * dimensions.x * dimensions.y) + (pos.y * dimensions.x) + pos.x);
    }

    static unsigned int posToVoxelNumberXZY(const tgt::ivec3& pos, const tgt::ivec3& dimensions)
    {
        return ((pos.y * dimensions.x * dimensions.z) + (pos.z * dimensions.x) + pos.x);
    }

    /**
     * Converts the given voxel (its number in the texture) to cartesian coordinates. The
     * texture is supposed to contain the data in XZY-order.
     */
    static tgt::ivec3 voxelNumberToPosXYZ(const unsigned int n, const tgt::ivec3& dimensions)
    {
        return tgt::ivec3((n % dimensions.x), ((n / dimensions.x) % dimensions.y),
            (n / (dimensions.x * dimensions.y)));
    }

    static tgt::ivec3 voxelNumberToPosXZY(const unsigned int n, const tgt::ivec3& dimensions)
    {
        return tgt::ivec3((n % dimensions.x), (n / (dimensions.x * dimensions.z)),
            ((n / dimensions.x) % dimensions.z));
    }

private:
    static const int edgeTable[256];
    static const int triTable[256][16];
};

// ----------------------------------------------------------------------------

template <typename T>
MarchingCubes<T>::GridCell::GridCell(const unsigned int number, const T* const texture,
                                  const tgt::ivec3& texDim, const VoxelOrder& voxelOrder)
    : voxelOrder_(voxelOrder)
{
    // use the correct voxelToPos method according to the voxel order.
    //
    tgt::ivec3 (*voxelNumberToPos)(unsigned int, const tgt::ivec3&);
    unsigned int (*posToVoxelNumber)(const tgt::ivec3&, const tgt::ivec3&);

    switch (voxelOrder_) {
        case XYZ:
            voxelNumberToPos = &MarchingCubes::voxelNumberToPosXYZ;
            posToVoxelNumber = &MarchingCubes::posToVoxelNumberXYZ;
            break;
        case XZY:
            voxelNumberToPos = &MarchingCubes::voxelNumberToPosXZY;
            posToVoxelNumber = &MarchingCubes::posToVoxelNumberXZY;
            break;
        default:
            voxelNumberToPos = 0;
            posToVoxelNumber = 0;
            break;
    }   // switch

    if ((voxelNumberToPos == 0) || (posToVoxelNumber == 0))
        throw std::bad_alloc();

    // left lower back (0, 0, 0)
    tgt::ivec3 pos = (*voxelNumberToPos)(number, texDim);
    vertices_[0] = static_cast<tgt::vec3>(pos);

    // right lower back (1, 0, 0)
    vertices_[1] = vertices_[0];
    vertices_[1].x += 1.0f;

    // right lower front (1, 0, 1);
    vertices_[2] = vertices_[1];
    vertices_[2].z += 1.0f;

    // left lower front (0, 0, 1)
    vertices_[3] = vertices_[0];
    vertices_[3].z += 1.0f;

    // left upper back (0, 1, 0)
    vertices_[4] = vertices_[0];
    vertices_[4].y += 1.0f;

    // right upper back (1, 1, 0)
    vertices_[5] = vertices_[4];
    vertices_[5].x += 1.0f;

    // right upper front (1, 1, 1)
    vertices_[6] = vertices_[5];
    vertices_[6].z += 1.0f;

    // left upper fron (0, 1, 1)
    vertices_[7] = vertices_[4];
    vertices_[7].z += 1.0f;

    tgt::ivec3 p(0, 0, 0);
    for (int i = 0; i < 8; ++i) {
        const tgt::vec3& pos = vertices_[i];
        p = static_cast<tgt::ivec3>(pos);
        unsigned int n = (*posToVoxelNumber)(p, texDim);
        values_[i] = texture[n];
    }
}

} //namespace

#endif // VRN_ISOSURFACEEXTRACTOR_H
