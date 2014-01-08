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

#include "tabbedview.h"

namespace voreen {

using tgt::ivec2;

TabbedView::TabbedView()
    : RenderProcessor()
    , borderColor_("borderColor", "Border Color", tgt::vec4(0.0f, 0.0f, 0.0f, 1.0f))
    , borderHoverColor_("borderHoverColor", "Border Hover Color", tgt::vec4(0.0f, 0.0f, 0.0f, 1.0f))
    , buttonColor_("buttonColor", "Button Color", tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f))
    , buttonHoverColor_("buttonHoverColor", "Button Hover Color", tgt::vec4(0.7f, 0.7f, 0.7f, 1.0f))
    , textColor_("textColor", "Text Color", tgt::vec4(0.0f, 0.0f, 0.0f, 1.0f))
    , textHoverColor_("textHoverColor", "Text Hover Color", tgt::vec4(0.0f, 0.0f, 0.0f, 1.0f))
    , currentView_("currentView", "Current View", 1, 1, 4)
    , fontProp_("voreen.fontprop", "Font:")
    , renderAtBottom_("renderAtBottom", "Render Tab-bar at Bottom", false)
    , hideTabbar_("hideTabbar_", "Hide Tab-bar", false)
    , tabText1_("tabText1", "Tab 1 Label:", "Tab 1")
    , tabText2_("tabText2", "Tab 2 Label:", "Tab 2")
    , tabText3_("tabText3", "Tab 3 Label:", "Tab 3")
    , tabText4_("tabText4", "Tab 4 Label:", "Tab 4")
    , outport_(Port::OUTPORT, "outport", "Image Output", true, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_RECEIVER)
    , inport1_(Port::INPORT, "inport1", "Image1 Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , inport2_(Port::INPORT, "inport2", "Image2 Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , inport3_(Port::INPORT, "inport3", "Image3 Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , inport4_(Port::INPORT, "inport4", "Image4 Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , insideViewPort_(false)
    , mouseOverButton_(-1)
    , isDragging_(false)
{
    borderColor_.setViews(Property::COLOR);
    addProperty(borderColor_);
    borderHoverColor_.setViews(Property::COLOR);
    addProperty(borderHoverColor_);

    buttonColor_.setViews(Property::COLOR);
    addProperty(buttonColor_);
    buttonHoverColor_.setViews(Property::COLOR);
    addProperty(buttonHoverColor_);

    textColor_.setViews(Property::COLOR);
    addProperty(textColor_);
    textHoverColor_.setViews(Property::COLOR);
    addProperty(textHoverColor_);

    addProperty(currentView_);

    fontProp_.onChange(CallMemberAction<TabbedView>(this, &TabbedView::updateSizes));
    addProperty(fontProp_);

    addProperty(tabText1_);
    addProperty(tabText2_);
    addProperty(tabText3_);
    addProperty(tabText4_);

    addProperty(renderAtBottom_);
    hideTabbar_.onChange(CallMemberAction<TabbedView>(this, &TabbedView::updateSizes));
    addProperty(hideTabbar_);

    addPort(outport_);
    addPort(inport1_);
    addPort(inport2_);
    addPort(inport3_);
    addPort(inport4_);

    outport_.onSizeReceiveChange<TabbedView>(this, &TabbedView::updateSizes);
}

TabbedView::~TabbedView() {
}

Processor* TabbedView::create() const {
    return new TabbedView();
}

bool TabbedView::isReady() const {
    if (!outport_.isReady())
        return false;

    //switch(currentView_.get()) {
        //case 1: if(!inport1_.isReady())
                    //return false;
                //break;
        //case 2: if(!inport2_.isReady())
                    //return false;
                //break;
        //case 3: if(!inport3_.isReady())
                    //return false;
                //break;
        //case 4: if(!inport4_.isReady())
                    //return false;
                //break;
    //}
    return true;
}

void TabbedView::process() {
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float scale = 1.0f;
    float offset = 0.0f;
    if(!hideTabbar_.get()) {
        scale = 1.0f - ((float) getBorderWidth() / (float)outport_.getSize().y);
        offset = (float) getBorderWidth() / (float) outport_.getSize().y;
    }

    if(renderAtBottom_.get())
        MatStack.translate(0.0f, +offset, 0.0f);
    else
        MatStack.translate(0.0f, -offset, 0.0f);

    MatStack.scale(1.0f, scale, 1.0f);

    RenderPort* currentPort = 0;
    switch(currentView_.get()) {
        case 1: currentPort = &inport1_;
                break;
        case 2: currentPort = &inport2_;
                break;
        case 3: currentPort = &inport3_;
                break;
        case 4: currentPort = &inport4_;
                break;
    }

    if(currentPort && currentPort->isReady()) {
        currentPort->bindColorTexture(GL_TEXTURE0);
        currentPort->getColorTexture()->enable();
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glDepthFunc(GL_ALWAYS);
        renderQuad();
        glDepthFunc(GL_LESS);

        currentPort->getColorTexture()->disable();
    }

    if(!hideTabbar_.get()) {
        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
        MatStack.loadIdentity();
        // render buttons with fonts in screenspace
        MatStack.translate(-1.0f, -1.0f, 0.0f);
        float scaleFactorX = 2.0f / (float)outport_.getSize().x;
        float scaleFactorY = 2.0f / (float)outport_.getSize().y;
        MatStack.scale(scaleFactorX, scaleFactorY, 1.0f);

        //float yStart = 0.0f;
        float yStart = (float)outport_.getSize().y;
        float yEnd = yStart - getBorderWidth();
        if(renderAtBottom_.get()) {
            yStart = (float)getBorderWidth();
            yEnd = 0;
        }
        glLineWidth(2.0f);
        int buttonWidth = tgt::iround(outport_.getSize().x / 4.0f);

        fontProp_.get()->setLineWidth(buttonWidth - 10.f); //TODO: use linewidth
        //fontProp_.get()->setTextAlignment(tgt::Font::Right);
        fontProp_.get()->setVerticalTextAlignment(tgt::Font::Middle);

        for(int i=0; i<4; i++) {
            if(mouseOverButton_ == i)
                glColor4f(buttonHoverColor_.get().r, buttonHoverColor_.get().g, buttonHoverColor_.get().b, buttonHoverColor_.get().a);
            else
                glColor4f(buttonColor_.get().r, buttonColor_.get().g, buttonColor_.get().b, buttonColor_.get().a);

            float xStart = 0.f + (i*buttonWidth);
            float xEnd = xStart + buttonWidth;;

            glBegin(GL_QUADS);
            glVertex3f(xStart, yStart, 0.1f);
            glVertex3f(xEnd, yStart, 0.1f);

            glVertex3f(xEnd, yEnd, 0.1f);
            glVertex3f(xStart, yEnd, 0.1f);
            glEnd();

            glDepthFunc(GL_ALWAYS);
            if(mouseOverButton_ == i)
                glColor4f(borderHoverColor_.get().r, borderHoverColor_.get().g, borderHoverColor_.get().b, borderHoverColor_.get().a);
            else
                glColor4f(borderColor_.get().r, borderColor_.get().g, borderColor_.get().b, borderColor_.get().a);

            glBegin(GL_LINE_LOOP);
            glVertex2f(xStart, yStart);
            glVertex2f(xEnd, yStart);

            glVertex2f(xEnd, yEnd);
            glVertex2f(xStart, yEnd);
            glEnd();
            glDepthFunc(GL_LESS);

            std::string label;
            switch(i) {
                case 0: label = tabText1_.get();
                        break;
                case 1: label = tabText2_.get();
                        break;
                case 2: label = tabText3_.get();
                        break;
                case 3: label = tabText4_.get();
                        break;
            }

            if(mouseOverButton_ == i)
                glColor4f(textHoverColor_.get().r, textHoverColor_.get().g, textHoverColor_.get().b, textHoverColor_.get().a);
            else
                glColor4f(textColor_.get().r, textColor_.get().g, textColor_.get().b, textColor_.get().a);

            float yMid = (yStart + yEnd) / 2.0f;
            fontProp_.get()->render(tgt::vec3(xStart + 5, yMid, 0), label);
        }

        glLineWidth(1.0f);
    }
    MatStack.loadIdentity();

    outport_.deactivateTarget();
    LGL_ERROR;
}

void TabbedView::initialize() throw (tgt::Exception) {
    RenderProcessor::initialize();
    updateSizes();
}

int TabbedView::getBorderWidth() {
    if(hideTabbar_.get())
        return 0;

    if (fontProp_.get()) {
        return tgt::iround(fontProp_.get()->getLineHeight() + 5);
    }
    else
        return 20;
}

ivec2 TabbedView::getInternalSize() {
    if (outport_.getReceivedSize() == tgt::ivec2(0))
        return ivec2(0, 0);
    else {
        if(!hideTabbar_.get())
            return outport_.getReceivedSize() - ivec2(0, getBorderWidth());
        else
            return outport_.getReceivedSize();
    }
}

void TabbedView::updateSizes() {
    if (outport_.getReceivedSize() == tgt::ivec2(0))
        return;

    tgt::ivec2 subsize = getInternalSize();
    inport1_.requestSize(subsize);
    inport2_.requestSize(subsize);
    inport3_.requestSize(subsize);
    inport4_.requestSize(subsize);
}

void TabbedView::handleMouseEvent(tgt::MouseEvent* e) {
    e->accept();
    bool wasInsideViewPort = insideViewPort_;

    if ((e->action() & tgt::MouseEvent::EXIT) == tgt::MouseEvent::EXIT) {
        insideViewPort_ = false;

        if(mouseOverButton_ != -1)
            invalidate();
        mouseOverButton_ = -1;
    }

    if ((e->action() & tgt::MouseEvent::PRESSED) == tgt::MouseEvent::PRESSED)
        isDragging_ = true;
    if ((e->action() & tgt::MouseEvent::RELEASED) == tgt::MouseEvent::RELEASED)
        isDragging_ = false;

    if (!isDragging_) {
        bool insideY;
        if(!hideTabbar_.get()) {
            if(renderAtBottom_.get()) {
                insideY = e->y() < (outport_.getSize().y - getBorderWidth());
                insideY &= e->y() > 0;
            }
            else {
                insideY = e->y() < outport_.getSize().y;
                insideY &= e->y() > getBorderWidth();
            }

            if(insideY && (e->x() >= 0) && (e->x() < outport_.getSize().x)) {
                insideViewPort_ = true;
            }
            else {
                insideViewPort_ = false;
            }
        }
        else
            insideViewPort_ = true;
    }

    if (wasInsideViewPort != insideViewPort_) {
        if(wasInsideViewPort) {
            tgt::MouseEvent leaveEvent(1, 1, tgt::MouseEvent::EXIT, e->modifiers(), e->button(), e->viewport() - ivec2(0, getBorderWidth()));
            leaveEvent.ignore();
            switch(currentView_.get()) {
                case 1:
                    inport1_.distributeEvent(&leaveEvent);
                    break;
                case 2:
                    inport2_.distributeEvent(&leaveEvent);
                    break;
                case 3:
                    inport3_.distributeEvent(&leaveEvent);
                    break;
                case 4:
                    inport4_.distributeEvent(&leaveEvent);
                    break;
            }
        }
        else {
            tgt::MouseEvent enterEvent(1, 1, tgt::MouseEvent::ENTER, e->modifiers(), e->button(), e->viewport() - ivec2(0, getBorderWidth()));
            enterEvent.ignore();
            switch(currentView_.get()) {
                case 1:
                    inport1_.distributeEvent(&enterEvent);
                    break;
                case 2:
                    inport2_.distributeEvent(&enterEvent);
                    break;
                case 3:
                    inport3_.distributeEvent(&enterEvent);
                    break;
                case 4:
                    inport4_.distributeEvent(&enterEvent);
                    break;
            }
        }
    }

    if(insideViewPort_) {
        //hidden tabbar is implicitely handled through getBorderWidth
        int newY =  e->y() - getBorderWidth();
        if(renderAtBottom_.get())
            newY = e->y();

        tgt::MouseEvent moveEvent(e->x(), newY, e->action(), e->modifiers(), e->button(), e->viewport() - ivec2(0, getBorderWidth()));
        moveEvent.ignore();

        switch(currentView_.get()) {
            case 1:
                inport1_.distributeEvent(&moveEvent);
                break;
            case 2:
                inport2_.distributeEvent(&moveEvent);
                break;
            case 3:
                inport3_.distributeEvent(&moveEvent);
                break;
            case 4:
                inport4_.distributeEvent(&moveEvent);
                break;
        }

        if(mouseOverButton_ != -1)
            invalidate();
        mouseOverButton_ = -1;
    }
    else {
        if(hideTabbar_.get()) {
            mouseOverButton_ = -1;
        }
        else {
            //in tab-bar:
            if((e->y() > 0) && (e->y() < outport_.getSize().y) && (e->x() > 0) && (e->x() < outport_.getSize().x)) {
                int div = outport_.getSize().x / 4;
                int n = e->x() / div;

                if(e->action() == tgt::MouseEvent::PRESSED) {
                    if(e->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT) {
                        currentView_.set(n+1);
                    }
                }

                if(mouseOverButton_ != n)
                    invalidate();
                mouseOverButton_ = n;
            }
            else {
                if(mouseOverButton_ != -1)
                    invalidate();
                mouseOverButton_ = -1;
            }
        }
    }
}

void TabbedView::invalidate(int inv) {
    RenderProcessor::invalidate(inv);
}

void TabbedView::onEvent(tgt::Event* e) {
    tgt::MouseEvent* me = dynamic_cast<tgt::MouseEvent*>(e);

    if(me) {
        handleMouseEvent(me);
    }
    else {
        switch(currentView_.get()) {
            case 1: inport1_.distributeEvent(e);
                    break;
            case 2: inport2_.distributeEvent(e);
                    break;
            case 3: inport3_.distributeEvent(e);
                    break;
            case 4: inport4_.distributeEvent(e);
                    break;
            default:;
        }
    }
}

} // namespace voreen
