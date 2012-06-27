#ifdef VRN_MODULE_FLOWREEN

#ifndef VRN_FLOWREADER_H
#define VRN_FLOWREADER_H

#include "voreen/core/io/volumereader.h"

namespace voreen {

class VolumeFlow3D;     // make use of pimpl-idiom

class FlowReader : public VolumeReader {
public:
    FlowReader(IOProgress* const progress = 0);
    virtual ~FlowReader() {}

    virtual VolumeCollection* read(const std::string& fileName)
        throw(tgt::FileException, std::bad_alloc);

    virtual VolumeCollection* readSlices(const std::string& fileName, size_t/* firstSlice = 0*/, size_t /*lastSlice = 0*/)
        throw(tgt::FileException, std::bad_alloc) {
            return read(fileName);
    }

private:
    typedef unsigned char BYTE;

private:
    VolumeFlow3D* readConvert(const tgt::ivec3& dimensions,
        const BYTE orientation, std::fstream& ifs);
    void reverseSlices(VolumeFlow3D* const volume, const BYTE sliceOrder) const ;

private:
    static const std::string loggerCat_;
};

}   // namespace

#endif  // VRN_FLOWREADER_H
#endif  // VRN_MODULE_FLOWREEN
