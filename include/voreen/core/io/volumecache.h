#ifndef VRN_VOLUMECACHE_H
#define VRN_VOLUMECACHE_H

#include "voreen/core/io/cache.h"

namespace voreen {

class VolumeHandle;

class VolumeCache : public Cache<VolumeHandle> {
public:
    VolumeCache();

    virtual ~VolumeCache() { /* dtor has nothing to do */ }

protected:
    virtual VolumeHandle* getPortData(const Identifier& portType, 
        LocalPortMapping* const localPortMapping) const;

    virtual VolumeHandle* loadObject(const std::string& directory, const std::string& filename) const;

    virtual std::string portContentToString(const Identifier& portID, 
        LocalPortMapping* const localPortMapping) const;

    virtual std::string saveObject(VolumeHandle* const object, const std::string& directory,
        const std::string& filename);
};

}   // namespace

#endif
