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

#include "slicerendererbase.h"

#include "tgt/tgt_math.h"
#include <limits>
#include <algorithm>
#include <sstream>

#include "tgt/tgt_gl.h"
#include "tgt/gpucapabilities.h"

#include "tgt/plane.h"
#include "tgt/glmath.h"
#include "tgt/textureunit.h"
#include "tgt/font.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/datastructures/volume/modality.h"
#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/ports/conditions/portconditionvolumetype.h"

using tgt::vec2;
using tgt::vec3;
using tgt::vec4;
using tgt::ivec2;
using tgt::ivec3;
using tgt::TextureUnit;

namespace {
template<typename T>
void copySliceData(const voreen::VolumeAtomic<T>* volume, tgt::Texture* sliceTexture,
    voreen::SliceAlignment sliceAlign, size_t sliceID, bool flipX, bool flipY);
}

namespace voreen {

const std::string SliceRendererBase::loggerCat_("voreen.base.SliceRendererBase");

SliceRendererBase::SliceRendererBase()
    : VolumeRenderer()
    , outport_(Port::OUTPORT, "image.outport", "Image Output", true, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_RECEIVER)
    , inport_(Port::INPORT, "volumehandle.volumehandle", "Volume Input")
    , legendPort_(Port::OUTPORT, "legendPort")
    , transferFunc_("transferFunction", "Transfer Function")
    , texMode_("textureMode", "Texture Mode", Processor::INVALID_PROGRAM)
    , sliceShader_(0)
    , legendShader_(0)
{
    inport_.addCondition(new PortConditionVolumeTypeGL());
    inport_.showTextureAccessProperties(true);
    addPort(inport_);
    addPort(outport_);
    addPrivateRenderPort(legendPort_); //< used for drawing the legend

    addProperty(transferFunc_);

    // texture mode (2D/3D)
    texMode_.addOption("2d-texture", "2D Textures", TEXTURE_2D);
    texMode_.addOption("3d-texture", "3D Texture", TEXTURE_3D);
    texMode_.selectByKey("3d-texture");
    addProperty(texMode_);
    texMode_.setGroupID(inport_.getID() + ".textureAccess");
}

void SliceRendererBase::initialize() throw (tgt::Exception) {
    VolumeRenderer::initialize();

    sliceShader_ = ShdrMgr.load("sl_base", buildShaderHeader(), false);
    LGL_ERROR;
    legendShader_ = ShdrMgr.loadSeparate("passthrouh.vert", "image/mapaoverb.frag",
        buildShaderHeader(), false);
    LGL_ERROR;

    adjustPropertyVisibilities();
}

void SliceRendererBase::deinitialize() throw (tgt::Exception) {
    ShdrMgr.dispose(sliceShader_);
    sliceShader_ = 0;
    ShdrMgr.dispose(legendShader_);
    legendShader_ = 0;
    VolumeRenderer::deinitialize();
}

// protected methods
//

void SliceRendererBase::beforeProcess() {
    VolumeRenderer::beforeProcess();

    if (inport_.hasChanged())
        adjustPropertyVisibilities();

    if (invalidationLevel_ >= Processor::INVALID_PROGRAM)
        rebuildShader();
}

bool SliceRendererBase::setupVolumeShader(tgt::Shader* shader, const VolumeBase* volumeHandle, TextureUnit* volUnit, TextureUnit* transferUnit,
    const tgt::Camera* camera, const tgt::vec4& lightPosition)
{
    tgtAssert(shader, "no shader");
    tgtAssert(volumeHandle, "no volumeHandle");
    tgtAssert(volUnit, "no volUnit");
    tgtAssert(transferUnit, "no transferUnit");

    // activate the shader and set the needed uniforms
    shader->activate();
    LGL_ERROR;

    transferFunc_.get()->setUniform(shader, "transferFunc_", "transferFuncTex_", transferUnit->getUnitNumber());
    shader->setUniform("numChannels_", static_cast<GLint>(volumeHandle->getNumChannels()));
    LGL_ERROR;

    // bind volume
    std::vector<VolumeStruct> volumeTextures;
    volumeTextures.push_back(VolumeStruct(
        volumeHandle,
        volUnit,
        "volume_","volumeStruct_",
        inport_.getTextureClampModeProperty().getValue(),
        tgt::vec4(inport_.getTextureBorderIntensityProperty().get()),
        inport_.getTextureFilterModeProperty().getValue())
    );
    bool success = bindVolumes(shader, volumeTextures, camera, lightPosition);
    LGL_ERROR;

    return success;
}

bool SliceRendererBase::setupSliceShader(tgt::Shader* shader, const VolumeBase* volumeHandle, const tgt::TextureUnit* transferUnit) {
    tgtAssert(shader, "no shader");
    tgtAssert(volumeHandle, "no volumehandle");
    tgtAssert(transferUnit, "no transferunit");

    // activate the shader and set the needed uniforms
    shader->activate();

    transferFunc_.get()->setUniform(shader, "transferFunc_", "transferFuncTex_", transferUnit->getUnitNumber());
    shader->setUniform("numChannels_", 1);

    RealWorldMapping rwm = volumeHandle->getRealWorldMapping();
    if (volumeHandle->hasRepresentation<VolumeRAM>()) {
        // map signed integer types from [-1.0:1.0] to [0.0:1.0] in order to avoid clamping of negative values
        // => the pixel transfer mapping (see below) has to be reverted in the shader (see bindSliceTexture)
        const VolumeRAM* volumeRAM = volumeHandle->getRepresentation<VolumeRAM>();
        if (volumeRAM->isInteger() && volumeRAM->isSigned()) {
            RealWorldMapping pixelTransferMapping(0.5f, 0.5f, "");
            rwm = RealWorldMapping::combine(pixelTransferMapping.getInverseMapping(), rwm);
        }
    }
    shader->setUniform("rwmScale_", rwm.getScale());
    shader->setUniform("rwmOffset_", rwm.getOffset());

    return true;
}

bool SliceRendererBase::bindSliceTexture(tgt::Shader* shader, const VolumeBase* volumeHandle,
    tgt::Texture* sliceTexture, tgt::TextureUnit* texUnit)
{
    tgtAssert(shader, "null pointer passed");
    tgtAssert(volumeHandle, "null pointer passed");
    tgtAssert(sliceTexture, "null pointer passed");
    tgtAssert(texUnit, "null pointer passed");

    // check state of slice shader
    if (!shader->isActivated()) {
        LERROR("bindSliceTexture() slice shader not activated");
        return false;
    }

    // pass number of texture channels to shader
    int numChannels = static_cast<int>(sliceTexture->getNumChannels());
    shader->setUniform("numChannels_", numChannels);

    // bind slicetexture to unit
    texUnit->activate();
    sliceTexture->bind();
    shader->setUniform("sliceTex_", texUnit->getUnitNumber());
    LGL_ERROR;

    // upload texture data
    const VolumeRAM* volumeRAM = 0;
    if (volumeHandle->hasRepresentation<VolumeRAM>())
        volumeRAM = volumeHandle->getRepresentation<VolumeRAM>();

    // map signed integer types from [-1.0:1.0] to [0.0:1.0] in order to avoid clamping of negative values
    if (volumeRAM && volumeRAM->isInteger() && volumeRAM->isSigned()) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPixelTransferf(GL_RED_SCALE,   0.5f);
        glPixelTransferf(GL_GREEN_SCALE, 0.5f);
        glPixelTransferf(GL_BLUE_SCALE,  0.5f);
        glPixelTransferf(GL_ALPHA_SCALE, 0.5f);

        glPixelTransferf(GL_RED_BIAS,    0.5f);
        glPixelTransferf(GL_GREEN_BIAS,  0.5f);
        glPixelTransferf(GL_BLUE_BIAS,   0.5f);
        glPixelTransferf(GL_ALPHA_BIAS,  0.5f);
    }

