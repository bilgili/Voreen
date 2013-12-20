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

#include "quadricrenderer.h"

#include "tgt/material.h"

namespace voreen {

using tgt::vec4;
using tgt::vec3;

QuadricRenderer::QuadricRenderer()
    : GeometryRendererBase()
    , enabled_("enabled", "Enabled", true)
    , quadricType_("quadricType", "Quadric Type")
    , position_("position", "Position", tgt::vec3(0.f), tgt::vec3(-99.0f), tgt::vec3(99.0f))
    , start_("start", "Start Position", tgt::vec3(0.f), tgt::vec3(-99.0f), tgt::vec3(99.0f))
    , end_("end", "End Position", tgt::vec3(1.f), tgt::vec3(-99.0f), tgt::vec3(99.0f))
    , radius_("radius", "Radius", 0.2f, 0.001f, 5.0f)
    , color_("color", "Color", tgt::Color(0.75f, 0.25f, 0.f, 1.f))
    , applyLighting_("applyLighting", "Apply Lighting", true)
    , lightPosition_("lightPosition", "Light Source Position", tgt::vec4(-2.f, 2.f, 2.f, 1.f), tgt::vec4(-10), tgt::vec4(10))
    , lightAmbient_("lightAmbient", "Ambient Light", tgt::Color(0.4f, 0.4f, 0.4f, 1.f))
    , lightDiffuse_("lightDiffuse", "Diffuse Light", tgt::Color(0.6f, 0.6f, 0.6f, 1.f))
    , lightSpecular_("lightSpecular", "Specular Light", tgt::Color(0.4f, 0.4f, 0.4f, 1.f))
    , materialShininess_("materialShininess", "Shininess", 60.f, 0.1f, 128.f)
{
    quadricType_.addOption("cylinder", "Cylinder");
    quadricType_.addOption("sphere",   "Sphere");

    addProperty(enabled_);
    addProperty(quadricType_);
    addProperty(start_);
    addProperty(end_);
    addProperty(position_);
    addProperty(radius_);
    addProperty(color_);

    addProperty(applyLighting_);
    addProperty(lightPosition_);
    addProperty(lightAmbient_);
    addProperty(lightDiffuse_);
    addProperty(lightSpecular_);
    addProperty(materialShininess_);

    color_.setViews(Property::COLOR);
    //lightPosition_.setViews(Property::View(Property::LIGHT_POSITION | Property::DEFAULT));
    lightAmbient_.setViews(Property::COLOR);
    lightDiffuse_.setViews(Property::COLOR);
    lightSpecular_.setViews(Property::COLOR);

    // assign lighting properties to property group
    lightPosition_.setGroupID("lighting");
    lightAmbient_.setGroupID("lighting");
    lightDiffuse_.setGroupID("lighting");
    lightSpecular_.setGroupID("lighting");
    materialShininess_.setGroupID("lighting");
    setPropertyGroupGuiName("lighting", "Lighting Parameters");

    quadricType_.onChange(CallMemberAction<QuadricRenderer>(this, &QuadricRenderer::adjustPropertyVisibilities));
    applyLighting_.onChange(CallMemberAction<QuadricRenderer>(this, &QuadricRenderer::adjustPropertyVisibilities));
}

Processor* QuadricRenderer::create() const {
    return new QuadricRenderer();
}

void QuadricRenderer::initialize() throw (tgt::Exception) {
    GeometryRendererBase::initialize();
    adjustPropertyVisibilities();
}

void QuadricRenderer::render() {

    if (!enabled_.get())
        return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glShadeModel(GL_SMOOTH);

    GLUquadricObj* quadric = gluNewQuadric();
    LGL_ERROR;

    if (applyLighting_.get()) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient_.get().elem);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse_.get().elem);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular_.get().elem);
        glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 128.f);

        tgt::Material material(color_.get(), color_.get(), color_.get(), materialShininess_.get());
        material.activate();

        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
        MatStack.pushMatrix();
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition_.get().elem);
        MatStack.popMatrix();
    }
    else { // no lighting
        glColor4fv(color_.get().elem);
    }
    LGL_ERROR;

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();

    if (quadricType_.isSelected("cylinder")) {
        MatStack.translate(start_.get().x, start_.get().y, start_.get().z);

        //calculate correct rotation matrix:
        vec3 rotz = normalize(end_.get() - start_.get());
        vec3 roty = normalize(vec3(rotz.y, -rotz.z, 0.0f));
        vec3 rotx = cross(roty, rotz);

        float m[16];

        m[0] = rotx.x;
        m[1] = rotx.y;
        m[2] = rotx.z;
        m[3] = 0.0f;

        m[4] = roty.x;
        m[5] = roty.y;
        m[6] = roty.z;
        m[7] = 0.0f;

        m[8] = rotz.x;
        m[9] = rotz.y;
        m[10] = rotz.z;
        m[11] = 0.0f;

        m[12] = 0.0f;
        m[13] = 0.0f;
        m[14] = 0.0f;
        m[15] = 1.0f;
        glMultMatrixf(m);

        float l = length(start_.get() - end_.get());
        gluCylinder(quadric, radius_.get(), radius_.get(), l, 200, 200);
    }
    else if (quadricType_.isSelected("sphere")) {
        MatStack.translate(position_.get().x, position_.get().y, position_.get().z);
        gluSphere(quadric, radius_.get(), 20, 20);
    }
    else {
        LERROR("Unknown quadric type: " << quadricType_.get());
    }
    LGL_ERROR;

    MatStack.popMatrix();
    glPopAttrib();

    gluDeleteQuadric(quadric);
    LGL_ERROR;
}

void QuadricRenderer::adjustPropertyVisibilities() {
    bool cylinder = quadricType_.isSelected("cylinder");
    bool sphere = quadricType_.isSelected("sphere");
    bool lighting = applyLighting_.get();

    start_.setVisible(cylinder);
    end_.setVisible(cylinder);
    position_.setVisible(sphere);
    setPropertyGroupVisible("lighting", lighting);
}

}
