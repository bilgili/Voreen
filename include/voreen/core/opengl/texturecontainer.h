/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#ifndef VRN_TEXTURECONTAINER_H
#define VRN_TEXTURECONTAINER_H

#include <string>
#include <vector>
#include <list>
#include <stack>
#include <map>

#include "tgt/types.h"
#include "tgt/vector.h"

#include "voreen/core/vis/identifier.h"

#include <GL/glew.h>

#ifdef VRN_WITH_FBO_CLASS
class FramebufferObject;
class Renderbuffer;
#endif

#ifdef VRN_WITH_RENDER_TO_TEXTURE
// This forward declaration is done to prevent indirectly including Xlib.h into the entire
// project, which would introduce some pretty nasty global defines (e.g. "Status").
class RenderTexture; 
#endif


namespace voreen {

/**
 * Manages render targets. Render targets are textures with/without depth
 * or the framebuffer. The type is determined by constants.
 * No range checking is performend in all time critical routines.
 */
class TextureContainer {
public:
    /**
     * Defines the various possible texture types for the \a TextureContainer.
     */
    enum TextureType {
        VRN_RGB                     = 0x00000001,
        VRN_RGBA                    = 0x00000002,
        VRN_RGB_FLOAT16             = 0x00000003,
        VRN_RGBA_FLOAT16            = 0x00000004,
        VRN_COLOR_CONSTS_MASK       = 0x0000000f,
        VRN_DEPTH                   = 0x00000010,
        VRN_DEPTH16                 = 0x00000020,
        VRN_DEPTH24                 = 0x00000030,
        VRN_DEPTH32                 = 0x00000040,
        VRN_DEPTH_STENCIL           = 0x00000050,
        VRN_DEPTH_CONSTS_MASK       = 0x000000f0,
        VRN_STENCIL1                = 0x00000100,
        VRN_STENCIL4                = 0x00000200,
        VRN_STENCIL8                = 0x00000300,
        VRN_STENCIL16               = 0x00000400,
        VRN_STENCIL_CONSTS_MASK     = 0x00000f00,
        VRN_DEPTH_TEX               = 0x00001000,
        VRN_DEPTH_TEX_CONSTS_MASK   = 0x00001000,
        VRN_TEX_RECT                = 0x00002000,
        VRN_TEX_RECT_CONSTS_MASK    = 0x00002000,
        VRN_CUBE_MAP                = 0x00004000,
        VRN_CUBE_MAP_CONSTS_MASK    = 0x00004000,
        // this item should not or-combined, but...
        VRN_FRAMEBUFFER             = 0x00100000,
        VRN_FRAMEBUFFER_CONSTS_MASK = 0x00f00000
    };

    /**
     * Defines the texture target type of the texture container.
     */
    enum TextureTarget {
        VRN_TEXTURE_2D,             /// GL_TEXTURE_2D
        VRN_TEXTURE_RECTANGLE,      /// GL_TEXTURE_RECTANGLE_ARB (defined by OpenGL-extension GL_ARB_texture_rectangle)
        VRN_TEXTURE_RESIZED_POT     /// GL_TEXTURE_2D resized to power-of-two (not implemented yet)
    };

    /**
     * Stores constants for addressing the different sides of a cube map.
     *
     * @Note: The implementation for cube maps is very rudimentary.
     */
    enum CubemapOrientation {
        VRN_NONE,
        VRN_CUBE_MAP_POSITIVE_X,
        VRN_CUBE_MAP_NEGATIVE_X,
        VRN_CUBE_MAP_POSITIVE_Y,
        VRN_CUBE_MAP_NEGATIVE_Y,
        VRN_CUBE_MAP_POSITIVE_Z,
        VRN_CUBE_MAP_NEGATIVE_Z
    };
    /**
     * Determines the type of the used TextureContainer.
     *
     */
    enum TextureContainerType {
        VRN_TEXTURE_CONTAINER_AUTO,  /// automatic selection
        VRN_TEXTURE_CONTAINER_FBO,   /// use frame buffer objects
        VRN_TEXTURE_CONTAINER_RTT    /// use render to texture
    };

    /**
     * Creates a container for RenderTargets.
     *
     * If sharing is enabled, the objects of this class works synchronized. I.e.
     * you don't have to disable one object to work with another.
     *
     * @param numRT Number of initial reserved RenderTargets.
     * @param sharing Share with other TextureContainers
     */
    TextureContainer(int numRT, bool sharing = false);
    virtual ~TextureContainer();

