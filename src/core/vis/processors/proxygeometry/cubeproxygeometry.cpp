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

#include "voreen/core/vis/processors/proxygeometry/cubeproxygeometry.h"
#include "voreen/core/vis/interaction/camerainteractionhandler.h"

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
    , clipLeftX_(setLeftClipPlane_, "Left clipping plane", 0, 0, 100000, true)
    , clipRightX_(setRightClipPlane_, "Right clipping plane", 0, 0, 10000, true)
    , clipUpY_(setTopClipPlane_, "Top clipping plane", 0, 0, 100000, true)
    , clipDownY_(setBottomClipPlane_, "Bottom clipping plane", 0, 0, 100000, true)
    , clipFrontZ_(setFrontClipPlane_, "Front clipping plane", 0, 0, 100000, true)
    , clipBackZ_(setBackClipPlane_, "Back clipping plane", 0, 0, 100000, true)
    , brickSelectionPriority_("regionPriority", "Region Priority", 1, 0, 100, true)
    , dl_(0)
    , useVirtualClipplane_("enableVirtualClipplane", "Use virtual clipping plane", false)
    , clipPlane_("virtualClipplane", "Plane equation", vec4(1.f/5.f, 2.f/5.f, 1.f, 0.3f),
                 tgt::vec4(-10.f), tgt::vec4(10.f))
    , oldHandle_(0)
{

    //These actions are needed to check if the planes went beyond each other:
    clipLeftX_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setLeftClipPlane));
    clipRightX_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setRightClipPlane));
    clipUpY_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setTopClipPlane));
    clipDownY_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setBottomClipPlane));
    clipFrontZ_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setFrontClipPlane));
    clipBackZ_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setBackClipPlane));
    addProperty(useClipping_);

    // test of condition system
    clipPlane_.setVisible(false);
    Call1ParMemberAction<Property, bool> spva(&clipPlane_, &Property::setVisible, true);
    Call1ParMemberAction<Property, bool> spia(&clipPlane_, &Property::setVisible, false);
    useVirtualClipplane_.onValueEqual(true, spva, spia);
    useVirtualClipplane_.set(false);

    addProperty(clipLeftX_);
    addProperty(clipRightX_);
    addProperty(clipDownY_);
    addProperty(clipUpY_);
    addProperty(clipFrontZ_);
    addProperty(clipBackZ_);
    addProperty(brickSelectionPriority_);

    addProperty(applyDatasetTransformationMatrix_);

    //addProperty(useVirtualClipplane_);
}

void CubeProxyGeometry::process() {
    if (inport_.hasChanged()) {
        volumeSize_ = inport_.getData()->getVolumeGL()->getVolume()->getCubeSize();
        tgt::ivec3 numSlices = inport_.getData()->getVolumeGL()->getVolume()->getDimensions();

        int oldMaxX = clipRightX_.getMaxValue();
        int oldMaxY = clipUpY_.getMaxValue();
        int oldMaxZ = clipBackZ_.getMaxValue();

        clipLeftX_.setMaxValue(numSlices.x);
        clipRightX_.setMaxValue(numSlices.x);

        clipDownY_.setMaxValue(numSlices.y);
        clipUpY_.setMaxValue(numSlices.y);

        clipFrontZ_.setMaxValue(numSlices.z);
        clipBackZ_.setMaxValue(numSlices.z);

        if ( (clipLeftX_.get() == 0)
          && (clipRightX_.get() == 0)
          && (clipDownY_.get() == 0)
          && (clipUpY_.get() == 0)
          && (clipFrontZ_.get() == 0)
          && (clipBackZ_.get() == 0) ) {
            //deserialized old (percent normalized) values...reset clipping:
            clipRightX_.set(numSlices.x);
            clipUpY_.set(numSlices.y);
            clipBackZ_.set(numSlices.z);
        }
        else {
            if(oldHandle_ != 0) {
                clipLeftX_.set(0);
                clipDownY_.set(0);
                clipFrontZ_.set(0);
                clipRightX_.set(numSlices.x);
                clipUpY_.set(numSlices.y);
                clipBackZ_.set(numSlices.z);
            }

            if ( (oldMaxX == clipRightX_.get()) || (clipRightX_.get() > clipRightX_.getMaxValue()) )
                clipRightX_.set(clipRightX_.getMaxValue());
            if ( (oldMaxY == clipUpY_.get()) || (clipUpY_.get() > clipUpY_.getMaxValue()) )
                clipUpY_.set(clipUpY_.getMaxValue());
            if ( (oldMaxZ == clipBackZ_.get()) || (clipBackZ_.get() > clipBackZ_.getMaxValue()) )
                clipBackZ_.set(clipBackZ_.getMaxValue());

            oldHandle_ = inport_.getData();
        }
        //invalidate();
    }
    revalidateCubeGeometry();
}

