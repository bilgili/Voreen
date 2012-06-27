/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/core/animation/animatedprocessor.h"
#include "voreen/core/animation/propertytimeline.h"
#include "voreen/core/animation/templatepropertytimeline.h"

#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/volumeurllistproperty.h"

#include "voreen/qt/widgets/animation/processortimelinewidget.h"
#include "voreen/qt/widgets/animation/propertytimelinewidget.h"
#include "voreen/qt/widgets/animation/templatepropertytimelinewidget.h"

#include "tgt/camera.h"
#include "tgt/vector.h"
#include "voreen/core/properties/shaderproperty.h"

#include <QComboBox>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QMenu>
#include <QFileDialog>

using tgt::Camera;

namespace voreen {

ProcessorTimelineWidget::ProcessorTimelineWidget(std::string name, AnimatedProcessor* animatedProcessor,
                                                 int position,  QWidget* parent)
        : QWidget(parent)
        , animatedProcessor_(animatedProcessor)
        , hiddenTimelines_(true)
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    QHBoxLayout* infoLabelLayout = new QHBoxLayout();
    infoLabelLayout->setAlignment(Qt::AlignLeft);
    timelineLayout_ = new QVBoxLayout();
    timelineWidget_ = new QWidget(this);            // this is a design helper widget for easy
                                                    // hiding of all propertytimlinewidgets
    propertyContainer_ = new QWidget(this);         // encapsulates propertywidgets

    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mainLayout->setAlignment(Qt::AlignTop);

    availablePropertiesMenu_ = new QMenu(this);

    QLabel* processorInfo = new QLabel(this);

    processorInfo->setGeometry(0, 0, 205, 20);
    QIcon icon = QIcon(":/qt/icons/expand-plus.png");
    hidePropertyTimelines_ = new QPushButton(icon, "", processorInfo);
    hidePropertyTimelines_->hide();
    hidePropertyTimelines_->setStyleSheet("QToolButton { border: none; padding: 1px; }");
    hidePropertyTimelines_->setFlat(true);
    hidePropertyTimelines_->setFocusPolicy(Qt::NoFocus);
    hidePropertyTimelines_->setGeometry(10, 10, 10, 10);
    hidePropertyTimelines_->move(position-11,-1);
    hideTimelines();

    processorInfo->setIndent(35);
    processorInfo->setText(QString::fromStdString("<p style=\"font-size:8pt\">"+name+"</p>"));
    processorInfo->setFixedWidth(position);
    processorInfo->setFixedHeight(30);
    infoLabelLayout->addWidget(processorInfo);
    timelineLayout_->addStretch();

    availablePropertiesButton_ = new QPushButton(this);
    availablePropertiesButton_->setIcon(QIcon(":/qt/icons/add.png"));
    availablePropertiesButton_->setFlat(true);
    availablePropertiesButton_->setMaximumWidth(30);
    availablePropertiesButton_->setToolTip(tr("Add a property timeline for this processor"));
    connect(availablePropertiesButton_, SIGNAL(clicked()), this, SLOT(showAvailableProperties()));
    connect(this, SIGNAL(currentFrameChanged(int)), this, SLOT(currentFrameChangedSlot(int)));
    connect(hidePropertyTimelines_, SIGNAL(pressed()),this, SLOT(hideTimelines()) );

    mainLayout->addLayout(infoLabelLayout);
    mainLayout->addWidget(timelineWidget_);
    mainLayout->addWidget(propertyContainer_);
    timelineWidget_->setLayout(timelineLayout_);

    timelineLayout_->setMargin(0);
    timelineLayout_->setSpacing(0);

    initPropertyTimelines();

    infoLabelLayout->setAlignment(Qt::AlignTop);
    populatePropertyMenu();
}

void ProcessorTimelineWidget::initPropertyTimelines() {
   const std::vector<PropertyTimeline*> timelines = animatedProcessor_->getPropertyTimelines();
        for (size_t i=0; i < timelines.size(); i++)
        {
            propertyTimelines_[timelines.at(i)] = false;   // not instantiated yet
        }
}

QSize ProcessorTimelineWidget::sizeHint() {
    return QSize(800, 100);
}

void ProcessorTimelineWidget::setFixedWidthSlot(int width) {
    setFixedWidth(width-30);
}

void ProcessorTimelineWidget::currentFrameChangedSlot(int frame) {
    currentFrame_ = frame;
}

