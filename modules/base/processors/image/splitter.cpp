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

#include "splitter.h"

namespace voreen {

using tgt::ivec2;
using tgt::vec3;

Splitter::Splitter()
    : RenderProcessor()
    , showGrid_("showGrid", "Show grid", true)
    , gridColor_("gridColor", "Grid color", tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f))
    , lineWidth_("lineWidth", "Line Width", 1.0f, 0.0f, 10.0f)
    , vertical_("vertical", "Vertical", true)
    , position_("position", "Position", 0.5f, 0.0f, 1.0f)
    , outport_(Port::OUTPORT, "outport", "Image Output", true, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_RECEIVER)
    , inport1_(Port::INPORT, "inport1", "Image1 Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , inport2_(Port::INPORT, "inport2", "Image2 Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , currentPort_(-1)
    , isDragging_(false)
{
    gridColor_.setViews(Property::COLOR);
    addProperty(showGrid_);
    addProperty(gridColor_);
    addProperty(lineWidth_);
    addProperty(vertical_);
    addProperty(position_);
    position_.onChange( CallMemberAction<Splitter>(this, &Splitter::updateSizes));
    vertical_.onChange( CallMemberAction<Splitter>(this, &Splitter::updateSizes));

    addPort(outport_);
    addPort(inport1_);
    addPort(inport2_);

    outport_.onSizeReceiveChange<Splitter>(this, &Splitter::updateSizes);
}

Splitter::~Splitter() {
}

Processor* Splitter::create() const {
    return new Splitter();
}

bool Splitter::isReady() const {
    return (inport1_.isReady() || inport2_.isReady()) && outport_.isReady();
}

int Splitter::getSplitIndex() const {
    if(vertical_.get())
        return 0;
    else
        return 1;
}

void Splitter::process() {
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (inport1_.isReady()) {
        inport1_.bindColorTexture(GL_TEXTURE0);
        inport1_.getColorTexture()->enable();
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        vec3 t = vec3(0.0f);
        t[getSplitIndex()] = ((position_.get() * 0.5f) * 2.0f) - 1.0f;
        MatStack.translate(t.x, t.y, t.z);

        vec3 s = vec3(1.0f);
        s[getSplitIndex()] = position_.get();
        MatStack.scale(s.x, s.y, s.z);

        glDepthFunc(GL_ALWAYS);
        renderQuad();
        glDepthFunc(GL_LESS);

        MatStack.loadIdentity();
        inport1_.getColorTexture()->disable();
    }

    if (inport2_.isReady()) {
        inport2_.bindColorTexture(GL_TEXTURE0);
        inport2_.getColorTexture()->enable();
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        vec3 t = vec3(0.0f);
        t[getSplitIndex()] = (((position_.get() + 1.0f) * 0.5f) * 2.0f) - 1.0f;
        MatStack.translate(t.x, t.y, t.z);

        vec3 s = vec3(1.0f);
        s[getSplitIndex()] = 1.0f - position_.get();
        MatStack.scale(s.x, s.y, s.z);

        glDepthFunc(GL_ALWAYS);
        renderQuad();
        glDepthFunc(GL_LESS);

        MatStack.loadIdentity();
        inport2_.getColorTexture()->disable();
    }

    glActiveTexture(GL_TEXTURE0);

    if(showGrid_.get()) {
        glDepthFunc(GL_ALWAYS);
        glColor4f(gridColor_.get().r, gridColor_.get().g, gridColor_.get().b, gridColor_.get().a);
        glLineWidth(lineWidth_.get());
        glBegin(GL_LINES);
        if(vertical_.get()) {
            glVertex2f((position_.get() * 2.0f) - 1.0f, -1.0f);
            glVertex2f((position_.get() * 2.0f) - 1.0f, 1.0f);
        }
        else {
            glVertex2f(1.0f, (position_.get() * 2.0f) - 1.0f);
            glVertex2f(-1.0f, (position_.get() * 2.0f) - 1.0f);
        }
        glEnd();
        glLineWidth(1.0f);
        glDepthFunc(GL_LESS);
    }

    outport_.deactivateTarget();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.loadIdentity();
    LGL_ERROR;
}

void Splitter::initialize() throw (tgt::Exception) {
    RenderProcessor::initialize();
    updateSizes();
}

void Splitter::updateSizes() {
    if( (outport_.getReceivedSize().x == 0) || (outport_.getReceivedSize().y == 0) )
        return;

    tgt::ivec2 subsize1 = outport_.getReceivedSize();
    subsize1[getSplitIndex()] = static_cast<int>(subsize1[getSplitIndex()] * position_.get());
    subsize1[getSplitIndex()] = std::max(1, subsize1[getSplitIndex()]);

    tgt::ivec2 subsize2 = outport_.getReceivedSize();
    subsize2[getSplitIndex()] = static_cast<int>(subsize2[getSplitIndex()] * (1.0f - position_.get()));
    subsize2[getSplitIndex()] = std::max(1, subsize2[getSplitIndex()]);

    inport1_.requestSize(subsize1);
    inport2_.requestSize(subsize2);
}

void Splitter::mouseEvent(tgt::MouseEvent* e) {
    e->accept();
    int prevPort = currentPort_;

    if ((e->action() & tgt::MouseEvent::EXIT) == tgt::MouseEvent::EXIT)
        currentPort_ = -1;

    if ((e->action() & tgt::MouseEvent::PRESSED) == tgt::MouseEvent::PRESSED)
        isDragging_ = true;
    if ((e->action() & tgt::MouseEvent::RELEASED) == tgt::MouseEvent::RELEASED)
        isDragging_ = false;

    ivec2 vp = e->viewport();
    switch(currentPort_) {
        case 1:
            vp[getSplitIndex()] = static_cast<int>(static_cast<float>(vp[getSplitIndex()]) * position_.get());
            break;
        case 2:
            vp[getSplitIndex()] = static_cast<int>(static_cast<float>(vp[getSplitIndex()]) * (1.0f - position_.get()));
            break;
    }

    if(vertical_.get()) {
        if (!isDragging_) {
            if(e->coord().x < (e->viewport().x * position_.get()))
                currentPort_ = 1;
            else
                currentPort_ = 2;
        }

        if (currentPort_ != prevPort) {
            tgt::MouseEvent leaveEvent(1, 1, tgt::MouseEvent::EXIT, e->modifiers(), e->button(), vp);
            tgt::MouseEvent enterEvent(1, 1, tgt::MouseEvent::ENTER, e->modifiers(), e->button(), vp);
            leaveEvent.ignore();
            enterEvent.ignore();
            switch(prevPort) {
                case 1:
                    inport1_.distributeEvent(&leaveEvent);
                    break;
                case 2:
                    inport2_.distributeEvent(&leaveEvent);
                    break;
            }
            switch(currentPort_) {
                case 1:
                    inport1_.distributeEvent(&enterEvent);
                    break;
                case 2:
                    inport2_.distributeEvent(&enterEvent);
                    break;
            }
        }

        ivec2 pNew = e->coord();
        switch(currentPort_) {
            case 1: {
                        tgt::MouseEvent moveEvent(pNew.x, pNew.y, e->action(), e->modifiers(), e->button(), vp);
                        moveEvent.ignore();
                        inport1_.distributeEvent(&moveEvent);
                        break;
                    }
            case 2: {
                        pNew.x -= static_cast<int>(position_.get() * e->viewport()[getSplitIndex()]);
                        tgt::MouseEvent moveEvent(pNew.x, pNew.y, e->action(), e->modifiers(), e->button(), vp);
                        moveEvent.ignore();
                        inport2_.distributeEvent(&moveEvent);
                        break;
                    }
        }
    }
    else {
        if (!isDragging_) {
            if(e->coord().y > (e->viewport().y * (1.0f - position_.get())))
                currentPort_ = 1;
            else
                currentPort_ = 2;
        }

        if (currentPort_ != prevPort) {
            tgt::MouseEvent leaveEvent(1, 1, tgt::MouseEvent::EXIT, e->modifiers(), e->button(), vp);
            tgt::MouseEvent enterEvent(1, 1, tgt::MouseEvent::ENTER, e->modifiers(), e->button(), vp);
            leaveEvent.ignore();
            enterEvent.ignore();
            switch(prevPort) {
                case 1:
                    inport1_.distributeEvent(&leaveEvent);
                    break;
                case 2:
                    inport2_.distributeEvent(&leaveEvent);
                    break;
            }
            switch(currentPort_) {
                case 1:
                    inport1_.distributeEvent(&enterEvent);
                    break;
                case 2:
                    inport2_.distributeEvent(&enterEvent);
                    break;
            }
        }

        ivec2 pNew = e->coord();
        switch(currentPort_) {
            case 1: {
                        pNew.y -= static_cast<int>((1.0f - position_.get()) * e->viewport().y);
                        tgt::MouseEvent moveEvent(pNew.x, pNew.y, e->action(), e->modifiers(), e->button(), vp);
                        moveEvent.ignore();
                        inport1_.distributeEvent(&moveEvent);
                        break;
                    }
            case 2: {
                        tgt::MouseEvent moveEvent(pNew.x, pNew.y, e->action(), e->modifiers(), e->button(), vp);
                        moveEvent.ignore();
                        inport2_.distributeEvent(&moveEvent);
                        break;
                    }
        }
    }
}

void Splitter::onEvent(tgt::Event* e) {
    tgt::MouseEvent* me = dynamic_cast<tgt::MouseEvent*>(e);

    if (!me)
        RenderProcessor::onEvent(e);
    else
        mouseEvent(me);
}

} // namespace voreen
