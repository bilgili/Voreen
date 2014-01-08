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

#include "voreen/qt/widgets/rendertargetviewer.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/network/processornetwork.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/ports/renderport.h"
#include "voreen/core/datastructures/rendertarget.h"
#include "voreen/qt/widgets/voreentoolwindow.h"
#include "tgt/font.h"
#include "tgt/vector.h"
#include "tgt/gpucapabilities.h"
#include "tgt/logmanager.h"

#include <math.h>
#include <QMouseEvent>
#include <QString>
#include <QMenu>
#include <QFileDialog>
#include <QApplication>
#include <QUrl>
#include <QMessageBox>
#include <QMainWindow>
#include <QLayout>
#include <QDesktopServices>

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;
using tgt::Texture;

namespace {
#ifdef __APPLE__
    int fontSize = 11;
#else
#ifdef VRN_MODULE_FONTRENDERING
    int fontSize = 12;
#else
    int fontSize = 8;
#endif
#endif
}

namespace voreen {

const std::string RenderTargetViewer::loggerCat_ = "voreen.RenderTargetViewer";

RenderTargetViewer::RenderTargetViewer(const QGLWidget* sharedContext)
    : QGLWidget(0, sharedContext)
    , colorProgram_(0)
    , inversecolorProgram_(0)
    , evaluator_(0)
    , paintInfos_(true)
    , selected_(0)
    , fullscreen_(false)
    , mouseX_(0)
    , mouseY_(0)
    , dimX_(0)
    , dimY_(0)
    , scaledWidth_(0)
    , scaledHeight_(0)
    , initialized_(false)
    , currentDepthBuffer_(0)
    , zoomScale_(1)
    , zoomTranslateX_(0)
    , zoomTranslateY_(0)
    , zoomMouseX_(0)
    , zoomMouseY_(0)
    , zoomOffsetX_(0)
    , zoomOffsetY_(0)
    , freezeInfos_(false)
    , fbo_(0)
    , colorTex_(0)
    , fontTex_(0)
    , sharedContext_(sharedContext)
    , isSubWidget_(false)
{
    mouseIsInside_ = underMouse();
    if(mouseIsInside_) {
        // make sure the keyoard focus is present at start
        grabKeyboard();
        // release keyboard so global shortcuts will work properly
        releaseKeyboard();
    }

    contextMenuMEN_ = new QMenu(this);
    contextMenuMEN_->setParent(this);
    backgroundBlackACT_ = contextMenuMEN_->addAction("Background: Black");
    backgroundBlackACT_->setCheckable(true);
    backgroundBlackACT_->setChecked(true);
    backgroundWhiteACT_ = contextMenuMEN_->addAction("Background: White");
    backgroundWhiteACT_->setCheckable(true);
    backgroundCheckerboardPatternACT_ = contextMenuMEN_->addAction("Background: Checkerboard Pattern");
    backgroundCheckerboardPatternACT_->setCheckable(true);

    contextMenuMEN_->addSeparator();

    colorRACT_ = contextMenuMEN_->addAction("Activate R Channel");
    colorRACT_->setCheckable(true);
    colorRACT_->setChecked(true);
    colorGACT_ = contextMenuMEN_->addAction("Activate G Channel");
    colorGACT_->setCheckable(true);
    colorGACT_->setChecked(true);
    colorBACT_ = contextMenuMEN_->addAction("Activate B Channel");
    colorBACT_->setCheckable(true);
    colorBACT_->setChecked(true);
    colorAACT_ = contextMenuMEN_->addAction("Activate A Channel");
    colorAACT_->setCheckable(true);
    colorAACT_->setChecked(true);

    contextMenuMEN_->addSeparator();

    colorRGBAACT_ = contextMenuMEN_->addAction("Color Channel");
    colorRGBAACT_->setCheckable(true);
    alphaOnlyACT_ = contextMenuMEN_->addAction("Alpha Channel");
    alphaOnlyACT_->setCheckable(true);
    depthOnlyACT_ = contextMenuMEN_->addAction("Depth Buffer");
    depthOnlyACT_->setCheckable(true);
    hOnlyACT_ = contextMenuMEN_->addAction("Hue Channel");
    hOnlyACT_->setCheckable(true);
    sOnlyACT_ = contextMenuMEN_->addAction("Saturation Channel");
    sOnlyACT_->setCheckable(true);
    vOnlyACT_ = contextMenuMEN_->addAction("Value Channel");
    vOnlyACT_->setCheckable(true);

    contextMenuMEN_->addSeparator();

    filterPortyBySelectedProcessorsACT_ = contextMenuMEN_->addAction("Display Selected Processor Ports Only");
    filterPortyBySelectedProcessorsACT_->setCheckable(true);
    filterPortyBySelectedProcessorsACT_->setChecked(false);

    keepAspectRatioACT_ = contextMenuMEN_->addAction("Keep Aspect Ratio");
    keepAspectRatioACT_->setCheckable(true);
    keepAspectRatioACT_->setChecked(true);

    contextMenuMEN_->addSeparator();

    showInfosACT_ = contextMenuMEN_->addAction("Show Infos");
    showInfosACT_->setCheckable(true);
    showInfosACT_->setChecked(true);
    showInfosDetailsACT_ = contextMenuMEN_->addAction("Show Info Details");
    showInfosDetailsACT_->setCheckable(true);
    showInfosDetailsACT_->setChecked(true);

    contextMenuMEN_->addSeparator();

    saveScreenshotACT_ = contextMenuMEN_->addAction("Save As Image...");
    saveScreenshotWithOverlayACT_ = contextMenuMEN_->addAction("Save As Image With Overlay...");

    contextMenuMEN_->addSeparator();

    openInNewWindowACT_ = contextMenuMEN_->addAction("Open In New Window");

    contextMenuMEN_->addSeparator();

    showHelpACT_ = contextMenuMEN_->addAction("Help...");

    backgroundToShowACG_ = new QActionGroup(this);
    backgroundToShowACG_->addAction(backgroundBlackACT_);
    backgroundToShowACG_->addAction(backgroundWhiteACT_);
    backgroundToShowACG_->addAction(backgroundCheckerboardPatternACT_);

    typeToShowACG_ = new QActionGroup(this);
    typeToShowACG_->addAction(colorRGBAACT_);
    typeToShowACG_->addAction(alphaOnlyACT_);
    typeToShowACG_->addAction(depthOnlyACT_);
    typeToShowACG_->addAction(hOnlyACT_);
    typeToShowACG_->addAction(sOnlyACT_);
    typeToShowACG_->addAction(vOnlyACT_);

    setWindowTitle(tr("Render Target Viewer:"));

#ifdef VRN_MODULE_FONTRENDERING
    font_ = new tgt::Font(VoreenApplication::app()->getFontPath("VeraMono.ttf"), fontSize, tgt::Font::TextureFont);
#else
    font_ = QFont("Monospace", fontSize);
    font_.setBold(true);
#endif

    setFocusPolicy(Qt::StrongFocus);
}

RenderTargetViewer::~RenderTargetViewer() {
    deinit();
#ifdef VRN_MODULE_FONTRENDERING
    delete font_;
#endif
}

void RenderTargetViewer::processorsSelected(const QList<Processor*>& processors) {
    selectedProcessors_ = processors;
    fullscreen_ = false;
    //update();
}

void RenderTargetViewer::setSubWidget(int subWidgetFullscreenIndex, int showType) {
    isSubWidget_ = true;
    fullscreen_ = true;
    selected_ = subWidgetFullscreenIndex;
    showType_[selected_] = showType;
}

void RenderTargetViewer::setEvaluator(NetworkEvaluator* evaluator) {
    setMouseTracking(false);
    if (evaluator_) {
        evaluator_->removeObserver(this);
    }

    evaluator_ = evaluator;

    if (evaluator_) {
        evaluator_->addObserver(this);
        if (evaluator_->getProcessorNetwork()) {
            setMouseTracking(true);
        }
    }

    update();
}

void RenderTargetViewer::afterNetworkProcess() {
    if (isVisible())
        update();
}

std::vector<RenderPort*> RenderTargetViewer::collectRenderPorts() {
    std::vector<RenderPort*> collectedRenderPorts = evaluator_->collectRenderPorts();

    if(collectedRenderPorts.empty() || !filterPortyBySelectedProcessorsACT_->isChecked()) {
        std::sort(collectedRenderPorts.begin(), collectedRenderPorts.end(), CompProcID());
        return collectedRenderPorts;
    }

    std::vector<RenderPort*> selectedPorts;
    for(unsigned int i=0; i<collectedRenderPorts.size(); i++) {
        if(std::find(selectedProcessors_.begin(), selectedProcessors_.end(), collectedRenderPorts[i]->getProcessor()) != selectedProcessors_.end())
            selectedPorts.push_back(collectedRenderPorts[i]);
    }
    std::sort(selectedPorts.begin(), selectedPorts.end(), CompProcID());

    return selectedPorts;
}

void RenderTargetViewer::updateSelected() {
    if (!evaluator_ || scaledWidth_ == 0 || scaledHeight_ == 0) {
        selected_ = -1;
        return;
    }
    std::vector<RenderPort*> renderPorts = collectRenderPorts();

    size_t index = mouseX_/scaledWidth_ + (dimY_ - 1 - mouseY_/scaledHeight_) * dimX_;
    if (index >= renderPorts.size())
        selected_ = -1;
    else
        selected_ = static_cast<int>(index);
}

void RenderTargetViewer::updateMousePosition(QMouseEvent* e) {
    if(!freezeInfos_) {
        mouseX_ = e->x();
        mouseY_ = height() - e->y();
    }
}

void RenderTargetViewer::enterEvent(QEvent* /*e*/) {
    mouseIsInside_ = true;
    grabKeyboard();
    repaint();
}

void RenderTargetViewer::leaveEvent(QEvent* /*e*/) {
    mouseIsInside_ = false;
    releaseKeyboard();
    repaint();
}

void RenderTargetViewer::mousePressEvent(QMouseEvent* e) {
    updateMousePosition(e);
    if (!evaluator_) {
        selected_ = -1;
        return;
    }
    std::vector<RenderPort*> renderPorts = collectRenderPorts();

    if (!fullscreen_)
        updateSelected();

    if (e->button() == Qt::RightButton) {
        if (selected_ >= 0 && selected_ < (int)renderPorts.size()) {
            RenderTarget* rt = renderPorts[selected_]->getRenderTarget();
            colorRGBAACT_->setEnabled(false);
            alphaOnlyACT_->setEnabled(false);
            depthOnlyACT_->setEnabled(false);
            hOnlyACT_->setEnabled(false);
            sOnlyACT_->setEnabled(false);
            vOnlyACT_->setEnabled(false);

            colorRGBAACT_->setVisible(true);
            alphaOnlyACT_->setVisible(true);
            depthOnlyACT_->setVisible(true);
            hOnlyACT_->setVisible(true);
            sOnlyACT_->setVisible(true);
            vOnlyACT_->setVisible(true);

            openInNewWindowACT_->setVisible(selected_ > -1);

            backgroundBlackACT_->setChecked((showType_[selected_] & BackgroundBlack) != 0);
            backgroundWhiteACT_->setChecked((showType_[selected_] & BackgroundWhite) != 0);
            backgroundCheckerboardPatternACT_->setChecked((showType_[selected_] & CheckerboardPattern) != 0);

            if (rt->getColorTexture()) {
                colorRGBAACT_->setEnabled(true);
                alphaOnlyACT_->setEnabled(true);
                hOnlyACT_->setEnabled(true);
                sOnlyACT_->setEnabled(true);
                vOnlyACT_->setEnabled(true);
            }

            if (rt->getDepthTexture())
                depthOnlyACT_->setEnabled(true);

            if((showType_[selected_] & Color) != 0)
                colorRGBAACT_->setChecked(true);
            if(showType_[selected_] & Alpha)
                alphaOnlyACT_->setChecked(true);
            if(showType_[selected_] & Depth)
                depthOnlyACT_->setChecked(true);
            if(showType_[selected_] & H)
                hOnlyACT_->setChecked(true);
            if(showType_[selected_] & S)
                sOnlyACT_->setChecked(true);
            if(showType_[selected_] & V)
                vOnlyACT_->setChecked(true);
        }
        else {
            colorRGBAACT_->setVisible(false);
            alphaOnlyACT_->setVisible(false);
            depthOnlyACT_->setVisible(false);
            hOnlyACT_->setVisible(false);
            sOnlyACT_->setVisible(false);
            vOnlyACT_->setVisible(false);
        }

        showInfosDetailsACT_->setVisible(fullscreen_);
        saveScreenshotACT_->setVisible(fullscreen_);
        saveScreenshotWithOverlayACT_->setVisible(fullscreen_);

        QAction* currentAction = contextMenuMEN_->exec(e->globalPos());

        // Workaround for QT bug as it seems underMouse() yields false after contextMenuMEN_->exec(...)
        mouseIsInside_ = true;

        // HACK this might be superflues if context menu will keep keyboard focus
        // get keyboard focus
        grabKeyboard();
        // release keyboard, must not interfere with global shortcuts
        releaseKeyboard();

        if (currentAction == saveScreenshotACT_)
            takeScreenshot(false);
        if (currentAction == saveScreenshotWithOverlayACT_)
            takeScreenshot(true);
        if (currentAction == openInNewWindowACT_)
            openSubWindow();
        if (currentAction == showHelpACT_)
            showHelp();

        if (colorRGBAACT_->isChecked())
            showType_[selected_] =
            Color
            |(colorRACT_->isChecked()?R:0)
            |(colorGACT_->isChecked()?G:0)
            |(colorBACT_->isChecked()?B:0)
            |(colorAACT_->isChecked()?A:0);
        else if (alphaOnlyACT_->isChecked())
            showType_[selected_] = Alpha;
        else if (depthOnlyACT_->isChecked())
            showType_[selected_] = Depth;
        else if (hOnlyACT_->isChecked())
            showType_[selected_] = H;
        else if (sOnlyACT_->isChecked())
            showType_[selected_] = S;
        else if (vOnlyACT_->isChecked())
            showType_[selected_] = V;


        if (backgroundBlackACT_->isChecked())
            showType_[selected_] |= BackgroundBlack;
        if (backgroundWhiteACT_->isChecked())
            showType_[selected_] |= BackgroundWhite;
        if (backgroundCheckerboardPatternACT_->isChecked())
            showType_[selected_] |= CheckerboardPattern;
    }

    if (fullscreen_ && e->button() == Qt::MidButton) {
        zoomScale_ = 1.0f;
    }

    if (e->button() == Qt::LeftButton) {
        if(fullscreen_ && zoomScale_ > 1.0f) {
            isPanning_ = true;
        } else {
            if(!isSubWidget_)
                fullscreen_ = !fullscreen_;
        }
    }

    //if(!fullscreen_)
        //zoomScale_ = 1.0f;

    update();
}

void RenderTargetViewer::mouseReleaseEvent(QMouseEvent* /*e*/) {
    isPanning_ = false;
}

void RenderTargetViewer::mouseMoveEvent(QMouseEvent* e) {
    updateMousePosition(e);
    if(!evaluator_){
        return;
    }
    if (!fullscreen_) {
        updateSelected();
    }
    update();
}

void RenderTargetViewer::wheelEvent(QWheelEvent* e) {
    if(isPanning_)
        return;

    if(zoomScale_ == 1.0) {
        zoomOffsetX_ = mouseX_;
        zoomOffsetY_ = mouseY_;
        zoomMouseX_ = -1.0;
        zoomMouseY_ = -1.0;
    }

    if(zoomMouseX_ != mouseX_ || zoomMouseY_ != mouseY_) {
        zoomOffsetX_ += (mouseX_ - zoomOffsetX_) / zoomScale_;
        zoomOffsetY_ += (mouseY_ - zoomOffsetY_) / zoomScale_;
        zoomMouseX_ = mouseX_;
        zoomMouseY_ = mouseY_;
    }
    zoomScale_ = std::max(1.0f, zoomScale_ + (e->delta()/1200.0f));

    zoomTranslateX_ = -((zoomScale_*zoomOffsetX_)-mouseX_)/zoomScale_;
    zoomTranslateY_ = -((zoomScale_*zoomOffsetY_)-mouseY_)/zoomScale_;

    update();
}

void RenderTargetViewer::showHelp() {
    QMessageBox help(QMessageBox::Information,
        "Help",
        "\
+/- (in fullscreen)\t: Zoom in/out \n \
CTRL + T\t\t: Close \n \
ESC (in overview)\t: Close \n \
ESC (in fullscreen)\t: Return to overview \n \
F (in fullscreen)\t\t: Freeze infos \n \
H\t\t: Help (this dialog) \n \
MMB (in fullscreen)\t: Reset zoom \n \
RMB\t\t: Context menu \n \
Wheel (in fullscreen)\t: Zoom in/out");
    help.exec();
}

void RenderTargetViewer::keyPressEvent(QKeyEvent* e) {
    if(e->key() == Qt::Key_H) {
        showHelp();
    }

    if (fullscreen_) {
        if (e->key() == Qt::Key_F)
            freezeInfos_ = !freezeInfos_;
        if (e->key() == Qt::Key_Plus)
            zoomScale_ += 0.5;
        if (e->key() == Qt::Key_Minus)
            zoomScale_ = max(1.0, zoomScale_ - 0.5);
        if (e->key() == Qt::Key_Escape) {
            zoomScale_ = 1;
            fullscreen_ = false;
        }
    } else {
        if (e->key() == Qt::Key_Escape)
            parentWidget()->parentWidget()->close();
    }
    repaint();
}

void RenderTargetViewer::closeEvent(QCloseEvent *event) {
    emit closing(false);
    event->accept();
}

void RenderTargetViewer::showEvent(QShowEvent* e) {
    QGLWidget::showEvent(e);
}

void RenderTargetViewer::hideEvent(QHideEvent* e) {
    emit hideSignal();
    QGLWidget::hideEvent(e);
}

QString RenderTargetViewer::internalTypeToString(GLint internalType) {
    switch (internalType) {
        case GL_RGB: return "GL_RGB";
        case GL_RGBA: return "GL_RGBA";
        case GL_RGBA16: return "GL_RGBA16";
        case GL_RGB16F_ARB: return "GL_RGB16F_ARB";
        case GL_RGBA16F_ARB: return "GL_RGBA16F_ARB";
        case GL_RGBA32F_ARB: return "GL_RGBA32F_ARB";
        case GL_R32F: return "GL_R32F";
        case GL_R32UI: return "GL_R32UI";
        case GL_LUMINANCE: return "GL_LUMINANCE";
        case GL_LUMINANCE32F_ARB: return "GL_LUMINANCE32F_ARB";
        case GL_DEPTH_COMPONENT16: return "GL_DEPTH_COMPONENT16";
        case GL_DEPTH_COMPONENT24: return "GL_DEPTH_COMPONENT24";
        case GL_DEPTH_COMPONENT32: return "GL_DEPTH_COMPONENT32";
        default:
            return "Unknown type";
    }
}

void RenderTargetViewer::initializeGL() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    //glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glViewport(0,0, width(), height());
    std::string header = "#version 130\n";

