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

#include "voreen/core/vis/processors/image/crosshair.h"
#include "voreen/core/application.h"
#include "voreen/core/vis/voreenpainter.h"

#include "tgt/vector.h"
#include "tgt/texturemanager.h"

#include <vector>

namespace voreen {

CrossHair::CrossHair()
    : ImageProcessor("pp_crosshair")
    , crossHairPrg_(0)
    , color_("crosshaircolor", "Crosshair-Color", tgt::vec4(1.0f,0.0f,0.0f,1.0f))
    , useChromaDepth_("ueschromadepth", "Use chroma depth gradient", false)
    , removeBackground_("removeBackground", "Remove background depth", true)
    , textureSize_("textureSize", "Size of transfer texture", 256, 0, 8192)
    , cutAfterDecimalPlaces_("cutAfterDecimalPlaces", "Cut depth values after decimal place", 4)
    , showCrossHair_(false)
    , depthToColorTex_(0)
    , inport_(Port::INPORT, "image.inport")
    , outport_(Port::OUTPORT, "image.outport")
{

    //FIXME: memory leak (EventAction?)
    eventProp_ = new TemplateMouseEventProperty<CrossHair>("Show crosshair", new EventAction<CrossHair, tgt::MouseEvent>(this, &CrossHair::eventMethod), tgt::MouseEvent::PRESSED | tgt::MouseEvent::MOTION | tgt::MouseEvent::RELEASED, tgt::Event::SHIFT, tgt::MouseEvent::MOUSE_ALL);

    chromaDepthTex_ = 0;
    addProperty(color_);
    addProperty(useChromaDepth_);
    addProperty(removeBackground_);
    addProperty(textureSize_);
    addProperty(cutAfterDecimalPlaces_);

    addEventProperty(eventProp_);

    mappingModeProp_ = new OptionProperty<int>("mappingmode", "Mapping");
    mappingModeProp_->addOption("relative", "relative", 0);
    mappingModeProp_->addOption("absolute", "absolute", 1);

    mappingModeProp_->selectByValue(0);
    addProperty(mappingModeProp_);
}

CrossHair::~CrossHair() {
    delete depthToColorTex_;
}

const std::string CrossHair::getProcessorInfo() const {
    return "This class renders a cross hair cursor.";
}

Processor* CrossHair::create() const {
    CrossHair* ch = new CrossHair();
    return ch;
}

void CrossHair::initialize() throw (VoreenException) {
    chromaDepthTex_ = TexMgr.load(VoreenApplication::app()->getTransFuncPath("chromadepthspectrum.bmp"),
                                  tgt::Texture::LINEAR, false, true, false);

    depthToColorTex_ = new tgt::Texture(tgt::ivec3(textureSize_.get(), 1, 1));

    ImageProcessor::initialize();
    crossHairPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", shaderFilename_ + ".frag",
            generateHeader() + "#define VRN_CROSSHAIR\n", false, false);
    if(!crossHairPrg_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }
    initialized_ = true;
    invalidate(Processor::INVALID_PROGRAM);
    if(getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
}

void CrossHair::eventMethod(tgt::MouseEvent* e) {
    if (e->button() & tgt::MouseEvent::PRESSED) {
        showCrossHair_ = true;
        cursorPos_ = e->coord();
        e->accept();
        invalidate();
    }

    if (e->button() & tgt::MouseEvent::MOTION) {
        if (showCrossHair_) {
            cursorPos_ = e->coord();
            e->accept();
            invalidate();
        }
        else
            e->ignore();
    }

    if (e->button() & tgt::MouseEvent::RELEASED) {
        if (showCrossHair_) {
            showCrossHair_ = false;
            e->accept();
            invalidate();
        }
        else
            e->ignore();
     }
}

float CrossHair::cutDecimalPlaces(float number, int decimalPlaces) {
    decimalPlaces = static_cast<int>(pow(10.0f, decimalPlaces));
    return floor(number*decimalPlaces)/decimalPlaces;
}

tgt::Color CrossHair::calcPositionsColor(int pos) {
    tgt::Color color;
    if (useChromaDepth_.get()) {
        GLubyte* texPtr = chromaDepthTex_->getPixelData();
        color.r = texPtr[pos*3];
        color.g = texPtr[pos*3+1];
        color.b = texPtr[pos*3+2];
        color.a = 255;

    }
    else {
        float intensity = 255.0f-(255.0f/(textureSize_.get()-1)*pos);
        color.r = color_.get().r * intensity;
        color.g = color_.get().g * intensity;
        color.b = color_.get().b * intensity;
        color.a = 255;
    }
    return color;
}

void CrossHair::createAbsDepthToColorTex() {

    GLubyte* texPtr = depthToColorTex_->getPixelData();

    for (int i=0; i<textureSize_.get(); ++i)
    {
        tgt::Color color = calcPositionsColor(i);
        texPtr[i*4] = static_cast<GLubyte>(color.r);
        texPtr[i*4+1] = static_cast<GLubyte>(color.g);
        texPtr[i*4+2] = static_cast<GLubyte>(color.b);
        texPtr[i*4+3] = static_cast<GLubyte>(color.a);

    }
    depthToColorTex_->uploadTexture();
    minValue_ = 0.0f;
    maxValue_ = 1.0f;
}

void CrossHair::createRelDepthToColorTex(RenderPort* source) {
    tgt::ivec2 size = outport_.getSize();
    if (cursorPos_.x < 0 || cursorPos_.y < 0 || cursorPos_.x >= size.x || cursorPos_.y >= size.y)
        return;
    std::vector<float> depthLine;
    source->getDepthTexture()->downloadTexture();
    float* pixels = (float*)source->getDepthTexture()->getPixelData();

    depthLine.reserve(size_t(size.x + size.y));
    float value;
    minValue_ = 1.0f;
    maxValue_ = 0.0f;
    for (int i=0; i<size.x; ++i) {
        value = pixels[size.x*(size.y-cursorPos_.y-1)+i];
        depthLine.push_back(cutDecimalPlaces(value, cutAfterDecimalPlaces_.get()));
    }
    for (int i=0; i<size.y; ++i) {
        value = pixels[size.x*i+cursorPos_.x];
        depthLine.push_back(cutDecimalPlaces(value, cutAfterDecimalPlaces_.get()));
    }
    source->getDepthTexture()->destroy();

    std::sort(depthLine.begin(), depthLine.end());

    // remove duplicates
    std::vector<float>::iterator newEnd = std::unique(depthLine.begin(), depthLine.end());
    depthLine.erase(newEnd, depthLine.end());

    // remove background
    if (removeBackground_.get()) {
        if ((*depthLine.rbegin()) == 1.0f)
            depthLine.pop_back();
    }

    int numDiffValues =  depthLine.size();

    if (numDiffValues == 0) { // image contains only background
        minValue_ = maxValue_ = 1.0f;
        return;
    }

    minValue_ = depthLine[0];
    maxValue_ = depthLine[numDiffValues-1];

    LDEBUG("Number if different values: " << numDiffValues << " (" << minValue_ << "-" << maxValue_ << ")");

    GLubyte* texPtr = depthToColorTex_->getPixelData();

    if (depthLine.size() < 3) {
        for (int i=0; i<textureSize_.get(); ++i) {
            texPtr[i*4] = 255;
            texPtr[i*4+1] = 0;
            texPtr[i*4+2] = 0;
            texPtr[i*4+3] = 255;
        }
        return;
    }

    float intervalStart = depthLine[0];
    float intervalEnd;

    for (size_t i=1; i<depthLine.size(); ++i) {
        intervalEnd = depthLine[i];
        int texStart = static_cast<int>((textureSize_.get()-1)*(intervalStart-minValue_)/(maxValue_-minValue_));
        int texEnd = static_cast<int>((textureSize_.get()-1)*(intervalEnd-minValue_)/(maxValue_-minValue_));

        for (int j=texStart; j<=texEnd; ++j)
        {
            tgt::Color color = calcPositionsColor(j);
            texPtr[j*4] = static_cast<GLubyte>(color.r);
            texPtr[j*4+1] = static_cast<GLubyte>(color.g);
            texPtr[j*4+2] = static_cast<GLubyte>(color.b);
            texPtr[j*4+3] = static_cast<GLubyte>(color.a);
        }
        intervalStart = intervalEnd;
    }
    depthToColorTex_->uploadTexture();
}

std::string CrossHair::generateHeader() {
    std::string header = ImageProcessor::generateHeader();
    return header;
}

void CrossHair::compile() {
    ImageProcessor::compile();
    if (crossHairPrg_)
        crossHairPrg_->rebuild();
}

void CrossHair::process() {
    if(getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();

    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    inport_.bindTextures(tm_.getGLTexUnit(shadeTexUnit_), tm_.getGLTexUnit(depthTexUnit_));
    LGL_ERROR;

    // copy image to the outport image
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex_", tm_.getTexUnit(depthTexUnit_));

    renderQuad();
    program_->deactivate();
    glActiveTexture(GL_TEXTURE0);
    if (showCrossHair_) {
        if (mappingModeProp_->getValue() == 0)
            createRelDepthToColorTex(&inport_);
        else
            createAbsDepthToColorTex();

        // bind colorToDepth texture
        glActiveTexture(tm_.getGLTexUnit(shadeTexUnit_));
        depthToColorTex_->bind();
        depthToColorTex_->setFilter(tgt::Texture::NEAREST);
        depthToColorTex_->setWrapping(tgt::Texture::CLAMP);
        LGL_ERROR;

        // bind depth from inport
        inport_.bindDepthTexture(tm_.getGLTexUnit(depthTexUnit_));
        LGL_ERROR;

        crossHairPrg_->activate();
        setGlobalShaderParameters(crossHairPrg_);

        crossHairPrg_->setUniform("depthTex_", tm_.getTexUnit(depthTexUnit_));
        crossHairPrg_->setUniform("depthToColorTex_", tm_.getTexUnit(shadeTexUnit_));
        crossHairPrg_->setUniform("minValue_", minValue_);
        crossHairPrg_->setUniform("maxValue_", maxValue_);
        crossHairPrg_->setUniform("decimalPlaces_", cutAfterDecimalPlaces_.get());

        // render cross hair
        float px = (2.0f * cursorPos_.x - outport_.getSize().x) / outport_.getSize().x;
        float py = (outport_.getSize().y - 2.0f * cursorPos_.y) / outport_.getSize().y;
        glBegin(GL_LINES);
        glVertex2f(-1.0f, py); glVertex2f(1.0f, py);
        glVertex2f(px, -1.0f); glVertex2f(px, 1.0f);
        glEnd();
        crossHairPrg_->deactivate();
        glActiveTexture(GL_TEXTURE0);
    }
    LGL_ERROR;
}

} // voreen namespace
