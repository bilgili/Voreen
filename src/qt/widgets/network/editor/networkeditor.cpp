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

#include "voreen/qt/widgets/network/editor/networkeditor.h"

#include "voreen/qt/widgets/network/editor/processorgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/guiitem.h"
#include "voreen/qt/widgets/network/editor/arrowgraphicsitem.h"

// required for safe determination of drop source (dirk)
#include "voreen/qt/widgets/network/processorlistwidget.h"

#include <iostream>

namespace voreen {

NetworkEditor::NetworkEditor(QWidget* parent, ProcessorNetwork* processorNet, NetworkEvaluator* evaluator)
    : QGraphicsView(parent)
    , activeTooltip_(0)
    , evaluator_(evaluator)
    , processorNetwork_(processorNet)
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
    ttimer_ = new TooltipTimer(QPoint(), 3, this); // For even better Tooltip experience
    ttimer_->setSingleShot(true);
    connect(ttimer_, SIGNAL(timeout()), this, SLOT(showTooltip()));

    createContextMenus();

    setDragMode(QGraphicsView::RubberBandDrag);
    setMinimumSize(200, 200);
}

NetworkEditor::~NetworkEditor() {
    clearScene();
    hideTooltip();
    delete ttimer_;
}

void NetworkEditor::generateGraphicsItems() {
    
    if (!processorNetwork_)
        return;

    // Iterate through Processors and create ProcessorItems
    for (int i = 0; i < processorNetwork_->getNumProcessors(); ++i) {
        // create ProcessorItems
        ProcessorGraphicsItem* processorItem = new ProcessorGraphicsItem(processorNetwork_->getProcessors()[i]);
        processorItemMap_[processorNetwork_->getProcessors()[i]] = processorItem;
        processorItem->loadMeta();
        connect(processorItem, SIGNAL(processorNameChanged(Processor*)), this, SIGNAL(processorNameChanged(Processor*)));
    }

    // Iterate through Processors again to create Connections between GuiItems
    for (size_t i = 0; i < processorNetwork_->getProcessors().size(); ++i) {
        std::vector<Port*> outports = processorNetwork_->getProcessors()[i]->getOutports();
        std::vector<Port*> coprocessoroutports = processorNetwork_->getProcessors().at(i)->getCoProcessorOutports();
        // append coprocessoroutports to outports because we can handle them identically FIXME is that really true?
        outports.insert(outports.end(), coprocessoroutports.begin(), coprocessoroutports.end());
        for (size_t j = 0; j < outports.size(); ++j) {
            // find all connections via this (out)port to (in)ports
            std::vector<Port*> connectedPorts = outports[j]->getConnected();
            for (size_t k = 0; k < connectedPorts.size(); ++k) {
                // get relevant data about connection
                Processor* processor = processorNetwork_->getProcessors()[i];
                Port* port = outports[j];
                std::string porttype = port->getTypeIdentifier().getName();
                Port* connectedport = connectedPorts[k];
                Processor* connectedprocessor = connectedport->getProcessor();
                std::string connectedporttype = connectedport->getTypeIdentifier().getName();
                // disconnect this connection
                // processor->disconnect(port, connectedport);
                // connect GuiItems (which also reconnects the processors)
                processorItemMap_[processor]->connectGuionly(
                    processorItemMap_[processor]->getPortItem(porttype),
                    processorItemMap_[connectedprocessor]->getPortItem(connectedporttype));
            }
        }
    }
   
    // finally add graphics items to scene
    std::map<Processor*,ProcessorGraphicsItem*>::iterator processorMapIter = processorItemMap_.begin();
    while (processorMapIter != processorItemMap_.end()) {
        scene_->addItem(processorMapIter->second);
        processorMapIter->second->showAllArrows();
        processorMapIter++;
    }

}

void NetworkEditor::addProcessor(ProcessorGraphicsItem* processor, const QPoint& pos) {
    addProcessor(processor);
    processor->moveBy(mapToScene(pos).x(), mapToScene(pos).y());
    processor->saveMeta();
}

void NetworkEditor::addProcessor(ProcessorGraphicsItem* processor) {
    scene_->addItem(processor);
    processorNetwork_->addProcessor(processor->getProcessor());
    processorItemMap_[processor->getProcessor()] = processor;
    processor->saveMeta();
}

