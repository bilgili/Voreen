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

#include "voreen/core/volume/volumehandlevalidator.h"
#include "voreen/core/volume/volumehandle.h"

namespace voreen {

bool VolumeHandleValidator::checkVolumeHandle(VolumeHandle*& handle, VolumeHandle* const newHandle,
                                              bool* handleChanged, const bool omitVolumeCheck)
{
    if (handle != 0) {
        // If the given VolumeHandle* is not identical to the current one,
        // (that is especially if handle is 0!), set the given one as current.
        //
        if (!handle->isIdentical(newHandle)) {
            handle = newHandle;
        } else {
            if (handleChanged != 0)
                *handleChanged = false;
            return true;
        }
    } else
        handle = newHandle;

    if (handleChanged != 0)
        *handleChanged = true;

    // As the current handle might have changed above and it may be NULL, perform
    // the NULL-check a second time and return the result.
    //
    bool result = (handle != 0);
    if (result && !omitVolumeCheck)
        result = (handle->getVolume() != 0);
    return result;
};

} // namespace
