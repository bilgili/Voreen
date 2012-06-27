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

#ifndef VRN_BRICKINGINFORMATION_H
#define VRN_BRICKINGINFORMATION_H

#include "voreen/core/datastructures/volume/bricking/brick.h"

#include "tgt/camera.h"

namespace voreen {

    class BrickingRegionManager;

    /**
     * This struct holds all the information needed during the different steps
     * of bricking. Some of these steps like calculating brick resolutions need
     * a lot of paramters and return a lot of values. Therefore, this struct is used
     * and is mostly passed by reference. Instead of returning a lot of values,
     * these values are written to this struct and as a result all the other steps
     * have all the information they need (because of passing this struct by
     * reference).
     */
    struct BrickingInformation {

        /**
        * The name of the volume that is to be bricked without filename extension.
        */
        std::string originalVolumeName;

        /**
        * The size in byte of one voxel in the original volume.
        */
        int originalVolumeVoxelSizeInByte;

        /**
        * The number of voxels in the original volume.
        */
        uint64_t originalVolumeNumVoxels;

        /**
        * The size in megabyte of the original volume.
        */
        int originalVolumeSizeMB;

        /**
        * The dimensions of the original volume.
        */
        tgt::ivec3 originalVolumeDimensions;

        /**
        * The lower left front of the original volume in world coordinates.
        */
        tgt::vec3 originalVolumeLLF;

        /**
        * The upper right back of the original volume in world coordinates.
        */
        tgt::vec3 originalVolumeURB;

        /**
        * The transformation matrix of the original volume.
        */
        tgt::mat4 originalTransformationMatrix;

        /**
        * The spacing of the original volume.
        */
        tgt::vec3 originalVolumeSpacing;

        /**
        * Bits stored of the original volume.
        */
        int originalVolumeBitsStored;

        /**
        * The number of BYTES allocated for each voxel in
        * the original volume.
        */
        int originalVolumeBytesAllocated;

        /**
        * The format of the original volume, lile UCHAR or USHORT
        */
        std::string originalVolumeFormat;

        /**
        * The ObjectModel of the original volume, like RGB or RGBA
        */
        std::string originalVolumeModel;

        /**
        * The total number of bricks needed to store the complete
        * original volume.
        */
        int totalNumberOfBricksNeeded;

        /**
        * The number of bricks in each dimension neccessary to store
        * the original volume.
        */
        tgt::ivec3 numBricks;

        /**
        * The number of bricks that have "empty" volumes, meaning
        * all their voxels have the same value. Such bricks can
        * be downsampled to the extreme, as no information is lost.
        */
        int numberOfBricksWithEmptyVolumes;

        /**
        * The number of voxels needed to store the packed volume.
        */
        int numberOfVoxelsNeededForPackedVolume;

        /**
        * The dimensions of the packed volume neccessary to hold all data.
        */
        tgt::ivec3 packedVolumeDimensions;

        /**
        * The size of a brick in each dimension.
        */
        int brickSize;

        /**
        * The number of voxels inside a brick.
        */
        int numVoxelsInBrick;

        /**
        * The total size of the memory on the gpu.
        */
        int gpuMemorySize;

        /**
        * A reserve for the memory on the graphics card. The brickingmanager
        * will try to fill the graphics card memory as much as possible in order
        * to produce an image with the best possible quality. But the dataset won't
        * be on the graphics card exclusively, we have to account for transfer functions,
        * rendertargets etc. That's why we introduce this reserve that the brickingmanager
        * won't touch.
        */
        int gpuMemoryReserve;

        /**
        * gpuMemorySizeMB_ - gpuMemoryReserve_
        */
        int gpuAvailableMemory;

        /*
        * The first element in the vector defines the number of bricks with maximum
        * resolution, 2nd element the number of bricks with (maximum/2) resolution
        * that fit into gpu memory and so on.
        */
        std::vector<int> brickResolutions;

        /**
        * The total number of different resolutions possible for a brick
        */
        int totalNumberOfResolutions;

        /**
        * A map storing the dimensions of volume data at a specific level of detail.
        */
        std::map<int,tgt::ivec3> lodToDimensionsMap;

        /**
        * The bricks the volume is divided into. These bricks are actually
        * of the type VolumeBrick<T>* , but this way they can be used in non-templated
        * classes.
        */
        std::vector<Brick*> volumeBricks;

        /**
        * The packingbricks subdivide a volume and receive their data from the volumebricks later on.
        * They basically only exist to make the packing of the resampled volumedata
        * into the packed volume efficient.
        */
        std::list<Brick*> packingBricks;

        /**
        * This list holds all PackingBricks that don't hold a VolumeBrick with only voxels
        * of the same value. So if the ResolutionCalculator is changed and new packing bricks
        * are necessary, one can just copy these. This saves rereading the voxel data from disc
        * again, as the volume bricks with only voxels of the same value retain their position
        * in the packed volume.
        */
        std::list<Brick*> packingBrickBackups;

        /**
        * These packingbricks have been given data and a position in the packed volume.
        * Calling write() for all these generates the packed volume.
        */
        std::vector<Brick*> packingBricksWithData;

        /**
        * The camera position. Mostly used for Lod selection
        */
        tgt::Camera* camera;

        /**
        * The class responsible for managing the regions that can be defined for bricking.
        */
        BrickingRegionManager* regionManager;

    };

} //namespace

#endif
