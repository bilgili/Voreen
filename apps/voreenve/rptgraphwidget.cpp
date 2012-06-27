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

#include "rptgraphwidget.h"

#include "voreen/core/vis/processors/networkevaluator.h"

#include "rptprocessoritem.h"
#include "rptaggregationitem.h"
#include "rptpropertysetitem.h"
#include "rptarrow.h"
#include "rptaggregationlistwidget.h"   // required for safe determination of drop source (dirk)

#include <iostream>

namespace voreen {

RptGraphWidget::RptGraphWidget(QWidget* parent, NetworkEvaluator* evaluator)
    : QGraphicsView(parent)
    , activeRptTooltip_(0)
    , evaluator_(evaluator)
{
    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    translateScene_ = false;
    scene_->setItemIndexMethod(QGraphicsScene::NoIndex);

	setBackgroundBrush(QBrush(Qt::darkGray));
    setCacheMode(CacheBackground);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    center_ = QPointF(0.0, 0.0);
    setResizeAnchor(AnchorViewCenter);
    setMouseTracking(true); // For better Tooltip experience
    ttimer_ = new RptTooltipTimer(QPoint(), 3, this); // For even better Tooltip experience
    ttimer_->setSingleShot(true);
    connect(ttimer_, SIGNAL(timeout()), this, SLOT(showRptTooltip()));

    createConnections();
    createContextMenu();

    setDragMode(QGraphicsView::RubberBandDrag);
    setMinimumSize(200, 200);
}

RptGraphWidget::~RptGraphWidget() {
    hideRptTooltip();
    delete ttimer_;
}

void RptGraphWidget::addItem(RptGuiItem* item, QPoint pos) {
    scene_->addItem(item);
    item->moveBy(mapToScene(pos).x(), mapToScene(pos).y());
}

void RptGraphWidget::addItem(RptGuiItem* item) {
    scene_->addItem(item);
}

void RptGraphWidget::clearScene() {
    if (scene_ == 0)
        return;

    resetMatrix();
    center_ = QPointF(0, 0);

    // Remove QGraphicsItems which belong directly
    // to the scene, because the have been added
    // via scene_->addItem().
    QList<QGraphicsItem*> list = scene_->items();
    for (int i=0; i < list.size(); ++i)
        scene_->removeItem(list[i]);
}

void RptGraphWidget::createConnections() {
}

void RptGraphWidget::scaleView(qreal scaleFactor) {
    scale(scaleFactor, scaleFactor);
}

void RptGraphWidget::paintEvent(QPaintEvent *event) {
    QPaintEvent adjusted = *event;

    if (event->region().rects().size() > 10) {
        QRect rect = event->region().boundingRect().adjusted(-1,-1,1,1);
        adjusted = QPaintEvent(rect);
    }
    
    QGraphicsView::paintEvent(&adjusted);
}

int RptGraphWidget::countSelectedProcessorItems() const {
    int nCount = 0;
    if (scene_ == 0)
        return -1;

    for (int i = 0; i < scene_->selectedItems().size(); i++ ) {
        QGraphicsItem* item = scene_->selectedItems()[i];
        if (typeid(*item) == typeid(RptProcessorItem))
            nCount++;
    }
    return nCount;
}

void RptGraphWidget::updateScene(const QList<QRectF> &rects ) {
    QGraphicsView::updateScene(rects);
}

void RptGraphWidget::center() {
    centerOn(center_);
}

void RptGraphWidget::setCenter(QPointF pos) {
    center_ = pos;
}

void RptGraphWidget::updateSelectedItems() {
    std::vector<Processor*> selectedProcessors;
    for (int i=0; i<scene()->selectedItems().size(); i++) {
        if (scene()->selectedItems()[i]->type() == RptProcessorItem::Type) {
            selectedProcessors.push_back(static_cast<RptProcessorItem*>(scene()->selectedItems()[i])->getProcessor());
        }
        else if (scene()->selectedItems()[i]->type() == RptAggregationItem::Type) {
            std::vector<RptProcessorItem*> guiItems = static_cast<RptAggregationItem*>(scene()->selectedItems()[i])->getProcessorItems();
            for (size_t j=0; j<guiItems.size(); j++) {
                selectedProcessors.push_back(guiItems[j]->getProcessor());
            }
        }
        else if (scene()->selectedItems()[i]->type() == RptPropertySetItem::Type) {
            selectedProcessors.push_back(static_cast<RptPropertySetItem*>(scene()->selectedItems()[i])->getProcessor());
        }
    }

    if (selectedProcessors.size() == 0)
        emit processorSelected(0);
    else if (selectedProcessors.size() == 1)
        emit processorSelected(selectedProcessors[0]);
    else {
        PropertySet* tps = PropertySet::getTmpPropSet();
        tps->setProcessors(selectedProcessors);
        emit processorSelected(tps);
    }
}

void RptGraphWidget::createContextMenu() {
    QAction* copyAction_ = new QAction(tr("Copy"), this);
    QAction* pasteAction_ = new QAction(tr("Paste"), this);

    // add actions to context menu
    contextMenu_.addAction(copyAction_);
    contextMenu_.addAction(pasteAction_);

    // connect actions
    connect(copyAction_, SIGNAL(triggered()), this, SLOT(copyActionSlot()));
    connect(pasteAction_, SIGNAL(triggered()), this, SLOT(pasteActionSlot()));
}

void RptGraphWidget::copyActionSlot() {
    emit copySignal();
}

void RptGraphWidget::pasteActionSlot() {
    emit pasteSignal();
}


// --- drag and drop ----------------------------------------------------------

void RptGraphWidget::dragEnterEvent(QDragEnterEvent *event) {
    if ((event->mimeData()->hasText()) || (event->mimeData()->hasFormat("application/x-voreenvolumesetpointer"))) {
        bool allowed = false;
        for (size_t i=0; i<allowedWidgets_.size(); i++) {
            if (event->source() == allowedWidgets_[i])
                allowed = true;
        }
        if (allowed) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
        else {
            event->acceptProposedAction();
        }
    }
    else {
        event->ignore();
    }
}

void RptGraphWidget::dropEvent(QDropEvent* event) {
    if (allowedWidgets_.size() < 2)
        return;

    if (event->mimeData()->hasText()) {
        QString idString = event->mimeData()->text();
        QPoint position = event->pos();

        // WARNING: previous construstion is dangerous: addAllowedWidget() may change
        // the order of the "allowedWidgets_" as it is public! I suggest using typeid()
        // and the typeid of the widget class to determine, which widget class
        // is the source of the event and to choose the action. (dirk)
        //
        if (dynamic_cast<RptProcessorListWidget*>(event->source()) != 0) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
            Identifier id(idString.toStdString());
            emit processorAdded(id,position);
        }
        else if (dynamic_cast<RptAggregationListWidget*>(event->source()) != 0) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
            emit aggregationAdded(idString.toStdString(), position);
        }
        else {
            event->acceptProposedAction();
        }
    }
    else {
        if (event->mimeData()->hasFormat("application/x-voreenvolumesetpointer")) {
            // Determine, whether "something" has been dropped onto a RptProcessorItem and foreward
            // the event if so.
            //
            RptProcessorItem* procItem = dynamic_cast<RptProcessorItem*>(itemAt(event->pos()));
            if (procItem != 0) {
                procItem->dropEvent(event);
                event->setDropAction(Qt::CopyAction);
                event->accept();
            }
            event->acceptProposedAction();
        }
        else {
            event->ignore();
        }
    }
}

