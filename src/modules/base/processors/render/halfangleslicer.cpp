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

#include "voreen/modules/base/processors/render/halfangleslicer.h"
#include "voreen/core/datastructures/transfunc/transfuncintensity.h"
#include "tgt/glmath.h"
#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

HalfAngleSlicer::HalfAngleSlicer()
    : VolumeSlicer()
    , slicingPrg_(0)
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle", false, Processor::INVALID_PROGRAM)
    , outport_(Port::OUTPORT, "image.output", true, Processor::INVALID_RESULT, GL_RGBA16F_ARB)
    , lightport_(Port::OUTPORT, "image.lightport", true, Processor::INVALID_PROGRAM, GL_RGBA16F_ARB)
    , cameraHandler_("cameraHandler", "Camera Handler", &camera_)
    , lightCamera_("light_camera", "Light Camera", new tgt::Camera(tgt::vec3(0.f, 0.f, 3.5f), tgt::vec3(0.f, 0.f, 0.f), tgt::vec3(0.f, 1.f, 0.f)))
    , halfLight_("halfLight", "Light source position", tgt::vec4(2.3f, 1.5f, 1.5f, 1.f), tgt::vec4(-15), tgt::vec4(15))
{

    addProperty(halfLight_);
    halfLight_.setViews(Property::View(Property::LIGHT_POSITION | Property::DEFAULT));

    addInteractionHandler(cameraHandler_);

    addPort(volumeInport_);
    addPort(outport_);
    addPrivateRenderPort(lightport_);
}

Processor* HalfAngleSlicer::create() const {
    return new HalfAngleSlicer(); 
}

std::string HalfAngleSlicer::getProcessorInfo() const {
    return "This class implements a slicing based volume renderer including shadows, based on the work of Joe Kniss.";
}

void HalfAngleSlicer::initialize() throw (VoreenException) {
    VolumeSlicer::initialize();

    loadShader();

    if (!slicingPrg_) {
        LERROR("Failed to load shader!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shader!");
    }
    initialized_ = true;
}

void HalfAngleSlicer::deinitialize() throw (VoreenException) {
    ShdrMgr.dispose(slicingPrg_);

    VolumeSlicer::deinitialize();
}

void HalfAngleSlicer::loadShader() {
    slicingPrg_ = ShdrMgr.loadSeparate("sl_singlevolume.vert", "sl_halfslicing.frag",
        generateHeader(), false);
}

void HalfAngleSlicer::compile() {
    slicingPrg_->setHeaders(generateHeader());
    slicingPrg_->rebuild();
}

bool HalfAngleSlicer::isReady() const {
    //check if all inports are connected:
    if(!volumeInport_.isReady())
        return false;

    //check if outport is connected:
    if (!outport_.isReady())
        return false;

    return true;
}

