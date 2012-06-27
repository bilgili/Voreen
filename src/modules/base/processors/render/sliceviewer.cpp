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

#include "voreen/modules/base/processors/render/sliceviewer.h"

#include <math.h>
#include <sstream>

#include "tgt/gpucapabilities.h"
#include "tgt/glmath.h"
#include "tgt/font.h"
#include "tgt/tgt_gl.h"
#include "tgt/textureunit.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/voreenapplication.h"

using tgt::TextureUnit;

namespace voreen {

const std::string SliceViewer::fontName_("Vera.ttf");

SliceViewer::SliceViewer()
    : SliceRendererBase()
    , sliceAlignment_("sliceAlignmentProp", "Slice Alignment")
    , sliceIndex_("sliceIndex", "Slice Number ", 0, 0, 10000)
    , numSlicesPerRow_("numSlicesPerRow", "Slices per Row", 1, 1, 5)
    , numSlicesPerCol_("numSlicesPerCol", "Slices per Column ", 1, 1, 5)
    , renderSliceBoundaries_("renderSliceBoundaries", "Render Slice Boundaries", true)
    , boundaryColor_("boundaryColor", "Boundary Color", tgt::Color::white)
    , showCursorInfos_("showCursorInformation", "Show Cursor Information")
    , showSliceNumber_("showSliceNumber", "Show Slice Number", true)
    , fontSize_("fontSize", "FontSize", 14, 8, 48)
    , voxelOffset_("voxelOffset", "Voxel Offset", tgt::vec2(0.f), tgt::vec2(-10000.f), tgt::vec2(10000.f))
    , zoomFactor_("zoomFactor", "Zoom Factor", 1.f, 0.01f, 1.f)
    , pickingMatrix_("pickingMatrix", "Picking Matrix", tgt::mat4::createIdentity(), tgt::mat4(-1e6f), tgt::mat4(1e6f), Processor::VALID)
    , mwheelCycleHandler_("mouseWheelHandler", "Slice Cycling", &sliceIndex_)
    , mwheelZoomHandler_("zoomHandler", "Slice Zoom", &zoomFactor_, tgt::MouseEvent::CTRL)
    , voxelPosPermutation_(0, 1, 2)
    , sliceLowerLeft_(1.f)
    , sliceSize_(0.f)
    , mousePosition_(-1, -1)
    , lastPickingPosition_(-1, -1, -1)
{

    mouseEventShift_ = new EventProperty<SliceViewer>("mouseEvent.Shift", "Slice Shift",
        this, &SliceViewer::shiftEvent,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT,
        tgt::MouseEvent::PRESSED | tgt::MouseEvent::MOTION, tgt::Event::CTRL);

    mouseEventMove_ = new EventProperty<SliceViewer>("mouseEvent.cursorPositionMove", "Cursor Position Move",
        this, &SliceViewer::mouseLocalization,
        tgt::MouseEvent::MOUSE_BUTTON_NONE,
        tgt::MouseEvent::MOTION, tgt::Event::MODIFIER_NONE,
        true);

    mouseEventPress_ = new EventProperty<SliceViewer>("mouseEvent.cursorPositionPress", "Cursor Position Press",
        this, &SliceViewer::mouseLocalization,
        static_cast<tgt::MouseEvent::MouseButtons>(tgt::MouseEvent::MOUSE_BUTTON_LEFT | tgt::MouseEvent::MOUSE_BUTTON_MIDDLE),
        tgt::MouseEvent::PRESSED | tgt::MouseEvent::WHEEL | tgt::MouseEvent::MOTION, tgt::Event::MODIFIER_NONE,
        true);

    addEventProperty(mouseEventPress_);
    addEventProperty(mouseEventMove_);
    addEventProperty(mouseEventShift_);

    addInteractionHandler(mwheelCycleHandler_);
    addInteractionHandler(mwheelZoomHandler_);

    sliceAlignment_.addOption("xy-plane", "XY-Plane (axial)", XY_PLANE);
    sliceAlignment_.addOption("xz-plane", "XZ-Plane (coronal)", XZ_PLANE);
    sliceAlignment_.addOption("yz-plane", "YZ-Plane (sagittal)", YZ_PLANE);
    sliceAlignment_.onChange(
        CallMemberAction<SliceViewer>(this, &SliceViewer::onSliceAlignmentChange) );
    addProperty(sliceAlignment_);

    addProperty(sliceIndex_);
    addProperty(numSlicesPerRow_);
    addProperty(numSlicesPerCol_);

    addProperty(renderSliceBoundaries_);
    addProperty(boundaryColor_);

    showCursorInfos_.addOption("never", "Never");
    showCursorInfos_.addOption("onClick", "On Mouse Click");
    showCursorInfos_.addOption("onMove", "On Mouse Move");
    showCursorInfos_.select("onMove");
    addProperty(showCursorInfos_);
    addProperty(showSliceNumber_);
    addProperty(fontSize_);

    addProperty(voxelOffset_);
    zoomFactor_.setStepping(0.01f);
    addProperty(zoomFactor_);
    pickingMatrix_.setWidgetsEnabled(false);
    addProperty(pickingMatrix_);

    // call this method to set the correct permutation for the
    // screen-position-to-voxel-position mapping.
    onSliceAlignmentChange();
}

SliceViewer::~SliceViewer() {
    delete mouseEventPress_;
    delete mouseEventMove_;
    delete mouseEventShift_;
}

Processor* SliceViewer::create() const {
    return new SliceViewer();
}

std::string SliceViewer::getProcessorInfo() const {
    return "Performs slice rendering of a single or multiple slices "
           "along one of the three main axis of the volume. ";
}

void SliceViewer::updateSliceProperties() {

    tgt::ivec3 volumeDim(0);
    if (inport_.getData() && inport_.getData()->getVolume())
        volumeDim = inport_.getData()->getVolume()->getDimensions();

    tgtAssert(sliceAlignment_.getValue() >= 0 && sliceAlignment_.getValue() <= 2, "Invalid alignment value");
    int numSlices = volumeDim[sliceAlignment_.getValue()];
    if (numSlices == 0)
        return;

    sliceIndex_.setMaxValue(numSlices-1);
    if (sliceIndex_.get() >= static_cast<int>(numSlices))
        sliceIndex_.set(static_cast<int>(numSlices / 2));

    numSlicesPerCol_.setMaxValue(numSlices);
    numSlicesPerRow_.setMaxValue(numSlices);

    if (numSlicesPerRow_.get() >= static_cast<int>(numSlices))
        numSlicesPerRow_.set(numSlices);

    if (numSlicesPerCol_.get() >= static_cast<int>(numSlices))
        numSlicesPerCol_.set(numSlices);

    tgt::vec2 halfDim = tgt::vec2((float)volumeDim[voxelPosPermutation_.x], (float)volumeDim[voxelPosPermutation_.y] - 1.f) / 2.f;
    voxelOffset_.setMinValue(-halfDim);
    voxelOffset_.setMaxValue(halfDim);
    voxelOffset_.set(tgt::clamp(voxelOffset_.get(), voxelOffset_.getMinValue(), voxelOffset_.getMaxValue()));
}

void SliceViewer::process() {
    if (inport_.hasChanged()) {
        updateSliceProperties();  // validate the currently set values and adjust them if necessary
        mousePosition_ = tgt::ivec2(-1);
        lastPickingPosition_ = tgt::ivec3(-1);
        transferFunc_.setVolumeHandle(inport_.getData());
    }

    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    if (!inport_.getData() || !inport_.getData()->getVolume() || !inport_.getData()->getVolumeGL() ) {
        outport_.deactivateTarget();
        LGL_ERROR;
        return;
    }

    Volume* volume = inport_.getData()->getVolume();
    VolumeGL* volumeGL = inport_.getData()->getVolumeGL();
    const VolumeTexture* tex = volumeGL->getTexture();
    if (!tex) {
        LERROR("No volume texture");
        outport_.deactivateTarget();
        LGL_ERROR;
        return;
    }

    TextureUnit volUnit, transferUnit;
    setupShader(volumeGL, &volUnit, &transferUnit); // also binds the volume
    if (!ready())
        return;

    // get voxel volume dimensions
    tgt::ivec3 volDim = volume->getDimensions();
    int numSlices = volDim[sliceAlignment_.getValue()];

    // get the textures dimensions
    tgt::vec3 urf = tex->getURB();
    tgt::vec3 llb = tex->getLLF();
    //urf.z = llb.z;
    //llb.z = tex->getURB().z;

    // Re-calculate texture dimensions, urf, llb and center of the texture
    // for it might be a NPOT texture and therefore might have been inflated.
    // In that case, the inflating need to be undone and the volume texture
    // needs to be "cropped" to its original measures.
    //
    const tgt::vec3 texDim = tex->getMatrix() * (urf - llb);
    urf = texDim / 2.f;
    llb = texDim / -2.f;
    tgt::vec3 texCenter = (llb + (texDim * 0.5f));

    // Use OpenGL's ability of multiplying texture coordinate vectors with a matrix
    // on the texture matrix stack to permute the components of the texture
    // coordinates to obtain a correct setting for the current slice alignment.
    //
    textureMatrix_ = tgt::mat4::zero;
    tgt::vec2 texDim2D(0.0f);   // laziness... (no matrix multiplication to determine 2D size of texture)

    // set slice's width and height according to currently slice alignment
    // and set the pointer to the slice rendering method to the matching version
    //
    switch (sliceAlignment_.getValue()) {
        case XZ_PLANE:
            texDim2D.x = texDim.x;
            texDim2D.y = texDim.z;
            textureMatrix_.t00 = 1.f;   // setup a permutation matrix, swapping z- and y-
            textureMatrix_.t12 = 1.f;   // components on vectors being multiplied with it
            textureMatrix_.t21 = 1.f;
            textureMatrix_.t33 = 1.f;
            break;

        case YZ_PLANE:
            texDim2D.x = texDim.y;
            texDim2D.y = texDim.z;
            textureMatrix_.t02 = 1.f;   // setup a permutation matrix, swapping x-, y- and z-
            textureMatrix_.t10 = -1.f;  // components on vectors being multiplied with it
            textureMatrix_.t21 = 1.f;
            textureMatrix_.t13 = 1.f;
            textureMatrix_.t33 = 1.f;
            break;

        case XY_PLANE:
            texDim2D.x = texDim.x;
            texDim2D.y = texDim.y;
            textureMatrix_ = tgt::mat4::identity;
            textureMatrix_.t11 = -1.f;  // invert y-axis, since
            textureMatrix_.t13 = 1.f;   // we want to look along positive z-axis
            textureMatrix_.t33 = 1.f;
        default:
            break;
    }   // switch

    float canvasWidth = static_cast<float>(outport_.getSize().x);
    float canvasHeight = static_cast<float>(outport_.getSize().y);
    int numSlicesCol = numSlicesPerCol_.get();
    int numSlicesRow = numSlicesPerRow_.get();
    float scaleWidth = canvasWidth / (texDim2D.x * numSlicesCol);
    float scaleHeight = canvasHeight / (texDim2D.y * numSlicesRow);

    // find minimal scaling factor (either scale along canvas' width or
    // canvas' height)
    if (scaleWidth <= scaleHeight) {

        sliceSize_ = texDim2D * scaleWidth * (1.f/zoomFactor_.get());

        sliceLowerLeft_.x = 0;
        sliceLowerLeft_.y = (canvasHeight - (numSlicesRow * sliceSize_.y)) / 2.f;

        // adapt for zooming
        tgt::vec2 zoomOffset = -((sliceSize_ - (texDim2D * scaleWidth)) / 2.f) * (float)numSlicesCol;
        tgt::vec2 volDimFloat = tgt::vec2((float)volDim[voxelPosPermutation_.x], (float)volDim[voxelPosPermutation_.y]) - 1.f;
        tgt::vec2 focusOffset = voxelOffset_.get() / volDimFloat;
        focusOffset *= sliceSize_;
        sliceLowerLeft_.x += zoomOffset.x + focusOffset.x;
        sliceLowerLeft_.y += focusOffset.y;
    }
    else {
        sliceSize_ = texDim2D * scaleHeight * (1.f/zoomFactor_.get());

        sliceLowerLeft_.x = (canvasWidth - (numSlicesCol * sliceSize_.x)) / 2.f;
        sliceLowerLeft_.y = 0.f;

        // adapt for zooming
        tgt::vec2 zoomOffset = -((sliceSize_ - (texDim2D * scaleHeight)) / 2.f) * (float)numSlicesRow;
        tgt::vec2 volDimFloat = tgt::vec2((float)volDim[voxelPosPermutation_.x], (float)volDim[voxelPosPermutation_.y]) - 1.f;
        tgt::vec2 focusOffset = voxelOffset_.get() / volDimFloat;
        focusOffset *= sliceSize_;
        sliceLowerLeft_.x += focusOffset.x;
        sliceLowerLeft_.y += zoomOffset.y + focusOffset.y;
    }

    // disable filtering on volume tex unit, if specified
    if (!filterTexture_.get()) {
        volUnit.activate();
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    transferUnit.activate();
    transferFunc_.get()->bind();

    textureMatrix_ *= tex->getMatrix();
    sliceShader_->setUniform("textureMatrix_", textureMatrix_);

    LGL_ERROR;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, canvasWidth, 0.f, canvasHeight, -1.0f, 1.0f);

    LGL_ERROR;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    LGL_ERROR;

    float depth = 0.0f;
    const size_t sliceIndex = static_cast<size_t>(sliceIndex_.get());
    for (int pos = 0, x = 0, y = 0; pos < (numSlicesCol * numSlicesRow);
        ++pos, x = pos % numSlicesCol, y = pos / numSlicesCol)
    {
        int sliceNumber = (pos + sliceIndex);
        if (sliceNumber >= numSlices)
            break;

        // calculate depth in llb/urf space
        depth = ((static_cast<float>(sliceNumber) / static_cast<float>(numSlices - 1)) - 0.5f)
            * texDim[sliceAlignment_.getValue()];

        // check whether the given slice is not within tex
        if ((depth < llb[sliceAlignment_.getValue()]) || (depth > urf[sliceAlignment_.getValue()]))
            continue;

        // map depth to [0, 1]
        depth -= texCenter[sliceAlignment_.getValue()];  // center around origin
        depth /= texDim[sliceAlignment_.getValue()];     // map to [-0.5, -0.5]
        depth += 0.5f;                                   // map to [0, 1]

        glLoadIdentity();
        glTranslatef(sliceLowerLeft_.x + (x * sliceSize_.x),
            sliceLowerLeft_.y + ((numSlicesRow - (y + 1)) * sliceSize_.y), 0.0f);
        glScalef(sliceSize_.x, sliceSize_.y, 1.0f);

        tgt::vec2 texLowerLeft = tgt::vec2(0.f);
        tgt::vec2 texUpperRight = tgt::vec2(1.f);
        glBegin(GL_QUADS);
            glTexCoord4f(texLowerLeft.x, texLowerLeft.y, depth, 1.f); glVertex2f(0.0f, 0.0f);
            glTexCoord4f(texUpperRight.x, texLowerLeft.y, depth, 1.f); glVertex2f(1.0f, 0.0f);
            glTexCoord4f(texUpperRight.x, texUpperRight.y, depth, 1.f); glVertex2f(1.0f, 1.0f);
            glTexCoord4f(texLowerLeft.x, texUpperRight.y, depth, 1.f); glVertex2f(0.0f, 1.0f);
        glEnd();

        LGL_ERROR;
    }   // for (pos

    deactivateShader();

    LGL_ERROR;

    // render a border around each slice's boundaries if desired
    //
    if (renderSliceBoundaries_.get()) {
        glLoadIdentity();
        glTranslatef(sliceLowerLeft_.x, sliceLowerLeft_.y, 0.0f);
        glScalef(sliceSize_.x * numSlicesCol, sliceSize_.y * numSlicesRow, 1.0f);
        glDepthFunc(GL_ALWAYS);
        renderSliceBoundaries();
        glDepthFunc(GL_LESS);
    }

    // If freetype is available render the slice's number and cursor information, otherwise this will do nothing
    renderInfoTexts();
    LGL_ERROR;

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    LGL_ERROR;

    // restore filtering mode
    if (!filterTexture_.get()) {
        volUnit.activate();
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    glActiveTexture(GL_TEXTURE0);
    outport_.deactivateTarget();
    LGL_ERROR;

    // propagate picking matrix, if in single slice mode and a consumer is connected
    if (singleSliceMode() && !pickingMatrix_.getLinks().empty())
        pickingMatrix_.set(generatePickingMatrix());
}

// protected methods
//

void SliceViewer::renderSliceBoundaries() const {

    int numSlicesRow = numSlicesPerRow_.get();
    int numSlicesCol = numSlicesPerCol_.get();
    tgtAssert(numSlicesRow > 0 && numSlicesCol > 0, "Invalid slice counts");

    glColor4f(boundaryColor_.get().r, boundaryColor_.get().g, boundaryColor_.get().b, boundaryColor_.get().a);
    glDisable(GL_DEPTH_TEST);
    glBegin(GL_LINE_LOOP);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(1.0f, 0.0f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(0.0f, 1.0f);
    glEnd();

    float delta_x = 1.f / numSlicesCol;
    float delta_y = 1.f / numSlicesRow;
    glBegin(GL_LINES);
    for (int x = 1; x < numSlicesCol; ++x) {
        glVertex2f(delta_x * x, 0.f);
        glVertex2f(delta_x * x, 1.f);
    }

    for (int y = 1; y < numSlicesRow; ++y) {
        glVertex3f(0.0f, delta_y * y, 0.f);
        glVertex3f(1.0f, delta_y * y, 0.f);
    }
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glColor4f(0.f, 0.f, 0.f, 0.f);

    LGL_ERROR;
}

#ifdef VRN_WITH_FONTRENDERING
void SliceViewer::renderInfoTexts() const {

    if (showCursorInfos_.isSelected("never") && !showSliceNumber_.get())
        return;

    int numSlicesRow = numSlicesPerRow_.get();
    int numSlicesCol = numSlicesPerCol_.get();
    tgtAssert(numSlicesRow > 0 && numSlicesCol > 0, "Invalid slice counts");

    tgtAssert(inport_.getData() && inport_.getData()->getVolume(), "No volume");
    Volume* volume = inport_.getData()->getVolume();
    tgt::ivec3 volDim = volume->getDimensions();
    int numSlices = volDim[sliceAlignment_.getValue()];

    glDisable(GL_DEPTH_TEST);

    // ESSENTIAL: if you don't use this, your text will become texturized!
    glActiveTexture(GL_TEXTURE0);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    LGL_ERROR;

    // render voxel position information
    if (!showCursorInfos_.isSelected("never")) {
        tgt::Font fontCursorInfos(VoreenApplication::app()->getFontPath(fontName_));
        // note: the font size may not be smaller than 8
        fontCursorInfos.setSize(fontSize_.get());

        // voxel position
        tgt::ivec3 voxelPos = tgt::iround(screenToVoxelPos(mousePosition_));
        if (mousePosition_.x != -1 && voxelPos.x != -1) { // save cursor position, if it is valid
            lastPickingPosition_ = voxelPos;
        }

        // render cursor information, if a valid picking position is available
        if (lastPickingPosition_.x != -1) {
            lastPickingPosition_ = tgt::clamp(lastPickingPosition_, tgt::ivec3(0), volDim-1);

            // determine renderString
            std::string renderStr;
            std::ostringstream oss;
            oss << "(" << lastPickingPosition_.x << " " << lastPickingPosition_.y << " " << lastPickingPosition_.z << ")";
            if (VolumeUInt8* volUint8 = dynamic_cast<VolumeUInt8*>(volume))
                oss << ": " << static_cast<int>(volUint8->voxel(lastPickingPosition_));
            else if (VolumeUInt16* volUint16 = dynamic_cast<VolumeUInt16*>(volume))
                oss << ": " <<  static_cast<int>(volUint16->voxel(lastPickingPosition_));
            else
                oss << ": " << volume->getVoxelFloat(lastPickingPosition_);
            renderStr = oss.str();

            // determine bounds and render the string
            tgt::Bounds bounds = fontCursorInfos.getBounds(tgt::vec3(6.f, 6.f, 0.f), renderStr);
            float textHeight = bounds.getURB().y - bounds.getLLF().y;
            glLoadIdentity();
            glTranslatef(0.f, outport_.getSize().y - textHeight - 25.f, 0.0f);
            fontCursorInfos.render(tgt::vec3(6.f, 6.f, 0), renderStr);
            glLoadIdentity();
        }

        LGL_ERROR;
    }

    if (showSliceNumber_.get()) {
        // note: the font size may not be smaller than 8
        tgt::Font fontSliceNumber(VoreenApplication::app()->getFontPath(fontName_));
        fontSliceNumber.setSize(fontSize_.get());

        // Therefore calculate the rendered text's bounding box by using a dummy
        // string.
        //
        std::string prefix10;
        std::string prefix100;
        std::string prefix1000;
        std::string dummy;
        if (numSlices < 100) {
            dummy = "88/88";
            prefix10 = "0";
        }
        else if (numSlices < 1000) {
            dummy = "888/888";
            prefix10 = "00";
            prefix100 = "0";
        }
        else {
            dummy = "8888/8888";
            prefix10 = "000";
            prefix100 = "00";
            prefix1000 = "0";
        }

        tgt::Bounds bounds = fontSliceNumber.getBounds(tgt::vec3(6.f, 6.f, 0.f), dummy);
        float textWidth = ceilf(bounds.getURB().x - bounds.getLLF().x);

        // Do not render the slice numbers if the slice width becomes too small to
        // prevent FTGL from creating OpenGL state errors.
        //
        if ((floorf(sliceSize_.x) > textWidth)) {
            // render the slice number information
            int sliceNumber = sliceIndex_.get();
            std::string prefix;
            for (int pos = 0, x = 0, y = 0; pos < numSlicesCol * numSlicesRow;
                ++pos, x = pos % numSlicesCol, y = pos / numSlicesCol)
            {
                glLoadIdentity();
                if ((pos + sliceNumber) >= numSlices)
                    break;
                glTranslatef(sliceLowerLeft_.x + (x * sliceSize_.x) + sliceSize_.x - textWidth - 12.f,
                    sliceLowerLeft_.y + ((numSlicesRow - (y + 1)) * sliceSize_.y), 0.0f);

                std::ostringstream oss;
                if ((sliceNumber + pos) < 10)
                    oss << prefix10;
                else if ((sliceNumber + pos) < 100)
                    oss << prefix100;
                else if ((sliceNumber + pos) < 1000)
                    oss << prefix1000;

                oss << (sliceNumber + pos) << "/" << numSlices - 1;

                fontSliceNumber.render(tgt::vec3(6,6,0), oss.str());
            }
            LGL_ERROR;
        }
    }

    glLoadIdentity();
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    LGL_ERROR;
}
#else
void SliceViewer::renderInfoTexts() const {
}
#endif  // VRN_WITH_FONTRENDERING

tgt::vec3 SliceViewer::screenToVoxelPos(tgt::ivec2 screenPos) const {

    if (!inport_.getData() || !inport_.getData()->getVolume() || !outport_.getData())
        return tgt::vec3(-1.f);

    tgt::vec3 volumeDim(inport_.getData()->getVolume()->getDimensions());
    tgt::ivec2 screenDim = outport_.getSize();

    tgt::ivec2 p(0, 0);
    p.x = screenPos.x - static_cast<int>(tgt::round(sliceLowerLeft_.x));
    p.y = (screenDim.y - screenPos.y) - static_cast<int>(tgt::round(sliceLowerLeft_.y));

    // if coordinates are negative, no slice could be hit
    if (tgt::hor(tgt::lessThan(p, tgt::ivec2(0))))
        return tgt::vec3(-1.f);

    const int numSlicesRow = numSlicesPerRow_.get();
    const int numSlicesCol = numSlicesPerCol_.get();
    const tgt::ivec2 sliceSizeInt = static_cast<tgt::ivec2>(sliceSize_);

    // if coordinates are greater than the number of slices per direction
    // times their extension in that direction, no slice could be hit either
    if ((p.x >= (sliceSizeInt.x * numSlicesCol)) || (p.y >= (sliceSizeInt.y * numSlicesRow)))
        return tgt::vec3(-1.f);

    // determine the picked slice
    const int sliceColID = p.x / sliceSizeInt.x;
    const int sliceRowID = (numSlicesRow-1) - (p.y / sliceSizeInt.y);
    const int slice = sliceColID + (sliceRowID * numSlicesCol) + sliceIndex_.get();

    // calculate the normalized position within the picked slice
    tgt::vec2 posWithinSlice(
        static_cast<float>(p.x % sliceSizeInt.x),
        static_cast<float>(p.y % sliceSizeInt.y));
    posWithinSlice /= sliceSize_;

    // calculate the normalized depth of the picked slice (texture z coordinate)
    float depth = slice / (volumeDim[voxelPosPermutation_.z] - 1.f);

    // now we have the assigned texture coordinates of the picked fragment
    tgt::vec4 texCoords(posWithinSlice, depth, 1.f);
    texCoords = tgt::clamp(texCoords, tgt::vec4(0.f), tgt::vec4(1.f));

    // apply current texture matrix to assigned tex coords
    tgt::vec3 texCoordsTransformed = (textureMatrix_ * texCoords).xyz();

    // transform final tex coords into volume coordinates
    tgt::vec3 voxPos = texCoordsTransformed * (volumeDim-1.f);
    voxPos = tgt::clamp(voxPos, tgt::vec3(0.f), tgt::vec3(volumeDim-1.f));

    return voxPos;
}

tgt::mat4 SliceViewer::generatePickingMatrix() const {

    if (!inport_.hasData() || !inport_.getData()->getVolume())
        return tgt::mat4::createIdentity();

    tgt::vec3 volumeDim(inport_.getData()->getVolume()->getDimensions());

    // 1. translate slice to origin
    tgt::mat4 originTranslation = tgt::mat4::createTranslation(tgt::vec3(-sliceLowerLeft_.x, -sliceLowerLeft_.y, 0.f));

    // 2. normalize screen coords with regard to the slice
    tgt::mat4 sliceScale = tgt::mat4::createScale(tgt::vec3(1.f / sliceSize_.x, 1.f / sliceSize_.y, 1.f / (volumeDim[voxelPosPermutation_.z] - 1.f)));

    // 3. apply current texture matrix
    tgt::mat4 textureMatrix = textureMatrix_;

    // 4. scale normalized coordinates to volume dimensions
    tgt::mat4 volumeScale = tgt::mat4::createScale(volumeDim - 1.f);

    // compose transformation matrix
    tgt::mat4 result = volumeScale * textureMatrix_ * sliceScale * originTranslation;

    return result;
}

void SliceViewer::onSliceAlignmentChange() {
    sliceAlignment_.getValue();
    switch (sliceAlignment_.getValue()) {
        case XY_PLANE:
            voxelPosPermutation_ = tgt::ivec3(0, 1, 2);
            break;
        case XZ_PLANE:
            voxelPosPermutation_ = tgt::ivec3(0, 2, 1);
            break;
        case YZ_PLANE:
            voxelPosPermutation_ = tgt::ivec3(2, 1, 0);
            break;
        default:
            break;
    }
    updateSliceProperties();
}

void SliceViewer::mouseLocalization(tgt::MouseEvent* e) {

    if (showCursorInfos_.isSelected("onMove") ||
        ((showCursorInfos_.isSelected("onClick") &&
        (e->getEventType() == tgt::MouseEvent::MOUSEPRESSEVENT || e->getEventType() == tgt::MouseEvent::WHEELEVENT))) ) {

            e->accept();
            if (mousePosition_ != e->coord()) {
                mousePosition_ = e->coord();
                invalidate();
            }
    }
    else {
        e->ignore();
    }
}

void SliceViewer::shiftEvent(tgt::MouseEvent* e) {

    e->ignore();
    if (!inport_.isReady() || !outport_.isReady())
        return;

    if (e->action() == tgt::MouseEvent::PRESSED) {
        mousePosition_ = e->coord();
        return;
    }

    tgt::vec3 volDim = tgt::vec3(inport_.getData()->getVolume()->getDimensions()) - 1.f;
    tgt::ivec2 screenDim = outport_.getSize();
    tgt::vec2 mouseCoords((float)e->coord().x, (float)e->coord().y);

    tgt::vec2 mouseOffset = mouseCoords - tgt::vec2(mousePosition_);
    mouseOffset.y *= -1.f;
    tgt::vec2 voxelOffset = voxelOffset_.get() +
        (mouseOffset / sliceSize_) * tgt::vec2(volDim[voxelPosPermutation_.x], volDim[voxelPosPermutation_.y]);
    voxelOffset = tgt::clamp(voxelOffset, voxelOffset_.getMinValue(), voxelOffset_.getMaxValue());
    voxelOffset_.set(voxelOffset);

    mousePosition_ = e->coord();
    e->accept();
}

bool SliceViewer::singleSliceMode() const {
    return (numSlicesPerRow_.get() == 1 && numSlicesPerCol_.get() == 1);
}


} // namespace voreen
