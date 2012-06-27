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

#include "tgt/gpucapabilities.h"

#include <string>

#ifdef WIN32
    #include <windows.h>
#else // WIN32 -> must be POSIX
    #include <sys/utsname.h> // for uname
#endif // WIN32

#include "tgt/logmanager.h"


//using namespace std;
using std::string;
using std::stringstream;

namespace tgt {

const std::string GpuCapabilities::loggerCat_("tgt.GpuCapabilities");

GpuCapabilities::GpuCapabilities(bool detectCaps) {
	if (detectCaps) {
		detectCapabilities();
		detectOS();
	}
}

GpuCapabilities::GlVersion GpuCapabilities::getGlVersion() {
    return glVersion_;
}

bool GpuCapabilities::isOpenGlVersionSupported(GpuCapabilities::GlVersion version) {
    return (glVersion_ >= version);
}

GpuCapabilities::GpuVendor GpuCapabilities::getVendor() {
    return vendor_;
}

std::string GpuCapabilities::getVendorAsString() {
    switch (vendor_) {
    case GPU_VENDOR_NVIDIA:
        return "NVIDIA";
    case GPU_VENDOR_ATI:
        return "ATI";
    case GPU_VENDOR_INTEL:
        return "Intel";
    default:
        return "unknown";
    }
}

bool GpuCapabilities::isExtensionSupported(string extension) {
    return (glExtensionsString_.find(extension) != string::npos);
}

string GpuCapabilities::getGlVersionString() {
    return glVersionString_;
}

string GpuCapabilities::getGlVendorString() {
    return glVendorString_;
}

string GpuCapabilities::getGlRendererString() {
    return glRendererString_;
}

string GpuCapabilities::getGlExtensionsString() {
    return glExtensionsString_;
}

bool GpuCapabilities::areShadersSupported() {
    return shaderSupport_;
}

bool GpuCapabilities::areShadersSupportedARB() {
    return shaderSupportARB_;
}

GpuCapabilities::ShaderModel GpuCapabilities::getShaderModel() {
    return shaderModel_;
}

bool GpuCapabilities::isShaderModelSupported(GpuCapabilities::ShaderModel shaderModel) {
    return (shaderModel_ >= shaderModel);
}

int GpuCapabilities::getMaxTextureSize() {
    return maxTexSize_;
}

bool GpuCapabilities::is3DTexturingSupported() {
    return texturing3D_;
}

int GpuCapabilities::getMax3DTextureSize() {
    return max3DTexSize_;
}

int GpuCapabilities::getNumTextureUnits() {
    return numTextureUnits_;
}

bool GpuCapabilities::isNpotSupported() {
    return npotTextures_;
}

bool GpuCapabilities::areTextureRectanglesSupported() {
    return textureRectangles_;
}

bool GpuCapabilities::isAnisotropicFilteringSupported() {
    return anisotropicFiltering_;
}

float GpuCapabilities::getMaxTextureAnisotropy() {
    return maxTextureAnisotropy_;
}

bool GpuCapabilities::isTextureCompressionSupported() {
    return textureCompression_;
}

bool GpuCapabilities::arePalettedTexturesSupported() {
    return palettedTextures_;
}

bool GpuCapabilities::areSharedPalettedTexturesSupported() {
    return sharedPalettedTextures_;
}

int GpuCapabilities::getColorTableWidth() {
    return colorTableWidth_;
}

bool GpuCapabilities::areFramebufferObjectsSupported() {
    return framebufferObjects_;
}

void GpuCapabilities::logCapabilities(bool extensionsString, bool osString) {
    if (osString)
        LINFO("OS version:          " << osVersionString_);

    LINFO("OpenGL Version:      " << glVersionString_);
    LINFO("OpenGL Renderer:     " << glRendererString_);
    if (glRendererString_.find("Cheetah") != string::npos) {
        LWARNING("It seems that NVIDIA GPU emulation is running on this system.");
        LWARNING("This sometimes leads to strange errors.");
        LWARNING("It is therefore strongly recommended to turn the emulation off!");
    }
    LINFO("GPU Vendor:          " << glVendorString_ << " (" << getVendorAsString() << ")");

    if (extensionsString)
        LINFO("OpenGL Extensions:   " << glExtensionsString_);

    stringstream features;
    features << "Texturing:           " << (isOpenGlVersionSupported(TGT_GL_VERSION_1_1) ? "yes" : "no");
    if (isOpenGlVersionSupported(TGT_GL_VERSION_1_1))
        features << ", max size: " << getMaxTextureSize();
    LINFO(features.str());

    features.clear();
    features.str("");
    features << "3D Texturing:        " << (is3DTexturingSupported() ? "yes" : "no");
    if (is3DTexturingSupported())
        features << ", max size: " << getMax3DTextureSize();
    LINFO(features.str());

    features.clear();
    features.str("");
    features << "Texture features:    " << getNumTextureUnits() << " units, "
             << (isNpotSupported() ? "" : "no ") << "NPOT, "
             << (areTextureRectanglesSupported() ? "" : "no") << "rectangles, "
             << (isTextureCompressionSupported() ? "" : "no") << "compression, ";
    if (isAnisotropicFilteringSupported())
        features << getMaxTextureAnisotropy() << "x anisotropic";
    else
        features << "no anisotropic";    
    
    LINFO(features.str());
/*    
    features.clear();
    features.str("");
    features << "Texture Units:       " << getNumTextureUnits();
    LINFO(features.str());

    features.clear();
    features.str("");
    features << "NPOT / Rectangles:   " << (isNpotSupported() ? "yes" : "no") << " / ";
    features << (areTextureRectanglesSupported() ? "yes" : "no");
    LINFO(features.str());

    features.clear();
    features.str("");
    features << "Texture Anisotropy:  " << (isAnisotropicFilteringSupported() ? "yes" : "no");
    if (isAnisotropicFilteringSupported())
        features << " (" << getMaxTextureAnisotropy() << ")";
    LINFO(features.str());

    features.clear();
    features.str("");
    features << "Texture Compression: " << (isTextureCompressionSupported() ? "yes" : "no");
    LINFO(features.str());
*/
    features.clear();
    features.str("");
    features << "Framebuffer Objects: " << (areFramebufferObjectsSupported() ? "yes" : "no");
    LINFO(features.str());

    features.clear();
    features.str("");
    features << "Shaders:             " << (areShadersSupported() ? "yes (OpenGL 2.0)" : "no");

    if (areShadersSupported()) {
        features << ", Shader Model ";
        if (shaderModel_ == SHADER_MODEL_4) {
            features << "4.0";
		}
        else if (shaderModel_ == SHADER_MODEL_3) {
            features << "3.0";
        }
        else if (shaderModel_ == SHADER_MODEL_2) {
            features << "2.0";
        }
        else {
            features << "unknown";
        }
    }
    LINFO(features.str());
}

GpuCapabilities::OSVersion GpuCapabilities::getOSVersion() {
    return osVersion_;
}

std::string GpuCapabilities::getOSVersionString() {
    return osVersionString_;
}

void GpuCapabilities::detectCapabilities() {
    glVersionString_ = string(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    glVendorString_  = string(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    glRendererString_  = string(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    glExtensionsString_ = string(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));

    // OpenGL version string begins with <major_number>.<minor_number>
    if (glVersionString_.substr(0,3) == "1.0")
        glVersion_ = TGT_GL_VERSION_1_0;
    else if (glVersionString_.substr(0,3) == "1.1")
        glVersion_ = TGT_GL_VERSION_1_1;
    else if (glVersionString_.substr(0,3) == "1.2")
        glVersion_ = TGT_GL_VERSION_1_2;
    else if (glVersionString_.substr(0,3) == "1.3")
        glVersion_ = TGT_GL_VERSION_1_3;
    else if (glVersionString_.substr(0,3) == "1.4")
        glVersion_ = TGT_GL_VERSION_1_4;
    else if (glVersionString_.substr(0,3) == "1.5")
        glVersion_ = TGT_GL_VERSION_1_5;
    else if (glVersionString_.substr(0,3) == "2.0")
        glVersion_ = TGT_GL_VERSION_2_0;
    else if (glVersionString_.substr(0,3) == "2.1")
        glVersion_ = TGT_GL_VERSION_2_1;
    else if (glVersionString_.substr(0,3) == "3.0")
        glVersion_ = TGT_GL_VERSION_3_0;
    else {
        glVersion_ = TGT_GL_VERSION_UNKNOWN;
        LWARNING("Unknown OpenGL version or malformed OpenGL version string!: " << glVersionString_);
    }

    // GPU Vendor
    if (glVendorString_.find("NVIDIA") != std::string::npos)
        vendor_ = GPU_VENDOR_NVIDIA;
    else if (glVendorString_.find("ATI") != std::string::npos)
        vendor_ = GPU_VENDOR_ATI;
    else if (glVendorString_.find("INTEL") != std::string::npos ||
             glVendorString_.find("Intel") != std::string::npos)
        vendor_ = GPU_VENDOR_INTEL;
    else {
        LWARNING("Unknown graphics board vendor: " << glVendorString_);
        vendor_ = GPU_VENDOR_UNKNOWN;
    }

    // Shaders
    shaderSupport_ = (glVersion_ >= TGT_GL_VERSION_2_0);
    shaderSupportARB_ = (isExtensionSupported("GL_ARB_vertex_program") &&
        isExtensionSupported("GL_ARB_fragment_program"));

#ifndef __APPLE__
    // see http://www.opengl.org/wiki/index.php/Shading_languages:_How_to_detect_shader_model%3F
    // for information about shader models in OpenGL
    if (isExtensionSupported("GL_EXT_geometry_shader4"))
        shaderModel_ = SHADER_MODEL_4;
    else if (isExtensionSupported("GL_NV_vertex_program3")  ||
              isExtensionSupported("GL_ATI_shader_texture_lod"))
        shaderModel_ = SHADER_MODEL_3;
    else if (glVersion_ >= TGT_GL_VERSION_2_0)
        shaderModel_ = SHADER_MODEL_2;
    else
        shaderModel_ = SHADER_MODEL_UNKNOWN;
#else
    LWARNING("ASSUMING ShaderModel 3.0 is supported on Apple!");
    shaderModel_ = SHADER_MODEL_3;
#endif

    // Texturing
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint *) &maxTexSize_);
    texturing3D_ = (glVersion_ >= TGT_GL_VERSION_1_2 ||
        isExtensionSupported("GL_EXT_texture3D"));

    if (texturing3D_) {
        if (glVersion_ >= TGT_GL_VERSION_2_0)
            glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, (GLint *) &max3DTexSize_);
        else
            glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE_EXT, (GLint *) &max3DTexSize_);
    }
    else {
        max3DTexSize_ = 0;
    }

    // see http://developer.nvidia.com/object/General_FAQ.html#t6
    // for information about texture units
    numTextureUnits_ = -1;
    if (isExtensionSupported("GL_ARB_fragment_program"))
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, (GLint *) &numTextureUnits_);

    if (numTextureUnits_ < 0)
        glGetIntegerv(GL_MAX_TEXTURE_UNITS, (GLint *) &numTextureUnits_);

    npotTextures_ = (isExtensionSupported("GL_ARB_texture_non_power_of_two"));
    textureRectangles_ = (isExtensionSupported("GL_ARB_texture_rectangle"));
    anisotropicFiltering_ = (isExtensionSupported("GL_EXT_texture_filter_anisotropic"));
    if (anisotropicFiltering_)
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxTextureAnisotropy_);
    else
        maxTextureAnisotropy_ = 1.0;
    textureCompression_ = (isExtensionSupported("GL_ARB_texture_compression"));
    palettedTextures_ = (isExtensionSupported("GL_EXT_paletted_texture"));
    sharedPalettedTextures_ = (isExtensionSupported("GL_EXT_shared_texture_palette"));
    if (sharedPalettedTextures_)
        glGetColorTableParameteriv(GL_SHARED_TEXTURE_PALETTE_EXT, GL_COLOR_TABLE_WIDTH, (GLint *) &colorTableWidth_);
    else
        colorTableWidth_ = 0;

    framebufferObjects_ = (isExtensionSupported("GL_EXT_framebuffer_object"));

}

