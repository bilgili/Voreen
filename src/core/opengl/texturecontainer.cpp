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

#include "voreen/core/opengl/texturecontainer.h"

#include "tgt/logmanager.h"
#include "tgt/texturemanager.h"
#include "tgt/gpucapabilities.h"

#ifdef VRN_WITH_FBO_CLASS
#include <fboClass/framebufferObject.h>
#include <fboClass/renderbuffer.h>
#endif

#ifdef VRN_WITH_RENDER_TO_TEXTURE
#include <rendertotexture/rendertexture.h>
#endif

#include <sstream>

using tgt::ivec2;
using tgt::GpuCapabilities;

namespace voreen {

TextureContainer::RenderTarget::RenderTarget()
  : attr_(VRN_NONE)
  , debugLabel_("")
#ifdef VRN_WITH_FBO_CLASS
     , tex_(0), depthTex_(0), rbDepth_(0), rbStencil_(0)
#endif
#ifdef VRN_WITH_RENDER_TO_TEXTURE
     , rt_(0)
#endif
{}

TextureContainer::RenderTarget::~RenderTarget() {
#ifdef VRN_WITH_RENDER_TO_TEXTURE
    delete rt_;
#endif
}

TextureContainer* TextureContainer::activeTc_ = 0;
const std::string TextureContainer::loggerCat_("voreen.opengl.TextureContainer");


TextureContainer::TextureContainer(int numRT, bool sharing /*=false*/)
    : rt_(0), capacity_(0),  capacityIncr_(4), used_(0)
    , sharing_(sharing), size_(2, 2)
    , current_(std::vector<int>())
    , textureTargetType_(VRN_TEXTURE_2D)
    , isOpenGLInitialized_(false)
{
    setCapacity(numRT);
}

TextureContainer::~TextureContainer() {
    for (int i = 0; i < used_; ++i) {
        #ifdef VRN_WITH_FBO_CLASS
            glDeleteTextures(1, &rt_->tex_);
            glDeleteTextures(1, &rt_->depthTex_);
            delete rt_->rbDepth_;
            delete rt_->rbStencil_;
        #endif
    }
    delete[] rt_;
}

TextureContainer* TextureContainer::createTextureContainer(int numRT, bool sharing, TextureContainerType type) {
    TextureContainer* tc = 0;
    switch(type) {
    case VRN_TEXTURE_CONTAINER_AUTO:
        if (GpuCaps.areFramebufferObjectsSupported()) {
#ifdef VRN_WITH_FBO_CLASS
            LINFO("Using framebuffer objects");
            tc = new TextureContainerFBO(numRT, sharing);
#else
            LINFO("Hardware supports frame buffer objects, but Voreen was compiled without fbo support.");
#endif
        }
        if (tc == 0) {
#ifdef VRN_WITH_RENDER_TO_TEXTURE
            LINFO("Using render to texture");
            tc = new TextureContainerRTT(numRT, sharing);
#endif
        }
        if (tc == 0) {
            LERROR("Unable to create texture container");
        }
        break;

    case VRN_TEXTURE_CONTAINER_FBO:
#ifdef VRN_WITH_FBO_CLASS
        LINFO("Using framebuffer objects");
        tc = new TextureContainerFBO(numRT, sharing);
#else
        LERROR("Voreen was compiled without support for framebuffer objects.");
#endif
        break;

    case VRN_TEXTURE_CONTAINER_RTT:
#ifdef VRN_WITH_RENDER_TO_TEXTURE
        LINFO("Using render to texture");
        tc = new TextureContainerRTT(numRT, sharing);
#else
        LERROR("Voreen was compiled without support for render to texture.");
#endif
        break;
    }
    return tc;
}

void TextureContainer::initializeTarget(int id, int attr) {
    if (id >= capacity_) {
        setCapacity(id + capacityIncr_);
    }
    if (id >= used_) {
        used_ = id + 1;
    }
    if (!isOpenGLInitialized_) {
        rt_[id].attr_ = attr;
    }
    else {
        rt_[id].attr_ = attr;
        if (attr & VRN_FRAMEBUFFER) {
            LDEBUG("Initialize RenderTarget " << id << " as framebuffer target.");
            if (attr != VRN_FRAMEBUFFER) {
                LWARNING("VRN_FRAMEBUFFER should be used exclusively");
            }
            rt_[id].free_ = false;
        }
    }
}

bool TextureContainer::initializeGL() {
    isOpenGLInitialized_ = true;
    return true;
}

void TextureContainer::setSize(const tgt::ivec2& size) {
    if (size != size_)
        size_ = size;
}

float* TextureContainer::getTargetAsFloats(int id) {
    pushActiveTarget();
    LGL_ERROR;
    setActiveTarget(id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    float* pixels = new float[hmul(size_)*4];
    glReadPixels(0, 0, size_.x, size_.y, GL_RGBA, GL_FLOAT, pixels);
    LGL_ERROR;
    popActiveTarget();
    return pixels;
}

float* TextureContainer::getTargetAsFloats(int id, int x, int y) {
    pushActiveTarget();
    LGL_ERROR;
    setActiveTarget(id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    float* pixels = new float[4];
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, pixels);
    LGL_ERROR;
    popActiveTarget();
    return pixels;
}

float* TextureContainer::getDepthTargetAsFloats(int id) {
    pushActiveTarget();
    setActiveTarget(id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    float* pixels = new float[hmul(size_)];
    glReadPixels(0, 0, size_.x, size_.y, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);
    LGL_ERROR;
    popActiveTarget();
    return pixels;
}

float TextureContainer::getDepthTargetAsFloats(int id, int x, int y) {
    pushActiveTarget();
    setActiveTarget(id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    float pixel;
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pixel);
    LGL_ERROR;
    popActiveTarget();
    return pixel;
}

int TextureContainer::adaptToGraphicsBoard(int attr) {
    return attr;
}

void TextureContainer::setCapacityIncr(int incr) {
    capacityIncr_ = incr;
}

int TextureContainer::getCapacity() const {
    return capacity_;
}

int TextureContainer::getNumAvailable() {
    return used_;
}

int TextureContainer::getAttr(int id) {
    return rt_[id].attr_;
}

int TextureContainer::getActiveTarget() {
    if (current_.size() > 0)
        return current_[0];
    else
        return -1;
}

TextureContainer::TextureTarget TextureContainer::getTextureContainerTextureType() {
    return textureTargetType_;
}

int TextureContainer::getCurrentMemorySize() {
    int res = static_cast<int>(hmul(size_));
    int memSize = 0;
    for (int i = 0; i < capacity_; ++i) {
        int attr = rt_[i].attr_;
        // TODO: complete this list. what is float16? (d.f.)

        //FIXME: this is incorrect. RGBA uses unsigned char per component, RGBA_FLOAT16 use a
        //       half-float (2 bytes) per component. joerg
        if (attr & VRN_RGBA)
            memSize += res * 4 * sizeof(float);

        if (attr & VRN_RGB)
            memSize += res * 3 * sizeof(float);

        if (attr & VRN_DEPTH)
            memSize += res * sizeof(float);

        if (attr & VRN_DEPTH16)
            memSize += res * 2;

        if (attr & VRN_DEPTH32)
            memSize += res * 4;
    }
    return memSize;
}

void TextureContainer::setCapacity(int capacity) {
    if (capacity != capacity_) {
        LDEBUG("change capacity from " << capacity_ << " to " << capacity);
        RenderTarget *rtNew = new RenderTarget[capacity];
        if (!rtNew)
            LERROR("failed while reserving memory.");

        int itemsToCopy = (capacity_ < capacity ? capacity_ : capacity);
        for (int i=0; i < itemsToCopy; ++i)
            rtNew[i] = rt_[i];

        delete[] rt_;
        rt_ = rtNew;
        capacity_ = capacity;
    }
}

void TextureContainer::clearAllTargets(tgt::Color clearColor, float depth) {
    LDEBUG("Clear all targets with color " << clearColor << " and depth " << depth << ".");
    int storeActiveTarget;

    storeActiveTarget = getActiveTarget();
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClearDepth(depth);
    for (int i=0; i<capacity_; ++i) {
        if (!(rt_[i].attr_&VRN_FRAMEBUFFER_CONSTS_MASK)) {
            setActiveTarget(i);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        }
    }
    setActiveTarget(storeActiveTarget);
    glClearDepth(1.0);
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

tgt::ivec2 TextureContainer::getSize() {
    return size_;
}

std::string TextureContainer::getTypeAsString(int type) const {
    std::string str;
    switch (type&VRN_COLOR_CONSTS_MASK) {
    case VRN_RGB:
        str = "VRN_RGB";
        break;
    case VRN_RGBA:
        str = "VRN_RGBA";
        break;
    case VRN_RGB_FLOAT16:
        str = "VRN_RGB_FLOAT16";
        break;
    case VRN_RGBA_FLOAT16:
        str = "VRN_RGBA_FLOAT16";
        break;
    case VRN_DEPTH:
        str = "VRN_DEPTH";
        break;
    case VRN_DEPTH_STENCIL:
        str = "VRN_DEPTH_STENCIL";
        break;
    case VRN_FRAMEBUFFER:
        str = "VRN_FRAMEBUFFER";
        break;
    }

    switch (type&VRN_DEPTH_CONSTS_MASK) {
    case VRN_DEPTH:
        str += "VRN_DEPTH";
        break;
    case VRN_DEPTH_STENCIL:
        str += "VRN_DEPTH_STENCIL";
        break;
    }

    if ((type&VRN_DEPTH_TEX_CONSTS_MASK))
        str += "(depth tex)";
    if ((type&VRN_TEX_RECT_CONSTS_MASK))
        str += "(tex rect)";
    if ((type&VRN_CUBE_MAP_CONSTS_MASK))
        str += "(cube map)";
    return str;
}

void TextureContainer::setFinalTarget(int id) {
    finalTarget_ = id;
}

int TextureContainer::getFinalTarget() {
    return finalTarget_;
}

void TextureContainer::pushActiveTarget() {
    currentRTStack_.push(getActiveTarget());
}

void TextureContainer::popActiveTarget() {
    if (!currentRTStack_.empty()) {
        setActiveTarget(currentRTStack_.top());
        currentRTStack_.pop();
    } else {
        LERROR("popActiveTarget(): Stack is empty.");
    }
}

void TextureContainer::setDebugLabel(int id, const std::string& s) {
    static int i = 0;
    std::ostringstream o;
    o << s << " (@" << i << ")";
    rt_[id].debugLabel_ = o.str();
    ++i;
}

std::string TextureContainer::getDebugLabel(int id) const {
    if (id < capacity_)
        return rt_[id].debugLabel_;
    else
        return "not a valid id";
}

//---------------------------------------------------------------------------

#ifdef VRN_WITH_FBO_CLASS
#ifdef VRN_DEBUG
    #define LFBO_ERROR                                  \
        {                                               \
            if (isFBOActive_) {                         \
                std::string err = getFBOError();        \
                if (err != "") {                        \
                    LERROR("FBO error: " << err);       \
                }                                       \
            }                                           \
        }
#else
    #define LFBO_ERROR
#endif

std::string TextureContainerFBO::getFBOError() {
    GLenum status;
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch (status) {
    case GL_FRAMEBUFFER_COMPLETE_EXT: // Everything's OK
        return "";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT\n";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT\n";
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        return "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT\n";
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        return "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT\n";
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        return "GL_FRAMEBUFFER_UNSUPPORTED_EXT\n";
    default:
        return "Unknown ERROR\n";
    }
}

const std::string TextureContainerFBO::loggerCat_("voreen.opengl.TextureContainerFBO");

TextureContainerFBO::TextureContainerFBO(int numRT, bool sharing/*= false*/)
    : TextureContainer(numRT, sharing),
      isFBOActive_(false),
      fbo_(0)
{}

TextureContainerFBO::~TextureContainerFBO() {
    delete fbo_;
}

TextureContainer::TextureContainerType TextureContainerFBO::getTextureContainerType() {
    return VRN_TEXTURE_CONTAINER_FBO;
}

bool TextureContainerFBO::initializeGL() {
    if (!TextureContainer::initializeGL())
        return false;

    if (!GpuCaps.areFramebufferObjectsSupported()) {
        LERROR("Framebuffer object extension not supported!");
        return false;
    }

    if (textureTargetType_ == VRN_TEXTURE_2D && !GpuCaps.isNpotSupported()) {
        if (GpuCaps.areTextureRectanglesSupported()){
            LINFO("Non-power-of-two textures not supported. Using texture rectangles instead.");
            textureTargetType_ = VRN_TEXTURE_RECTANGLE;
        } else {
            LERROR("Neither non-power-of-two textures nor texture rectangles supported!");
            return false;
        }
    }

    switch (textureTargetType_) {
    case VRN_TEXTURE_2D :
        LINFO("Type of render targets: GL_TEXTURE_2D");
        break;
    case VRN_TEXTURE_RECTANGLE :
        LINFO("Type of render targets: GL_TEXTURE_RECTANGLE_ARB");
        break;
    case VRN_TEXTURE_RESIZED_POT :
        LINFO("Type of render targets: RESIZED_POT");
        break;
    }

    delete fbo_;
    fbo_ = new FramebufferObject();
    if (!fbo_) {
        LERROR("Failed to initialize framebuffer object!");
        return false;
    }
    return true;
}

void TextureContainerFBO::initializeTarget(int id, int attr) {
    attr = adaptToGraphicsBoard(attr);
    TextureContainer::initializeTarget(id, attr);
    if (isOpenGLInitialized_) {
        if (attr & VRN_COLOR_CONSTS_MASK) {
            if (!rt_[id].tex_)
                glGenTextures(1, &rt_[id].tex_);

            GLenum texTarget = getGLTexTarget(id);
            glBindTexture(texTarget, rt_[id].tex_);
            glTexParameteri(texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(texTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(texTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);
            createColorTexture(id);
            //setActiveTarget(id, "Initialized");
            //glClear(GL_COLOR_BUFFER_BIT);
        }
        if (attr & VRN_DEPTH_CONSTS_MASK) {
            if (attr & VRN_DEPTH_TEX_CONSTS_MASK) {
                if (!rt_[id].depthTex_)
                    glGenTextures(1, &rt_[id].depthTex_);

                glBindTexture(getGLDepthTexTarget(id), rt_[id].depthTex_);
            }
            createDepthComponent(id);
        }
        LGL_ERROR;
    }
}

void TextureContainerFBO::setSize(const tgt::ivec2& size) {
    if (size != size_) {
        TextureContainer::setSize(size);

        for (int i=0; i<capacity_; ++i) {
            int attr = rt_[i].attr_;
            if (attr & VRN_COLOR_CONSTS_MASK) {
                glBindTexture(getGLTexTarget(i), rt_[i].tex_);
                createColorTexture(i);
            }
            if (attr & VRN_DEPTH_CONSTS_MASK) {
                if (attr & VRN_DEPTH_TEX_CONSTS_MASK)
                    glBindTexture(getGLDepthTexTarget(i), rt_[i].depthTex_);
                createDepthComponent(i);
            }
        }
    }
    if (isOpenGLInitialized_)
        clearAllTargets();
}

void TextureContainerFBO::setActiveTarget(int id, const std::string& debugLabel/*= ""*/,
                                          CubemapOrientation cubemapOrientation /*=VRN_NONE*/) {
    if (id < 0)
        return;

    LFBO_ERROR;
    LGL_ERROR;

    if (sharing_) {
        TextureContainerFBO* activeTc = dynamic_cast<TextureContainerFBO*>(activeTc_);
        if (activeTc && (activeTc != this)) {
            activeTc->unattach(activeTc->current_);
            LGL_ERROR;
            activeTc->current_.clear();
            activeTc->deactivateFBO();
            LGL_ERROR;
            activeTc_ = this;
        }
    }
    if (debugLabel != "")
        setDebugLabel(id, debugLabel);

    LFBO_ERROR
    LGL_ERROR;


    if (current_.size() == 1 && current_[0] == id) {
        LDEBUG("rt " << id << " already selected.");
        return;
    }
    unattach(current_);

    current_.clear();
    current_.push_back(id);

    int attr = rt_[id].attr_;
    if (attr & VRN_COLOR_CONSTS_MASK) {
        if (!isFBOActive_) {
            activateFBO();
        }
        LDEBUG("Attach rt " << id <<" as color texture.");

        fbo_->AttachTexture(getGLTexTarget(id, cubemapOrientation), rt_[id].tex_,
                            GL_COLOR_ATTACHMENT0_EXT);
        LFBO_ERROR
        LGL_ERROR;

        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

        LFBO_ERROR
        LGL_ERROR;

    }
    if (attr & VRN_DEPTH_CONSTS_MASK) {
        if (!isFBOActive_)
            activateFBO();
        if (attr & VRN_DEPTH_TEX_CONSTS_MASK) {
            if ((attr & VRN_DEPTH_STENCIL) == VRN_DEPTH_STENCIL) {
                LDEBUG("Attach rt " << id <<" as depth&stencil texture.");
                fbo_->AttachTexture(getGLDepthTexTarget(id), rt_[id].depthTex_,
                                    GL_DEPTH_ATTACHMENT_EXT);
                fbo_->AttachTexture(getGLDepthTexTarget(id), rt_[id].depthTex_,
                                    GL_STENCIL_ATTACHMENT_EXT);
            }
            else {
                LDEBUG("Attach rt " << id <<" as depth texture.");
                fbo_->AttachTexture(getGLDepthTexTarget(id),    rt_[id].depthTex_,
                                    GL_DEPTH_ATTACHMENT_EXT);
            }
        }
        else {
            if ((attr & VRN_DEPTH_STENCIL) == VRN_DEPTH_STENCIL) {
                LDEBUG("Attach rt " << id <<" as depth&stencil render buffer.");
                fbo_->AttachRenderBuffer(rt_[id].rbDepth_->GetId(),
                                         GL_STENCIL_ATTACHMENT_EXT);
                fbo_->AttachRenderBuffer(rt_[id].rbDepth_->GetId(),
                                         GL_DEPTH_ATTACHMENT_EXT);
            }
            else {
                LDEBUG("Attach rt " << id <<" as depth render buffer.");
                fbo_->AttachRenderBuffer(rt_[id].rbDepth_->GetId(),
                                         GL_DEPTH_ATTACHMENT_EXT);
            }
        }
        LFBO_ERROR
        LGL_ERROR;
    }
    if (attr & VRN_FRAMEBUFFER_CONSTS_MASK) {
        LDEBUG("Attach rt " << id <<" as framebuffer.");
        if (isFBOActive_)
            deactivateFBO();
        LGL_ERROR;
    }
}

void TextureContainerFBO::setActiveTargets(const std::vector<int>& targets, const std::string& debugLabel) {
    int id = targets[0];
    LGL_ERROR;
    if (sharing_) {
        TextureContainerFBO* activeTc = dynamic_cast<TextureContainerFBO*>(activeTc_);
        if (activeTc && (activeTc != this)) {
            activeTc->unattach(activeTc->current_);
            LGL_ERROR;
            activeTc->current_.clear();
            activeTc->deactivateFBO();
            LGL_ERROR;
        }
        activeTc_ = this;
    }
    int attr = rt_[id].attr_;
    LGL_ERROR;

    unattach(current_);

    current_.clear();
    for (size_t i=0; i<targets.size(); i++) {
        current_.push_back(targets[i]);
        rt_[targets[i]].free_ = false;
        if (debugLabel != "")
            setDebugLabel(targets[i], debugLabel);
    }
    if (attr & VRN_COLOR_CONSTS_MASK) {
        if (!isFBOActive_) {
            activateFBO();
        }
        LDEBUG("Attach rt " << id <<" as color texture.");

        GLenum* buffers = new GLenum[targets.size()];

        for (size_t i=0; i < targets.size();++i) {
            fbo_->AttachTexture(getGLTexTarget(targets[i]), rt_[targets[i]].tex_,
                GL_COLOR_ATTACHMENT0_EXT+i);
            LGL_ERROR;
            buffers[i] = GL_COLOR_ATTACHMENT0_EXT+i;
        }

        glDrawBuffers(targets.size(), buffers);
        LGL_ERROR;
        delete[] buffers;

        LFBO_ERROR
        LGL_ERROR;

    }
    if (attr & VRN_DEPTH_CONSTS_MASK) {
        if (!isFBOActive_) activateFBO();
        if (attr & VRN_DEPTH_TEX_CONSTS_MASK) {
            if ((attr & VRN_DEPTH_STENCIL) == VRN_DEPTH_STENCIL) {
                LDEBUG("Attach rt " << id <<" as depth&stencil texture.");
                fbo_->AttachTexture(getGLDepthTexTarget(id), rt_[id].depthTex_,
                                    GL_DEPTH_ATTACHMENT_EXT);
                fbo_->AttachTexture(getGLDepthTexTarget(id), rt_[id].depthTex_,
                                    GL_STENCIL_ATTACHMENT_EXT);
            }
            else {
                LDEBUG("Attach rt " << id <<" as depth texture.");
                fbo_->AttachTexture(getGLDepthTexTarget(id),    rt_[id].depthTex_,
                                    GL_DEPTH_ATTACHMENT_EXT);
            }
        }
        else {
            if ((attr & VRN_DEPTH_STENCIL) == VRN_DEPTH_STENCIL) {
                LDEBUG("Attach rt " << id <<" as depth&stencil render buffer.");
                fbo_->AttachRenderBuffer(rt_[id].rbDepth_->GetId(),
                                         GL_STENCIL_ATTACHMENT_EXT);
                fbo_->AttachRenderBuffer(rt_[id].rbDepth_->GetId(),
                                         GL_DEPTH_ATTACHMENT_EXT);
            }
            else {
                LDEBUG("Attach rt " << id <<" as depth render buffer.");
                fbo_->AttachRenderBuffer(rt_[id].rbDepth_->GetId(),
                                         GL_DEPTH_ATTACHMENT_EXT);
            }
        }
        LFBO_ERROR
        LGL_ERROR;
    }
    if (attr & VRN_FRAMEBUFFER_CONSTS_MASK) {
        LDEBUG("Attach rt " << id <<" as framebuffer.");
        if (isFBOActive_)
            deactivateFBO();
        LGL_ERROR;
    }
}

GLuint TextureContainerFBO::getGLTexID(int id) {
    return rt_[id].tex_;
}

GLuint TextureContainerFBO::getGLDepthTexID(int id) {
    return rt_[id].depthTex_;
}

GLenum TextureContainerFBO::getGLTexTarget(int id, CubemapOrientation cubemapOrientation/* = VRN_NONE*/) {
    int attr = rt_[id].attr_;
    if ((attr & VRN_CUBE_MAP_CONSTS_MASK)) {
        switch(cubemapOrientation) {
        case VRN_NONE:
            return GL_TEXTURE_CUBE_MAP;
        case VRN_CUBE_MAP_POSITIVE_X:
            return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        case VRN_CUBE_MAP_NEGATIVE_X:
            return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        case VRN_CUBE_MAP_POSITIVE_Y:
            return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        case VRN_CUBE_MAP_NEGATIVE_Y:
            return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        case VRN_CUBE_MAP_POSITIVE_Z:
            return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
        case VRN_CUBE_MAP_NEGATIVE_Z:
            return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
        default:
            LERROR("getGLTexTarget: Invalid cube map texture type.");
            return GL_NONE;
        }
    }

    if (attr & VRN_TEX_RECT_CONSTS_MASK)
        return GL_TEXTURE_RECTANGLE_ARB;
    else
        return GL_TEXTURE_2D;
}

GLenum TextureContainerFBO::getGLDepthTexTarget(int id) {
    int attr = rt_[id].attr_;
    if ((attr & VRN_DEPTH_TEX) == VRN_DEPTH_TEX) {
        if (attr & VRN_TEX_RECT_CONSTS_MASK)
            return GL_TEXTURE_RECTANGLE_ARB;
        else
            return GL_TEXTURE_2D;
    }
    LWARNING("getGLDepthTexTarget: Attempted to get texture target for a renderbuffer.");
    return GL_NONE;
}

void TextureContainerFBO::createColorTexture(int id) {
    int attr = rt_[id].attr_;
    GLenum texTarget = getGLTexTarget(id);
    LDEBUG("Create " << getTypeAsString(attr) <<
                    " texture for rt " << id << " with " << size_.x << " x " << size_.y);
    if ((attr & VRN_CUBE_MAP_CONSTS_MASK)) {
        if (size_.x != size_.y)
            LERROR("Cube maps have to be quadratic.");
        if ((attr & VRN_COLOR_CONSTS_MASK) != VRN_RGBA)
            LERROR("This cube map type is not implemented yet.");
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, size_.x,
                     size_.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, size_.x,
                     size_.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, size_.x,
                     size_.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, size_.x,
                     size_.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, size_.x,
                     size_.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, size_.x,
                     size_.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }
    else {
        switch (attr & VRN_COLOR_CONSTS_MASK) {
        case VRN_RGB:
            glTexImage2D(texTarget, 0, GL_RGB, size_.x, size_.y, 0, GL_RGB,
                         GL_UNSIGNED_BYTE, 0);
            break;
        case VRN_RGBA:
            glTexImage2D(texTarget, 0, GL_RGBA, size_.x, size_.y, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, 0);
            break;
        case VRN_RGB_FLOAT16:
            glTexImage2D(texTarget, 0, GL_RGB16F_ARB, size_.x, size_.y,
                         0, GL_RGB, GL_FLOAT, 0);
            break;
        case VRN_RGBA_FLOAT16:
            glTexImage2D(texTarget, 0, GL_RGBA16F_ARB, size_.x, size_.y,
                         0, GL_RGBA, GL_FLOAT, 0);
            break;
        default:
            LERROR("Reached default branch in CreateTexture.");
        }
    }
    //setActiveTarget(id);
    //glClear(GL_COLOR_BUFFER_BIT);
}

void TextureContainerFBO::createDepthComponent(int id) {
    int attr = rt_[id].attr_;
    if (attr & VRN_DEPTH_TEX_CONSTS_MASK) {
        switch (attr & VRN_DEPTH_CONSTS_MASK) {
        case VRN_DEPTH16:
            LDEBUG("Create VRN_DEPTH16 texture for rt " << id <<
                            " with " << size_.x << " x " << size_.y);
            glTexImage2D(getGLDepthTexTarget(id), 0, GL_DEPTH_COMPONENT16, size_.x, size_.y,
                         0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameterf(getGLDepthTexTarget(id), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameterf(getGLDepthTexTarget(id), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case VRN_DEPTH24:
            LDEBUG("Create VRN_DEPTH24 texture for rt " << id <<
                            " with " << size_.x << " x " << size_.y);
            glTexImage2D(getGLDepthTexTarget(id), 0, GL_DEPTH_COMPONENT24, size_.x, size_.y,
                         0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameterf(getGLDepthTexTarget(id), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameterf(getGLDepthTexTarget(id), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case VRN_DEPTH32:
            LDEBUG("Create VRN_DEPTH32 texture for rt " << id <<
                            " with " << size_.x << " x " << size_.y);
            glTexImage2D(getGLDepthTexTarget(id), 0, GL_DEPTH_COMPONENT32, size_.x, size_.y,
                         0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameterf(getGLDepthTexTarget(id), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameterf(getGLDepthTexTarget(id), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case VRN_DEPTH_STENCIL:
            LDEBUG("Create VRN_DEPTH_STENCIL texture for rt " << id <<
                            " with " << size_.x << " x " << size_.y);
            glTexImage2D(getGLDepthTexTarget(id), 0, GL_DEPTH24_STENCIL8_EXT,
                         size_.x, size_.y, 0, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT, 0);
            break;
        default: LERROR("Reached default branch in "
                                 "CreateDepthComponent.");
        }
        LGL_ERROR;
    }
    else {
        if (!rt_[id].rbDepth_)
            rt_[id].rbDepth_ = new Renderbuffer();
        switch(attr & VRN_DEPTH_CONSTS_MASK) {
        case VRN_DEPTH16:
            LDEBUG("Create VRN_DEPTH16 Renderbuffer for rt " << id <<
                   " with " << size_.x << " x " << size_.y);
            rt_[id].rbDepth_->Set(GL_DEPTH_COMPONENT16, size_.x, size_.y);
            break;
        case VRN_DEPTH24:
            LDEBUG("Create VRN_DEPTH24 Renderbuffer for rt " << id <<
                   " with " << size_.x << " x " << size_.y);
            rt_[id].rbDepth_->Set(GL_DEPTH_COMPONENT24, size_.x, size_.y);
            break;
        case VRN_DEPTH32:
            LDEBUG("Create VRN_DEPTH32 Renderbuffer for rt " << id <<
                   " with " << size_.x << " x " << size_.y);
            rt_[id].rbDepth_->Set(GL_DEPTH_COMPONENT32, size_.x, size_.y);
            break;
        case VRN_DEPTH_STENCIL:
            LDEBUG("Create VRN_DEPTH_STENCIL Renderbuffer for rt " << id <<
                   " with " << size_.x << " x " << size_.y);
            rt_[id].rbDepth_->Set(GL_DEPTH24_STENCIL8_EXT, size_.x, size_.y);
            break;
        default:
            LERROR("Reached default branch in createDepthComponent.");
        }
    }
    LGL_ERROR;
}

void TextureContainerFBO::createStencilBuffer(int id) {
    int attr = rt_[id].attr_;
    switch(attr & VRN_STENCIL_CONSTS_MASK) {
    case VRN_STENCIL8:
        LDEBUG("Create VRN_STENCIL8 Renderbuffer for rt " << id <<
               " with " << size_.x << " x " << size_.y);
        if (!rt_[id].rbStencil_)
            rt_[id].rbStencil_ = new Renderbuffer();
        rt_[id].rbStencil_->Set(GL_STENCIL_INDEX8_EXT, size_.x, size_.y);
                LGL_ERROR;
        break;
    default:
        LERROR("Reached default branch in CreateStencilComponent.");
    }
}

void TextureContainerFBO::unattach(int id) {
    LFBO_ERROR
    int attr = rt_[id].attr_;
    LDEBUG("Unattach rt " << id);
    if (attr & VRN_COLOR_CONSTS_MASK) {
        LDEBUG("Unattach GL_COLOR_ATTACHMENT0_EXT.");
        fbo_->Unattach(GL_COLOR_ATTACHMENT0_EXT);
    }
    if (attr & VRN_DEPTH_CONSTS_MASK) {
        LDEBUG("Unattach GL_DEPTH_ATTACHMENT_EXT.");
        fbo_->Unattach(GL_DEPTH_ATTACHMENT_EXT);
        if ((attr & VRN_DEPTH_STENCIL) == VRN_DEPTH_STENCIL) {
            LDEBUG("Unattach GL_STENCIL_ATTACHMENT_EXT.");
            fbo_->Unattach(GL_STENCIL_ATTACHMENT_EXT);
        }
    }
    if (attr & VRN_STENCIL_CONSTS_MASK) {
        LDEBUG("Unattach GL_STENCIL_ATTACHMENT_EXT.");
        fbo_->Unattach(GL_STENCIL_ATTACHMENT_EXT);
    }
}

void TextureContainerFBO::unattach(std::vector<int> id) {
    if (id.size() == 0)
        return;

    if (id.size() == 1) {
        unattach(id[0]);
        return;
    }

    // mrt unattach
    unattach(id[0]);
    //fbo_->UnattachAll();
}

void TextureContainerFBO::activateFBO() {
    LDEBUG("activateFBO: Begin");
    //LFBO_ERROR
    fbo_->Bind();
    isFBOActive_ = true;
    LDEBUG("activateFBO: End");
    //LFBO_ERROR
}

void TextureContainerFBO::deactivateFBO() {
    LDEBUG("deactivateFBO: Begin");
    if (isFBOActive_) {
        fbo_->Disable();
    }
    isFBOActive_ = false;
    LDEBUG("deactivateFBO: End");
}

FramebufferObject* TextureContainerFBO::getFBO() {
    return fbo_;
}

int TextureContainerFBO::adaptToGraphicsBoard(int attr) {
    if (GpuCaps.getVendor() == GpuCapabilities::GPU_VENDOR_NVIDIA) {
        if ((attr & VRN_DEPTH_CONSTS_MASK) == VRN_DEPTH) {
            attr &= ~VRN_DEPTH_CONSTS_MASK;
            attr |= VRN_DEPTH24;
        }
    }
    else {
        if ((attr & VRN_DEPTH_CONSTS_MASK) == VRN_DEPTH) {
            attr &= ~VRN_DEPTH_CONSTS_MASK;
            attr |= VRN_DEPTH16;
        }
    }
    if (textureTargetType_ == VRN_TEXTURE_RECTANGLE) {
        if (!(attr & VRN_FRAMEBUFFER_CONSTS_MASK))
            attr |= VRN_TEX_RECT;
    }
    return attr;
}

#endif // VRN_WITH_FBO_CLASS

//---------------------------------------------------------------------------

#ifdef VRN_WITH_RENDER_TO_TEXTURE

const std::string TextureContainerRTT::loggerCat_("voreen.opengl.TextureContainerRTT");

TextureContainerRTT::TextureContainerRTT(int numRT, bool sharing)
    : TextureContainer(numRT, sharing), curRenderTexture_(0)
{}

TextureContainerRTT::~TextureContainerRTT() {
}

TextureContainer::TextureContainerType TextureContainerRTT::getTextureContainerType() {
    return VRN_TEXTURE_CONTAINER_RTT;
}

bool TextureContainerRTT::initializeGL() {
    if (!TextureContainer::initializeGL())
        return false;
    textureTargetType_ = VRN_TEXTURE_RECTANGLE;
    return true;
}

void TextureContainerRTT::initializeTarget(int id, int attr) {
    attr = adaptToGraphicsBoard(attr);
    TextureContainer::initializeTarget(id, attr);
    if (isOpenGLInitialized_) {
        bool colorTex = false;
        bool depthTex = false;
        bool share = true;
        bool depth = false;
        bool stencil = false;
        bool mipmap = false;
        bool anisoFilter = false;
        unsigned int rBits = 0;
        unsigned int gBits = 0;
        unsigned int bBits = 0;
        unsigned int aBits = 0;
        if (attr & VRN_COLOR_CONSTS_MASK) {
            switch (attr & VRN_COLOR_CONSTS_MASK) {
            case VRN_RGB:
                rBits = gBits = bBits = 8;
                aBits = 0;
                colorTex = true;
                break;
            case VRN_RGBA:
                rBits = gBits = bBits = 8;
                aBits = 8;
                colorTex = true;
                break;
            case VRN_RGB_FLOAT16:
                rBits = gBits = bBits = 16;
                aBits = 0;
                colorTex = true;
                break;
            case VRN_RGBA_FLOAT16:
                rBits = gBits = bBits = 16;
                aBits = 16;
                colorTex = true;
                break;
            default:
                LERROR("Reached default branch in color mode selection of initializeTarget.");
            }
        }
        if (attr & VRN_DEPTH_CONSTS_MASK) {
            switch (attr & VRN_DEPTH_CONSTS_MASK) {
            case VRN_DEPTH:
                depth = true;
                depthTex = true;
                break;
            case VRN_DEPTH16:
                depth = true;
                depthTex = true;
                break;
            case VRN_DEPTH24:
                depth = true;
                depthTex = true;
                break;
            case VRN_DEPTH32:
                depth = true;
                depthTex = true;
                break;
            default:
                LERROR("Reached default branch in depth mode selection of initializeTarget.");
            }
        }
        if (colorTex || depthTex) {
            LDEBUG("Create renderTexture.");
            LDEBUG("Share: " << share << ", depth: " << depth << ", stencil: " <<
                stencil << ", mipmap: " << mipmap << ", anisoFilter: " << anisoFilter);
            LDEBUG("rBits: " << rBits << ", gBits: " << gBits << ", bBits: " << bBits <<
                ", aBits: " << aBits);
            rt_[id].rt_ = new RenderTexture(size_.x, size_.y, colorTex, depthTex);
            rt_[id].rt_->Initialize(share, depth, stencil, mipmap, anisoFilter, rBits, gBits, bBits, aBits);
        }
    }
}

void TextureContainerRTT::setSize(const tgt::ivec2& size) {
    if (size != size_) {
        TextureContainer::setSize(size);

        for (int i=0; i<capacity_; ++i) {
            delete rt_[i].rt_;
            initializeTarget(i, rt_[i].attr_);
        }
        /*
        if (capacity_>0) {
            switch(rt_[0].rt_->GetTextureTarget()) {
            case GL_TEXTURE_RECTANGLE_NV:
                textureTargetType_ = VRN_TEXTURE_RECTANGLE;
                LINFO("Type of render targets: GL_TEXTURE_RECTANGLE_NV");
                break;
            case GL_TEXTURE_2D:
                textureTargetType_ = VRN_TEXTURE_2D;
                LINFO("Type of render targets: GL_TEXTURE_2D");
                break;
            default:
                LERROR("Unknown texture target type: " << rt_[0].rt_->GetTextureTarget());
            }
        }
        */
    }
}

void TextureContainerRTT::setActiveTarget(int id, const std::string& debugLabel,
                                          CubemapOrientation /*cubemapOrientation*/)
{
    if (debugLabel != "")
        setDebugLabel(id, debugLabel);
    int attr = rt_[id].attr_;
    if (attr & VRN_FRAMEBUFFER_CONSTS_MASK) {
        LDEBUG("Attach rt " << id <<" as framebuffer.");
        if (curRenderTexture_) {
            curRenderTexture_->EndCapture();
            curRenderTexture_ = 0;
        }
        LGL_ERROR;
    } else {
        if (id >= 0) {
            if (curRenderTexture_)
                curRenderTexture_->EndCapture();
            rt_[id].rt_->BeginCapture();
            curRenderTexture_ = rt_[id].rt_;
        }
    }
}

void TextureContainerRTT::setActiveTargets(const std::vector<int>& /*targets*/, const std::string& /*debugLabel*/) {
    LERROR("Multiple render targets are not available.");
}

GLuint TextureContainerRTT::getGLTexID(int id) {
    return rt_[id].rt_->GetTextureID();
}

GLuint TextureContainerRTT::getGLDepthTexID(int id) {
    return rt_[id].rt_->GetDepthTextureID();
}

GLenum TextureContainerRTT::getGLTexTarget(int id, CubemapOrientation /*cubemapOrientation*/) {
    return rt_[id].rt_->GetTextureTarget();
}

GLenum TextureContainerRTT::getGLDepthTexTarget(int id) {
    return rt_[id].rt_->GetTextureTarget();
}

void TextureContainerRTT::unattach(int /*id*/) {
}

void TextureContainerRTT::unattach(std::vector<int> /*id*/) {
}

int TextureContainerRTT::adaptToGraphicsBoard(int attr) {
    switch (attr & VRN_COLOR_CONSTS_MASK) {
    case VRN_RGB_FLOAT16:
        LWARNING("Float textures are currently not supported.");
        attr = attr & ~VRN_COLOR_CONSTS_MASK;
        attr = attr | VRN_RGB;
        break;
    case VRN_RGBA_FLOAT16:
        LWARNING("Float textures are currently not supported.");
        attr = attr & ~VRN_COLOR_CONSTS_MASK;
        attr = attr | VRN_RGBA;
        break;
    }
    if (textureTargetType_ == VRN_TEXTURE_RECTANGLE) {
        if (!(attr & VRN_FRAMEBUFFER_CONSTS_MASK))
            attr |= VRN_TEX_RECT;
    }
    return attr;
}

#endif // VRN_WITH_RENDER_TO_TEXTURE

} // namespace voreen
