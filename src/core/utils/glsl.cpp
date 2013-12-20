/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/core/utils/glsl.h"

#include "voreen/core/datastructures/volume/volumegl.h"

namespace voreen {

using tgt::vec3;
using std::string;

void setUniform(tgt::Shader* shader, const std::string& volumeUniform, const std::string& structUniform, const VolumeBase* vh, const tgt::TextureUnit* texUnit, const tgt::Camera* camera, const tgt::vec4& lightPosition) {
    if(texUnit)
        shader->setUniform(volumeUniform, texUnit->getUnitNumber());

    // volume size, i.e. dimensions of the proxy geometry in world coordinates
    shader->setUniform(structUniform + ".datasetDimensions_", tgt::vec3(vh->getDimensions()));
    shader->setUniform(structUniform + ".datasetDimensionsRCP_", vec3(1.f) / tgt::vec3(vh->getDimensions()));

    // volume spacing, i.e. voxel size
    shader->setUniform(structUniform + ".datasetSpacing_", vh->getSpacing());
    shader->setUniform(structUniform + ".datasetSpacingRCP_", vec3(1.f) / vh->getSpacing());

    // volume's size in its physical coordinates
    shader->setUniform(structUniform + ".volumeCubeSize_", vh->getCubeSize());
    shader->setUniform(structUniform + ".volumeCubeSizeRCP_", vec3(1.f) / vh->getCubeSize());

    shader->setUniform(structUniform + ".volumeOffset_", vh->getOffset());

    shader->setUniform(structUniform + ".numChannels_", static_cast<GLint>(vh->getNumChannels()));

    // volume's transformation matrix
    shader->setUniform(structUniform + ".physicalToWorldMatrix_", vh->getPhysicalToWorldMatrix());

    tgt::mat4 invTm = vh->getWorldToPhysicalMatrix();
    shader->setUniform(structUniform + ".worldToPhysicalMatrix_", invTm);

    shader->setUniform(structUniform + ".worldToTextureMatrix_", vh->getWorldToTextureMatrix());
    shader->setUniform(structUniform + ".textureToWorldMatrix_", vh->getTextureToWorldMatrix());

    // camera position in volume object coords
    if (camera)
        shader->setUniform(structUniform + ".cameraPositionPhysical_", invTm*camera->getPositionWithOffsets());

    // light position in volume object coords
    shader->setUniform(structUniform + ".lightPositionPhysical_", (invTm*lightPosition).xyz());

    LGL_ERROR;

    // bit depth of the volume
    shader->setUniform(structUniform + ".bitDepth_", (GLint)(vh->getBytesPerVoxel() * 8));

    // construct shader real-world mapping by combining volume rwm and pixel transfer mapping
    RealWorldMapping rwm = vh->getRealWorldMapping();
    RealWorldMapping transferMapping;
    if (vh->getRepresentation<VolumeGL>())
        transferMapping = vh->getRepresentation<VolumeGL>()->getPixelTransferMapping();
    else
        LWARNINGC("voreen.glsl", "setUniform(): no VolumeGL");
    RealWorldMapping shaderMapping = RealWorldMapping::combine(transferMapping.getInverseMapping(), rwm);
    shader->setUniform(structUniform + ".rwmScale_", shaderMapping.getScale());
    shader->setUniform(structUniform + ".rwmOffset_", shaderMapping.getOffset());
}

bool bindVolumeTexture(const VolumeBase* vh, const tgt::TextureUnit* texUnit, GLint filterMode, GLint wrapMode, tgt::vec4 borderColor) {
    const VolumeGL* volumeGL = vh->getRepresentation<VolumeGL>();
    if (!volumeGL || !volumeGL->getTexture()) {
        LWARNINGC("voreen.glsl", "No volume texture while binding volumes");
        return false;
    }
    const VolumeTexture* volumeTex = volumeGL->getTexture();

    texUnit->activate();

    volumeTex->bind();

    // Returns the residence status of the target texture. If the value returned in params is
    // GL_TRUE, the texture is resident in texture memory
    GLint resident;
    glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_RESIDENT, &resident);

    if (resident != GL_TRUE){
        if(GpuCaps.getVendor() == tgt::GpuCapabilities::GPU_VENDOR_ATI){
            LDEBUGC("voreen.glsl", "texture not resident " << vh->getOrigin().getPath());
        } else {
            LWARNINGC("voreen.glsl", "texture not resident " << vh->getOrigin().getPath());
        }
    }
    LGL_ERROR;

    // texture filtering
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filterMode);
    LGL_ERROR;

    // texture wrapping
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapMode);
    glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, static_cast<tgt::Vector4<GLfloat> >(borderColor).elem);
    LGL_ERROR;

    texUnit->setZeroUnit();

    return true;
}

