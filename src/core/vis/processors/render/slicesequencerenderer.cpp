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
    , boundaryColor_("boundaryColor", "Boundary Color", tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f))
    , alignment_(XY_PLANE)
    , numSlices_(0)
    , slicePos_(1.0f)
    , sliceSize_(0.0f)
    , volumeDimensions_(0, 0, 0)
    , lastMousePosition_(0, 0)
    , voxelPosPermutation_(0, 1, 2)
{
    //FIXME: memory leak (EventAction?)
    eventPressProp_ = new TemplateMouseEventProperty<SliceSequenceRenderer>("Show cursor position", new EventAction<SliceSequenceRenderer, tgt::MouseEvent>(this, &SliceSequenceRenderer::mouseLocalization), tgt::MouseEvent::PRESSED, tgt::Event::NONE, tgt::MouseEvent::MOUSE_ALL);
    //FIXME: memory leak (EventAction?)
    eventMoveProp_ = new TemplateMouseEventProperty<SliceSequenceRenderer>("Show cursor position", new EventAction<SliceSequenceRenderer, tgt::MouseEvent>(this, &SliceSequenceRenderer::mouseLocalization), tgt::MouseEvent::MOTION, tgt::Event::NONE, tgt::MouseEvent::MOUSE_ALL);
    eventPressProp_->setOwner(this);
    eventMoveProp_->setOwner(this);

    alignmentProp_ = new OptionProperty<SliceAlignment>("sliceAlignmentProp", "slice alignment:");
    alignmentProp_->addOption("xy-plane", "xy-plane", XY_PLANE);
    alignmentProp_->addOption("xz-plane", "xz-plane", XZ_PLANE);
    alignmentProp_->addOption("zy-plane", "zy-plane", ZY_PLANE);
        alignmentProp_->onChange(
        CallMemberAction<SliceSequenceRenderer>(this, &SliceSequenceRenderer::onSliceAlignmentChange) );
    addProperty(alignmentProp_);

    addProperty(sliceIndexProp_);
    if (isSingleSlice == false) {
        addProperty(numSlicesPerRowProp_);
        addProperty(numSlicesPerColProp_);
    }
    addProperty(renderSliceBoundariesProp_);
    addProperty(boundaryColor_);

    addEventProperty(eventPressProp_);
    addEventProperty(eventMoveProp_);

    // call this method to set the correct permutation for the
    // screen-position-to-voxel-position mapping.
    //
    onSliceAlignmentChange();
}

SliceSequenceRenderer::~SliceSequenceRenderer() {
    delete eventPressProp_;
    delete eventMoveProp_;
    delete alignmentProp_;
}

const std::string SliceSequenceRenderer::getProcessorInfo() const {
    return "Performs rendering of multiple slices parallel to the front, side or top of a dataset.";
}

void SliceSequenceRenderer::updateNumSlices() {
    numSlices_ = static_cast<std::size_t>(volumeDimensions_[alignment_]);
    if (numSlices_ == 0)
        return;

    sliceIndexProp_.setMaxValue(numSlices_);
    if (sliceIndexProp_.get() > static_cast<int>(numSlices_))
        sliceIndexProp_.set(static_cast<int>(numSlices_ / 2));

    numSlicesPerColProp_.setMaxValue(numSlices_);
    numSlicesPerRowProp_.setMaxValue(numSlices_);

    if (numSlicesPerRowProp_.get() >= static_cast<int>(numSlices_))
        numSlicesPerRowProp_.set( numSlices_ );

    if (numSlicesPerColProp_.get() >= static_cast<int>(numSlices_))
        numSlicesPerColProp_.set( numSlices_ );
}

void SliceSequenceRenderer::mouseLocalization(tgt::MouseEvent* e) {
    if (e != 0) {
        lastMousePosition_ = e->coord();
        e->ignore();
    }
}