    sliceTexture->uploadTexture();
    LGL_ERROR;

    if (volumeRAM && volumeRAM->isInteger() && volumeRAM->isSigned()) {
        glPopAttrib();
    }

    // texture filtering
    GLint filterMode = static_cast<GLint>(inport_.getTextureFilterModeProperty().getValue());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    LGL_ERROR;

    // texture wrapping
    GLint clampMode = static_cast<GLint>(inport_.getTextureClampModeProperty().getValue());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampMode);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, static_cast<tgt::Vector4<GLfloat> >(inport_.getTextureBorderIntensityProperty().get()).elem);
    LGL_ERROR;

    // pass TextureParameters struct values to shader
    shader->setIgnoreUniformLocationError(true);
    shader->setUniform("sliceTexParameters_.dimensions_", tgt::vec2(sliceTexture->getDimensions().xy()));
    shader->setUniform("sliceTexParameters_.dimensionsRCP_", tgt::vec2(1.0f) / tgt::vec2(sliceTexture->getDimensions().xy()));
    shader->setUniform("sliceTexParameters_.matrix_", tgt::mat4::identity);
    shader->setIgnoreUniformLocationError(false);
    LGL_ERROR;

    return true;
}

std::string SliceRendererBase::buildShaderHeader() {
    std::string header = VolumeRenderer::generateHeader();

    if (texMode_.isSelected("2d-texture"))
        header += "#define SLICE_TEXTURE_MODE_2D \n";
    else if (texMode_.isSelected("3d-texture"))
        header += "#define SLICE_TEXTURE_MODE_3D \n";
    else {
        LWARNING("Unknown texture mode: " << texMode_.get());
    }

    header += transferFunc_.get()->getShaderDefines();

    return header;
}

