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

#include "voreen/core/vis/processors/proxygeometry/axialsliceproxygeometry.h"

namespace voreen {

const std::string AxialSliceProxyGeometry::setAxis_("Axis");
const std::string AxialSliceProxyGeometry::setBegin_("Begin");
const std::string AxialSliceProxyGeometry::setThickness_("Thickness");

using tgt::vec3;

AxialSliceProxyGeometry::AxialSliceProxyGeometry()
    : ProxyGeometry()
    , dl_(0)
    , alignmentProp_("sliceAlignmentProp", "slice alignment")
    , begin_(setBegin_, "Begin")
    , thickness_(setThickness_, "Thickness", 5)
{

    alignmentProp_.addOption("sagittal","SAGITTAL", AxialSliceProxyGeometry::SAGITTAL);
    alignmentProp_.addOption("axial",   "AXIAL",    AxialSliceProxyGeometry::AXIAL);
    alignmentProp_.addOption("coronal", "CORONAL",  AxialSliceProxyGeometry::CORONAL);
    alignmentProp_.onChange(CallMemberAction<AxialSliceProxyGeometry>(this, &AxialSliceProxyGeometry::setAxis));
    addProperty(alignmentProp_);

    begin_.onChange(CallMemberAction<AxialSliceProxyGeometry>(this, &AxialSliceProxyGeometry::setBegin));
    thickness_.onChange(CallMemberAction<AxialSliceProxyGeometry>(this, &AxialSliceProxyGeometry::setThickness));

    addProperty(begin_);
    addProperty(thickness_);
}

AxialSliceProxyGeometry::~AxialSliceProxyGeometry() {
    if (dl_)
        glDeleteLists(dl_,1);
}

Processor* AxialSliceProxyGeometry::create() const {
    return new AxialSliceProxyGeometry();
}

const std::string AxialSliceProxyGeometry::getProcessorInfo() const {
    return "Provides a Slice-based proxy geometry. An axis can be chosen and slices of \
           varying thickness can be created.";
}

void AxialSliceProxyGeometry::render() {
    if (volume_) {
        if (needsBuild_) {
            revalidateGeometry();
            needsBuild_ = false;
        }
        if (dl_)
            glCallList(dl_);
    }
}

void AxialSliceProxyGeometry::revalidateGeometry() {
    float beginRatio = begin_.get() / 100.f;
    float thicknessRatio = thickness_.get() / 100.f;

    vec3 geomLlf = (-volumeSize_/2.f) + volumeCenter_;
    vec3 geomUrb = (volumeSize_/2.f) + volumeCenter_;

    int alignment = alignmentProp_.getValue();

    geomLlf[alignment] = -(volumeSize_[alignment] * (thicknessRatio/2.f)) + volumeCenter_[alignment];
    geomUrb[alignment] =  (volumeSize_[alignment] * (thicknessRatio/2.f)) + volumeCenter_[alignment];

    vec3 texLlf = vec3(0.f);
    vec3 texUrb = vec3(1.f);
    texLlf[alignment] = beginRatio;
    texUrb[alignment] = beginRatio + thicknessRatio;

    if (!dl_)
        dl_ = glGenLists(1);

    glNewList(dl_, GL_COMPILE);

    glBegin(GL_QUADS);
        // back face
         glTexCoord3f(texLlf[0],texUrb[1],texUrb[2]); glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
         glTexCoord3f(texLlf[0],texLlf[1],texUrb[2]); glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
         glTexCoord3f(texUrb[0],texLlf[1],texUrb[2]); glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
         glTexCoord3f(texUrb[0],texUrb[1],texUrb[2]); glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
         // front face
         glTexCoord3f(texUrb[0],texLlf[1],texLlf[2]); glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
         glTexCoord3f(texLlf[0],texLlf[1],texLlf[2]); glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
         glTexCoord3f(texLlf[0],texUrb[1],texLlf[2]); glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
         glTexCoord3f(texUrb[0],texUrb[1],texLlf[2]); glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
         // top face
         glTexCoord3f(texUrb[0],texUrb[1],texLlf[2]); glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
         glTexCoord3f(texLlf[0],texUrb[1],texLlf[2]); glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
         glTexCoord3f(texLlf[0],texUrb[1],texUrb[2]); glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
         glTexCoord3f(texUrb[0],texUrb[1],texUrb[2]); glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
         // bottom face
         glTexCoord3f(texLlf[0],texLlf[1],texUrb[2]); glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
         glTexCoord3f(texLlf[0],texLlf[1],texLlf[2]); glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
         glTexCoord3f(texUrb[0],texLlf[1],texLlf[2]); glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
         glTexCoord3f(texUrb[0],texLlf[1],texUrb[2]); glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
         // right face
         glTexCoord3f(texUrb[0],texLlf[1],texUrb[2]); glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
         glTexCoord3f(texUrb[0],texLlf[1],texLlf[2]); glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
         glTexCoord3f(texUrb[0],texUrb[1],texLlf[2]); glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
         glTexCoord3f(texUrb[0],texUrb[1],texUrb[2]); glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
         // left face
         glTexCoord3f(texLlf[0],texUrb[1],texLlf[2]); glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
         glTexCoord3f(texLlf[0],texLlf[1],texLlf[2]); glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
         glTexCoord3f(texLlf[0],texLlf[1],texUrb[2]); glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
         glTexCoord3f(texLlf[0],texUrb[1],texUrb[2]); glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
    glEnd();

    glEndList();

    LGL_ERROR;
}

void AxialSliceProxyGeometry::setAxis() {
    needsBuild_ = true;
    invalidate();
}

void AxialSliceProxyGeometry::setBegin() {
    if ((begin_.get() + thickness_.get()) > 100)
        begin_.set(100 - thickness_.get());

    needsBuild_ = true;
    invalidate();
}

void AxialSliceProxyGeometry::setThickness() {
    if ((begin_.get() + thickness_.get()) > 100)
        thickness_.set(100 - begin_.get());

    needsBuild_ = true;
    invalidate();
}

} // namespace