void ProcessorTimelineWidget::createPropertyTimelineWidget(PropertyTimeline* tl) {
    PropertyTimelineWidget* propertyTimelineWidget;
    if(dynamic_cast<TemplatePropertyTimeline<float>*>(tl)) {
        TemplatePropertyTimeline<float>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<float>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<float>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_,  this);
    }
    else if(dynamic_cast<TemplatePropertyTimeline<int>*>(tl)) {
        TemplatePropertyTimeline<int>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<int>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<int>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }
    else if(dynamic_cast<TemplatePropertyTimeline<bool>*>(tl)) {
        TemplatePropertyTimeline<bool>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<bool>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<bool>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }
    else if(dynamic_cast<TemplatePropertyTimeline<tgt::ivec2>*>(tl)) {
        TemplatePropertyTimeline<tgt::ivec2>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<tgt::ivec2>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<tgt::ivec2>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
        if(strcmp(tl->getPropertyName().c_str(), "Canvas Size") == 0)
            propertyTimelineWidget->activateTimeline(false);
    }
    else if(dynamic_cast<TemplatePropertyTimeline<tgt::ivec3>*>(tl)) {
        TemplatePropertyTimeline<tgt::ivec3>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<tgt::ivec3>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<tgt::ivec3>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }
    else if(dynamic_cast<TemplatePropertyTimeline<tgt::ivec4>*>(tl)) {
        TemplatePropertyTimeline<tgt::ivec4>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<tgt::ivec4>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<tgt::ivec4>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }
    else if(dynamic_cast<TemplatePropertyTimeline<tgt::vec2>*>(tl)) {
        TemplatePropertyTimeline<tgt::vec2>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<tgt::vec2>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<tgt::vec2>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }
    else if(dynamic_cast<TemplatePropertyTimeline<tgt::vec3>*>(tl)) {
        TemplatePropertyTimeline<tgt::vec3>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<tgt::vec3>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<tgt::vec3>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }
    else if(dynamic_cast<TemplatePropertyTimeline<tgt::vec4>*>(tl)) {
        TemplatePropertyTimeline<tgt::vec4>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<tgt::vec4>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<tgt::vec4>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }
    else if(dynamic_cast<TemplatePropertyTimeline<std::string>*>(tl)) {
        TemplatePropertyTimeline<std::string>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<std::string>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<std::string>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }
    else if(dynamic_cast<TemplatePropertyTimeline<ShaderSource>*>(tl)) {
        TemplatePropertyTimeline<ShaderSource>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<ShaderSource>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<ShaderSource>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }
    else if(dynamic_cast<TemplatePropertyTimeline<TransFunc*>*>(tl)) {
        TemplatePropertyTimeline<TransFunc*>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<TransFunc*>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<TransFunc*>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }
    // TODO
    /*else if(dynamic_cast<TemplatePropertyTimeline<VolumeCollection*>*>(tl)) {
        TemplatePropertyTimeline<VolumeCollection*>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<VolumeCollection*>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<VolumeCollection*>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }*/
    /*else if(dynamic_cast<TemplatePropertyTimeline<Volume*>*>(tl)) {
        TemplatePropertyTimeline<Volume*>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<Volume*>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<Volume*>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }*/
    else if(dynamic_cast<TemplatePropertyTimeline<Camera>*>(tl)) {
        TemplatePropertyTimeline<Camera>* tempPropTL = dynamic_cast<TemplatePropertyTimeline<Camera>*>(tl);
        propertyTimelineWidget = new TemplatePropertyTimelineWidget<Camera>(tl->getPropertyName().c_str(), tempPropTL, currentFrame_, this);
    }
    else {
        propertyTimelineWidget = new PropertyTimelineWidget("not working", tl, this);      // this should never happen. maybe propertytimelines should be made virtual = 0
    }

    // Checking whether this is a canvas size
    if (!(tl->getPropertyName() == "Canvas Size" && dynamic_cast<TemplatePropertyTimeline<tgt::ivec2>*>(tl)))
        propertyTimelineWidget->activateTimeline(tl->getActiveOnRendering());

    timelineLayout_->addWidget(propertyTimelineWidget);
    propertyTimelineWidget->show();
    if(!hidePropertyTimelines_->isVisible())
        hidePropertyTimelines_->show();

    connect(this, SIGNAL(sceneOrder(QMatrix)), propertyTimelineWidget->getPropertyTimelineView(), SLOT(sceneOrder(QMatrix)));// Request scene trans matrix position up to the AnimationEditor, then drop a order down the chain of command
    //connect(hidePropertyTimelines_, SIGNAL(pressed()),this, SLOT(hideTimelines()) );
    connect(this, SIGNAL(scrollBarOrder(int)), propertyTimelineWidget->getPropertyTimelineView(), SLOT(scrollBarOrder(int)));

    connect(this, SIGNAL(barMovement(int)), propertyTimelineWidget->getPropertyTimelineView(), SLOT(scrollBarOrder(int)));

    connect(propertyTimelineWidget, SIGNAL(renderAt(float)), this, SIGNAL(renderAt(float)));
    connect(propertyTimelineWidget, SIGNAL(keyframeAdded()), this, SIGNAL(keyframeAdded()));
    connect(propertyTimelineWidget->getPropertyTimelineView(), SIGNAL(sceneRequest(QMatrix)), this, SIGNAL(sceneRequest(QMatrix)));
    connect(propertyTimelineWidget->getPropertyTimelineView(), SIGNAL(scrollBarRequest(int)), this, SIGNAL(scrollBarRequest(int)));    // Request scrollbar position up to the AnimationEditor, then drop an order down the chain of command
    connect(propertyTimelineWidget->getPropertyTimelineView(), SIGNAL(addKeyframe(QPointF)), this, SIGNAL(keyframeAdded()));
    connect(propertyTimelineWidget->getPropertyTimelineView(), SIGNAL(frameChange(int)), this, SIGNAL(viewFrameChange(int)));

    connect(this, SIGNAL(currentFrameChanged(int)),propertyTimelineWidget, SLOT(setCurrentFrame(int)));
    connect(this, SIGNAL(durationChanged(int)), propertyTimelineWidget, SLOT(setDuration(int)));
    connect(this, SIGNAL(fpsChanged(int)), propertyTimelineWidget, SLOT(setFps(int)));
    connect(propertyTimelineWidget, SIGNAL(viewResizeSignal(int)), this, SIGNAL(viewResizeSignal(int)));
    emit viewResizeSignal(propertyTimelineWidget->getPropertyTimelineView()->geometry().width());
}