bool SliceRendererBase::rebuildShader() {
    // do nothing if there is no shader at the moment
    if (!sliceShader_)
        return false;

    sliceShader_->setHeaders(buildShaderHeader());
    return sliceShader_->rebuild();
}

void SliceRendererBase::deactivateShader() {
    if (sliceShader_ && sliceShader_->isActivated())
        sliceShader_->deactivate();
}

void SliceRendererBase::adjustPropertyVisibilities() {
}

tgt::Texture* SliceRendererBase::generateAlignedSliceTexture(const VolumeBase* volumeHandle,
        const tgt::vec3& texLowerLeft, const tgt::vec3& texUpperRight) const {

    // get cpu volume
    if (!volumeHandle) {
        LERROR("generateSliceTexture(): volumeHandle must not be null");
        return 0;
    }
    const VolumeRAM* volume = volumeHandle->getRepresentation<VolumeRAM>();
    if (!volume) {
        LERROR("generateSliceTexture(): no CPU representation");
        return 0;
    }
    tgt::ivec3 volDim = volume->getDimensions();

    // check range of texcoord parameters
    if (!tgt::hand(tgt::greaterThanEqual(texLowerLeft, tgt::vec3(0.f))) ||
        !tgt::hand(tgt::lessThanEqual(texLowerLeft, tgt::vec3(1.f))) ) {
            LERROR("generateSliceTexture(): texLowerLeft outside valid range [0.0;1.0]^3");
            return 0;
    }
    if (!tgt::hand(tgt::greaterThanEqual(texUpperRight, tgt::vec3(0.f))) ||
        !tgt::hand(tgt::lessThanEqual(texUpperRight, tgt::vec3(1.f))) ) {
            LERROR("generateSliceTexture(): texUpperRight outside valid range [0.0;1.0]^3");
            return 0;
    }

    // convert float tex coords to voxel indices
    tgt::ivec3 voxelLowerLeft = tgt::iround(texLowerLeft*tgt::vec3(volDim-1));
    voxelLowerLeft = tgt::clamp(voxelLowerLeft, tgt::ivec3(0), volDim-1);
    tgt::ivec3 voxelUpperRight = tgt::iround(texUpperRight*tgt::vec3(volDim-1));
    voxelUpperRight = tgt::clamp(voxelUpperRight, tgt::ivec3(0), volDim-1);

    // determine slice orientation and position from voxel LF/UR
    // and generate slice texture
    SliceAlignment sliceAlign;
    tgt::ivec3 sliceTexDim;
    size_t sliceID;
    bool flipX = false;
    bool flipY = false;
    if (voxelLowerLeft.x == voxelUpperRight.x){
        // Y-Z-Slice
        sliceAlign = YZ_PLANE;
        sliceTexDim = tgt::ivec3(volDim.yz(), 1);
        sliceID = voxelLowerLeft.x;
        flipX = voxelLowerLeft.y > voxelUpperRight.y;
        flipY = voxelLowerLeft.z > voxelUpperRight.z;
    }
    else if (voxelLowerLeft.y == voxelUpperRight.y) {
        // X-Z-Slice
        sliceAlign = XZ_PLANE;
        sliceTexDim = tgt::ivec3(volDim.x, volDim.z, 1);
        sliceID = voxelLowerLeft.y;
        flipX = voxelLowerLeft.x > voxelUpperRight.x;
        flipY = voxelLowerLeft.z > voxelUpperRight.z;
    }
    else if (voxelLowerLeft.z == voxelUpperRight.z) {
        // X-Y-Slice
        sliceAlign = XY_PLANE;
        sliceTexDim = tgt::ivec3(volDim.xy(), 1);
        sliceID = voxelLowerLeft.z;
        flipX = voxelLowerLeft.x > voxelUpperRight.x;
        flipY = voxelLowerLeft.y > voxelUpperRight.y;
    }
    else {
        LERROR("generateSliceTexture(): slice " << voxelLowerLeft << "," << voxelUpperRight
            << " is not axis-aligned");
        return 0;
    }

    // create slice texture of the same type as the volume texture, and copy over tex data to slice
    tgt::Texture* sliceTex = 0;
    // scalar
    if (dynamic_cast<const VolumeAtomic<uint8_t>*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_ALPHA, GL_ALPHA8, GL_UNSIGNED_BYTE);
        LGL_ERROR;
        copySliceData<uint8_t>(static_cast<const VolumeAtomic<uint8_t>*>(volume), sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<int8_t>*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_ALPHA, GL_ALPHA8, GL_BYTE);
        LGL_ERROR;
        copySliceData<int8_t>(static_cast<const VolumeAtomic<int8_t>*>(volume), sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<uint16_t>*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_ALPHA, GL_ALPHA16, GL_UNSIGNED_SHORT);
        LGL_ERROR;
        copySliceData<uint16_t>(static_cast<const VolumeAtomic<uint16_t>*>(volume), sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<int16_t>*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_ALPHA, GL_ALPHA16, GL_SHORT);
        LGL_ERROR;
        copySliceData<int16_t>(static_cast<const VolumeAtomic<int16_t>*>(volume), sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<uint32_t>*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_ALPHA, GL_ALPHA, GL_UNSIGNED_INT);
        LGL_ERROR;
        copySliceData<uint32_t>(static_cast<const VolumeAtomic<uint32_t>*>(volume), sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<int32_t>*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_ALPHA, GL_ALPHA, GL_INT);
        LGL_ERROR;
        copySliceData<int32_t>(static_cast<const VolumeAtomic<int32_t>*>(volume), sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<uint64_t>*>(volume)) {
        LERROR("Texture data type 'uint64' not supported by OpenGL");
    }
    else if (dynamic_cast<const VolumeAtomic<int64_t>*>(volume)) {
        LERROR("Texture data type 'int64' not supported by OpenGL");
    }
    else if (dynamic_cast<const VolumeAtomic<float>*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_ALPHA, GL_ALPHA, GL_FLOAT);
        LGL_ERROR;
        copySliceData<float>(static_cast<const VolumeAtomic<float>*>(volume), sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<double>*>(volume)) {
        LERROR("Texture data type 'double' not supported by OpenGL");
    }
    // vec2
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<uint8_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE);
        LGL_ERROR;
        copySliceData<tgt::Vector2<uint8_t> >(static_cast<const VolumeAtomic<tgt::Vector2<uint8_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<int8_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_BYTE);
        LGL_ERROR;
        copySliceData<tgt::Vector2<int8_t> >(static_cast<const VolumeAtomic<tgt::Vector2<int8_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<uint16_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_SHORT);
        LGL_ERROR;
        copySliceData<tgt::Vector2<uint16_t> >(static_cast<const VolumeAtomic<tgt::Vector2<uint16_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<int16_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_SHORT);
        LGL_ERROR;
        copySliceData<tgt::Vector2<int16_t> >(static_cast<const VolumeAtomic<tgt::Vector2<int16_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<uint32_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_INT);
        LGL_ERROR;
        copySliceData<tgt::Vector2<uint32_t> >(static_cast<const VolumeAtomic<tgt::Vector2<uint32_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<int32_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_INT);
        LGL_ERROR;
        copySliceData<tgt::Vector2<int32_t> >(static_cast<const VolumeAtomic<tgt::Vector2<int32_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<uint64_t> >*>(volume)) {
        LERROR("Texture data type 'uint64' not supported by OpenGL");
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<int64_t> >*>(volume)) {
        LERROR("Texture data type 'int64' not supported by OpenGL");
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<float> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_FLOAT);
        LGL_ERROR;
        copySliceData<tgt::Vector2<float> >(static_cast<const VolumeAtomic<tgt::Vector2<float> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector2<double> >*>(volume)) {
        LERROR("Texture data type 'double' not supported by OpenGL");
    }
    // vec3
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<uint8_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGB, GL_RGB8, GL_UNSIGNED_BYTE);
        LGL_ERROR;
        copySliceData<tgt::Vector3<uint8_t> >(static_cast<const VolumeAtomic<tgt::Vector3<uint8_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<int8_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGB, GL_RGB8, GL_BYTE);
        LGL_ERROR;
        copySliceData<tgt::Vector3<int8_t> >(static_cast<const VolumeAtomic<tgt::Vector3<int8_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<uint16_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGB, GL_RGB16, GL_UNSIGNED_SHORT);
        LGL_ERROR;
        copySliceData<tgt::Vector3<uint16_t> >(static_cast<const VolumeAtomic<tgt::Vector3<uint16_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<int16_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGB, GL_RGB16, GL_SHORT);
        LGL_ERROR;
        copySliceData<tgt::Vector3<int16_t> >(static_cast<const VolumeAtomic<tgt::Vector3<int16_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<uint32_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGB, GL_RGB, GL_UNSIGNED_INT);
        LGL_ERROR;
        copySliceData<tgt::Vector3<uint32_t> >(static_cast<const VolumeAtomic<tgt::Vector3<uint32_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<int32_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGB, GL_RGB, GL_INT);
        LGL_ERROR;
        copySliceData<tgt::Vector3<int32_t> >(static_cast<const VolumeAtomic<tgt::Vector3<int32_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<uint64_t> >*>(volume)) {
        LERROR("Texture data type 'uint64' not supported by OpenGL");
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<int64_t> >*>(volume)) {
        LERROR("Texture data type 'int64' not supported by OpenGL");
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<float> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGB, GL_RGB, GL_FLOAT);
        LGL_ERROR;
        copySliceData<tgt::Vector3<float> >(static_cast<const VolumeAtomic<tgt::Vector3<float> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector3<double> >*>(volume)) {
        LERROR("Texture data type 'double' not supported by OpenGL");
    }
    // vec4
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<uint8_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
        LGL_ERROR;
        copySliceData<tgt::Vector4<uint8_t> >(static_cast<const VolumeAtomic<tgt::Vector4<uint8_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<int8_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGBA, GL_RGBA8, GL_BYTE);
        LGL_ERROR;
        copySliceData<tgt::Vector4<int8_t> >(static_cast<const VolumeAtomic<tgt::Vector4<int8_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<uint16_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT);
        LGL_ERROR;
        copySliceData<tgt::Vector4<uint16_t> >(static_cast<const VolumeAtomic<tgt::Vector4<uint16_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<int16_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGBA, GL_RGBA16, GL_SHORT);
        LGL_ERROR;
        copySliceData<tgt::Vector4<int16_t> >(static_cast<const VolumeAtomic<tgt::Vector4<int16_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<uint32_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGBA, GL_RGBA, GL_UNSIGNED_INT);
        LGL_ERROR;
        copySliceData<tgt::Vector4<uint32_t> >(static_cast<const VolumeAtomic<tgt::Vector4<uint32_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<int32_t> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGBA, GL_RGBA, GL_INT);
        LGL_ERROR;
        copySliceData<tgt::Vector4<int32_t> >(static_cast<const VolumeAtomic<tgt::Vector4<int32_t> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<uint64_t> >*>(volume)) {
        LERROR("Texture data type 'uint64' not supported by OpenGL");
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<int64_t> >*>(volume)) {
        LERROR("Texture data type 'int64' not supported by OpenGL");
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<float> >*>(volume)) {
        sliceTex = new tgt::Texture(sliceTexDim, GL_RGBA, GL_RGBA, GL_FLOAT);
        LGL_ERROR;
        copySliceData<tgt::Vector4<float> >(static_cast<const VolumeAtomic<tgt::Vector4<float> >*>(volume),
            sliceTex, sliceAlign, sliceID, flipX, flipY);
    }
    else if (dynamic_cast<const VolumeAtomic<tgt::Vector4<double> >*>(volume)) {
        LERROR("Texture data type 'double' not supported by OpenGL");
    }
    else {
        LERROR("generateSliceTexture(): unknown or unsupported volume type");
    }

    LGL_ERROR;
    return sliceTex;
}

