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

#include "canvasrendererwidget.h"

#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/utils/voreenpainter.h"
#include "voreen/qt/widgets/screenshotplugin.h"
#include "voreen/qt/voreenapplicationqt.h"

#include <QDialog>
#include <QFileDialog>
#include <QGridLayout>
#include <QMainWindow>
#include <QMdiSubWindow>

namespace voreen {

CanvasRendererWidget::CanvasRendererWidget(QWidget* parent, CanvasRenderer* canvasRenderer)
    : QProcessorWidget(canvasRenderer, parent)
    , canvasWidget_(0)
    , screenshotTool_(0)
{
    tgtAssert(canvasRenderer, "No CanvasRenderer");

    setWindowTitle(QString::fromStdString(canvasRenderer->getGuiName()));
    resize(256, 256);
}

CanvasRendererWidget::~CanvasRendererWidget() {
    delete screenshotTool_;

    // deregister canvas at owning CanvasRenderer
    CanvasRenderer* canvasRenderer = dynamic_cast<CanvasRenderer*>(processor_);
    if (canvasRenderer)
        canvasRenderer->setCanvas(0);

    if (canvasWidget_)
        delete canvasWidget_->getPainter();
    delete canvasWidget_;

}

void CanvasRendererWidget::initialize() throw (VoreenException){
    QProcessorWidget::initialize();

    CanvasRenderer* canvasRenderer = dynamic_cast<CanvasRenderer*>(processor_);
    tgtAssert(canvasRenderer, "CanvasRenderer expected");

    if (!VoreenApplication::app()) {
        LERRORC("voreen.qt.CanvasRendererWidget", "VoreenApplication not instantiated");
        throw VoreenException("VoreenApplication not instantiated");
    }
    NetworkEvaluator* evaluator = VoreenApplication::app()->getNetworkEvaluator(processor_);
    if (!evaluator) {
        LERRORC("voreen.qt.CanvasRendererWidget", "No evaluator assigned to VoreenApplication");
        throw VoreenException("No evaluator assigned to VoreenApplication");
    }

    canvasWidget_ = new tgt::QtCanvas("", tgt::ivec2(getSize().x, getSize().y), tgt::GLCanvas::RGBADD, this, true, 0);
    VoreenApplicationQt::qtApp()->sendTouchEventsTo(canvasWidget_);
    canvasWidget_->setMinimumSize(64, 64);
    canvasWidget_->setMouseTracking(true); // for receiving mouse move events without a pressed button

    QGridLayout* layout = new QGridLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(canvasWidget_, 0, 0);
    setLayout(layout);

    //show();

    VoreenPainter* painter = new VoreenPainter(canvasWidget_, evaluator, canvasRenderer);
    canvasWidget_->setPainter(painter);
    painter->initialize();

    canvasRenderer->setCanvas(canvasWidget_);
    initialized_ = true;
}

#ifdef VRN_MODULE_DEVIL
void CanvasRendererWidget::keyPressEvent(QKeyEvent* event) {
    if ((event->modifiers() == Qt::AltModifier) && (event->key() == Qt::Key_P) ) {
        if (!screenshotTool_)
            createScreenshotTool();
        screenshotTool_->show();
        screenshotTool_->setFocus();
    }
#else
void CanvasRendererWidget::keyPressEvent(QKeyEvent* event) {
#endif
    if (event->key() == Qt::Key_F11) {
        if (canvasWidget_ && processor_) {

            //return if it is the first canvas in application mode
            if(dynamic_cast<QMdiSubWindow*>(parent())) return;

            Property* prop = processor_->getProperty("showFullScreen");
            if(prop) {
                BoolProperty* bp = dynamic_cast<BoolProperty*>(prop);
                if(isFullScreen()){
                    bp->set(false);
                } else {
                    bp->set(true);
                }
            }
        }
    }
}

bool CanvasRendererWidget::event(QEvent *event) {
    if(event->type() == QEvent::WinIdChange && canvasWidget_) {
        VoreenApplicationQt::qtApp()->sendTouchEventsTo(canvasWidget_);
        return true;
    }
    return QWidget::event(event);
}

void CanvasRendererWidget::resizeEvent(QResizeEvent* event) {
    QProcessorWidget::resizeEvent(event);
    if (screenshotTool_)
        screenshotTool_->updateFromProcessor();
}

void CanvasRendererWidget::createScreenshotTool() {
    if (screenshotTool_)
        return;

    screenshotTool_ = new ScreenshotPlugin(VoreenApplicationQt::qtApp()->getMainWindow(),
                                       dynamic_cast<CanvasRenderer*>(processor_));
    screenshotTool_->adjustSize();
    screenshotTool_->setFixedSize(screenshotTool_->sizeHint());
}

void CanvasRendererWidget::showScreenshotTool() {
    if (!screenshotTool_)
        createScreenshotTool();
    screenshotTool_->show();
    screenshotTool_->raise();
}

void CanvasRendererWidget::updateFromProcessor() {
    if (canvasWidget_ && processor_) {
        // handle show canvas
        BoolProperty* showCanvasProp = dynamic_cast<BoolProperty*>(processor_->getProperty("showCanvas"));
        if (showCanvasProp) {
            setVisible(showCanvasProp->get());
        }

        // handle show cursor
        Property* prop = processor_->getProperty("showCursor");
        if(prop) {
            BoolProperty* bp = dynamic_cast<BoolProperty*>(prop);
            if(bp) {
                if(bp->get())
                    canvasWidget_->setCursor(Qt::ArrowCursor);
                else
                    canvasWidget_->setCursor(Qt::BlankCursor);
            }
        }
        // handle fullscreen
        prop = processor_->getProperty("showFullScreen");
        if(prop) {
            BoolProperty* bp = dynamic_cast<BoolProperty*>(prop);
            if(bp) {
                if(bp->get()){
                    if(!isFullScreen()){
                        if (processor_->isInitialized()) { //to not overwrite values if started in fullscreen
                            //store prefered size/pos
                            IVec2MetaData* sizeMeta = new IVec2MetaData(getSize());
                            processor_->getMetaDataContainer().addMetaData("preferedNoFullscreenCanvasSize",sizeMeta);
                            IVec2MetaData* positionMeta = new IVec2MetaData(getPosition());
                            processor_->getMetaDataContainer().addMetaData("preferedNoFullscreenCanvasPosition",positionMeta);
                        }
                        //set fullscreen
                        setWindowState(windowState() | Qt::WindowFullScreen);
                    }
                }
                else {
                    if(isFullScreen()){
                        showNormal();
                        //restore position
                        IVec2MetaData* positionMeta = dynamic_cast<IVec2MetaData*>(processor_->getMetaDataContainer().getMetaData("preferedNoFullscreenCanvasPosition"));
                        if (!positionMeta)
                            LDEBUGC("voreen.core.io.output.CanvasRendererWidget", "updateFromProcessor(): No meta data object returned");
                        else
                            setPosition(positionMeta->getValue().x, positionMeta->getValue().y);
                        //restore size
                        IVec2MetaData* sizeMeta = dynamic_cast<IVec2MetaData*>(processor_->getMetaDataContainer().getMetaData("preferedNoFullscreenCanvasSize"));
                        if (!sizeMeta)
                            LDEBUGC("voreen.core.io.output.CanvasRendererWidget", "updateFromProcessor(): No meta data object returned");
                        else
                            setSize(sizeMeta->getValue().x, sizeMeta->getValue().y);
                    }
                }
            }
        }
    }
    QProcessorWidget::updateFromProcessor();
}

} //namespace voreen
