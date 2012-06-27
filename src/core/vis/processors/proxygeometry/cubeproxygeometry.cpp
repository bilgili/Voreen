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
#include "voreen/core/vis/messagedistributor.h"

#include "tgt/vector.h"
#include "tgt/plane.h"
#include "tgt/quadric.h"
#include "tgt/glmath.h"

namespace voreen {

using tgt::vec3;
using tgt::vec4;

CubeProxyGeometry::CubeProxyGeometry()
    : ProxyGeometry(),
      useClipping_(setUseClipping_, "Use clipping", true),
      clipLeftX_(setLeftClipPlane_, "Left clipping plane", 0, 0, 100, true),
      clipRightX_(setRightClipPlane_, "Right clipping plane", 0, 0, 100, true),
      clipUpY_(setTopClipPlane_, "Top clipping plane", 0, 0, 100, true),
      clipDownY_(setBottomClipPlane_, "Bottom clipping plane", 0, 0, 100, true),
      clipFrontZ_(setFrontClipPlane_, "Front clipping plane", 0, 0, 100, true),
      clipBackZ_(setBackClipPlane_, "Back clipping plane", 0, 0, 100, true),
      dl_(0),
      useVirtualClipplane_("switch.virtualClipplane", "Use virtual clipping plane", false),
      clipPlane_("set.virtualClipplane", "Plane equation", vec4(1.f/5.f, 2.f/5.f, 1.f, 0.3f),
                 tgt::vec4(-10.f), tgt::vec4(10.f))
{
    setName("CubeProxyGeometry");

    clipLeftX_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setLeftClipPlane));
    clipRightX_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setRightClipPlane));
    clipUpY_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setTopClipPlane));
    clipDownY_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setBottomClipPlane));
    clipFrontZ_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setFrontClipPlane));
    clipBackZ_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::setBackClipPlane));

    useClipping_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::markAsChanged));
    useVirtualClipplane_.onChange(CallMemberAction<CubeProxyGeometry>(this, &CubeProxyGeometry::markAsChanged));

    addProperty(&useClipping_);
    addProperty(&useVirtualClipplane_);

    // test of condition system
    clipPlane_.setVisible(false);
    Call1ParMemberAction<Property, bool> spva(&clipPlane_, &Property::setVisible, true);
    Call1ParMemberAction<Property, bool> spia(&clipPlane_, &Property::setVisible, false);
    useVirtualClipplane_.onValueEqual(true, spva, spia);
    useVirtualClipplane_.set(false);

    addProperty(&clipPlane_);
    addProperty(&clipLeftX_);
    addProperty(&clipRightX_);
    addProperty(&clipDownY_);
    addProperty(&clipUpY_);
    addProperty(&clipFrontZ_);
    addProperty(&clipBackZ_);

    createInport("volumehandle.volumehandle");
    createCoProcessorOutport("coprocessor.proxygeometry", &Processor::call);

    setIsCoprocessor(true);
}

const std::string CubeProxyGeometry::getProcessorInfo() const {
    return "Provides a simple cube proxy with clipping.";
}

CubeProxyGeometry::~CubeProxyGeometry() {
    if (dl_)
        glDeleteLists(dl_, 1);
}

bool CubeProxyGeometry::getUseVirtualClipplane() {
    return useVirtualClipplane_.get();
}

/**
 * Renders the OpenGL display list (and creates it, when needed).
 */
void CubeProxyGeometry::render() {
    if (volume_) {
        if (needsBuild_) {
            revalidateCubeGeometry();
            needsBuild_ = false;
        }
        if (dl_)
            glCallList(dl_);
    }
}

tgt::vec3 CubeProxyGeometry::getClipPlaneLDF() {
    return tgt::vec3(static_cast<float>(clipLeftX_.get()),
                     static_cast<float>(clipDownY_.get()),
                     static_cast<float>(clipFrontZ_.get()) );
}

tgt::vec3 CubeProxyGeometry::getClipPlaneRUB() {
    return tgt::vec3(static_cast<float>(clipRightX_.get()),
                     static_cast<float>(clipUpY_.get()),
                     static_cast<float>(clipBackZ_.get()) );
}

