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

#include "voreen/modules/base/processors/utility/textoverlay.h"

#include "voreen/core/voreenapplication.h"

#include "tgt/textureunit.h"

#include <sstream>

using tgt::TextureUnit;

namespace {
#ifdef __APPLE__
    const int fontSize = 11;
#else
    const int fontSize = 12;
#endif
}

namespace voreen {

const std::string TextOverlay::loggerCat_("voreen.TextOverlay");

TextOverlay::TextOverlay()
    : ImageProcessor("textoverlay")
    , inport_(Port::INPORT, "image.input")
    , outport_(Port::OUTPORT, "image.output")
    , privatePort_(Port::OUTPORT, "image.tmp")
    , text0_(Port::INPORT, "text.text0", true)
    , text1_(Port::INPORT, "text.text1", true)
    , text2_(Port::INPORT, "text.text2", true)
    , text3_(Port::INPORT, "text.text3", true)
    , layout0_("layout0", "Text position 1:")
    , layout1_("layout1", "Text position 2:")
    , layout2_("layout2", "Text position 3:")
    , layout3_("layout3", "Text position 4:")
    , font_(0)
{
    addPort(inport_);
    addPort(outport_);
    addPrivateRenderPort(&privatePort_);
    addPort(text0_);
    addPort(text1_);
    addPort(text2_);
    addPort(text3_);

    std::vector<Option<std::string> > options;
    options.push_back(Option<std::string>("N", "North", ""));
    options.push_back(Option<std::string>("NE", "North-East", ""));
    options.push_back(Option<std::string>("E", "East", ""));
    options.push_back(Option<std::string>("SE", "South-East", ""));
    options.push_back(Option<std::string>("S", "South", ""));
    options.push_back(Option<std::string>("SW", "South-West", ""));
    options.push_back(Option<std::string>("W", "West", ""));
    options.push_back(Option<std::string>("NW", "North-West", ""));
    options.push_back(Option<std::string>("CENTER", "Center", ""));

    layout0_.setOptions(options);
    layout0_.select("NW");
    layout1_.setOptions(options);
    layout1_.select("NW");
    layout2_.setOptions(options);
    layout2_.select("NW");
    layout3_.setOptions(options);
    layout3_.select("NW");

    addProperty(layout0_);
    addProperty(layout1_);
    addProperty(layout2_);
    addProperty(layout3_);
}

TextOverlay::~TextOverlay() {

}

std::string TextOverlay::getProcessorInfo() const {
    return "Renders a text overlay on top of the input image.";
}

Processor* TextOverlay::create() const {
    return new TextOverlay();
}

void TextOverlay::initialize() throw (VoreenException) {
    ImageProcessor::initialize();

    std::string fontFile = VoreenApplication::app()->getFontPath("VeraMono.ttf");
    font_ = new tgt::Font(fontFile, fontSize, tgt::BitmapFont);
    if (!font_) {
        LERRORC("voreen.TextOverlay", "Failed to load font " << fontFile);
        initialized_ = false;
        throw VoreenException("Failed to load font " + fontFile);
    }
}

void TextOverlay::deinitialize() throw (VoreenException) {
    delete font_;
    font_ = 0;

    ImageProcessor::deinitialize();
}

bool TextOverlay::isReady() const {

    if (!isInitialized())
        return false;

    if (!inport_.isReady())
        return false;

    if (!outport_.isReady())
        return false;

    return true;
}

void TextOverlay::process() {

    if (!inport_.isReady())
        outport_.activateTarget();
    else
        privatePort_.activateTarget();

    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearDepth(1.0);

    glDisable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifndef VRN_WITH_FONTRENDERING
    LWARNING("Empty output, recompile with VRN_WITH_FONTRENDERING.");
#endif

    renderOverlay();

    if (!inport_.isReady())
        outport_.deactivateTarget();
    else
        privatePort_.deactivateTarget();

    glEnable(GL_DEPTH_TEST);

    // skip if there's nothing to render above
    if (inport_.isReady()) {
        outport_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        TextureUnit shadeUnit0, shadeUnit1, depthUnit0, depthUnit1;
        privatePort_.bindTextures(shadeUnit0.getEnum(), depthUnit0.getEnum());
        inport_.bindTextures(shadeUnit1.getEnum(), depthUnit1.getEnum());

        // initialize shader
        program_->activate();
        setGlobalShaderParameters(program_);
        program_->setUniform("shadeTex0_", shadeUnit0.getUnitNumber());
        program_->setUniform("shadeTex1_", shadeUnit1.getUnitNumber());
        program_->setUniform("depthTex1_", depthUnit1.getUnitNumber());
        privatePort_.setTextureParameters(program_, "textureParameters0_");
        inport_.setTextureParameters(program_, "textureParameters1_");

        ImageProcessor::renderQuad();

        outport_.deactivateTarget();

        TextureUnit::setZeroUnit();
        program_->deactivate();
    }
    LGL_ERROR;
}

void TextOverlay::renderOverlay() {
    tgtAssert(font_, "No font object");

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // render font(s)
    glPushMatrix();
    glTranslatef(-1,-1,0);
    float scaleFactorX = 2.0f / (float)inport_.getSize().x;
    float scaleFactorY = 2.0f / (float)inport_.getSize().y;
    glScalef(scaleFactorX, scaleFactorY, 1);

    font_->setLineWidth((float)inport_.getSize().x - 5);

    font_->setTextAlignment(tgt::Left);
    font_->renderWithLayout(tgt::vec3(5, (float)inport_.getSize().y - fontSize - 3, 0), collectText("NW"));
    font_->renderWithLayout(tgt::vec3(5, ((float)inport_.getSize().y * 0.5f)
                                      + (getNumberOfLines(collectText("W")) - 1) * 0.5f * font_->getLineHeight(), 0), collectText("W"));
    font_->renderWithLayout(tgt::vec3(5, (getNumberOfLines(collectText("SW")) - 1) * font_->getLineHeight(), 0), collectText("SW"));

    font_->setTextAlignment(tgt::Right);
    font_->renderWithLayout(tgt::vec3(0, (float)inport_.getSize().y - fontSize - 3, 0), collectText("NE"));
    font_->renderWithLayout(tgt::vec3(5, ((float)inport_.getSize().y * 0.5f)
                                      + (getNumberOfLines(collectText("E")) - 1) * 0.5f * font_->getLineHeight(), 0), collectText("E"));
    font_->renderWithLayout(tgt::vec3(5, (getNumberOfLines(collectText("SE")) - 1) * font_->getLineHeight(), 0), collectText("SE"));

    font_->setLineWidth((float)inport_.getSize().x - 10);

    font_->setTextAlignment(tgt::Center);
    font_->renderWithLayout(tgt::vec3(5, (float)inport_.getSize().y - fontSize - 3, 0), collectText("N"));
    font_->renderWithLayout(tgt::vec3(5, ((float)inport_.getSize().y * 0.5f)
                                      + (getNumberOfLines(collectText("CENTER")) - 1) * 0.5f * font_->getLineHeight(), 0),
                            collectText("CENTER"));
    font_->renderWithLayout(tgt::vec3(5, (getNumberOfLines(collectText("S")) - 1) * font_->getLineHeight(), 0), collectText("S"));

    glPopMatrix();
}

std::string TextOverlay::collectText(std::string key) {
    std::stringstream ss("");
    std::vector<std::string> data;
    data = text0_.getAllData();
    if (text0_.isReady() && strcmp(layout0_.getKey().c_str(), key.c_str()) == 0) {
        for (unsigned int i=0; i<data.size(); i++) {
            if (ss.str().length() > 0)
                ss << "\n";
            ss << data[i].c_str();
        }
    }
    data = text1_.getAllData();
    if (text1_.isReady() && strcmp(layout1_.getKey().c_str(), key.c_str()) == 0) {
        for (unsigned int i=0; i<data.size(); i++) {
            if (ss.str().length() > 0)
                ss << "\n";
            ss << data[i].c_str();
        }
    }
    data = text2_.getAllData();
    if (text2_.isReady() && strcmp(layout2_.getKey().c_str(), key.c_str()) == 0) {
        for (unsigned int i=0; i<data.size(); i++) {
            if (ss.str().length() > 0)
                ss << "\n";
            ss << data[i].c_str();
        }
    }
    data = text3_.getAllData();
    if (text3_.isReady() && strcmp(layout3_.getKey().c_str(), key.c_str()) == 0) {
        for (unsigned int i=0; i<data.size(); i++) {
            if (ss.str().length() > 0)
                ss << "\n";
            ss << data[i].c_str();
        }
    }
    return ss.str();
}

int TextOverlay::getNumberOfLines(std::string s) {
    int numLines = 0;
    std::string line;
    std::stringstream ss(s);
    while (std::getline(ss, line))
        numLines++;

    return numLines;
}

} // namespace
