#ifndef VRN_RENDERTARGET_H
#define VRN_RENDERTARGET_H

namespace voreen {

class TextureContainer;

class RenderTarget {
public:
    struct TargetComparator {
        bool operator ()(const RenderTarget& rt1, const RenderTarget& rt2) const {
            return (rt1 > rt2);
        }
    };

public:
    RenderTarget(TextureContainer* const tc, const int textureID);
    RenderTarget(const RenderTarget& other);
    RenderTarget& operator =(const RenderTarget& other);
    bool operator >(const RenderTarget& other) const;

private:
    void copy(const RenderTarget& other);

public:
    TextureContainer* tc_;
    int textureID_;
    bool isReAssignable_;
};

}   // namespace

#endif  // VRN_RENDERTARGET_H
