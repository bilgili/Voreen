/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/modules/base/processors/utility/multiscale.h"

namespace voreen {

MultiScale::MultiScale()
    : ScalingProcessor(),
      scalingMode1_("scalingMode1", "Scaling Mode 1", Processor::INVALID_RESULT),
      scalingMode2_("scalingMode2", "Scaling Mode 2", Processor::INVALID_RESULT),
      scalingMode3_("scalingMode3", "Scaling Mode 3", Processor::INVALID_RESULT),
      scalingMode4_("scalingMode4", "Scaling Mode 4", Processor::INVALID_RESULT),
      selectionMode_("selectionMode", "Selection Mode", Processor::INVALID_RESULT),
      outport1_(Port::OUTPORT, "image.outport1"),
      outport2_(Port::OUTPORT, "image.outport2"),
      outport3_(Port::OUTPORT, "image.outport3"),
      outport4_(Port::OUTPORT, "image.outport4")
{

    scalingMode1_.addOption("brute-force",     "Brute force",            0);
    scalingMode1_.addOption("1:1",             "Pixels 1:1",             1);
    scalingMode1_.addOption("keep-ratio-all",  "Keep ratio, view all",   2);
    scalingMode1_.addOption("keep-ratio-fill", "Keep ratio, fill",       3);
    scalingMode1_.addOption("scale-to-height", "Scale to height",        4);
    scalingMode1_.addOption("scale-to-width",  "Scale to width",        5);
    scalingMode1_.select("keep-ratio-all");
    addProperty(scalingMode1_);

    scalingMode2_.addOption("brute-force",     "Brute force",            0);
    scalingMode2_.addOption("1:1",             "Pixels 1:1",             1);
    scalingMode2_.addOption("keep-ratio-all",  "Keep ratio, view all",   2);
    scalingMode2_.addOption("keep-ratio-fill", "Keep ratio, fill",       3);
    scalingMode2_.addOption("scale-to-height", "Scale to height",        4);
    scalingMode2_.addOption("scale-to-width",  "Scale to width",        5);
    scalingMode2_.select("keep-ratio-all");
    addProperty(scalingMode2_);

    scalingMode3_.addOption("brute-force",     "Brute force",            0);
    scalingMode3_.addOption("1:1",             "Pixels 1:1",             1);
    scalingMode3_.addOption("keep-ratio-all",  "Keep ratio, view all",   2);
    scalingMode3_.addOption("keep-ratio-fill", "Keep ratio, fill",       3);
    scalingMode3_.addOption("scale-to-height", "Scale to height",        4);
    scalingMode3_.addOption("scale-to-width",  "Scale to width",        5);
    scalingMode3_.select("keep-ratio-all");
    addProperty(scalingMode3_);

    scalingMode4_.addOption("brute-force",     "Brute force",            0);
    scalingMode4_.addOption("1:1",             "Pixels 1:1",             1);
    scalingMode4_.addOption("keep-ratio-all",  "Keep ratio, view all",   2);
    scalingMode4_.addOption("keep-ratio-fill", "Keep ratio, fill",       3);
    scalingMode4_.addOption("scale-to-height", "Scale to height",        4);
    scalingMode4_.addOption("scale-to-width",  "Scale to width",        5);
    scalingMode4_.select("keep-ratio-all");
    addProperty(scalingMode4_);

    selectionMode_.addOption("size",           "Size");
    selectionMode_.addOption("aspect-ratio",  "Aspect ratio");
    selectionMode_.addOption("width",          "Width");
    selectionMode_.addOption("height",         "Height");
    addProperty(selectionMode_);

    addPort(outport1_);
    addPort(outport2_);
    addPort(outport3_);
    addPort(outport4_);
}

MultiScale::~MultiScale() {
}

std::string MultiScale::getProcessorInfo() const {
    return "Selects the optimal size for previous renderers from all connected outports and resizes the output for other outports. This processor is only needed when working with multiple canvases of independant size.";
}

void MultiScale::initialize() throw (VoreenException) {
    inport_.resize(selectBest());
    ScalingProcessor::initialize();
    initialized_ = true;
}

bool MultiScale::isReady() const {
    if (!inport_.isReady() || (!outport1_.isReady() && !outport2_.isReady() && !outport3_.isReady() && !outport4_.isReady()))
        return false;

    return true;
}

void MultiScale::process() {
    inport_.bindTextures(GL_TEXTURE0, GL_TEXTURE1);
    LGL_ERROR;

    // initialize shader
    program_->activate();
    program_->setUniform("shadeTex_", 0);
    program_->setUniform("depthTex_", 1);
    program_->setIgnoreUniformLocationError(true);
    program_->setUniform("shadeTexParameters_.dimensions_", tgt::vec2(inport_.getSize()));
    program_->setUniform("shadeTexParameters_.dimensionsRCP_", tgt::vec2(1.0f) / tgt::vec2(inport_.getSize()));
    program_->setUniform("depthTexParameters_.dimensions_", tgt::vec2(inport_.getSize()));
    program_->setUniform("depthTexParameters_.dimensionsRCP_", tgt::vec2(1.0f) / tgt::vec2(inport_.getSize()));
    program_->setIgnoreUniformLocationError(false);

    glMatrixMode(GL_MODELVIEW);

    glDepthFunc(GL_ALWAYS);
    if (outport1_.isReady()) {
        outport1_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        applyScalingMatrix(scalingMode1_.getValue(), &inport_, &outport1_);
        renderQuad();
        outport1_.deactivateTarget();
    }
    if (outport2_.isReady()) {
        outport2_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        applyScalingMatrix(scalingMode2_.getValue(), &inport_, &outport2_);
        renderQuad();
        outport2_.deactivateTarget();
    }
    if (outport3_.isReady()) {
        outport3_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        applyScalingMatrix(scalingMode3_.getValue(), &inport_, &outport3_);
        renderQuad();
        outport3_.deactivateTarget();
    }
    if (outport4_.isReady()) {
        outport4_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        applyScalingMatrix(scalingMode4_.getValue(), &inport_, &outport4_);
        renderQuad();
        outport4_.deactivateTarget();
    }
    glDepthFunc(GL_LESS);

    glLoadIdentity();

    program_->deactivate();
    glActiveTexture(GL_TEXTURE0);
    LGL_ERROR;
}

void MultiScale::textureContainerChanged(RenderPort* /*p*/) {
    //TODO: tc_
    //portResized(p);
}

tgt::ivec2 MultiScale::selectBest() {
    if (!outport1_.isConnected() && !outport2_.isConnected() && !outport3_.isConnected() && !outport4_.isConnected())
        return tgt::ivec2(128,128);

    tgt::ivec2 best(0,0);

    if (selectionMode_.get() == "size") {
        if (outport1_.isConnected() && (hmul(best) < hmul(outport1_.getSize())))
            best = outport1_.getSize();

        if (outport2_.isConnected() && (hmul(best) < hmul(outport2_.getSize())))
            best = outport2_.getSize();

        if (outport3_.isConnected() && (hmul(best) < hmul(outport3_.getSize())))
            best = outport3_.getSize();

        if (outport4_.isConnected() && (hmul(best) < hmul(outport4_.getSize())))
            best = outport4_.getSize();
    }
    else if (selectionMode_.get() == "aspect-ratio") {

        float bestAR = 999999.0;
        if (outport1_.isConnected()) {
            float ar = outport1_.getSize().x / (float)outport1_.getSize().y;
            ar = std::max(ar, 1.0f/ar);
               if (ar < bestAR) {
                best = outport1_.getSize();
                bestAR = best.x / (float)best.y;
                bestAR = std::max(bestAR, 1.0f/bestAR);
            }
        }

        if (outport2_.isConnected()) {
            float ar = outport2_.getSize().x / (float)outport2_.getSize().y;
            ar = std::max(ar, 1.0f/ar);
               if (ar < bestAR) {
                best = outport2_.getSize();
                bestAR = best.x / (float)best.y;
                bestAR = std::max(bestAR, 1.0f/bestAR);
            }
        }

        if(outport3_.isConnected()) {
            float ar = outport3_.getSize().x / (float)outport3_.getSize().y;
            ar = std::max(ar, 1.0f/ar);
               if (ar < bestAR) {
                best = outport3_.getSize();
                bestAR = best.x / (float)best.y;
                bestAR = std::max(bestAR, 1.0f/bestAR);
            }
        }

        if(outport4_.isConnected()) {
            float ar = outport4_.getSize().x / (float)outport4_.getSize().y;
            ar = std::max(ar, 1.0f/ar);
               if (ar < bestAR) {
                best = outport4_.getSize();
                bestAR = best.x / (float)best.y;
                bestAR = std::max(bestAR, 1.0f/bestAR);
            }
        }
    }
    else if (selectionMode_.get() == "width") {
        if (outport1_.isConnected() && (best.x < outport1_.getSize().x))
            best = outport1_.getSize();

        if (outport2_.isConnected() && (best.x < outport2_.getSize().x))
            best = outport2_.getSize();

        if (outport3_.isConnected() && (best.x < outport3_.getSize().x))
            best = outport3_.getSize();

        if (outport4_.isConnected() && (best.x < outport4_.getSize().x))
            best = outport4_.getSize();
    }
    else if (selectionMode_.get() == "height") {
        if (outport1_.isConnected() && (best.y < outport1_.getSize().y))
            best = outport1_.getSize();

        if (outport2_.isConnected() && (best.y < outport2_.getSize().y))
            best = outport2_.getSize();

        if (outport3_.isConnected() && (best.y < outport3_.getSize().y))
            best = outport3_.getSize();

        if (outport4_.isConnected() && (best.y < outport4_.getSize().y))
            best = outport4_.getSize();
    }
    return best;
}

void MultiScale::portResized(RenderPort* p, tgt::ivec2 newsize) {
    p->resize(newsize);
    inport_.resize(selectBest());
    invalidate();
}

} // voreen namespace
