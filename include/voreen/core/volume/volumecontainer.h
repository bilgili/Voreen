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

#ifndef VRN_VOLUMECONTAINER_H
#define VRN_VOLUMECONTAINER_H

#include "voreen/core/volume/volumecollection.h"
#include "voreen/core/volume/volumehandle.h"

#include <vector>
#include <string>

namespace voreen {

class Modality;

class VolumeContainer : public VolumeCollection {

public:

    VolumeContainer();

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
    virtual void add(const VolumeCollection& volumeCollection);

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
     * @return VolumeCollection containing VolumeHandles
     *      that wrap the loaded volumes.
     *
     * @todo exception on load failure
     */
    virtual VolumeCollection* loadVolume(const std::string& filename) throw (tgt::FileException, std::bad_alloc);

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
     *
     * @throw tgt::FileException
     * @throw std::bad_alloc
     *
     * @return VolumeHandle wrapping the loaded volume.
     *
     * @todo exception on load failure
     */
    virtual VolumeHandle* loadRawVolume(const std::string& filename,
                                        const std::string& objectModel, const std::string& format,
                                        const tgt::ivec3& dimensions, const tgt::vec3& spacing = tgt::vec3(1.f),
                                        int headerskip = 0) throw (tgt::FileException, std::bad_alloc);;

    /**
     * Deletes the passed VolumeHandle and removes it
     * from the container. If the passed handle is not contained
     * by the container, this function has no effect.
     */
    virtual void free(const VolumeHandle* handle);

    /**
     * Deletes all VolumeHandles contained by the passed collection
     * and removes them from the container. Handles that are not
     * contained by the container are not touched.
     */
    virtual void free(const VolumeCollection& volumeCollection);

    /**
     * Deletes all VolumeHandles of the container
     * and clears it afterwards.
     */
    virtual void freeAll();

protected:

    /// category for logging.
    static const std::string loggerCat_;

};


}   // namespace

#endif
