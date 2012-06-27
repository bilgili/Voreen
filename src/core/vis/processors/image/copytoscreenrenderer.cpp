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

#include "voreen/core/vis/processors/image/copytoscreenrenderer.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/processors/networkevaluator.h"

namespace voreen {

CopyToScreenRenderer::CopyToScreenRenderer(tgt::Camera* camera, TextureContainer* tc)
    : Processor(camera, tc)
    , ignoreCoarseness_(false)
    , useCoarseness_(VoreenPainter::switchCoarseness_, "Use Interaction Coarseness", false)
    , coarsenessFactor_(setCoarseness_, "Interaction Coarseness", 4, 1, 20)
    , raycastPrg_(0)
    , caching_(true)
    , cachedImage_(-1)
    , cachedStereoImageLeft_(-1)
    , cachedStereoImageRight_(-1)
{
    setName("CopyToScreenRenderer");
    addProperty(&coarsenessFactor_);
    addProperty(&useCoarseness_);
    coarsenessFactor_.setAutoChange(true);
}

CopyToScreenRenderer::~CopyToScreenRenderer() {
}

const std::string CopyToScreenRenderer::getProcessorInfo() const {
    return "A CopyToScreenRenderer is the root node of the rendering network. Its main purpose is to "
           "render (copy) its producer's rendering result to screen. Additionally it is able to cache "
           "the producer's rendering, if no parameters regarding any element of the producer have been "
           "changed since last <i>render()</i> call. An object of this class can also be used within the "
           "network in order to cache a network's fork. The usage of multiple CopyToScreenRenderer objects "
           "in the same network is possible. A network element (Renderer object) has to indicate a change "
           "of its rendering parameters that necessiates a new rendering by calling <i>invalidate()</i>." ;
}

int CopyToScreenRenderer::initializeGL() {
    initStatus_ = Processor::initializeGL();
    if (initStatus_ != VRN_OK)
        return initStatus_;

    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "vrn_interactionmode.frag",
        generateHeader(), false);

    if (!raycastPrg_)
        initStatus_ = VRN_ERROR;

    return initStatus_;
}

void CopyToScreenRenderer::processMessage(Message* msg, const Identifier& dest) {
    Processor::processMessage(msg, dest);
    if (msg->id_ == VoreenPainter::switchCoarseness_)
    {
        if (msg->getValue<bool>() != useCoarseness_.get()) {
            useCoarseness_.set(msg->getValue<bool>());
            MsgDistr.postMessage(new ProcessorPointerMsg(NetworkEvaluator::unsetCachedBackward_,this),"evaluator");
            invalidate();
        }
    }
    else if (msg->id_ == setCoarseness_)
        coarsenessFactor_.set(msg->getValue<int>());
}

void CopyToScreenRenderer::setCaching(bool caching) {
    caching_ = caching;
}

void CopyToScreenRenderer::setIgnoreCoarseness(bool ignoreCoarseness) {
    ignoreCoarseness_ = ignoreCoarseness;
}

void CopyToScreenRenderer::invalidate() {
    Processor::invalidate();

    // no distinction between stereo/non-stereo mode is done here because it is not clear whether
    // the mode is the same as at the last render() call.
    if (cachedImage_ != -1)
        cachedImage_ = -1;
    if (cachedStereoImageRight_ != -1)
        cachedStereoImageRight_ = -1;
    if (cachedStereoImageLeft_ != -1)
        cachedStereoImageLeft_ = -1;
}

} // namespace voreen
