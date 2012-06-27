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

#ifndef VRN_RAWVOLUMEREADER_H
#define VRN_RAWVOLUMEREADER_H

#include <string>

#include "tgt/vector.h"
#include "tgt/matrix.h"

#include "voreen/core/io/volumereader.h"
#include "voreen/core/volume/modality.h"

namespace voreen {

class IOProgress;

/**
 * Reads a raw volume dataset. This requires information hints about dimension, format, etc.,
 * usually given by an accompanying <tt>.dat</tt>. DatVolumeReader reads this information and
 * uses RawVolumeReader to read the actual data.
 */
class RawVolumeReader : public VolumeReader {
public:
    RawVolumeReader(IOProgress* progress = 0);

    /**
     * Set hints about the volume dataset. Must be set before read() is called.
     *
     * @param dimensions number of voxels in x-, y- and z-direction
     * @param bitsStored number of bits stored in this dataset per voxel
     * @param spacing non-uniform voxel scaling
     * @param objectModel \c I (intensity) or \c RGBA
     * @param format voxel data format, one of \c UCHAR, \c USHORT, \c USHORT_12 (for CT datasets),
     *        \c FLOAT 8 and \c FLOAT16.
     * @param zeroPoint offset for the voxel data for supporting signed data
     * @param transformation 4x4-matrix for affine transformation of volume
     * @param metaString a string containing arbitrary meta-data
     * @param offset if the loading shouldn't start at the beginning but at an offset
     * @param unit a string containing the unit of the dataset
     */
    void readHints(tgt::ivec3 dimensions,
                   tgt::vec3 spacing,
                   int bitsStored,
                   const std::string objectModel = "RGBA",
                   const std::string format = "UCHAR",
                   int zeroPoint = 0,
                   tgt::mat4 transformation = tgt::mat4::identity,
                   Modality modality = Modality::MODALITY_UNKNOWN,
				   float timeStep_ = -1.0f,
                   std::string metaString = "",
                   std::string unit = "",
                   int offset = 0);

    virtual VolumeSet* read(const std::string& fileName)
        throw(tgt::CorruptedFileException, tgt::IOException, std::bad_alloc);

private:

    tgt::ivec3 dimensions_;
    int bitsStored_;
    tgt::vec3 spacing_;
    std::string objectModel_;
    std::string format_;
    int zeroPoint_;
    tgt::mat4 transformation_;
    Modality modality_;
    float timeStep_;
    std::string metaString_;
    int offset_;
    std::string unit_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_RAWVOLUMEREADER_H
