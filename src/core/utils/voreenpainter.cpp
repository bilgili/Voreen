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

#include "voreen/core/utils/voreenpainter.h"

#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/network/processornetwork.h"

#include "modules/core/processors/output/canvasrenderer.h" //< core module is always available

namespace voreen {

const std::string VoreenPainter::loggerCat_ = "voreen.core.VoreenPainer";

VoreenPainter::VoreenPainter(tgt::GLCanvas* canvas, NetworkEvaluator* evaluator, CanvasRenderer* canvasRenderer)
    : tgt::Painter(canvas)
    , evaluator_(evaluator)
    , canvasRenderer_(canvasRenderer)
{
    tgtAssert(canvas, "No canvas");
    tgtAssert(evaluator_, "No network evaluator");
    tgtAssert(canvasRenderer_, "No canvas renderer");
}

VoreenPainter::~VoreenPainter() {
}

void VoreenPainter::initialize() {

    if (getCanvas()) {
        getCanvas()->getGLFocus();
    }
    else {
        LWARNING("initialize(): No canvas");
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

}

void VoreenPainter::sizeChanged(const tgt::ivec2& size) {

    getCanvas()->getGLFocus();

    tgt::ivec2 validSize = size;
    validSize.y = validSize.y ? validSize.y : 1;

    // setup viewport, projection etc.:
    glViewport(0, 0, static_cast<GLint>(validSize.x), static_cast<GLint>(validSize.y));

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.loadIdentity();

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.loadIdentity();

    tgtAssert(canvasRenderer_, "No canvas renderer");
    canvasRenderer_->canvasResized(size);
}

void VoreenPainter::paint() {
    if (!getCanvas()) {
        LWARNING("No canvas assigned");
        return;
    }

    if (!evaluator_) {
        LWARNING("No network evaluator assigned");
        return;
    }
//         glDrawBuffer(GL_BACK_LEFT);
// //         camera->setStereo(true); <- TODO doesn't exist anymore, FL
//         camera->setEye(tgt::Camera::EYE_LEFT);
//         glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//         if (evaluator_)
//             evaluator_->render();
//         if (overlayMgr_)
//             overlayMgr_->paint();
//
//         // we must invalidate here or nothing will happen when we render the other eye-perspective
//         evaluator_->invalidate();
//
//         glDrawBuffer(GL_BACK_RIGHT);
//         camera->setEye(tgt::Camera::EYE_RIGHT);
//         glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//         if (evaluator_)
//             evaluator_->render();
//         if (overlayMgr_)
//             overlayMgr_->paint();
//
//         glDrawBuffer(GL_BACK);
// //         camera->setStereo(false); // doesn't exist anymore, FL
//         camera->setEye(tgt::Camera::EYE_MIDDLE);

    if (!evaluator_->isLocked())
        getCanvas()->getGLFocus();
    evaluator_->process();

}

void VoreenPainter::repaint() {
    getCanvas()->repaint();
}

void VoreenPainter::renderToSnapshot(const std::string& fileName, const tgt::ivec2& size)
    throw (std::bad_alloc, tgt::FileException) {

    tgtAssert(canvasRenderer_, "No canvas renderer");
    canvasRenderer_->renderToImage(fileName, size);

}

NetworkEvaluator* VoreenPainter::getEvaluator() const {
    return evaluator_;
}

CanvasRenderer* VoreenPainter::getCanvasRenderer() const {
    return canvasRenderer_;
}

} // namespace
