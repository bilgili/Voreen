#include "voreen/core/io/volumecache.h"

#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/vis/processors/portmapping.h"
#include "voreen/core/volume/volumeset.h"

#include <time.h>

namespace voreen {

VolumeCache::VolumeCache() 
    : Cache<VolumeHandle>("VolumeHandle", "volumehandle")
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

VolumeHandle* VolumeCache::loadObject(const std::string& directory, const std::string& filename) const {
    VolumeSerializerPopulator populator;
    VolumeSerializer* serializer = populator.getVolumeSerializer();
    VolumeSet* volumeSet = serializer->load(directory + "/" + filename);
    if (volumeSet != 0) {
        std::vector<VolumeHandle*> volumeHandles = volumeSet->getAllVolumeHandles();
        if (volumeHandles.size() > 0)
            return volumeHandles[0];
    }
    return 0;
}

std::string VolumeCache::saveObject(VolumeHandle* const object, const std::string& directory,
                                    const std::string& filename)
{
    if ((object == 0) || (object->getVolume() == 0))
        return "";

    std::stringstream oss;
    oss << time(0) << "_" << rand() << ".dat";

    VolumeSerializerPopulator populator;
    VolumeSerializer* serializer = populator.getVolumeSerializer();
    if (filename.empty() == true) {
        serializer->save(directory + "/" + oss.str(), object->getVolume());
        return oss.str();
    }
    serializer->save(directory + "/" + filename, object->getVolume());
    return filename;
}

}   // namespace
