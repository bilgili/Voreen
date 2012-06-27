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

#include "voreen/qt/widgets/canvasmodifier.h"
#include "tgt/logmanager.h"

namespace voreen {

CanvasModifier::CanvasModifier(tgt::GLCanvas* canvas, bool useSignals) : useSignals_(useSignals) {
    canvas_ = canvas;
    canvasDetached_ = false;
    canvasWidget_ = 0;
}

void CanvasModifier::keyEvent(tgt::KeyEvent* e) {
    tgt::QtCanvas* qtCanvas = dynamic_cast<tgt::QtCanvas*>(canvas_);
    if (!e->pressed() && !qtCanvas) {
        e->ignore();
        return;
    }

    bool printSize = false;
    if (keyPrint_ == e->keyCode() && e->pressed()) {
        printSize = true;
    }
    if (e->keyCode() == tgt::KeyEvent::K_0) {
        qtCanvas->resize(64, 64);
        printSize = true;
    }
    else if (e->keyCode() == tgt::KeyEvent::K_1) {
        qtCanvas->resize(128, 128);
        printSize = true;
    }
    else if (e->keyCode() == tgt::KeyEvent::K_2) {
        qtCanvas->resize(256, 256);
        printSize = true;
    }
    else if (e->keyCode() == tgt::KeyEvent::K_3) {
        qtCanvas->resize(300, 300);
        printSize = true;
    }
    else if (e->keyCode() == tgt::KeyEvent::K_4) {
        qtCanvas->resize(400, 400);
        printSize = true;
    }
    else if (e->keyCode() == tgt::KeyEvent::K_5) {
        qtCanvas->resize(512, 512);
        printSize = true;
    }
    else if (e->keyCode() == tgt::KeyEvent::K_6) {
        qtCanvas->resize(600, 600);
        printSize = true;
    }
    else if (e->keyCode() == tgt::KeyEvent::K_7) {
        qtCanvas->resize(1024, 1024);
        printSize = true;
    }
    else if (keyShowWidget_ == e->keyCode() && e->pressed()) {
        if (!canvasDetached_) {
            canvasWidget_ = dynamic_cast<QWidget*>(qtCanvas->parent());
            if (useSignals_)
                emit detachFromTab();
            else {
                qtCanvas->setParent(0);
                qtCanvas->show();
            }
        } else {
            if (useSignals_)
                emit attachToTab();
            else {
                qtCanvas->setParent(canvasWidget_);
                qtCanvas->show();
            }
        }
        canvasDetached_ = !canvasDetached_;
    } else
        e->ignore();

    if (printSize)
        LINFOC("voreen.qt.widgets.CanvasModifier",
               "Canvas size: " << canvas_->getWidth() << "x" << canvas_->getHeight());
}

void CanvasModifier::connect() {
    canvas_->getEventHandler()->addListenerToFront(this);
}

void CanvasModifier::disconnect() {
    canvas_->getEventHandler()->removeListener(this);
}

} // namespace