void GpuCapabilities::detectOS() {
    osVersion_ = OS_UNKNOWN;
    osVersionString_ = "unknown";
    std::ostringstream oss;

#ifdef WIN32
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (!GetVersionEx((OSVERSIONINFO*)&osvi)) {
        oss << "unknown: GetVersionEx() failed";
    }
    else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
        if (osvi.wProductType == VER_NT_WORKSTATION) {
             osVersion_ = OS_WIN_VISTA;
             oss << "Windows Vista";
        }
        else {
             osVersion_ = OS_WIN_SERVER_2008;
             oss << "Windows Server 2008";
        }
    }
    else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
        osVersion_ = OS_WIN_SERVER_2003;
        oss << "Windows Server 2003";
    }
    else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
        osVersion_ = OS_WIN_XP;
        oss << "Windows XP";
    }
    else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
        osVersion_ = OS_WIN_2000;
        oss << "Windows 2000";
    }
    else {
        oss << "unknown Windows version " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion;
    }
    // Include service pack (if any) and build number.
    if (strlen(osvi.szCSDVersion) > 0) {
        oss << " " << osvi.szCSDVersion;
    }
    oss << " (build " <<  osvi.dwBuildNumber << ")";
    osVersionString_ = oss.str();
    
#else // WIN32 -> must be UNIX/POSIX
    osVersion_ = OS_POSIX;

    utsname name;
    if (uname(&name) != 0)
        return; // command not successful

    oss << name.sysname << ' ' << name.release << ' ' << name.version << ' ' << name.machine;

#endif // WIN32

    osVersionString_ = oss.str();
}

} // namespace tgt
