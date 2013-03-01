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

#ifndef VRN_VOLUMEDISK_H
#define VRN_VOLUMEDISK_H

#include <typeinfo>
#include <vector>

#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volumegl.h"

namespace voreen {

/// A representation storing the information to do a lazy loading of the volume data.
class VRN_CORE_API VolumeDisk : public VolumeRepresentation {
public:
    /**
     * @param filename Absolute file name.
     * @param format @see VolumeFactory
     * @param offset If offset >= 0: Bytes offset from beginning of file. If offset < 0: assume data is aligned to end of file.
     * @param swapEndian Swaps endianness if true.
     */
    VolumeDisk(const std::string& filename, const std::string& format, tgt::ivec3 dimensions, int64_t offset = 0, bool swapEndian = false);
    VolumeDisk(const VolumeDisk* diskrep);

    virtual ~VolumeDisk();

    std::string getFileName() const { return filename_; }
    ///@see VolumeFactory
    virtual std::string getFormat() const { return format_; }

    /// Returns the base type (e.g., "float" for a representation of format "Vector3(float)").
    virtual std::string getBaseType() const;

    virtual int getNumChannels() const;
    virtual int getBytesPerVoxel() const;
    ///loads slices from the disk volume and returns the new (ram)volume
    virtual VolumeRAM* loadSlices(const size_t firstSlice, const size_t lastSlice) const;
    ///loads a sub brick volume
    virtual VolumeRAM* loadBrick(const tgt::svec3 offset, const tgt::svec3 dimensions) const;
    ///Creates new disk representation based on current disk representation.
    virtual VolumeDisk* getSubVolume(tgt::svec3 dimensions, tgt::svec3 offset = tgt::svec3(0,0,0)) const throw (std::bad_alloc);

    ///Offset in the file (in bytes).
    int64_t getOffset() const { return offset_; }
    bool getSwapEndian() const { return swapEndian_; }
protected:
    std::string filename_;
    std::string format_;
    int64_t offset_;
    bool swapEndian_;

    static const std::string loggerCat_;
};

/// Creates a VolumeRam from a VolumeDisk.
class VRN_CORE_API  RepresentationConverterLoadFromDisk : public RepresentationConverter<VolumeRAM> {
public:
    virtual bool canConvert(const VolumeRepresentation* source) const;
    virtual VolumeRepresentation* convert(const VolumeRepresentation* source) const;
};

/// Creates a VolumeGL from a VolumeDisk.
class VRN_CORE_API  RepresentationConverterLoadFromDiskToGL : public RepresentationConverter<VolumeGL> {
public:
    virtual bool canConvert(const VolumeRepresentation* source) const;
    virtual VolumeRepresentation* convert(const VolumeRepresentation* source) const;
};

} // namespace voreen

#endif
