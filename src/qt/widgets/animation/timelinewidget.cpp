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

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

#include "voreen/core/animation/animatedprocessor.h"
#include "voreen/core/animation/templatepropertytimeline.h"
#include "voreen/core/animation/animatedprocessor.h"
#include "voreen/core/animation/interpolationfunctionfactory.h"
#include "voreen/core/animation/interpolation/intinterpolationfunctions.h"

#include "voreen/qt/widgets/animation/overviewwidget.h"
#include "voreen/qt/widgets/animation/animationeditor.h"
#include "voreen/qt/widgets/animation/timelinewidget.h"
#include "voreen/qt/widgets/animation/processortimelinewidget.h"
#include "voreen/qt/widgets/animation/propertytimelinewidget.h"


namespace voreen {

TimelineWidget::TimelineWidget(Animation* animation, AnimationEditor* parent, NetworkEvaluator* networkEval)
        : QWidget(parent)
        , animation_(animation)
        , currentTime_(0)
        , changed_(false)
{
    // register as observer in the core
    animation_->addObserver(this);
    setMinimumWidth(550);

    mainLayout_ = new QVBoxLayout(this);
    QHBoxLayout* controlLayout = new QHBoxLayout();

    timeBox_ = new QGroupBox(tr("Time"), this);
    QHBoxLayout* timeLayout = new QHBoxLayout();
    timeBox_->setLayout(timeLayout);
    timeCounter_ = new QLabel(this);
    timeCounter_->setText("00:00");
    timeBox_->setFixedWidth(timeBox_->width());
    timeLayout->addWidget(timeCounter_);

    mainLayout_->addLayout(controlLayout);
    controlLayout->addWidget(timeBox_);
    controlLayout->addSpacerItem(new QSpacerItem(180,0));
    overviewTimeline_ = new OverviewWidget(this, networkEval);
    controlLayout->addWidget(overviewTimeline_);

    connect(this, SIGNAL(viewResizeSignal(int)), overviewTimeline_, SLOT(updateViewportRect(int)));
    connect(this, SIGNAL(autoPreview(bool)), overviewTimeline_, SLOT(autoPreview(bool)));
    connect(this, SIGNAL(updatePreviews()), overviewTimeline_, SLOT(updatePreviews()));
    connect(this, SIGNAL(durationChanged(int)), overviewTimeline_, SLOT(setDuration(int)));
    connect(overviewTimeline_, SIGNAL(currentFrameChanged(int)), this, SIGNAL(currentFrameChanged(int)));
    connect(overviewTimeline_, SIGNAL(recordAt(int)), this, SIGNAL(recordAt(int)));
    connect(parent, SIGNAL(currentFrameChanged(int)), overviewTimeline_, SLOT(setCurrentFrame(int)));
    connect(overviewTimeline_, SIGNAL(currentFrameChanged(int)), parent, SLOT(setCurrentFrame(int)));
    connect(this, SIGNAL(scrollBarOrder(int)), overviewTimeline_, SLOT(scrollBarOrder(int)));
    connect(this, SIGNAL(sceneOrder(QMatrix)), overviewTimeline_, SLOT(sceneOrder(QMatrix)));


    mainLayout_->setMargin(1);
    mainLayout_->setSpacing(1);
    mainLayout_->setAlignment(Qt::AlignTop);

    scrollArea_ = new QScrollArea();
    containerWidget_ = new QWidget(scrollArea_);
    scrollArea_->setWidget(containerWidget_);
    scrollAreaLayout_ = new QVBoxLayout(containerWidget_);;
    scrollAreaLayout_->setSizeConstraint(QLayout::SetMinimumSize);
    mainLayout_->addWidget(scrollArea_);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setAlignment(Qt::AlignTop);

    populateProcessors();
}

void TimelineWidget::rebuildAnimation(Animation* animation) {
    animation_ = animation;
    animation_->addObserver(this);
    for(size_t i= 0; i < processorTimelineWidgets_.size(); ++i ) {
        if(processorTimelineWidgets_.at(i)->getAnimatedProcessor() != 0) {
            processorTimelineWidgets_.at(i)->hide();
            processorTimelineWidgets_.at(i)->setEnabled(false);
            processorTimelineWidgets_.at(i)->setFixedHeight(0);
            processorTimelineWidgets_.at(i)->deleteLater();
        }
    }
    processorTimelineWidgets_.clear();
    populateProcessors();

    overviewTimeline_->setStandardRenderport();
    overviewTimeline_->reset();
    emit setAnimationEditorDuration(static_cast<int>(animation_->getDuration() * 30.0f));
    emit(resizeSignal(scrollArea_->width()));
    if (!animation_->isEmpty()) {
        emit showActiveTimelines();
        overviewTimeline_->renderPreviews();
    }

}

void TimelineWidget::populateProcessors() {
    const std::vector<AnimatedProcessor*> procs = animation_->getAnimatedProcessors();

    for(size_t i=0; i<procs.size();i++) {
        if (procs.at(i)->getPropertyTimelines().size() != 0) {
            animatedProcessorAdded(procs.at(i));
        }
    }
}

void TimelineWidget::reloadAnimation() {
    // Implement
}

void TimelineWidget::animatedProcessorAdded(const AnimatedProcessor* processor) {
    AnimatedProcessor* proc = const_cast<AnimatedProcessor*>(processor);    // Maybe we should do this another way, but today we feel quick and dirty
    ProcessorTimelineWidget* processorTimelineWidget = new ProcessorTimelineWidget(processor->getProcessorName(), proc, animation_, timeBox_->width()+172, this);
    scrollAreaLayout_->addWidget(processorTimelineWidget);
    processorTimelineWidgets_.push_back(processorTimelineWidget);

    connect(processorTimelineWidget, SIGNAL(sceneRequest(QMatrix)), this, SIGNAL(sceneOrder(QMatrix)));
    connect(processorTimelineWidget, SIGNAL(scrollBarRequest(int)), this, SIGNAL(scrollBarOrder(int)));
    connect(processorTimelineWidget, SIGNAL(renderAt(float)), this, SLOT(renderAt(float)));
    connect(processorTimelineWidget, SIGNAL(keyframeAdded()), overviewTimeline_, SLOT(renderPreviews()));
    connect(processorTimelineWidget, SIGNAL(keyframeChanged()), overviewTimeline_, SLOT(renderPreviews()));
    connect(processorTimelineWidget, SIGNAL(removeProcessorTimelineWidget(ProcessorTimelineWidget*)), this, SLOT(removeProcessorTimelineWidget(ProcessorTimelineWidget*)));
    connect(overviewTimeline_, SIGNAL(barMovement(int)), processorTimelineWidget, SIGNAL(barMovement(int)));
    connect(this, SIGNAL(sceneOrder(QMatrix)), processorTimelineWidget, SIGNAL(sceneOrder(QMatrix)));
    connect(this, SIGNAL(scrollBarOrder(int)), processorTimelineWidget, SIGNAL(scrollBarOrder(int)));
    connect(this, SIGNAL(currentFrameChanged(int)), processorTimelineWidget, SIGNAL(currentFrameChanged(int)));
    connect(this, SIGNAL(recordSignal()), processorTimelineWidget, SLOT(showAnimatedPropertyTimelines()));
    connect(this, SIGNAL(showActiveTimelines()),  processorTimelineWidget, SLOT(showAnimatedPropertyTimelines()));

    connect(this, SIGNAL(resizeSignal(int)),processorTimelineWidget, SLOT(setFixedWidthSlot(int)));

    connect(this, SIGNAL(durationChanged(int)), processorTimelineWidget, SIGNAL(durationChanged(int)));
    connect(processorTimelineWidget, SIGNAL(viewResizeSignal(int)), this, SIGNAL(viewResizeSignal(int)));
    connect(processorTimelineWidget, SIGNAL(viewFrameChange(int)), overviewTimeline_, SIGNAL(currentFrameChanged(int)));

    // Disable canvas size for animation
    std::vector<PropertyTimeline*> vec = proc->getPropertyTimelines();
    std::vector<PropertyTimeline*>::iterator it = vec.begin();
    while (it != vec.end()) {
        if ((*it)->getPropertyName() == "Canvas Size" && dynamic_cast<TemplatePropertyTimeline<tgt::ivec2>*>(*it))
            (*it)->setActiveOnRendering(false);
        ++it;
    }
}

void TimelineWidget::removeProcessorTimelineWidget(ProcessorTimelineWidget* ptlw) {
    if(ptlw->getTimelineCount() > 0) {
        LWARNINGC("voreen.TimelineWidget", "Trying to remove non-empty ProcessorTimelineWidget");
        return;
    }

    for(int i=0; i<scrollAreaLayout_->count(); i++) {
        QLayoutItem* li = scrollAreaLayout_->itemAt(i);
        QWidget* qw = li->widget();
        if(qw) {
            if(ptlw == qw) {
                scrollAreaLayout_->takeAt(i);
                break;
            }
        }
    }

    std::vector<ProcessorTimelineWidget*>::iterator it;
    for(it = processorTimelineWidgets_.begin(); it != processorTimelineWidgets_.end(); it++) {
        if((*it) == ptlw) {
            processorTimelineWidgets_.erase(it);
            ptlw->deleteLater();
            return;
        }
    }
}

void TimelineWidget::animatedProcessorRemoved(const AnimatedProcessor* processor) {
    for (size_t i= 0; i < processorTimelineWidgets_.size(); i++) {
        if (processor == processorTimelineWidgets_.at(i)->getAnimatedProcessor()) {
            scrollAreaLayout_->removeWidget(processorTimelineWidgets_.at(i));
            processorTimelineWidgets_.at(i)->hide();
            processorTimelineWidgets_.at(i)->deleteLater();
            processorTimelineWidgets_.erase(processorTimelineWidgets_.begin() + i);
            break;
        }
        scrollAreaLayout_->update();
    }
}

float TimelineWidget::getCurrentTime() {
    return currentTime_;
}

void TimelineWidget::setCurrentTime(float time) {
    currentTime_ = time;
}

void TimelineWidget::renderAt(float time) {
        animation_->renderAt(time);
}

void TimelineWidget::currentFrame(int frame) {
    timeCounter_->setText(getTimeString(frame));
}

void TimelineWidget::checkForChanges() {
    if (overviewTimeline_->getCurrentFrame() == 0 && !changed_) {
        for (size_t i= 0; i < processorTimelineWidgets_.size(); i++) {
            if(processorTimelineWidgets_.at(i)->getTimelineCount() != 0)
                changed_ = true;
            break;
        }

        if (!changed_) {
            QMessageBox* box = new QMessageBox(this);
            box->setText(tr("The animation is already up to date. "
                            "To show the timelines either modify properties and "
                            "press the snapshot button or use the buttons in front "
                            "of the processor name to add timelines manually."));
            box->exec();
        }
    }
}

QString TimelineWidget::getTimeString(int frame) {
    int min = 0;
    int sec = frame / 30;

    while(sec >= 60) {
        sec -= 60;
        min++;
    }
    QString minutes;
    QString seconds;
    if(min >= 10)
        minutes = QString::number(min);
    else
        minutes = "0"+QString::number(min);
    if(sec >= 10)
        seconds = QString::number(sec);
    else
        seconds = "0"+QString::number(sec);

    QString ret = minutes+":"+seconds;
    return ret;
}

void TimelineWidget::resizeEvent(QResizeEvent* event) {
    emit(resizeSignal(scrollArea_->width()));
    QWidget::resizeEvent(event);
}

} // namespace voreen

