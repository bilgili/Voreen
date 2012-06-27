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

#include "voreen/modules/base/processors/image/quadview.h"

namespace voreen {

QuadView::QuadView()
    : RenderProcessor()
    , showGrid_("showGrid", "Show grid", true)
    , gridColor_("gridColor", "Grid color", tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f))
    , maximized_("maximized", "Maximized sub-view", 0, 0, 4)
    , maximizeOnDoubleClick_("maximizeOnDoubleClick", "Maximize on double click", true)
#ifdef _MSC_VER
#pragma warning(disable:4355)  // passing 'this' is safe here
#endif
    , maximizeEventProp_("mouseEvent.maximize", "Maximize Event", this, &QuadView::toggleMaximization,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::MouseEvent::DOUBLECLICK, tgt::MouseEvent::MODIFIER_NONE)
#ifdef _MSC_VER
#pragma warning(disable:4355)  // passing 'this' is safe here
#endif
    , mouseMoveEventProp_("mouseEvent.move", "Move Event", this, &QuadView::mouseMove,
    tgt::MouseEvent::MOUSE_BUTTON_NONE, tgt::MouseEvent::MOTION | tgt::MouseEvent::CLICK | tgt::MouseEvent::ENTER_EXIT, tgt::MouseEvent::MODIFIER_NONE)
    , outport_(Port::OUTPORT, "outport")
    , inport1_(Port::INPORT, "inport1")
    , inport2_(Port::INPORT, "inport2")
    , inport3_(Port::INPORT, "inport3")
    , inport4_(Port::INPORT, "inport4")
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
    addPort(inport4_);

    inport1_.sizeOriginChanged(this);
    inport2_.sizeOriginChanged(this);
    inport3_.sizeOriginChanged(this);
    inport4_.sizeOriginChanged(this);
}

QuadView::~QuadView() {
}

Processor* QuadView::create() const {
    return new QuadView();
}

std::string QuadView::getProcessorInfo() const {
    return "Allows to arrange four renderings in one multiview layout.";
}

bool QuadView::isReady() const {
    if (!outport_.isReady())
        return false;

    if (!inport1_.isReady() && !inport2_.isReady() && !inport3_.isReady() && !inport4_.isReady())
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
            case 4: if(!inport4_.isReady())
                        return false;
                    break;
        }
    }
    return true;
}

void QuadView::process() {
    if (maximized_.get() == 0) {
        glMatrixMode(GL_MODELVIEW);
        outport_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (inport1_.isReady()) {
            inport1_.bindColorTexture(GL_TEXTURE0);
            inport1_.getColorTexture()->enable();
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glTranslatef(-0.5f, 0.5f, 0.0f);
            glScalef(0.5f, 0.5f, 1.0f);

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
            glTranslatef(0.5f, 0.5f, 0.0f);
            glScalef(0.5f, 0.5f, 1.0f);

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
            glTranslatef(-0.5f, -0.5f, 0.0f);
            glScalef(0.5f, 0.5f, 1.0f);

            glDepthFunc(GL_ALWAYS);
            renderQuad();
            glDepthFunc(GL_LESS);

            glLoadIdentity();
            inport3_.getColorTexture()->disable();
        }

        if (inport4_.isReady()) {
            inport4_.bindColorTexture(GL_TEXTURE0);
            inport4_.getColorTexture()->enable();
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glTranslatef(0.5f, -0.5f, 0.0f);
            glScalef(0.5f, 0.5f, 1.0f);

            glDepthFunc(GL_ALWAYS);
            renderQuad();
            glDepthFunc(GL_LESS);

            glLoadIdentity();
            inport4_.getColorTexture()->disable();
        }

        glActiveTexture(GL_TEXTURE0);

        if(showGrid_.get()) {
            glDepthFunc(GL_ALWAYS);
            glColor4f(gridColor_.get().r, gridColor_.get().g, gridColor_.get().b, gridColor_.get().a);
            glBegin(GL_LINES);
            glVertex2f(-1.0f, 0.0f);
            glVertex2f(1.0f, 0.0f);

            glVertex2f(0.0f, 1.0f);
            glVertex2f(0.0f, -1.0f);
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
            case 4: if(!inport4_.isReady())
                        return;

                    outport_.activateTarget();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    inport4_.bindColorTexture(GL_TEXTURE0);
                    inport4_.getColorTexture()->enable();
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

                    glDepthFunc(GL_ALWAYS);
                    renderQuad();
                    glDepthFunc(GL_LESS);

                    glLoadIdentity();
                    inport4_.getColorTexture()->disable();
                    outport_.deactivateTarget();
                    break;
        }
    }
}

void QuadView::initialize() throw (VoreenException) {
    RenderProcessor::initialize();
}

void QuadView::portResized(RenderPort* /*p*/, tgt::ivec2 newsize) {
    outport_.resize(newsize);

    updateSizes();
    invalidate();
}

