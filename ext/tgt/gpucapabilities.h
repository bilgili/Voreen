/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#ifndef TGT_GPUCAPABILITIES_H
#define TGT_GPUCAPABILITIES_H

#include <string>

#include "tgt/config.h"
#include "tgt/singleton.h"
#include "tgt/tgt_gl.h"

namespace tgt {

/**
 * This tgt-Singleton provides information about the graphics system.
 * This information includes:
 *  - Operating system
 *  - OpenGL version
 *  - Supported OpenGL extensions
 *  - GPU vendor
 *  - Texturing and shader capabilities
 *
 * All data except the operating system information is exclusively retrieved 
 * through the OpenGL API and can thus be regarded as reliable.
 *
 * The global identifier of this class' singleton is <tt>GpuCaps</tt>.
 */
class GpuCapabilities {
public:

    /**
     * Specifies the version
     * of the OpenGL/glsl implementation.
     * TGT_GL_VERSION_x_y denotes OpenGL version x.y.
     *
     * TGT prefix is necessary due to name clashes with glew.
     */
	class GlVersion {
		public:
		GlVersion(int major = 0, int minor = 0, int release = 0);

		/**
		 * Parse OpenGL version string as specified:
		 *
		 * The GL_VERSION and GL_SHADING_LANGUAGE_VERSION strings begin with a version number.
		 * The version number uses one of these forms:
		 *      major_number.minor_number
		 *      major_number.minor_number.release_number
		 *
		 * Vendor-specific information may follow the version number.
		 * Its format depends on the implementation, but a space always separates the version number and the vendor-specific information.
		 */
		bool parseVersionString(const std::string& st);

		int major_;
		int minor_;
		int release_;

		int major() const { return major_; }
		int minor() const { return minor_; }
		int release() const { return release_; }

		friend bool operator==(const GlVersion& x, const GlVersion& y);
		friend bool operator!=(const GlVersion& x, const GlVersion& y);
		friend bool operator<(const GlVersion& x, const GlVersion& y);
		friend bool operator<=(const GlVersion& x, const GlVersion& y);
		friend bool operator>(const GlVersion& x, const GlVersion& y);
		friend bool operator>=(const GlVersion& x, const GlVersion& y);
		friend std::ostream& operator<<(std::ostream& s, const GlVersion& v); 

		static const GlVersion TGT_GL_VERSION_1_0;
 		static const GlVersion TGT_GL_VERSION_1_1;
 		static const GlVersion TGT_GL_VERSION_1_2;
 		static const GlVersion TGT_GL_VERSION_1_3;
 		static const GlVersion TGT_GL_VERSION_1_4;
 		static const GlVersion TGT_GL_VERSION_1_5;
 		static const GlVersion TGT_GL_VERSION_2_0;
 		static const GlVersion TGT_GL_VERSION_2_1;
 		static const GlVersion TGT_GL_VERSION_3_0;
 		static const GlVersion TGT_GL_VERSION_3_1;
 		static const GlVersion TGT_GL_VERSION_3_2;
 		static const GlVersion TGT_GL_VERSION_3_3;
 		static const GlVersion TGT_GL_VERSION_4_0;

        static const GlVersion SHADER_VERSION_110; ///< GLSL version 1.10
        static const GlVersion SHADER_VERSION_120; ///< GLSL version 1.20
        static const GlVersion SHADER_VERSION_130; ///< GLSL version 1.30
        static const GlVersion SHADER_VERSION_140; ///< GLSL version 1.40
        static const GlVersion SHADER_VERSION_150; ///< GLSL version 1.50
        static const GlVersion SHADER_VERSION_330; ///< GLSL version 3.30
        static const GlVersion SHADER_VERSION_400; ///< GLSL version 4.00
	};


    /**
     * Identifies the vendor of the GPU
     * (not the graphics board's vendor).
     */
    enum GpuVendor {
        GPU_VENDOR_NVIDIA,
        GPU_VENDOR_ATI,
        GPU_VENDOR_INTEL,
        GPU_VENDOR_UNKNOWN
    };

