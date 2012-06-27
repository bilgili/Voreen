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

#include "voreen/modules/base/processors/utility/distancemeasure.h"

#include "voreen/core/voreenapplication.h"

#include "tgt/textureunit.h"

#include <sstream>

using tgt::TextureUnit;

namespace voreen {


DistanceMeasure::DistanceMeasure()
    : ImageProcessor("pp_distance")
    , imgInport_(Port::INPORT, "image.input0")
    , fhpInport_(Port::INPORT, "image.input1", true, Processor::INVALID_PROGRAM, GL_RGBA16F_ARB)
    , outport_(Port::OUTPORT, "image.output")
    , volumePort_(Port::INPORT, "volume.input")
    , camera_("camera", "Camera", new tgt::Camera(tgt::vec3(0.f, 0.f, 3.5f), tgt::vec3(0.f, 0.f, 0.f), tgt::vec3(0.f, 1.f, 0.f)))
    , unit_("unit", "Display unit", "mm")
    , unitFactor_("unitFactor", "Scale factor", 1.0f, 0.0f, 1000.0f)
    , numDigits_("numDigits", "Number of digits", 2, 1, 12)
{
    addPort(imgInport_);
    addPort(fhpInport_);
    addPort(volumePort_);
    addPort(outport_);

    addProperty(camera_);
    addProperty(unit_);
    addProperty(unitFactor_);
    addProperty(numDigits_);

    mouseEventProp_ = new EventProperty<DistanceMeasure>(
        "mouseEvent.measure", "Distance measure", this,
        &DistanceMeasure::measure,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT,
        tgt::MouseEvent::MOTION | tgt::MouseEvent::PRESSED | tgt::MouseEvent::RELEASED,
        tgt::Event::CTRL, false);
    addEventProperty(mouseEventProp_);
    mouseDown_ = false;
    mouseStartPos2D_ = tgt::ivec2(0, 0);
    mouseStartPos3D_ = tgt::vec4(0.0f);

    font_ = new tgt::Font(VoreenApplication::app()->getFontPath("VeraMono.ttf"), 11, tgt::Font::BitmapFont);
}

DistanceMeasure::~DistanceMeasure() {
    delete font_;
    delete mouseEventProp_;
}

Processor* DistanceMeasure::create() const {
    return new DistanceMeasure();
}

std::string DistanceMeasure::getProcessorInfo() const {
    return "Allows to interactively measure distances.";
}

bool DistanceMeasure::isReady() {
    if (!isInitialized() || !imgInport_.isReady() || !fhpInport_.isReady() || !outport_.isReady())
        return false;

    if (!volumePort_.isReady())
        return false;

    return true;
}

tgt::ivec2 DistanceMeasure::cropToViewport(tgt::ivec2 mousePos) {
    tgt::ivec2 result = mousePos;
    tgt::ivec2 size = imgInport_.getSize();
    if (result.x < 0) result.x = 0;
    else if (result.x > size.x-1) result.x = size.x-1;
    if (result.y < 0) result.y = 0;
    else if (result.y > size.y-1) result.y = size.y-1;
    return result;
}

void DistanceMeasure::measure(tgt::MouseEvent* e) {
    if (e->action() & tgt::MouseEvent::PRESSED) {
        if (!mouseDown_) {
            mouseStartPos2D_ = cropToViewport(tgt::ivec2(e->coord().x, e->viewport().y-e->coord().y));
            tgt::vec4 fhp;
            fhpInport_.activateTarget();
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glReadPixels(mouseStartPos2D_.x, mouseStartPos2D_.y, 1, 1, GL_RGBA, GL_FLOAT, &fhp);
            if (length(fhp) > 0.0f) {
                mouseDown_ = true;
                distance_ = 0.0f;
                mouseStartPos3D_ = fhp;
                e->accept();
            }
            fhpInport_.deactivateTarget();
        }
    }
    if (e->action() & tgt::MouseEvent::MOTION) {
        if (mouseDown_) {
            // check domain
            tgt::ivec2 oldMouseCurPos2D_ = mouseCurPos2D_;
            mouseCurPos2D_ = cropToViewport(tgt::ivec2(e->coord().x, e->viewport().y-e->coord().y));
            tgt::vec4 fhp;
            fhpInport_.activateTarget();
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glReadPixels(mouseCurPos2D_.x, mouseCurPos2D_.y, 1, 1, GL_RGBA, GL_FLOAT, &fhp);
            if (length(fhp) > 0.0f) {
                distance_ = length(mouseStartPos3D_-fhp);
                mouseCurPos3D_ = fhp;
                invalidate();
            } else mouseCurPos2D_ = oldMouseCurPos2D_;
            e->accept();
            fhpInport_.deactivateTarget();
        }
    }
    if (e->action() & tgt::MouseEvent::RELEASED) {
        if (mouseDown_) {
            mouseDown_ = false;
            mouseStartPos2D_ = tgt::ivec2(0, 0);
            mouseStartPos3D_ = tgt::vec4(0.0f);
            invalidate();
            e->accept();
        }
    }
}

void DistanceMeasure::process() {
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();

    outport_.activateTarget();
    outport_.clearTarget();

    TextureUnit shadeUnit, depthUnit;
    imgInport_.bindTextures(shadeUnit.getEnum(), depthUnit.getEnum());

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", shadeUnit.getUnitNumber());
    program_->setUniform("depthTex_", depthUnit.getUnitNumber());
    imgInport_.setTextureParameters(program_, "textureParameters_");

    renderQuad();

    program_->deactivate();
    TextureUnit::setZeroUnit();

    if (mouseDown_) {
        // render text
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        // construct the string for display
        std::string label = ftos(distance_*unitFactor_.get());
        label = label.substr(0, label.find(".")+numDigits_.get()+1);
        label += " "+unit_.get();

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(-1.0f, -1.0f, 0.0f);
        float scaleFactorX = 2.0f / static_cast<float>(imgInport_.getSize().x);
        float scaleFactorY = 2.0f / static_cast<float>(imgInport_.getSize().y);
        glScalef(scaleFactorX, scaleFactorY, 1);
        glColor3f(0.0f, 0.0f, 0.0f);
        font_->render(tgt::vec3(static_cast<float>(mouseCurPos2D_.x+11), static_cast<float>(mouseCurPos2D_.y+11), 0.0f), label);
        glColor3f(1.0f, 1.0f, 1.0f);
        font_->render(tgt::vec3(static_cast<float>(mouseCurPos2D_.x+10), static_cast<float>(mouseCurPos2D_.y+10), 0.0f), label);

        glBegin(GL_LINES);
            glColor3f(0.0f, 0.0f, 0.0f);
            glVertex2i(mouseStartPos2D_.x, mouseStartPos2D_.y);
            glVertex2i(mouseCurPos2D_.x, mouseCurPos2D_.y);
            glColor3f(1.0f, 1.0f, 1.0f);
            glVertex2i(mouseStartPos2D_.x+1, mouseStartPos2D_.y+1);
            glVertex2i(mouseCurPos2D_.x+1, mouseCurPos2D_.y+1);
        glEnd();
        glPopMatrix();
        glEnable(GL_DEPTH_TEST);

        // render spherical marker at end point
        GLUquadricObj* quadric = gluNewQuadric();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        //material white plastic
        GLfloat wh_ambient[4] = {1.0, 1.0, 1.0, 1.0};
        GLfloat wh_diffuse[4] = {1.0, 1.0, 1.0, 1.0};
        GLfloat wh_specular[4] = {1.0, 1.0, 1.0, 1.0};
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 20.0f);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, wh_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, wh_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, wh_specular);
        // set modelview and projection matrices
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        tgt::loadMatrix(camera_.get()->getProjectionMatrix());
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        tgt::loadMatrix(camera_.get()->getViewMatrix());

        glPushMatrix();
            glTranslatef(mouseStartPos3D_.x, mouseStartPos3D_.y, mouseStartPos3D_.z);
            gluSphere(quadric, 0.02f, 20, 20);
        glPopMatrix();
        glTranslatef(mouseCurPos3D_.x, mouseCurPos3D_.y, mouseCurPos3D_.z);
        gluSphere(quadric, 0.02f, 20, 20);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        gluDeleteQuadric(quadric);
        glPopAttrib();
    }

    outport_.deactivateTarget();
    LGL_ERROR;

}

} // namespace voreen
