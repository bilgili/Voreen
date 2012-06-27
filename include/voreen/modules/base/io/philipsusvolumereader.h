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

#ifndef VRN_PHILIPSUSVOLUMEREADER_H
#define VRN_PHILIPSUSVOLUMEREADER_H

#include "voreen/core/io/volumereader.h"

namespace voreen {

/**
 * Reader for Philips Sonos 7500 <tt>.dcm</tt> files. This reader is implemented based
 * on the example reader specified in the document 'Reading Philips 3D DICOM Files from
 * Sonos 7500 D.0'.
 */
class PhilipsUSVolumeReader : public VolumeReader {
public:
    PhilipsUSVolumeReader(IOProgress* progress = 0);

    virtual VolumeReader* create(IOProgress* progress = 0) const;

    virtual VolumeCollection* read(const std::string& url)
        throw (tgt::FileException, std::bad_alloc);

    virtual VolumeCollection* readSlices(const std::string& url, size_t firstSlice=0, size_t lastSlice=0)
        throw (tgt::FileException, std::bad_alloc);

private:
    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_PHILIPSUSVOLUMEREADER_H