    /**
     * Creates a TextureContainer which is supported on the current hardware.
     */
    static TextureContainer* createTextureContainer(int numRT, bool sharing = false, TextureContainerType type=VRN_TEXTURE_CONTAINER_AUTO);

    /**
     * Returns the type of this TextureContainer.
     */
    virtual TextureContainerType getTextureContainerType() = 0;

    /**
     * Set the capacity increment.
     *
     * Set the number of new elements, that will be reserved, when space for an
     * additional element is needed.
     *
     * @param incr capacity increment
     */
    virtual void setCapacityIncr(int incr);

    /**
     * Initialize the TextureContainer. Since the TextureContainer depends on the
     * framebuffer object extension (EXT_frambuffer_object) it may fail with some graphic boards.
     * On windows operating system it is important to call this routine after OpenGL is
     * initialized.
     *
     * @return: true if initialization was succesfull, false otherwise
     */
    virtual bool initializeGL();

    /**
     * Initialize a RenderTarget.
     *
     * If id exceeds the capacity, the capacity will be increased.
     *
     * @param id Specifies the rt to be initialized.
     * @param attr Specifies the type of the rt with or-combined
     * constants. Valid color attributes are VRN_RGB, VRN_RGBA,
     * VRN_RGB_FLOAT16, VRN_RGBA_FLOAT16. Valid depth constants are VRN_DEPTH.
     * To render in a depth texture instead of a depth renderbuffer, use
     * VRN_DEPTH|VRN_DEPTH_TEX.
     * For rendering into the framebuffer use VRN_FRAMEBUFFER.
     */
    virtual void initializeTarget(int id, int attr);

	/**
     * Makes the rendertarget id the active RenderTarget.
     *
     * If the new rt is an fbo and the fbo isn't active, it will be activated and
     * vice versa.
     *
     * @param id Id of the new active rt.
     * @param cubeMapSide Side of the cube map to attach.
     */
    virtual void setActiveTarget(int id, CubemapOrientation cubemapOrientation);

    virtual void setActiveTarget(int id, const std::string& debugLabel = "", CubemapOrientation cubemapOrientation=VRN_NONE) = 0;

    /**
     * Work in progress: This function currently only supports a vector with 2 elements,
     *        the first render target is set normally (as when calling activateTarget), the
     *        second render target is only set as a color attachment.
     */
    virtual void setActiveTargets(const std::vector<int>& targets) = 0;

    /**
     * Returns the OpenGL texture id of the color texture.
     *
     * @param id Id of the rt.
     *
     * @return OpenGL texture id.
     */
    virtual GLuint getGLTexID(int id) = 0;

    /**
     * Returns the OpenGL texture id of the depth texture.
     *
     * @param id Id of the rt.
     *
     * @return OpenGL texture id.
     */
    virtual GLuint getGLDepthTexID(int id) = 0;

    /**
     * Returns the appropiate texture target.
     *
     * This can be GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE_ARB or
     * GL_TEXTURE_CUBE_MAP. If you specify cubeMapType (with a appropriate
     * VRN_CUBE_MAP_* constant) you will get a GL_TEXTURE_CUBE_MAP_*
     * target.
     *
     * @param id Id of the rt.
     * @param cubeMapSide Side of the cube map to attach.
     */
    virtual GLenum getGLTexTarget(int id, CubemapOrientation cubemapOrientation = VRN_NONE) = 0;

    /**
     * Returns the appropiate depth texture target.
     *
     * This can be GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE_ARB or
     * GL_NONE.
     *
     * @param id Id of the rt.
     */
    virtual GLenum getGLDepthTexTarget(int id) = 0;

    /**
     * Resizes all stored elements.
     *
     * @param size new size
     */
    virtual void setSize(const tgt::ivec2& size);

    /**
     * Returns the current size.
     */
    virtual tgt::ivec2 getSize();

    /**
     * Returns the number of available targets.
     *
     * @return number of available targets.
     */
    virtual int getNumAvailable();

    /**
     * Returns the attributes of the desired RenderTarget.
     *
     * @param id Desired rt.
     *
     * @return Attributes of the rt.
     */
    virtual int getAttr(int id);

    /**
     * Returns the current active target.
     */
    virtual int getActiveTarget();

    /**
     * Returns the id of the render target to that will be finally rendered.
     */
    virtual int getFinalTarget();

    /**
     * Set the id of the render target to that will be finally rendered.
     */
    virtual void setFinalTarget(int id);

    /**
     * Pushes the currently active RenderTarget on an internal stack.
     */
    virtual void pushActiveTarget();

