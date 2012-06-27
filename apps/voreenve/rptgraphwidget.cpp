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

#include "rptgraphwidget.h"

#include "voreen/core/vis/processors/networkevaluator.h"

#include "rptprocessoritem.h"
#include "rptaggregationitem.h"
#include "rptpropertysetitem.h"
#include "rptarrow.h"
#include "rptnetworkserializergui.h"
#include "rptaggregationlistwidget.h"   // required for safe determination of drop source (dirk)

#include <iostream>

namespace voreen {

RptGraphWidget::RptGraphWidget(QWidget* parent, NetworkEvaluator* evaluator)
    : QGraphicsView(parent)
    , activeRptTooltip_(0)
    , evaluator_(evaluator)
    , rptnet_(0)
    , needsScale_(false)
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

    createContextMenus();

    setDragMode(QGraphicsView::RubberBandDrag);
    setMinimumSize(200, 200);
}

RptGraphWidget::~RptGraphWidget() {
    clearScene();
    hideRptTooltip();
    delete ttimer_;
}

void RptGraphWidget::addProcessor(RptProcessorItem* processor, const QPoint& pos) {
    addProcessor(processor);
    processor->moveBy(mapToScene(pos).x(), mapToScene(pos).y());
}

void RptGraphWidget::addProcessor(RptProcessorItem* processor) {
    scene_->addItem(processor);
    rptnet_->addProcessor(processor);
}

void RptGraphWidget::clearScene() {
    if (rptnet_)
        rptnet_->processorItems.clear(); // we will take care of deleting them

    hideRptTooltip(); // prevent double-free
    
#if QT_VERSION >= 0x040400    
    scene_->clear();
#else
    QList<QGraphicsItem*> items = scene_->items();
    QList<QGraphicsItem*> deleteitems;
    for (int i = 0; i < items.size(); ++i) {
        if (QGraphicsItem* item = items.at(i)) {
            if (!item->parentItem())
                deleteitems << item;
        }
    }
    qDeleteAll(deleteitems);
#endif

}

void RptGraphWidget::setNetwork(RptNetwork* network) {
    rptnet_ = network;

    std::vector<RptProcessorItem*> processorItems = rptnet_->processorItems;
    for (size_t i=0; i< processorItems.size(); i++) {
        scene_->addItem(processorItems[i]);
        processorItems[i]->showAllArrows();
    }

    resetMatrix();
    setCenter(QPointF(0, 0));
    center();
    resetTransform();

    // do not scale immediately when we might be in visualization mode and this window might
    // not have its final size yet
    if (isVisible()) {
        scaleView(1.f);
        needsScale_ = false;
    } else {
        needsScale_ = true;
    }
}

void RptGraphWidget::paintEvent(QPaintEvent* event) {
    QPaintEvent adjusted = *event;

    if (event->region().rects().size() > 10) {
        QRect rect = event->region().boundingRect().adjusted(-1, -1, 1, 1);
        adjusted = QPaintEvent(rect);
    }

    QGraphicsView::paintEvent(&adjusted);
}

void RptGraphWidget::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);

    if (isVisible() && needsScale_) {
        scaleView(1.f);
        needsScale_ = false;
    }

    //TODO: This could be used to fit the network to the window when resizing. Problem is that
    //this limits zooming in, somehow resizeEvents are triggerd by wheelEvents. joerg
//     resetMatrix();
//     resetTransform();
//     scaleView(1.f);
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

void RptGraphWidget::createContextMenus() {
    QAction* copyAction = new QAction(tr("Copy"), this);
    QAction* pasteAction = new QAction(tr("Paste"), this);

    // add actions to context menu
    contextMenu_.addAction(copyAction);
    contextMenu_.addAction(pasteAction);

    // connect actions
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copyActionSlot()));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(pasteActionSlot()));

    // single selection context menu
    QAction* deleteAction = new QAction(QIcon(":/vrn_app/icons/eraser.png"), tr("Delete"), this);
    QAction* renameAction = new QAction(tr("Rename"), this);

    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteActionSlot()));
    connect(renameAction, SIGNAL(triggered()), this, SLOT(renameActionSlot()));

    rightClickMenuSingle_.addAction(deleteAction);
    rightClickMenuSingle_.addAction(renameAction);
    
    rightClickMenuMultiple_.addAction(deleteAction);
}

void RptGraphWidget::copyActionSlot() {
    emit copySignal();
}

void RptGraphWidget::pasteActionSlot() {
    emit pasteSignal();
}

void RptGraphWidget::deleteActionSlot() {
    emit deleteSignal();
    emit processorSelected(0);    
}

void RptGraphWidget::renameActionSlot() {
    QList<QGraphicsItem*> selected = scene()->selectedItems();
    if (selected.size() > 0)
        // static_cast is save here, because context menu only appears for RptGuiItems
        static_cast<RptGuiItem*>(selected[0])->enterRenameMode();
}

// --- drag and drop ----------------------------------------------------------

