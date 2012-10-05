/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

TripleView::TripleView()
    : RenderProcessor()
    , showGrid_("showGrid", "Show grid", true)
    , gridColor_("gridColor", "Grid color", tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f))
    , maximized_("maximized", "Maximized sub-view", 0, 0, 4)
    , maximizeOnDoubleClick_("maximizeOnDoubleClick", "Maximize on double click", true)
    , maximizeEventProp_("mouseEvent.maximize", "Maximize Event", this, &TripleView::toggleMaximization,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::MouseEvent::DOUBLECLICK, tgt::MouseEvent::MODIFIER_NONE)
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
    addProperty(maximized_);
    maximized_.setVisible(false);
    addProperty(maximizeOnDoubleClick_);
    addEventProperty(maximizeEventProp_);
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

    if(maximized_.get() == 0) {
        switch(maximized_.get()) {
            case 1: if(!inport1_.isReady())
                        return false;
                    break;
            case 2: if(!inport2_.isReady())
                        return false;
                    break;
            case 3: if(!inport3_.isReady())
                        return false;
                    break;
        }
    }
    return true;
}

void TripleView::process() {
    if (maximized_.get() == 0) {
        glMatrixMode(GL_MODELVIEW);
        outport_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (inport1_.isReady()) {
            inport1_.bindColorTexture(GL_TEXTURE0);
            inport1_.getColorTexture()->enable();
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glTranslatef(-0.66666f, 0.0f, 0.0f);
            glScalef(0.333333f, 1.0f, 1.0f);

            glDepthFunc(GL_ALWAYS);
            renderQuad();
            glDepthFunc(GL_LESS);

            glLoadIdentity();
            inport1_.getColorTexture()->disable();
        }

        if (inport2_.isReady()) {
            inport2_.bindColorTexture(GL_TEXTURE0);
            inport2_.getColorTexture()->enable();
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            //glTranslatef(0.0f, 0.0f, 0.0f);
            glScalef(0.333333f, 1.0f, 1.0f);

            glDepthFunc(GL_ALWAYS);
            renderQuad();
            glDepthFunc(GL_LESS);

            glLoadIdentity();
            inport2_.getColorTexture()->disable();
        }

        if (inport3_.isReady()) {
            inport3_.bindColorTexture(GL_TEXTURE0);
            inport3_.getColorTexture()->enable();
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glTranslatef(0.666666f, 0.0f, 0.0f);
            glScalef(0.333333f, 1.0f, 1.0f);

            glDepthFunc(GL_ALWAYS);
            renderQuad();
            glDepthFunc(GL_LESS);

            glLoadIdentity();
            inport3_.getColorTexture()->disable();
        }

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
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        LGL_ERROR;
    }
    else {
        //maximized:
        switch(maximized_.get()) {
            case 1: if(!inport1_.isReady())
                        return;

                    outport_.activateTarget();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    inport1_.bindColorTexture(GL_TEXTURE0);
                    inport1_.getColorTexture()->enable();
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

                    glDepthFunc(GL_ALWAYS);
                    renderQuad();
                    glDepthFunc(GL_LESS);

                    glLoadIdentity();
                    inport1_.getColorTexture()->disable();
                    outport_.deactivateTarget();
                    break;
            case 2: if(!inport2_.isReady())
                        return;

                    outport_.activateTarget();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    inport2_.bindColorTexture(GL_TEXTURE0);
                    inport2_.getColorTexture()->enable();
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

                    glDepthFunc(GL_ALWAYS);
                    renderQuad();
                    glDepthFunc(GL_LESS);

                    glLoadIdentity();
                    inport2_.getColorTexture()->disable();
                    outport_.deactivateTarget();
                    break;
            case 3: if(!inport3_.isReady())
                        return;

                    outport_.activateTarget();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    inport3_.bindColorTexture(GL_TEXTURE0);
                    inport3_.getColorTexture()->enable();
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

                    glDepthFunc(GL_ALWAYS);
                    renderQuad();
                    glDepthFunc(GL_LESS);

                    glLoadIdentity();
                    inport3_.getColorTexture()->disable();
                    outport_.deactivateTarget();
                    break;
        }
    }
}

void TripleView::initialize() throw (tgt::Exception) {
    RenderProcessor::initialize();
    updateSizes();
}

