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

#include "voreen/modules/base/processors/utility/intensitymeasure.h"

#include "voreen/core/voreenapplication.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

IntensityMeasure::IntensityMeasure()
    : ImageProcessor("pp_distance")
    , imgInport_(Port::INPORT, "image.input0")
    , fhpInport_(Port::INPORT, "image.input1")
    , outport_(Port::OUTPORT, "image.output")
    , volumePort0_(Port::INPORT, "volume.input0")
    , volumePort1_(Port::INPORT, "volume.input1")
    , volumePort2_(Port::INPORT, "volume.input2")
    , volumePort3_(Port::INPORT, "volume.input3")
    , applyWorldToVoxel_("world2voxel", "Apply world to voxel matrix", false)
    , useVolume0_("useVolume0", "Use volume 0", true)
    , useVolume1_("useVolume1", "Use volume 1", false)
    , useVolume2_("useVolume2", "Use volume 2", false)
    , useVolume3_("useVolume3", "Use volume 3", false)
    , unit0_("unit0", "Display unit 0", "HU")
    , unit1_("unit1", "Display unit 1", "HU")
    , unit2_("unit2", "Display unit 2", "HU")
    , unit3_("unit3", "Display unit 3", "HU")
    , unitFactor0_("unitFactor0", "Scale factor 0", 1.0f, 0.0f, 1000.0f)
    , unitFactor1_("unitFactor1", "Scale factor 1", 1.0f, 0.0f, 1000.0f)
    , unitFactor2_("unitFactor2", "Scale factor 2", 1.0f, 0.0f, 1000.0f)
    , unitFactor3_("unitFactor3", "Scale factor 3", 1.0f, 0.0f, 1000.0f)
    , numDigits0_("numDigits0", "Number of digits 0", 2, 1, 12)
    , numDigits1_("numDigits1", "Number of digits 1", 2, 1, 12)
    , numDigits2_("numDigits2", "Number of digits 2", 2, 1, 12)
    , numDigits3_("numDigits3", "Number of digits 3", 2, 1, 12)
{
    addPort(imgInport_);
    addPort(fhpInport_);
    addPort(volumePort0_);
    addPort(volumePort1_);
    addPort(volumePort2_);
    addPort(volumePort3_);
    addPort(outport_);

    addProperty(applyWorldToVoxel_);
    addProperty(useVolume0_);
    addProperty(unit0_);
    addProperty(unitFactor0_);
    addProperty(numDigits0_);
    addProperty(useVolume1_);
    addProperty(unit1_);
    addProperty(unitFactor1_);
    addProperty(numDigits1_);
    addProperty(useVolume2_);
    addProperty(unit2_);
    addProperty(unitFactor2_);
    addProperty(numDigits2_);
    addProperty(useVolume3_);
    addProperty(unit3_);
    addProperty(unitFactor3_);
    addProperty(numDigits3_);

    mouseEventProp_ = new EventProperty<IntensityMeasure>(
        "mouseEvent.measure", "Intensity measure", this,
        &IntensityMeasure::measure,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT,
        tgt::MouseEvent::MOTION | tgt::MouseEvent::PRESSED | tgt::MouseEvent::RELEASED,
        tgt::Event::ALT, false);
    addEventProperty(mouseEventProp_);
    mouseDown_ = false;
    mousePos2D_ = tgt::ivec2(0, 0);
    mousePos3D_ = tgt::Color(0.0f);

    font_ = new tgt::Font(VoreenApplication::app()->getFontPath("VeraMono.ttf"), 11, tgt::BitmapFont);
}

IntensityMeasure::~IntensityMeasure() {
    delete font_;
    delete mouseEventProp_;
}

Processor* IntensityMeasure::create() const {
    return new IntensityMeasure();
}

std::string IntensityMeasure::getProcessorInfo() const {
    return "Allows to interactively measure intensities.";
}

bool IntensityMeasure::isReady() {
    if (!isInitialized() || !imgInport_.isReady() || !fhpInport_.isReady() || !outport_.isReady())
        return false;

    if (!volumePort0_.isReady())
        return false;

    return true;
}

