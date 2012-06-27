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

#include "voreen/modules/base/processors/proxygeometry/cubeproxygeometry.h"
#include "voreen/core/interaction/camerainteractionhandler.h"

#include "tgt/vector.h"
#include "tgt/plane.h"
#include "tgt/quadric.h"
#include "tgt/glmath.h"

namespace voreen {

using tgt::vec3;
using tgt::vec4;

CubeProxyGeometry::CubeProxyGeometry()
    : ProxyGeometry()
    , useClipping_(setUseClipping_, "Use clipping", true)
    , clipLeftX_(setLeftClipPlane_, "Left clipping plane (x)", 0.0f, 0.0f, 10000.0f)
    , clipRightX_(setRightClipPlane_, "Right clipping plane (-x)", 0.0f, 0.0f, 1000.0f)
    , clipFrontY_(setBottomClipPlane_, "Front clipping plane (y)", 0.0f, 0.0f, 10000.0f)
    , clipBackY_(setTopClipPlane_, "Back clipping plane (-y)", 0.0f, 0.0f, 10000.0f)
    , clipBottomZ_(setFrontClipPlane_, "Bottom clipping plane (z)", 0.0f, 0.0f, 10000.0f)
    , clipTopZ_(setBackClipPlane_, "Top clipping plane (-z)", 0.0f, 0.0f, 10000.0f)
    , brickSelectionPriority_("regionPriority", "Region Priority", 1, 0, 100)
    , displayList_(0)
    , oldHandle_(0)
{
    //These actions are needed to check if the planes went beyond each other:
    clipLeftX_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setLeftClipPlane));
    clipRightX_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setRightClipPlane));
    clipBackY_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setBackClipPlane));
    clipFrontY_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setFrontClipPlane));
    clipBottomZ_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setBottomClipPlane));
    clipTopZ_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setTopClipPlane));
    addProperty(useClipping_);

    addProperty(clipLeftX_);
    addProperty(clipRightX_);
    addProperty(clipFrontY_);
    addProperty(clipBackY_);
    addProperty(clipBottomZ_);
    addProperty(clipTopZ_);
    addProperty(brickSelectionPriority_);

    addProperty(applyDatasetTransformationMatrix_);
}

void CubeProxyGeometry::process() {
    if (inport_.hasChanged()) {
        volumeSize_ = inport_.getData()->getVolume()->getCubeSize();
        tgt::ivec3 numSlices = inport_.getData()->getVolume()->getDimensions();

        float oldMaxX = clipRightX_.getMaxValue();
        float oldMaxY = clipBackY_.getMaxValue();
        float oldMaxZ = clipTopZ_.getMaxValue();

        clipLeftX_.setMaxValue(numSlices.x-1.f);
        clipRightX_.setMaxValue(numSlices.x-1.f);

        clipFrontY_.setMaxValue(numSlices.y-1.f);
        clipBackY_.setMaxValue(numSlices.y-1.f);

        clipBottomZ_.setMaxValue(numSlices.z-1.f);
        clipTopZ_.setMaxValue(numSlices.z-1.f);

        if ( (clipLeftX_.get() == 0.0f)
          && (clipRightX_.get() == 0.0f)
          && (clipFrontY_.get() == 0.0f)
          && (clipBackY_.get() == 0.0f)
          && (clipBottomZ_.get() == 0.0f)
          && (clipTopZ_.get() == 0.0f) ) {
            //deserialized old (percent normalized) values...reset clipping:
            clipRightX_.set(numSlices.x-1.f);
            clipBackY_.set(numSlices.y-1.f);
            clipTopZ_.set(numSlices.z-1.f);
        }
        else {
            if ( (oldMaxX == clipRightX_.get()) || (clipRightX_.get() > clipRightX_.getMaxValue()) )
                clipRightX_.set(clipRightX_.getMaxValue());
            if ( (oldMaxY == clipBackY_.get()) || (clipBackY_.get() > clipBackY_.getMaxValue()) )
                clipBackY_.set(clipBackY_.getMaxValue());
            if ( (oldMaxZ == clipTopZ_.get()) || (clipTopZ_.get() > clipTopZ_.getMaxValue()) )
                clipTopZ_.set(clipTopZ_.getMaxValue());

            oldHandle_ = inport_.getData();
        }
        //invalidate();
    }
    revalidateCubeGeometry();
}

