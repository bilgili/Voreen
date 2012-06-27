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

#ifndef VRN_ZIPVOLUMEREADER_H
#define VRN_ZIPVOLUMEREADER_H

#include <string>
#include <vector>

#include "voreen/core/io/volumereader.h"
#include "voreen/core/volume/modality.h"

#include "tgt/vector.h"

namespace voreen {

/**
 * Reads multiple volumes stored in a container <tt>.zip</tt>-file together with a description
 * file.
 *
 * The file <tt>description.txt</tt> in the <tt>.zip</tt>-file reference the volumes to be
 * loaded. For <tt>.raw</tt> datasets the information contained in the corresponding
 * <tt>.dat</tt> files is also stored in the description file.
 *
 */
class ZipVolumeReader : public VolumeReader {
public:
    ZipVolumeReader(IOProgress* progress = 0);

    virtual VolumeSet* read(const std::string& fileName, bool generateVolumeGL = true)
        throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc);

protected:
    bool parseDescFile(const std::string& filename);

    struct VolInfo {
        VolInfo(tgt::ivec3 resolution, tgt::vec3 sliceThickness,
                std::string format, std::string objectModel)
            : timeStamp_(0), resolution_(resolution), sliceThickness_(sliceThickness),
              format_(format), objectModel_(objectModel), modality_(Modality::MODALITY_UNKNOWN),
              metaString_("") {}
        
        std::string filename_;
        std::vector<std::string> extInfo_;
        unsigned int timeStamp_;
        tgt::ivec3 resolution_;
        tgt::vec3 sliceThickness_;
        std::string format_;
        std::string objectModel_;
        Modality modality_;
        std::string metaString_;
    };

    std::vector<VolInfo*> volInfos_;

private:
    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_ZIPVOLUMEREADER_H