void setUniform(tgt::Shader* shader, const std::string& textureUniform, const std::string& structUniform, const VolumeSliceGL* slice, const tgt::TextureUnit* texUnit) {
    tgtAssert(shader, "null pointer passed");
    tgtAssert(slice, "null pointer passed");
    tgtAssert(slice->getTexture(), "volume slice has not texture");
    tgtAssert(texUnit, "null pointer passed");

    // check state of slice shader
    if (!shader->isActivated()) {
        LERRORC("voreen.glsl", "bindSliceTexture() slice shader not activated");
        return;
    }

    // pass tex unit
    if (texUnit)
        shader->setUniform(textureUniform, texUnit->getUnitNumber());
    LGL_ERROR;

    // determine real-world-mapping
    RealWorldMapping rwm = slice->getRealWorldMapping();
    bool isSignedInteger = slice->getBaseType() == "int8"  ||
                           slice->getBaseType() == "int16" ||
                           slice->getBaseType() == "int32" ||
                           slice->getBaseType() == "int64"   ;
    if (isSignedInteger) {
        // map signed integer types from [-1.0:1.0] to [0.0:1.0] in order to avoid clamping of negative values
        // => the pixel transfer mapping has to be reverted in the shader (see bindSliceTexture() below)
        RealWorldMapping pixelTransferMapping(0.5f, 0.5f, "");
        rwm = RealWorldMapping::combine(pixelTransferMapping.getInverseMapping(), rwm);
    }

    // pass TextureParameters struct values to shader
    //shader->setIgnoreUniformLocationError(true);
    shader->setUniform(structUniform + ".dimensions_", tgt::vec2(slice->getTexture()->getDimensions().xy()));
    shader->setUniform(structUniform + ".dimensionsRCP_", tgt::vec2(1.0f) / tgt::vec2(slice->getTexture()->getDimensions().xy()));
    shader->setUniform(structUniform + ".matrix_", tgt::mat4::identity);
    shader->setUniform(structUniform + ".numChannels_", (int)slice->getTexture()->getNumChannels());
    shader->setUniform(structUniform + ".rwmScale_", rwm.getScale());
    shader->setUniform(structUniform + ".rwmOffset_", rwm.getOffset());
    //shader->setIgnoreUniformLocationError(false);
    LGL_ERROR;
}

bool bindSliceTexture(const VolumeSliceGL* slice, const tgt::TextureUnit* texUnit,
    GLint filterMode /*= GL_LINEAR*/, GLint wrapMode /*= GL_CLAMP_TO_EDGE*/, tgt::vec4 borderColor /*= tgt::vec4(0.f)*/)
{
    tgtAssert(slice, "null pointer passed");
    tgtAssert(texUnit, "null pointer passed");

    /// bind slicetexture to unit
    texUnit->activate();
    slice->bind();
    LGL_ERROR;

    // set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    LGL_ERROR;

    // set texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor.elem);
    LGL_ERROR;

    texUnit->setZeroUnit();

    return true;
}

std::string generateStandardShaderHeader(const tgt::GpuCapabilities::GlVersion* version) {
    if (!tgt::GpuCapabilities::isInited()) {
        return "";
    }

    using tgt::GpuCapabilities;

    tgt::GpuCapabilities::GlVersion useVersion;

    //use supplied version if available, else use highest available.
    //if no version is supplied, use up tp 1.30 as default.
    if (version && GpuCaps.getShaderVersion() >= *version)
        useVersion = *version;
    else if(GpuCaps.getShaderVersion() > GpuCapabilities::GlVersion::SHADER_VERSION_410)
        useVersion = GpuCapabilities::GlVersion::SHADER_VERSION_410;
    else
        useVersion = GpuCaps.getShaderVersion();

    std::stringstream versionHeader;
    versionHeader << useVersion.major() << useVersion.minor();

    std::string header = "#version " + versionHeader.str();

    if(header.length() < 12)
        header += "0";

    //Run in compability mode to use deprecated functionality
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_150)
        header += " compatibility";
    else if(useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_140)
        header += "\n#extension GL_ARB_compatibility : enable";

    header += "\n";

    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_410)
        header += "#define GLSL_VERSION_410\n";
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_400)
        header += "#define GLSL_VERSION_400\n";
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_330)
        header += "#define GLSL_VERSION_330\n";
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_150)
        header += "#define GLSL_VERSION_150\n";
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_140)
        header += "#define GLSL_VERSION_140\n";
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_130) {
        header += "#define GLSL_VERSION_130\n";
        header += "precision highp float;\n";
    }
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_120)
        header += "#define GLSL_VERSION_120\n";
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_110)
        header += "#define GLSL_VERSION_110\n";

    if (GLEW_NV_fragment_program2) {
        GLint i = -1;
        glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_LOOP_COUNT_NV, &i);
        if (i > 0) {
            std::ostringstream o;
            o << i;
            header += "#define VRN_MAX_PROGRAM_LOOP_COUNT " + o.str() + "\n";
        }
    }

    //
    // add some defines needed for workarounds in the shader code
    //
    if (GLEW_ARB_draw_buffers)
        header += "#define VRN_GLEW_ARB_draw_buffers\n";

    #ifdef __APPLE__
        header += "#define VRN_OS_APPLE\n";
        if (GpuCaps.getVendor() == GpuCaps.GPU_VENDOR_ATI)
            header += "#define VRN_VENDOR_ATI\n";
    #endif

    if (GpuCaps.getShaderVersion() >= GpuCapabilities::GlVersion::SHADER_VERSION_130) {
        // define output for single render target
        header += "//$ @name = \"outport0\"\n";
        header += "out vec4 FragData0;\n";
    }
    else {
        header += "#define FragData0 gl_FragData[0]\n";
    }

    return header;
}

