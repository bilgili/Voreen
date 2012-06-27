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

    void setFileName(const std::string& fileName);
    const std::string& getFileName() const;

    void setImagePositionZ(float pos);
    float getImagePositionZ() const;

    void setTransformation(const tgt::mat4& trans);
    const tgt::mat4& getTransformation() const;

    void setSize(const tgt::ivec2& size);
    const tgt::ivec2& getSize() const;

    //
    // further methods
    //

    /// extracts the file name of a name given with full path
    static std::string getFileNameWithoutPath(const std::string& fullpath);

protected:
    std::string string_;
    std::string fileName_;
    float imagePositionZ_;
    tgt::mat4 transformation_;
    tgt::ivec2 size_;
};

} // namespace voreen

#endif // VRN_VOLUMEMETADATA_H