    /**
     * Pops from the internal stack and activates a RenderTarget.
     */
    virtual void popActiveTarget();

    /**
     * Returns the content of a rt as floats
     *
     * The caller has to free the memory after using (with delete[]).
     */
    virtual float* getTargetAsFloats(int id);

    /**
     * Returns the value of pixel (x,y) of a rt.
     *
     * The caller has to free the memory after using (with delete[]).
     *
     */
    virtual float* getTargetAsFloats(int id, int x, int y);

    /**
     * Returns the content of the depth buffer of a rt as floats
     *
     * The caller has to free the memory after using (with delete[]).
     */
    virtual float* getDepthTargetAsFloats(int id);

    /**
     * Returns the value of pixel (x,y) of the depth buffer of a rt as a float.
     *
     */
    virtual float getDepthTargetAsFloats(int id, int x, int y);

    /**
     * Set a label for a target to allow debugging.
     *
     * A consecutive number is added to allow the user to check the age of this target.
     *
     * @param id Id of the rt.
     * @param s debug label.
     */
    virtual void setDebugLabel(int id, const std::string& s);

    /**
     * Returns a given debug label.
     */
    virtual std::string getDebugLabel(int id) const;

    /**
     * Returns a type as string.
     *
     * Assumes, that type contains a single type i.e. no or-combined value.
     *
     * @param type Type to convert.
     *
     * @return Type as string.
     */
    virtual std::string getTypeAsString(int attr) const;

    /**
     * Returns the texture target type of the render targets. All texture targets
     * are of the same type!
     *
     * \see TextureTarget
     */
    virtual TextureTarget getTextureContainerTextureType();

    friend std::ostream& operator<<(std::ostream& os, const TextureContainer& tc);
    GLuint shadowTexID_;

protected:
    /**
     * Select appropriate texture types (depending on graphics board.)
     */
    virtual int adaptToGraphicsBoard(int attr);

    /**
     * Change the capacity.
     *
     * Contents of rb_ and tex_ remains unchanged in the case of an increase.
     *
     * @param capacity New capacity.
     */
    virtual void setCapacity(int capacity);

    virtual void unattach(int id) = 0;
    virtual void unattach(std::vector<int> id) = 0;

    /**
     * Stores a single RenderTarget. Contains the attributes and where
     * required a Renderbuffer, and/or the OpenGL id of a color texture
     * and a depth texture.
     */
    struct RenderTarget {
        RenderTarget();
        ~RenderTarget(); 
        int attr_;
        bool free_;
        std::string debugLabel_;
        bool persistent_;
#ifdef VRN_WITH_FBO_CLASS
        GLuint tex_;
        GLuint depthTex_;
        Renderbuffer* rbDepth_;
        Renderbuffer* rbStencil_;
#endif
#ifdef VRN_WITH_RENDER_TO_TEXTURE
        RenderTexture* rt_;
#endif
    };
    
    /// Pointer to the list of RenderTargets
    RenderTarget *rt_;
    /// Current capacity of RenderTargets
    int capacity_;
    /// Increment size for capacity change
    int capacityIncr_;
    /// Current number of used RenderTargets
    int used_;
    /// is the FBO currently active?

    /// the graphics board is (normally) a single resource. If multiple TextureContainer share this
    /// resource, the active TextureContainer has to be known.
    bool sharing_;
    static TextureContainer* activeTc_;

    /// size of the textures
    tgt::ivec2 size_;
    /// currently used RT(s) (a list is needed because of multiple RTs)
    std::vector<int> current_;

    int finalTarget_;

    /// the texture target type of all render targets
    TextureTarget textureTargetType_;

    /// Name of the logger
    static const std::string loggerCat_;

    bool isOpenGLInitialized_;
    /// stack for push/popActiveTarget
    std::stack<int> currentRTStack_;
};

// output for debugging
inline std::ostream& operator<<(std::ostream& os, const TextureContainer& tc) {
    for (int i=0; i < tc.used_; ++i)
        os << "    rt" << (i < 10 ? "0" : "") << i << ": "
           << " '" << tc.rt_[i].debugLabel_ << "'"
           << (i == tc.finalTarget_ ? " FINAL" : "")
           << "\n";
    return os;
}

//---------------------------------------------------------------------------

#ifdef VRN_WITH_FBO_CLASS

class TextureContainerFBO : public TextureContainer {
public:
protected:
    /**
     * Creates a container for RenderTargets.
     *
     * If sharing is enabled, the objects of this class works synchronized. I.e.
     * you don't have to disable one object to work with another.
     *
     * @param numRT Number of initial reserved RenderTargets.
     * @param sharing Share with other TextureContainers
     */
    TextureContainerFBO(int numRT, bool sharing = false);
public:
    friend TextureContainer* TextureContainer::createTextureContainer(int numRT, bool sharin, 
        TextureContainer::TextureContainerType type);
    ~TextureContainerFBO();

