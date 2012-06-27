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

#include "voreen/core/vis/processors/render/slicesequencerenderer.h"

#include <math.h>
#include <sstream>

#include "tgt/gpucapabilities.h"
#include "tgt/glmath.h"
#include "tgt/font.h"
#include "tgt/tgt_gl.h"

#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/application.h"

using tgt::vec2;
using tgt::vec3;

namespace voreen {

const std::string SliceSequenceRenderer::fontName_("Vera.ttf");

SliceSequenceRenderer::SliceSequenceRenderer(const bool isSingleSlice)
    : SliceRendererBase()
    , alignmentProp_(0)
    , sliceIndexProp_("sliceIndex", "slice number: ", 1, 1, 100)
    , numSlicesPerRowProp_("numSlicesPerRowProp", "slices per Row: ", 4, 1, 5)
    , numSlicesPerColProp_("numSlicesPerColProp", "slices per Column: ", 4, 1, 5)
    , renderSliceBoundariesProp_("renderSliceBoundariesProp", "render slice boundaries: ", true)
    , eventProp_("Show cursor position", tgt::Event::NONE, tgt::MouseEvent::MOUSE_ALL)
    , alignment_(SAGITTAL)
    , numSlices_(0)
    , slicePos_(1.0f)
    , sliceSize_(0.0f)
    , volumeDimensions_(0, 0, 0)
    , lastMousePosition_(0, 0)
    , voxelPosPermutation_(0, 1, 2)
{
    setName("SliceSequenceRenderer");

    std::vector<std::string> alignments;
    alignments.push_back("SAGITTAL");
    alignments.push_back("AXIAL");
    alignments.push_back("CORONAL");
    alignmentProp_ = new EnumProp("sliceAlignmentProp", "slice alignment: ", alignments, 0);
    alignmentProp_->onChange(
        CallMemberAction<SliceSequenceRenderer>(this, &SliceSequenceRenderer::onSliceAlignmentChange) );
    addProperty(alignmentProp_);

    addProperty(&sliceIndexProp_);
    if (isSingleSlice == false) {
        addProperty(&numSlicesPerRowProp_);
        addProperty(&numSlicesPerColProp_);
    }
    addProperty(&renderSliceBoundariesProp_);
    addProperty(&eventProp_);

    // call this method to set the correct permutation for the
    // screen-position-to-voxel-position mapping.
    //
    onSliceAlignmentChange();
}

SliceSequenceRenderer::~SliceSequenceRenderer() {
    delete alignmentProp_;
}

const Identifier SliceSequenceRenderer::getClassName() const {
    return "SliceRenderer.SliceSequenceRenderer";
}

const std::string SliceSequenceRenderer::getProcessorInfo() const {
    return "Performs rendering of multiple slices parallel to the front, side or top of a dataset.";
}

void SliceSequenceRenderer::updateNumSlices() {
    numSlices_ = static_cast<std::size_t>(volumeDimensions_[alignment_]);
    if (numSlices_ == 0)
        return;

    int sliceIndex = static_cast<int>(numSlices_ / 2);
    sliceIndexProp_.setMaxValue(numSlices_);
    sliceIndexProp_.set(sliceIndex);

    numSlicesPerColProp_.setMaxValue(numSlices_);
    numSlicesPerRowProp_.setMaxValue(numSlices_);

    if (numSlicesPerRowProp_.get() >= static_cast<int>(numSlices_))
        numSlicesPerRowProp_.set( numSlices_ );

    if (numSlicesPerColProp_.get() >= static_cast<int>(numSlices_))
        numSlicesPerColProp_.set( numSlices_ );
}

void SliceSequenceRenderer::mouseMoveEvent(tgt::MouseEvent* e) {
    if ((e != 0) && (eventProp_.accepts(e))) {
        lastMousePosition_ = e->coord();
        e->ignore();
    }
}

void SliceSequenceRenderer::mousePressEvent(tgt::MouseEvent* e) {
    if ((e != 0) && (eventProp_.accepts(e))) {
        lastMousePosition_ = e->coord();
        e->ignore();

        // call invalidate() in order to re-render the textual infos...
        //
        invalidate();
    }
}

void SliceSequenceRenderer::process(LocalPortMapping* portMapping) {
    bool handleChanged = false;
    if (VolumeHandleValidator::checkVolumeHandle(currentVolumeHandle_,
        portMapping->getVolumeHandle("volumehandle.volumehandle"), &handleChanged) == false)
    {
        return;
    }

    if (handleChanged == true) {
        volumeDimensions_ = currentVolumeHandle_->getVolume()->getDimensions();
        updateNumSlices();  // validate the currently set values and adjust them if necessary
        transferFunc_.setVolumeHandle(currentVolumeHandle_);
    }

    setupShader();
    if (ready() == false)
        return;

    if (tc_ != 0) {
        std::ostringstream oss;
        oss << "SingleSliceRenderer::render(dataset = " << currentVolumeHandle_ << ") dest";
        int dest = portMapping->getTarget("image.outport");
        tc_->setActiveTarget(dest, oss.str());
    } else
        return;

    VolumeGL* volumeGL = currentVolumeHandle_->getVolumeGL();
    const VolumeTexture* const tex = volumeGL->getTexture();
    if (tex == 0) {
        LERROR("setVolumeHandle(): VolumeTexture in VolumGL is NULL!");
        return;
    }

    // get the textures dimensions
    //
    tgt::vec3 urf = tex->getURB();
    tgt::vec3 llb = tex->getLLF();
    urf.z = llb.z;
    llb.z = tex->getURB().z;
    const tgt::vec3 texDim = urf - llb;
    tgt::vec2 texDim2D(0.0f);

    // pointer to the method responsible for rendering the correctly aligned slice
    //
    void (SliceSequenceRenderer::*renderSliceFunc)(float) = 0;

    // set slice's width and height according to currently slice alignment
    // and set the pointer to the slice rendering method to the matching version
    //
    switch (alignment_) {
        case SliceSequenceRenderer::SAGITTAL:
            texDim2D.x = texDim.y;
            texDim2D.y = texDim.z;
            renderSliceFunc = &SliceSequenceRenderer::renderSagittalSlice;
            break;

        case SliceSequenceRenderer::AXIAL:
            texDim2D.x = texDim.x;
            texDim2D.y = texDim.z;
            renderSliceFunc = &SliceSequenceRenderer::renderAxialSlice;
            break;

        case SliceSequenceRenderer::CORONAL:
            texDim2D.x = texDim.y;
            texDim2D.y = texDim.x;
            renderSliceFunc = &SliceSequenceRenderer::renderCoronalSlice;
            break;

        default:
            break;
    }   // switch

    if (renderSliceFunc == 0) {
        LERROR("process(): failed to find a suitable rendering function for slice alignment '"
            << static_cast<int>(alignment_) << "'!");
        return;
    }

    const float canvasWidth = static_cast<float>(size_.x);
    const float canvasHeight = static_cast<float>(size_.y);
    const size_t numSlicesCol = static_cast<size_t>(numSlicesPerColProp_.get());
    const size_t numSlicesRow = static_cast<size_t>(numSlicesPerRowProp_.get());
    float scaleWidth = canvasWidth / (texDim2D.x * numSlicesCol);
    float scaleHeight = canvasHeight / (texDim2D.y * numSlicesRow);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, canvasWidth, 0.0f, canvasHeight, -1.0f, 1.0f);