    try {
        colorProgram_ = ShdrMgr.loadSeparate("", "rendertargetviewer/color.frag", header, false);
        inversecolorProgram_ = ShdrMgr.loadSeparate("", "rendertargetviewer/inversecolor.frag", header, false);
    }
    catch(tgt::Exception& e) {
        LERROR(e.what());
    }

    if (!fbo_)
        fbo_ = new tgt::FramebufferObject();

    if (!fbo_) {
        LERROR("Failed to initialize framebuffer object!");
        glPopAttrib();
        return;
    }

    tgt::ivec3 size(width(), height(), 1);

    //colorTex_ = new Texture(0, size, GL_RGBA, GL_RGBA16F_ARB, GL_FLOAT, Texture::LINEAR);
    //colorTex_ = new Texture(0, size, GL_RGBA, GL_RGBA32F_ARB, GL_FLOAT, Texture::LINEAR);
    colorTex_ = new Texture(0, size, GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT, Texture::LINEAR);
    colorTex_->uploadTexture();
    colorTex_->setWrapping(tgt::Texture::CLAMP_TO_EDGE);

    fontTex_ = new Texture(0, size, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT, Texture::LINEAR);
    fontTex_->uploadTexture();
    fontTex_->setWrapping(tgt::Texture::CLAMP_TO_EDGE);

