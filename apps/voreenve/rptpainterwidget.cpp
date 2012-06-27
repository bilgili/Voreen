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

#include "rptpainterwidget.h"
#include "voreen/core/vis/messagedistributor.h"
#include "tgt/navigation/trackball.h"

#include "tgt/qt/qttimer.h"

namespace voreen {

RptPainterWidget::RptPainterWidget(QWidget* parent, RptPainterWidget::CameraNavigation navigation)
    : tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, parent, true, 0)
    , tgt::EventListener()
    , currentNavigation_(navigation)
{
    canvasDetached_ = false;
}

RptPainterWidget::~RptPainterWidget() {
    delete trackNavi_;
    delete flythroughNavi_;
    delete painter_;
}

bool RptPainterWidget::setEvaluator(NetworkEvaluator* evaluator) {
    eval = evaluator;
    eval->setSize(tgt::ivec2(getWidth(), getHeight()));

    bool result = painter_->setEvaluator(evaluator);

    repaint();

    return result;
}

void RptPainterWidget::closeEvent(QCloseEvent* e) {
    e->ignore();
    //emit attachSignal();
}

void RptPainterWidget::hideEvent(QHideEvent* e) {
    e->ignore();
    //emit attachSignal();
}

TrackballNavigation* RptPainterWidget::getTrackballNavigation() const {
    return trackNavi_;
}

FlythroughNavigation* RptPainterWidget::getFlythroughNavigation() const {
    return flythroughNavi_;
}

void RptPainterWidget::setCurrentNavigation(RptPainterWidget::CameraNavigation navi) {

    if (currentNavigation_ != navi) {
        
        currentNavigation_ = navi;

        if (currentNavigation_ == TRACKBALL_NAVIGATION) {
            MsgDistr.remove(flythroughNavi_);
            getEventHandler()->removeListener(flythroughNavi_);
            MsgDistr.insert(trackNavi_);
            getEventHandler()->addListenerToBack(trackNavi_);
        }
        else if (currentNavigation_ == FLYTHROUGH_NAVIGATION) {
            MsgDistr.remove(trackNavi_);
            getEventHandler()->removeListener(trackNavi_);
            MsgDistr.insert(flythroughNavi_);
            getEventHandler()->addListenerToBack(flythroughNavi_);
        }
    }
}

RptPainterWidget::CameraNavigation RptPainterWidget::getCurrentNavigation() const {

    return currentNavigation_;
}


VoreenPainter* RptPainterWidget::getPainter() {
    return painter_;
}

void RptPainterWidget::init(TextureContainer* tc, tgt::Camera* camera) {
    tc_ = tc;

    camera_ = camera;
    setCamera(camera_);

    tgt::EventHandler* timeHandler = new tgt::EventHandler();
    tgt::Trackball* trackball = new tgt::Trackball(this, false, new tgt::QtTimer(timeHandler));
    trackball->setCenter(tgt::vec3(0.f));
    trackball->setMouseMove();

    trackNavi_ = new TrackballNavigation(trackball, true, 0.05f, 15.f);
    flythroughNavi_ = new FlythroughNavigation(this);
    
    if (currentNavigation_ == TRACKBALL_NAVIGATION) {
        MsgDistr.insert(trackNavi_);
        getEventHandler()->addListenerToBack(trackNavi_);
    }
    else if (currentNavigation_ == FLYTHROUGH_NAVIGATION) {
        MsgDistr.insert(flythroughNavi_);
        getEventHandler()->addListenerToBack(flythroughNavi_);
    }

    painter_ = new VoreenPainter(this, trackball, "mainview");
    MsgDistr.insert(painter_);
    
    eval->setTextureContainer(tc_); 

    getEventHandler()->addListenerToFront(this);
    startTimer(10);

    painter_->setEvaluator(eval);
    setPainter(painter_); // might be dangerous, as this calls NetworkEvaluator::initializeGL()
    getGLFocus();
    painter_->paint();
}

TextureContainer* RptPainterWidget::getTextureContainer(){
    return tc_;
}

void RptPainterWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        QMainWindow* mainWindow = qobject_cast<QMainWindow*>(parentWidget());
        if (mainWindow->isFullScreen())
            mainWindow->showMaximized();
        else
            mainWindow->showFullScreen();
    } else {
        tgt::QtCanvas::mouseDoubleClickEvent(event);
    }
}

} // namespace
        