void CubeProxyGeometry::revalidateCubeGeometry() {
    // The original size of the volume
    vec3 geomLlf = -(volumeSize_ / 2.f) + volumeCenter_;
    vec3 geomUrb = (volumeSize_  / 2.f) + volumeCenter_;

    float clipLeft = 0;
    float clipRight = 0;
    float clipUp = 0;
    float clipDown = 0;
    float clipFront = 0;
    float clipBack = 0;

    if (useClipping_.get()) {
        clipLeft = clipLeftX_.get() / 100.f;
        clipRight = clipRightX_.get() / 100.f;
        clipUp = clipUpY_.get() / 100.f;
        clipDown = clipDownY_.get() / 100.f;
        clipFront = clipFrontZ_.get() / 100.f;
        clipBack = clipBackZ_.get() / 100.f;

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

void CubeProxyGeometry::processMessage(Message* msg, const Identifier& dest) {
    ProxyGeometry::processMessage(msg, dest);

    if (msg->id_ == setUseClipping_) {
        useClipping_.set(msg->getValue<bool>());
        markAsChanged();
    }
    else if (msg->id_ == setLeftClipPlane_) {
        clipLeftX_.set(msg->getValue<int>());
        setLeftClipPlane();
    }
    else if (msg->id_ == setRightClipPlane_) {
        clipRightX_.set(msg->getValue<int>());
        setRightClipPlane();
    }
    else if (msg->id_ == setTopClipPlane_) {
        clipUpY_.set(msg->getValue<int>());
        setTopClipPlane();
    }
    else if (msg->id_ == setBottomClipPlane_) {
        clipDownY_.set(msg->getValue<int>());
        setBottomClipPlane();
    }
    else if (msg->id_ == setFrontClipPlane_) {
        clipFrontZ_.set(msg->getValue<int>());
        setFrontClipPlane();
    }
    else if (msg->id_ == setBackClipPlane_) {
        clipBackZ_.set(msg->getValue<int>());
        setBackClipPlane();
    }
    else if (msg->id_ == "switch.virtualClipplane") {
        useVirtualClipplane_.set(msg->getValue<bool>());
        markAsChanged();
    }
    else if (msg->id_ == "set.virtualClipplane") {
        clipPlane_.set(msg->getValue<vec4>());
        markAsChanged();
    }
    else if (msg->id_ == resetClipPlanes_) {
        resetClippingPlanes();
        markAsChanged();
    }
}

// methods for reaction on property changes
void CubeProxyGeometry::setLeftClipPlane() {
    if ((clipLeftX_.get() + clipRightX_.get()) >= 100)
        clipRightX_.set(100 - clipLeftX_.get());

    markAsChanged();
}

void CubeProxyGeometry::setRightClipPlane() {
    if ((clipLeftX_.get() + clipRightX_.get()) >= 100)
        clipLeftX_.set(100 - clipRightX_.get());

    markAsChanged();
}

void CubeProxyGeometry::setTopClipPlane() {
    if ((clipUpY_.get() + clipDownY_.get()) >= 100)
        clipDownY_.set(100 - clipUpY_.get());

    markAsChanged();
}

void CubeProxyGeometry::setBottomClipPlane() {
    if ((clipUpY_.get() + clipDownY_.get()) >= 100)
        clipUpY_.set(100 - clipDownY_.get());

    markAsChanged();
}

void CubeProxyGeometry::setFrontClipPlane() {
    if ((clipFrontZ_.get() + clipBackZ_.get()) >= 100)
        clipBackZ_.set(100-clipFrontZ_.get());

    markAsChanged();
}

void CubeProxyGeometry::setBackClipPlane() {
    if ((clipBackZ_.get() + clipFrontZ_.get()) >= 100)
        clipFrontZ_.set(100-clipBackZ_.get());

    markAsChanged();
}

void CubeProxyGeometry::markAsChanged() {
    needsBuild_ = true;
    invalidate();
}

} // namespace
