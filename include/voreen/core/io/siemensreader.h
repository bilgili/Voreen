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

#ifndef VRN_SIEMENSREADER_H
#define VRN_SIEMENSREADER_H

#include "voreen/core/io/volumereader.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace voreen {

/**
 * Handles loading ultrasound image data from the GPU Gems book. The raw data is
 * acquired from a Siemens ultrasound machine.
 */
class SiemensReader : public VolumeReader {
public:
    SiemensReader();

    virtual VolumeCollection* read(const std::string& fileName)
        throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc);

private:
    void readInfoFile(const std::string& fname);

    /// splits the line \p s into its parts at the split point \p ch
    std::vector<std::string> splitLine(std::string s, const char* ch);

    /// delete all occurences of a string \p c in the string \p s
    std::string& deleteAll(std::string& s, const std::string& c);

    size_t datasetXRes_, datasetYRes_, datasetZRes_;
    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_SIEMENSREADER_H
