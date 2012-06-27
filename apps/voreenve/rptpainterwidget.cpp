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

#include "rptpainterwidget.h"

#include "voreenmainwindow.h"

#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/trackballnavigation.h"
#include "voreen/core/vis/flythroughnavigation.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/application.h"

#include "tgt/navigation/trackball.h"
#include "tgt/qt/qttimer.h"

#include <QMouseEvent>
#include <QWidget>

namespace voreen {

RptPainterWidget::RptPainterWidget(QWidget* parent, RptPainterWidget::CameraNavigation navigation)
    : tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, parent, true, 0)
    , currentNavigation_(navigation)
    , evaluator_(0)
{
    setMinimumSize(300, 300);
}

RptPainterWidget::~RptPainterWidget() {
    delete trackNavi_;
    delete flythroughNavi_;
    delete painter_;
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

void RptPainterWidget::init(NetworkEvaluator* eval, tgt::Camera* camera) {
    evaluator_ = eval;
    camera_ = camera;

    ShdrMgr.addPath(VoreenApplication::app()->getShaderPath());
    const int finalTarget = 20;
    evaluator_->initTextureContainer(finalTarget);
    
    tgt::EventHandler* timeHandler = new tgt::EventHandler();
    tgt::Trackball* trackball = new tgt::Trackball(this, false, new tgt::QtTimer(timeHandler));
    trackball->setCenter(tgt::vec3(0.0f));
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

    // set interval for timeevents (used by the navigation)
    startTimer(10);

    VoreenPainter* painterTmp = new VoreenPainter(this, trackball, "mainview");
    MsgDistr.insert(painterTmp);
    painterTmp->setEvaluator(evaluator_);
    painter_ = painterTmp;
    // also sets the texturecontainer to apropriate size
    painter_->sizeChanged(size_);
}

void RptPainterWidget::initializeGL() {
    // initialize OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

} // namespace voreen