void RptGraphWidget::dragMoveEvent(QDragMoveEvent* /*event*/) {
}

void RptGraphWidget::dragLeaveEvent(QDragLeaveEvent* /*event*/) {
}

void RptGraphWidget::mousePressEvent(QMouseEvent* event) {
    // needed for copy action on scene. Without items would be deselected before copied
    if (event->button() == Qt::RightButton)
        return;
    // shift and left button activate translation of scene. FIXME: broken
    else if ((event->button() == Qt::LeftButton) && (event->modifiers() == Qt::ShiftModifier)) {
        translateScene_ = true;
        sceneTranslate_ = mapToScene(event->globalPos());
    }
    else
        QGraphicsView::mousePressEvent(event);
}

void RptGraphWidget::mouseMoveEvent(QMouseEvent *event) {
    if (translateScene_) {
        sceneTranslate_ -= mapToScene(event->globalPos());
        translate(sceneTranslate_.x(), sceneTranslate_.y());
        sceneTranslate_ = mapToScene(event->globalPos());
    }
    else {
        // tooltip behavior
        lastMousePosition_ = event->pos();
        QGraphicsItem* item;
        HasRptTooltip* hastooltip;
        if ((item = itemAt(event->pos())) && // Is there an item at the cursor's position?
            (hastooltip = dynamic_cast<HasRptTooltip*>(item)))  // Does it have a custom tooltip?
        {
            lastItemWithTooltip_ = hastooltip;
            ttimer_->resetIfDistant(event->pos(),500);
        }
        else {
             if (ttimer_->isDistant(event->pos()))
                 hideRptTooltip();
        }
        QGraphicsView::mouseMoveEvent(event);
    }
}

void RptGraphWidget::mouseReleaseEvent(QMouseEvent  *event) {
    if (event->button() == Qt::LeftButton)
        translateScene_ = false;

    QGraphicsView::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
        updateSelectedItems();
}

void RptGraphWidget::wheelEvent(QWheelEvent *event) {
    scaleView(pow(2.0, -event->delta() / 240.0));
}

void RptGraphWidget::mouseDoubleClickEvent(QMouseEvent  *event) {
    if (event->button() == Qt::LeftButton) {
        if (itemAt(event->pos()))
            emit showPropertiesSignal();
        else
            centerOn(center_);
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void RptGraphWidget::contextMenuEvent(QContextMenuEvent *event) {
    QGraphicsItem* item = itemAt(event->pos());

    if (item != 0) {
        if (typeid(*item) == typeid(RptProcessorItem)) {
            RptProcessorItem* pProc = static_cast<RptProcessorItem*>(item);
            const int numProcis = countSelectedProcessorItems();
            if (numProcis <= 1) {
                pProc->enableAggregateContextMenuEntry(false);
            }
            else {
                pProc->enableAggregateContextMenuEntry(true);
            }
        }
        QGraphicsView::contextMenuEvent(event); // send event to item
    }
    else {
        contextMenu_.exec(event->globalPos());
    }
}

void RptGraphWidget::showRptTooltip(const QPoint & pos, HasRptTooltip* hastooltip) {
    if (!activeRptTooltip_) {
        activeRptTooltip_ = hastooltip->rptTooltip();
        if (activeRptTooltip_) {
            activeRptTooltip_->setPos(mapToScene(pos));
            activeRptTooltip_->setZValue(10); // Towering above the rest
            scene_->addItem(activeRptTooltip_);
        }
    }
}

void RptGraphWidget::showRptTooltip() {
    showRptTooltip(lastMousePosition_, lastItemWithTooltip_);
}

void RptGraphWidget::hideRptTooltip() {
    ttimer_->stop();
    if (activeRptTooltip_) {
        delete activeRptTooltip_; // Item is automatically removed from the scene
        activeRptTooltip_ = 0;
    }
}


} //namespace voreen
