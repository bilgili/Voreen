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

#include "voreen/qt/widgets/rendertargetdebugwidget.h"
#include "voreen/core/vis/network/networkevaluator.h"
#include "voreen/core/vis/rendertarget.h"
#include "tgt/vector.h"
#include "tgt/gpucapabilities.h"

#include <QMouseEvent>
#include <QString>
#include <QMenu>

namespace voreen {

RenderTargetDebugWidget::RenderTargetDebugWidget(const QGLWidget* shareWidget)
    : QGLWidget(0, shareWidget),
      NetworkEvaluator::ProcessWrapper(),
      floatProgram_(0),
      depthProgram_(0),
      evaluator_(0),
      paintInfos_(true),
      selected_(0),
      fullscreen_(false)
{
    contextMenuMEN_ = new QMenu(this);
    colorBufferACT_ = contextMenuMEN_->addAction("color buffer");
    colorBufferACT_->setCheckable(true);
    alphaChannelACT_ = contextMenuMEN_->addAction("alpha channel");
    alphaChannelACT_->setCheckable(true);
    depthBufferACT_ = contextMenuMEN_->addAction("depth buffer");
    depthBufferACT_->setCheckable(true);
    showInfosACT_ = contextMenuMEN_->addAction("Show infos");
    showInfosACT_->setCheckable(true);
    showInfosACT_->setChecked(true);

    typeToShowACG_ = new QActionGroup(this);
    typeToShowACG_->addAction(colorBufferACT_);
    typeToShowACG_->addAction(alphaChannelACT_);
    typeToShowACG_->addAction(depthBufferACT_);

    for (unsigned int i=0; i < 16; ++i)
        showType_[i] = 0;

    setWindowTitle(tr("Render Targets"));
}

RenderTargetDebugWidget::~RenderTargetDebugWidget() {
}

void RenderTargetDebugWidget::setEvaluator(NetworkEvaluator* evaluator) {

    if (evaluator_)
       evaluator_->removeProcessWrapper(this);

    evaluator_ = evaluator;

    if (evaluator_)
        evaluator_->addProcessWrapper(this);

    update();

}

void RenderTargetDebugWidget::afterNetworkProcess() {
    if (isVisible())
        update();
}

void RenderTargetDebugWidget::mousePressEvent(QMouseEvent * e) {
    if (!fullscreen_) {
        size_t selected = e->x()*4/width_ + 4* (3-(e->y()*4/height_));
        if (selected_ != selected) {
            selected_ = selected;
            //repaint();
        }
    }
    if (e->button() == Qt::LeftButton) {
        fullscreen_ = ! fullscreen_;
        repaint();
    }
    if (e->button() == Qt::MidButton) {
        repaint();
    }
    if (e->button() == Qt::RightButton) {
        const std::vector<RenderTarget*>& rts = evaluator_->getCurrentRenderTargets();
        if(selected_ < rts.size()) {
                //int attr = tc->getAttr(i);
                colorBufferACT_->setEnabled(false);
                alphaChannelACT_->setEnabled(false);
                depthBufferACT_->setEnabled(false);

                colorBufferACT_->setVisible(true);
                alphaChannelACT_->setVisible(true);
                depthBufferACT_->setVisible(true);

                if (rts[selected_]->getColorTexture()) {
                    colorBufferACT_->setEnabled(true);
                    //TODO: disable for RGB
                    alphaChannelACT_->setEnabled(true);
                }

                if (rts[selected_]->getDepthTexture()) {
                    depthBufferACT_->setEnabled(true);
                }

                switch(showType_[selected_]) {
                    case 1:
                        colorBufferACT_->setChecked(true);
                        break;
                    case 2:
                        depthBufferACT_->setChecked(true);
                        break;
                    case 9:
                        alphaChannelACT_->setChecked(true);
                        break;
                }
                contextMenuMEN_->exec(e->globalPos());

                if (colorBufferACT_->isChecked())
                    showType_[selected_] = 1;
                else if (depthBufferACT_->isChecked())
                    showType_[selected_] = 2;
                else if (alphaChannelACT_->isChecked())
                    showType_[selected_] = 3;
        }
        else {
            colorBufferACT_->setVisible(false);
            alphaChannelACT_->setVisible(false);
            depthBufferACT_->setVisible(false);

            contextMenuMEN_->exec(e->globalPos());
        }
    }

    updateGL();
}

void RenderTargetDebugWidget::keyPressEvent(QKeyEvent* e) {
    if ((e->key() == Qt::Key_T && (e->modifiers() & Qt::ControlModifier)) || e->key() == Qt::Key_Escape)
        close();
}

void RenderTargetDebugWidget::closeEvent(QCloseEvent *event) {
    emit closing(false);
    event->accept();
}

void RenderTargetDebugWidget::showEvent(QShowEvent* e) {
    QGLWidget::showEvent(e);
}

void RenderTargetDebugWidget::hideEvent(QHideEvent* e) {
    emit hideSignal();
    QGLWidget::hideEvent(e);
}

void RenderTargetDebugWidget::initializeGL() {
    floatProgram_ = ShdrMgr.loadSeparate("", "stc_showfloatcolor.frag", generateHeader(), false, false);
    depthProgram_ = ShdrMgr.loadSeparate("", "stc_showdepth.frag", generateHeader(), false, false);
    tgtAssert(floatProgram_, "");
    tgtAssert(depthProgram_, "");

    floatRectProgram_ = 0;
    depthRectProgram_ = 0;

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
}

void RenderTargetDebugWidget::resizeGL(int width, int height) {
    glViewport(0,0, width, height);
    width_ = width;
    height_ = height;
}

QString RenderTargetDebugWidget::internalTypeToString(GLint internalType) {
    switch (internalType) {
        case GL_RGB: return "GL_RGB";
        case GL_RGBA: return "GL_RGBA";
        case GL_RGB16F_ARB: return "GL_RGB16F_ARB";
        case GL_RGBA16F_ARB: return "GL_RGBA16F_ARB";
        case GL_DEPTH_COMPONENT16: return "GL_DEPTH_COMPONENT16";
        case GL_DEPTH_COMPONENT24: return "GL_DEPTH_COMPONENT24";
        case GL_DEPTH_COMPONENT32: return "GL_DEPTH_COMPONENT32";
        default:
            return "Unknown type";
    }
}

void RenderTargetDebugWidget::paintInfos(RenderTarget* rt) {
    QString colorStr = "";
    QString depthStr = "";
    QString sizeStr = QString("%1x%2").arg(rt->getSize().x).arg(rt->getSize().y);

    if (rt->getColorTexture()) {
        colorStr += internalTypeToString(rt->getColorTexture()->getInternalFormat());
    }
    else
        colorStr += "No color texture";

    if (rt->getDepthTexture()) {
        depthStr += internalTypeToString(rt->getDepthTexture()->getInternalFormat());
    }
    else
        depthStr += "No depth texture";

    int mem = 0;
    if (rt->getColorTexture())
        mem += rt->getColorTexture()->getSizeOnGPU();
    if (rt->getDepthTexture())
        mem += rt->getDepthTexture()->getSizeOnGPU();

    mem /= 1024;

    QString memStr = QString("%1 kb").arg(mem);
    QString numUpdateStr = QString("@%1").arg(rt->getNumUpdates());

    glColor4f(1.0f, 1.0f, 1.0f, 1.f);
    LGL_ERROR;
    renderText(-0.95, 0.8, -1.0, QString(rt->getDebugLabel().c_str()));
    renderText(-0.95, 0.6, -1.0, sizeStr);
    renderText(-0.95, 0.4, -1.0, colorStr);
    renderText(-0.95, 0.2, -1.0, depthStr);
    renderText(-0.95, 0.0, -1.0, memStr);
    renderText(-0.95, -0.2, -1.0, numUpdateStr);
    LGL_ERROR;
}

std::string RenderTargetDebugWidget::generateHeader() {
    std::string header = "";
    if(!GpuCaps.isNpotSupported())
        header += "#define VRN_TEXTURE_RECTANGLE\n";
    else
        header += "#define VRN_TEXTURE_2D\n";

    return header;
}

void RenderTargetDebugWidget::paint(RenderTarget* rt, int id) {
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);