void SliceSequenceRenderer::process() {
    if (inport_.hasChanged()) {
        volumeDimensions_ = inport_.getData()->getVolume()->getDimensions();
        updateNumSlices();  // validate the currently set values and adjust them if necessary
        transferFunc_.setVolumeHandle(inport_.getData());
    }

    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    setupShader();
    if (ready() == false)
        return;

    VolumeGL* volumeGL = inport_.getData()->getVolumeGL();
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

    // Re-calculate texutre dimensions, urf, llb and center of the texture
    // for it might be a NPOT texture and therefore migh have been inflated.
    // In that case, the inflating need to be undone and the volume texture
    // needs to be "cropped" to its original measures.
    //
    const tgt::vec3 texDim = tex->getMatrix() * (urf - llb);
    urf = texDim / 2.0f;
    llb = texDim / -2.0f;
    tgt::vec3 texCenter = (llb + (texDim * 0.5f));

    // Use OpenGL's ability of multiplying texture coodinate vectors with a matrix
    // on the texture matrix stack to permute the components of the texture
    // coordinates to obtain a correct setting for the current slice alignment.
    //
    tgt::mat4 textureMatrix = tgt::mat4::zero;
    tgt::vec2 texDim2D(0.0f);   // laziness... (no matrix multiplication to determine 2D size of texture)

    // set slice's width and height according to currently slice alignment
    // and set the pointer to the slice rendering method to the matching version
    //
    switch (alignment_) {
        case SliceSequenceRenderer::XZ_PLANE:
            texDim2D.x = texDim.x;
            texDim2D.y = texDim.z;
            textureMatrix.t00 = 1.0f;   // setup a permutation matrix, swaping z- and y-
            textureMatrix.t12 = 1.0f;   // components on vectors being multiplied with it
            textureMatrix.t21 = 1.0f;
            textureMatrix.t33 = 1.0f;
            break;

        case SliceSequenceRenderer::ZY_PLANE:
            texDim2D.x = texDim.z;
            texDim2D.y = texDim.y;
            textureMatrix.t02 = 1.0f;   // setup a permutation matrix, swaping x- and z-
            textureMatrix.t11 = 1.0f;   // components on vectors being multiplied with it
            textureMatrix.t20 = 1.0f;
            textureMatrix.t33 = 1.0f;
            break;

        case SliceSequenceRenderer::XY_PLANE:
            texDim2D.x = texDim.x;
            texDim2D.y = texDim.y;
            // no break here
        default:
            textureMatrix = tgt::mat4::identity;    // use identity as default
            break;
    }   // switch

    const float canvasWidth = static_cast<float>(outport_.getSize().x);
    const float canvasHeight = static_cast<float>(outport_.getSize().y);
    const size_t numSlicesCol = static_cast<size_t>(numSlicesPerColProp_.get());
    const size_t numSlicesRow = static_cast<size_t>(numSlicesPerRowProp_.get());
    float scaleWidth = canvasWidth / (texDim2D.x * numSlicesCol);
    float scaleHeight = canvasHeight / (texDim2D.y * numSlicesRow);

    glActiveTexture(tm_.getGLTexUnit(transFuncTexUnit_));
    transferFunc_.get()->bind();
    glActiveTexture(tm_.getGLTexUnit(volTexUnit_));
    tex->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    LGL_ERROR;

    // find minimal scaling factor (either scale along canvas' width or
    // canvas' height)
    //
    if (scaleWidth <= scaleHeight) {
        sliceSize_ = texDim2D * scaleWidth;
        slicePos_.x = 0.0f;
        slicePos_.y = (canvasHeight - (numSlicesRow * sliceSize_.y)) / 2.0f;
    }
    else {
        sliceSize_ = texDim2D * scaleHeight;
        slicePos_.x = (canvasWidth - (numSlicesCol * sliceSize_.x)) / 2.0f;
        slicePos_.y = 0.0f;
    }

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    tgt::loadMatrix(textureMatrix);
    tgt::multMatrix(tex->getMatrix());

    LGL_ERROR;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, canvasWidth, 0.0f, canvasHeight, -1.0f, 1.0f);

    LGL_ERROR;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    LGL_ERROR;

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
            * texDim[alignment_];

        // check whether the given slice is not within tex
        if ((depth < llb[alignment_]) || (depth > urf[alignment_]))
            continue;

        // map depth to [0, 1]
        depth -= texCenter[alignment_];  // center around origin
        depth /= texDim[alignment_];            // map to [-0.5, -0.5]
        depth += 0.5f;                          // map to [0, 1]

        glLoadIdentity();
        glTranslatef(slicePos_.x + (x * sliceSize_.x),
            slicePos_.y + ((numSlicesRow - (y + 1)) * sliceSize_.y), 0.0f);
        glScalef(sliceSize_.x, sliceSize_.y, 1.0f);

        glBegin(GL_QUADS);
            glTexCoord3f(0.0f, 0.0f, depth); glVertex2f(0.0f, 0.0f);
            glTexCoord3f(1.0f, 0.0f, depth); glVertex2f(1.0f, 0.0f);
            glTexCoord3f(1.0f, 1.0f, depth); glVertex2f(1.0f, 1.0f);
            glTexCoord3f(0.0f, 1.0f, depth); glVertex2f(0.0f, 1.0f);
        glEnd();

        LGL_ERROR;
    }   // for (pos

    // Has to be done here for some reason. I suspect pending side-effects in subsequent
    // calls to other methods / functions. (dirk)
    //
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    deactivateShader();

    LGL_ERROR;

    // render a white border around each slice's boundaries if desired
    //
    if (renderSliceBoundariesProp_.get() == true) {
        glLoadIdentity();
        glTranslatef(slicePos_.x, slicePos_.y, 0.0f);
        glScalef(sliceSize_.x * numSlicesCol, sliceSize_.y * numSlicesRow, 1.0f);
        glDepthFunc(GL_ALWAYS);
        renderSliceBoundaries(numSlicesRow, numSlicesCol);
        glDepthFunc(GL_LESS);
    }

    // If freetype is available render the slice's number, otherwise this will do nothing.
    //
    renderInfoTexts(numSlicesRow, numSlicesCol);
    LGL_ERROR;

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    LGL_ERROR;

    glActiveTexture(GL_TEXTURE0);

    outport_.deactivateTarget();

    LGL_ERROR;
}