void HalfAngleSlicer::process() {

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
    LGL_ERROR;

    lightport_.activateTarget();
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    outport_.activateTarget();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind transfer function
    TextureUnit transferUnit;
    transferUnit.activate();
    if (transferFunc_.get())
        transferFunc_.get()->bind();

    transferFunc_.setVolumeHandle(volumeInport_.getData());

    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

    // add main volume
    TextureUnit volUnit;
    volumeTextures.push_back(VolumeStruct(
        volumeInport_.getData()->getVolumeGL(),
        &volUnit,
        "volume_",
        "volumeParameters_",
        true)
    );

    // compute the distance between two adjacent slices in texture coordinates
    float sliceDistance = 0.1f;

    // acquire volume size for sampling
    if (volumeTextures.size() > 0) {
        tgt::ivec3 dim = volumeTextures[0].volume_->getTexture()->getDimensions();
        sliceDistance = 1.0f / (tgt::min(dim) * samplingRate_.get());
    }

    // initialize slicing shader
    slicingPrg_->activate();

    // fragment shader uniforms
    slicingPrg_->setUniform("transferFunc_", transferUnit.getUnitNumber());

    //clipping uniforms
    slicingPrg_->setUniform("nSequence_", nSeq_, 64);
    slicingPrg_->setUniform("vecVertices_", cubeVertices_, 8);
    slicingPrg_->setUniform("v1_", v1_, 24);
    slicingPrg_->setUniform("v2_", v2_, 24);

    lightCamera_.get()->setPosition(halfLight_.get().xyz());
    //lightCamera_.get()->setUpVector(cross(-normalize(halfLight_.get().xyz()), normalize(lightCamera_.get()->getStrafe())));
    tgt::vec3 camView = camera_.get()->getLook();
    tgt::vec3 lightView = -1.f * normalize(lightCamera_.get()->getPosition());

    tgt::vec3 halfAngleVec;
    bool invert;

    if (dot(camView, lightView) > 0) {
        halfAngleVec = normalize(camView + lightView);
        invert = false;
    } else {
        halfAngleVec = normalize(-camView + lightView);
        invert = true;
    }

    // compute front index needed for clipping
    int frontIdx = 0; // index of the vertex closest to the camera
    float minCameraDistance = 1000.0;//std::numeric_limits<float>::max(); // distance between camera and closest vertex
    tgt::vec3 camPos = -halfAngleVec * (length(camera_.get()->getPosition()) + length(lightCamera_.get()->getPosition())) * 0.5f;
    for (unsigned int i=0;i<8;i++) {
        float cameraDistance = length(camPos-cubeVertices_[i]);
        if (cameraDistance < minCameraDistance) {
            minCameraDistance = cameraDistance;
            frontIdx = i;
        }
    }

    slicingPrg_->setUniform("frontIdx_", frontIdx);
    slicingPrg_->setUniform("camDistance_", length(camPos - cubeVertices_[frontIdx]));
    slicingPrg_->setUniform("vecView_", halfAngleVec);

    sliceDistance = 0.5f*std::sqrt(1.f + dot(normalize(camView), normalize(lightView)))*sliceDistance;
    //if(sliceDistance < 0.0005f)
        //sliceDistance = 0.0005f;

    slicingPrg_->setUniform("dPlaneIncr_", sliceDistance);

    // set common uniforms used by all shaders
    setGlobalShaderParameters(slicingPrg_, camera_.get());
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(slicingPrg_, volumeTextures, camera_.get(), lightPosition_.get());

    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    tgt::loadMatrix(camera_.get()->getProjectionMatrix());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    slicingPrg_->activate();
    glEnable(GL_BLEND);

    unsigned int numSlices = static_cast<unsigned int>(1.0f / sliceDistance);

    TextureUnit lightBufferUnit;

    for (unsigned int curSlice = 0; curSlice < numSlices; curSlice++) {
        outport_.activateTarget();

        // FIRST PASS
        if(invert)
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        else
            glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);

        tgt::loadMatrix(camera_.get()->getViewMatrix());

        lightBufferUnit.activate();
        glEnable(GL_TEXTURE_2D);
        lightport_.bindColorTexture();

        setGlobalShaderParameters(slicingPrg_, camera_.get());
        slicingPrg_->setUniform("secondPass_", false);
        slicingPrg_->setUniform("lightBuf_", lightBufferUnit.getUnitNumber());
        slicingPrg_->setUniform("lightMat_", lightCamera_.get()->getViewMatrix());
        lightport_.setTextureParameters(slicingPrg_, "lightBufParameters_");

        glBegin(GL_POLYGON);
            for (unsigned int curPoint=0; curPoint<6; curPoint++)
                glVertex2i(curPoint, curSlice);
        glEnd();

        // SECOND PASS
        lightport_.activateTarget();

        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        tgt::loadMatrix(lightCamera_.get()->getViewMatrix());

        setGlobalShaderParameters(slicingPrg_, lightCamera_.get());
        slicingPrg_->setUniform("secondPass_", true);

        glBegin(GL_POLYGON);
            for (unsigned int curPoint=0; curPoint<6; curPoint++)
                glVertex2i(curPoint, curSlice);
        glEnd();
    }

    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    slicingPrg_->deactivate();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    outport_.deactivateTarget();
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

std::string HalfAngleSlicer::generateHeader() {
    std::string headerSource = VolumeRenderer::generateHeader();
    headerSource += transferFunc_.get()->getShaderDefines();
    return headerSource;
}

} // namespace