void QuadView::updateSizes() {
    if (outport_.getSize() == tgt::ivec2(0))
        return;

    if(maximized_.get() == 0) {
        tgt::ivec2 subsize = outport_.getSize() / 2;
        inport1_.resize(subsize);
        inport2_.resize(subsize);
        inport3_.resize(subsize);
        inport4_.resize(subsize);
    }
    else {
        switch(maximized_.get()) {
            case 1: inport1_.resize(outport_.getSize());
                    break;
            case 2: inport2_.resize(outport_.getSize());
                    break;
            case 3: inport3_.resize(outport_.getSize());
                    break;
            case 4: inport4_.resize(outport_.getSize());
                    break;
            default:;
        }
    }
}

void QuadView::mouseMove(tgt::MouseEvent* e) {
    e->accept();
    int prevCurrenPort = currentPort_;

    if((e->action() & tgt::MouseEvent::EXIT) == tgt::MouseEvent::EXIT)
        currentPort_ = -1;

    if((e->action() & tgt::MouseEvent::PRESSED) == tgt::MouseEvent::PRESSED)
        isDragging_ = true;
    if((e->action() & tgt::MouseEvent::RELEASED) == tgt::MouseEvent::RELEASED)
        isDragging_ = false;

    if(!isDragging_) {
        if(e->y() < (e->viewport().y/2)) {
            if(e->x() < (e->viewport().x/2)) {
                currentPort_ = 1;
            } else {
                currentPort_ = 2;
            }
        } else {
            if(e->x() < (e->viewport().x/2)) {
                currentPort_ = 3;
            } else {
                currentPort_ = 4;
            }
        }
    }

    if(currentPort_ != prevCurrenPort) {
        tgt::MouseEvent leaveEvent(1, 1, tgt::MouseEvent::EXIT, e->modifiers(), e->button(), e->viewport() / 2);
        tgt::MouseEvent enterEvent(1, 1, tgt::MouseEvent::ENTER, e->modifiers(), e->button(), e->viewport() / 2);
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
            case 4:
                inport4_.distributeEvent(&leaveEvent);
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
            case 4:
                inport4_.distributeEvent(&enterEvent);
                break;
        }
    }
    tgt::MouseEvent moveEvent(e->x() % (e->viewport().x/2), e->y() % (e->viewport().y/2), tgt::MouseEvent::MOTION, e->modifiers(), e->button(), e->viewport() / 2);
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
        case 4:
            inport4_.distributeEvent(&moveEvent);
            break;
    }
}

void QuadView::sizeOriginChanged(RenderPort* /*p*/) {
}

bool QuadView::testSizeOrigin(const RenderPort* p, void* so) const {
    if(p->getSizeOrigin() == so)
        return true;

    if(!so)
        return true;

    if(!p->getSizeOrigin())
        return true;

    return false;
}

void QuadView::invalidate(int inv) {
    RenderProcessor::invalidate(inv);
}

void QuadView::onEvent(tgt::Event* e) {

    tgt::MouseEvent* me = dynamic_cast<tgt::MouseEvent*>(e);

    if (!me || mouseMoveEventProp_.accepts(me) || (maximizeEventProp_.accepts(me) && maximizeOnDoubleClick_.get())) {
        RenderProcessor::onEvent(e);
        return;
    }

    if (maximized_.get() == 0) {
        if (me->y() < (me->viewport().y / 2)) {
            if (me->x() < (me->viewport().x / 2)) {
                tgt::MouseEvent newme(me->x(), me->y(), me->action(), me->modifiers(), me->button(), me->viewport() / 2);
                newme.ignore();  // accepted is set to true by default
                inport1_.distributeEvent(&newme);
                if (newme.isAccepted())
                    me->accept();
            }
            else {
                tgt::MouseEvent newme(me->x() - (me->viewport().x / 2), me->y(), me->action(), me->modifiers(), me->button(), me->viewport() / 2);
                newme.ignore();
                inport2_.distributeEvent(&newme);
                if (newme.isAccepted())
                    me->accept();
            }
        }
        else {
            if (me->x() < (me->viewport().x / 2)) {
                tgt::MouseEvent newme(me->x(), me->y() - (me->viewport().y / 2), me->action(), me->modifiers(), me->button(), me->viewport() / 2);
                newme.ignore();
                inport3_.distributeEvent(&newme);
                if (newme.isAccepted())
                    me->accept();
            }
            else {
                tgt::MouseEvent newme(me->x() - (me->viewport().x / 2), me->y() - (me->viewport().y / 2), me->action(), me->modifiers(), me->button(), me->viewport() / 2);
                newme.ignore();
                inport4_.distributeEvent(&newme);
                if (newme.isAccepted())
                    me->accept();
            }
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
            case 4: inport4_.distributeEvent(me);
                    break;
            default:;
        }
    }
}

void QuadView::toggleMaximization(tgt::MouseEvent* me) {

    if (maximizeOnDoubleClick_.get()) {
        if (maximized_.get() == 0) {
            if (me->y() < (me->viewport().y / 2)) {
                if (me->x() < (me->viewport().x / 2)) {
                    maximized_.set(1);
                }
                else {
                    maximized_.set(2);
                }
            }
            else {
                if (me->x() < (me->viewport().x / 2)) {
                    maximized_.set(3);
                }
                else {
                    maximized_.set(4);
                }
            }
        }
        else {
            maximized_.set(0);
        }
        updateSizes();

        me->accept();
    }
}

} // namespace voreen