void TripleView::updateSizes() {
    if (outport_.getReceivedSize() == tgt::ivec2(0))
        return;

    if(maximized_.get() == 0) {
        tgt::ivec2 subsize = outport_.getReceivedSize();
        subsize.x /= 3;
        inport1_.requestSize(subsize);
        inport2_.requestSize(subsize);
        inport3_.requestSize(subsize);
    }
    else {
        switch(maximized_.get()) {
            case 1: inport1_.requestSize(outport_.getReceivedSize());
                    break;
            case 2: inport2_.requestSize(outport_.getReceivedSize());
                    break;
            case 3: inport3_.requestSize(outport_.getReceivedSize());
                    break;
            default:;
        }
    }
}

void TripleView::mouseMove(tgt::MouseEvent* e) {
    e->accept();
    int prevCurrenPort = currentPort_;

    if (maximized_.get() == 0) {
        if ((e->action() & tgt::MouseEvent::EXIT) == tgt::MouseEvent::EXIT)
            currentPort_ = -1;

        if ((e->action() & tgt::MouseEvent::PRESSED) == tgt::MouseEvent::PRESSED)
            isDragging_ = true;
        if ((e->action() & tgt::MouseEvent::RELEASED) == tgt::MouseEvent::RELEASED)
            isDragging_ = false;

        if (!isDragging_) {
            if(e->x() < (e->viewport().x / 3))
                currentPort_ = 1;
            else if(e->x() < (e->viewport().x * 0.66666f))
                currentPort_ = 2;
            else
                currentPort_ = 3;
        }

        if (currentPort_ != prevCurrenPort) {
            tgt::MouseEvent leaveEvent(1, 1, tgt::MouseEvent::EXIT, e->modifiers(), e->button(), ivec2(e->viewport().x / 3, e->viewport().y));
            tgt::MouseEvent enterEvent(1, 1, tgt::MouseEvent::ENTER, e->modifiers(), e->button(), ivec2(e->viewport().x / 3, e->viewport().y));
            leaveEvent.ignore();
            enterEvent.ignore();
            switch(prevCurrenPort) {
                case 1:
                    inport1_.distributeEvent(&leaveEvent);
                    break;
                case 2:
                    inport2_.distributeEvent(&leaveEvent);
                    break;
                case 3:
                    inport3_.distributeEvent(&leaveEvent);
                    break;
            }
            switch(currentPort_) {
                case 1:
                    inport1_.distributeEvent(&enterEvent);
                    break;
                case 2:
                    inport2_.distributeEvent(&enterEvent);
                    break;
                case 3:
                    inport3_.distributeEvent(&enterEvent);
                    break;
            }
        }
        tgt::MouseEvent moveEvent(e->x() % (e->viewport().x/3), e->y(), tgt::MouseEvent::MOTION, e->modifiers(), e->button(), ivec2(e->viewport().x / 3, e->viewport().y));
        moveEvent.ignore();
        switch(currentPort_) {
            case 1:
                inport1_.distributeEvent(&moveEvent);
                break;
            case 2:
                inport2_.distributeEvent(&moveEvent);
                break;
            case 3:
                inport3_.distributeEvent(&moveEvent);
                break;
        }
    }
    else {
        switch(maximized_.get()) {
            case 1: inport1_.distributeEvent(e);
                break;
            case 2: inport2_.distributeEvent(e);
                break;
            case 3: inport3_.distributeEvent(e);
                break;
            default:;
        }
    }
}

void TripleView::invalidate(int inv) {
    RenderProcessor::invalidate(inv);
}

void TripleView::onEvent(tgt::Event* e) {

    tgt::MouseEvent* me = dynamic_cast<tgt::MouseEvent*>(e);

    if (!me || mouseMoveEventProp_.accepts(me) || (maximizeEventProp_.accepts(me) && maximizeOnDoubleClick_.get())) {
        RenderProcessor::onEvent(e);
        return;
    }

    if (maximized_.get() == 0) {
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
    }
    else {
        switch(maximized_.get()) {
            case 1: inport1_.distributeEvent(me);
                    break;
            case 2: inport2_.distributeEvent(me);
                    break;
            case 3: inport3_.distributeEvent(me);
                    break;
            default:;
        }
    }
}

void TripleView::toggleMaximization(tgt::MouseEvent* me) {
    if (maximizeOnDoubleClick_.get()) {
        if (maximized_.get() == 0) {
            if (me->x() < (me->viewport().x * 0.333333f))
                maximized_.set(1);
            else if (me->x() < (me->viewport().x * 0.666666f))
                maximized_.set(2);
            else
                maximized_.set(3);
        }
        else
            maximized_.set(0);

        updateSizes();
        me->accept();
    }
}

} // namespace voreen