const std::string CubeProxyGeometry::getProcessorInfo() const {
    return "Provides a simple cube proxy with clipping.";
}

CubeProxyGeometry::~CubeProxyGeometry() {
    if (dl_)
        glDeleteLists(dl_, 1);
}

/**
 * Renders the OpenGL display list (and creates it, when needed).
 */
void CubeProxyGeometry::render() {

    tgtAssert(inport_.isReady(), "render() called with an not-ready inport");
    tgtAssert(inport_.getData()->getVolume(), "no volume");

    if (dl_) {
        
        // transform bounding box by dataset transformation matrix
        if (applyDatasetTransformationMatrix_.get()) {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            tgt::multMatrix(inport_.getData()->getVolume()->getTransformation());
        }

        glCallList(dl_);

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
    tgt::ivec3 numSlices = inport_.getData()->getVolumeGL()->getVolume()->getDimensions();

    float clipLeft = 0;
    float clipRight = 0;
    float clipUp = 0;
    float clipDown = 0;
    float clipFront = 0;
    float clipBack = 0;

    if (useClipping_.get()) {
        clipLeft = clipLeftX_.get() / (float) numSlices.x;
        clipRight = (numSlices.x - clipRightX_.get()) / (float) numSlices.x;

        clipDown = clipDownY_.get() / (float) numSlices.y;
        clipUp = (numSlices.y - clipUpY_.get()) / (float) numSlices.y;

        clipFront = clipFrontZ_.get() / (float) numSlices.z;
        clipBack = (numSlices.z - clipBackZ_.get()) / (float) numSlices.z;

        // clipping along the xyz axes
        geomLlf[0] += (volumeSize_[0] * clipLeft);
        geomLlf[1] += (volumeSize_[1] * clipDown);
        geomLlf[2] += (volumeSize_[2] * clipFront);

        geomUrb[0] -= (volumeSize_[0] * clipRight);
        geomUrb[1] -= (volumeSize_[1] * clipUp);
        geomUrb[2] -= (volumeSize_[2] * clipBack);
    }

    if (!dl_)
        dl_ = glGenLists(1);

    // recreate display list
    glNewList(dl_, GL_COMPILE);

    if (useVirtualClipplane_.get()) {
        tgt::plane clipPlane(normalize(vec3(clipPlane_.get().elem)), clipPlane_.get().w);

        std::vector<vec3> clippedPolygon;
        clipPlane.clipAAB(geomLlf, geomUrb, clippedPolygon);

        // draw patch
        glBegin(GL_POLYGON);
        for (size_t i = 0; i < clippedPolygon.size(); ++i)
            glVertex3fv(clippedPolygon[i].elem);
        glEnd();

        // set clip plane in OpenGL
        glEnable(GL_CLIP_PLANE0);
        tgt::setClipPlane(GL_CLIP_PLANE0, clipPlane);
    }

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

    if (useVirtualClipplane_.get())
        glDisable(GL_CLIP_PLANE0);

    glEndList();

    LGL_ERROR;
}

void CubeProxyGeometry::resetClippingPlanes() {
    clipLeftX_.set(0);
    clipRightX_.set(0);
    clipFrontZ_.set(0);
    clipBackZ_.set(0);
    clipDownY_.set(0);
    clipUpY_.set(0);
}

void CubeProxyGeometry::defineBoxBrickingRegion() {
    vec3 llfPlane = vec3(static_cast<float>(clipLeftX_.get()),
                     static_cast<float>(clipDownY_.get()),
                     static_cast<float>(clipFrontZ_.get()) );

    vec3 urbPlane = vec3(static_cast<float>(clipRightX_.get()),
                     static_cast<float>(clipUpY_.get()),
                     static_cast<float>(clipBackZ_.get()) );

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

void CubeProxyGeometry::setTopClipPlane() {
    if ((clipUpY_.get() < clipDownY_.get()))
        clipDownY_.set(clipUpY_.get());
}

void CubeProxyGeometry::setBottomClipPlane() {
    if ((clipUpY_.get() < clipDownY_.get()))
        clipUpY_.set(clipDownY_.get());
}

void CubeProxyGeometry::setFrontClipPlane() {
    if ((clipFrontZ_.get() > clipBackZ_.get()))
        clipBackZ_.set(clipFrontZ_.get());
}

void CubeProxyGeometry::setBackClipPlane() {
    if ((clipBackZ_.get() < clipFrontZ_.get()))
        clipFrontZ_.set(clipBackZ_.get());
}
} // namespace
