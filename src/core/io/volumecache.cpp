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
#include "voreen/core/volume/volumecollection.h"

#include <time.h>

namespace voreen {

VolumeCache::VolumeCache()
    : Cache<VolumeHandle>("VolumeHandle", typeid(VolumePort))
{
}

//VolumeHandle* VolumeCache::getPortData(Port* const port) const
//{
    //if ((port == 0) || (port->getType() != assignedPortType_))
        //return 0;

    //VolumePort* vp = dynamic_cast<VolumePort*>(port);

    //if(vp)
        //return vp->getData();
    //else
        //return 0;
//}

std::string VolumeCache::portContentToString(Port* const port) const
{
    if ((port == 0) || (typeid(*port) != assignedPortType_))
        return "";

    VolumeHandle* handle = getPortData(port);
    if (handle != 0) {
        const VolumeOrigin& origin = handle->getOrigin();
        std::stringstream oss;
        oss << origin.getURL();
        return oss.str();;
    }
    return "";
}

VolumeHandle* VolumeCache::loadObject(const std::string& filename) const {

    VolumeHandle* result = 0;

    VolumeSerializerPopulator populator;
    const VolumeSerializer* serializer = populator.getVolumeSerializer();
    try {
        VolumeCollection* volumeCollection = serializer->load(filename);
        if (volumeCollection && !volumeCollection->empty()) {
            result = volumeCollection->first();
        }
    } catch(tgt::FileNotFoundException&) {}
    return 0;
}

std::string VolumeCache::saveObject(VolumeHandle* object, const std::string& directory,
                                    const std::string& filename)
{
    if ((object == 0) || (object->getVolume() == 0))
        return "";

    VolumeSerializerPopulator populator;
    const VolumeSerializer* serializer = populator.getVolumeSerializer();
    if (filename.empty() == true) {
        std::ostringstream oss;
        oss << time(0) << "_" << rand() << ".dat";
        serializer->save(directory + "/" + oss.str(), object);
        return oss.str();
    }
    serializer->save(directory + "/" + filename, object);
    return filename;
}

}   // namespace
