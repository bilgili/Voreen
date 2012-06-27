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

#include "voreen/qt/widgets/splitviewwidget.h"

#include <QGridLayout>

#include "tgt/camera.h"

#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

SplitViewWidget::SplitViewWidget(tgt::Camera* camera3D,
                                 tgt::EventListener* eventListener,
                                 QWidget* parent)
    : QWidget(parent)
{
     // the iniliziation order here is critical!!!
    Processor* processor = 0;//factory.generate(rendererType, true);

    canvases_[0] = new Split3dView(this, processor, camera3D);
    //RPTMERGE
    //((VoreenPainter*)(canvases_[0]->getPainter()))->setProcessor(processor);

    splitView_ = new SplitView(canvases_, processor);
    ((Split3dView*) canvases_[0])->setSplitView(splitView_);

    canvases_[1] =
        new SplitEmphasizedView(
            this,                       // parent
            splitView_->getProcessor(1), // renderer
            (Split3dView*) canvases_[0] // split3dView
        );
    //RPTMERGE
    //((VoreenPainter*)(canvases_[1]->getPainter()))->setProcessor(splitView_->getProcessor(1));

    for (int i=0; i<2; i++) {
        canvases_[i]->getEventHandler()->addListenerToFront(eventListener);
    }

    QHBoxLayout* hBox = new QHBoxLayout();

    hBox->addWidget((Split3dView*) canvases_[0]);
    hBox->addWidget((SplitEmphasizedView*) canvases_[1]);

    setLayout(hBox);
}

SplitViewWidget::~SplitViewWidget() {
    MsgDistr.remove((Split3dView*)canvases_[0]);
    MsgDistr.remove((SplitEmphasizedView*)canvases_[1]);
    delete splitView_;
}

//------------------------------------------------------------------------------

void SplitViewWidget::Split3dView::mouseMoveEvent(tgt::MouseEvent* e) {
    tgt::vec2 mouseCoords = tgt::vec2(e->x(), e->y());

    // map to [-1,1] and convert to GL
    mouseCoords.x /= width();
    mouseCoords.y /= height();
    mouseCoords *= 2.f;
    mouseCoords -= 1.f;
    mouseCoords.y = -mouseCoords.y;

    splitView_->setMouseCoords(mouseCoords);
    splitView_->update();

    updateGL();

    MsgDistr.postMessage( new Message("splitViewUpdate"), "splitview_1");
    e->ignore();
//     VoreenGLCanvas::tgtMoveEvent(e);
}

//------------------------------------------------------------------------------

void SplitViewWidget::SplitEmphasizedView::processMessage(Message* msg, const Identifier& dest/*=Identifier::all*/) {
    MessageReceiver::processMessage(msg, dest);

    if (msg->id_ == "splitViewUpdate") {
        updateGL();
    }
    else if (msg->id_ == "endPainting") {
        tgtAssert( typeid(*msg) == typeid(GLCanvasPtrMsg), "wrong message type used" );
        // do only an update if this message was sent from split3dView_
        if ( ((GLCanvasPtrMsg*) msg)->value_ == split3dView_ ) {
            updateGL();
        }
    }
    else
        ((VoreenPainter*)painter_)->processMessage(msg, dest);
}
} // namespace voreen