tgt::ivec2 IntensityMeasure::cropToViewport(tgt::ivec2 mousePos) {
    tgt::ivec2 result = mousePos;
    tgt::ivec2 size = imgInport_.getSize();
    if (result.x < 0) result.x = 0;
    else if (result.x > size.x-1) result.x = size.x-1;
    if (result.y < 0) result.y = 0;
    else if (result.y > size.y-1) result.y = size.y-1;
    return result;
}

tgt::vec3 IntensityMeasure::transformWorld2Voxel(tgt::vec3 worldPos, VolumePort volPort) {
    tgt::vec3 llf = volPort.getData()->getVolume()->getLLF();
    tgt::vec3 urb = volPort.getData()->getVolume()->getURB();
    tgt::vec3 cubesize = urb - llf;
    tgt::mat4 invmat;
    volPort.getData()->getVolume()->getTransformation().invert(invmat);
    tgt::vec3 result = invmat * worldPos;
    result -= llf;
    result /= cubesize;

    if ((result.x < 0.0f) || (result.y < 0.0f) || (result.z < 0.0f)) {
        //return false;
        result.x = std::max(result.x, 0.0f);
        result.y = std::max(result.y, 0.0f);
        result.z = std::max(result.z, 0.0f);
    }

    if ((result.x > 1.0f) || (result.y > 1.0f) || (result.z > 1.0f)) {
        //return false;
        result.x = std::min(result.x, 1.0f);
        result.y = std::min(result.y, 1.0f);
        result.z = std::min(result.z, 1.0f);
    }

    result.z = 1.0f - result.z;

    result *= static_cast<tgt::vec3>(volPort.getData()->getVolume()->getDimensions());

    return result;
}

void IntensityMeasure::measure(tgt::MouseEvent* e) {
    if (((e->action() & tgt::MouseEvent::PRESSED) && !mouseDown_) ||
        ((e->action() & tgt::MouseEvent::MOTION) && mouseDown_)) {
        mousePos2D_ = cropToViewport(tgt::ivec2(e->coord().x, e->viewport().y-e->coord().y));
        fhpInport_.getColorTexture()->downloadTexture();
        tgt::Color curColor = fhpInport_.getColorTexture()->texelAsFloat(mousePos2D_);
        if (length(curColor) > 0.0f) {
            if (!mouseDown_) mouseDown_ = true;
            mousePos3D_ = curColor;
            invalidate();
            e->accept();
        }
    }
    if (e->action() & tgt::MouseEvent::RELEASED) {
        if (mouseDown_) {
            mouseDown_ = false;
            mousePos2D_ = tgt::ivec2(0, 0);
            mousePos3D_ = tgt::Color(0.0f);
            invalidate();
            e->accept();
        }
    }
}