void NetworkEditor::clearScene() {

    processorItemMap_.clear();

    hideTooltip(); // prevent double-free
    
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

void NetworkEditor::setNetwork(ProcessorNetwork* network) {

    processorNetwork_ = network;    

    clearScene();
    generateGraphicsItems();

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

void NetworkEditor::paintEvent(QPaintEvent* event) {
    QPaintEvent adjusted = *event;

    if (event->region().rects().size() > 10) {
        QRect rect = event->region().boundingRect().adjusted(-1, -1, 1, 1);
        adjusted = QPaintEvent(rect);
    }

    QGraphicsView::paintEvent(&adjusted);
}

void NetworkEditor::resizeEvent(QResizeEvent* event) {
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

int NetworkEditor::countSelectedProcessorItems() const {
    int nCount = 0;
    if (scene_ == 0)
        return -1;

    for (int i = 0; i < scene_->selectedItems().size(); i++ ) {
        QGraphicsItem* item = scene_->selectedItems()[i];
        if (typeid(*item) == typeid(ProcessorGraphicsItem))
            nCount++;
    }
    return nCount;
}

void NetworkEditor::center() {
    centerOn(center_);
}

void NetworkEditor::setCenter(QPointF pos) {
    center_ = pos;
}

void NetworkEditor::updateSelectedItems() {
    std::vector<Processor*> selectedProcessors;
    for (int i=0; i<scene()->selectedItems().size(); i++) {
        if (scene()->selectedItems()[i]->type() == ProcessorGraphicsItem::Type) {
            selectedProcessors.push_back(static_cast<ProcessorGraphicsItem*>(scene()->selectedItems()[i])->getProcessor());
            static_cast<ProcessorGraphicsItem*>(scene()->selectedItems()[i])->saveMeta();
        }   
    }

    if (selectedProcessors.size() == 0)
        emit processorSelected(0);
    else if (selectedProcessors.size() == 1)
        emit processorSelected(selectedProcessors[0]);
}

void NetworkEditor::createContextMenus() {
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

void NetworkEditor::copyActionSlot() {
    emit copySignal();
}

void NetworkEditor::pasteActionSlot() {
    emit pasteSignal();
}

void NetworkEditor::deleteActionSlot() {
    
    removeSelectedItems();   

    emit processorSelected(0);    
}

void NetworkEditor::renameActionSlot() {
    QList<QGraphicsItem*> selected = scene()->selectedItems();
    if (selected.size() > 0)
        // static_cast is save here, because context menu only appears for GuiItems
        static_cast<ProcessorGraphicsItem*>(selected[0])->enterRenameMode();
}

// --- drag and drop ----------------------------------------------------------

void NetworkEditor::dragEnterEvent(QDragEnterEvent* event) {
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

void NetworkEditor::dropEvent(QDropEvent* event) {
    if (allowedWidgets_.size() <= 0)
        return;

    if (event->mimeData()->hasText()) {
        QString idString = event->mimeData()->text();
        QPoint position = event->pos();

        // WARNING: previous construstion is dangerous: addAllowedWidget() may change
        // the order of the "allowedWidgets_" as it is public! I suggest using typeid()
        // and the typeid of the widget class to determine, which widget class
        // is the source of the event and to choose the action. (dirk)
        if (dynamic_cast<ProcessorListWidget*>(event->source()) != 0) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
            Identifier id(idString.toStdString());
            ProcessorGraphicsItem* rgi = new ProcessorGraphicsItem(id);
            addProcessor(rgi, position);
        }
        else {
            event->acceptProposedAction();
        }
    }
    else {
        if (event->mimeData()->hasFormat("application/x-voreenvolumesetpointer")) {
            // Determine, whether "something" has been dropped onto a ProcessorGraphicsItem and forward
            // the event if so.
            ProcessorGraphicsItem* procItem = dynamic_cast<ProcessorGraphicsItem*>(itemAt(event->pos()));
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

void NetworkEditor::dragMoveEvent(QDragMoveEvent* /*event*/) {
    // empty, required for drag'n'drop
}

void NetworkEditor::dragLeaveEvent(QDragLeaveEvent* /*event*/) {
    // empty, required for drag'n'drop
}

void NetworkEditor::mousePressEvent(QMouseEvent* event) {
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

void NetworkEditor::mouseMoveEvent(QMouseEvent* event) {
    if (translateScene_) {
        sceneTranslate_ -= mapToScene(event->pos());
        translate(sceneTranslate_.x(), sceneTranslate_.y());
        sceneTranslate_ = mapToScene(event->pos());
    }
    else {
        // tooltip behavior
        lastMousePosition_ = event->pos();
        QGraphicsItem* item = itemAt(event->pos());
        HasTooltip* hastooltip = dynamic_cast<HasTooltip*>(item);
        if (item && // Is there an item at the cursor's position?
            hastooltip)  // Does it have a custom tooltip?
        {
            lastItemWithTooltip_ = hastooltip;
            ttimer_->resetIfDistant(event->pos(),500);
        }
        else {
             if (ttimer_->isDistant(event->pos()))
                 hideTooltip();
        }
        QGraphicsView::mouseMoveEvent(event);
    }
}

void NetworkEditor::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton)
        translateScene_ = false;

    QGraphicsView::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
        updateSelectedItems();
}

void NetworkEditor::wheelEvent(QWheelEvent *event) {
    QPointF p = mapToScene(event->pos());
    translate(-p.x(), -p.y());
    float factor = pow(2.0, event->delta() / 360.0);
    scale(factor, factor);
    translate(p.x(), p.y());
}

void NetworkEditor::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (itemAt(event->pos()))
            emit showPropertiesSignal();
        else
            centerOn(center_);
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void NetworkEditor::keyReleaseEvent(QKeyEvent* event) {
    QGraphicsView::keyReleaseEvent(event);

    if (!event->isAccepted() && event->key() == Qt::Key_Delete)
        deleteActionSlot();

    if (event->key() == Qt::Key_Plus) {
        QPointF p = mapToScene(QCursor::pos());
        translate(-p.x(), -p.y());
        float factor = pow(2.0, 0.35);
        scale(factor, factor);
        translate(p.x(), p.y());
    }

    if (event->key() == Qt::Key_Minus) {
        QPointF p = mapToScene(QCursor::pos());
        translate(-p.x(), -p.y());
        float factor = pow(2.0, -0.35);
        scale(factor, factor);
        translate(p.x(), p.y());
    }
}

void NetworkEditor::contextMenuEvent(QContextMenuEvent* event) {
    event->accept();
    QGraphicsItem* item = itemAt(event->pos());
    if (item) {
        ProcessorGraphicsItem* guiItem;
        if (item->type() == ProcessorGraphicsItem::Type) {
            guiItem = static_cast<ProcessorGraphicsItem*>(item);
        }
        else if (item->type() == TextGraphicsItem::Type) {
            guiItem = static_cast<ProcessorGraphicsItem*>(item->parentItem());
        }
        else
            return;

        if (!guiItem->isSelected()) {
            QList<QGraphicsItem*> selected = scene()->selectedItems();
            for (int i = 0; i < selected.size(); ++i)
                selected[i]->setSelected(false);
        
            guiItem->setSelected(true);
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

void NetworkEditor::showTooltip(const QPoint& pos, HasTooltip* hastooltip) {
    if (!activeTooltip_) {
        activeTooltip_ = hastooltip->tooltip();
        if (activeTooltip_) {
            activeTooltip_->setPos(mapToScene(pos));
            activeTooltip_->setZValue(10); // above the rest
            scene_->addItem(activeTooltip_);
        }
    }
}

void NetworkEditor::showTooltip() {
    showTooltip(lastMousePosition_, lastItemWithTooltip_);
}

void NetworkEditor::hideTooltip() {
    ttimer_->stop();
    if (activeTooltip_) {
        delete activeTooltip_; // Item is automatically removed from the scene
        activeTooltip_ = 0;
    }
}

void NetworkEditor::scaleView(float maxFactor) {
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

void NetworkEditor::removeSelectedItems() {

    QList<QGraphicsItem*> selectedItems = scene()->selectedItems();

    // sort selectedItems by their type to not delete a port/arrow-item
    // that has already been deleted indirectly with the guiitem
    // so at first kick out the ports:
    for (int i=0; i < selectedItems.size(); i++) {
        if (selectedItems.at(i)->type() == PortGraphicsItem::Type)
            selectedItems.removeAt(i--);
    }
    for (int i=0; i < selectedItems.size(); i++) {
        if (selectedItems.at(i)->type() == TextGraphicsItem::Type)
            selectedItems.removeAt(i--);
    }

    // next delete arrows
    for (int i=0; i<selectedItems.size(); i++) {
        QGraphicsItem* item = selectedItems[i];
        if (selectedItems.at(i)->type() == ArrowGraphicsItem::Type) {

            ArrowGraphicsItem* arrow = static_cast<ArrowGraphicsItem*>(item);
            removeArrowItem(arrow);
            selectedItems.removeAt(i--);
        }
    }

    // finally delete processor items
    for (int i=0; i<selectedItems.size(); i++) {
        QGraphicsItem* item = selectedItems[i];
        if (selectedItems.at(i)->type() == ProcessorGraphicsItem::Type) {
            ProcessorGraphicsItem* processorItem = static_cast<ProcessorGraphicsItem*>(item);
            removeProcessorItem(processorItem->getProcessor());
            selectedItems.removeAt(i--);
        }
    }

}

void NetworkEditor::removeProcessorItem(Processor* processor) {

    ProcessorGraphicsItem* processorItem = processorItemMap_[processor];

    // remove connected arrows, also removes the connections in the backend
    processorItem->disconnectAll(); 
    // remove item from the scene
    scene()->removeItem(processorItem);

    // remove item from processor map
    std::map<Processor*, ProcessorGraphicsItem*>::iterator procIter = processorItemMap_.find(processor);
    processorItemMap_.erase(procIter);

    if (processorNetwork_)
        processorNetwork_->removeProcessor(processor);
    delete processorItem;

}

void NetworkEditor::removeArrowItem(ArrowGraphicsItem* arrow) {
    
    if (arrow->getDestNode() != 0) {
        // arrow between ports or arrow between property set and guiitem
        if (arrow->getDestNode()->type() == PortGraphicsItem::Type) {
            static_cast<PortGraphicsItem*>(arrow->getSourceNode())->getParent()
                ->disconnect(static_cast<PortGraphicsItem*>(arrow->getSourceNode()),
                static_cast<PortGraphicsItem*>(arrow->getDestNode()));
        }
    }

}




} // namespace voreen
