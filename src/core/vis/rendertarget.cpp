#include "voreen/core/vis/rendertarget.h"

#include "tgt/logmanager.h"
#include "tgt/gpucapabilities.h"

using tgt::Texture;

namespace voreen {

const std::string RenderTarget::loggerCat_ = "voreen.RenderTarget";

RenderTarget::RenderTarget()
    : fbo_(0),
    colorTex_(0),
    depthTex_(0),
    numUpdates_(0)
{
}

RenderTarget::~RenderTarget()
{
    if (fbo_) {
        delete fbo_;
        fbo_ = 0;
    }
    if (colorTex_) {
        delete colorTex_;
        colorTex_ = 0;
    }
    if (depthTex_)  {
        delete depthTex_;
        depthTex_ = 0;
    }
}

void RenderTarget::initialize(GLint internalColorFormat, GLint internalDepthFormat) {

    if (fbo_)
        return;

    if (!GpuCaps.isNpotSupported() && !GpuCaps.areTextureRectanglesSupported()) {
        LWARNING("Neither non-power-of-two textures nor texture rectangles seem to be supported!");
    }

    bool textureRectangles = !GpuCaps.isNpotSupported() && GpuCaps.areTextureRectanglesSupported();

    tgt::ivec3 size(128, 128, 1);

    switch(internalColorFormat) {
        case GL_RGB:
            colorTex_ = new Texture(0, size, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, Texture::LINEAR, textureRectangles);
            break;
        case GL_RGBA:
            colorTex_ = new Texture(0, size, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, Texture::LINEAR, textureRectangles);
            break;
        case GL_RGB16F_ARB:
            colorTex_ = new Texture(0, size, GL_RGB, GL_RGB16F_ARB, GL_FLOAT, Texture::LINEAR, textureRectangles);
            break;
        case GL_RGBA16F_ARB:
            colorTex_ = new Texture(0, size, GL_RGBA, GL_RGBA16F_ARB, GL_FLOAT, Texture::LINEAR, textureRectangles);
            break;
        default:
            LERRORC("voreen.RenderTarget", "Unknown internal format!");
    }
    colorTex_->uploadTexture();

    switch(internalDepthFormat) {
        case GL_DEPTH_COMPONENT16:
            depthTex_ = new Texture(0, size, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_FLOAT, Texture::LINEAR, textureRectangles);
            break;
        case GL_DEPTH_COMPONENT24:
            depthTex_ = new Texture(0, size, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, GL_FLOAT, Texture::LINEAR, textureRectangles);
            break;
        case GL_DEPTH_COMPONENT32:
            depthTex_ = new Texture(0, size, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32, GL_FLOAT, Texture::LINEAR, textureRectangles);
            break;
        default:
            LERRORC("voreen.RenderTarget", "Unknown internal depth format!");
    }
    depthTex_->uploadTexture();

    fbo_ = new tgt::FramebufferObject();
    if (!fbo_) {
        LERRORC("voreen.RenderTarget", "Failed to initialize framebuffer object!");
        return;
    }
    fbo_->activate();

    fbo_->attachTexture(colorTex_);
    fbo_->isComplete();

    fbo_->attachTexture(depthTex_, GL_DEPTH_ATTACHMENT_EXT);
    fbo_->isComplete();
}

void RenderTarget::activateTarget(const std::string& debugLabel) {
    if (!fbo_) {
        LWARNINGC("voreen.RenderTarget", "Using lazy RenderTarget initialization! This is deprecated and can cause hard to find bugs! FIX IT! (" << debugLabel << ")");
        initialize();
    }
    fbo_->activate();
    glViewport(0, 0, colorTex_->getWidth(), colorTex_->getHeight());
    debugLabel_ = debugLabel;
}

void RenderTarget::resize(tgt::ivec2 newsize) {
    if (!fbo_)
        return;

    // no change
    if (newsize == colorTex_->getDimensions().xy())
        return;

    // recreate fbo textures
    glActiveTexture(GL_TEXTURE0);
    colorTex_->destroy();
    colorTex_->setDimensions(tgt::ivec3(newsize.x, newsize.y, 1));
    colorTex_->uploadTexture();

    depthTex_->destroy();
    depthTex_->setDimensions(tgt::ivec3(newsize.x, newsize.y, 1));
    depthTex_->uploadTexture();

    //clearAllTargets();
}

void RenderTarget::bindColorTexture() {
    tgtAssert(colorTex_, "No color texture available!");
    if (colorTex_)
        colorTex_->bind();
}

void RenderTarget::bindColorTexture(GLint texUnit) {
    glActiveTexture(texUnit);
    bindColorTexture();
}

void RenderTarget::bindDepthTexture() {
    tgtAssert(depthTex_, "No depth texture available!");
    if(depthTex_)
        depthTex_->bind();
}

void RenderTarget::bindDepthTexture(GLint texUnit) {
    glActiveTexture(texUnit);
    bindDepthTexture();
}

tgt::ivec2 RenderTarget::getSize() const {
    if (colorTex_)
        return colorTex_->getDimensions().xy();
	else if (depthTex_)
        return depthTex_->getDimensions().xy();
    else
        return tgt::ivec2(0,0);
}

tgt::vec4 RenderTarget::getColorAtPos(tgt::ivec2 pos) {
    activateTarget();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    tgt::vec4 pixels;
    glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, GL_FLOAT, &pixels);
    return pixels;
}

}   // namespace
