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

#include "voreen/qt/widgets/multiviewwidget.h"

#include <QGridLayout>

#include "tgt/assert.h"
#include "tgt/glmath.h"

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/vis/processors/networkserializer.h"
#include "voreen/core/vis/processors/processorfactory.h"

namespace voreen {

MultiViewWidget::MultiViewWidget(tgt::Camera* camera3D, tgt::EventListener* eventListener, VolumeContainer* volumeContainer, QWidget* parent, bool multimodal)
    : QWidget(parent)
{
    // the iniliziation order here is critical!!!
    canvases_[0] = new MV3dView(this, camera3D);
    canvases_[0]->getGLFocus();
    //Renderer* renderer = factory.generate(rendererType, true);
    RptNetworkSerializer* serializer = new RptNetworkSerializer();
    ProcessorNetwork net = serializer->readNetworkFromFile("standard.svnf");
    RptNetworkEvaluator* eval = new RptNetworkEvaluator();
    MsgDistr.insert(eval);
    eval->setTextureContainer(ProcessorFactory::getInstance()->getTextureContainer());
    eval->setVolumeContainer(volumeContainer);
    for (size_t i=0;i<net.processors.size(); ++i) {
        net.processors.at(i)->setCamera(camera3D);
    }
    eval->setProcessors(net.processors);
    if (eval->analyze() >= 0) {
        ((VoreenPainter*)(canvases_[0]->getPainter()))->setEvaluator(eval);
    }

    multiView_ = new MultiView(eval,volumeContainer, false/*multimodal*/);
    MsgDistr.insert(multiView_);

    tgt::Color borderColors[3];
    borderColors[0] = tgt::vec4(1.f,0.f,0.f,1.f);
    borderColors[1] = tgt::vec4(0.f,1.f,0.f,1.f);
    borderColors[2] = tgt::vec4(0.f,0.f,1.f,1.f);

    for (size_t i = 1; i < 4; ++i) {
        canvases_[i] = new MVSliceView(this, multiView_->getEvaluator(i), multiView_, i-1,
            borderColors[i-1], 6);
        canvases_[i]->getGLFocus();
    }

    for (int i=0; i<4; i++) {
        if (eventListener)
            canvases_[i]->getEventHandler()->addListenerToFront(eventListener);
    }

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->addWidget((MV3dView*)    canvases_[0],            0, 0);
    gridLayout->addWidget((MVSliceView*) canvases_[TOP_RIGHT],    0, 1);
    gridLayout->addWidget((MVSliceView*) canvases_[BOTTOM_LEFT],  1, 0);
    gridLayout->addWidget((MVSliceView*) canvases_[BOTTOM_RIGHT], 1, 1);

    setLayout(gridLayout);
}

MultiViewWidget::~MultiViewWidget() {
    for (size_t i = 0; i < 4; ++i)
        delete canvases_[i];

    MsgDistr.remove(multiView_);
    delete multiView_;
}

//------------------------------------------------------------------------------

void MultiViewWidget::MV3dView::processMessage(Message* msg, const Identifier& dest/*=Identifier::all*/) {
    MessageReceiver::processMessage(msg, dest);

    if (msg->id_ == "sliceChanged")
        repaint();
    else
        ((VoreenPainter*)painter_)->processMessage(msg, dest);
}

//------------------------------------------------------------------------------

void MultiViewWidget::MVSliceView::processMessage(Message* msg, const Identifier& dest/*=Identifier::all*/) {
    MessageReceiver::processMessage(msg, dest);

    if (msg->id_ == "sliceChanged") {
        tgtAssert( typeid(*msg) == typeid(SizeTMsg), "wrong message type used" );
        if (index_ == ((SizeTMsg*) msg)->value_) {
            repaint();
        }
    }
    else
        ((VoreenPainter*)painter_)->processMessage(msg, dest);
}

void MultiViewWidget::MVSliceView::mousePressEvent(tgt::MouseEvent* e) {
    if (e->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT) {
        leftPressed_ = true;
        oldMouseY_ = e->y();
    }
}

void MultiViewWidget::MVSliceView::mouseReleaseEvent(tgt::MouseEvent* e) {
    if (e->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT)
        leftPressed_ = false;
}

void MultiViewWidget::MVSliceView::mouseMoveEvent(tgt::MouseEvent* e)  {
    if(leftPressed_) {
        bool accepted = false;
        if (e->y() - oldMouseY_ < -4) {
            multiView_->inc4Slice(index_);
            accepted = true;
        }
        else if (e->y() - oldMouseY_ > 4) {
            multiView_->dec4Slice(index_);
            accepted = true;
        }

        if (accepted) {
            oldMouseY_ = e->y();
            MsgDistr.postMessage( new SizeTMsg("sliceChanged", index_), "mv1");
        }
    }
}

void MultiViewWidget::MVSliceView::mouseDoubleClickEvent(tgt::MouseEvent* /*e*/) {
    if ( ((MV3dView*) ((MultiViewWidget*) parent())->canvases_[0])->isVisible() ) {
        // set all other canveses invisible
        ((MV3dView*) ((MultiViewWidget*) parent())->canvases_[0])->setVisible(false);
        for (size_t i = 1; i < 4; ++i) {
            if (i == index_ + 1)
                continue;
            ((MVSliceView*) ((MultiViewWidget*) parent())->canvases_[i])->setVisible(false);
        }
    }
    else {
        // set all other canveses visible
        ((MV3dView*) ((MultiViewWidget*) parent())->canvases_[0])->setVisible(true);
        for (size_t i = 1; i < 4; ++i) {
            if (i == index_ + 1)
                continue;
            ((MVSliceView*) ((MultiViewWidget*) parent())->canvases_[i])->setVisible(true);
        }
    }
}

void MultiViewWidget::MVSliceView::wheelEvent(tgt::MouseEvent* e) {
    if (e->button() == tgt::MouseEvent::MOUSE_WHEEL_DOWN)
        multiView_->decSlice(index_);
    else
        multiView_->incSlice(index_);

    MsgDistr.postMessage( new SizeTMsg("sliceChanged", index_), "mv1");
}

} // namespace voreen

