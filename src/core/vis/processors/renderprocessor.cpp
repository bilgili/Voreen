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

#include "voreen/core/vis/processors/renderprocessor.h"

#include "tgt/glmath.h"
#include "tgt/camera.h"
#include "tgt/shadermanager.h"
#include "tgt/gpucapabilities.h"

#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/vis/lightmaterial.h"

#include <sstream>

using tgt::vec3;
using tgt::vec4;
using tgt::Color;

namespace voreen {

const Identifier RenderProcessor::setBackgroundColor_("set.backgroundColor");

const std::string RenderProcessor::loggerCat_("voreen.RenderProcessor");

const std::string RenderProcessor::XmlElementName_("RenderProcessor");

RenderProcessor::RenderProcessor(tgt::Camera* camera, TextureContainer* tc)
    : Processor()
    , tc_(tc)
    , size_(0.f)
    , camera_(camera)
    , backgroundColor_(setBackgroundColor_, "Background color", tgt::Color(0.0f, 0.0f, 0.0f, 0.0f))
    , lightPosition_(LightMaterial::setLightPosition_, "Light source position", tgt::vec4(2.3f, 1.5f, 1.5f, 1.f),
                     tgt::vec4(-10), tgt::vec4(10))
    , lightAmbient_(LightMaterial::setLightAmbient_, "Ambient light", tgt::Color(0.4f, 0.4f, 0.4f, 1.f))
    , lightDiffuse_(LightMaterial::setLightDiffuse_, "Diffuse light", tgt::Color(0.8f, 0.8f, 0.8f, 1.f))
    , lightSpecular_(LightMaterial::setLightSpecular_, "Specular light", tgt::Color(0.6f, 0.6f, 0.6f, 1.f))
    , lightAttenuation_(LightMaterial::setLightAttenuation_, "Attenutation", tgt::vec3(1.f, 0.f, 0.f))
    , materialAmbient_(LightMaterial::setMaterialAmbient_, "Ambient material color", tgt::Color(1.f, 1.f, 1.f, 1.f))
    , materialDiffuse_(LightMaterial::setMaterialDiffuse_, "Diffuse material color", tgt::Color(1.f, 1.f, 1.f, 1.f))
    , materialSpecular_(LightMaterial::setMaterialSpecular_, "Specular material color", tgt::Color(1.f, 1.f, 1.f, 1.f))
    , materialEmission_(LightMaterial::setMaterialEmission_, "Emissive material color", tgt::Color(0.f, 0.f, 0.f, 1.f))
    , materialShininess_(LightMaterial::setMaterialShininess_, "Shininess", 60.f, 0.1f, 128.f)
{
    //addProperty(&lightPosition_);
    //addProperty(&backgroundColor_);
    //addProperty(&lightAmbient_);
    //addProperty(&lightDiffuse_);
    //addProperty(&lightSpecular_);
    //addProperty(&lightAttenuation_);
    //addProperty(&materialAmbient_);
    //addProperty(&materialDiffuse_);
    //addProperty(&materialSpecular_);
    //addProperty(&materialEmission_);
    //addProperty(&materialShininess_);
}

RenderProcessor::~RenderProcessor() {
}


void RenderProcessor::processMessage(Message* msg, const Identifier& dest) {
    MessageReceiver::processMessage(msg, dest);

    if (msg->id_ == setBackgroundColor_) {
        backgroundColor_.set(msg->getValue<tgt::Color>());
    }
    else if (msg->id_ == LightMaterial::setLightPosition_) {
        lightPosition_.set(msg->getValue<vec4>());
    }   
    else if (msg->id_ == "set.viewport") {
        // needed for benchmark script fps.py
        if (camera_) {
            tgt::ivec2 v = msg->getValue<tgt::ivec2>();
            glViewport(0, 0, v.x, v.y);
            setSizeTiled(v.x, v.y);
            if (tc_)
                tc_->setSize(v);

            LINFO("set.viewport to " << v);
        }
    }
    
}

void RenderProcessor::setTextureContainer(TextureContainer* tc) {
    tc_ = tc;
}

void RenderProcessor::setCamera(tgt::Camera* camera) {
    camera_ = camera;
}

tgt::Camera* RenderProcessor::getCamera() const {
    return camera_;
}

TextureContainer* RenderProcessor::getTextureContainer() {
    return tc_;
}

void RenderProcessor::setGeometryContainer(GeometryContainer* geoCont) {
    geoContainer_ = geoCont;
}

GeometryContainer* RenderProcessor::getGeometryContainer() const {
    return geoContainer_;
}

void RenderProcessor::setSize(const tgt::ivec2 &size) {
    setSize(static_cast<tgt::vec2>(size));
}

void RenderProcessor::setSize(const tgt::vec2& size) {
    if (camera_ && (size_ != size)) {
        size_ = size;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        camera_->getFrustum().setRatio(size.x/size.y);
        camera_->updateFrustum();
        tgt::loadMatrix(camera_->getProjectionMatrix());

        glMatrixMode(GL_MODELVIEW);

        invalidate();
    }
}

tgt::ivec2 RenderProcessor::getSize() const {
    return static_cast<tgt::ivec2>(size_);
}

tgt::vec2 RenderProcessor::getSizeFloat() const {
    return size_;
}

void RenderProcessor::renderQuad() {
    glBegin(GL_QUADS);
        glVertex2f(-1.0, -1.0);
        glVertex2f( 1.0, -1.0);
        glVertex2f( 1.0,  1.0);
        glVertex2f(-1.0,  1.0);
    glEnd();
}

std::string RenderProcessor::generateHeader() {
    std::string header;

    // use highest available shading language version up to version 1.30
    using tgt::GpuCapabilities;
    if (GpuCaps.getShaderVersion() >= GpuCapabilities::GlVersion::SHADER_VERSION_130) {
        header += "#version 130\n";
        header += "precision highp float;\n";
    }
    else if (GpuCaps.getShaderVersion() == GpuCapabilities::GlVersion::SHADER_VERSION_120)
        header += "#version 120\n";
    else if (GpuCaps.getShaderVersion() == GpuCapabilities::GlVersion::SHADER_VERSION_110)
        header += "#version 110\n";
    
    if (tc_) {
        if (tc_->getTextureContainerTextureType() == TextureContainer::VRN_TEXTURE_2D)
            header += "#define VRN_TEXTURE_2D\n";
        else if (tc_->getTextureContainerTextureType() == TextureContainer::VRN_TEXTURE_RECTANGLE)
            header += "#define VRN_TEXTURE_RECTANGLE\n";
    }

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

    return header;
}

// Parameters currently set:
// - screenDim_
// - screenDimRCP_
// - cameraPosition_ (camera position in world coordinates)
// - lightPosition_ (light source position in world coordinates)
void RenderProcessor::setGlobalShaderParameters(tgt::Shader* shader) {
    shader->setIgnoreUniformLocationError(true);

    if (tc_) {
        shader->setUniform("screenDim_", tgt::vec2(tc_->getSize()));
        shader->setUniform("screenDimRCP_", 1.f / tgt::vec2(tc_->getSize()));
    }

    // camera position in world coordinates
    shader->setUniform("cameraPosition_", camera_->getPosition());

    // light source position in world coordinates
    shader->setUniform("lightPosition_", getLightPosition());

    shader->setIgnoreUniformLocationError(false);
}

void RenderProcessor::setLightingParameters() {
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient_.get().elem);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse_.get().elem);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular_.get().elem);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, lightAttenuation_.get().x);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, lightAttenuation_.get().y);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, lightAttenuation_.get().z);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmission_.get().elem);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess_.get());
}

void RenderProcessor::setSizeTiled(uint width, uint height) {
    setSize(tgt::ivec2(width, height));

    // gluPerspective replacement taken from
    // http://nehe.gamedev.net/data/articles/article.asp?article=11

    float fovY = 45.f;
    float aspect = static_cast<float>(width) / height;
    float zNear = 0.1f;
    float zFar = 50.f;

    float fw, fh;
    fh = tanf(fovY / 360 * tgt::PIf) * zNear;
    fw = fh * aspect;

    tgt::Frustum frust_ = tgt::Frustum(-fw, fw, - fh, fh, zNear, zFar);
    camera_->setFrustum(frust_);
    camera_->updateFrustum();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    tgt::loadMatrix(camera_->getProjectionMatrix());
    glMatrixMode(GL_MODELVIEW);
}

tgt::vec3 RenderProcessor::getLightPosition() const {
    return tgt::vec3(lightPosition_.get().x, lightPosition_.get().y, lightPosition_.get().z);
}

} // namespace voreen
