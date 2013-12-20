/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#include "camerapositionrenderer.h"

namespace voreen {

using tgt::vec4;
using tgt::vec3;

CameraPositionRenderer::CameraPositionRenderer()
    : GeometryRendererBase()
    , enable_("enable", "Enable", true)
    , displayCamera_("displayCamera", "Display Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
{
    addProperty(enable_);
    addProperty(displayCamera_);

    // light parameters
    light_pos[0] = 0.0f;
    light_pos[1] = 1.0f;
    light_pos[2] = 1.1f;
    light_pos[3] = 1.0f;
    light_ambient[0] = 1.0f;
    light_ambient[1] = 1.0f;
    light_ambient[2] = 1.0f;
    light_ambient[3] = 1.0f;
    light_diffuse[0] = 1.0f;
    light_diffuse[1] = 1.0f;
    light_diffuse[2] = 1.0f;
    light_diffuse[3] = 1.0f;
    light_specular[0] = 1.0f;
    light_specular[1] = 1.0f;
    light_specular[2] = 1.0f;
    light_specular[3] = 1.0f;

    // parameters for yellow plastic
    //ye_ambient[0]    = 0.25f;
    //ye_ambient[1]    = 0.2f;
    //ye_ambient[2]    = 0.07f;
    //ye_ambient[3]    = 1.0f;
    //ye_diffuse[0]    = 0.75f;
    //ye_diffuse[1]    = 0.61f;
    //ye_diffuse[2]    = 0.23f;
    //ye_diffuse[3]    = 1.0f;
    //ye_specular[0]    = 0.63f;
    //ye_specular[1]    = 0.56f;
    //ye_specular[2]    = 0.37f;
    //ye_specular[3]    = 1.0f;
    //ye_shininess    = 51.0f;

    ye_ambient[0]    = 0.25f;
    ye_ambient[1]    = 0.25f;
    ye_ambient[2]    = 0.25f;
    ye_ambient[3]    = 1.0f;
    ye_diffuse[0]    = 0.75f;
    ye_diffuse[1]    = 0.75f;
    ye_diffuse[2]    = 0.75f;
    ye_diffuse[3]    = 1.0f;
    ye_specular[0]    = 0.6f;
    ye_specular[1]    = 0.6f;
    ye_specular[2]    = 0.6f;
    ye_specular[3]    = 1.0f;
    ye_shininess    = 51.0f;
}

CameraPositionRenderer::~CameraPositionRenderer() {
}

Processor* CameraPositionRenderer::create() const {
    return new CameraPositionRenderer();
}

void CameraPositionRenderer::render() {
    if (enable_.get()) {
        GLUquadricObj* quadric = gluNewQuadric();

        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
        MatStack.pushMatrix();
        MatStack.translate(displayCamera_.get().getPosition().x, displayCamera_.get().getPosition().y, displayCamera_.get().getPosition().z);
        LGL_ERROR;

        float m[16];
        m[0] = displayCamera_.get().getStrafe().x;
        m[1] = displayCamera_.get().getStrafe().y;
        m[2] = displayCamera_.get().getStrafe().z;
        m[3] = 0.0f;

        m[4] = displayCamera_.get().getUpVector().x;
        m[5] = displayCamera_.get().getUpVector().y;
        m[6] = displayCamera_.get().getUpVector().z;
        m[7] = 0.0f;

        m[8] = displayCamera_.get().getLook().x;
        m[9] = displayCamera_.get().getLook().y;
        m[10] = displayCamera_.get().getLook().z;
        m[11] = 0.0f;

        m[12] = 0.0f;
        m[13] = 0.0f;
        m[14] = 0.0f;
        m[15] = 1.0f;

        glMultMatrixf(m);

        float fovy = displayCamera_.get().getFovy();
        float fovx = fovy * camera_.getRatio();
        float mult = sin(fovx * (2.0f * tgt::PIf) / 360.0f);
        //LINFO(fovy << " " << fovx);
        LGL_ERROR;

        glDepthFunc(GL_ALWAYS);

        glBegin(GL_LINES);
        glColor3f(1.0f, 1.0f, 1.0f);

        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(mult * 1.0f, 0.0f, 1.0f);

        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(mult * -1.0f, 0.0f, 1.0f);
        glEnd();

        glPushAttrib(GL_ALL_ATTRIB_BITS);

        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        LGL_ERROR;

        //light moves with cone...looks better :)
        light_pos[0] = displayCamera_.get().getPosition().x + displayCamera_.get().getUpVector().x;
        light_pos[1] = displayCamera_.get().getPosition().y + displayCamera_.get().getUpVector().y;
        light_pos[2] = displayCamera_.get().getPosition().z + displayCamera_.get().getUpVector().z;
        light_pos[3] = 1.0f;
        glLightfv(GL_LIGHT3,GL_POSITION,light_pos);
        glLightfv(GL_LIGHT3,GL_AMBIENT,light_ambient);
        glLightfv(GL_LIGHT3,GL_DIFFUSE,light_diffuse);
        glLightfv(GL_LIGHT3,GL_SPECULAR,light_specular);
        glDisable(GL_LIGHT0);
        glEnable(GL_LIGHT3);
        glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,light_diffuse);
        LGL_ERROR;
        //light sphere widget
        glMaterialf( GL_FRONT_AND_BACK,    GL_SHININESS,    ye_shininess);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        ye_ambient);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        ye_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_SPECULAR,    ye_specular);
        LGL_ERROR;

        //gluSphere(quadric,0.005f,20,20);
        float conel = 0.01f;
        gluCylinder(quadric,0.0f, conel * mult, conel, 20,20);
        glPopAttrib();

        MatStack.popMatrix();
        LGL_ERROR;

        gluDeleteQuadric(quadric);
        LGL_ERROR;

        glDepthFunc(GL_LESS);
    }
}

}

