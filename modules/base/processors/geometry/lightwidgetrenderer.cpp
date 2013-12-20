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

#include "lightwidgetrenderer.h"

namespace voreen {

using tgt::vec4;
using tgt::vec3;

LightWidgetRenderer::LightWidgetRenderer()
    : GeometryRendererBase()
    , showLightWidget_("set.showLightWidget", "Show Light Widget", true)
    , isClicked_(false)
    , lightPosition_("lightPosition", "Light Source Position", tgt::vec4(2.3f, 1.5f, 1.5f, 1.f),
                     tgt::vec4(-10000.f), tgt::vec4(10000.f))
{

    moveSphereProp_ = new EventProperty<LightWidgetRenderer>(
        "mouseEvent.moveSphere", "Light widget motion",
        this, &LightWidgetRenderer::moveSphere,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT,
        tgt::MouseEvent::PRESSED | tgt::MouseEvent::MOTION | tgt::MouseEvent::RELEASED,
        tgt::Event::MODIFIER_NONE);
    addProperty(showLightWidget_);
    addProperty(lightPosition_);
    addEventProperty(moveSphereProp_);

    //lightPosition_.setViews(Property::View(Property::LIGHT_POSITION | Property::DEFAULT));

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
    ye_ambient[0]    = 0.25f;
    ye_ambient[1]    = 0.2f;
    ye_ambient[2]    = 0.07f;
    ye_ambient[3]    = 1.0f;
    ye_diffuse[0]    = 0.75f;
    ye_diffuse[1]    = 0.61f;
    ye_diffuse[2]    = 0.23f;
    ye_diffuse[3]    = 1.0f;
    ye_specular[0]    = 0.63f;
    ye_specular[1]    = 0.56f;
    ye_specular[2]    = 0.37f;
    ye_specular[3]    = 1.0f;
    ye_shininess    = 51.0f;
}

LightWidgetRenderer::~LightWidgetRenderer() {
    delete moveSphereProp_;
}

Processor* LightWidgetRenderer::create() const {
    return new LightWidgetRenderer();
}

void LightWidgetRenderer::moveSphere(tgt::MouseEvent* e) {
    LGL_ERROR;
    if (!idManager_)
        return;

    if (e->action() & tgt::MouseEvent::PRESSED) {
        if (idManager_->isHit(tgt::ivec2(e->x(), e->viewport().y - e->y() ), this)) {
            lightPosition_.toggleInteractionMode(true, this);
            e->accept();
            invalidate();
            isClicked_ = true;
            lightPositionAbs_.x = lightPosition_.get().x;
            lightPositionAbs_.y = lightPosition_.get().y;
            lightPositionAbs_.z = lightPosition_.get().z;
            startCoord_.x = e->coord().x;
            startCoord_.y = e->coord().y;
        }
        return;
    }

    if (e->action() & tgt::MouseEvent::MOTION) {
        if (isClicked_) {
            e->accept();

            LGL_ERROR;
            GLint deltaX, deltaY;

            GLint viewport[4];
            GLdouble modelview[16];
            GLdouble projection[16];
            GLdouble winX, winY, winZ;
            GLdouble posX, posY, posZ;

            deltaX = e->coord().x - startCoord_.x;
            deltaY = startCoord_.y - e->coord().y;

            tgt::mat4 projection_tgt = camera_.getProjectionMatrix(idManager_->getRenderTarget()->getSize());
            tgt::mat4 modelview_tgt = camera_.getViewMatrix();
            for (int i = 0; i < 4; ++i) {
                modelview[i+0]   = modelview_tgt[i].x;
                modelview[i+4]   = modelview_tgt[i].y;
                modelview[i+8]   = modelview_tgt[i].z;
                modelview[i+12]  = modelview_tgt[i].w;
                projection[i+0]  = projection_tgt[i].x;
                projection[i+4]  = projection_tgt[i].y;
                projection[i+8]  = projection_tgt[i].z;
                projection[i+12] = projection_tgt[i].w;
            }
            viewport[0] = 0;
            viewport[1] = 0;
            //viewport[2] = static_cast<GLint>(e->viewport().x);
            //viewport[3] = static_cast<GLint>(e->viewport().y);
            viewport[2] = static_cast<GLint>(idManager_->getRenderTarget()->getSize().x);
            viewport[3] = static_cast<GLint>(idManager_->getRenderTarget()->getSize().y);

            posX = lightPositionAbs_.x;
            posY = lightPositionAbs_.y;
            posZ = lightPositionAbs_.z;

            LGL_ERROR;
            gluProject(posX, posY,posZ,modelview,projection, viewport,&winX, &winY, &winZ);

            winX = winX + deltaX;
            winY = winY + deltaY;

            LGL_ERROR;
            gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
            LGL_ERROR;

            lightPosition_.set(vec4(static_cast<float>(posX), static_cast<float>(posY), static_cast<float>(posZ), 1.f));

            LGL_ERROR;
            invalidate();
            LGL_ERROR;
        }
        return;
    }

    if (e->action() & tgt::MouseEvent::RELEASED) {
        if (isClicked_) {
            lightPosition_.toggleInteractionMode(false, this);
            e->accept();
            isClicked_ = false;
            invalidate();
        }
        return;
    }
}

void LightWidgetRenderer::render() {
    if (showLightWidget_.get()) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);

        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        LGL_ERROR;

        glLightfv(GL_LIGHT3,GL_POSITION,light_pos);
        glLightfv(GL_LIGHT3,GL_AMBIENT,light_ambient);
        glLightfv(GL_LIGHT3,GL_DIFFUSE,light_diffuse);
        glLightfv(GL_LIGHT3,GL_SPECULAR,light_specular);
        glDisable(GL_LIGHT0);
        glEnable(GL_LIGHT3);
        glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,light_diffuse);

        GLUquadricObj* quadric = gluNewQuadric();

        //light sphere widget
        glMaterialf( GL_FRONT_AND_BACK,    GL_SHININESS,    ye_shininess);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        ye_ambient);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        ye_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_SPECULAR,    ye_specular);
        LGL_ERROR;

        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
        MatStack.pushMatrix();
        MatStack.translate(lightPosition_.get().x, lightPosition_.get().y, lightPosition_.get().z);
        LGL_ERROR;

        gluSphere(quadric,0.03f,20,20);
        LGL_ERROR;

        MatStack.popMatrix();
        LGL_ERROR;
        glPopAttrib();
        LGL_ERROR;

        gluDeleteQuadric(quadric);
        LGL_ERROR;
    }
}

void LightWidgetRenderer::renderPicking() {
    if (!idManager_)
        return;
    if (showLightWidget_.get()) {
        GLUquadricObj* quadric = gluNewQuadric();

        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
        MatStack.pushMatrix();
        MatStack.translate(lightPosition_.get().x, lightPosition_.get().y, lightPosition_.get().z);
        LGL_ERROR;

        idManager_->setGLColor(this);
        gluSphere(quadric,0.03f,20,20);
        LGL_ERROR;

        MatStack.popMatrix();
        gluDeleteQuadric(quadric);
        LGL_ERROR;
    }

}

void LightWidgetRenderer::setIDManager(IDManager* idm) {
    if (idManager_ == idm)
        return;

    idManager_ = idm;
    if (idManager_) {
        idm->registerObject(this);
    }
}

}

