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

#include "tripleview.h"

namespace voreen {

using tgt::ivec2;
using tgt::vec3;

TripleView::TripleView()
    : RenderProcessor()
    , showGrid_("showGrid", "Show grid", true)
    , gridColor_("gridColor", "Grid color", tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f))
    , configuration_("configuration", "Configuration")
    , mouseMoveEventProp_("mouseEvent.move", "Move Event", this, &TripleView::mouseMove,
    tgt::MouseEvent::MOUSE_BUTTON_NONE, tgt::MouseEvent::MOTION | tgt::MouseEvent::CLICK | tgt::MouseEvent::ENTER_EXIT, tgt::MouseEvent::MODIFIER_NONE)
    , outport_(Port::OUTPORT, "outport", "Image Output", true, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_RECEIVER)
    , inport1_(Port::INPORT, "inport1", "Image1 Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , inport2_(Port::INPORT, "inport2", "Image2 Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , inport3_(Port::INPORT, "inport3", "Image3 Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , currentPort_(-1)
    , isDragging_(false)
{
    gridColor_.setViews(Property::COLOR);
    addProperty(showGrid_);
    addProperty(gridColor_);
    configuration_.addOption("abc", "abc", abc);
    configuration_.addOption("Abc", "Abc", Abc);
    configuration_.addOption("Bac", "Bac", Bac);
    configuration_.addOption("Cab", "Cab", Cab);
    configuration_.addOption("A", "A", A);
    configuration_.addOption("B", "B", B);
    configuration_.addOption("C", "C", C);
    addProperty(configuration_);
    configuration_.onChange(CallMemberAction<TripleView>(this, &TripleView::updateSizes));
    addEventProperty(mouseMoveEventProp_);

    addPort(outport_);
    addPort(inport1_);
    addPort(inport2_);
    addPort(inport3_);

    outport_.onSizeReceiveChange<TripleView>(this, &TripleView::updateSizes);
}

TripleView::~TripleView() {
}

Processor* TripleView::create() const {
    return new TripleView();
}

bool TripleView::isReady() const {
    if (!outport_.isReady())
        return false;

    if (!inport1_.isReady() && !inport2_.isReady() && !inport3_.isReady())
        return false;

    if(configuration_.getValue() > 3) {
        switch(configuration_.getValue()) {
            case A: if(!inport1_.isReady())
                        return false;
                    break;
            case B: if(!inport2_.isReady())
                        return false;
                    break;
            case C: if(!inport3_.isReady())
                        return false;
                    break;
        }
    }
    return true;
}

void TripleView::renderPortQuad(RenderPort& rp, tgt::vec3 translate, tgt::vec3 scale) {
    rp.bindColorTexture(GL_TEXTURE0);
    rp.getColorTexture()->enable();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    MatStack.translate(translate.x, translate.y, translate.z);
    MatStack.scale(scale.x, scale.y, scale.z);

    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    MatStack.loadIdentity();
    rp.getColorTexture()->disable();
}

void TripleView::renderLargeSmallSmall(RenderPort& large, RenderPort& small1, RenderPort& small2) {
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (large.isReady())
        renderPortQuad(large, vec3(-0.333333f, 0.0f, 0.0f), vec3(0.666666, 1.0f, 1.0f));

    if (small1.isReady())
        renderPortQuad(small1, vec3(0.666666f, 0.5f, 0.0f), vec3(0.333333f, 0.5f, 1.0f));

    if (small2.isReady())
        renderPortQuad(small2, vec3(0.666666f, -0.5f, 0.0f), vec3(0.333333f, 0.5f, 1.0f));

    glActiveTexture(GL_TEXTURE0);

    if(showGrid_.get()) {
        glDepthFunc(GL_ALWAYS);
        glColor4f(gridColor_.get().r, gridColor_.get().g, gridColor_.get().b, gridColor_.get().a);
        glBegin(GL_LINES);
        glVertex2f(0.333333f, -1.0f);
        glVertex2f(0.333333f, 1.0f);

        glVertex2f(0.333333f, 0.0f);
        glVertex2f(1.f, 0.0f);

        glEnd();
        glDepthFunc(GL_LESS);
    }

    outport_.deactivateTarget();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.loadIdentity();
    LGL_ERROR;
}

void TripleView::process() {
    switch(configuration_.getValue()) {
        case abc: {
                      MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
                      outport_.activateTarget();
                      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                      if (inport1_.isReady())
                          renderPortQuad(inport1_, vec3(-0.66666f, 0.0f, 0.0f), vec3(0.333333f, 1.0f, 1.0f));

                      if (inport2_.isReady())
                          renderPortQuad(inport2_, vec3(0.0f, 0.0f, 0.0f), vec3(0.333333f, 1.0f, 1.0f));

                      if (inport3_.isReady())
                          renderPortQuad(inport3_, vec3(0.666666f, 0.0f, 0.0f), vec3(0.333333f, 1.0f, 1.0f));

                      glActiveTexture(GL_TEXTURE0);

                      if(showGrid_.get()) {
                          glDepthFunc(GL_ALWAYS);
                          glColor4f(gridColor_.get().r, gridColor_.get().g, gridColor_.get().b, gridColor_.get().a);
                          glBegin(GL_LINES);
                          glVertex2f(-0.333333f, -1.0f);
                          glVertex2f(-0.333333f, 1.0f);

                          glVertex2f(0.333333f, -1.0f);
                          glVertex2f(0.333333f, 1.0f);
                          glEnd();
                          glDepthFunc(GL_LESS);
                      }

                      outport_.deactivateTarget();
                      MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
                      MatStack.loadIdentity();
                      LGL_ERROR;
                  }
                  break;
        case Abc: renderLargeSmallSmall(inport1_, inport2_, inport3_);
                  break;
        case Bac: renderLargeSmallSmall(inport2_, inport1_, inport3_);
                  break;
        case Cab: renderLargeSmallSmall(inport3_, inport1_, inport2_);
                  break;
        //maximized:
        case A: if(!inport1_.isReady())
                    return;

                outport_.activateTarget();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                renderPortQuad(inport1_, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));
                outport_.deactivateTarget();
                break;
        case B: if(!inport2_.isReady())
                    return;

                outport_.activateTarget();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                renderPortQuad(inport2_, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));
                outport_.deactivateTarget();
                break;
        case C: if(!inport3_.isReady())
                    return;

                outport_.activateTarget();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                renderPortQuad(inport3_, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));
                outport_.deactivateTarget();
                break;
    }
}

