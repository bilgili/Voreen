/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#ifndef VRN_VOLUMEREADER_H
#define VRN_VOLUMEREADER_H

#include <string>
#include <vector>

#include "tgt/exception.h"

#ifndef VRN_VOLUMESET_H
#include "voreen/core/volume/volumeset.h"
#endif

namespace voreen {

// forward declarations
class IOProgress;
class Volume;

/**
 * Reads a volume dataset.
 * Implement this class in order to support a new format.
 */
class VolumeReader {
public:

    VolumeReader(IOProgress* progress = 0);
    virtual ~VolumeReader() {}

    /**
     * Loads a volume file and build a new Volume.
     *
     * @param fileName The file name of the data set which should be read
     * @param generateVolumeGL If this is true a VolumeGL will be created,
     *      has no effect in the VRN_NO_OPENGL-Version!
     * @return new VolumeContainer, the caller is responsible for freeing the memory
     */
    virtual VolumeSet* read(const std::string& fileName, bool generateVolumeGL = true)
        throw(tgt::CorruptedFileException, tgt::IOException, std::bad_alloc) = 0;

    const std::vector<std::string>& getExtensions() const;

protected:

    void read(Volume* volume, std::fstream& fin);

    std::vector<std::string> extensions_;
    std::string name_;
    IOProgress* progress_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_VOLUMEREADER_H
