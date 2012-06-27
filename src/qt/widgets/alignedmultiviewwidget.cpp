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

#include "voreen/qt/widgets/alignedmultiviewwidget.h"

#include <QGridLayout>

#include "tgt/camera.h"

#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

AlignedMultiViewWidget::AlignedMultiViewWidget(tgt::Camera* camera3D,
                                               tgt::EventListener* eventListener,
                                               QWidget* parent)
    : QWidget(parent)
{
    // the iniliziation order here is critical!!!
    Processor* processor = 0;//factory.generate(rendererType, true);
    canvases_[0] = new AMV3dView(this, processor, camera3D);
    //RPTMERGE
    //((VoreenPainter*)(canvases_[0]->getPainter()))->setProcessor(processor);

    alignedMultiView_ = new AlignedMultiView(canvases_, processor);

    for (size_t i = 1; i < 4; ++i) {
        canvases_[i] =
            new AMVSliceView(
                this,
                alignedMultiView_->getProcessor(i),
                (AMV3dView*)canvases_[0],
                alignedMultiView_,
                i - 1
            );
    }

    for (int i=0; i<4; i++) {
        canvases_[i]->getEventHandler()->addListenerToFront(eventListener);
    }

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->addWidget((AMV3dView*)    canvases_[0],            0, 0);
    gridLayout->addWidget((AMVSliceView*) canvases_[TOP_RIGHT],    0, 1);
    gridLayout->addWidget((AMVSliceView*) canvases_[BOTTOM_LEFT],  1, 0);
    gridLayout->addWidget((AMVSliceView*) canvases_[BOTTOM_RIGHT], 1, 1);

    setLayout(gridLayout);
}

AlignedMultiViewWidget::~AlignedMultiViewWidget() {
    MsgDistr.remove((AMV3dView*)canvases_[0]);

    for (size_t i = 1; i < 4; ++i)
        MsgDistr.remove((AMVSliceView*)canvases_[i]);

    delete alignedMultiView_;
}

//------------------------------------------------------------------------------

void AlignedMultiViewWidget::AMV3dView::processMessage(Message* msg, const Identifier& dest/*=Identifier::all*/) {
    MessageReceiver::processMessage(msg, dest);

    if (msg->id_ == "sliceChanged") {
        repaint();
    }
    else
        ((VoreenPainter*)painter_)->processMessage(msg, dest);
}

//------------------------------------------------------------------------------

void AlignedMultiViewWidget::AMVSliceView::processMessage(Message* msg, const Identifier& dest/*=Identifier::all*/) {
    MessageReceiver::processMessage(msg, dest);

    if (msg->id_ == "endPainting") {
        tgtAssert( typeid(*msg) == typeid(GLCanvasPtrMsg), "wrong message type used" );
        // do only an update if this message was sent from the amv3dView_
        if ( ((GLCanvasPtrMsg*) msg)->value_ == amv3dView_ ) {
            repaint();
        }
    }
    else if (msg->id_ == "sliceChanged") {
        tgtAssert( typeid(*msg) == typeid(SizeTMsg), "wrong message type used" );
        if (index_ == ((SizeTMsg*) msg)->value_) {
            repaint();
        }
    }
    else
        ((VoreenPainter*)painter_)->processMessage(msg, dest);
}

void AlignedMultiViewWidget::AMVSliceView::mousePressEvent(tgt::MouseEvent* e) {
    if (e->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT) {
        leftPressed_ = true;
        oldMouseY_ = e->y();
    }
}

void AlignedMultiViewWidget::AMVSliceView::mouseReleaseEvent(tgt::MouseEvent* e) {
    if (e->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT)
        leftPressed_ = false;
}

void AlignedMultiViewWidget::AMVSliceView::mouseMoveEvent(tgt::MouseEvent* e)  {
    if(leftPressed_) {
        if (e->y() - oldMouseY_ < 0)
            alignedMultiView_->inc4Slice(index_);
        else
            alignedMultiView_->dec4Slice(index_);

        oldMouseY_ = e->y();

        MsgDistr.postMessage( new SizeTMsg("sliceChanged", index_), "amv1");
    }
}

void AlignedMultiViewWidget::AMVSliceView::mouseDoubleClickEvent(tgt::MouseEvent* /*e*/) {
    if ( ((AMV3dView*) ((AlignedMultiViewWidget*) parent())->canvases_[0])->isVisible() ) {
        // set all other canveses invisible
        ((AMV3dView*) ((AlignedMultiViewWidget*) parent())->canvases_[0])->setVisible(false);
        for (size_t i = 1; i < 4; ++i) {
            if (i == index_ + 1)
                continue;
            ((AMVSliceView*) ((AlignedMultiViewWidget*) parent())->canvases_[i])->setVisible(false);
        }
    }
    else {
        // set all other canveses visible
        ((AMV3dView*) ((AlignedMultiViewWidget*) parent())->canvases_[0])->setVisible(true);
        for (size_t i = 1; i < 4; ++i) {
            if (i == index_ + 1)
                continue;
            ((AMVSliceView*) ((AlignedMultiViewWidget*) parent())->canvases_[i])->setVisible(true);
        }
    }
}

void AlignedMultiViewWidget::AMVSliceView::wheelEvent(tgt::MouseEvent* e) {
    if (e->button() == tgt::MouseEvent::MOUSE_WHEEL_DOWN)
        alignedMultiView_->decSlice(index_);
    else
        alignedMultiView_->incSlice(index_);

    MsgDistr.postMessage( new SizeTMsg("sliceChanged", index_), "amv1");
}

} // namespace voreen
