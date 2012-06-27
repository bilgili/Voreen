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
#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/io/ioprogress.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

RptPainterWidget::RptPainterWidget(QWidget* parent)
    : tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, 0, true, 0)
    , tgt::EventListener()
    , ioSystem_(new IOSystem(parent))
    , volumeSerializerPopulator_(ioSystem_->getObserver())
{
	
}

void RptPainterWidget::setEvaluator(NetworkEvaluator* evaluator) {
	eval=evaluator;
	eval->setSize(tgt::ivec2(getWidth(), getHeight()));

    painter_->setEvaluator(evaluator);

	repaint();
}

void RptPainterWidget::init(TextureContainer* tc,tgt::Camera* camera) {
    tc_ = tc;

    camera_ = camera;
    setCamera(camera_);

    trackball_ = new tgt::Trackball(this, false);
    trackball_->setCenter(tgt::vec3(0.f));
    trackball_->setMouseMove();

    // set initial orientation to coronal view
    float c = 0.5f * sqrtf(2.f);
    tgt::quat q = tgt::quat(c, 0.f, 0.f, c);
    //trackball_->rotate(q);

    trackNavi_ = new TrackballNavigation(trackball_, true, 0.05f, 15.f);
    MsgDistr.insert(trackNavi_);
    getEventHandler()->addListenerToBack(trackNavi_);
    getEventHandler()->addListenerToFront(this);
    startTimer(10);

    painter_ = new voreen::VoreenPainter(this, trackball_, "mainview");  
    MsgDistr.insert(painter_);
    eval ->setTextureContainer(tc_); 

    painter_->setEvaluator(eval);
    setPainter(painter_);
    getGLFocus();
    painter_->paint();
}

voreen::TextureContainer* RptPainterWidget::getTextureContainer(){
    return tc_;
}

} //namespace voreen