// protected methods
//

void SliceSequenceRenderer::renderSliceBoundaries(const size_t numSlicesRow, const size_t numSlicesCol) {
    if ((numSlicesRow <= 0) || (numSlicesCol <= 0))
        return;

    glColor4f(boundaryColor_.get().r, boundaryColor_.get().g, boundaryColor_.get().b, boundaryColor_.get().a);
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
void SliceSequenceRenderer::renderInfoTexts(const size_t numSlicesRow, const size_t numSlicesCol)
{
    glDisable(GL_DEPTH_TEST);

    // ESSENTIAL: if you don't use this, your text will become texturized!
    glActiveTexture(GL_TEXTURE0);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    LGL_ERROR;

	// render voxel position information
    tgt::Font fontPos(VoreenApplication::app()->getFontPath(fontName_));
    fontPos.setSize(20);
	tgt::ivec3 voxelPos = screenToVoxelPos(lastMousePosition_);
	glLoadIdentity();
	std::ostringstream oss;
	oss << "[" << voxelPos.x << "," << voxelPos.y << "," << voxelPos.z << "]";
	fontPos.render(tgt::vec3(10, 10, 0), oss.str());
	LGL_ERROR;

    // Initialize the font with its size according to the slices' width, but
    // the font size may not be smaller than 8.
	//
    tgt::Font font(VoreenApplication::app()->getFontPath(fontName_));
	const int n = static_cast<int>(outport_.getSize().x / sliceSize_.x);
	font.setSize(std::max(8, (20 - n)));

    // Therefore calculate the rendered text's bounding box by using a dummy
    // string.
    //
    tgt::Bounds bounds = font.getBounds(tgt::vec3(10.0f, 10.0f, 0.0f), "000/000");
    float textWidth = ceilf(bounds.getURB().x - bounds.getLLF().x);

    // Do not render the slice numbers if the slice width becomes too small to
    // prevent FTGL from creating OpenGL state errors.
    //
    if (floorf(sliceSize_.x) > textWidth) {
	    // render the slice number information
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
	    LGL_ERROR;
    }

    glLoadIdentity();
	glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	LGL_ERROR;
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
    alignment_ = alignmentProp_->getValue();
    switch (alignment_) {
        case ZY_PLANE:
            voxelPosPermutation_ = tgt::ivec3(2, 1, 0);
            break;
        case XY_PLANE:
            voxelPosPermutation_ = tgt::ivec3(0, 1, 2);
            break;
        case XZ_PLANE:
        default:
            voxelPosPermutation_ = tgt::ivec3(0, 2, 1);
            break;
    }
    updateNumSlices();
}

} // namespace voreen