tgt::Texture* SliceRendererBase::createDistanceLegend(const VolumeBase* volumeHandle, tgt::vec4 legendColor, tgt::vec2 legendPos) {
    tgtAssert(volumeHandle, "null pointer passed");

    //if outport is active, deactived it
    bool wasActive = false;
    if(outport_.isActive()){
        outport_.deactivateTarget();
        wasActive = true;
    }

    //set size of the legend texture
    int sizeX = static_cast<int>(volumeHandle->getDimensions().x);
    int sizeY = static_cast<int>(volumeHandle->getDimensions().y);
    while((sizeX < 256) || (sizeY < 256)){
        sizeX *= 2; sizeY *= 2;
    }
    legendPort_.resize(tgt::ivec2(sizeX,sizeY));

    float fontOffset = 0.03f;
    float lPercent = 10.f;  //how huge the legend should be 100% / lPercent
    float eValue; //exact length
    float lengthNormalized; //length normalized to [0,2]
    //setting values of the right dimension
    float helpSpacing = volumeHandle->getSpacing().x;
    size_t helpDim = volumeHandle->getDimensions().x;

    eValue = helpDim*helpSpacing/lPercent;
    float legendValue;
    std::string legendStr;
    scaleLegend(eValue, legendValue, legendStr);
    lengthNormalized = 2*legendValue/(helpDim*helpSpacing);

    //start new opengl context
    legendPort_.activateTarget();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

        glClearColor(legendColor.r, legendColor.g, legendColor.b, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //render text
        glPushMatrix();
            glTranslatef(-1,-1,0);
            float scaleFactorX = 2.0f / static_cast<float>(legendPort_.getSize().x);
            float scaleFactorY = 2.0f / static_cast<float>(legendPort_.getSize().y);
            glScalef(scaleFactorX, scaleFactorY, 1);
            glColor4f(legendColor.r, legendColor.g, legendColor.b, legendColor.a);
            tgt::Font* legendFont = new tgt::Font(VoreenApplication::app()->getFontPath("VeraMono.ttf"),
                static_cast<int>(std::min(legendPort_.getSize().y, legendPort_.getSize().x)/(2.5*lPercent)),
                tgt::Font::ExtrudeFont,
                static_cast<float>(((lengthNormalized+fontOffset*4)/2.f)*legendPort_.getSize().x),
                tgt::Font::Center,
                tgt::Font::Middle);
            std::stringstream strstr;
            strstr << legendStr;
            legendFont->render(tgt::vec3(sizeX*(legendPos.x-fontOffset), sizeY*(legendPos.y-0.05f), 0.f), strstr.str());
            delete legendFont;
        glPopMatrix();

        //render lines
        glPushMatrix();
            glBegin(GL_LINES);
                glVertex2f(legendPos.x*2-1,                     legendPos.y*2-1);
                glVertex2f(legendPos.x*2-1+lengthNormalized,    legendPos.y*2-1);
                glVertex2f(legendPos.x*2-1,                     legendPos.y*2-1+lengthNormalized/10.f);
                glVertex2f(legendPos.x*2-1,                     legendPos.y*2-1-lengthNormalized/10.f);
                glVertex2f(legendPos.x*2-1+lengthNormalized,    legendPos.y*2-1+lengthNormalized/10.f);
                glVertex2f(legendPos.x*2-1+lengthNormalized,    legendPos.y*2-1-lengthNormalized/10.f);
            glEnd();
        glPopMatrix();
    //reset opengl context
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glPopAttrib();
    legendPort_.deactivateTarget();

    if (wasActive)
        outport_.activateTarget();

    return legendPort_.getColorTexture();
}

void SliceRendererBase::drawDistanceLegend(float xl, float xr, float yd, float yu) {
    legendShader_->activate();
    setGlobalShaderParameters(legendShader_);

    // image texture parameters
    TextureUnit colorU, depthU, colorO, depthO;
    outport_.deactivateTarget();
    outport_.bindTextures(colorU,depthU);
    outport_.activateTarget();
    legendPort_.bindTextures(colorO, depthO);
    outport_.setTextureParameters(legendShader_, "textureParameters0_");
    legendShader_->setUniform("colorTex0_", colorU.getUnitNumber());
    legendShader_->setUniform("depthTex0_", depthU.getUnitNumber());
    legendShader_->setUniform("colorTex1_", colorO.getUnitNumber());
    legendShader_->setUniform("depthTex1_", depthO.getUnitNumber());

    glDepthFunc(GL_ALWAYS);
    glBegin(GL_QUADS);
        glTexCoord2f(0,1); glVertex2f(xl,yu);
        glTexCoord2f(1,1); glVertex2f(xr,yu);
        glTexCoord2f(1,0); glVertex2f(xr,yd);
        glTexCoord2f(0,0); glVertex2f(xl,yd);
    glEnd();
    glDepthFunc(GL_LESS);

    legendShader_->deactivate();
}

void SliceRendererBase::scaleLegend(float oldValue, float &outFloat, std::string& outStr) {
    std::stringstream strstr;

    if(oldValue > 1000000.f){ //km
        oldValue = floorf(oldValue/100000.f+0.5f)/10.f;
        strstr << oldValue << "km";
        oldValue *= 1000000.f;
    } else if (oldValue > 1000.f){ //m
        oldValue = floorf(oldValue/100.f+0.5f)/10.f;
        strstr << oldValue << "m";
        oldValue *= 1000.f;
    } else if (oldValue > 10.f){ //cm
        oldValue = floorf(oldValue+0.5f)/10.f;
        strstr << oldValue << "cm";
        oldValue *= 10.f;
    } else if (oldValue < 0.0001f){ //nm
        oldValue = floorf(oldValue*10000000.f+0.5f)/10.f;
        strstr << oldValue << "nm";
        oldValue /= 1000000.f;
    } else if (oldValue < 0.1f){ //muem
        oldValue = floorf(oldValue*10000.f+0.5f)/10.f;
        strstr << oldValue << "µm";
        oldValue /= 1000.f;
    }  else { //mm
        oldValue = floorf(oldValue*10.f+0.5f)/10.f;
        strstr << oldValue << "mm";
    }
    outFloat = oldValue;
    outStr = strstr.str();
}

}   // namespace voreen