    //unsigned int attr = tc->getAttr(rt->textureID_);
    if (showType_[id] == 0) {
        showType_[id] = 1;
        if (!rt->getColorTexture())
            showType_[id] = 2;
    }
    switch(showType_[id]) {
    case 1: // color buffer
    case 3:
        {
            if(rt->getColorTexture()) {
                rt->bindColorTexture();

                //FIXME: set consistent texture filtering mode here, but restore old state afterwards

                tgt::Shader* prg = floatProgram_;

                if (!prg)
                    return;
                prg->activate();
                prg->setUniform("tex_", 0);
                LGL_ERROR;

                prg->setIgnoreUniformLocationError(true);
                prg->setUniform("texParameters_.dimensions_", tgt::vec2(rt->getSize()));
                prg->setUniform("texParameters_.dimensionsRCP_", 1.f / tgt::vec2(rt->getSize()));
                prg->setIgnoreUniformLocationError(false);

                LGL_ERROR;
                if (showType_[id] == 3)
                    prg->setUniform("showAlpha_", true);
                else
                    prg->setUniform("showAlpha_", false);
                LGL_ERROR;

                glDisable(GL_DEPTH_TEST);
                glDisable(GL_LIGHTING);
                glBegin(GL_QUADS);
                glTexCoord3f(0, 0, 0); glVertex3f(-1, -1, -0.5f);
                glTexCoord3f(1, 0, 0); glVertex3f(1, -1, -0.5f);
                glTexCoord3f(1, 1, 0); glVertex3f(1,  1, -0.5f);
                glTexCoord3f(0, 1, 0); glVertex3f(-1,  1, -0.5f);
                glEnd();
                glEnable(GL_DEPTH_TEST);
                prg->deactivate();
            }
            break;
        }

    case 2: // depth buffer
        {
            if(rt->getDepthTexture()) {
                rt->bindDepthTexture();
                tgt::Shader* prg = depthProgram_;

                prg->activate();
                prg->setUniform("tex_", 0);
                prg->setIgnoreUniformLocationError(true);
                prg->setUniform("texParameters_.dimensions_", tgt::vec2(rt->getSize()));
                prg->setUniform("texParameters_.dimensionsRCP_", 1.f / tgt::vec2(rt->getSize()));
                prg->setIgnoreUniformLocationError(false);
                LGL_ERROR;

                glDisable(GL_DEPTH_TEST);
                glDisable(GL_LIGHTING);
                glBegin(GL_QUADS);
                glTexCoord3f(0, 0, 0); glVertex3f(-1, -1, -0.5f);
                glTexCoord3f(1, 0, 0); glVertex3f(1, -1, -0.5f);
                glTexCoord3f(1, 1, 0); glVertex3f(1,  1, -0.5f);
                glTexCoord3f(0, 1, 0); glVertex3f(-1,  1, -0.5f);
                glEnd();
                glEnable(GL_DEPTH_TEST);
                prg->deactivate();
            }
            break;
        }
    default:; // do nothing
    }
    LGL_ERROR;