std::string CubeProxyGeometry::getProcessorInfo() const {
    return "Provides a simple cube proxy with axis-aligned clipping planes."
           "<p><span style=\"color: red; font-weight: bold;\">Deprecated:</span> "
           "Use CubeMeshProxyGeometry and MeshEntryExitPoints instead.</p>";
}

CubeProxyGeometry::~CubeProxyGeometry() {
    if (displayList_)
        glDeleteLists(displayList_, 1);
}

/**
 * Renders the OpenGL display list (and creates it, when needed).
 */
void CubeProxyGeometry::render() {

    tgtAssert(inport_.isReady(), "render() called with an not-ready inport");
    tgtAssert(inport_.getData()->getVolume(), "no volume");

    if (displayList_) {

        // transform bounding box by dataset transformation matrix
        if (applyDatasetTransformationMatrix_.get()) {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            tgt::multMatrix(inport_.getData()->getVolume()->getTransformation());
        }

        glCallList(displayList_);

        // restore matrix stack
        if (applyDatasetTransformationMatrix_.get()) {
            glPopMatrix();
        }
    }
}

void CubeProxyGeometry::revalidateCubeGeometry() {
    // The original size of the volume
    vec3 geomLlf = -(volumeSize_ / 2.f) + volumeCenter_;
    vec3 geomUrb = (volumeSize_  / 2.f) + volumeCenter_;
    tgt::ivec3 numSlices = inport_.getData()->getVolume()->getDimensions();

    float clipLeft = 0;
    float clipRight = 0;
    float clipUp = 0;
    float clipDown = 0;
    float clipFront = 0;
    float clipBack = 0;

    if (useClipping_.get()) {
        clipLeft = clipLeftX_.get() / (float) numSlices.x;
        clipRight = (numSlices.x - clipRightX_.get() - 1.0f) / (float) numSlices.x;

        clipDown = clipFrontY_.get() / (float) numSlices.y;
        clipUp = (numSlices.y - clipBackY_.get() - 1.0f) / (float) numSlices.y;

        clipFront = clipBottomZ_.get() / (float) numSlices.z;
        clipBack = (numSlices.z - clipTopZ_.get() - 1.0f) / (float) numSlices.z;

        // clipping along the xyz axes
        geomLlf[0] += (volumeSize_[0] * clipLeft);
        geomLlf[1] += (volumeSize_[1] * clipDown);
        geomLlf[2] += (volumeSize_[2] * clipFront);

        geomUrb[0] -= (volumeSize_[0] * clipRight);
        geomUrb[1] -= (volumeSize_[1] * clipUp);
        geomUrb[2] -= (volumeSize_[2] * clipBack);
    }

    if (!displayList_)
        displayList_ = glGenLists(1);

    // recreate display list
    glNewList(displayList_, GL_COMPILE);

    glBegin(GL_QUADS);
        // back face
        glTexCoord3f(clipLeft,1-clipUp,1-clipBack); glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
        glTexCoord3f(clipLeft,clipDown,1-clipBack); glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glTexCoord3f(1-clipRight,clipDown,1-clipBack); glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        glTexCoord3f(1-clipRight,1-clipUp,1-clipBack); glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
        // front face
        glTexCoord3f(1-clipRight,clipDown,clipFront); glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
        glTexCoord3f(clipLeft,clipDown,clipFront); glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glTexCoord3f(clipLeft,1-clipUp,clipFront); glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(1-clipRight,1-clipUp,clipFront); glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        // top face
        glTexCoord3f(1-clipRight,1-clipUp,clipFront); glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(clipLeft,1-clipUp,clipFront); glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(clipLeft,1-clipUp,1-clipBack); glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
        glTexCoord3f(1-clipRight,1-clipUp,1-clipBack); glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
        // bottom face
        glTexCoord3f(clipLeft,clipDown,1-clipBack); glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glTexCoord3f(clipLeft,clipDown,clipFront); glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glTexCoord3f(1-clipRight,clipDown,clipFront); glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
        glTexCoord3f(1-clipRight,clipDown,1-clipBack); glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        // right face
        glTexCoord3f(1-clipRight,clipDown,1-clipBack); glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        glTexCoord3f(1-clipRight,clipDown,clipFront); glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
        glTexCoord3f(1-clipRight,1-clipUp,clipFront); glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(1-clipRight,1-clipUp,1-clipBack); glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
        // left face
        glTexCoord3f(clipLeft,1-clipUp,clipFront); glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(clipLeft,clipDown,clipFront); glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glTexCoord3f(clipLeft,clipDown,1-clipBack); glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glTexCoord3f(clipLeft,1-clipUp,1-clipBack); glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
    glEnd();

    glEndList();

    LGL_ERROR;
}

