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

#include "voreen/core/processors/canvasrenderer.h"
#include "voreen/core/utils/voreenpainter.h"
#include "voreen/core/processors/processorwidgetfactory.h"
#include "voreen/core/voreenapplication.h"

#include "tgt/glcanvas.h"
#include "tgt/texturemanager.h"
#include "tgt/gpucapabilities.h"

namespace voreen {

const std::string CanvasRenderer::loggerCat_("voreen.CanvasRenderer");

CanvasRenderer::CanvasRenderer()
    : RenderProcessor()
    , canvasSize_("canvasSize", "Canvas Size", tgt::ivec2(256), tgt::ivec2(32), tgt::ivec2(2 << 12), Processor::VALID)
    , canvas_(0)
    , shader_(0)
    , inport_(Port::INPORT, "image.input")
    , errorTex_(0)
    , renderToImage_(false)
    , renderToImageFilename_("")
{
    addPort(inport_);
    addProperty(canvasSize_);
    inport_.sizeOriginChanged(&inport_);

    canvasSize_.onChange(CallMemberAction<CanvasRenderer>(this, &CanvasRenderer::sizePropChanged));
}

CanvasRenderer::~CanvasRenderer() {
}

Processor* CanvasRenderer::create() const {
    return new CanvasRenderer();
}

std::string CanvasRenderer::getProcessorInfo() const {
    return "The Canvas processor is an end node in the network. It copies its input \
            to the associated OpenGL canvas and can take snapshots of arbitrary size, \
            only limited by the graphics board's maximal 2D texture dimensions.";
}

void CanvasRenderer::process() {
    if (!canvas_)
        return;

    canvas_->getGLFocus();
    glViewport(0, 0, canvas_->getSize().x, canvas_->getSize().y);

    if (inport_.isReady()) {
        // render inport to image, if renderToImage flag has been set
        if (renderToImage_) {
            try {
                renderInportToImage(renderToImageFilename_);
                LINFO("Saved rendering " << inport_.getSize() << " to file: " << renderToImageFilename_);
            }
            catch (std::bad_alloc& /*e*/) {
                LERROR("Exception in CanvasRenderer::renderInportToImage(): bad allocation (" << getName() << ")");
                renderToImageError_ = "Not enough system memory (bad allocation)";
            }
            catch (VoreenException& e) {
                LERROR(e.what());
                renderToImageError_ = std::string(e.what());
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

            // activate shader
            shader_->activate();

            // set common uniforms
            setGlobalShaderParameters(shader_);

            // manually pass the viewport dimensions to the shader,
            // since setGlobalShaderParameters() expects a render outport, which we do not have
            shader_->setIgnoreUniformLocationError(true);
            shader_->setUniform("screenDim_", tgt::vec2(canvas_->getSize()));
            shader_->setUniform("screenDimRCP_", 1.f / tgt::vec2(canvas_->getSize()));
            shader_->setIgnoreUniformLocationError(false);

            // bind input textures
            inport_.bindTextures(GL_TEXTURE0, GL_TEXTURE1);

            // pass texture parameters to the shader
            shader_->setUniform("colorTex_", 0);
            shader_->setUniform("depthTex_", 1);
            inport_.setTextureParameters(shader_, "texParams_");
            LGL_ERROR;

            // execute the shader
            renderQuad();
            shader_->deactivate();
            LGL_ERROR;
        }
    }
    else {
        // render error texture
        if (!errorTex_) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            return;
        }
        glClear(GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        errorTex_->bind();
        errorTex_->enable();

        glColor3f(1.f, 1.f, 1.f);
        renderQuad();

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

    shader_ = ShdrMgr.loadSeparate("passthrough.vert", "copyimage.frag", generateHeader(), false);

    if (!shader_) {
        initialized_ = false;
        throw VoreenException("failed to load shaders");
    }

    errorTex_ = TexMgr.load(VoreenApplication::app()->getTexturePath("error.tga"));

    canvasSize_.setMaxValue(tgt::ivec2(GpuCaps.getMaxTextureSize()));
}

void CanvasRenderer::deinitialize() throw (VoreenException) {
   ShdrMgr.dispose(shader_);
   shader_ = 0;
   if (errorTex_)
       TexMgr.dispose(errorTex_);
   errorTex_ = 0;
   LGL_ERROR;

   setCanvas(0);
   LGL_ERROR;

   RenderProcessor::deinitialize();
}

bool CanvasRenderer::isEndProcessor() const {
    return true;
}

void CanvasRenderer::canvasResized(tgt::ivec2 newsize) {
    if (canvas_) {
        canvas_->getGLFocus();
        inport_.resize(newsize);
        canvasSize_.set(newsize);
    }
}

void CanvasRenderer::invalidate(int inv) {
    // Since VoreenPainter::paint() already calls process() on the
    // NetworkEvaluator, we bypass Processor::invalidate()
    // in order to prevent a needless double processing of the network.
    PropertyOwner::invalidate(inv);
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

    if (!inport_.hasRenderTarget()) {
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

    if (!inport_.hasRenderTarget()) {
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

void CanvasRenderer::renderInportToImage(const std::string& filename) throw (VoreenException) {
    if (!inport_.hasRenderTarget())
        throw VoreenException("No rendering");

    inport_.saveToImage(filename);
}

const tgt::Texture* CanvasRenderer::getImageColorTexture() const {
    if (inport_.hasRenderTarget())
        return inport_.getColorTexture();
    else
        return 0;
}

tgt::Texture* CanvasRenderer::getImageColorTexture() {
    if (inport_.hasRenderTarget())
        return inport_.getColorTexture();
    else
        return 0;
}

const tgt::Texture* CanvasRenderer::getImageDepthTexture() const {
    if (inport_.hasRenderTarget())
        return inport_.getDepthTexture();
    else
        return 0;
}

tgt::Texture* CanvasRenderer::getImageDepthTexture() {
    if (inport_.hasRenderTarget())
        return inport_.getDepthTexture();
    else
        return 0;
}

std::string CanvasRenderer::getRenderToImageError() const {
    return renderToImageError_;
}

void CanvasRenderer::resizeCanvas(tgt::ivec2 newsize) {

    if (!tgt::hand(tgt::greaterThanEqual(newsize, tgt::ivec2(canvasSize_.getMinValue()))) && tgt::hand(tgt::lessThanEqual(newsize, canvasSize_.getMaxValue()))) {
        LWARNING("Invalid canvas dimensions: " << newsize << ". Ignoring.");
    }

    if (!canvas_)
        return;

    if (getProcessorWidget() && getProcessorWidget()->getSize() != newsize) {
        if (getProcessorWidget()->isVisible()) {
            getProcessorWidget()->setSize(newsize.x, newsize.y);
        }
        newsize = getProcessorWidget()->getSize();
    }

    if (newsize != inport_.getSize()) {
        canvas_->getGLFocus();
        glViewport(0, 0, static_cast<GLint>(newsize.x), static_cast<GLint>(newsize.y));
        inport_.resize(newsize);
    }

    canvasSize_.set(newsize);
}

void CanvasRenderer::sizePropChanged() {
    resizeCanvas(canvasSize_.get());
}

} // namespace voreen