namespace {

// template definitions
template<typename T>
void copySliceData(const voreen::VolumeAtomic<T>* volume, tgt::Texture* sliceTexture,
        voreen::SliceAlignment sliceAlign, size_t sliceID, bool flipX, bool flipY) {

    tgtAssert(volume, "volume is null");
    tgtAssert(sliceTexture, "sliceTexture is null");
    tgtAssert(sliceTexture->getPixelData(), "sliceTexture pixel data is null");
    tgtAssert(sliceTexture->getDimensions().z == 1, "2D texture expected");

    tgt::ivec3 volDim = volume->getDimensions();
    tgt::ivec2 sliceDim = sliceTexture->getDimensions().xy();
    switch (sliceAlign) {
    case voreen::YZ_PLANE:
        {
            tgtAssert(volDim.yz() == sliceDim, "dimensions mismatch");
            tgtAssert(static_cast<int>(sliceID) < volDim.x, "invalid slice id");
            for (size_t y=0; y<static_cast<size_t>(sliceDim.y); y++) {
                for (size_t x=0; x<static_cast<size_t>(sliceDim.x); x++) {
                    sliceTexture->texel<T>(x, y) = volume->voxel(sliceID,
                        !flipX ? x : (sliceDim.x-1) - x, !flipY ? y : (sliceDim.y-1) - y);
                }
            }
        }
        break;
    case voreen::XZ_PLANE:
        {
            tgtAssert(volDim.x == sliceDim.x && volDim.z == sliceDim.y, "dimensions mismatch");
            tgtAssert(static_cast<int>(sliceID) < volDim.y, "invalid slice id");
            for (size_t y=0; y<static_cast<size_t>(sliceDim.y); y++) {
                for (size_t x=0; x<static_cast<size_t>(sliceDim.x); x++) {
                    sliceTexture->texel<T>(x, y) = volume->voxel(!flipX ? x : (sliceDim.x-1) - x, sliceID,
                        !flipY ? y : (sliceDim.y-1) - y);
                }
            }
        }
        break;
    case voreen::XY_PLANE:
        {
            tgtAssert(volDim.xy() == sliceDim, "dimensions mismatch");
            tgtAssert(static_cast<int>(sliceID) < volDim.z, "invalid slice id");
            for (size_t y=0; y<static_cast<size_t>(sliceDim.y); y++) {
                for (size_t x=0; x<static_cast<size_t>(sliceDim.x); x++) {
                    sliceTexture->texel<T>(x, y) = volume->voxel(!flipX ? x : (sliceDim.x-1) - x,
                        !flipY ? y : (sliceDim.y-1) - y, sliceID);
                }
            }
        }
        break;
    default:
        tgtAssert(false, "invalid slice alignment");

    }

}



} // namespace anonymous

