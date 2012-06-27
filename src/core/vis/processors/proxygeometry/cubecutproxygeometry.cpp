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

#include "voreen/core/vis/processors/proxygeometry/cubecutproxygeometry.h"

#include "tgt/vector.h"

namespace voreen {

using tgt::vec3;

CubeCutProxyGeometry::CubeCutProxyGeometry()
  : ProxyGeometry()
  , cutCube_("cutCube", "Render cut cube", false)
  , cubeSize_("cutCubeSize", "Cutted cube size", vec3(25.f, 50.f, 75.f), vec3(0.f), vec3(100.f)),
    dl_(0)
{

    CallMemberAction<CubeCutProxyGeometry> cma(this, &CubeCutProxyGeometry::onSettingsChange);
    cutCube_.onChange(cma);
    cubeSize_.onChange(cma);

    addProperty(cutCube_);
    addProperty(cubeSize_);
}

const std::string CubeCutProxyGeometry::getProcessorInfo() const {
    return "Provides a simple clipping cube proxy.";
}

CubeCutProxyGeometry::~CubeCutProxyGeometry() {
    if (dl_)
        glDeleteLists(dl_, 1);
}

Processor* CubeCutProxyGeometry::create() const {
    return new CubeCutProxyGeometry();
}

/**
 * Renders the OpenGL list (and creates it, when needed).
 */
void CubeCutProxyGeometry::render() {
    if (inport_.hasData()) {
        if (needsBuild_) {
            if (!dl_)
                dl_ = glGenLists(1);
            revalidateCubeGeometry();
            needsBuild_ = false;
        }
        glCallList(dl_);
    }
}

void CubeCutProxyGeometry::renderCubeWithCutting() {
    vec3 geomLlf = (-volumeSize_ / 2.f) + volumeCenter_;
    vec3 geomUrb = (volumeSize_ / 2.f) + volumeCenter_;
    vec3 cubeVec = ((cubeSize_.get()/vec3(50.f)) - vec3(1.f)) + volumeCenter_;

    vec3 texLlf = vec3(0.f);
    vec3 texUrb = vec3(1.f);

    glNewList(dl_, GL_COMPILE);
    glBegin(GL_QUADS);
        // back face
        glTexCoord3f(texLlf[0], texUrb[1], texUrb[2]); glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texUrb[2]); glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glTexCoord3f(texUrb[0], texLlf[1], texUrb[2]); glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        glTexCoord3f(texUrb[0], texUrb[1], texUrb[2]); glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
        // front face
        //o---oo---o   y
        //! 2 !!   !   !
        //o---o! 1 !   !
        //     !   !   0---- x
        //     o---o
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(cubeVec[0], geomLlf[1], geomLlf[2]);
        glTexCoord3f(texLlf[0], texUrb[1], texLlf[2]); glVertex3f(cubeVec[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(texUrb[0], texUrb[1], texLlf[2]); glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(texUrb[0], texLlf[1], texLlf[2]); glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);

        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(geomLlf[0], cubeVec[1], geomLlf[2]);
        glTexCoord3f(texLlf[0], texUrb[1], texLlf[2]); glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(texLlf[0], texUrb[1], texLlf[2]); glVertex3f(cubeVec[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(cubeVec[0], cubeVec[1], geomLlf[2]);

        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(geomLlf[0], geomLlf[1], cubeVec[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(geomLlf[0], cubeVec[1], cubeVec[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(cubeVec[0], cubeVec[1], cubeVec[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(cubeVec[0], geomLlf[1], cubeVec[2]);

        // top face
        glTexCoord3f(texUrb[0], texUrb[1], texLlf[2]); glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(texLlf[0], texUrb[1], texLlf[2]); glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(texLlf[0], texUrb[1], texUrb[2]); glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
        glTexCoord3f(texUrb[0], texUrb[1], texUrb[2]); glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);

        // bottom face
        //      o---oo---o   0--- x
        //     / 2 //   /   /
        //    o---o/ 1 /   /
        //        /   /   z
        //       o---o
        glTexCoord3f(texLlf[0], texLlf[1], texUrb[2]); glVertex3f(cubeVec[0], geomLlf[1], geomUrb[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(cubeVec[0], geomLlf[1], geomLlf[2]);
        glTexCoord3f(texUrb[0], texLlf[1], texLlf[2]); glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
        glTexCoord3f(texUrb[0], texLlf[1], texUrb[2]); glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);

        glTexCoord3f(texLlf[0], texLlf[1], texUrb[2]); glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(geomLlf[0], geomLlf[1], cubeVec[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(cubeVec[0], geomLlf[1], cubeVec[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texUrb[2]); glVertex3f(cubeVec[0], geomLlf[1], geomUrb[2]);

        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(geomLlf[0], cubeVec[1], cubeVec[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(geomLlf[0], cubeVec[1], geomLlf[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(cubeVec[0], cubeVec[1], geomLlf[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(cubeVec[0], cubeVec[1], cubeVec[2]);

        // right face
        glTexCoord3f(texUrb[0], texLlf[1], texUrb[2]); glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        glTexCoord3f(texUrb[0], texLlf[1], texLlf[2]); glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
        glTexCoord3f(texUrb[0], texUrb[1], texLlf[2]); glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(texUrb[0], texUrb[1], texUrb[2]); glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);

        // left face
        //         o
        //        /!
        //       / !         y
        //      o  !         !
        //     o!1 !         !
        //    /!!  o         !
        //   / !!  /         0
        //  /  o! /         /
        // o 2/ !/         /
        // ! /  o         /
        // !/            z
        // o
        glTexCoord3f(texLlf[0], texLlf[1], texUrb[2]); glVertex3f(geomLlf[0], cubeVec[1], geomUrb[2]);
        glTexCoord3f(texLlf[0], texUrb[1], texUrb[2]); glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
        glTexCoord3f(texLlf[0], texUrb[1], texLlf[2]); glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(geomLlf[0], cubeVec[1], geomLlf[2]);

        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(geomLlf[0], geomLlf[1], cubeVec[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texUrb[2]); glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texUrb[2]); glVertex3f(geomLlf[0], cubeVec[1], geomUrb[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(geomLlf[0], cubeVec[1], cubeVec[2]);

        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(cubeVec[0], geomLlf[1], geomLlf[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(cubeVec[0], geomLlf[1], cubeVec[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(cubeVec[0], cubeVec[1], cubeVec[2]);
        glTexCoord3f(texLlf[0], texLlf[1], texLlf[2]); glVertex3f(cubeVec[0], cubeVec[1], geomLlf[2]);
    glEnd();
    glEndList();
}

void CubeCutProxyGeometry::renderCuttedCube() {
    vec3 geomLlf = -volumeSize_ / 2.f;
    vec3 geomUrb = volumeSize_ / 2.f;
    vec3 cubeVec = (cubeSize_.get()/vec3(50.f)) - vec3(1.f);
    glNewList(dl_, GL_COMPILE);
    glBegin(GL_QUADS);
        // back face
        glVertex3f(geomLlf[0], cubeVec[1], cubeVec[2]);
        glVertex3f(geomLlf[0], geomLlf[1], cubeVec[2]);
        glVertex3f(cubeVec[0], geomLlf[1], cubeVec[2]);
        glVertex3f(cubeVec[0], cubeVec[1], cubeVec[2]);
        // front face
        glVertex3f(cubeVec[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomLlf[0], cubeVec[1], geomLlf[2]);
        glVertex3f(cubeVec[0], cubeVec[1], geomLlf[2]);
        // top face
        glVertex3f(cubeVec[0], cubeVec[1], geomLlf[2]);
        glVertex3f(geomLlf[0], cubeVec[1], geomLlf[2]);
        glVertex3f(geomLlf[0], cubeVec[1], cubeVec[2]);
        glVertex3f(cubeVec[0], cubeVec[1], cubeVec[2]);
        // bottom face
        glVertex3f(geomLlf[0], geomLlf[1], cubeVec[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glVertex3f(cubeVec[0], geomLlf[1], geomLlf[2]);
        glVertex3f(cubeVec[0], geomLlf[1], cubeVec[2]);
        // right face
        glVertex3f(cubeVec[0], geomLlf[1], cubeVec[2]);
        glVertex3f(cubeVec[0], geomLlf[1], geomLlf[2]);
        glVertex3f(cubeVec[0], cubeVec[1], geomLlf[2]);
        glVertex3f(cubeVec[0], cubeVec[1], cubeVec[2]);
        // left face
        glVertex3f(geomLlf[0], cubeVec[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomLlf[1], cubeVec[2]);
        glVertex3f(geomLlf[0], cubeVec[1], cubeVec[2]);
    glEnd();
    glEndList();
}

void CubeCutProxyGeometry::revalidateCubeGeometry() {
    if (cutCube_.get())
        renderCuttedCube();
    else
        renderCubeWithCutting();
    LGL_ERROR;

}

void CubeCutProxyGeometry::onSettingsChange() {
    needsBuild_ = true;
}

} // namespace