void IntensityMeasure::process() {
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();

    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextureUnit shadeUnit, depthUnit;
    imgInport_.bindTextures(shadeUnit.getEnum(), depthUnit.getEnum());

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", shadeUnit.getUnitNumber());
    program_->setUniform("depthTex_", depthUnit.getUnitNumber());
    imgInport_.setTextureParameters(program_, "textureParameters_");

    renderQuad();

    outport_.deactivateTarget();

    program_->deactivate();
    TextureUnit::setZeroUnit();

    if (mouseDown_) {
        // render text
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        // construct the string for display
        std::string label;
        if (useVolume0_.get()) {
            tgt::vec3 samplePos = static_cast<tgt::vec3>(mousePos3D_.xyz());
            if (applyWorldToVoxel_.get())
                samplePos = transformWorld2Voxel(samplePos, volumePort0_);
            Volume* volume = volumePort0_.getData()->getVolume();
            float intensity = volume->getVoxelFloat(static_cast<tgt::ivec3>(samplePos*static_cast<tgt::vec3>(volume->getDimensions())));
            std::string label0 = ftos(intensity*unitFactor0_.get());
            label0 = label0.substr(0, label0.find(".")+numDigits0_.get()+1);
            label0 += " "+unit0_.get();
            label0 += " ";
            label += label0;
        }
        if (useVolume1_.get()) {
            tgt::vec3 samplePos = static_cast<tgt::vec3>(mousePos3D_.xyz());
            if (applyWorldToVoxel_.get())
                samplePos = transformWorld2Voxel(samplePos, volumePort1_);
            Volume* volume = volumePort1_.getData()->getVolume();
            float intensity = volume->getVoxelFloat(static_cast<tgt::ivec3>(samplePos*static_cast<tgt::vec3>(volume->getDimensions())));
            std::string label1 = ftos(intensity*unitFactor1_.get());
            label1 = label1.substr(0, label1.find(".")+numDigits1_.get()+1);
            label1 += " "+unit1_.get();
            label1 += " ";
            label += label1;
        }
        if (useVolume2_.get()) {
            tgt::vec3 samplePos = static_cast<tgt::vec3>(mousePos3D_.xyz());
            if (applyWorldToVoxel_.get())
                samplePos = transformWorld2Voxel(samplePos, volumePort2_);
            Volume* volume = volumePort2_.getData()->getVolume();
            float intensity = volume->getVoxelFloat(static_cast<tgt::ivec3>(samplePos*static_cast<tgt::vec3>(volume->getDimensions())));
            std::string label2 = ftos(intensity*unitFactor2_.get());
            label2 = label2.substr(0, label2.find(".")+numDigits2_.get()+1);
            label2 += " "+unit2_.get();
            label2 += " ";
            label += label2;
        }
        if (useVolume3_.get()) {
            tgt::vec3 samplePos = static_cast<tgt::vec3>(mousePos3D_.xyz());
            if (applyWorldToVoxel_.get())
                samplePos = transformWorld2Voxel(samplePos, volumePort3_);
            Volume* volume = volumePort3_.getData()->getVolume();
            float intensity = volume->getVoxelFloat(static_cast<tgt::ivec3>(samplePos*static_cast<tgt::vec3>(volume->getDimensions())));
            std::string label3 = ftos(intensity*unitFactor3_.get());
            label3 = label3.substr(0, label3.find(".")+numDigits3_.get()+1);
            label3 += " "+unit3_.get();
            label3 += " ";
            label += label3;
        }

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(-1.0f, -1.0f, 0.0f);
        float scaleFactorX = 2.0f / static_cast<float>(imgInport_.getSize().x);
        float scaleFactorY = 2.0f / static_cast<float>(imgInport_.getSize().y);
        glScalef(scaleFactorX, scaleFactorY, 1);
        glColor3f(0.0f, 0.0f, 0.0f);
        font_->renderWithLayout(tgt::vec3(static_cast<float>(mousePos2D_.x+11), static_cast<float>(mousePos2D_.y+11), 0.0f), label);
        glColor3f(1.0f, 1.0f, 1.0f);
        font_->renderWithLayout(tgt::vec3(static_cast<float>(mousePos2D_.x+10), static_cast<float>(mousePos2D_.y+10), 0.0f), label);

        glBegin(GL_LINES);
            glColor3f(0.0f, 0.0f, 0.0f);
            glVertex2i(mousePos2D_.x-5, mousePos2D_.y);
            glVertex2i(mousePos2D_.x+5, mousePos2D_.y);
            glVertex2i(mousePos2D_.x, mousePos2D_.y-5);
            glVertex2i(mousePos2D_.x, mousePos2D_.y+5);
            glColor3f(1.0f, 1.0f, 1.0f);
            glVertex2i(mousePos2D_.x-5, mousePos2D_.y+1);
            glVertex2i(mousePos2D_.x+5, mousePos2D_.y+1);
            glVertex2i(mousePos2D_.x+1, mousePos2D_.y-5);
            glVertex2i(mousePos2D_.x+1, mousePos2D_.y+5);
        glEnd();
        glPopMatrix();
        glEnable(GL_DEPTH_TEST);
    }
}

} // namespace voreen