    glMatrixMode(GL_TEXTURE);
    tgt::loadMatrix( tex->getMatrix() );

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glActiveTexture(tm_.getGLTexUnit(transFuncTexUnit_));
    transferFunc_.get()->bind();
    glActiveTexture(tm_.getGLTexUnit(volTexUnit_));
    tex->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // find minimal scaling factor (either scale along canvas' width or
    // canvas' height)
    //
    if (scaleWidth <= scaleHeight) {
        sliceSize_ = texDim2D * scaleWidth;
        slicePos_.x = 0.0f;
        slicePos_.y = (canvasHeight - (numSlicesRow * sliceSize_.y)) / 2.0f;
    } else {
        sliceSize_ = texDim2D * scaleHeight;
        slicePos_.x = (canvasWidth - (numSlicesCol * sliceSize_.x)) / 2.0f;
        slicePos_.y = 0.0f;
    }

    float depth = 0.0f;
    const size_t sliceIndex = static_cast<size_t>(sliceIndexProp_.get() - 1);
    for (size_t pos = 0, x = 0, y = 0; pos < (numSlicesCol * numSlicesRow);
        ++pos, x = pos % numSlicesCol, y = pos / numSlicesCol)
    {
        size_t sliceNumber = (pos + sliceIndex);
        if (sliceNumber >= numSlices_)
            break;

        // calculate depth in llb/urf space
        depth = ((static_cast<float>(sliceNumber) / static_cast<float>(numSlices_ - 1)) - 0.5f)
            * volumeGL->getVolume()->getCubeSize()[alignment_];

        // check whether the given slice is not within tex
        if ((depth < llb[alignment_]) || (depth > urf[alignment_]))
            continue;

        // map depth to [0, 1]
        depth -= tex->getCenter()[alignment_];  // center around origin
        depth /= texDim[alignment_];            // map to [-0.5, -0.5]
        depth += 0.5f;                          // map to [0, 1]

        glLoadIdentity();
        glTranslatef(slicePos_.x + (x * sliceSize_.x),
            slicePos_.y + ((numSlicesRow - (y + 1)) * sliceSize_.y), 0.0f);
        glScalef(sliceSize_.x, sliceSize_.y, 1.0f);

        if (renderSliceFunc != 0)
            (this->*renderSliceFunc)(depth);
    }   // for (pos