void RptGraphWidget::dragEnterEvent(QDragEnterEvent* event) {
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
    if (allowedWidgets_.size() <= 0)
        return;

    if (event->mimeData()->hasText()) {
        QString idString = event->mimeData()->text();
        QPoint position = event->pos();

        // WARNING: previous construstion is dangerous: addAllowedWidget() may change
        // the order of the "allowedWidgets_" as it is public! I suggest using typeid()
        // and the typeid of the widget class to determine, which widget class
        // is the source of the event and to choose the action. (dirk)
        if (dynamic_cast<RptProcessorListWidget*>(event->source()) != 0) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
            Identifier id(idString.toStdString());
            RptProcessorItem* rgi = new RptProcessorItem(id);
            emit processorAdded(id, position);
            emit processorAdded(rgi);
            addProcessor(rgi, position);
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
            // Determine, whether "something" has been dropped onto a RptProcessorItem and forward
            // the event if so.
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
    // empty, required for drag'n'drop
}

void RptGraphWidget::dragLeaveEvent(QDragLeaveEvent* /*event*/) {
    // empty, required for drag'n'drop
}

void RptGraphWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton)
        return;
    // shift and left button activate translation of scene.
    else if (event->button() == Qt::LeftButton && event->modifiers() == Qt::ShiftModifier) {
        translateScene_ = true;
        sceneTranslate_ = mapToScene(event->pos());
    }
    else
        QGraphicsView::mousePressEvent(event);
}

void RptGraphWidget::mouseMoveEvent(QMouseEvent* event) {
    if (translateScene_) {
        sceneTranslate_ -= mapToScene(event->pos());
        translate(sceneTranslate_.x(), sceneTranslate_.y());
        sceneTranslate_ = mapToScene(event->pos());
    }
    else {
        // tooltip behavior
        lastMousePosition_ = event->pos();
        QGraphicsItem* item = itemAt(event->pos());
        HasRptTooltip* hastooltip = dynamic_cast<HasRptTooltip*>(item);
        if (item && // Is there an item at the cursor's position?
            hastooltip)  // Does it have a custom tooltip?
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

void RptGraphWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton)
        translateScene_ = false;

    QGraphicsView::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
        updateSelectedItems();
}

void RptGraphWidget::wheelEvent(QWheelEvent *event) {
    QPointF p = mapToScene(event->pos());
    translate(-p.x(), -p.y());
    float factor = pow(2.0, event->delta() / 240.0);
    scale(factor, factor);
    translate(p.x(), p.y());
}

void RptGraphWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (itemAt(event->pos()))
            emit showPropertiesSignal();
        else
            centerOn(center_);
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void RptGraphWidget::keyReleaseEvent(QKeyEvent* event) {
    QGraphicsView::keyReleaseEvent(event);

    if (!event->isAccepted() && event->key() == Qt::Key_Delete)
        deleteActionSlot();

    if (event->key() == Qt::Key_Plus) {
        QPointF p = mapToScene(QCursor::pos());
        translate(-p.x(), -p.y());
        float factor = pow(2.0, 0.5);
        scale(factor, factor);
        translate(p.x(), p.y());
    }

    if (event->key() == Qt::Key_Minus) {
        QPointF p = mapToScene(QCursor::pos());
        translate(-p.x(), -p.y());
        float factor = pow(2.0, -0.5);
        scale(factor, factor);
        translate(p.x(), p.y());
    }
}

void RptGraphWidget::contextMenuEvent(QContextMenuEvent* event) {
    event->accept();
    QGraphicsItem* item = itemAt(event->pos());
    if (item) {
        RptGuiItem* rptItem;
        if (item->type() == RptProcessorItem::Type) {
            rptItem = static_cast<RptGuiItem*>(item);
        }
        else if (item->type() == RptTextItem::Type) {
            rptItem = static_cast<RptGuiItem*>(item->parentItem());
        }
        else
            return;

        if (!rptItem->isSelected()) {
            QList<QGraphicsItem*> selected = scene()->selectedItems();
            for (int i = 0; i < selected.size(); ++i)
                selected[i]->setSelected(false);
        
            rptItem->setSelected(true);
        }

        // display correct context menu
        if (scene()->selectedItems().size() > 1) {
            rightClickMenuMultiple_.popup(event->globalPos());
        }
        else {
            rightClickMenuSingle_.popup(event->globalPos());
        }
    }
    else {
        // mouse cursor is not over a guiitem -> deselect all selected items
        QList<QGraphicsItem*> selected = scene()->selectedItems();
        for (int i = 0; i < selected.size(); ++i)
            selected[i]->setSelected(false);
    }
}

void RptGraphWidget::showRptTooltip(const QPoint& pos, HasRptTooltip* hastooltip) {
    if (!activeRptTooltip_) {
        activeRptTooltip_ = hastooltip->rptTooltip();
        if (activeRptTooltip_) {
            activeRptTooltip_->setPos(mapToScene(pos));
            activeRptTooltip_->setZValue(10); // above the rest
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

void RptGraphWidget::scaleView(float maxFactor) {
    // the scene rectangle
    QRectF sceneRect = getScene()->itemsBoundingRect();
    getScene()->setSceneRect(sceneRect);

    // calculate scale factors, take size of scroll bars into account
    float scaleFactor = std::min(static_cast<float>((width() - 10) / sceneRect.width()),
                                 static_cast<float>((height() - 10) / sceneRect.height()));
    scaleFactor = std::min(scaleFactor, maxFactor);
    
    // scale smaller dimension
    scale(scaleFactor, scaleFactor);
    // calculate center
    setCenter(QPointF(sceneRect.left() + sceneRect.width() / 2,
                      sceneRect.top() + sceneRect.height() / 2));
    center();
    invalidateScene();
}

} // namespace voreen