const AnimatedProcessor* ProcessorTimelineWidget::getAnimatedProcessor() const {
    return animatedProcessor_;
}

void ProcessorTimelineWidget::hideTimelines() {
    timelineWidget_->setVisible(hiddenTimelines_);
    propertyContainer_->setVisible(hiddenTimelines_);
    if(!hiddenTimelines_)
        hidePropertyTimelines_->setIcon(QIcon(":/qt/icons/expand-plus.png"));
    else
        hidePropertyTimelines_->setIcon(QIcon(":/qt/icons/expand-minus.png"));
    hiddenTimelines_ = !hiddenTimelines_;
}

void ProcessorTimelineWidget::populatePropertyMenu() {
    availablePropertiesMenu_->clear();
    std::map<PropertyTimeline*, bool>::iterator it = propertyTimelines_.begin();
    while(it != propertyTimelines_.end()) {
        if((*it).second == 0) {
            availablePropertiesMenu_->addAction(QString::fromStdString((*it).first->getPropertyName()));
        }
        it++;
    }
}

void ProcessorTimelineWidget::showPropertyTimeline(QString name) {

    std::map<PropertyTimeline*, bool>::iterator it = propertyTimelines_.begin();
    while(it != propertyTimelines_.end()) {
        if(name == QString::fromStdString((*it).first->getPropertyName())) {
            createPropertyTimelineWidget((*it).first);
            propertyTimelines_[(*it).first] = true;
        }
        it++;
    }
    populatePropertyMenu();
}
int ProcessorTimelineWidget::getTimelineCount() {
    return timelineCount_;
}

void ProcessorTimelineWidget::showAnimatedPropertyTimelines() {
    timelineCount_ = 0;
    const std::vector<PropertyTimeline*> timelines = animatedProcessor_->getPropertyTimelines();
    bool altered = false;
    for (uint i = 0; i < timelines.size(); i++) {
        if(propertyTimelines_[timelines.at(i)] == false) {  // Widget not instantiated yet
            if(timelines.at(i)->isChanged()) {
                ++timelineCount_;
                createPropertyTimelineWidget(timelines.at(i));
                propertyTimelines_[timelines.at(i)] = true;
                altered = true;
            }
        }
    }
    if(altered)
        populatePropertyMenu();
}

void ProcessorTimelineWidget::showAvailableProperties() {
    QAction* action = availablePropertiesMenu_->exec(QCursor::pos());
    if(action) {
        showPropertyTimeline(action->text());
        hidePropertyTimelines_->show();
    }
}

} // namespace voreen
