/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/core/datastructures/transfunc/transfunc.h"

#include "tgt/gpucapabilities.h"
#include "tgt/shadermanager.h"

namespace voreen {

const std::string TransFunc::loggerCat_("voreen.TransFunc");

TransFunc::TransFunc(int width, int height, int depth,
                     GLint format, GLenum dataType, tgt::Texture::Filter filter)
    : tex_(0)
    , dimensions_(width, height, depth)
    , format_(format)
    , dataType_(dataType)
    , filter_(filter)
    , textureInvalid_(true)
{
    //fitDimensions(dimensions_.x, dimensions_.y, dimensions_.z);
}

TransFunc::~TransFunc() {
    delete tex_;
    LGL_ERROR;
}

const std::vector<std::string>& TransFunc::getLoadFileFormats() const {
    return loadFileFormats_;
}

const std::vector<std::string>& TransFunc::getSaveFileFormats() const {
    return saveFileFormats_;
}

void TransFunc::invalidateTexture() {
    textureInvalid_ = true;
}

void TransFunc::reset() {
    createTex();
}

bool TransFunc::isTextureInvalid() const {
    return textureInvalid_;
}

void TransFunc::bind() {
    if (textureInvalid_)
        updateTexture();

    tgtAssert(tex_, "No texture");
    tex_->bind();
}

void TransFunc::updateTexture() {

    if (!tex_ || (tex_->getDimensions() != dimensions_))
        createTex();

    if (!tex_) {
        LERROR("Failed to create texture");
        return;
    }

    tex_->uploadTexture();
    textureInvalid_ = false;
}

void TransFunc::createTex() {
    delete tex_;

    tex_ = new tgt::Texture(dimensions_, format_, dataType_, filter_);
    tex_->setWrapping(tgt::Texture::CLAMP);
    LGL_ERROR;
}

tgt::Texture* TransFunc::getTexture() {
    if (textureInvalid_)
        updateTexture();

    return tex_;
}

std::string TransFunc::getSamplerType() const {
    if (dimensions_.z > 1)
        return "sampler3D";
    else if (dimensions_.y > 1)
        return "sampler2D";
    else
        return "sampler1D";
}

void TransFunc::setUniform(tgt::Shader* shader, const std::string& uniform, const std::string& uniformTex, const GLint texUnit) {
    tgtAssert(shader, "Null pointer passed");
    bool oldIgnoreError = shader->getIgnoreUniformLocationError();
    shader->setIgnoreUniformLocationError(true);
    shader->setUniform(uniformTex, texUnit);

    if(getNumDimensions() == 1) {
        shader->setUniform(uniform + ".domainLower_", tgt::vec3(getDomain(0).x));
        shader->setUniform(uniform + ".domainUpper_", tgt::vec3(getDomain(0).y));
    }
    else if(getNumDimensions() == 2) {
        shader->setUniform(uniform + ".domainLower_", tgt::vec3(getDomain(0).x, getDomain(1).x, 0.0f));
        shader->setUniform(uniform + ".domainUpper_", tgt::vec3(getDomain(0).y, getDomain(1).y, 0.0f));
    }
    else {
        LERROR("Unhandled dimensionality in glsl TransFunc object");
    }

    shader->setIgnoreUniformLocationError(oldIgnoreError);
}

std::string TransFunc::getShaderDefines() const {
    return "#define TF_SAMPLER_TYPE " + getSamplerType() + "\n";
}

void TransFunc::resize(int width, int height, int depth) {
    fitDimensions(width, height, depth);

    if (width != dimensions_.x) {
        dimensions_.x = width;
        textureInvalid_ = true;
    }
    if (height != dimensions_.y) {
        dimensions_.y = height;
        textureInvalid_ = true;
    }
    if (depth != dimensions_.z) {
        dimensions_.z = depth;
        textureInvalid_ = true;
    }
}

tgt::ivec3 TransFunc::getDimensions() const {
    return dimensions_;
}

void TransFunc::fitDimensions(int& width, int& height, int& depth) const {
    int maxTexSize;
    if (depth == 1)
        maxTexSize = GpuCaps.getMaxTextureSize();
    else
        maxTexSize = GpuCaps.getMax3DTextureSize();

    if (maxTexSize < width)
        width = maxTexSize;

    if (maxTexSize < height)
        height = maxTexSize;

    if (maxTexSize < depth)
        depth = maxTexSize;
}

bool TransFunc::load(const std::string& /*filename*/) {
    return false;  // override in a subclass
}

void TransFunc::serialize(XmlSerializer& /*s*/) const {
    // nothing to serialize here
}

void TransFunc::deserialize(XmlDeserializer& /*s*/) {
    // nothing to deserialize here
}

void TransFunc::setPixelData(GLubyte* data) {

    if (!tex_ || (tex_->getDimensions() != dimensions_))
        createTex();
    tgtAssert(tex_, "No texture");

    if (tex_->getPixelData() != reinterpret_cast<GLubyte*>(data))
        tex_->destroy();
    tex_->setPixelData(data);

    textureInvalid_ = true;
}

GLubyte* TransFunc::getPixelData() {
    if (textureInvalid_)
        updateTexture();
    tgtAssert(tex_, "No texture");
    return tex_->getPixelData();
}

TransFunc* TransFunc::clone() const {
    TransFunc* func = new TransFunc(dimensions_.x,dimensions_.y,dimensions_.z);
    func->dimensions_.x = dimensions_.x;
    func->dimensions_.y = dimensions_.y;
    func->dimensions_.z = dimensions_.z;
    func->format_ = format_;
    func->dataType_ = dataType_;
    func->filter_ = filter_;

    TransFunc* tempthis = const_cast<TransFunc*>(this);
    GLubyte* pixels = tempthis->getPixelData();
    int factor = tempthis->getTexture()->getBpp();
    GLubyte* newpixels = new GLubyte[tgt::hmul(dimensions_) * factor];
    for (int i=0;i<dimensions_.x*dimensions_.y*dimensions_.z*factor;i++) {
        newpixels[i] = pixels[i];
    }
    func->setPixelData(newpixels);

    return func;
}

GLint TransFunc::getFormat() const {
    return format_;
}

tgt::vec2 TransFunc::getDomain(int dimension /*= 0*/) const {
    return tgt::vec2(0.0f, 1.0f);
}

void TransFunc::setDomain(tgt::vec2 domain, int dimension /*= 0*/) {
    // no-op
}

void TransFunc::setDomain(float lower, float upper, int dimension) {
    setDomain(tgt::vec2(lower, upper), dimension);
}

float TransFunc::realWorldToNormalized(float rw, int dimension) const {
    tgt::vec2 domain = getDomain(dimension);

    if(rw < domain.x)
        return 0.0f;
    else if(rw > domain.y)
        return 1.0f;
    else {
        if(domain.y == domain.x){
            LERROR("Transfer Function Domain Length is 0! Causes Division by Zero!");
            tgtAssert(false, "Transfer Function Domain Length is 0! Causes Division by Zero!");
            return 1.0f;
        } else
            return (rw - domain.x) / (domain.y - domain.x);
    }
}

float TransFunc::normalizedToRealWorld(float n, int dimension) const {
    tgt::vec2 domain = getDomain(dimension);

    return domain.x + (domain.y - domain.x) * n;
}

} // namespace voreen