    /**
     * Defines the DirectX shader model
     * supported by the GPU. This value
     * can be used to derive information
     * about the GPU's GLSL shader capabilities.
     * The shader model is fully downwards compatible.
     */
    enum ShaderModel {
        SHADER_MODEL_UNKNOWN,
        SHADER_MODEL_2,     ///< implied by OpenGL version 2.0
        SHADER_MODEL_3,     ///< extension NV_vertex_program3 or ATI_shader_texture_lod
        SHADER_MODEL_4,     ///< extension ARB_geometry_shader4
        SHADER_MODEL_5      ///< extension ARB_tessellation_shader
    };

    /**
     * Identifies the used operating system.
     */
    enum OSVersion {
        OS_UNKNOWN,
        OS_WIN_2000,
        OS_WIN_XP,
        OS_WIN_VISTA,
        OS_WIN_SERVER_2003,
        OS_WIN_SERVER_2008,
        OS_WIN_7,
        OS_POSIX     ///< For Linux and other POSIX-like OS. Have a look at getOSVersionString for details.
    };

    /**
     * Creates an object for the detection of graphics system properties. If detectCapabilities
	 * is set to false, the capabilities of the graphics card aren't detected right away in the
	 * constructor. This way you can use GpuCapabilitiesWindows to detect the amount of memory
	 * on the graphics card before initGL() is called. Otherwise GpuCapabilities tries to
	 * detect GL values while initGL() isn't called yet and produces a crash.
     */
    GpuCapabilities(bool detectCaps = true);

    virtual ~GpuCapabilities() {}

    /**
     * Returns the OpenGL version implemented by the driver.
     *
     * @see GlVersion
     */
    GlVersion getGlVersion();

    /**
     * Returns whether a certain OpenGL version is supported.
     *
     * @param version the OpenGL version to check
     *
     * @see GlVersion
     */
    bool isOpenGlVersionSupported(GlVersion version);

    /**
     * Returns the vendor of the GPU.
     *
     * @see GpuVendor
     */
    GpuVendor getVendor();

    /**
     * Returns the vendor of the GPU.
     *
     * @see GpuVendor
     */
    std::string getVendorAsString();
    
    /**
     * Returns wether a certain OpenGL extension
     * is supported by this implementation. The
     * check is done by parsing the OpenGL 
     * extensions-string provided by the graphics driver.
     *
     * @param extension the exact name string of the extension
     *      as found in http://www.opengl.org/registry/
     */
    bool isExtensionSupported(std::string extension);

    /**
     * Returns the complete OpenGL version string
     * retrieved by <tt>glGetString(GL_VERSION)</tt>.
     */
    std::string getGlVersionString();

    /**
     * Returns the complete OpenGL vendor string
     * retrieved by <tt>glGetString(GL_VENDOR)</tt>.
     */
    std::string getGlVendorString();

    /**
     * Returns the complete OpenGL renderer string
     * retrieved by <tt>glGetString(GL_RENDERER)</tt>.
     */
    std::string getGlRendererString();

    /**
     * Returns the complete OpenGL extensions-string
     * retrieved by <tt>glGetString(GL_EXTENSIONS)</tt>.
     * This strings contains all OpenGL extensions supported
     * by this OpenGL implementation, separated by spaces.
     */
    std::string getGlExtensionsString();

    /**
    * Returns the complete Shading Language Version string
    * retrieved by <tt>glGetString(GL_SHADING_LANGUAGE_VERSION)</tt>.
    */
    std::string getShadingLanguageVersionString();

    /**
     * Returns wether shaders are supported, which
     * is true for OpenGL version 2.0 or later.
     */
    bool areShadersSupported();

    /**
     * Returns wether the ARB shader extensions
     * are supported (GL_ARB_vertex_program and
     * GL_ARB_fragment_program).
     *
     * \warning If you want to use shaders based on these
     * extensions, you have call the ARB variants of the
     * shader functions, e.g. <tt>glCompileShaderARB</tt> instead of
     * <tt>glCompileShader</tt>
     */
    bool areShadersSupportedARB();

    /**
    * Returns the GLSL shading language version
    * supported by the GPU.
    *
    * @see GlVersion 
    */
    GlVersion getShaderVersion();

    /**
     * Returns the DirectX shader model
     * supported by the GPU.
     *
     * @see ShaderModel
     */
    ShaderModel getShaderModel();

