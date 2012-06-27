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

#include "voreen/qt/widgets/stackviewwidget.h"

#include <QGridLayout>

#include "tgt/camera.h"

#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

StackViewWidget::StackViewWidget(tgt::Camera* camera3D, tgt::EventListener* eventListener, QWidget* parent)
    : QWidget(parent)
{
     // the iniliziation order here is critical!!!
    Processor* processor = 0;//factory.generate(rendererType, true);

    canvases_[0] = new Stack3dView(this, processor, camera3D);
    //RPTMERGE
    //((VoreenPainter*)(canvases_[0]->getPainter()))->setProcessor(processor);

    stackView_ = new StackView(canvases_, processor);
    ((Stack3dView*) canvases_[0])->setStackView(stackView_);

//     ((VoreenPainter*)(canvases_[0]->getPainter()))->setProcessor(stackView_->getProcess0r(0));
//     MsgDistr.insert((Stack3dView*)canvases_[0] );

    for (size_t i = 1; i < 8; ++i) {
        canvases_[i] =
            new StackSliceView(
                this,
                stackView_->getProcessor(i),
                (Stack3dView*) canvases_[0]
            );
         //RPTMERGE
        //((VoreenPainter*)(canvases_[i]->getPainter()))->setProcessor(stackView_->getProcessor(i));
    }

    for (int i=0; i<8; i++) {
        canvases_[i]->getEventHandler()->addListenerToFront(eventListener);
    }

    QHBoxLayout* mainHBox;
    QHBoxLayout* tHBox;
    QHBoxLayout* mHBox;
    QHBoxLayout* bHBox;
    QVBoxLayout* lVBox;
    QVBoxLayout* rVBox;

    mainHBox = new QHBoxLayout();
    lVBox = new QVBoxLayout();
    rVBox = new QVBoxLayout();
    tHBox = new QHBoxLayout();
    mHBox = new QHBoxLayout();
    bHBox = new QHBoxLayout();

    lVBox->addWidget((Stack3dView*) canvases_[0]);
    lVBox->addWidget((StackSliceView*) canvases_[4]);

    tHBox->addWidget((StackSliceView*) canvases_[1]);
    tHBox->addWidget((StackSliceView*) canvases_[5]);

    mHBox->addWidget((StackSliceView*) canvases_[2]);
    mHBox->addWidget((StackSliceView*) canvases_[6]);

    bHBox->addWidget((StackSliceView*) canvases_[3]);
    bHBox->addWidget((StackSliceView*) canvases_[7]);

    rVBox->addLayout(tHBox);
    rVBox->addLayout(mHBox);
    rVBox->addLayout(bHBox);

    mainHBox->addLayout(lVBox);
    mainHBox->addLayout(rVBox);

    setLayout(mainHBox);
}

StackViewWidget::~StackViewWidget() {
    for (size_t i = 1; i < 8; ++i)
        MsgDistr.remove((StackSliceView*)canvases_[i] );

    delete stackView_;
}

//------------------------------------------------------------------------------

void StackViewWidget::Stack3dView::mouseMoveEvent(tgt::MouseEvent* e) {
    tgt::vec2 mouseCoords = tgt::vec2(e->x(), e->y());

    // map to [-1,1]
    mouseCoords.x /= width();
    mouseCoords.y /= height();
    mouseCoords *= 2.f;
    mouseCoords -= 1.f;

    stackView_->setMouseCoords(mouseCoords);
    stackView_->update();

    updateGL();

    MsgDistr.postMessage( new Message("stackViewUpdate"), "sv1");
    e->ignore();
}

//------------------------------------------------------------------------------

void StackViewWidget::StackSliceView::processMessage(Message* msg, const Identifier& dest/*=Identifier::all*/) {
    MessageReceiver::processMessage(msg, dest);

    if (msg->id_ == "stackViewUpdate") {
        makeCurrent();
        updateGL();
    }
    else if (msg->id_ == "endPainting") {
        tgtAssert( typeid(*msg) == typeid(GLCanvasPtrMsg), "wrong message type used" );
        // do only an update if this message was sent from stack3dView_
        if ( ((GLCanvasPtrMsg*) msg)->value_ == stack3dView_ ) {
            makeCurrent();
            updateGL();
        }
    }
    else
        ((VoreenPainter*)painter_)->processMessage(msg, dest);
}
} // namespace voreen

