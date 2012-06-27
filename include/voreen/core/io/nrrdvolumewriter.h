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

#ifndef VRN_NRRDVOLUMEWRITER_H
#define VRN_NRRDVOLUMEWRITER_H

#include "voreen/core/io/volumewriter.h"

namespace voreen {

/**
 * Writer for <tt>.nrrd</tt> volume files (nearly raw raster data).
 * Writes the volume into a .nhdr and a .raw file.
 *
 * See http://teem.sourceforge.net/nrrd/ for details about the file format.
 */
class NrrdVolumeWriter : public VolumeWriter {
public:
    NrrdVolumeWriter();

    /**
     * Writes the data of a volume into a nrrd- and a raw-file.
     *
     * @param fileName File name to be written
     * @param volume Volume dataset
     */
    virtual void write(const std::string& filename, Volume* volume)
        throw (tgt::IOException);

private:
    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_NRRDVOLUMEWRITER_H
