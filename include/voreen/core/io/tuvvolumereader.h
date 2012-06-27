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

#ifndef VRN_TUVVOLUMEREADER_H
#define VRN_TUVVOLUMEREADER_H

#include "voreen/core/io/volumereader.h"

#include "tgt/vector.h"

namespace voreen {

/**
 * This is a reader for the dat files provided by TU Vienna.
 * To avoid confusion the file extension of the file to be read
 * has to be renamed from '.dat' to '.tuv'.
 *
 * @see http://www.cg.tuwien.ac.at/research/vis/datasets/
 */
class TUVVolumeReader : public VolumeReader {
public:
    TUVVolumeReader() {
         name_ = "TUV Reader";
         extensions_.push_back("tuv");
    }

    virtual VolumeSet* read(const std::string& fileName)
        throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc);

private:
    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_TUVVOLUMEREADER_H
