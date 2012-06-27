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

#include "voreen/core/vis/transfunc/transfunceditor.h"

namespace voreen {

TransFuncEditor::TransFuncEditor(MessageReceiver* msgReceiver, Identifier target, Identifier id)
: target_(target), id_(id), receiver_(msgReceiver) {
}

TransFuncEditor::~TransFuncEditor() {
}

void TransFuncEditor::transFuncChanged() {
}

void TransFuncEditor::setTarget(Identifier target) {
    target_ = target;
}

Identifier TransFuncEditor::getTarget() {
    return target_;
}

void TransFuncEditor::setIdentifier(Identifier id) {
    id_ = id;
}

Identifier TransFuncEditor::getIdentifier() {
    return id_;
}

void TransFuncEditor::sendTFMessage() {
    receiver_->postMessage(new TransFuncPtrMsg(id_, getTransFunc()), target_);
    MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
	//receiver_->postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

void TransFuncEditor::setReceiver(MessageReceiver* receiver) {
    receiver_ = receiver;
}

void TransFuncEditor::setInteractionCoarseness(bool interactionCoarseness) {
    /*receiver_->postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, ic), VoreenPainter::visibleViews_);
    receiver_->postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);*/
	MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, interactionCoarseness), VoreenPainter::visibleViews_);
	MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

void TransFuncEditor::selected() {
}

void TransFuncEditor::deselected() {
}

}