void fillShadingModesProperty(StringOptionProperty& shadeMode) {
    shadeMode.addOption("none",                   "none"                   );
    shadeMode.addOption("phong-diffuse",          "Phong (Diffuse)"        );
    shadeMode.addOption("phong-specular",         "Phong (Specular)"       );
    shadeMode.addOption("phong-diffuse-ambient",  "Phong (Diffuse+Amb.)"   );
    shadeMode.addOption("phong-diffuse-specular", "Phong (Diffuse+Spec.)"  );
    shadeMode.addOption("phong",                  "Phong (Full)"           );
    shadeMode.addOption("toon",                   "Toon"                   );
    shadeMode.addOption("cook-torrance",          "Cook-Torrance"          );
    shadeMode.addOption("oren-nayar",             "Oren-Nayar"             );
    shadeMode.addOption("ward",                   "Ward (Isotropic)"       );
    shadeMode.select("phong");
}

string getShaderDefine(string shadeMode, string functionName, string n, string pos, string lPos, string cPos, string ka, string kd, string ks) {
    string headerSource = "#define " + functionName + "(" + n + ", " + pos + ", " + lPos + ", " + cPos + ", " + ka + ", " + kd + ", " + ks + ") ";
    if (shadeMode == "none")
        headerSource += "" + ka + ";\n";
    else if (shadeMode == "phong-diffuse")
        headerSource += "phongShadingD(" + n + ", " + pos + ", " + lPos + ", " + cPos + ", " + kd + ");\n";
    else if (shadeMode == "phong-specular")
        headerSource += "phongShadingS(" + n + ", " + pos + ", " + lPos + ", " + cPos + ", " + ks + ");\n";
    else if (shadeMode == "phong-diffuse-ambient")
        headerSource += "phongShadingDA(" + n + ", " + pos + ", " + lPos + ", " + cPos + ", " + kd + ", " + ka + ");\n";
    else if (shadeMode == "phong-diffuse-specular")
        headerSource += "phongShadingDS(" + n + ", " + pos + ", " + lPos + ", " + cPos + ", " + kd + ", " + ks + ");\n";
    else if (shadeMode == "phong")
        headerSource += "phongShading(" + n + ", " + pos + ", " + lPos + ", " + cPos + ", " + ka + ", " + kd + ", " + ks + ");\n";
    else if (shadeMode == "toon")
        headerSource += "toonShading(" + n + ", " + pos + ", " + lPos + ", " + cPos + ", " + kd + ", 3);\n";
    else if (shadeMode == "cook-torrance")
        headerSource += "cookTorranceShading(" + n + ", " + pos + ", " + lPos + ", " + cPos + ", " + ka + ", " + kd + ", " + ks + ");\n";
    else if (shadeMode == "oren-nayar")
        headerSource += "orenNayarShading(" + n + ", " + pos + ", " + lPos + ", " + cPos + ", " + ka + ", " + kd + ");\n";
    else if (shadeMode == "lafortune")
        headerSource += "lafortuneShading(" + n + ", " + pos + ", " + lPos + ", " + cPos + ", " + ka + ", " + kd + ", " + ks + ");\n";
    else if (shadeMode == "ward")
        headerSource += "wardShading(" + n + ", " + pos + ", " + lPos + ", " + cPos + ", " + ka + ", " + kd + ", " + ks + ");\n";

    return headerSource;
}

} // namespace
