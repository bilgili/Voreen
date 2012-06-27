/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/modules/base/processors/render/glslraycaster.h"

#include "voreen/core/properties/cameraproperty.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

using tgt::vec3;

namespace voreen {

GLSLRaycaster::GLSLRaycaster()
    : VolumeRaycaster()
    , volumePort_(Port::INPORT, "volumehandle.volumehandle")
    , entryPort_(Port::INPORT, "image.entrypoints")
    , exitPort_(Port::INPORT, "image.exitpoints")
    , outport_(Port::OUTPORT, "image.output", true)
    , transferFunc_("transferFunction", "Transfer function")
    , shader_("shader", "Fragment shader", "rc_simple.frag")
    , camera_("camera", "Camera", new tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
{
    addPort(volumePort_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);

    addProperty(shader_);
    addProperty(transferFunc_);
    addProperty(camera_);

    addProperty(lightPosition_);
    addProperty(lightAmbient_);
    addProperty(lightDiffuse_);
    addProperty(lightSpecular_);
}

std::string GLSLRaycaster::getProcessorInfo() const {
    return "Performs a simple single pass raycasting which can be modified by changing the shader property.";
}

Processor* GLSLRaycaster::create() const {
    return new GLSLRaycaster();
}

void GLSLRaycaster::initialize() throw (VoreenException) {

    // assign header and create shader
    //Add definition of sampler type to allow shader compilation:
    initialized_ = true; //< prevent warning
    shader_.setHeader(generateHeader() + "\n #define TF_SAMPLER_TYPE sampler1D\n");
    initialized_ = false;
    VolumeRaycaster::initialize();  // initializes the shader and transfunc properties

    tgt::Shader* sh = shader_.getShader();
    if (!sh || !sh->isLinked()) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }

    initialized_ = true;
}

void GLSLRaycaster::loadShader() {
    shader_.rebuild();
}

void GLSLRaycaster::compile(VolumeHandle* volumeHandle) {
    shader_.setHeader(generateHeader(volumeHandle));
    shader_.rebuild();
}

void GLSLRaycaster::process() {
    if (!volumePort_.isReady())
        return;

    if (!outport_.isReady())
        return;

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile(volumePort_.getData());

    tgt::Shader* sh = shader_.getShader();
    if (!sh || !sh->isLinked())
        return;

    outport_.activateTarget();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    transferFunc_.setVolumeHandle(volumePort_.getData());

    TextureUnit entryUnit, entryDepthUnit, exitUnit, exitDepthUnit;
    // bind entry params
    entryPort_.bindTextures(entryUnit.getEnum(), entryDepthUnit.getEnum());
    LGL_ERROR;

    // bind exit params
    exitPort_.bindTextures(exitUnit.getEnum(), exitDepthUnit.getEnum());
    LGL_ERROR;

    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

    // add main volume
    TextureUnit volUnit;
    volumeTextures.push_back(VolumeStruct(
        volumePort_.getData()->getVolumeGL(),
        &volUnit,
        "volume_",
        "volumeParameters_")
    );

    updateBrickingParameters(volumePort_.getData());
    addBrickedVolumeModalities(volumePort_.getData(), volumeTextures);

    // segmentation volume
    // TODO: fetch from inport
    //VolumeGL* volumeSeg = volumePort_.getData()->getRelatedVolumeGL(Modality::MODALITY_SEGMENTATION);
    VolumeGL* volumeSeg = 0;

    TextureUnit segUnit;
    if (useSegmentation_.get() && (volumeSeg != 0) ) {
        volumeTextures.push_back(VolumeStruct(
            volumeSeg,
            &segUnit,
            "segmentation_",
            "segmentationParameters_")
        );

        segUnit.activate();

        // set texture filters for this texture
        //FIXME: this does NOTHING! is this the right place to set filtering for segmentation?
        glPushAttrib(GL_TEXTURE_BIT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glPopAttrib();
    }

    // bind transfer function
    TextureUnit transferUnit;
    transferUnit.activate();
    if (transferFunc_.get())
        transferFunc_.get()->bind();

    // initialize shader
    sh->activate();

    // set common uniforms used by all shaders
    setGlobalShaderParameters(sh, camera_.get());
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(sh, volumeTextures);

    // pass the remaining uniforms to the shader
    sh->setUniform("entryPoints_", entryUnit.getUnitNumber());
    sh->setUniform("entryPointsDepth_", entryDepthUnit.getUnitNumber());
    sh->setUniform("exitPoints_", exitUnit.getUnitNumber());
    sh->setUniform("exitPointsDepth_", entryDepthUnit.getUnitNumber());
    sh->setUniform("transferFunc_", transferUnit.getUnitNumber());
    if (useSegmentation_.get() && volumeSeg)
        sh->setUniform("segment_" , static_cast<float>(segment_.get()));

    setBrickedVolumeUniforms(volumePort_.getData());

    renderQuad();

    sh->deactivate();

    outport_.deactivateTarget();
    LGL_ERROR;

    glActiveTexture(GL_TEXTURE0);
}

std::string GLSLRaycaster::generateHeader(VolumeHandle* volumeHandle) {
    std::string header = VolumeRaycaster::generateHeader(volumeHandle);

    if(transferFunc_.get())
        header += transferFunc_.get()->getShaderDefines();

    return header;
}

} // namespace voreen
