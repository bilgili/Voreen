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

#ifndef VRN_VOLUMESERIALIZERPOPULATOR_H
#define VRN_VOLUMESERIALIZERPOPULATOR_H

#include <vector>
#include <string>

namespace voreen {

// forward declarations
class IOProgress;
class VolumeReader;
class VolumeWriter;
class VolumeSerializer;

/**
 * This class creates a VolumeSerializer and registers all known VolumeReader
 * and VolumeWriter objects. You can get a pointer to the VolumeSerializer.
 * The VolumeSerializer will be destroyed when this class is deleted.
 */
class VolumeSerializerPopulator {
public:
    VolumeSerializerPopulator(bool showProgress = true);
    ~VolumeSerializerPopulator();

    /// get a pointer of the generated VolumeSerializer
    const VolumeSerializer* getVolumeSerializer() const;

    /**
     * Returns all file extensions that are supported by the
     * available volume readers.
     */
    std::vector<std::string> getSupportedReadExtensions() const;

    /**
     * Returns all file extensions that are supported by the
     * available volume writers.
     */
    std::vector<std::string> getSupportedWriteExtensions() const;

private:
    std::vector<VolumeReader*> readers_;
    std::vector<VolumeWriter*> writers_;
    VolumeSerializer* const vs_;
    IOProgress* progressDialog_;
};

} // namespace voreen

#endif // VRN_VOLUMESERIALIZERPOPULATOR_H
