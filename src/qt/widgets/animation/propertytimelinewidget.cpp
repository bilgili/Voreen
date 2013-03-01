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

#include "voreen/qt/widgets/animation/animationeditor.h"
#include "voreen/qt/widgets/animation/animationexportwidget.h"
#include "voreen/qt/widgets/property/qpropertywidget.h"
#include "voreen/qt/widgets/animation/currentframegraphicsitem.h"
#include "voreen/qt/widgets/animation/keyframegraphicsitem.h"
#include "voreen/qt/widgets/animation/propertytimelinewidget.h"
#include "voreen/qt/widgets/property/colorpropertywidget.h"
#include "voreen/qt/widgets/property/shaderpropertywidget.h"

#include "voreen/core/animation/interpolationfunction.h"
#include "voreen/core/animation/interpolationfunctionfactory.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/volumeurllistproperty.h"

#include "tgt/vector.h"
#include "voreen/core/properties/shaderproperty.h"

#include <QContextMenuEvent>
#include <QGLWidget>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QMenu>
#include <QPen>
#include <QVBoxLayout>

#include <cmath>
#include <iostream>

namespace voreen {

PropertyTimelineWidget::PropertyTimelineWidget(std::string name, PropertyTimeline* propertyTimeline, QWidget* parent)
        : QWidget(parent)
        , propertyTimeline_(propertyTimeline)
        , inInterpolationMenu_(0)
        , outInterpolationMenu_(0)
        {
    guiName_ = name;
    id_ = name;
    mainLayout_ = new QHBoxLayout(this);

    mainLayout_->setMargin(0);
    mainLayout_->setSpacing(0);
    currentFrameGraphicsItem_ = new CurrentFrameGraphicsItem(false, false);

    smoothItem_ = new QGraphicsEllipseItem(0, 0, 8, 8);
    smoothItem_->setBrush(QBrush(QColor(50,255,50)));
    smoothItem_->moveBy(10, -7);
    smoothItem_->setCursor(Qt::ArrowCursor);
    smoothTextItem_ = new QGraphicsSimpleTextItem("smoothness");
    smoothTextItem_->moveBy(22, -10);
    smoothGroup_ = new QGraphicsItemGroup();
    smoothGroup_->addToGroup(smoothItem_);
    smoothGroup_->addToGroup(smoothTextItem_);
    smoothGroup_->hide();
    smoothGroup_->setZValue(5);
    propertyTimelineScene_ = new QGraphicsScene(this);

    propertyTimelineView_ = new PropertyTimelineView(propertyTimelineScene_);
    propertyTimelineView_->setStyleSheet("background:transparent");
    propertyTimelineView_->setFixedHeight(40);
    connect(propertyTimelineView_, SIGNAL(mousePressedAt(QPointF, const QGraphicsItem*)), this, SLOT(interpolationSelectorPressed(QPointF, const QGraphicsItem*)));

    inInterpolationSelector_ = new QGraphicsPixmapItem(QPixmap(":/qt/icons/1leftarrow.png"));
    outInterpolationSelector_ = new QGraphicsPixmapItem(QPixmap(":/qt/icons/1rightarrow.png"));
    inInterpolationSelector_->setCursor(Qt::ArrowCursor);
    outInterpolationSelector_->setCursor(Qt::ArrowCursor);

    inInterpolationSelector_->setMatrix(QMatrix(0.5, 0, 0, 0.5, 1, 1), true);
    outInterpolationSelector_->setMatrix(QMatrix(0.5, 0, 0, 0.5, 1, 1), true);

    propertyTimelineScene_->addItem(smoothGroup_);

    propertyTimelineScene_->addItem(inInterpolationSelector_);
    propertyTimelineScene_->addItem(outInterpolationSelector_);
    currentFrameCounter_ = propertyTimelineScene_->addText("0");
    inInterpolationSelector_->setPos(20, 20);
    outInterpolationSelector_->setPos(40, 20);
    inInterpolationSelector_->setZValue(2);
    outInterpolationSelector_->setZValue(2);

    activateTimelineButton_ = new QPushButton(QIcon(":/qt/icons/apply.png"), "", this);
    showFrameHUD(false);

    activateTimelineButton_->setToolTip(tr("Activate or deactivate timeline"));
    activateTimelineButton_->setStyleSheet("QToolButton { border: none; padding: 1px; }");
    activateTimelineButton_->setFlat(true);
    activateTimelineButton_->setFocusPolicy(Qt::NoFocus);
    activateTimelineButton_->setCheckable(true);
    activateTimelineButton_->setMaximumWidth(16);
    activateTimelineButton_->setMaximumHeight(16);
    activateTimelineButton_->setChecked(propertyTimeline->getActiveOnRendering());

    // note: this is duplicated in activateTimeline() below
    if (!propertyTimeline->getActiveOnRendering())
        activateTimelineButton_->setIcon(QIcon(":/qt/icons/button_cancel.png"));
    else
        activateTimelineButton_->setIcon(QIcon(":/qt/icons/apply.png"));


    connect(activateTimelineButton_, SIGNAL(toggled(bool)), this, SLOT(activateTimeline(bool)));

    setFps(30.0f);

    mainLayout_->addWidget(propertyTimelineView_);
    mainLayout_->addWidget(activateTimelineButton_);
    QLabel* nameLabel = new QLabel(this);
    nameLabel->move(8, 18);
    nameLabel->lower();
    nameLabel->setText(QString::fromStdString(name));
    nameLabel->setMinimumWidth(300);
    nameLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    nameLabel->setEnabled(false);
    setCurrentFrame(0);
    propertyTimelineScene_->addItem(currentFrameGraphicsItem_);

    inInterpolationSelector_->show();
    outInterpolationSelector_->show();
    showFrameHUD(false);
    currentFrameGraphicsItem_->setPos(0, 1);
    currentFrameGraphicsItem_->setZValue(0.5);
    fps_ = 30.0f;
    setDuration(AnimationEditor::getDuration());
}

void PropertyTimelineWidget::moveFrameHUD(int x) {
    inInterpolationSelector_->setPos(17+x, 0);
    outInterpolationSelector_->setPos(49+x, 0);
    smoothGroup_->setPos(60+x, 0);
    currentFrameCounter_->setPos(25+x, -15);
}

void PropertyTimelineWidget::showFrameHUD(bool vis) {
    inInterpolationSelector_->setVisible(vis);
    outInterpolationSelector_->setVisible(vis);
    smoothGroup_->setVisible(vis);
    currentFrameCounter_->setVisible(vis);
}

void PropertyTimelineWidget::interpolationSelectorPressed(QPointF, const QGraphicsItem* item) {
    if(item == inInterpolationSelector_) {
        showInInterpolationMenu();
    }
    else if(item == outInterpolationSelector_) {
        showOutInterpolationMenu();
    }
    else {
        smoothness_ = !smoothness_;
        changeSmoothnessColor();
        this->setSmooth(smoothness_);
    }
}

void PropertyTimelineWidget::changeSmoothnessColor() {
    if(smoothness_)
        smoothItem_->setBrush(QBrush(QColor(50,255,50)));
    else
        smoothItem_->setBrush(QBrush(QColor(255,50,50)));
}

void PropertyTimelineWidget::initConnections() {
    connect(propertyTimelineView_, SIGNAL(addKeyframe(QPointF)), this, SLOT(addKeyframeCore(QPointF)));
    connect(propertyTimelineView_, SIGNAL(clearTimeline()), this, SLOT(clearTimeline()));
    connect(propertyTimelineView_, SIGNAL(removeTimeline()), this, SLOT(removeTimeline()));
    connect(propertyTimelineView_, SIGNAL(noItemPressed(bool)), this, SLOT(disablePropertyWidget(bool)));
    connect(propertyTimelineView_, SIGNAL(snapshot(int, bool)), this, SLOT(snapshot(int, bool)));
    if(propertyWidget_ != 0) {
        connect(propertyWidget_, SIGNAL(widgetChanged()), this, SLOT(updateKeyframe()));
        connect(propertyWidget_, SIGNAL(modified()), this, SLOT(updateKeyframe()));
    }
}

KeyframeGraphicsItem* PropertyTimelineWidget::addKeyframeCore(QPointF pos) {    //this adds also to the scene
    KeyframeGraphicsItem* kfgi = new KeyframeGraphicsItem();
    keyframeGraphicsItems_.push_back(kfgi);
    kfgi->moveBy(pos.x(), 10);
    kfgi->setZValue(2.1);
    propertyTimelineScene_->addItem(kfgi);
    connect(kfgi, SIGNAL(itemClicked(KeyframeGraphicsItem*)), this, SLOT(itemClicked(KeyframeGraphicsItem*)));
    connect(kfgi, SIGNAL(itemReleased(KeyframeGraphicsItem*, bool)), this, SLOT(itemReleased(KeyframeGraphicsItem*, bool)));
    connect(kfgi, SIGNAL(itemMoving(KeyframeGraphicsItem*)), this, SLOT(itemMoving(KeyframeGraphicsItem*)));

    // Adds Keyframe to Animation core and puts it into map<graphicsitem, keyvalue>
    addTemplateKeyframeCore(kfgi, pos);
    return kfgi;
}

void PropertyTimelineWidget::clearTimeline() {
    propertyTimeline_->resetTimeline();
    timelineChange_ = true;
    timelineChanged();
}

void PropertyTimelineWidget::removeTimeline() {
    emit removeTimeline(propertyTimeline_->getProperty());
}

KeyframeGraphicsItem* PropertyTimelineWidget::addKeyframeScene(PropertyKeyValueBase* key) {
    KeyframeGraphicsItem* kfgi = new KeyframeGraphicsItem();
    keyframeGraphicsItems_.push_back(kfgi);
    kfgi->moveBy(key->getTime()*fps_, 10);
    kfgi->setZValue(1.0);
    propertyTimelineScene_->addItem(kfgi);
    connect(kfgi, SIGNAL(itemClicked(KeyframeGraphicsItem*)), this, SLOT(itemClicked(KeyframeGraphicsItem*)));
    connect(kfgi, SIGNAL(itemReleased(KeyframeGraphicsItem*, bool)), this, SLOT(itemReleased(KeyframeGraphicsItem*, bool)));
    connect(kfgi, SIGNAL(itemMoving(KeyframeGraphicsItem*)), this, SLOT(itemMoving(KeyframeGraphicsItem*)));
    // adds keyframe to propertytimelinescene and puts it into map<graphicsitem, keyvalue>
    addTemplateKeyframeScene(kfgi, key);
    return kfgi;
}

void PropertyTimelineWidget::snapshot(int pos, bool force) {
        templateSnapshot(pos, force);
}

void PropertyTimelineWidget::activateTimeline(bool active) {
    activateTemplateTimeline(active);
    if (!active)
        activateTimelineButton_->setIcon(QIcon(":/qt/icons/button_cancel.png"));
    else
        activateTimelineButton_->setIcon(QIcon(":/qt/icons/apply.png"));
}

PropertyTimelineView* PropertyTimelineWidget::getPropertyTimelineView() {
    return propertyTimelineView_;
}

void PropertyTimelineWidget::setCurrentFrame(int x) {
    currentFrameGraphicsItem_->setPos(x, 1);
}

QPropertyWidget* PropertyTimelineWidget::getWidget() {
    return 0;
}

void PropertyTimelineWidget::disablePropertyWidget(bool hide) {
    if(propertyWidget_ != 0) {
        KeyframeWidget_->setEnabled(!hide);
    }
    showFrameHUD(!hide);
}

void PropertyTimelineWidget::itemClicked(KeyframeGraphicsItem* kfgi) {
    // show propertywidget
    disablePropertyWidget(false);
    moveFrameHUD(kfgi->pos().x()-40);
    currentFrameCounter_->setHtml(getTimeString(kfgi->pos().x()));
    showFrameHUD(true);
    templateItemClicked(kfgi);
    inInterpolationSelector_->show();
    outInterpolationSelector_->show();
}

QString PropertyTimelineWidget::getTimeString(int frame) {
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

void PropertyTimelineWidget::itemReleased(KeyframeGraphicsItem* kfgi, bool shift) {
    float x = (float)kfgi->pos().x()/30;

    if(shift)
        shiftTemplateKeyframePosition(x, kfgi);
    else
        updateTemplateKeyframePosition(x, kfgi);
}

void PropertyTimelineWidget::itemMoving(KeyframeGraphicsItem* kfgi) {
    moveFrameHUD(kfgi->pos().x()-40);
    currentFrameCounter_->setHtml(getTimeString(kfgi->pos().x()));
}

void PropertyTimelineWidget::updateKeyframe() {
    updateTemplateKeyframe();
    emit keyframeChanged();
}

void PropertyTimelineWidget::keyPressEvent(QKeyEvent* e) {
    if(e->key() == Qt::Key_Delete) {
        deleteTemplateKeyframe();
    }
}

void PropertyTimelineWidget::showInInterpolationMenu() {
    if(inInterpolationMenu_ != 0) {
        QAction* action = inInterpolationMenu_->exec(QCursor::pos());
        if(action != 0)
            setTemplateInInterpolation(action);
    }
}

void PropertyTimelineWidget::showOutInterpolationMenu() {
    if(outInterpolationMenu_ != 0) {
        QAction* action = outInterpolationMenu_->exec(QCursor::pos());
        if(action != 0)
            setTemplateOutInterpolation(action);
    }
}

void PropertyTimelineWidget::setFps(int fps) {
    QList<QGraphicsItem*> items = propertyTimelineScene_->items();
    for(int i = 0; i < items.size(); ++i) {
        if(dynamic_cast<QGraphicsLineItem*>(items.at(i)))
            propertyTimelineScene_->removeItem(items.at(i));
    }
    propertyTimelineScene_->addRect(0, 10, (int)AnimationEditor::getDuration(), 1);
    fps_ = fps;
    for(int x = 0; x < (int)AnimationEditor::getDuration(); x++) {        //add snaplines
            if(x % (2*fps) == 0) {
                propertyTimelineScene_->addLine(x, 3, x, 17);
            }
            else if(x % fps == 0) {
                propertyTimelineScene_->addLine(x, 5, x, 15);
            }
        }
    propertyTimelineView_->setSceneRect(-2,-2, duration_, 20);
    visualize();
}

void PropertyTimelineWidget::setDuration(int duration) {
    duration_ = duration;
    setFps(fps_);
}

void PropertyTimelineWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    emit(viewResizeSignal(propertyTimelineView_->width()));
}

void PropertyTimelineWidget::emitKeyframeAdded() {
    emit keyframeAdded();
}

void PropertyTimelineWidget::setSmooth(bool smooth) {
    setTemplateSmooth(smooth);
}

void PropertyTimelineWidget::setTemplateSmooth(bool) {
}

} //namespace voreen