void CubeProxyGeometry::resetClippingPlanes() {
    clipLeftX_.set(0.0f);
    clipRightX_.set(0.0f);
    clipBottomZ_.set(0.0f);
    clipTopZ_.set(0.0f);
    clipFrontY_.set(0.0f);
    clipBackY_.set(0.0f);
}

void CubeProxyGeometry::defineBoxBrickingRegion() {
    vec3 llfPlane = vec3(clipLeftX_.get(),
                     clipFrontY_.get(),
                     clipBottomZ_.get() );

    vec3 urbPlane = vec3(clipRightX_.get(),
                     clipBackY_.get(),
                     clipTopZ_.get() );

    vec3 geomLlf = -(volumeSize_ / 2.f) + volumeCenter_;
    vec3 geomUrb = (volumeSize_  / 2.f) + volumeCenter_;

    llfPlane = llfPlane / 100.f;
    urbPlane = urbPlane / 100.f;

    llfPlane = geomLlf + (volumeSize_ * llfPlane);
    urbPlane = geomUrb - (volumeSize_ * urbPlane);

    if (currentVolumeHandle_) {
        LargeVolumeManager* lvm = currentVolumeHandle_->getLargeVolumeManager();
        if (lvm) {
            lvm->addBoxBrickingRegion(brickSelectionPriority_.get(), llfPlane, urbPlane);
        }
    }
}

// methods for reaction on property changes
void CubeProxyGeometry::setLeftClipPlane() {
    if ((clipLeftX_.get() > clipRightX_.get()))
        clipRightX_.set(clipLeftX_.get());
}

void CubeProxyGeometry::setRightClipPlane() {
    if ((clipLeftX_.get() > clipRightX_.get()))
        clipLeftX_.set(clipRightX_.get());
}

void CubeProxyGeometry::setBackClipPlane() {
    if ((clipBackY_.get() < clipFrontY_.get()))
        clipFrontY_.set(clipBackY_.get());
}

void CubeProxyGeometry::setFrontClipPlane() {
    if ((clipBackY_.get() < clipFrontY_.get()))
        clipBackY_.set(clipFrontY_.get());
}

void CubeProxyGeometry::setBottomClipPlane() {
    if ((clipBottomZ_.get() > clipTopZ_.get()))
        clipTopZ_.set(clipBottomZ_.get());
}

void CubeProxyGeometry::setTopClipPlane() {
    if ((clipTopZ_.get() < clipBottomZ_.get()))
        clipBottomZ_.set(clipTopZ_.get());
}
} // namespace