    /**
     * Returns wether a certain shader model
     * is supported.
     *
     * @param shaderModel the shader model to check
     */
    bool isShaderModelSupported(ShaderModel shaderModel);

    /**
     * Returns the maximal side length of 1D and 2D textures.
     *
     * @see getMax3DTextureSize
     */
    int getMaxTextureSize();

    /**
     * Returns wether 3D textures are supported.
     * This is the case for OpenGL version 1.2
     * and later.
     */
    bool is3DTexturingSupported();

    /**
     * Returns the maximal side length
     * of 3D textures. If 3D texturing
     * is not supported, 0 is returned.
     */
    int getMax3DTextureSize();

    /**
     * Returns the number of texture units
     * provided by the GPU.
     */
    int getNumTextureUnits();

    /**
     * Returns wether non-power-of-two textures
     * are supported (extension GL_ARB_texture_non_power_of_two).
     */
    bool isNpotSupported();

    /**
     * Returns wether texture rectangles
     * are supported (extension GL_ARB_texture_rectangle).
     */
    bool areTextureRectanglesSupported();

    /**
     * Returns wether anisotropic texture filtering
     * is supported (extension GL_EXT_texture_filter_anisotropic).
     */
    bool isAnisotropicFilteringSupported();

    /**
     * Returns the maximum anisotropy. If
     * anisotropic texture filtering is not
     * supported, 1.0 is returned.
     */
    float getMaxTextureAnisotropy();

    /**
     * Returns wether texture compression
     * is supported (extension GL_ARB_texture_compression).
     */
    bool isTextureCompressionSupported();

    /**
     * Returns wether paletted textures
     * are supported (extension GL_EXT_paletted_texture).
     */
    bool arePalettedTexturesSupported();

    /**
     * Returns wether shared paletted textures
     * are supported (extension GL_EXT_shared_texture_palette).
     */
    bool areSharedPalettedTexturesSupported();

    /**
     * Returns the number of elements in a color table
     * when shared paletted textures are supported. 
     * Otherwise 0 is returned.
     */
    int getColorTableWidth();

    /**
     * Returns wether FramebufferObjects (FBOs)
     * are supported (extension GL_EXT_framebuffer_object).
     */
    bool areFramebufferObjectsSupported();

	///Returns the maximal number of color attachments for a FBO
	int getMaxColorAttachments(); 

    /**
     * Writes the most important GPU features to the
     * log (debug-level info).
     *
     * @param extensionsString determines wether the
     *      extensions string is also written. This is disabled by default
     *      since the extensions string is usually very long.
     * @param osString determines wether the operating system describing string
     *      is also written
     */
    virtual void logCapabilities(bool extensionsString = false, bool osString = true);

    /**
     * Get the OS version.
     */
    OSVersion getOSVersion();
    
    /**
     * Get the OS version as string.
     */
    std::string getOSVersionString();

protected:
    /**
     * Is called by the constructor and performs the
     * complete hardware detection. The results
     * are internally stored.
     */
    virtual void detectCapabilities();
    
    /**
     * Is called by the constructor and performs the
     * operating system detection. The results
     * are internally stored.
     */
    virtual void detectOS();

    static const std::string loggerCat_;

private:
    // detection results are stored in the following members

    OSVersion osVersion_;
    std::string osVersionString_;

    GlVersion glVersion_;
    std::string glVersionString_;
    std::string glExtensionsString_;
    std::string glVendorString_;
    std::string glRendererString_;
    std::string glslVersionString_;
    GpuVendor vendor_;

    bool shaderSupport_;
    bool shaderSupportARB_;
    GlVersion shaderVersion_;
    ShaderModel shaderModel_;

    int maxTexSize_;
    bool texturing3D_;
    int max3DTexSize_;
    int numTextureUnits_;
    bool npotTextures_;
    bool textureRectangles_;
    bool anisotropicFiltering_;
    float maxTextureAnisotropy_;
    bool textureCompression_;
    bool palettedTextures_;
    bool sharedPalettedTextures_;
    int colorTableWidth_;
    bool framebufferObjects_;
	int maxColorAttachments_;
};

} // namespace tgt

#define GpuCaps tgt::Singleton<tgt::GpuCapabilities>::getRef()

#endif // TGT_GPUCAPABILITIES_H
