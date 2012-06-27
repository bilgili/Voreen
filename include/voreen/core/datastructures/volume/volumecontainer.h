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

#ifndef VRN_VOLUMECONTAINER_H
#define VRN_VOLUMECONTAINER_H

#include "voreen/core/datastructures/volume/volumecollection.h"
#include "voreen/core/datastructures/volume/volumehandle.h"

#include <vector>
#include <string>

namespace voreen {

/**
 * Extension of the VolumeCollection that performs memory management,
 * i.e., the volume container takes ownership of passed volume handles and
 * deletes them on removal or its own destruction.
 *
 * Additionally, this class provides convenience functions for
 * volume loading.
 *
 * @see VolumeCollection
 */
class VolumeContainer : public VolumeCollection {

public:

    VolumeContainer();

    /**
     * Deletes all contained VolumeHandles.
     *
     * @see releaseAll
     */
    virtual ~VolumeContainer();

   /**
    * Adds the passed VolumeHandle to the container,
    * if it is not already contained.
    *
    * In contrast to the base class VolumeCollection the VolumeContainer
    * does take ownership of the added VolumeHandle and therefore
    * deletes it on its own destruction.
    */
    virtual void add(VolumeHandle* volumeHandle);

   /**
    * Adds VolumeHandles contained by the passed VolumeCollection
    * to the collection, if they are not already contained.
    *
    * In contrast to the base class VolumeCollection the VolumeContainer
    * does take ownership of the added VolumeHandles and therefore
    * deletes them on its own destruction.
    */
    virtual void add(const VolumeCollection* volumeCollection);

    /**
     * Deletes the passed VolumeHandle and removes it
     * from the container. If the passed handle is not contained
     * by the container, this function has no effect.
     *
     * @see releaseVolume
     */
    virtual void remove(const VolumeHandle* handle);

    /**
     * Deletes all VolumeHandles contained by the passed collection
     * and removes them from the container. Handles that are not
     * contained by the container are not touched.
     *
     * @see releaseVolume
     */
    virtual void remove(const VolumeCollection* volumeCollection);

    /**
     * Removes all VolumeHandles objects from the container and deletes them.
     *
     * @see releaseAll
     */
    virtual void clear();

    /**
     * Loads the data set with the specified filename and
     * adds it to the collection. The container takes
     * ownership of the loaded volume and deletes it
     * on destruction. In case, the specified file
     * contains more than one file, all files are loaded.
     *
     * @param filename the volume data file to load
     *
     * @throw tgt::FileException
     * @throw std::bad_alloc
     *
     * @return VolumeCollection containing VolumeHandles that wrap the loaded volumes,
     *     or 0 in case no volume could be loaded
     */
    virtual VolumeCollection* loadVolume(const std::string& filename)
        throw (tgt::FileException, std::bad_alloc);

    /**
     * Loads a raw data set from the specified file
     * and adds it to the collection. The container takes
     * ownership of the loaded volume and deletes it
     * on destruction.
     *
     * @param filename the volume data file to load
     * @param objectModel \c I (intensity) or \c RGBA
     * @param format voxel data format, one of \c UCHAR, \c USHORT, \c USHORT_12 (for CT datasets),
     *        \c FLOAT 8 and \c FLOAT16.
     * @param dimensions number of voxels in x-, y- and z-direction
     * @param spacing size of a single voxel
     * @param headerskip number of bytes to skip at the beginning of the data file
     * @param bigEndian if set to true, the data is converted from big endian to little endian byte order
     *
     * @throw tgt::FileException
     * @throw std::bad_alloc
     *
     * @return VolumeHandle wrapping the loaded volume, or 0 in case the volume could not be loaded
     */
    virtual VolumeHandle* loadRawVolume(const std::string& filename,
                                        const std::string& objectModel, const std::string& format,
                                        const tgt::ivec3& dimensions, const tgt::vec3& spacing = tgt::vec3(1.f),
                                        int headerskip = 0, bool bigEndian = false)
        throw (tgt::FileException, std::bad_alloc);


    /**
     * Removes the passed volume handle from the container
     * without deleting it.
     */
    virtual void release(const VolumeHandle* handle);

    /**
     * Removes all contained volume handles from the container
     * without deleting them.
     */
    virtual void releaseAll();

protected:

    /// category for logging.
    static const std::string loggerCat_;
};

}   // namespace

#endif
