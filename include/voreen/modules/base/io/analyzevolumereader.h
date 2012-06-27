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

#ifndef VRN_ANALYZEVOLUMEREADER_H
#define VRN_ANALYZEVOLUMEREADER_H

#include "voreen/core/io/volumereader.h"

namespace voreen {

class AnalyzeVolumeReader : public VolumeReader {
public:
    AnalyzeVolumeReader(IOProgress* progress = 0);

    virtual VolumeReader* create(IOProgress* progress = 0) const;

    static std::string getRelatedImgFileName(const std::string& fileName);

    virtual VolumeCollection* read(const std::string& url)
        throw (tgt::FileException, std::bad_alloc);

private:
    virtual VolumeCollection* readAnalyze(const std::string& fileName)
        throw (tgt::FileException, std::bad_alloc);

    virtual VolumeCollection* readNifti(const std::string& fileName, bool standalone)
        throw (tgt::FileException, std::bad_alloc);

    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_ANALYZEVOLUMEREADER_H