void TripleView::initialize() throw (tgt::Exception) {
    RenderProcessor::initialize();
    updateSizes();
}

void TripleView::updateSizes() {
    if (outport_.getReceivedSize() == tgt::ivec2(0))
        return;
    else {
        tgt::ivec2 outsize = outport_.getReceivedSize();
        switch(configuration_.getValue()) {
            case abc: inport1_.requestSize(ivec2(static_cast<int>(outsize.x * 0.333333f), outsize.y));
                      inport2_.requestSize(ivec2(static_cast<int>(outsize.x * 0.333333f), outsize.y));
                      inport3_.requestSize(ivec2(static_cast<int>(outsize.x * 0.333333f), outsize.y));
                      break;
            case Abc: inport1_.requestSize(ivec2(static_cast<int>(outsize.x * 0.666666f), outsize.y));
                      inport2_.requestSize(ivec2(static_cast<int>(outsize.x * 0.333333f), outsize.y / 2));
                      inport3_.requestSize(ivec2(static_cast<int>(outsize.x * 0.333333f), outsize.y / 2));
                      break;
            case Bac: inport2_.requestSize(ivec2(static_cast<int>(outsize.x * 0.666666f), outsize.y));
                      inport1_.requestSize(ivec2(static_cast<int>(outsize.x * 0.333333f), outsize.y / 2));
                      inport3_.requestSize(ivec2(static_cast<int>(outsize.x * 0.333333f), outsize.y / 2));
                      break;
            case Cab: inport3_.requestSize(ivec2(static_cast<int>(outsize.x * 0.666666f), outsize.y));
                      inport1_.requestSize(ivec2(static_cast<int>(outsize.x * 0.333333f), outsize.y / 2));
                      inport2_.requestSize(ivec2(static_cast<int>(outsize.x * 0.333333f), outsize.y / 2));
                      break;
            case A: inport1_.requestSize(outport_.getReceivedSize());
                    break;
            case B: inport2_.requestSize(outport_.getReceivedSize());
                    break;
            case C: inport3_.requestSize(outport_.getReceivedSize());
                    break;
            default:;
        }
    }
}

