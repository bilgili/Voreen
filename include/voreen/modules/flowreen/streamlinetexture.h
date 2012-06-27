#ifndef VRN_STREAMLINETEXTURE_H
#define VRN_STREAMLINETEXTURE_H

#include "voreen/modules/flowreen/simpletexture.h"

namespace voreen {

class Flow2D;
class Flow3D;

template<typename T>
class StreamlineTexture : public SimpleTexture<T> {

using SimpleTexture<T>::data_;
using SimpleTexture<T>::autoFree_;
using SimpleTexture<T>::numElements_;
using SimpleTexture<T>::dimensions_;

public:
    struct StreamlineTextureElement {
        StreamlineTextureElement(T& elem, size_t& counter)
            : elem_(elem), counter_(counter)
        {}

        operator T() { return elem_; }

        T& elem_;
        size_t& counter_;
    };

public:
    StreamlineTexture(const tgt::ivec2& dimensions, const bool autoFree = true);
    StreamlineTexture(const tgt::ivec3& dimensions, const bool autoFree = true);
    virtual ~StreamlineTexture();

    virtual void free();

    const typename StreamlineTexture<T>::StreamlineTextureElement operator[](size_t index) const;
    const typename StreamlineTexture<T>::StreamlineTextureElement operator[](const tgt::ivec2& position) const;
    const typename StreamlineTexture<T>::StreamlineTextureElement operator[](const tgt::ivec3& position) const;

    typename StreamlineTexture<T>::StreamlineTextureElement operator[](size_t index);
    typename StreamlineTexture<T>::StreamlineTextureElement operator[](const tgt::ivec2& position);
    typename StreamlineTexture<T>::StreamlineTextureElement operator[](const tgt::ivec3& position);

    static T* integrateDraw(const Flow2D& flow, const size_t textureScaling,
        const size_t sampling, const tgt::vec2& thresholds = tgt::vec2(0.0f));

    static T* integrateDraw(const Flow3D& flow, const size_t textureScaling,
        const size_t sampling, const tgt::vec2& thresholds = tgt::vec2(0.0f));

    static T* fastLIC(const Flow2D& flow, const SimpleTexture<float>& inputTexture,
        const size_t textureScaling, const size_t sampling, const int maxKernelSize,
        const tgt::vec2& threshold = tgt::vec2(0.0f), const bool useAdaptiveKernelSize = false);

private:
    // prevent objects of this class from being copied
    //
    StreamlineTexture(const StreamlineTexture&);
    StreamlineTexture& operator=(const StreamlineTexture&);

    static float fastLICIntensity(const SimpleTexture<float>& inputTexture,
        const int indexR0, const int kernelSize, const std::vector<tgt::ivec2>& streamline);

private:
    size_t* const counter_;
};

}   // namespace

#endif
