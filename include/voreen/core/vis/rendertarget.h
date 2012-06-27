#ifndef VRN_RENDERTARGET_H
#define VRN_RENDERTARGET_H

#include "tgt/framebufferobject.h"

namespace voreen {

class RenderTarget {
public:
    RenderTarget();
    virtual ~RenderTarget();

    void initialize(GLint internalColorFormat = GL_RGBA16F_ARB, GLint internalDepthFormat = GL_DEPTH_COMPONENT24);

    void activateTarget(const std::string &debugLabel="");

    void bindColorTexture(GLint texUnit);
    void bindDepthTexture(GLint texUnit);

    void bindColorTexture();
    void bindDepthTexture();

    tgt::Texture* getColorTexture() { return colorTex_; }
    tgt::Texture* getDepthTexture() { return depthTex_; }

    //Get the color at position pos. This method activates the RenderTarget!
    tgt::vec4 getColorAtPos(tgt::ivec2 pos);

    tgt::ivec2 getSize() const;
    void resize(tgt::ivec2 newsize);

    //these are just for debugging purposes:
    std::string getDebugLabel() { return debugLabel_; }
    void setDebugLabel(const std::string& debugLabel) { debugLabel_ = debugLabel; }

    void increaseNumUpdates() { numUpdates_++; }
    int getNumUpdates() { return numUpdates_; }

protected:
    tgt::FramebufferObject* fbo_;

    tgt::Texture* colorTex_;
    tgt::Texture* depthTex_;

    std::string debugLabel_;
    int numUpdates_;

    static const std::string loggerCat_;
};

}   // namespace

#endif  // VRN_RENDERTARGET_H
