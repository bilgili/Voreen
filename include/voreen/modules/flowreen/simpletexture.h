#ifndef VRN_SIMPLETEXTURE_H
#define VRN_SIMPLETEXTURE_H

#include "tgt/vector.h"

namespace voreen {

template<typename T>
class SimpleTexture {
public:
    typedef T ElemType;

public:
    SimpleTexture(const tgt::ivec2& dimensions, const bool autoFree = true);
    SimpleTexture(const tgt::ivec3& dimensions, const bool autoFree = true);
    virtual ~SimpleTexture();

    virtual void free();

    const T* getData() const { return data_; }

    const tgt::ivec3 getDimensions() const { return dimensions_; }
    size_t getNumElements() const { return numElements_; }

    bool doesAutoFree() const { return autoFree_; }
    void setAutoFree(const bool autoFree) { autoFree_ = autoFree; }

    const T& operator[](size_t index) const;
    const T& operator[](const tgt::ivec2& position) const;
    const T& operator[](const tgt::ivec3& position) const;

    T& operator[](size_t index);
    T& operator[](const tgt::ivec2& position);
    T& operator[](const tgt::ivec3& position);

    void createWhiteNoise();

protected:
    size_t positionToIndex(const tgt::ivec2& position) const;
    size_t positionToIndex(const tgt::ivec3& position) const;

protected:
    const tgt::ivec3 dimensions_;
    const size_t numElements_;
    T* const data_;
    bool autoFree_;

private:
    SimpleTexture(const SimpleTexture&);
    SimpleTexture& operator=(const SimpleTexture&);
};

}   // namespace

#endif