    glDisable(GL_TEXTURE_2D);

    if(showInfosACT_->isChecked())
        paintInfos(rt);
    LGL_ERROR;
}

void RenderTargetDebugWidget::paintGL() {

    makeCurrent();
    if (!isValid())
        return;

    glViewport(0, 0, width(), height());
    glClearColor(0.7, 0.7, 0.7, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0,0,0,0);

    LGL_ERROR;

    if (!evaluator_ || !evaluator_->isInitialized()) {
        window()->setWindowTitle("Render Targets");
        return;
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    LGL_ERROR;

    const std::vector<RenderTarget*>& rts = evaluator_->getCurrentRenderTargets();

    if (fullscreen_) {
        if(selected_ < rts.size())
            paint(rts[selected_], selected_);
    }
    else {
        for (size_t i = 0; (i < rts.size()); ++i) {
            glPushMatrix();
            glTranslatef(-0.75+0.5*(i%4), -0.75+0.5*(i/4), 0.0);
            glScalef(0.25, 0.25, 1.0);
            paint(rts[i], i);
            glPopMatrix();
        }
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    LGL_ERROR;

    int memsize = 0;
    for (size_t i = 0; (i < rts.size()); ++i) {
        if (rts[i]->getColorTexture())
            memsize += rts[i]->getColorTexture()->getSizeOnGPU();
        if (rts[i]->getDepthTexture())
            memsize += rts[i]->getDepthTexture()->getSizeOnGPU();
    }

    memsize /= 1024 * 1024;

    window()->setWindowTitle(tr("%1 Render Targets (%2 mb)").arg(rts.size()).arg(memsize));
}

} // namespace voreen