    virtual TextureContainerType getTextureContainerType();

    /**
     * Initialize the TextureContainer. Since the TextureContainer depends on the
     * framebuffer object extension (EXT_frambuffer_object) it may fail with some graphic boards.
     * On windows operating system it is important to call this routine after OpenGL is
     * initialized.
     *
     * @return: true if initialization was succesfull, false otherwise
     */
    virtual bool initializeGL();

    /**
     * Initialize a RenderTarget.
     *
     * If id exceeds the capacity, the capacity will be increased.
     *
     * @param id Specifies the rt to be initialized.
     * @param attr Specifies the type of the rt with or-combined
     * constants. Valid color attributes are VRN_RGB, VRN_RGBA,
     * VRN_RGB_FLOAT16, VRN_RGBA_FLOAT16. Valid depth constants are VRN_DEPTH.
     * To render in a depth texture instead of a depth renderbuffer, use
     * VRN_DEPTH|VRN_DEPTH_TEX.
     * For rendering into the framebuffer use VRN_FRAMEBUFFER.
     */
    virtual void initializeTarget(int id, int attr);

	/**
     * Makes the rendertarget id the active RenderTarget.
     *
     * If the new rt is an fbo and the fbo isn't active, it will be activated and
     * vice versa.
     *
     * @param id Id of the new active rt.
     * @param cubeMapSide Side of the cube map to attach.
     */
    //virtual void setActiveTarget(int id, CubemapOrientation cubemapOrientation);

    virtual void setActiveTarget(int id, const std::string& debugLabel = "", CubemapOrientation cubemapOrientation=VRN_NONE);

    /**
     * Work in progress: This function currently only supports a vector with 2 elements,
     *        the first render target is set normally (as when calling activateTarget), the
     *        second render target is only set as a color attachment.
     */
    void setActiveTargets(const std::vector<int>& targets);

    /**
     * Returns the OpenGL texture id of the color texture.
     *
     * @param id Id of the rt.
     *
     * @return OpenGL texture id.
     */
    virtual GLuint getGLTexID(int id);

    /**
     * Returns the OpenGL texture id of the depth texture.
     *
     * @param id Id of the rt.
     *
     * @return OpenGL texture id.
     */
    virtual GLuint getGLDepthTexID(int id);

    /**
     * Returns the appropiate texture target.
     *
     * This can be GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE_ARB or
     * GL_TEXTURE_CUBE_MAP. If you specify cubeMapType (with a appropriate
     * VRN_CUBE_MAP_* constant) you will get a GL_TEXTURE_CUBE_MAP_*
     * target.
     *
     * @param id Id of the rt.
     * @param cubeMapSide Side of the cube map to attach.
     */
    virtual GLenum getGLTexTarget(int id, CubemapOrientation cubemapOrientation = VRN_NONE);

    /**
     * Returns the appropiate depth texture target.
     *
     * This can be GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE_ARB or
     * GL_NONE.
     *
     * @param id Id of the rt.
     */
    virtual GLenum getGLDepthTexTarget(int id);

    /**
     * Resizes all stored elements.
     *
     * @param size new size
     */
    void setSize(const tgt::ivec2& size);

    /**
      * Returns the framebuffer object
      */
    FramebufferObject* getFBO();

    friend std::ostream& operator<<(std::ostream& os, const TextureContainerFBO& tc);
    GLuint shadowTexID_;
    
protected:

    /**
     * Select appropriate texture types (depending on graphics board.)
     */
    virtual int adaptToGraphicsBoard(int attr);

    /**
     * Creates an OpenGL color texture.
     *
     * The new texture optains the size from size_ and gets the format
     * from rt_[id].attr_. No glBindTexture is performend and has to be done by the
     * calling method.
     *
     * @param id Id of the rt.
     */
    void createColorTexture(int id);

    /**
     * Creates a Renderbuffer.
     *
     * The rb optains its size from size_ and gets the format from
     * rt_[id].attr_.
     *
     * @param id Id of the rt.
     */
    void createDepthComponent(int id);
    void createStencilBuffer(int id);
    void unattach(int id);
    void unattach(std::vector<int> id);
    void activateFBO();
    void deactivateFBO();
    std::string getFBOError();

