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

#ifndef VRN_VOLUMEPROCESSOR_H
#define VRN_VOLUMEPROCESSOR_H

#include "voreen/core/processors/processor.h"
#include "voreen/core/ports/volumeport.h"

namespace voreen {

/**
 * Base class for all processors operating on volumes.
 */
class VolumeProcessor : public Processor {
public:
    VolumeProcessor();
    virtual ~VolumeProcessor();

protected:
    /**
     * Computes the matrix necessary to map a vector from the originVolume to its counterpart in
     * the destinationVolume
     */
    tgt::mat4 computeConversionMatrix(const Volume* originVolume, const Volume* destinationVolume) const;
};

}   //namespace

#endif  //VRN_VOLUMEPROCESSOR_H
