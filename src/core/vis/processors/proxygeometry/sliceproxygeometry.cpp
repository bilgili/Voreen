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

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/proxygeometry/sliceproxygeometry.h"

namespace voreen {

using tgt::vec3;

const Identifier SliceProxyGeometry::setSlicePos_("set.slicePos");

SliceProxyGeometry::SliceProxyGeometry()
  : ProxyGeometry(),
    dl_(0),
    slicePos_(setSlicePos_, "Slice", tgt::vec3(1.0f), tgt::vec3(0.0f), tgt::vec3(2.0f))
{
    setName("Slice-ProxyGeometry");

    slicePos_.onChange(CallMemberAction<SliceProxyGeometry>(this, &SliceProxyGeometry::changeSlicePos));
    addProperty(&slicePos_);

    createInport("volumehandle.volumehandle");
    createCoProcessorOutport("coprocessor.proxygeometry", &Processor::call);

    setIsCoprocessor(true);
}

const std::string SliceProxyGeometry::getProcessorInfo() const {
    return "Provides a slice proxy geometry. The resulting geometry depends on the ratio \
        of the values in dim.";
}

SliceProxyGeometry::~SliceProxyGeometry() {
    if (dl_)
        glDeleteLists(dl_, 1);
}

/**
 * Renders the OpenGL list (and creates it, when needed).
 */
void SliceProxyGeometry::render() {
    if (volume_) {
        if (needsBuild_) {
            if (!dl_) {
                dl_ = glGenLists(1);
            }
            revalidateSliceGeometry();
            needsBuild_ = false;
        }
        glCallList(dl_);
    }
}

void SliceProxyGeometry::revalidateSliceGeometry() {
    vec3 geomLlf = -volumeSize_/2.f;
    vec3 geomUrb = volumeSize_/2.f;

    //vec3 texLlf = vec3(0.f);
    //vec3 texUrb = vec3(1.f);
    vec3 texLlf = (geomLlf + 1.f) /2.f;
    vec3 texUrb = (geomUrb + 1.f) /2.f;

    // recreate display list
    glNewList(dl_, GL_COMPILE);
    const tgt::vec3& slicePos = slicePos_.get();

    glBegin(GL_QUADS);
        glTexCoord3d(geomLlf[0]+slicePos.x, texLlf[1], texLlf[2]); glVertex3f(geomLlf[0]+slicePos.x, geomLlf[1], geomLlf[2]);
        glTexCoord3d(geomLlf[0]+slicePos.x, texUrb[1], texLlf[2]); glVertex3f(geomLlf[0]+slicePos.x, geomUrb[1], geomLlf[2]);
        glTexCoord3d(geomLlf[0]+slicePos.x, texUrb[1], texUrb[2]); glVertex3f(geomLlf[0]+slicePos.x, geomUrb[1], geomUrb[2]);
        glTexCoord3d(geomLlf[0]+slicePos.x, texLlf[1], texUrb[2]); glVertex3f(geomLlf[0]+slicePos.x, geomLlf[1], geomUrb[2]);

        //glTexCoord3d(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(geomLlf[0], geomLlf[1]+slicePos.y, geomLlf[2]);
        //glTexCoord3d(texLlf[0], texLlf[1], texUrb[2]); glVertex3f(geomLlf[0], geomLlf[1]+slicePos.y, geomUrb[2]);
        //glTexCoord3d(texUrb[0], texLlf[1], texUrb[2]); glVertex3f(geomUrb[0], geomLlf[1]+slicePos.y, geomUrb[2]);
        //glTexCoord3d(texUrb[0], texLlf[1], texLlf[2]); glVertex3f(geomUrb[0], geomLlf[1]+slicePos.y, geomLlf[2]);

        //glTexCoord3d(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]+slicePos.z);
        //glTexCoord3d(texUrb[0], texLlf[1], texLlf[2]); glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]+slicePos.z);
        //glTexCoord3d(texUrb[0], texUrb[1], texLlf[2]); glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]+slicePos.z);
        //glTexCoord3d(texLlf[0], texUrb[1], texLlf[2]); glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]+slicePos.z);
    glEnd();

    glEndList();

    LGL_ERROR;
}

/** reacts on changes of the slicePos_ property */
void SliceProxyGeometry::changeSlicePos() {
    needsBuild_ = true;
    invalidate();
}

} // namespace voreen
