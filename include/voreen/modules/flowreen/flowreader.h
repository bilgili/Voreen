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

#ifdef VRN_MODULE_FLOWREEN

#ifndef VRN_FLOWREADER_H
#define VRN_FLOWREADER_H

#include "voreen/core/io/volumereader.h"

namespace voreen {

class VolumeFlow3D;     // make use of pimpl-idiom

class FlowReader : public VolumeReader {
public:
    FlowReader(ProgressBar* const progress = 0);
    virtual ~FlowReader() {}

    virtual VolumeReader* create(ProgressBar* progress = 0) const;

    virtual VolumeCollection* read(const std::string& url)
        throw(tgt::FileException, std::bad_alloc);

    virtual VolumeCollection* readSlices(const std::string& url, size_t/* firstSlice = 0*/, size_t /*lastSlice = 0*/)
        throw(tgt::FileException, std::bad_alloc) {
            return read(url);
    }

private:
    typedef unsigned char BYTE;

private:
    VolumeFlow3D* readConvert(const tgt::ivec3& dimensions,
        const BYTE orientation, std::fstream& ifs);
    void reverseSlices(VolumeFlow3D* const volume, const BYTE sliceOrder) const ;

private:
    static const std::string loggerCat_;
};

}   // namespace

#endif  // VRN_FLOWREADER_H
#endif  // VRN_MODULE_FLOWREEN
