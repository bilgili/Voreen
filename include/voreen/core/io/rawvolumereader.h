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

#ifndef VRN_RAWVOLUMEREADER_H
#define VRN_RAWVOLUMEREADER_H

#include <string>

#include "tgt/vector.h"
#include "tgt/matrix.h"

#include "voreen/core/io/volumereader.h"
#include "voreen/core/datastructures/volume/modality.h"

namespace voreen {

class IOProgress;

/**
 * Reads a raw volume dataset. This requires information hints about dimension, format, etc.,
 * usually given by an accompanying <tt>.dat</tt>. DatVolumeReader reads this information and
 * uses RawVolumeReader to read the actual data.
 */
class RawVolumeReader : public VolumeReader {
public:
    /**
     * Contains hints about the volume dataset.
     */
    struct ReadHints {
        ReadHints(tgt::ivec3 dimensions = tgt::ivec3(0),
                  tgt::vec3 spacing = tgt::vec3(0.f),
                  int bitsStored = 0,
                  const std::string& objectModel = "I",
                  const std::string& format = "UCHAR",
                  int headerskip = 0,
                  bool bigEndianByteOrder = false);

        tgt::ivec3 dimensions_;       ///< number of voxels in x-, y- and z-direction
        tgt::vec3 spacing_;           ///< non-uniform voxel scaling
        int bitsStored_;              ///< number of bits stored in this dataset per voxel
        std::string objectModel_;     ///< \c I (intensity) or \c RGBA
        std::string format_;          ///< voxel data format
        uint64_t headerskip_;         ///< number of bytes to skip at the beginning of the raw file
        bool bigEndianByteOrder_;     ///< data is saved in big endian format an needs to be converted
        tgt::mat4 transformation_;    ///< 4x4-matrix for affine transformation of volume
        Modality modality_;
        float timeStep_;
        float spreadMin_;             ///< minimum value in the volume to use for spreading data range to [0; 1]
        float spreadMax_;             ///< maximum value in the volume to use for spreading data range to [0; 1]
        std::string metaString_;      ///< a string containing arbitrary meta-data
        std::string unit_;            ///< a string containing the unit of the dataset
        std::string sliceOrder_;
    };

    RawVolumeReader(IOProgress* progress = 0);

    virtual VolumeReader* create(IOProgress* progress = 0) const;

    /**
     * Set hints about the volume dataset. Must be set before read() is called. More options
     * can be specified by using the other overloaded variant.
     *
     * @param dimensions number of voxels in x-, y- and z-direction
     * @param spacing non-uniform voxel scaling
     * @param bitsStored number of bits stored in this dataset per voxel
     * @param objectModel \c I (intensity) or \c RGBA
     * @param format voxel data format, one of \c UCHAR, \c USHORT, \c USHORT_12 (for CT datasets),
     *        \c FLOAT, \c FLOAT8 and \c FLOAT16 (the last two will be converted to UCHAR and USHORT).
     * @param headerskip number of bytes to skip at the beginning of the raw file
     * @param bigEndian if set to true, the data is converted from big endian to little endian byte order
     */
    void setReadHints(tgt::ivec3 dimensions,
                      tgt::vec3 spacing,
                      int bitsStored,
                      const std::string& objectModel = "I",
                      const std::string& format = "UCHAR",
                      int headerskip = 0,
                      bool bigEndian = false);

    /**
     * Set hints about the volume dataset. Must be set before read() is called.
     */
    void setReadHints(const ReadHints& hints);

    virtual VolumeCollection* read(const std::string& url)
        throw(tgt::CorruptedFileException, tgt::IOException, std::bad_alloc);

    virtual VolumeCollection* readSlices(const std::string& url, size_t firstSlice=0, size_t lastSlice=0)
        throw(tgt::CorruptedFileException, tgt::IOException, std::bad_alloc);

    virtual VolumeCollection* readBrick(const std::string& url, tgt::ivec3 brickStartPos, int brickSize)
        throw(tgt::FileException, std::bad_alloc);

    /**
     * Extracts the parameters necessary for loading the raw volume from the passed Origin and loads it.
     */
    virtual VolumeHandle* read(const VolumeOrigin& origin)
        throw (tgt::FileException, std::bad_alloc);

    /**
     * Constructs a volume from a set of raw slice files by stacking them in z-direction.
     *
     * @note Read hints are applied for each slice.
     */
    virtual VolumeHandle* readSliceStack(const std::vector<std::string>& sliceFiles)
        throw(tgt::FileException, std::bad_alloc);

private:
    ReadHints hints_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_RAWVOLUMEREADER_H
