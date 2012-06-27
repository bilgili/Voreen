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

#include "voreen/core/vis/processors/image/canvasrenderer.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/processors/processorwidgetfactory.h"
#include "voreen/core/application.h"

#include "tgt/glcanvas.h"
#include "tgt/texturemanager.h"

namespace voreen {

CanvasRenderer::CanvasRenderer()
    : RenderProcessor()
    , canvas_(0)
    , inport_(Port::INPORT, "image.input")
    , errorTex_(0)
    , renderToImage_(false)
    , renderToImageFilename_("")
{
    addPort(inport_);
    inport_.sizeOriginChanged(&inport_);
}

CanvasRenderer::~CanvasRenderer() {
    setCanvas(0);

    if (errorTex_)
        TexMgr.dispose(errorTex_);
}

Processor* CanvasRenderer::create() const {
    return new CanvasRenderer();
}

const std::string CanvasRenderer::getProcessorInfo() const {
    return "The Canvas processor is an end node in the network. It copies its input \
            to the associated OpenGL canvas and can take snapshots of arbitrary size, \
            only limited by the graphics board's maximal 2D texture dimensions.";
}

void CanvasRenderer::process() {
    if (!canvas_)
        return;

    canvas_->getGLFocus();

    // deactivate frame buffer object => render to frame buffer
    tgt::FramebufferObject::deactivate();

    glViewport(0, 0, canvas_->getSize().x, canvas_->getSize().y);

    if (inport_.isReady()) {
        // render inport to image, if renderToImage flag has been set
        if (renderToImage_) {
            try {
                renderInportToImage(renderToImageFilename_);
            }
            catch (std::bad_alloc& /*e*/) {
                LERROR("Exception in CanvasRenderer::renderInportToImage(): bad allocation (" << getName() << ")");
                renderToImageError_ = "Not enough system memory (bad allocation)";
            }
            catch (std::exception& e) {
                LERROR("Exception in CanvasRenderer::renderInportToImage(): " << e.what() << " (" << getName() << ")");
                renderToImageError_ = std::string(e.what());
            }
            renderToImage_ = false;
        }
        // map texture of input target onto a screen-aligned quad
        else {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            inport_.bindColorTexture(GL_TEXTURE0);
            inport_.getColorTexture()->enable();
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            glDepthFunc(GL_ALWAYS);
            renderQuadWithTexCoords(inport_);
            glDepthFunc(GL_LESS);

            inport_.getColorTexture()->disable();
			glDisable(GL_BLEND);
        }
    }
    else {
        // render error texture
        if (!errorTex_) {
            LERROR("No error texture");
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            return;
        }

        glActiveTexture(GL_TEXTURE0);
        errorTex_->bind();
        errorTex_->enable();

        glDepthFunc(GL_ALWAYS);
        renderQuad();
        glDepthFunc(GL_LESS);

        errorTex_->disable();
    }

    glActiveTexture(GL_TEXTURE0);
    LGL_ERROR;
}

bool CanvasRenderer::isReady() const {
    return true;
}

void CanvasRenderer::initialize() throw (VoreenException) {

    RenderProcessor::initialize();

    errorTex_ = TexMgr.load(VoreenApplication::app()->getDataPath("textures/error.tga"));

    initialized_ = true;
}

bool CanvasRenderer::isEndProcessor() const {
    return true;
}

void CanvasRenderer::portResized(RenderPort* /*p*/, tgt::ivec2 newsize) {
    if (canvas_) {
        canvas_->getGLFocus();
        inport_.resize(newsize);
    }
}

void CanvasRenderer::invalidate(InvalidationLevel inv) {
    RenderProcessor::invalidate(inv);
    if (canvas_)
        canvas_->update();
}

void CanvasRenderer::setCanvas(tgt::GLCanvas* canvas) {

    if (canvas == canvas_)
        return;

    //remove from old canvas:
    if (canvas_) {
        tgt::EventHandler* eh = canvas_->getEventHandler();
        if (eh) {
            eh->removeListener(this);
        }
    }
    canvas_ = canvas;
    //register at new canvas:
    if (canvas_) {
        tgt::EventHandler* eh = canvas_->getEventHandler();
        if (eh) {
            eh->addListenerToFront(this);
        }
        inport_.resize(canvas->getSize());
    }

    invalidate();
}

tgt::GLCanvas* CanvasRenderer::getCanvas() const {
    return canvas_;
}

void CanvasRenderer::onEvent(tgt::Event* e) {
    if (canvas_) {
        canvas_->getGLFocus();
        Processor::onEvent(e);
    }
}

bool CanvasRenderer::renderToImage(const std::string &filename) {

    if (!canvas_) {
        LWARNING("CanvasRenderer::renderToImage(): no canvas assigned");
        renderToImageError_ = "No canvas assigned";
        return false;
    }

    if (!inport_.hasData()) {
        LWARNING("CanvasRenderer::renderToImage(): inport has no data");
        renderToImageError_ = "No rendering";
        return false;
    }

    // enable render-to-file on next process
    renderToImageFilename_ = filename;
    renderToImage_ = true;
    renderToImageError_.clear();

    // force rendering pass
    canvas_->repaint();

    return (renderToImageError_.empty());

}

bool CanvasRenderer::renderToImage(const std::string &filename, tgt::ivec2 dimensions) {

    if (!canvas_) {
        LWARNING("CanvasRenderer::renderToImage(): no canvas assigned");
        renderToImageError_ = "No canvas assigned";
        return false;
    }

    if (!inport_.hasData()) {
        LWARNING("CanvasRenderer::renderToImage(): inport has no data");
        renderToImageError_ = "No rendering";
        return false;
    }

    tgt::ivec2 oldDimensions = inport_.getSize();
    // resize texture container to desired image dimensions and propagate change
    canvas_->getGLFocus();
    inport_.resize(dimensions);

    // render with adjusted viewport size
    bool success = renderToImage(filename);

    // reset texture container dimensions from canvas size
    inport_.resize(oldDimensions);

    return success;
}

void CanvasRenderer::renderInportToImage(const std::string& filename)
    throw(VoreenException, std::bad_alloc)
{
    if (!inport_.hasData())
        throw VoreenException("No rendering");

    inport_.saveToImage(filename);
}

std::string CanvasRenderer::getRenderToImageError() const {
    return renderToImageError_;
}

} // namespace voreen
