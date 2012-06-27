/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/qt/widgets/overviewwidget.h"

#include <cmath>

#include <QGridLayout>

#include "tgt/assert.h"
#include "tgt/glmath.h"

#include "voreen/qt/widgets/transfuncplugin.h"
#include "voreen/core/vis/processors/render/slicerenderer.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/processors/image/copytoscreenrenderer.h"


namespace voreen {

OverViewWidget::OverViewWidget(tgt::EventListener* eventListener, QWidget* parent,
                               TextureContainer* tc, tgt::Camera* camera)
    : tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, parent, true, 0)
    , voreen::MessageReceiver("ov1")
    , leftPressed_(false)
    , rightPressed_(false)
    , oldMouseX_(0.f)
    , oldMouseY_(0.f)
    , translation_(0.f)
    , scale_(1.f)
{
    painter_ = new VoreenPainter(this, new tgt::Trackball(this, false), "ov1");

    or_ = new OverviewRenderer(true, tgt::Color(0.2f, 0.2f, 0.2f , 0.6f), tgt::Color(0.4f, 0.4f, 0.4f, 1.f));
    or_->setPropertyDestination("PrimaryRaycaster");
    if (tc && camera) {
        CopyToScreenRenderer* toScreen = new CopyToScreenRenderer(camera, tc);
        toScreen->setCaching(false);
        toScreen->setIgnoreCoarseness(true);
        toScreen->setRenderToScreen(true);
        //RPTMERGE
        //((VoreenPainter*)painter_)->setProcessor(toScreen);
    }
    else {
        //RPTMERGE
        //((VoreenPainter*)painter_)->setProcessor(or_);
    }

    MsgDistr.insert(this);
    eventHandler_->addListenerToBack(this);
    if (eventListener)
        eventHandler_->addListenerToFront(eventListener);
}

OverViewWidget::~OverViewWidget() {
    delete or_;
    delete painter_;

    MsgDistr.remove(this);
}

void OverViewWidget::processMessage(Message* msg, const Identifier& dest/*=Identifier::all*/) {
    MessageReceiver::processMessage(msg, dest);
    ((VoreenPainter*)painter_)->processMessage(msg, dest);

    // perhaps more to come...
}

void OverViewWidget::mousePressEvent(tgt::MouseEvent* e) {
    if (e->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT) {
        leftPressed_ = true;
        oldMouseX_ = e->x();
        oldMouseY_ = e->y();
        or_->startDragging(false, 0.1f);
        e->accept();
    }
    else if (e->button() == tgt::MouseEvent::MOUSE_BUTTON_RIGHT) {
        rightPressed_ = true;
        oldMouseX_ = e->x();
        oldMouseY_ = e->y();
        e->accept();
    }
}

void OverViewWidget::mouseReleaseEvent(tgt::MouseEvent* e) {
    if (e->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT) {
        leftPressed_ = false;
        or_->stopDragging();
        //or_->snapToGrid();
        updateGL();
        e->accept();
    }
    else if (e->button() == tgt::MouseEvent::MOUSE_BUTTON_RIGHT) {
        rightPressed_ = false;
        updateGL();
        e->accept();
    }
}

void OverViewWidget::mouseMoveEvent(tgt::MouseEvent* e)  {
    if(leftPressed_) {
        or_->addTranslation(tgt::vec2( e->x()-oldMouseX_, -(e->y()-oldMouseY_)), true);

        oldMouseX_ = e->x();
        oldMouseY_ = e->y();

        updateGL();

        e->accept();
    }
    else if (rightPressed_) {

        float ZOOM_SCALE = 0.1f;

        int v_offset = int(e->y() - oldMouseY_);
        float timesScale = std::floor( abs(v_offset)*ZOOM_SCALE + 0.5f);
        if (timesScale > 0.f) {

            float base = (v_offset > 0 ? 1.1f : 1.0f/1.1f);
            float scaleFactor = static_cast<float>(
                pow( static_cast<double>(base), static_cast<double>(timesScale) ) );
            or_->scale(scaleFactor);
            oldMouseX_ = e->x();
            oldMouseY_ = e->y();
            updateGL();
        }

        e->accept();

    }
}

void OverViewWidget::mouseDoubleClickEvent(tgt::MouseEvent* e) {

    if (e->button() == tgt::MouseEvent::MOUSE_BUTTON_RIGHT) {
        or_->showOverview();
    } else if (e->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT) {
        or_->showSlice(tgt::ivec2(e->x(), e->y()));
    }

    updateGL();

}

void OverViewWidget::wheelEvent(tgt::MouseEvent* e) {

    if (e->button() == tgt::MouseEvent::MOUSE_WHEEL_DOWN)
        or_->scale( 1.1f );
    else
        or_->scale( 1.f/1.1f );

    //or_->snapToGrid();
    updateGL();
}

} // namespace voreen

