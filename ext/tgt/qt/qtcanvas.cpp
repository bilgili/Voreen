/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "qtcanvas.h"

namespace tgt {

// shared context widget
QGLWidget* QtCanvas::shareWidget_ = 0;

QtCanvas::QtCanvas(const std::string& title,
                   const ivec2& size,
                   const Buffers buffers,
                   QWidget* parent, bool shared, Qt::WFlags f, char* /*name*/)
    : QGLWidget(getQGLFormat(buffers), parent, (shared ? shareWidget_ : 0), f),
      GLCanvas(title, size, buffers)
{
    resize(size.x, size.y);
    if (shared && shareWidget_ == 0)
        shareWidget_ = this;

    setWindowTitle(QString(title.c_str()));

    // we have our own AutoBufferSwap-mechanism (GLCanvas::setAutoFlush), so disable the one of qt
    setAutoBufferSwap(false);

    rgbaSize_ = ivec4(format().redBufferSize(),
                      format().greenBufferSize(),
                      format().blueBufferSize(),
                      format().alphaBufferSize());
    stencilSize_ = format().stencilBufferSize();
    depthSize_ = format().depthBufferSize();
    doubleBuffered_ = doubleBuffer();
    stereoViewing_ = format().stereo();
}

QtCanvas::QtCanvas(QWidget* parent, bool shared, Qt::WFlags f, char* /*name*/)
    : QGLWidget(parent, (shared ? shareWidget_ : 0), f),
      GLCanvas()
{
    if (shared && shareWidget_ == 0)
        shareWidget_ = this;

    // we have our own AutoBufferSwap-mechanism (GLCanvas::setAutoFlush), so disable the one of qt
    setAutoBufferSwap(false);

    rgbaSize_ = ivec4(format().redBufferSize(),
                      format().greenBufferSize(),
                      format().blueBufferSize(),
                      format().alphaBufferSize());
    stencilSize_ = format().stencilBufferSize();
    depthSize_ = format().depthBufferSize();
    doubleBuffered_ = doubleBuffer();
    stereoViewing_ = format().stereo();
}

QtCanvas::~QtCanvas() {}

//TODO: check if this is necessary. joerg
void QtCanvas::init() {

    // FIXME: TODO comment
//    setAttribute(Qt::WA_DeleteOnClose, true);

    show();

    GLCanvas::init();
}

void QtCanvas::initializeGL() {
}

void QtCanvas::resizeGL(int w, int h) {
    sizeChanged(ivec2(w, h));
}

void QtCanvas::paintGL() {
    paint();
}

void QtCanvas::repaint() {
    updateGL();
}

void QtCanvas::update() {
    QWidget::update();
}

void QtCanvas::swap() {
    QGLWidget::swapBuffers();
}

void QtCanvas::getGLFocus() {
    QGLWidget::makeCurrent();
}

void QtCanvas::toggleFullScreen() {
    if (fullscreen_) {
        fullscreen_ = false;
        showNormal();
    }
    else {
        showFullScreen();
        fullscreen_ = !fullscreen_;
    }
};

/*
    These functions are called by qt if there is a user-caused event, like the pressing of a
    mouse-button, as in this case. The press-event has to be turned into a tgt::MouseEvent and
    handled: Either by a custom eventloop, or by directly broadcasting it to all eventlisteners
    that are associated with this canvas. The same is true for the other event-handling
    methods.
*/
void QtCanvas::mousePressEvent(QMouseEvent* e) {
    tgt::MouseEvent* prEv = new tgt::MouseEvent(e->x(), e->y(), tgt::MouseEvent::PRESSED,
                                                getModifier(e), getButton(e));
    eventHandler_->broadcast(prEv);
}

// See mousePressEvent
void QtCanvas::mouseReleaseEvent (QMouseEvent* e) {
    tgt::MouseEvent* relEv = new tgt::MouseEvent(e->x(), e->y(), tgt::MouseEvent::RELEASED,
                                                 getModifier(e), getButton(e));
    eventHandler_->broadcast(relEv);
}

// See mousePressEvent
void QtCanvas::mouseMoveEvent(QMouseEvent*  e) {
    tgt::MouseEvent* movEv = new tgt::MouseEvent(e->x(), e->y(), tgt::MouseEvent::MOTION,
                                                 getModifier(e), getButtons(e)); // FIXME: submit information which mouse buttons are pressed
    eventHandler_->broadcast(movEv);
}

// See mousePressEvent
void QtCanvas::mouseDoubleClickEvent(QMouseEvent* e) {
    tgt::MouseEvent* dcEv = new tgt::MouseEvent(e->x(), e->y(), tgt::MouseEvent::DOUBLECLICK,
                                                getModifier(e), getButton(e));
    eventHandler_->broadcast(dcEv);
}

// See mousePressEvent
// does not distinguish correctly between UP and DOWN => works fine in voreen!
void QtCanvas::wheelEvent(QWheelEvent* e) {
    tgt::MouseEvent::MouseButtons b = tgt::MouseEvent::MOUSE_WHEEL_DOWN;
    if (e->delta() > 0)
        b = tgt::MouseEvent::MOUSE_WHEEL_UP;
    tgt::MouseEvent* wheelEv = new tgt::MouseEvent(e->x(),e->y(), tgt::MouseEvent::WHEEL,
                                                   getModifier(e), b);
    eventHandler_->broadcast(wheelEv);
}

// See mousePressEvent
void QtCanvas::keyPressEvent(QKeyEvent* event) {
    tgt::KeyEvent* ke = new tgt::KeyEvent(getKey(event->key()), getModifier(event), true);
    eventHandler_->broadcast(ke);
}

// See mousePressEvent
void QtCanvas::keyReleaseEvent(QKeyEvent* event) {
    tgt::KeyEvent* ke = new tgt::KeyEvent(getKey(event->key()), getModifier(event), false);
    eventHandler_->broadcast(ke);
}

// yes, we need this in voreen FL
void QtCanvas::timerEvent(QTimerEvent* /*e*/) {
    tgt::TimeEvent* te = new tgt::TimeEvent();
    eventHandler_->broadcast(te);
}

tgt::MouseEvent::MouseButtons QtCanvas::getButton(QMouseEvent* e) {
    switch (e->button()) {
        case Qt::LeftButton:
            return tgt::MouseEvent::MOUSE_BUTTON_LEFT;
        case Qt::RightButton:
            return tgt::MouseEvent::MOUSE_BUTTON_RIGHT;
        case Qt::MidButton:
            return tgt::MouseEvent::MOUSE_BUTTON_MIDDLE;
        default:
            return tgt::MouseEvent::NO_MOUSE_BUTTON;
    }
}

tgt::MouseEvent::MouseButtons QtCanvas::getButtons(QMouseEvent* e) {
    /*tgt::MouseEvent::MouseButtons*/ int buttons = tgt::MouseEvent::NO_MOUSE_BUTTON;

    if (e->buttons() & Qt::LeftButton)
        buttons |= tgt::MouseEvent::MOUSE_BUTTON_LEFT;
    if (e->buttons() & Qt::RightButton)
        buttons |= tgt::MouseEvent::MOUSE_BUTTON_RIGHT;
    if (e->buttons() & Qt::MidButton)
        buttons |= tgt::MouseEvent::MOUSE_BUTTON_MIDDLE;

    return static_cast<tgt::MouseEvent::MouseButtons>(buttons);
}

tgt::Event::Modifier QtCanvas::getModifier(QInputEvent* e) {
    int result = 0;
    if (e->modifiers() & Qt::ControlModifier)
        result |= int(tgt::Event::CTRL);
    if (e->modifiers() & Qt::ShiftModifier)
        result |= int(tgt::Event::SHIFT);
    if (e->modifiers() & Qt::AltModifier)
        result |= int(tgt::Event::ALT);
    if (e->modifiers() & Qt::MetaModifier)
        result |= int(tgt::Event::META);
    if (e->modifiers() == Qt::NoModifier)
        result = int(tgt::Event::NONE);
    return tgt::Event::Modifier(result);
}

QGLFormat QtCanvas::getQGLFormat(const Buffers buffers) {
    QGLFormat format = QGLFormat();
    format.setAlpha(buffers & GLCanvas::ALPHA_BUFFER);
    format.setDepth(buffers & GLCanvas::DEPTH_BUFFER);
    format.setDoubleBuffer(buffers & GLCanvas::DOUBLE_BUFFER);
    format.setStencil(buffers & GLCanvas::STENCIL_BUFFER);
    format.setAccum(buffers & GLCanvas::ACCUM_BUFFER);
    format.setStereo(buffers & GLCanvas::STEREO_VIEWING);
    format.setSampleBuffers(buffers & GLCanvas::MULTISAMPLING);
        
    return format;
}


#include "keymap.cpp" // FIXME: is this division of source code files sensible? - I think it is...the included file is really ugly :)

} // namespace
