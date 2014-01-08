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

#include "voreen/qt/widgets/transfunc/transfuncpropertywidgetpainter.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/core/datastructures/volume/histogram.h"
#include "voreen/core/utils/stringutils.h"

#include "tgt/shadermanager.h"
#include "tgt/gpucapabilities.h"
#include "tgt/glcanvas.h"

#include <sstream>

#include <QMouseEvent>
#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QToolTip>
namespace voreen {

//**********************************************************************************
// slider base functions
//**********************************************************************************
TransFuncPropertyWidgetPainterBaseSlider::TransFuncPropertyWidgetPainterBaseSlider()
    : canvasSizeInPixel_(100,40)
    , subCanvasOffsetInPixel_(0)
    , subCanvasWidthInPixel_(100)
    , valueRange_(0.f,1.f)
{
}

void TransFuncPropertyWidgetPainterBaseSlider::setCanvasSize(tgt::ivec2 size) {
    if(canvasSizeInPixel_ != size) {
        canvasSizeInPixel_ = size;
        updateSliderPosition();
    }
}

void TransFuncPropertyWidgetPainterBaseSlider::setSubCanvasOffset(int offset) {
    if(subCanvasOffsetInPixel_ != offset) {
        subCanvasOffsetInPixel_ = offset;
        updateSliderPosition();
    }
}

void TransFuncPropertyWidgetPainterBaseSlider::setSubCanvasWidth(int width) {
    if(subCanvasWidthInPixel_ != width) {
        subCanvasWidthInPixel_ = width;
        updateSliderPosition();
    }
}

void TransFuncPropertyWidgetPainterBaseSlider::setValueRange(tgt::vec2 range) {
    if(valueRange_ != range) {
        valueRange_ = range;
        updateSliderPosition();
    }
}

//**********************************************************************************
// slider gamma functions
//**********************************************************************************
TransFuncPropertyWidgetPainterGammaSlider::TransFuncPropertyWidgetPainterGammaSlider()
    : TransFuncPropertyWidgetPainterBaseSlider()
    , position_(0)
    , gamma_(1.f)
{
    valueRange_ = tgt::vec2(0.1f,5.f);
}

void TransFuncPropertyWidgetPainterGammaSlider::paint() {
    //define variables
    int sliderSize = canvasSizeInPixel_.y/6;
    float posCenter = static_cast<float>(position_)/static_cast<float>(canvasSizeInPixel_.x-1);
    float posLeft = static_cast<float>(position_-sliderSize)/static_cast<float>(canvasSizeInPixel_.x-1);
    float posRight = static_cast<float>(position_+sliderSize)/static_cast<float>(canvasSizeInPixel_.x-1);
    float posUp = 1.0f;
    float posDown = 5.f/6.f;
    //draw black arrow
    glBegin(GL_TRIANGLES);
        glColor4f(0.f,0.f,0.f,1.f);
        glVertex3f(posCenter,posDown,-0.5f);
        glVertex3f(posLeft,posUp,-0.5f);
        glVertex3f(posRight,posUp,-0.5f);
    glEnd();
    //draw white border
    glLineWidth(2.f);
    glBegin(GL_LINE_LOOP);
        glColor4f(1.f,1.f,1.f,1.f);
        glVertex3f(posCenter,posDown,-0.5f);
        glVertex3f(posLeft,posUp,-0.5f);
        glVertex3f(posRight,posUp,-0.5f);
    glEnd();
    glLineWidth(1.f);
}

bool TransFuncPropertyWidgetPainterGammaSlider::isHit(tgt::ivec2 pos) {
    //if(std::max(0,pos.y) + std::abs(pos.x - position_) <= canvasSizeInPixel_.y/6) //exact
    if((std::abs(pos.x-position_) <= (canvasSizeInPixel_.y/3)) && (std::max(0,pos.y) <= (canvasSizeInPixel_.y/3)))
        return true;
    else
        return false;
}

void TransFuncPropertyWidgetPainterGammaSlider::setPosition(int x) {
    x = tgt::clamp(x,subCanvasOffsetInPixel_,subCanvasOffsetInPixel_+subCanvasWidthInPixel_-1);
    if(position_ != x) {
        position_ = x;
        updateStoredValue();
    }
}

int TransFuncPropertyWidgetPainterGammaSlider::getPosition() {
    return position_;
}

void TransFuncPropertyWidgetPainterGammaSlider::setGammaValue(float gamma) {
    gamma = tgt::clamp(gamma,valueRange_.x,valueRange_.y);
    if(gamma_ != gamma) {
        gamma_ = gamma;
        updateSliderPosition();
    }
}

float TransFuncPropertyWidgetPainterGammaSlider::getGammaValue() {
    return gamma_;
}

void TransFuncPropertyWidgetPainterGammaSlider::updateSliderPosition() {
    int firstCanvasHalf = subCanvasWidthInPixel_/2;
    int secondCanvasHalf = subCanvasWidthInPixel_-firstCanvasHalf;

    int position = 1;

    if(gamma_ < 1.f) {
        //gamma between min and 1
        position = static_cast<int>(((gamma_-valueRange_.x)*static_cast<float>(firstCanvasHalf))/(1.f-valueRange_.x))+subCanvasOffsetInPixel_;
    } else {
       //gamma between 1 and max
        position = static_cast<int>(((gamma_-1.f)*static_cast<float>(secondCanvasHalf-1))/(valueRange_.y-1.f))+subCanvasOffsetInPixel_+firstCanvasHalf;
    }
    position_ = position;
}

void TransFuncPropertyWidgetPainterGammaSlider::updateStoredValue() {
    int firstCanvasHalf = subCanvasWidthInPixel_/2;
    int secondCanvasHalf = subCanvasWidthInPixel_-firstCanvasHalf;
    float gamma = 1.f;
    if(position_ < subCanvasOffsetInPixel_+firstCanvasHalf) {
        //gamma between min and 1
        gamma = ((1.f - valueRange_.x)*static_cast<float>(position_-subCanvasOffsetInPixel_))/static_cast<float>(firstCanvasHalf)+valueRange_.x;
    } else {
       //gamma between 1 and max
        gamma = ((valueRange_.y - 1.f)*static_cast<float>(position_-firstCanvasHalf-subCanvasOffsetInPixel_))/static_cast<float>(secondCanvasHalf-1)+1.f;
    }

    if(gamma_ != gamma) {
        gamma_ = gamma;
        emit gammaChanged(gamma);
    }
}

//***********************************************************************************
// slider domain functions
//***********************************************************************************
TransFuncPropertyWidgetPainterDomainSlider::TransFuncPropertyWidgetPainterDomainSlider()
    : TransFuncPropertyWidgetPainterBaseSlider()
    , leftPosition_(0)
    , rightPosition_(100)
    , domain_(0.f,1.f)
{
}

void TransFuncPropertyWidgetPainterDomainSlider::paint() {
    int sliderSize = canvasSizeInPixel_.y/3;
    float posDown = 0.0f;
    float posUp = 1.f/3.f;
    //left
    float posCenter = static_cast<float>(leftPosition_)/static_cast<float>(canvasSizeInPixel_.x-1);
    float posLeft = static_cast<float>(leftPosition_-sliderSize)/static_cast<float>(canvasSizeInPixel_.x-1);
    float posRight = static_cast<float>(leftPosition_+sliderSize)/static_cast<float>(canvasSizeInPixel_.x-1);

    glBegin(GL_TRIANGLES);
        glColor4f(0.f,0.f,0.f,1.f);
        glVertex3f(posCenter,posUp,-0.5f);
        glVertex3f(posCenter,posDown,-0.5f);
        glVertex3f(posRight,posDown,-0.5f);
    glEnd();
    glLineWidth(2.f);
    glBegin(GL_LINE_LOOP);
        glColor4f(1.f,1.f,1.f,1.f);
        glVertex3f(posCenter,posUp,-0.5f);
        glVertex3f(posCenter,posDown,-0.5f);
        glVertex3f(posRight,posDown,-0.5f);
    glEnd();
    glLineWidth(1.f);

    //right
    posCenter = static_cast<float>(rightPosition_)/static_cast<float>(canvasSizeInPixel_.x-1);
    posLeft = static_cast<float>(rightPosition_-sliderSize)/static_cast<float>(canvasSizeInPixel_.x-1);
    posRight = static_cast<float>(rightPosition_+sliderSize)/static_cast<float>(canvasSizeInPixel_.x-1);

    glBegin(GL_TRIANGLES);
        glColor4f(0.f,0.f,0.f,1.f);
        glVertex3f(posCenter,posUp,-0.5f);
        glVertex3f(posLeft,posDown,-0.5f);
        glVertex3f(posCenter,posDown,-0.5f);
    glEnd();
    glLineWidth(2.f);
    glBegin(GL_LINE_LOOP);
        glColor4f(1.f,1.f,1.f,1.f);
        glVertex3f(posCenter,posUp,-0.5f);
        glVertex3f(posLeft,posDown,-0.5f);
        glVertex3f(posCenter,posDown,-0.5f);
    glEnd();
    glLineWidth(1.f);
}

bool TransFuncPropertyWidgetPainterDomainSlider::isHit(tgt::ivec2 pos) {
    return isLeftHit(pos) || isRightHit(pos);
}

bool TransFuncPropertyWidgetPainterDomainSlider::isLeftHit(tgt::ivec2 pos) {
    int toleranceInPixel = 4;
    if((std::abs(pos.x-(leftPosition_+canvasSizeInPixel_.y/4)) <= (canvasSizeInPixel_.y/4)+toleranceInPixel) && (std::abs(canvasSizeInPixel_.y-pos.y) <= (canvasSizeInPixel_.y/2)))
        return true;
    else
        return false;
}

bool TransFuncPropertyWidgetPainterDomainSlider::isRightHit(tgt::ivec2 pos) {
    int toleranceInPixel = 4;
    if((std::abs(pos.x-(rightPosition_-canvasSizeInPixel_.y/4)) <= (canvasSizeInPixel_.y/4)+toleranceInPixel) && (std::abs(canvasSizeInPixel_.y-pos.y) <= (canvasSizeInPixel_.y/2)))
        return true;
    else
        return false;
}

void TransFuncPropertyWidgetPainterDomainSlider::setLeftPosition(int x) {
    x = tgt::clamp(x,subCanvasOffsetInPixel_,subCanvasOffsetInPixel_+subCanvasWidthInPixel_-1);
    if(leftPosition_ != x) {
        leftPosition_ = x;
        if(leftPosition_ > rightPosition_)
            rightPosition_ = leftPosition_;
        updateStoredValue();
    }
}

void TransFuncPropertyWidgetPainterDomainSlider::setRightPosition(int x) {
    x = tgt::clamp(x,subCanvasOffsetInPixel_,subCanvasOffsetInPixel_+subCanvasWidthInPixel_-1);
    if(rightPosition_ != x) {
        rightPosition_ = x;
        if(rightPosition_ < leftPosition_)
            leftPosition_ = rightPosition_;
        updateStoredValue();
    }
}

tgt::ivec2 TransFuncPropertyWidgetPainterDomainSlider::getPosition() {
    return tgt::ivec2(leftPosition_,rightPosition_);
}

void TransFuncPropertyWidgetPainterDomainSlider::setDomainValue(tgt::vec2 domain) {
    domain.x = tgt::clamp(domain.x,valueRange_.x,valueRange_.y);
    domain.y = tgt::clamp(domain.y,valueRange_.x,valueRange_.y);
    if(domain_ != domain) {
        domain_ = domain;
        updateSliderPosition();
    }
}

tgt::vec2 TransFuncPropertyWidgetPainterDomainSlider::getDomainValue() {
    return domain_;
}

void TransFuncPropertyWidgetPainterDomainSlider::updateSliderPosition() {
    float range = valueRange_.y-valueRange_.x;
    leftPosition_ = static_cast<int>((domain_.x-valueRange_.x)*static_cast<float>(canvasSizeInPixel_.x-1)/range)+subCanvasOffsetInPixel_;
    rightPosition_ = static_cast<int>((domain_.y-valueRange_.x)*static_cast<float>(canvasSizeInPixel_.x-1)/range)+subCanvasOffsetInPixel_;
}

void TransFuncPropertyWidgetPainterDomainSlider::updateStoredValue() {
    tgt::vec2 domain(0.f,1.f);
    float range = valueRange_.y-valueRange_.x;

    domain.x = static_cast<float>(leftPosition_-subCanvasOffsetInPixel_)*range/static_cast<float>(canvasSizeInPixel_.x-1)+valueRange_.x;
    domain.y = static_cast<float>(rightPosition_-subCanvasOffsetInPixel_)*range/static_cast<float>(canvasSizeInPixel_.x-1)+valueRange_.x;

    if(domain.x == domain.y) {
        if(domain.x - 0.0001f < valueRange_.x)
            domain.y += 0.0001f;
        else
            domain.x -= 0.0001f;
    }

    if(domain_ != domain) {
        domain_ = domain;
        emit domainChanged(domain);
    }
}

//***********************************************************************************
// widget painter functions
//***********************************************************************************
//--------------------------------
//--------------------------------
// base and tgt::Paint functions
//--------------------------------
//--------------------------------
TransFuncPropertyWidgetPainter::TransFuncPropertyWidgetPainter(tgt::GLCanvas* canvas)
    : tgt::Painter(canvas)
    //base member
    , tf_(0), histogram_(0)
    //slider
    , gammaSlider_(0)
    , domainSlider_(0)
    , pressedSlider_(NO_SLIDER)
    , mousePressedPosition_(-1) , domainLeftPressedPosition_(-1), domainRightPressedPosition_(-1)
    //helper
    , isInitialized_(false), renderHistogram_(true), renderGammaSlider_(true), renderDomainSlider_(true)
    , logarithmic_(true), minDomainValue_(0.0f), maxDomainValue_(1.0f)
{
    //create slider and connect slots
    gammaSlider_ = new TransFuncPropertyWidgetPainterGammaSlider();
    domainSlider_ = new TransFuncPropertyWidgetPainterDomainSlider();
    connect(gammaSlider_,SIGNAL(gammaChanged(float)),this,SLOT(gammaSlot(float)));
    connect(domainSlider_,SIGNAL(domainChanged(tgt::vec2)),this,SLOT(domainSlot(tgt::vec2)));
}

TransFuncPropertyWidgetPainter::~TransFuncPropertyWidgetPainter() {
    delete gammaSlider_;
    delete domainSlider_;
}

void TransFuncPropertyWidgetPainter::initialize() {
    getCanvas()->getGLFocus();

    //glViewport(0, 0, getCanvas()->getSize().x, getCanvas()->getSize().y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, 1.f, 0.f, 1.f, -2.f, 1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    isInitialized_ = true;
}

void TransFuncPropertyWidgetPainter::sizeChanged(const tgt::ivec2& size) {
    getCanvas()->getGLFocus();

    glViewport(0, 0, size.x, size.y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, 1.f, 0.f, 1.f, -2.f, 1.f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    isInitialized_ = true;

    domainSlider_->setCanvasSize(size);
    gammaSlider_->setCanvasSize(size);

    domainSlider_->setSubCanvasWidth(size.x);
    gammaSlider_->setSubCanvasWidth(domainSlider_->getPosition().y-domainSlider_->getPosition().x+1);
    gammaSlider_->setSubCanvasOffset(domainSlider_->getPosition().x);
}

//--------------------------------
//--------------------------------
//          paint
//--------------------------------
//--------------------------------
void TransFuncPropertyWidgetPainter::paint() {
    int view[4];
    glGetIntegerv(GL_VIEWPORT,view);
    if(view[2] != getCanvas()->getSize().x || view[3] != getCanvas()->getSize().y)
        sizeChanged(getCanvas()->getSize());

    if(!isInitialized_)
        initialize();

    // ensure that no shader is active while painting
    if (GpuCaps.areShadersSupported())
        tgt::Shader::deactivate();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //draw check board
    drawCheckBoard();

    //return, if no tf is present
    if(!tf_)
        return;

    glScalef(1.0f / (maxDomainValue_ - minDomainValue_), 1.0f, 1.0f);
    glTranslatef(-minDomainValue_, 0.0f, 0.0f);

    //draw transfer function
    drawTransferFunction();

    //draw histogram
    if(histogram_ && renderHistogram_) {
        drawHistogram();
    }

    //draw threshold
    drawThreshold();

    glLoadIdentity();
    //render slider
    glDisable(GL_DEPTH_TEST);
    if(renderGammaSlider_)
        gammaSlider_->paint();
    if(renderDomainSlider_)
        domainSlider_->paint();
    glColor4f(1.f,1.f,1.f,1.f);
    glEnable(GL_DEPTH_TEST);

}

void TransFuncPropertyWidgetPainter::drawCheckBoard() {
    float inc = 0.1f;
    // paint checkerboard
    for (int i = 0 ; i < 10 ; ++i) {
        glBegin(GL_QUADS);
            // Front Face
            if (i % 2)
                glColor3f(0.6f, 0.6f, 0.6f);
            else
                glColor3f(1.f, 1.f, 1.f);
            glVertex3f( i      * inc, 0.0f,  -0.5f);  // Bottom Left
            glVertex3f((i + 1) * inc, 0.0f,  -0.5f);  // Bottom Right
            glVertex3f((i + 1) * inc, 0.5f,  -0.5f);  // Top Right
            glVertex3f( i      * inc, 0.5f,  -0.5f);  // Top Left
        glEnd();
        glBegin(GL_QUADS);
            // Front Face
            if (i % 2)
                glColor3f(1.f, 1.f, 1.f);
            else
                glColor3f(0.6f, 0.6f, 0.6f);
            glVertex3f( i      * inc, 0.5f,  -0.5f);  // Bottom Left
            glVertex3f((i + 1) * inc, 0.5f,  -0.5f);  // Bottom Right
            glVertex3f((i + 1) * inc, 1.0f,  -0.5f);  // Top Right
            glVertex3f( i      * inc, 1.0f,  -0.5f);  // Top Left
        glEnd();
    }
}

void TransFuncPropertyWidgetPainter::drawTransferFunction() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
    tf_->bind();

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    tgt::vec2 dom = tf_->getDomain();

    //draw tf with alpha
    glBegin(GL_QUADS);
        // Quad left of domain:
        glTexCoord1f(0.f);
        glVertex3f(minDomainValue_, 0.f, -0.5f);
        glVertex3f(dom.x, 0.f, -0.5f);
        glVertex3f(dom.x, 1.f, -0.5f);
        glVertex3f(minDomainValue_, 1.f, -0.5f);
        // Inside domain:
        glTexCoord1f(0.f);
        glVertex3f(dom.x, 0.f, -0.5f);
        glTexCoord1f(1.f);
        glVertex3f(dom.y, 0.f, -0.5f);
        glVertex3f(dom.y, 1.f, -0.5f);
        glTexCoord1f(0.f);
        glVertex3f(dom.x, 1.f, -0.5f);
        // Quad right of domain:
        glTexCoord1f(1.f);
        glVertex3f(dom.y, 0.f, -0.5f);
        glVertex3f(maxDomainValue_, 0.f, -0.5f);
        glVertex3f(maxDomainValue_, 1.f, -0.5f);
        glVertex3f(dom.y, 1.f, -0.5f);
    glEnd();

    //draw tf without alpha
    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
    float height = 22.f/30.f;
    glBegin(GL_QUADS);
        glColor4f(1.f, 1.f, 1.f, 1.f);
        // Quad left of domain:
        glTexCoord1f(0.f);
        glVertex3f(minDomainValue_, height, -0.5f);
        glVertex3f(dom.x, height, -0.5f);
        glVertex3f(dom.x, 1.f, -0.5f);
        glVertex3f(minDomainValue_, 1.f, -0.5f);
        // Inside domain:
        glTexCoord1f(0.f);
        glVertex3f(dom.x, height, -0.5f);
        glTexCoord1f(1.f);
        glVertex3f(dom.y, height, -0.5f);
        glVertex3f(dom.y, 1.f, -0.5f);
        glTexCoord1f(0.f);
        glVertex3f(dom.x, 1.f, -0.5f);
        // Quad right of domain:
        glTexCoord1f(1.f);
        glVertex3f(dom.y, height, -0.5f);
        glVertex3f(maxDomainValue_, height, -0.5f);
        glVertex3f(maxDomainValue_, 1.f, -0.5f);
        glVertex3f(dom.y, 1.f, -0.5f);
    glEnd();

    glDisable(GL_TEXTURE_1D);

    glBegin(GL_LINES);
        glColor3f(0.2f,0.2f,0.2f);
        glVertex3f(minDomainValue_,height-(1.f/30.f)+0.02,-0.5f);
        glVertex3f(maxDomainValue_,height-(1.f/30.f)+0.02,-0.5f);
    glEnd();
    glEnable(GL_DEPTH_TEST);
}

void TransFuncPropertyWidgetPainter::drawHistogram() {
    // paint histogram
        glDisable(GL_DEPTH_TEST);
        int nBuckets = (int)histogram_->getNumBuckets();

        glColor3f(1.f, 0.53f, 0.53f);
        float bucketSize = (histogram_->getMaxValue() - histogram_->getMinValue()) / nBuckets;
        float offset = histogram_->getMinValue();
        float max = histogram_->getMaxBucket();
        if(max > 0) {
            if(logarithmic_)
                max = logf(max);
            glBegin(GL_QUADS);
            for (int i = 0 ; i < nBuckets ; ++i) {
                float y = 0.0f;
                int bucket = histogram_->getBucket(i);
                if(bucket > 0) {
                    if(logarithmic_) {
                        y = logf(static_cast<float>(bucket)) / max;
                    }
                    else {
                        y = bucket / max;
                    }
                }

                //only 2/3 should be histogram
                y *= 2.f/3.f;

                glVertex3f(offset + (i * bucketSize), 0.0f,  -0.5f);  // Bottom Left
                glVertex3f(offset + ((i+1) * bucketSize), 0.0f,  -0.5f);  // Bottom Right
                glVertex3f(offset + ((i+1) * bucketSize), y,  -0.5f);  // Top Right
                glVertex3f(offset + (i * bucketSize), y,  -0.5f);  // Top Left
            }
            glEnd();
            glEnable(GL_DEPTH_TEST);
        }
}

void TransFuncPropertyWidgetPainter::drawThreshold() {
    //paint threshold quads
    tgt::vec2 thresh = tf_->getThresholds();
    tgt::vec2 dom = tf_->getDomain();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glColor4f(0.75f, 0.75f, 0.75f, 0.9f);

    if(thresh.x > 0.f) {
    // Quad left of threshold:
        glVertex3f(minDomainValue_, 0.f, -0.5f);
        glVertex3f(thresh.x*(dom.y-dom.x)+dom.x, 0.f, -0.5f);
        glVertex3f(thresh.x*(dom.y-dom.x)+dom.x, 1.f, -0.5f);
        glVertex3f(minDomainValue_, 1.f, -0.5f);
    }

    if(thresh.y < 1.f) {
    // Quad right of threshold:
    glVertex3f(thresh.y*(dom.y-dom.x)+dom.x, 0.f, -0.5f);
    glVertex3f(maxDomainValue_, 0.f, -0.5f);
    glVertex3f(maxDomainValue_, 1.f, -0.5f);
    glVertex3f(thresh.y*(dom.y-dom.x)+dom.x, 1.f, -0.5f);
    }
    glColor4f(1.f,1.f,1.f,1.f);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
}

//--------------------------------
//--------------------------------
//      getter and setter
//--------------------------------
//--------------------------------
void TransFuncPropertyWidgetPainter::setTransFunc(TransFunc* tf,float minDomainValue, float maxDomainValue) {
    if (tf_ != tf) {
        tf_ = tf;
        //get min and max domain value
        if(minDomainValue <= maxDomainValue) {
            minDomainValue_ = std::min(minDomainValue, tf_->getDomain().x);
            maxDomainValue_ = std::max(maxDomainValue, tf_->getDomain().y);
            domainSlider_->setValueRange(tgt::vec2(minDomainValue_,maxDomainValue_));
            gammaSlider_->setSubCanvasWidth(domainSlider_->getPosition().y-domainSlider_->getPosition().x+1);
            gammaSlider_->setSubCanvasOffset(domainSlider_->getPosition().x);
        } else { // if not existing
            resetZoom();
        }
    }

    if(tf) {
        if(tf->getDomain().x < minDomainValue_)
            minDomainValue_ = tf->getDomain().x;

        if(tf->getDomain().y > maxDomainValue_)
            maxDomainValue_ = tf->getDomain().y;
        //update Slider
        domainSlider_->setValueRange(tgt::vec2(minDomainValue_,maxDomainValue_));
        domainSlider_->setDomainValue(tf->getDomain());

        gammaSlider_->setSubCanvasWidth(domainSlider_->getPosition().y-domainSlider_->getPosition().x+1);
        gammaSlider_->setSubCanvasOffset(domainSlider_->getPosition().x);
        gammaSlider_->setGammaValue(tf->getGammaValue());
    }

}

void TransFuncPropertyWidgetPainter::setHistogram(const Histogram1D* histogram) {
    histogram_ = histogram;
}

//--------------------------------------------
//--------------------------------------------
//          Mouse Events
//--------------------------------------------
//--------------------------------------------
void TransFuncPropertyWidgetPainter::mousePressEvent(tgt::MouseEvent* e) {
    //TODO: does not worg
    //emit showToolTip(e-> coord(),QString("test"));
    if(renderGammaSlider_ && gammaSlider_->isHit(e->coord())) {
        pressedSlider_ = GAMMA_SLIDER;
    } else if(renderDomainSlider_ && domainSlider_->isHit(e->coord())) {
        if(domainSlider_->isLeftHit(e->coord()))
            pressedSlider_ = DOMAIN_LEFT_SLIDER;
        else
            pressedSlider_ = DOMAIN_RIGHT_SLIDER;
    } else if(renderDomainSlider_ && (e->coord().x <= domainSlider_->getPosition().y) && (e->coord().x >= domainSlider_->getPosition().x)) {
        pressedSlider_ = DOMAIN_BOTH_SLIDER;
        mousePressedPosition_ = e->coord().x;
        domainLeftPressedPosition_ = domainSlider_->getPosition().x;
        domainRightPressedPosition_ = domainSlider_->getPosition().y;
    } else
        pressedSlider_ = NO_SLIDER;

    if(pressedSlider_ != NO_SLIDER)
        emit interaction(true);
}

void TransFuncPropertyWidgetPainter::mouseMoveEvent(tgt::MouseEvent* e) {
    switch(pressedSlider_) {
    case GAMMA_SLIDER:
        gammaSlider_->setPosition(e->coord().x);
        getCanvas()->update();
    break;
    case DOMAIN_LEFT_SLIDER:
        domainSlider_->setLeftPosition(e->coord().x);
        getCanvas()->update();
    break;
    case DOMAIN_RIGHT_SLIDER:
        domainSlider_->setRightPosition(e->coord().x);
        getCanvas()->update();
    break;
    case DOMAIN_BOTH_SLIDER:
        tgtAssert(mousePressedPosition_ != -1, "mousePressedPosition has not been set!");
        tgtAssert(domainLeftPressedPosition_ != -1, "domainLeftPressedPosition has not been set!");
        tgtAssert(domainRightPressedPosition_ != -1, "domainRightPressedPosition has not been set!");
        domainSlider_->setLeftPosition(domainLeftPressedPosition_ - (mousePressedPosition_ -e->coord().x));
        domainSlider_->setRightPosition(domainRightPressedPosition_ - (mousePressedPosition_ -e->coord().x));
        getCanvas()->update();
    break;
    }
    //creates and shows tooltip
    createInfoToolTip(QPoint(e->coord().x,e->coord().y));
}

void TransFuncPropertyWidgetPainter::mouseReleaseEvent(tgt::MouseEvent* e) {
    pressedSlider_ = NO_SLIDER;
    mousePressedPosition_ = -1;
    domainLeftPressedPosition_ = -1;
    domainRightPressedPosition_ = -1;
    emit hideInfoToolTip();
    emit interaction(false);
}

void TransFuncPropertyWidgetPainter::mouseDoubleClickEvent(tgt::MouseEvent* e) {
     if(renderGammaSlider_ && gammaSlider_->isHit(e->coord())) {
         gammaSlider_->setGammaValue(1.f);
         gammaSlot(1.f);
         getCanvas()->update();
     }
}

void TransFuncPropertyWidgetPainter::createInfoToolTip(QPoint mousePos) {
    //info tool tip output
    QPoint ittPos;    //x: center of tip, y: lower border
    std::stringstream ittText;
    QString qText;
    //helper to determine tool tip position
    int ittLeftBorder = 6; // empty space on left side (estimated)
    int ittWidth = 0;
    QFont ttFont = QToolTip::font();
    QFont bttFont = ttFont;
    bttFont.setBold(true);
    QFontMetrics ttMetr(ttFont);
    QFontMetrics bttMetr(bttFont);
    //percision
    float interval = domainSlider_->getDomainValue().y - domainSlider_->getDomainValue().x;
    int percision = 3;
    if(interval > 1000.f)
        percision = 0;
    else if(interval > 100.f)
        percision = 1;
    else if(interval > 10.f)
        percision = 2;

    switch(pressedSlider_){
    case GAMMA_SLIDER:
        ittPos.setX(mousePos.x());
        ittPos.setY(-6);
        //gamma
        qText = QString(1,QChar(0x03b3)) + QString("=");
        ittWidth += bttMetr.width(qText);
        //value
        ittText << ftos(gammaSlider_->getGammaValue(),2) << "</b>";
        ittWidth += bttMetr.width(ftos(gammaSlider_->getGammaValue(),2).c_str());
        qText = QString("<b>") + qText + QString(ittText.str().c_str());
        break;
    case DOMAIN_LEFT_SLIDER:
        ittPos.setX(getCanvas()->getWidth() / 2);
        ittPos.setY(getCanvas()->getHeight()+20);
        ittText << "<nobr>Domain: [ <b>" << ftos(domainSlider_->getDomainValue().x,percision) << "</b> , " <<
                                    ftos(domainSlider_->getDomainValue().y,percision) << " ]</nobr>";
        ittWidth += ttMetr.width("Domain: [  ,  ]");
        ittWidth += bttMetr.width(ftos(domainSlider_->getDomainValue().x,percision).c_str());
        ittWidth += ttMetr.width(ftos(domainSlider_->getDomainValue().y,percision).c_str());
        qText = QString(ittText.str().c_str());
        break;
    case DOMAIN_RIGHT_SLIDER:
        ittPos.setX(getCanvas()->getWidth() / 2);
        ittPos.setY(getCanvas()->getHeight()+20);
        ittText << "<nobr>Domain: [ " + ftos(domainSlider_->getDomainValue().x,percision) << " , <b>" <<
                                ftos(domainSlider_->getDomainValue().y,percision) << "</b> ]</nobr>";
        ittWidth += ttMetr.width("Domain: [  ,  ]");
        ittWidth += ttMetr.width(ftos(domainSlider_->getDomainValue().x,percision).c_str());
        ittWidth += bttMetr.width(ftos(domainSlider_->getDomainValue().y,percision).c_str());
        qText = QString(ittText.str().c_str());
        break;
    case DOMAIN_BOTH_SLIDER:
        ittPos.setX(getCanvas()->getWidth() / 2);
        ittPos.setY(getCanvas()->getHeight()+20);
        ittText << "<nobr>Domain: [ <b>" << ftos(domainSlider_->getDomainValue().x,percision) << "</b> , <b>" +
                                    ftos(domainSlider_->getDomainValue().y,percision) << "</b> ]</nobr>";
        ittWidth += ttMetr.width("Domain: [  ,  ]");
        ittWidth += bttMetr.width(ftos(domainSlider_->getDomainValue().x,percision).c_str());
        ittWidth += bttMetr.width(ftos(domainSlider_->getDomainValue().y,percision).c_str());
        qText = QString(ittText.str().c_str());
        break;
    default:
        return;
        break;
    }
    //shift info tool tip
    ittPos.setX(ittPos.x()-ittWidth/2-ittLeftBorder);
    //show tip
    emit showInfoToolTip(ittPos,qText);
}

void TransFuncPropertyWidgetPainter::gammaSlot(float gamma) {
    tf_->setGammaValue(gamma);
    emit changedGamma();
}
void TransFuncPropertyWidgetPainter::domainSlot(tgt::vec2 domain) {
    tf_->setDomain(domain);
    //move gamma slider
    gammaSlider_->setSubCanvasWidth(domainSlider_->getPosition().y-domainSlider_->getPosition().x+1);
    gammaSlider_->setSubCanvasOffset(domainSlider_->getPosition().x);
    emit changedDomain();
}

//--------------------------------------------
//--------------------------------------------
//  zoom functions
//--------------------------------------------
//--------------------------------------------
void TransFuncPropertyWidgetPainter::zoomIn() {
    if(tf_) {
        float domCenter = (tf_->getDomain().x + tf_->getDomain().y) * 0.5f;
        float domSize = (tf_->getDomain().y - tf_->getDomain().x);
        float viewSize = maxDomainValue_ - minDomainValue_;
        viewSize *= 0.5f;
        if(viewSize < domSize)
            viewSize = domSize;

        minDomainValue_ = domCenter - (viewSize * 0.5f);
        maxDomainValue_ = domCenter + (viewSize * 0.5f);

        domainSlider_->setValueRange(tgt::vec2(minDomainValue_,maxDomainValue_));
        gammaSlider_->setSubCanvasWidth(domainSlider_->getPosition().y-domainSlider_->getPosition().x+1);
        gammaSlider_->setSubCanvasOffset(domainSlider_->getPosition().x);
    }
}

void TransFuncPropertyWidgetPainter::zoomOut() {
    if(tf_) {
        float domCenter = (tf_->getDomain().x + tf_->getDomain().y) * 0.5f;
        float domSize = (tf_->getDomain().y - tf_->getDomain().x);
        float viewSize = maxDomainValue_ - minDomainValue_;
        float viewCenter = domCenter;

        viewSize *= 2.0f;

        minDomainValue_ = viewCenter - (viewSize * 0.5f);
        maxDomainValue_ = viewCenter + (viewSize * 0.5f);

        domainSlider_->setValueRange(tgt::vec2(minDomainValue_,maxDomainValue_));
        gammaSlider_->setSubCanvasWidth(domainSlider_->getPosition().y-domainSlider_->getPosition().x+1);
        gammaSlider_->setSubCanvasOffset(domainSlider_->getPosition().x);
    }
}

void TransFuncPropertyWidgetPainter::resetZoom() {
    if(tf_) {
        minDomainValue_ = tf_->getDomain().x;
        maxDomainValue_ = tf_->getDomain().y;

        domainSlider_->setValueRange(tgt::vec2(minDomainValue_,maxDomainValue_));
        gammaSlider_->setSubCanvasWidth(domainSlider_->getPosition().y-domainSlider_->getPosition().x+1);
        gammaSlider_->setSubCanvasOffset(domainSlider_->getPosition().x);
    }
}

float TransFuncPropertyWidgetPainter::getMinDomainValue() const {
    return minDomainValue_;
}

float TransFuncPropertyWidgetPainter::getMaxDomainValue() const {
    return maxDomainValue_;
}

} // namespace voreen