void TripleView::mouseMove(tgt::MouseEvent* e) {
    //FIXME
    //e->accept();
    //int prevCurrenPort = currentPort_;

    //if (configuration_.getValue() == abc) {
        //if ((e->action() & tgt::MouseEvent::EXIT) == tgt::MouseEvent::EXIT)
            //currentPort_ = -1;

        //if ((e->action() & tgt::MouseEvent::PRESSED) == tgt::MouseEvent::PRESSED)
            //isDragging_ = true;
        //if ((e->action() & tgt::MouseEvent::RELEASED) == tgt::MouseEvent::RELEASED)
            //isDragging_ = false;

        //if (!isDragging_) {
            //if(e->x() < (e->viewport().x / 3))
                //currentPort_ = 1;
            //else if(e->x() < (e->viewport().x * 0.66666f))
                //currentPort_ = 2;
            //else
                //currentPort_ = 3;
        //}

        //if (currentPort_ != prevCurrenPort) {
            //tgt::MouseEvent leaveEvent(1, 1, tgt::MouseEvent::EXIT, e->modifiers(), e->button(), ivec2(e->viewport().x / 3, e->viewport().y));
            //tgt::MouseEvent enterEvent(1, 1, tgt::MouseEvent::ENTER, e->modifiers(), e->button(), ivec2(e->viewport().x / 3, e->viewport().y));
            //leaveEvent.ignore();
            //enterEvent.ignore();
            //switch(prevCurrenPort) {
                //case 1:
                    //inport1_.distributeEvent(&leaveEvent);
                    //break;
                //case 2:
                    //inport2_.distributeEvent(&leaveEvent);
                    //break;
                //case 3:
                    //inport3_.distributeEvent(&leaveEvent);
                    //break;
            //}
            //switch(currentPort_) {
                //case 1:
                    //inport1_.distributeEvent(&enterEvent);
                    //break;
                //case 2:
                    //inport2_.distributeEvent(&enterEvent);
                    //break;
                //case 3:
                    //inport3_.distributeEvent(&enterEvent);
                    //break;
            //}
        //}
        //tgt::MouseEvent moveEvent(e->x() % (e->viewport().x/3), e->y(), tgt::MouseEvent::MOTION, e->modifiers(), e->button(), ivec2(e->viewport().x / 3, e->viewport().y));
        //moveEvent.ignore();
        //switch(currentPort_) {
            //case 1:
                //inport1_.distributeEvent(&moveEvent);
                //break;
            //case 2:
                //inport2_.distributeEvent(&moveEvent);
                //break;
            //case 3:
                //inport3_.distributeEvent(&moveEvent);
                //break;
        //}
    //}
    //else {
        //switch(configuration_.getValue()) {
            //case A: inport1_.distributeEvent(e);
                //break;
            //case B: inport2_.distributeEvent(e);
                //break;
            //case C: inport3_.distributeEvent(e);
                //break;
            //default:;
        //}
    //}
}

void TripleView::invalidate(int inv) {
    RenderProcessor::invalidate(inv);
}

void TripleView::distributeEventLargeSmallSmall(RenderPort& large, RenderPort& small1, RenderPort& small2, tgt::MouseEvent* me) {
    if (me->x() < (me->viewport().x * 0.666666f)) {
        tgt::MouseEvent newme(me->x(), me->y(), me->action(), me->modifiers(), me->button(), ivec2(static_cast<int>(me->viewport().x * 0.666666f), me->viewport().y));
        newme.ignore();  // accepted is set to true by default
        large.distributeEvent(&newme);
        if (newme.isAccepted())
            me->accept();
    }
    else if (me->y() < (me->viewport().y * 0.5)) {
        tgt::MouseEvent newme(me->x() - static_cast<int>(me->viewport().x * 0.666666f), me->y(), me->action(), me->modifiers(), me->button(), ivec2(me->viewport().x / 3, me->viewport().y / 2));
        newme.ignore();
        small1.distributeEvent(&newme);
        if (newme.isAccepted())
            me->accept();
    }
    else {
        tgt::MouseEvent newme(me->x() - static_cast<int>(me->viewport().x * 0.666666f), me->y() - static_cast<int>(me->viewport().y * 0.5f), me->action(), me->modifiers(), me->button(), ivec2(me->viewport().x / 3, me->viewport().y / 2));
        newme.ignore();
        small2.distributeEvent(&newme);
        if (newme.isAccepted())
            me->accept();
    }
}

void TripleView::onEvent(tgt::Event* e) {
    tgt::MouseEvent* me = dynamic_cast<tgt::MouseEvent*>(e);

    if (!me || mouseMoveEventProp_.accepts(me)) {
        RenderProcessor::onEvent(e);
        return;
    }

    switch(configuration_.getValue()) {
        case abc:
            if (me->x() < (me->viewport().x * 0.33333f)) {
                tgt::MouseEvent newme(me->x(), me->y(), me->action(), me->modifiers(), me->button(), ivec2(me->viewport().x / 3, me->viewport().y));
                newme.ignore();  // accepted is set to true by default
                inport1_.distributeEvent(&newme);
                if (newme.isAccepted())
                    me->accept();
            }
            else if (me->x() < (me->viewport().x * 0.66666f)) {
                tgt::MouseEvent newme(me->x() - (me->viewport().x / 3), me->y(), me->action(), me->modifiers(), me->button(), ivec2(me->viewport().x / 3, me->viewport().y));
                newme.ignore();
                inport2_.distributeEvent(&newme);
                if (newme.isAccepted())
                    me->accept();
            }
            else {
                tgt::MouseEvent newme(me->x() - (me->viewport().x * 2 / 3), me->y(), me->action(), me->modifiers(), me->button(), ivec2(me->viewport().x / 3, me->viewport().y));
                newme.ignore();
                inport3_.distributeEvent(&newme);
                if (newme.isAccepted())
                    me->accept();
            }
            break;
        case Abc: distributeEventLargeSmallSmall(inport1_, inport2_, inport3_, me);
                  break;
        case Bac: distributeEventLargeSmallSmall(inport2_, inport1_, inport3_, me);
                  break;
        case Cab: distributeEventLargeSmallSmall(inport3_, inport1_, inport2_, me);
                  break;
        case A: inport1_.distributeEvent(me);
                break;
        case B: inport2_.distributeEvent(me);
                break;
        case C: inport3_.distributeEvent(me);
                break;
        default:;
    }
}

} // namespace voreen
