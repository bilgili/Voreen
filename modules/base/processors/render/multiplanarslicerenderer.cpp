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

#include "multiplanarslicerenderer.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/numericproperty.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

inline tgt::vec3 permuteComponents(const tgt::vec3& input,
                                   const tgt::ivec3& permutation) {
    return tgt::vec3(input[permutation.x], input[permutation.y], input[permutation.z]);
}

namespace voreen {

const std::string MultiplanarSliceRenderer::loggerCat_("voreen.base.MultiplanarSliceRenderer");

MultiplanarSliceRenderer::MultiplanarSliceRenderer()
    : SliceRendererBase(),
    renderXYSlice_("renderSlice.XY", "Render XY Slice", true),
    renderXZSlice_("renderSlice.XZ", "Render XZ Slice", false),
    renderYZSlice_("renderSlice.YZ", "Render YZ Slice", false),
    sliceNumberXY_("sliceNumber.XY", "XY Slice Number", 0, 0, 10000),
    sliceNumberXZ_("sliceNumber.XZ", "XZ Slice Number", 0, 0, 10000),
    sliceNumberYZ_("sliceNumber.YZ", "YZ Slice Number", 0, 0, 10000),
    camProp_("camera", "Camera", tgt::Camera(tgt::vec3(0.0f, 0.0f, 3.5f), tgt::vec3(0.0f, 0.0f, 0.0f), tgt::vec3(0.0f, 1.0f, 0.0f))),
    cameraHandler_(0)
{
    addProperty(renderXYSlice_);
    addProperty(sliceNumberXY_);
    addProperty(renderXZSlice_);
    addProperty(sliceNumberXZ_);
    addProperty(renderYZSlice_);
    addProperty(sliceNumberYZ_);
    addProperty(camProp_);

    cameraHandler_ = new CameraInteractionHandler("cameraHandler", "Camera Handler", &camProp_);
    addInteractionHandler(cameraHandler_);
}

MultiplanarSliceRenderer::~MultiplanarSliceRenderer() {
    delete cameraHandler_;
}

Processor* MultiplanarSliceRenderer::create() const {
    return new MultiplanarSliceRenderer();
}

void MultiplanarSliceRenderer::process() {

    tgtAssert(inport_.isReady(), "Inport not ready");
    tgtAssert(inport_.getData()->getRepresentation<VolumeRAM>(), "No volume");

    if (inport_.hasChanged()) {
        updateNumSlices();  // validate the currently set values and adjust them if necessary
        transferFunc_.setVolumeHandle(inport_.getData());
    }

    outport_.activateTarget("OrthogonalSliceRenderer::process()");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // get GL resources and setup shader
    TextureUnit transferUnit, texUnit;
    bool setupSuccessful;
    if (texMode_.isSelected("2d-texture")) {      // 2D texture
        setupSuccessful = setupSliceShader(sliceShader_, inport_.getData(), &transferUnit);
    }
    else if (texMode_.isSelected("3d-texture")) { // 3D texture
        // also binds the volume
        setupSuccessful = setupVolumeShader(sliceShader_, inport_.getData(), &texUnit, &transferUnit, 0, lightPosition_.get());
    }
    else {
        LERROR("unknown texture mode: " << texMode_.get());
        setupSuccessful = false;
    }
    if (!setupSuccessful) {
        outport_.deactivateTarget();
        return;
    }

    transferUnit.activate();
    transferFunc_.get()->bind();

    sliceShader_->setUniform("textureMatrix_", tgt::mat4::identity);

    // important: save current camera state before using the processor's camera or
    // successive processors will use those settings!
    //
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    camProp_.look(outport_.getSize());

    // transform bounding box by dataset transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    tgt::multMatrix(inport_.getData()->getPhysicalToWorldMatrix());

    if (renderXYSlice_.get()) {
        renderSlice(SLICE_XY, sliceNumberXY_.get(), texUnit);
    }
    if (renderXZSlice_.get()) {
        renderSlice(SLICE_XZ, sliceNumberXZ_.get(), texUnit);
    }
    if (renderYZSlice_.get()) {
        renderSlice(SLICE_YZ, sliceNumberYZ_.get(), texUnit);
    }

    // restore matrix stack
    glPopMatrix();

    deactivateShader();

    glActiveTexture(GL_TEXTURE0);
    outport_.deactivateTarget();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// protected methods
//

void MultiplanarSliceRenderer::renderSlice(SliceAlignment sliceAlign, int sliceNo, tgt::TextureUnit& texUnit) {

    tgtAssert(inport_.hasData(), "No volume");
    const VolumeBase* volume =  inport_.getData();
    tgt::ivec3 volDim = volume->getDimensions();
    tgt::vec3 textureSize = volume->getCubeSize();

    tgt::vec3 dim = static_cast<tgt::vec3>(volDim - tgt::ivec3(1));
    tgt::vec3 llb = volume->getLLF();

    tgt::ivec3 permutation(0, 1, 2);
    switch (sliceAlign) {
        default:
        case SLICE_XY:
            break;
        case SLICE_XZ:
            permutation = tgt::ivec3(0, 2, 1);
            break;
        case SLICE_YZ:
            permutation = tgt::ivec3(2, 1, 0);
            break;
    }
    float s = tgt::clamp(sliceNo / (dim[permutation.z]-1), 0.f, 1.f);

    tgt::vec3 ll = permuteComponents(tgt::vec3(0.f, 0.f, s), permutation);
    tgt::vec3 lr = permuteComponents(tgt::vec3(1.f, 0.f, s), permutation);
    tgt::vec3 ur = permuteComponents(tgt::vec3(1.f, 1.f, s), permutation);
    tgt::vec3 ul = permuteComponents(tgt::vec3(0.f, 1.f, s), permutation);

    if (texMode_.isSelected("2d-texture")) {
        tgt::vec3 texLowerLeft = ll;//(textureMatrix_*ll).xyz();
        tgt::vec3 texUpperRight = ur;//(textureMatrix_*ur).xyz();

        tgt::Texture* sliceTexture = generateAlignedSliceTexture(inport_.getData(), texLowerLeft, texUpperRight);
        if (sliceTexture) {
            bindSliceTexture(sliceShader_, inport_.getData(), sliceTexture, &texUnit);
            LGL_ERROR;

            sliceShader_->setUniform("textureMatrix_", tgt::mat4::identity);
            glBegin(GL_QUADS);

            glTexCoord2f(0.f, 0.f);
            glVertex3fv( ((ll * textureSize) + llb).elem );

            if (sliceAlign == SLICE_YZ)
                glTexCoord2f(0.f, 1.f);
            else
                glTexCoord2f(1.f, 0.f);
            glVertex3fv( ((lr * textureSize) + llb).elem );

            glTexCoord2f(1.f, 1.f);
            glVertex3fv( ((ur * textureSize) + llb).elem );

            if (sliceAlign == SLICE_YZ)
                glTexCoord2f(1.f, 0.f);
            else
                glTexCoord2f(0.f, 1.f);
            glVertex3fv( ((ul * textureSize) + llb).elem );

            glEnd();
            delete sliceTexture;

            LGL_ERROR;
        }
    }
    else if (texMode_.isSelected("3d-texture")) {

        glBegin(GL_QUADS);
            glTexCoord3fv(ll.elem);
            glVertex3fv( ((ll * textureSize) + llb).elem );

            glTexCoord3fv(lr.elem);
            glVertex3fv( ((lr * textureSize) + llb).elem );

            glTexCoord3fv(ur.elem);
            glVertex3fv( ((ur * textureSize) + llb).elem );

            glTexCoord3fv(ul.elem);
            glVertex3fv( ((ul * textureSize) + llb).elem );
        glEnd();
    }
}

void MultiplanarSliceRenderer::updateNumSlices() {

    tgtAssert(inport_.hasData() && inport_.getData()->getRepresentation<VolumeRAM>(), "No volume");
    tgt::ivec3 volDim = inport_.getData()->getRepresentation<VolumeRAM>()->getDimensions();

    // set number of slice for xy-plane (along z-axis)
    sliceNumberXY_.setMaxValue(volDim.z);
    if (sliceNumberXY_.get() >= volDim.z)
        sliceNumberXY_.set(volDim.z / 2);
    sliceNumberXY_.updateWidgets();

    // set number of slices for zx-plane (along y-axis)
    sliceNumberXZ_.setMaxValue(volDim.y);
    if (sliceNumberXZ_.get() >= volDim.y)
        sliceNumberXZ_.set(volDim.y / 2);
    sliceNumberXZ_.updateWidgets();

    // set number of slices for sagittal plane (along x-axis)
    sliceNumberYZ_.setMaxValue(volDim.x);
    if (sliceNumberYZ_.get() >= volDim.x)
        sliceNumberYZ_.set(volDim.x / 2);
    sliceNumberYZ_.updateWidgets();
}


}   // namespace
