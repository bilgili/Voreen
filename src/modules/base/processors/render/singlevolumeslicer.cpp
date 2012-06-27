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

#include "voreen/modules/base/processors/render/singlevolumeslicer.h"
#include "voreen/core/datastructures/transfunc/transfuncintensity.h"
#include "tgt/glmath.h"
#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

SingleVolumeSlicer::SingleVolumeSlicer()
    : VolumeSlicer()
    , slicingPrg_(0)
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle", false, Processor::INVALID_PROGRAM)
    , outport_(Port::OUTPORT, "image.output", true, Processor::INVALID_RESULT, GL_RGBA16F_ARB)
    , cameraHandler_("cameraHandler", "Camera Handler", &camera_)
{

    addInteractionHandler(cameraHandler_);

    addPort(volumeInport_);
    addPort(outport_);
}

SingleVolumeSlicer::~SingleVolumeSlicer() {
}

Processor* SingleVolumeSlicer::create() const {
    return new SingleVolumeSlicer();
}

std::string SingleVolumeSlicer::getProcessorInfo() const {
    return "This class implements a slicing based volume renderer. The slices are view plane aligned and clipped \
            against the volume on the GPU as described in 'Real-Time Volume Graphics' (1st Ed., Chapter 3.5.1). \
            At the moment no correct depth values are calculated. Furthermore, ping-pong buffering is used instead \
            of OpenGL blending, which decreases the performance. Currently only 1D transfer functions are supported.";
}

void SingleVolumeSlicer::initialize() throw (VoreenException) {
    VolumeSlicer::initialize();

    loadShader();

    if (!slicingPrg_) {
        LERROR("Failed to load shader!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shader!");
    }
    initialized_ = true;
}

void SingleVolumeSlicer::deinitialize() throw (VoreenException) {
    ShdrMgr.dispose(slicingPrg_);
    slicingPrg_ = 0;

    VolumeSlicer::deinitialize();
}

void SingleVolumeSlicer::loadShader() {
    slicingPrg_ = ShdrMgr.loadSeparate("sl_singlevolume.vert", "sl_singlevolume.frag",
        generateHeader(), false, false);
}

void SingleVolumeSlicer::compile() {
    slicingPrg_->setHeaders(generateHeader(), false);
    slicingPrg_->rebuild();
}

bool SingleVolumeSlicer::isReady() const {
    //check if all inports are connected:
    if(!volumeInport_.isReady())
        return false;

    //check if outport is connected:
    if (!outport_.isReady())
        return false;

    return true;
}

void SingleVolumeSlicer::process() {

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
    LGL_ERROR;

    // compute front index needed for clipping
    int frontIdx = 0; // index of the vertex closest to the camera
    float minCameraDistance = 1000.0;//std::numeric_limits<float>::max(); // distance between camera and closest vertex
    tgt::vec3 camPos = camera_.get()->getPosition();
    for (unsigned int i=0;i<8;i++) {
        float cameraDistance = length(camPos-cubeVertices_[i]);
        if (cameraDistance < minCameraDistance) {
            minCameraDistance = cameraDistance;
            frontIdx = i;
        }
    }

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
        "volumeParameters_")
    );

    // compute the distance between two adjacent slices in texture coordinates
    float sliceDistance = 0.1f;
    // acquire volume size for sampling
    if (volumeTextures.size() > 0) {
        tgt::ivec3 dim = volumeTextures[0].volume_->getTexture()->getDimensions();
        sliceDistance = 1.0f / (tgt::min(dim) * samplingRate_.get());
        //if (interactionMode())
        //    sliceDistance /= interactionQuality_.get();
    }

    // initialize slicing shader
    slicingPrg_->activate();
    // fragment shader uniforms
    slicingPrg_->setUniform("transferFunc_", transferUnit.getUnitNumber());
    //clipping uniforms
    slicingPrg_->setUniform("frontIdx_", frontIdx);
    slicingPrg_->setUniform("vecView_", camera_.get()->getLook()-camera_.get()->getPosition());
    //slicingPrg_->setUniform("vecView_", camera_.get()->getFocus()-camera_.get()->getPosition());
    slicingPrg_->setUniform("camDistance_", length(camera_.get()->getPosition()-cubeVertices_[frontIdx]));
    slicingPrg_->setUniform("dPlaneIncr_", sliceDistance);
    slicingPrg_->setUniform("nSequence_", nSeq_, 64);
    slicingPrg_->setUniform("vecVertices_", cubeVertices_, 8);
    slicingPrg_->setUniform("v1_", v1_, 24);
    slicingPrg_->setUniform("v2_", v2_, 24);

    // set common uniforms used by all shaders
    setGlobalShaderParameters(slicingPrg_, camera_.get());
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(slicingPrg_, volumeTextures);

    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    tgt::loadMatrix(camera_.get()->getProjectionMatrix());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    tgt::loadMatrix(camera_.get()->getViewMatrix());

    unsigned int numSlices = static_cast<unsigned int>(1.0f / sliceDistance);

    slicingPrg_->activate();
    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);

    for (unsigned int curSlice=0; curSlice<numSlices; curSlice++) {
        glBegin(GL_POLYGON);
            for (unsigned int curPoint=0; curPoint<6; curPoint++)
                glVertex2i(curPoint, curSlice);
        glEnd();
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    slicingPrg_->deactivate();
    TextureUnit::setZeroUnit();

    LGL_ERROR;
}

std::string SingleVolumeSlicer::generateHeader() {
    std::string headerSource = VolumeRenderer::generateHeader();
    headerSource += transferFunc_.get()->getShaderDefines();
    return headerSource;
}



} // namespace
