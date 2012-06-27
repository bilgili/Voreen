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

#ifndef VRN_VOLUMEMETADATA_H
#define VRN_VOLUMEMETADATA_H

#include <string>

#include "tgt/matrix.h"

namespace voreen {

/**
 * Encapsulates the meta data of a Volume object.
 */
class VolumeMetaData {
public:
    VolumeMetaData(const std::string& str = std::string());

    //
    // getters and setters
    //

    /// get meta data string
    const std::string& getString() const;
    /// set meta data string
    void setString(const std::string& str);

    void setImagePositionZ(float pos);
    float getImagePositionZ() const;

    void setBrickSize(size_t bricksize);
    size_t getBrickSize();

    void setParentVolumeDimensions(tgt::ivec3 dimensions);
    tgt::ivec3 getParentVolumeDimensions();

    void setUnit(const std::string& unit);
    const std::string& getUnit() const;

protected:
    std::string string_;
    float imagePositionZ_;

    /**
     * If the volume is a bricked volume, you can store the size of the bricks here.
     */
    size_t brickSize_;

    /**
     * If this volume is part of a bigger volume (for example if this volume
     * was created by createSubset, you can store the dimensions of the
     * bigger volume here). Uses in bricking for example.
     */
    tgt::ivec3 parentVolumeDimensions_;

    std::string unit_;

};

} // namespace voreen

#endif // VRN_VOLUMEMETADATA_H