    deactivateShader();

    // render a white border around each slice's boundaries if desired
    //
    if (renderSliceBoundariesProp_.get() == true) {
        glLoadIdentity();
        glTranslatef(slicePos_.x, slicePos_.y, 0.0f);
        glScalef(sliceSize_.x * numSlicesCol, sliceSize_.y * numSlicesRow, 1.0f);
        renderSliceBoundaries(numSlicesRow, numSlicesCol);
    }

    // If freetype is available render the slice's number, otherwise this will do nothing
    //
    renderInfoTexts(numSlicesRow, numSlicesCol);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glActiveTexture(GL_TEXTURE0);
}

// protected methods
//

void SliceSequenceRenderer::renderCoronalSlice(const float depth) {
    glTranslatef(0.5f, 0.5f, 0.0f);
    glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
        glTexCoord3f(0.0f, 0.0f, depth); glVertex2f(-0.5f, -0.5f);
        glTexCoord3f(1.0f, 0.0f, depth); glVertex2f(0.5f, -0.5f);
        glTexCoord3f(1.0f, 1.0f, depth); glVertex2f(0.5f, 0.5f);
        glTexCoord3f(0.0f, 1.0f, depth); glVertex2f(-0.5f, 0.5f);
    glEnd();
}

void SliceSequenceRenderer::renderSagittalSlice(const float depth) {
    glBegin(GL_QUADS);
        glTexCoord3f(depth, 0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord3f(depth, 1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
        glTexCoord3f(depth, 1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord3f(depth, 0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
    glEnd();
}

void SliceSequenceRenderer::renderAxialSlice(const float depth) {
    glTranslatef(0.5f, 0.5f, 0.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
        glTexCoord3f(0.0f, depth, 0.0f); glVertex2f(-0.5f, -0.5f);
        glTexCoord3f(0.0f, depth, 1.0f); glVertex2f(0.5f, -0.5f);
        glTexCoord3f(1.0f, depth, 1.0f); glVertex2f(0.5f, 0.5f);
        glTexCoord3f(1.0f, depth, 0.0f); glVertex2f(-0.5f, 0.5f);
    glEnd();
}

void SliceSequenceRenderer::renderSliceBoundaries(const size_t numSlicesRow, const size_t numSlicesCol) {
    if ((numSlicesRow <= 0) || (numSlicesCol <= 0))
        return;

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glBegin(GL_LINE_LOOP);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(1.0f, 0.0f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(0.0f, 1.0f);
    glEnd();

    const float delta_x = 1.0f / numSlicesCol;
    const float delta_y = 1.0f / numSlicesRow;
    glBegin(GL_LINES);
    for (size_t x = 1; x < numSlicesCol; ++x) {
        glVertex2f(delta_x * x, 0.0f);
        glVertex2f(delta_x * x, 1.0f);
    }

    for (size_t y = 1; y < numSlicesRow; ++y) {
        glVertex3f(0.0f, delta_y * y, 0.0f);
        glVertex3f(1.0f, delta_y * y, 0.0f);
    }
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
}

#ifdef VRN_WITH_FONTRENDERING
void SliceSequenceRenderer::renderInfoTexts(const size_t numSlicesRow, const size_t numSlicesCol) {
    tgt::Font font(VoreenApplication::app()->getFontPath(fontName_));
    tgt::Font fontPos(VoreenApplication::app()->getFontPath(fontName_));

    glDisable(GL_DEPTH_TEST);

    // Initialize the font with its size according to the slices' width
    //
    const int n = ((static_cast<int>(size_.x / sliceSize_.x) / 2) - 0);
    font.setSize(20 - (2*n));

    fontPos.setSize(20);

    // ESSENTIAL: if you don't use this, your text will become texturized!
    glActiveTexture(GL_TEXTURE0);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // render voxel position information
    //
    tgt::ivec3 voxelPos = screenToVoxelPos(lastMousePosition_);
    glLoadIdentity();
    std::ostringstream oss;
    oss << "[" << voxelPos.x << "," << voxelPos.y << "," << voxelPos.z << "]";
    fontPos.render(tgt::vec3(10, 10, 0), oss.str());

    // render the slice number information
    //
    size_t sliceNumber = static_cast<size_t>(sliceIndexProp_.get() - 1);
    for (size_t pos = 0, x = 0, y = 0; pos < static_cast<size_t>(numSlicesCol * numSlicesRow);
        ++pos, x = pos % numSlicesCol, y = pos / numSlicesCol)
    {
        glLoadIdentity();
        if ((pos + sliceNumber) >= numSlices_)
            break;
        glTranslatef(slicePos_.x + (x * sliceSize_.x),
            slicePos_.y + ((numSlicesRow - (y + 1)) * sliceSize_.y), 0.0f);
        std::ostringstream oss;
        oss << (sliceNumber + pos + 1) << "/" << numSlices_;
        font.render(tgt::vec3(10,10,0), oss.str());
    }

    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
}
#else
void SliceSequenceRenderer::renderInfoTexts(const size_t /*numSlicesRow*/, const size_t /*numSlicesCol*/) {
}
#endif  // VRN_WITH_FONTRENDERING

tgt::ivec3 SliceSequenceRenderer::screenToVoxelPos(const tgt::ivec2& screenPos) {
    tgt::ivec3 p(0, 0, 0);
    p.x = screenPos.x - static_cast<int>(tgt::round(slicePos_.x));
    p.y = screenPos.y - static_cast<int>(tgt::round(slicePos_.y));

    // if coordinates are negative, no slice could be hit
    //
    if ((p.x < 0) || (p.y < 0))
        return tgt::ivec3(-1, -1, -1);

    const int numSlicesRow = numSlicesPerRowProp_.get();
    const int numSlicesCol = numSlicesPerColProp_.get();
    const tgt::ivec2 sliceSizeInt = static_cast<tgt::ivec2>(sliceSize_);

    // if coordinates are greater than the number of slice per direction
    // times their extension in that direction, no slice could be hit either
    //
    if ((p.x >= (sliceSizeInt.x * numSlicesCol)) || (p.y >= (sliceSizeInt.y * numSlicesRow)))
        return tgt::ivec3(-1, -1, -1);

    // dertemine the number of the current slice in the 2x2 "array" of slices currently
    // visible
    //
    const int sliceX = p.x / sliceSizeInt.x;
    const int sliceY = p.y / sliceSizeInt.y;

    // calculate the position within a single slice in PIXEL
    //
    const tgt::vec2 posWithinSlice(static_cast<float>(p.x % sliceSizeInt.x),
        static_cast<float>(p.y % sliceSizeInt.y));

    // Now the position within the original volume can be determined by dividing
    // the position within the slice in pixel by the size of the slice in pixel.
    // 1.0 - ... is taken in order to correct from "top-down" to "bottom-up" axis.
    // voxelPosPermutation_ contains the permutation of the coordinates in p to which
    // the value read from the mouse position applies to in the volumetric texture.
    // volxelPosPerumation_ is set whenever the alignment of the slices are changed
    // (most frequently this will be done by the user interacting with the processor)
    //
    p[voxelPosPermutation_.x] = static_cast<int>(volumeDimensions_[voxelPosPermutation_.x]
        * (1.0f - (posWithinSlice.x / sliceSize_.x)));
    p[voxelPosPermutation_.y] = static_cast<int>(volumeDimensions_[voxelPosPermutation_.y]
        * (1.0f - (posWithinSlice.y / sliceSize_.y)));
    p[voxelPosPermutation_.z] = sliceX + (sliceY * numSlicesCol) + sliceIndexProp_.get();

    return p;
}

void SliceSequenceRenderer::onSliceAlignmentChange() {
    if (alignmentProp_ == 0)
        return;

    // Use fool-proof method instead of directly assigning
    // alignmentProp_->get() to alignment_. Someone could
    // a string to the property...
    //
    switch (alignmentProp_->get()) {
        case 1:
            alignment_ = SliceSequenceRenderer::AXIAL;
            voxelPosPermutation_ = tgt::ivec3(0, 2, 1);
            break;
        case 2:
            alignment_ = SliceSequenceRenderer::CORONAL;
            voxelPosPermutation_ = tgt::ivec3(0, 1, 2);
            break;
        case 0:
        default:
            alignment_ = SliceSequenceRenderer::SAGITTAL;
            voxelPosPermutation_ = tgt::ivec3(1, 2, 0);
            break;
    }
    updateNumSlices();
}

} // namespace voreen