    /// is the FBO currently active?
    bool isFBOActive_;
    /// The FBO
    FramebufferObject* fbo_;
};

// output for debugging
inline std::ostream& operator<<(std::ostream& os, const TextureContainerFBO& tc) {
    for (int i=0; i < tc.used_; ++i)
        os << "    rt" << (i < 10 ? "0" : "") << i << ": "
           << " '" << tc.rt_[i].debugLabel_ << "'"
           << (i == tc.finalTarget_ ? " FINAL" : "")
           << "\n";
    return os;
}

#endif // VRN_WITH_FBO_CLASS

//---------------------------------------------------------------------------

#ifdef VRN_WITH_RENDER_TO_TEXTURE

class TextureContainerRTT : public TextureContainer 
{
public:
protected:
    TextureContainerRTT(int numRT, bool sharing = false);
public:
    friend TextureContainer* TextureContainer::createTextureContainer(int numRT, bool sharin,
        TextureContainer::TextureContainerType type);
    ~TextureContainerRTT();

    virtual TextureContainerType getTextureContainerType();

    /**
     * Initialize the TextureContainer. Since the TextureContainer depends on the
     * framebuffer object extension (EXT_frambuffer_object) it may fail with some graphic boards.
     * On windows operating system it is important to call this routine after OpenGL is
     * initialized.
     *
     * @return: true if initialization was succesfull, false otherwise
     */
    virtual bool initializeGL();
    /**
     * Initialize a RenderTarget.
     *
     * If id exceeds the capacity, the capacity will be increased.
     *
     * @param id Specifies the rt to be initialized.
     * @param attr Specifies the type of the rt with or-combined
     * constants. Valid color attributes are VRN_RGB, VRN_RGBA,
     * VRN_RGB_FLOAT16, VRN_RGBA_FLOAT16. Valid depth constants are VRN_DEPTH.
     * To render in a depth texture instead of a depth renderbuffer, use
     * VRN_DEPTH|VRN_DEPTH_TEX.
     * For rendering into the framebuffer use VRN_FRAMEBUFFER.
     */
    virtual void initializeTarget(int id, int attr);
    /**
     * Makes the rendertarget id the active RenderTarget.
     *
     * If the new rt is an fbo and the fbo isn't active, it will be activated and
     * vice versa.
     *
     * @param id Id of the new active rt.
     * @param cubeMapSide Side of the cube map to attach.
     */
    //virtual void setActiveTarget(int id, CubemapOrientation cubemapOrientation);

    virtual void setActiveTarget(int id, const std::string& debugLabel = "", CubemapOrientation cubemapOrientation=VRN_NONE);

    /**
     * Work in progress: This function currently only supports a vector with 2 elements,
     *        the first render target is set normally (as when calling activateTarget), the
     *        second render target is only set as a color attachment.
     */
    virtual void setActiveTargets(const std::vector<int>& targets);
    /**
     * Returns the OpenGL texture id of the color texture.
     *
     * @param id Id of the rt.
     *
     * @return OpenGL texture id.
     */
    virtual GLuint getGLTexID(int id);

    /**
     * Returns the OpenGL texture id of the depth texture.
     *
     * @param id Id of the rt.
     *
     * @return OpenGL texture id.
     */
    virtual GLuint getGLDepthTexID(int id);

    /**
     * Returns the appropiate texture target.
     *
     * This can be GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE_ARB or
     * GL_TEXTURE_CUBE_MAP. If you specify cubeMapType (with a appropriate
     * VRN_CUBE_MAP_* constant) you will get a GL_TEXTURE_CUBE_MAP_*
     * target.
     *
     * @param id Id of the rt.
     * @param cubeMapSide Side of the cube map to attach.
     */
    virtual GLenum getGLTexTarget(int id, CubemapOrientation cubemapOrientation = VRN_NONE);

    /**
     * Returns the appropiate depth texture target.
     *
     * This can be GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE_ARB or
     * GL_NONE.
     *
     * @param id Id of the rt.
     */
    virtual GLenum getGLDepthTexTarget(int id);
    /**
     * Resizes all stored elements.
     *
     * @param size new size
     */
    virtual void setSize(const tgt::ivec2& size);

protected:
    virtual void unattach(int id);
    virtual void unattach(std::vector<int> id);

    /**
     * Select appropriate texture types (depending on graphics board.)
     */
    virtual int adaptToGraphicsBoard(int attr);

    RenderTexture* curRenderTexture_;
};

#endif // VRN_WITH_RENDER_TO_TEXTURE

} // namespace voreen

#endif // VRN_TEXTURECONTAINER_H
