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

#include "voreen/core/io/volumecache.h"

#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/vis/processors/portmapping.h"
#include "voreen/core/volume/volumeset.h"

#include <time.h>

namespace voreen {

VolumeCache::VolumeCache() 
    : Cache<VolumeHandle*>("VolumeHandle", "volumehandle")
{
}

VolumeHandle* VolumeCache::getPortData(const Identifier& portType, 
                                       LocalPortMapping* const localPortMapping) const
{
    if (portType.getSubString(0) == assignedPortType_)
        return localPortMapping->getVolumeHandle(portType);
    return 0;
}

std::string VolumeCache::portContentToString(const Identifier& portID, 
                                             LocalPortMapping* const localPortMapping) const
{
    const std::string type = portID.getSubString(0);
    if (type != assignedPortType_)
        return "";
    
    VolumeHandle* handle = localPortMapping->getVolumeHandle(portID);
    if (handle != 0) {
        const VolumeHandle::Origin& origin = handle->getOrigin();
        std::stringstream oss;
        oss << origin.filename << "(" << origin.timestep << ")";
        return oss.str();;
    }
    return "";
}

VolumeHandle* VolumeCache::loadObject(const std::string& filename) const {
    VolumeSerializerPopulator populator;
    VolumeSerializer* serializer = populator.getVolumeSerializer();
    try {
        VolumeSet* volumeSet = serializer->load(filename);
        if (volumeSet != 0) {
            std::vector<VolumeHandle*> volumeHandles = volumeSet->getAllVolumeHandles();
            if (volumeHandles.size() > 0)
                return volumeHandles[0];
        }
    } catch(tgt::FileNotFoundException&) {}
    return getInvalidValue();
}

std::string VolumeCache::saveObject(VolumeHandle* const object, const std::string& directory,
                                    const std::string& filename)
{
    if ((object == 0) || (object->getVolume() == 0))
        return "";

    VolumeSerializerPopulator populator;
    VolumeSerializer* serializer = populator.getVolumeSerializer();
    if (filename.empty() == true) {
        std::ostringstream oss;
        oss << time(0) << "_" << rand() << ".dat";
        serializer->save(directory + "/" + oss.str(), object->getVolume());
        return oss.str();
    }
    serializer->save(directory + "/" + filename, object->getVolume());
    return filename;
}

}   // namespace