    glPopAttrib();
}

void RenderTargetViewer::deinit() {
    if (evaluator_)
        evaluator_->removeObserver(this);

    delete fbo_;
    ShdrMgr.dispose(colorProgram_);
    ShdrMgr.dispose(inversecolorProgram_);

    fbo_ = 0;
    colorProgram_ = 0;
    inversecolorProgram_ = 0;
}

void RenderTargetViewer::resizeGL(int width, int height) {
    glViewport(0,0, width, height);
    if (evaluator_ && dimX_ > 0 && dimY_ > 0) {
        scaledWidth_ = this->width() / dimX_;
        scaledHeight_ = this->height() / dimY_;
    }
}

void RenderTargetViewer::paintGL() {
    if (!isValid())
        return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glClearColor(0.7, 0.7, 0.7, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor4f(1.0, 1.0, 1.0, 1.0);

    LGL_ERROR;

    if (!evaluator_) {
        glPopAttrib();
        return;
    }

    std::vector<RenderPort*> renderPorts = collectRenderPorts();

    // update window title
    int memsize = 0;
    for (size_t i = 0; i < renderPorts.size(); ++i) {
        RenderTarget* rt = renderPorts[i]->getRenderTarget();
        if (rt->getColorTexture())
            memsize += rt->getColorTexture()->getSizeOnGPU();
        if (rt->getDepthTexture())
            memsize += rt->getDepthTexture()->getSizeOnGPU();
    }
    memsize /= 1024 * 1024;
    QString title = tr("Render Target Viewer: %1 Render Targets (%2 mb)").arg(renderPorts.size()).arg(memsize);
    if (parentWidget() && parentWidget()->parentWidget())
        parentWidget()->parentWidget()->setWindowTitle(title);
    else
        setWindowTitle(title);

    // render port contents
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glOrtho(0,width(),0,height(),-1,1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if (renderPorts.empty()) {
        renderFont(13, 13, "No rendertargets selected.");
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glPopAttrib();
        return;
    }

    // update layout dimensions
    dimX_ = (int)ceil(sqrt((float)renderPorts.size()));
    dimY_ = ceil((float)renderPorts.size() / dimX_);

    scaledWidth_ = width() / dimX_;
    scaledHeight_ = height() / dimY_;

    LGL_ERROR;

    if (fullscreen_) {
        if(selected_ >= 0 && selected_ < (int)renderPorts.size()) {
            paintPort(renderPorts[selected_], selected_);
        }
    }
    else {
        for (int y = 0; y < dimY_; ++y) {
            for (int x = 0; x < dimX_; ++x) {
                int index = (dimX_*y)+x;
                if (index >= static_cast<int>(renderPorts.size()))
                    break;

                glPushMatrix();
                glTranslatef(scaledWidth_ * x, scaledHeight_ * (dimY_ - 1 - y), 0.0);
                paintPort(renderPorts[index], static_cast<int>(index));
                glPopMatrix();
            }
        }
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    LGL_ERROR;

    glPopAttrib();
}

void RenderTargetViewer::paintPort(RenderPort* rp, int index) {

    if (!rp || !rp->getRenderTarget())
        return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    RenderTarget* rt = rp->getRenderTarget();

    // set default display type if nothing has been set before
    if(showType_[index] == 0)
        showType_[index] = Color|R|G|B|A|BackgroundBlack;

    // calculate dimension
    currentWidth_ = width();
    currentHeight_ = height();
    if (!fullscreen_) {
        currentWidth_ = scaledWidth_;
        currentHeight_ = scaledHeight_;
    }

    // calculate scaling
    if (keepAspectRatioACT_->isChecked()) {
        float scaleX = 1.0f;
        float scaleY = 1.0f;
        tgt::ivec2 size = rt->getSize();

        float aspectRatioRT = (float)size.x / size.y;
        if(currentHeight_ * aspectRatioRT > currentWidth_) {
            scaleY = (currentWidth_ / aspectRatioRT) / currentHeight_;
        }
        else {
            scaleX = (currentHeight_ * aspectRatioRT) / currentWidth_;
        }
        currentWidth_ = static_cast<int>(currentWidth_ * scaleX);
        currentHeight_ = static_cast<int>(currentHeight_ * scaleY);
    }

    glPushMatrix();
    glLoadIdentity();

    if (fullscreen_) {

        glScalef(zoomScale_, zoomScale_, 1.0);
        glTranslatef(zoomTranslateX_, zoomTranslateY_, 0);

        // fetch depthBuffer value under cursor
        minDepth_ = 0;
        maxDepth_ = 1;
        renderTargetToTexture(rt, Depth, colorTex_);
        fbo_->activate();
        fbo_->attachTexture(colorTex_);
        fbo_->isComplete();
        GLfloat depthBuffer;
        glReadPixels(mouseX_, mouseY_, 1, 1, GL_RED, GL_FLOAT, &depthBuffer);
        currentDepthBuffer_ = pow(depthBuffer, 50);
        fbo_->deactivate();

        // fetch texel value under cursor
        renderTargetToTexture(rt, Color|R|G|B|A|BackgroundBlack, colorTex_);
        fbo_->activate();
        fbo_->attachTexture(rp->getColorTexture());
        fbo_->isComplete();

        tgt::vec2 p = (tgt::vec2(mouseX_, mouseY_) / tgt::vec2(currentWidth_, currentHeight_));
        p /= zoomScale_;
        p -= tgt::vec2(zoomTranslateX_, zoomTranslateY_) / tgt::vec2(currentWidth_, currentHeight_);

        if(p.x < 1.f && p.x > 0.f && p.y < 1.f && p.y > 0.f) {
            tgt::ivec2 origPos = p * tgt::vec2(rp->getColorTexture()->getDimensions().xy());

            // The following are the internal formats used in RenderTarget.  If other formats are added in that class, this will have to be extended.
            if(rp->getColorTexture()->getInternalFormat() == GL_RGB) {
                tgt::Vector3<GLubyte> val;
                glReadPixels(origPos.x, origPos.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, val.elem);
                currentTexel_ = tgt::vec4(tgt::vec3(val), 255.f);
                currentValueScale_ = 255.f;
            }
            else if(rp->getColorTexture()->getInternalFormat() == GL_RGBA) {
                tgt::Vector4<GLubyte> val;
                glReadPixels(origPos.x, origPos.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, val.elem);
                currentTexel_ = tgt::vec4(val);
                currentValueScale_ = 255.f;
            }
            else if(rp->getColorTexture()->getInternalFormat() == GL_RGBA16) {
                tgt::Vector4<GLushort> val;
                glReadPixels(origPos.x, origPos.y, 1, 1, GL_RGBA, GL_UNSIGNED_SHORT, val.elem);
                currentTexel_ = tgt::vec4(val);
                currentValueScale_ = 65535.f;
            }
            else if(rp->getColorTexture()->getInternalFormat() == GL_R32UI) {
                GLuint val;
                glReadPixels(origPos.x, origPos.y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &val);
                currentTexel_ = tgt::vec4(val,0.f,0.f,0.f);
                currentValueScale_ = 4294967295.f;
            }
            else {  // all float formats are treated equally
                glReadPixels(origPos.x, origPos.y, 1, 1, GL_RGBA, GL_FLOAT, currentTexel_.elem);
                currentValueScale_ = 1.f;
            }
        }

        fbo_->detachAll();
        fbo_->deactivate();
    }

    // render current type
    if(!fullscreen_ || showType_[index] != (Color|R|G|B|A|BackgroundBlack))
        renderTargetToTexture(rt, showType_[index], colorTex_);

    if((showType_[index] & Depth) == Depth) {
        minDepth_ = 0;
        maxDepth_ = 1;
        renderTargetToTexture(rt, Depth, colorTex_);
        fbo_->activate();
        fbo_->attachTexture(colorTex_);
        fbo_->isComplete();
        size_t size = colorTex_->getWidth() * colorTex_->getHeight();
        GLfloat* texelBuffer = new GLfloat[size];
        glReadPixels(0, 0, colorTex_->getWidth(), colorTex_->getHeight(), GL_RED, GL_FLOAT, texelBuffer);
        fbo_->deactivate();
        minDepth_ = 1;
        maxDepth_ = 0;
        for(size_t i=0; i < size; i++) {
            if(texelBuffer[i] >= 0)
                minDepth_ = std::min(minDepth_, texelBuffer[i]);
            maxDepth_ = std::max(maxDepth_, texelBuffer[i]);
        }
        maxDepth_ = std::max(maxDepth_, minDepth_);
        delete[] texelBuffer;
        renderTargetToTexture(rt, Depth, colorTex_);
    }

    glPopMatrix();

    renderInfosToFontTexture(rt);

    // combine color-/font-texture by shader
    paintCombinedTextures();

    // draw red line bounds around selected render target
    if (!fullscreen_ && index == selected_)
        paintOutline();

    glPopAttrib();
}

void RenderTargetViewer::renderTargetToTexture(RenderTarget* rt, unsigned int showType, tgt::Texture* texture) {

    tgt::Shader* shaderProgram = colorProgram_;
    if (!shaderProgram)
        return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    tgt::ivec3 size(currentWidth_, currentHeight_, 1);

    glActiveTexture(GL_TEXTURE0);
    // resize textures if necessary
    if (size != texture->getDimensions()) {
        texture->destroy();
        texture->setDimensions(size);
        texture->uploadTexture();
    }

    // activate fbo
    fbo_->activate();
    fbo_->attachTexture(texture);
    fbo_->isComplete();

    glClearColor(0.7, 0.7, 0.7, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    tgtAssert(rt, "No render target");

    if((showType & Depth) == Depth) {
        rt->bindDepthTexture();
    } else {
        rt->bindColorTexture();
    }

    tgtAssert(shaderProgram, "No shader");
    shaderProgram->activate();
    shaderProgram->setUniform("tex_", 0);
    LGL_ERROR;
    shaderProgram->setUniform("enableColorR_", (showType & R) == R);
    shaderProgram->setUniform("enableColorG_", (showType & G) == G);
    shaderProgram->setUniform("enableColorB_", (showType & B) == B);
    shaderProgram->setUniform("enableColorA_", (showType & A) == A);
    shaderProgram->setUniform("showAlpha_", (showType & Alpha) == Alpha);
    shaderProgram->setUniform("showDepth_", (showType & Depth) == Depth);
    shaderProgram->setUniform("showHue_", (showType & H) == H);
    shaderProgram->setUniform("showSaturation_", (showType & S) == S);
    shaderProgram->setUniform("showValue_", (showType & V) == V);
    shaderProgram->setUniform("minDepth_", (float)minDepth_);
    shaderProgram->setUniform("maxDepth_", (float)maxDepth_);
    shaderProgram->setUniform("enableBackgroundCheckerboardPattern_", (showType & CheckerboardPattern) == CheckerboardPattern);
    float bgColor = ((showType & BackgroundWhite) == BackgroundWhite) ? 1.0f : 0.0f;
    shaderProgram->setUniform("backgroundColorR_", bgColor);
    shaderProgram->setUniform("backgroundColorB_", bgColor);
    shaderProgram->setUniform("backgroundColorG_", bgColor);

    LGL_ERROR;
    shaderProgram->setIgnoreUniformLocationError(true);
    shaderProgram->setUniform("texParameters_.dimensions_", tgt::vec2(rt->getSize()));
    shaderProgram->setUniform("texParameters_.dimensionsRCP_", 1.f / tgt::vec2(rt->getSize()));
    shaderProgram->setUniform("texParameters_.matrix_", tgt::mat4::identity);
    shaderProgram->setIgnoreUniformLocationError(false);
    LGL_ERROR;

    renderQuad();

    shaderProgram->deactivate();
    LGL_ERROR;

    fbo_->deactivate();
    LGL_ERROR;

    glPopAttrib();
}

void RenderTargetViewer::renderInfosToFontTexture(RenderTarget* rt) {
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    tgt::ivec3 size(currentWidth_, currentHeight_, 1);

    glLoadIdentity();
    LGL_ERROR;

    glActiveTexture(GL_TEXTURE0);
    // resize texture if necessary
    if (size != fontTex_->getDimensions()) {
        fontTex_->destroy();
        fontTex_->setDimensions(size);
        fontTex_->uploadTexture();
    }
    LGL_ERROR;

    // activate fbo
    fbo_->activate();
    fbo_->attachTexture(fontTex_);
    fbo_->isComplete();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    LGL_ERROR;

    if (showInfosACT_->isChecked()) {

        // render fonts
        QString colorStr = "";
        QString depthStr = "";
        QString sizeStr = tr("%1x%2").arg(rt->getSize().x).arg(rt->getSize().y);
        int deltaY = 15; // Zeilenhöhe

        if (rt->getColorTexture())
            colorStr += internalTypeToString(rt->getColorTexture()->getInternalFormat());
        else
            colorStr += "No color texture";

        if (rt->getDepthTexture())
            depthStr += internalTypeToString(rt->getDepthTexture()->getInternalFormat());
        else
            depthStr += "No depth texture";

        int mem = 0;
        if (rt->getColorTexture())
            mem += rt->getColorTexture()->getSizeOnGPU();
        if (rt->getDepthTexture())
            mem += rt->getDepthTexture()->getSizeOnGPU();
        mem /= 1024;

        QString memStr = tr("%1 kb").arg(mem);
        QString numUpdateStr = tr("@%1").arg(rt->getNumUpdates());
        LGL_ERROR;

        if(mouseIsInside_ || freezeInfos_) {
            if(fullscreen_ && showInfosDetailsACT_->isChecked()){
                int textureCoordX = static_cast<int>((float)(zoomOffsetX_ + (mouseX_ - zoomOffsetX_) / zoomScale_) * rt->getSize().x / currentWidth_);
                int textureCoordY = static_cast<int>((float)(zoomOffsetY_ + (mouseY_ - zoomOffsetY_) / zoomScale_) * rt->getSize().y / currentHeight_);

                if(textureCoordX < rt->getSize().x && textureCoordY < rt->getSize().y) {
                    int numLines = 7;
                    int offsetX = 3;
                    int offsetY = currentHeight_ - numLines * deltaY - 3;

                    renderFont(offsetX, offsetY, tr("Y: %1").arg(textureCoordY));

                    offsetY += deltaY;
                    renderFont(offsetX, offsetY, tr("X: %1").arg(textureCoordX));

                    offsetY += deltaY;
                    renderFont(offsetX, offsetY, tr("D: %1").arg(currentDepthBuffer_));

                    offsetY += deltaY;
                    stringstream ssA;
                    ssA << "A: " << currentTexel_[3] << " " << setprecision(2) << fixed << currentTexel_[3] / currentValueScale_;
                    renderFont(offsetX, offsetY, tr(ssA.str().c_str()));

                    tgt::vec3 hsv = rgbToHsv(currentTexel_.xyz() / currentValueScale_);

                    offsetY += deltaY;
                    stringstream ssB;
                    ssB << "B: " << currentTexel_[2] << " " << setprecision(2) << fixed << currentTexel_[2] / currentValueScale_;
                    ssB << " " << "V: " << fixed << hsv[2];
                    renderFont(offsetX, offsetY, tr(ssB.str().c_str()));

                    offsetY += deltaY;
                    stringstream ssG;
                    ssG << "G: " <<  currentTexel_[1] << " " << setprecision(2) << fixed << currentTexel_[1] / currentValueScale_;
                    ssG << " " << "S: " << fixed << hsv[1];
                    renderFont(offsetX, offsetY, tr(ssG.str().c_str()));

                    offsetY += deltaY;
                    stringstream ssR;
                    ssR << "R: " << currentTexel_[0] << " " << setprecision(2) << fixed << currentTexel_[0] / currentValueScale_;
                    ssR << " " << "H: " << setfill('0') << setw(3) << (int)hsv[0];
                    renderFont(offsetX, offsetY, tr(ssR.str().c_str()));
                }
            }
        }
        LGL_ERROR;

        int offsetX = 3;
        int offsetY = 5;

        renderFont(offsetX, offsetY, tr(rt->getDebugLabel().c_str()));
        offsetY += deltaY;
        renderFont(offsetX, offsetY, depthStr);
        offsetY += deltaY;
        renderFont(offsetX, offsetY, colorStr);
        offsetY += deltaY;
        renderFont(offsetX, offsetY, sizeStr);
        offsetY += deltaY;
        renderFont(offsetX, offsetY, memStr);
        offsetY += deltaY;
        renderFont(offsetX, offsetY, numUpdateStr);
        LGL_ERROR;
    }

    // deactivate fbo
    fbo_->deactivate();

    glPopAttrib();
    glPopMatrix();

    LGL_ERROR;
}

void RenderTargetViewer::paintCombinedTextures() {

    tgt::Shader* shaderProgram = inversecolorProgram_;
    if (!shaderProgram)
        return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glActiveTexture(GL_TEXTURE0);
    LGL_ERROR;
    colorTex_->bind();
    LGL_ERROR;
    glActiveTexture(GL_TEXTURE1);
    LGL_ERROR;
    fontTex_->bind();
    LGL_ERROR;
    glActiveTexture(GL_TEXTURE0);
    LGL_ERROR;

    tgtAssert(shaderProgram, "No shader");
    shaderProgram->activate();
    LGL_ERROR;

    shaderProgram->setUniform("backTex_", 0);
    shaderProgram->setUniform("frontTex_", 1);
    shaderProgram->setUniform("threshold_", 0.15f);
    LGL_ERROR;
    shaderProgram->setIgnoreUniformLocationError(true);
    shaderProgram->setUniform("texParameters_.dimensions_", tgt::vec2(colorTex_->getDimensions().xy()));
    shaderProgram->setUniform("texParameters_.dimensionsRCP_", 1.f / tgt::vec2(colorTex_->getDimensions().xy()));
    shaderProgram->setUniform("texParameters_.matrix_", tgt::mat4::identity);
    shaderProgram->setIgnoreUniformLocationError(false);
    LGL_ERROR;
    renderQuad();
    LGL_ERROR;
    shaderProgram->deactivate();
    LGL_ERROR;

    glPopAttrib();
}

void RenderTargetViewer::paintOutline() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(1, 1);
    glVertex2f(currentWidth_, 1);
    glVertex2f(currentWidth_, currentHeight_);
    glVertex2f(1, currentHeight_);
    glEnd();
    LGL_ERROR;

    glPopAttrib();
}

void RenderTargetViewer::renderQuad() {
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(currentWidth_, 0);
    glTexCoord2f(1, 1); glVertex2f(currentWidth_, currentHeight_);
    glTexCoord2f(0, 1); glVertex2f(0, currentHeight_);
    glEnd();
}

void RenderTargetViewer::renderFont(float x, float y, QString text) {
#ifdef VRN_MODULE_FONTRENDERING
    font_->render(tgt::vec3(x, y, 0.0), text.toStdString());
#else
    renderText(x, y, 0.0, text, font_);
#endif
    LGL_ERROR;
}

void RenderTargetViewer::takeScreenshot(bool withOverlay) {
    QString path;

    //tgtAssert(initialized_, "Not initialized");
    tgtAssert(evaluator_, "No evaluator");

    if (!fullscreen_) {
        // should not get here
        QMessageBox::critical(this, tr("Error saving screenshot"), tr("Only supported in fullscreen."));
        return;
    }

    std::vector<RenderPort*> renderPorts = collectRenderPorts();

    tgtAssert(selected_ >= 0 && selected_ < (int)renderPorts.size(), "Invalid render port index");

    QFileDialog filedialog(this);
    filedialog.setWindowTitle(tr("Save Screenshot"));
    filedialog.setDirectory(VoreenApplication::app()->getUserDataPath("screenshots").c_str());
    filedialog.setDefaultSuffix(tr("png"));

    QStringList filter;
    filter << tr("PNG image (*.png)");
    filter << tr("JPEG image (*.jpg)");
    filedialog.setFilters(filter);
    filedialog.setAcceptMode(QFileDialog::AcceptSave);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getUserDataPath("screenshots").c_str());
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    filedialog.setSidebarUrls(urls);

    struct tm* Tm;
    time_t currentTime = time(NULL);
    Tm = localtime(&currentTime);
    std::stringstream timestamp;
    timestamp << "screenshot " << (Tm->tm_year+1900) << "-" << (Tm->tm_mon+1) << "-" << Tm->tm_mday << "-" << Tm->tm_hour << "-" << Tm->tm_min << "-" << Tm->tm_sec;
    timestamp << ".png";
    filedialog.selectFile(tr(timestamp.str().c_str()));

    QStringList fileList;
    if (filedialog.exec())
        fileList = filedialog.selectedFiles();
    if (fileList.empty())
        return;

    path = filedialog.directory().absolutePath();

    if (!fileList.at(0).endsWith(".jpg") && !fileList.at(0).endsWith(".png")) {
        std::string text = "Screenshot could not be saved.\n";
        int index = fileList[0].lastIndexOf(".");
        if ((index == -1) || (index+1 == fileList[0].size()))
            text += "No file extension specified.";
        else
            text += "Invalid file extension: " + fileList[0].right(fileList[0].size()-index-1).toStdString();

        QMessageBox::critical(this, tr("Error saving screenshot"), tr(text.c_str()));
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    qApp->processEvents();
    try {
        try {
            if(withOverlay) {
                if(!grabFrameBuffer(true).save(tr(fileList.at(0).toStdString().c_str())))
                    throw VoreenException("Could not grab framebuffer.");
            } else {
                // repaint the widget without overlay if necessary, then grab screenshot
                bool showInfos = false;
                if(showInfosACT_->isChecked()) {
                    showInfos = true;
                    showInfosACT_->setChecked(false);
                    updateGL();
                }

                if(!grabFrameBuffer(true).save(tr(fileList.at(0).toStdString().c_str())))
                    throw VoreenException("Could not grab framebuffer.");

                if(showInfos)
                    showInfosACT_->setChecked(true);

            }
        }
        catch (const VoreenException& e) {
            QString text = tr("Screenshot could not be saved:\n%1").arg(e.what());
            QMessageBox::warning(this, tr("Error saving screenshot"), text);
        }
    }
    catch (const std::exception& e) {
        QString text = tr("Screenshot could not be saved:\n%1").arg(e.what());
        QMessageBox::warning(this, tr("Error saving screenshot"), text);
    }
    QApplication::restoreOverrideCursor();
}

void RenderTargetViewer::openSubWindow() {
    if(parentWidget() && parentWidget()->parentWidget() && parentWidget()->parentWidget()->parentWidget()) {
        RenderTargetViewer* renderTargetViewer = new RenderTargetViewer(sharedContext_);
        renderTargetViewer->setSubWidget(selected_, showType_[selected_]);
        renderTargetViewer->setEvaluator(evaluator_);
        renderTargetViewer->setMinimumSize(200, 200);
        VoreenToolWindow* window = new VoreenToolWindow(0, parentWidget()->parentWidget()->parentWidget(), renderTargetViewer, "RenderTargetViewer", false);
        window->widget()->setContentsMargins(0,0,0,0);
        window->widget()->layout()->setContentsMargins(0,0,0,0);
        window->resize(500, 500);
        window->show();
    }
}

tgt::vec3 RenderTargetViewer::rgbToHsv(tgt::vec3 rgb) {
    tgt::vec3 hsv;
    float min = std::min(rgb.r, std::min(rgb.g, rgb.b));
    float max = std::max(rgb.r, std::max(rgb.g, rgb.b));
    float diff = max - min;
    if(max == min)
        hsv[0] = 0;
    else if(max == rgb.r)
        hsv[0] = 60 * (0 + rgb.g - rgb.b / diff);
    else if(max == rgb.g)
        hsv[0] = 60 * (2 + rgb.b - rgb.r / diff);
    else if(max == rgb.b)
        hsv[0] = 60 * (4 + rgb.r - rgb.g / diff);
    if(hsv[0] < 0)
        hsv[0] += 360;
    if(max == 0)
        hsv[1] = 0;
    else
        hsv[1] = diff / max;
    hsv[2] = max;
    return hsv;
}


} // namespace voreen
