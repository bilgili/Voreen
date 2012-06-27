#ifndef RAW2FLOWCONVERTER_H
#define RAW2FLOWCONVERTER_H

#include <string>
#include <vector>

class Raw2FlowConverter {
public:
    enum VoxelOrientation { XYZ = 1, XZY = 0, YXZ = 2, YZX = 3, ZXY = 4, ZYX = 5};

public:
    Raw2FlowConverter(const unsigned int dimensions = 3);
    ~Raw2FlowConverter() {}
    bool convert(const std::string& filename) const;
    bool setup(const std::string& sizes, const std::string& voxelOrientation = "xyz", const std::string& reverseSlices = "");

private:
    enum { MAGIC_NUMBER_SIZE = 11 };

private:
    std::vector<unsigned int> parseSizes(const std::string& input) const;
    char parseSliceReversal(const std::string& input) const;
    VoxelOrientation parseVoxelOrientation(const std::string& input) const;
    void writeHeader(std::ofstream& ofs) const;

private:
    VoxelOrientation voxelOrientation_;
    static char magicNumber_[MAGIC_NUMBER_SIZE];
    static int fileVersion_;
    unsigned int dimensions_;
    union {
        struct { unsigned int x_, y_, z_; };
        unsigned int sizes_[3];
    };
    unsigned int dataSize_;
    char sliceReversal_;
    bool setup_;
};

#endif
