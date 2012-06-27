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

#include "voreen/qt/widgets/processor/qprocessorwidgetfactory.h"

#include "voreen/qt/widgets/network/editor/processorgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/arrowgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/textgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/portgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/linkarrowgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/propertygraphicsitem.h"
#include "voreen/qt/widgets/network/editor/itooltip.h"

#include "voreen/core/vis/workspace/workspace.h"
#include "voreen/core/vis/processors/processorfactory.h"

#include "voreen/qt/widgets/network/editor/propertylinkdialog.h"

// required for safe determination of drop source (dirk)
#include "voreen/qt/widgets/network/processorlistwidget.h"
#include "voreen/qt/widgets/network/editor/tooltiptimer.h"

#include <QToolButton>
#include <iostream>

namespace {
    const QSize minimumSize = QSize(200, 200);
    const qreal tooltiptimerRadius = 3.f;
    const QSize layerButtonSize = QSize(30, 30);
    const QSize autoLinkingSize = QSize(30, 30);
    const int layerButtonSpacingX = 10;
    const int layerButtonSpacingY = 10;
    const QRectF sceneRectSpacing = QRect(-10, -10, 10, 10);
    const qreal scaleFactorFactor = 0.9f;
    const qreal keyPressScaleFactor = pow(2.f, 0.35f);
}

namespace voreen {

class LinkEvaluatorBase;

const std::string NetworkEditor::loggerCat_("voreen.NetworkEditor");

NetworkEditor::NetworkEditor(QWidget* parent, Workspace* workspace, NetworkEvaluator* evaluator)
    : QGraphicsView(parent)
    , workspace_(workspace)
    , evaluator_(evaluator)
    , selectedLink_(0)
    , selectedPropertyGraphicsItem_(0)
    , needsScale_(false)
    , activeTooltip_(0)
    , layerButtonContainer_(0)
    , dataflowLayerButton_(0)
    , linkingLayerButton_(0)
    , currentLayer_(NetworkEditorLayerDataflow)
{
    tgtAssert(evaluator_ != 0, "passed null pointer");
    tgtAssert(workspace_ != 0, "passed null pointer");
    tgtAssert(workspace_->getProcessorNetwork() != 0, "no network available");

    setScene(new QGraphicsScene(this));
    translateScene_ = false;

    setBackgroundBrush(QBrush(Qt::darkGray));
    setCacheMode(CacheBackground);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    center_ = QPointF(0.0, 0.0);
    setResizeAnchor(AnchorViewCenter);
    setMouseTracking(true); // For better Tooltip experience
    setDragMode(QGraphicsView::RubberBandDrag);
    //setMinimumSize(minimumSize);
    setMinimumSize(QSize(200, 200));

    createContextMenus();
    createTooltipTimer();
    createLayerButtons();
}

NetworkEditor::~NetworkEditor() {
    clearClipboard();
    clearScene();
    hideTooltip();
    delete ttimer_;
}

// ------------------------------------------------------------------------------------------------
// initialization methods
// ------------------------------------------------------------------------------------------------

void NetworkEditor::generateGraphicsItems() {
    foreach (Processor* proc, workspace_->getProcessorNetwork()->getProcessors())
        createProcessorGraphicsItem(proc);

    foreach (Processor* proc, workspace_->getProcessorNetwork()->getProcessors()) {
        std::vector<Port*> outports = proc->getOutports();
        std::vector<CoProcessorPort*> coprocessoroutports = proc->getCoProcessorOutports();
        // append coprocessoroutports to outports because we can handle them identically
        outports.insert(outports.end(), coprocessoroutports.begin(), coprocessoroutports.end());

        foreach (Port* port, outports) {
            std::vector<Port*> connectedPorts = port->getConnected();

            foreach (Port* connectedPort, connectedPorts) {
                Processor* connectedProcessor = connectedPort->getProcessor();
                processorItemMap_[proc]->connectGuionly(
                    processorItemMap_[proc]->getPort(port),
                    processorItemMap_[connectedProcessor]->getPort(connectedPort));
            }
        }
    }

    foreach (ProcessorGraphicsItem* procItem, processorItemMap_.values()) {
        scene()->addItem(procItem);
        procItem->showAllArrows();
    }

    foreach (PropertyLink* link, workspace_->getProcessorNetwork()->getPropertyLinks()) {
        LinkArrowGraphicsItem* arrow = createLinkArrowForPropertyLink(link);
        arrow->hide();
    }
}

void NetworkEditor::createContextMenus() {
    QAction* copyAction = new QAction(QIcon(":/voreenve/icons/edit-copy.png"), tr("Copy"), this);
    QAction* pasteAction = new QAction(QIcon(":/voreenve/icons/edit-paste.png"), tr("Paste"), this);
    QAction* deleteAction = new QAction(QIcon(":/voreenve/icons/eraser.png"), tr("Delete"), this);
    QAction* deleteLinkAction = new QAction(QIcon(":/voreenve/icons/eraser.png"), tr("Delete"), this);
    QAction* renameAction = new QAction(tr("Rename"), this);
    QAction* editAction = new QAction(tr("Edit"), this);

    // connect actions
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copyActionSlot()));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(pasteActionSlot()));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteActionSlot()));
    connect(renameAction, SIGNAL(triggered()), this, SLOT(renameActionSlot()));
    connect(deleteLinkAction, SIGNAL(triggered()), this, SLOT(deletePropertyLinkSlot()));
    connect(editAction, SIGNAL(triggered()), this, SLOT(editPropertyLinkSlot()));

    rightClickMenuNone_.addAction(pasteAction);

    rightClickMenuSingle_.addAction(deleteAction);
    rightClickMenuSingle_.addAction(renameAction);
    rightClickMenuSingle_.addAction(copyAction);
    rightClickMenuSingle_.addAction(pasteAction);

    rightClickMenuMultiple_.addAction(deleteAction);
    //rightClickMenuMultiple_.addAction(copyAction);
    rightClickMenuMultiple_.addAction(pasteAction);

    rightClickLinkMenu_.addAction(deleteLinkAction);
    rightClickLinkMenu_.addAction(editAction);
}

void NetworkEditor::createTooltipTimer() {
    ttimer_ = new TooltipTimer(QPoint(), tooltiptimerRadius, this); // For even better Tooltip experience
    ttimer_->setSingleShot(true);
    connect(ttimer_, SIGNAL(timeout()), this, SLOT(showTooltip()));
}

void NetworkEditor::createLayerButtons() {
    layerButtonContainer_ = new QWidget(this);
    QBoxLayout* layerLayout = new QHBoxLayout(layerButtonContainer_);

    dataflowLayerButton_ = new QToolButton;
    dataflowLayerButton_->setIcon(QIcon(":/voreenve/icons/dataflow-mode.png"));
    dataflowLayerButton_->setIconSize(layerButtonSize);
    dataflowLayerButton_->setToolTip(tr("switch to data flow mode"));
    dataflowLayerButton_->setCheckable(true);
    connect(dataflowLayerButton_, SIGNAL(clicked()), this, SLOT(setLayerToDataflow()));
    layerLayout->addWidget(dataflowLayerButton_);

    linkingLayerButton_ = new QToolButton;
    linkingLayerButton_->setIcon(QIcon(":/voreenve/icons/linking-mode.png"));
    linkingLayerButton_->setIconSize(layerButtonSize);
    linkingLayerButton_->setToolTip(tr("switch to linking mode"));
    linkingLayerButton_->setCheckable(true);
    connect(linkingLayerButton_, SIGNAL(clicked()), this, SLOT(setLayerToLinking()));
    layerLayout->addWidget(linkingLayerButton_);

    autoLinkingContainer_ = new QWidget(this);
    QBoxLayout* autoLinkingLayout = new QVBoxLayout(autoLinkingContainer_);

    linkCamerasAutoButton_ = new QToolButton;
    linkCamerasAutoButton_->setIcon(QIcon(":/voreenve/icons/linking-camera-auto.png"));
    linkCamerasAutoButton_->setIconSize(autoLinkingSize);
    linkCamerasAutoButton_->setToolTip(tr("Link cameras of processors when they are added to the network"));
    linkCamerasAutoButton_->setCheckable(true);
    linkCamerasAutoButton_->setChecked(true);
    autoLinkingLayout->addWidget(linkCamerasAutoButton_);

    linkCamerasButton_ = new QToolButton;
    linkCamerasButton_->setIcon(QIcon(":/voreenve/icons/linking-camera.png"));
    linkCamerasButton_->setIconSize(autoLinkingSize);
    linkCamerasButton_->setToolTip(tr("Link all cameras in the network"));
    connect(linkCamerasButton_, SIGNAL(clicked()), this, SLOT(linkCameras()));
    autoLinkingLayout->addWidget(linkCamerasButton_);

    removePropertyLinksButton_ = new QToolButton;
    removePropertyLinksButton_->setIcon(QIcon(":/voreenve/icons/linking-remove.png"));
    removePropertyLinksButton_->setIconSize(autoLinkingSize);
    removePropertyLinksButton_->setToolTip(tr("Remove all property links from the network"));
    connect(removePropertyLinksButton_, SIGNAL(clicked()), this, SLOT(removePropertyLinks()));
    autoLinkingLayout->addWidget(removePropertyLinksButton_);

    // add to button group, so only one can be checked at the same time
    QButtonGroup* group = new QButtonGroup;
    group->addButton(dataflowLayerButton_);
    group->addButton(linkingLayerButton_);

    layoutLayerButtons();
}

// ------------------------------------------------------------------------------------------------
// other methods
// ------------------------------------------------------------------------------------------------

void NetworkEditor::layoutLayerButtons() {
    int x = size().width() - layerButtonContainer_->size().width() - layerButtonSpacingX;
    int y = layerButtonSpacingY;

    layerButtonContainer_->move(x,y);

    x += layerButtonContainer_->width() - autoLinkingContainer_->width();
    y += static_cast<int>(layerButtonContainer_->height() * 0.85f);

    autoLinkingContainer_->move(x,y);
}

ProcessorGraphicsItem* NetworkEditor::createProcessorGraphicsItem(Processor* processor, QPoint /*position*/) {
    ProcessorGraphicsItem* result = new ProcessorGraphicsItem(processor, this);
    processorItemMap_[processor] = result;
    result->setLayer(currentLayer_);
    result->loadMeta();

    connect(result, SIGNAL(pressedPropertyGraphicsItem(PropertyGraphicsItem*)), this, SLOT(pressedPropertyGraphicsItem(PropertyGraphicsItem*)));
    connect(result, SIGNAL(portConnectionsChanged()), this, SLOT(processorConnectionsChanged()));

    return result;
}

void NetworkEditor::clearClipboard() {
    foreach (ProcessorGraphicsItem* proc, clipboardProcessors_) {
        delete proc;
    }

    clipboardProcessors_.clear();
}

void NetworkEditor::clearScene() {
    emit update();
    processorItemMap_.clear();
    hideTooltip(); // prevent double-free
    scene()->clear();
}

void NetworkEditor::newNetwork() {
    clearScene();
    linkArrows_.clear();

    if (workspace_->getProcessorNetwork())
        workspace_->getProcessorNetwork()->addObserver(this);
    else
        return;

    generateGraphicsItems();

    resetMatrix();
    setCenter(QPointF(0, 0));
    center();
    resetTransform();

    // do not scale immediately when we might be in visualization mode and this window might
    // not have its final size yet
    if (isVisible()) {
        scaleView(1.0f);
        needsScale_ = false;
    } else {
        needsScale_ = true;
    }

    dataflowLayerButton_->setChecked(true);
    setLayer(currentLayer_);
}

void NetworkEditor::center() {
    centerOn(center_);
}

QList<ProcessorGraphicsItem*> NetworkEditor::selectedProcessorItems() const {
    QList<ProcessorGraphicsItem*> result;

    foreach (QGraphicsItem* item, scene()->selectedItems()) {
        ProcessorGraphicsItem* procItem = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);

        if (procItem)
            result.append(procItem);
    }

    return result;
}

int NetworkEditor::countSelectedProcessorItems() const {
    return selectedProcessorItems().count();
}

void NetworkEditor::setCenter(QPointF pos) {
    center_ = pos;
}

ProcessorNetwork* NetworkEditor::getProcessorNetwork() {
    return workspace_->getProcessorNetwork();
}

void NetworkEditor::processorConnectionsChanged() {
    evaluator_->onNetworkConnectionsChange();
}

QSize NetworkEditor::sizeHint() const {
    return QSize(400, 600);
}

void NetworkEditor::updateSelectedItems() {
    std::vector<Processor*> selectedProcessors;
    for (int i=0; i<scene()->selectedItems().size(); i++) {
        if (scene()->selectedItems()[i]->type() == ProcessorGraphicsItem::Type) {
            selectedProcessors.push_back(qgraphicsitem_cast<ProcessorGraphicsItem*>(scene()->selectedItems()[i])->getProcessor());
            qgraphicsitem_cast<ProcessorGraphicsItem*>(scene()->selectedItems()[i])->saveMeta();
        }
    }

    emit processorsSelected(selectedProcessors);
}

void NetworkEditor::scaleView(float maxFactor) {
    // the scene rectangle
    QRectF sceneRect = scene()->itemsBoundingRect();
	sceneRect.setTop(sceneRect.top()-((sceneRect.bottom()-sceneRect.top())/2.5));

    // set standard rect, if network is empty
    if (sceneRect.isEmpty()) {
        int viewportWidth = tgt::iround(width() * 0.9f);
        int viewportHeight = tgt::iround(height() * 0.9f);
        sceneRect.setCoords( -viewportWidth / 2, -viewportHeight / 2, viewportWidth / 2, viewportHeight / 2);
    }
    scene()->setSceneRect(sceneRect);

    //sceneRectSpacing
    sceneRect.adjust(sceneRectSpacing.x(), sceneRectSpacing.y(), sceneRectSpacing.width(), sceneRectSpacing.height());

    // calculate scale factors, take size of scroll bars into account
    float widthRatio = width() / sceneRect.width();
    float heightRatio = height() / sceneRect.height();
    float scaleFactor = std::min(widthRatio, heightRatio);

    // create some space around the objects
    scaleFactor = std::max(scaleFactor, maxFactor);
    scaleFactor *= scaleFactorFactor;

    scale(scaleFactor, scaleFactor);
    // calculate center
    setCenter(QPointF(sceneRect.left() + sceneRect.width() / 2,
                      sceneRect.top() + sceneRect.height() / 2));
    center();
    invalidateScene();
}

bool NetworkEditor::processorHasPropertyLinks(ProcessorGraphicsItem* processor) {
    foreach (LinkArrowGraphicsItem* arrow, linkArrows_) {
        if ((arrow->firstPropertyItem()->getProcessorGraphicsItem() == processor)  || (arrow->secondPropertyItem()->getProcessorGraphicsItem() == processor))
            return true;
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// slots
// ------------------------------------------------------------------------------------------------

void NetworkEditor::copyActionSlot() {
    // If we have any processors stores, we have to destroy them first
    if (clipboardProcessors_.count() != 0)
        clearClipboard();

    QList<QGraphicsItem*> selectedItems = scene()->selectedItems();

    foreach (QGraphicsItem* item, selectedItems) {
        ProcessorGraphicsItem* procGuiItem = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);

        if (procGuiItem) {
            // We store clones, so the processors within are save from deletion
            clipboardProcessors_.append(procGuiItem->clone());
        }
    }

    emit copySignal();
}

void NetworkEditor::pasteActionSlot() {
    foreach (ProcessorGraphicsItem* procGuiItem, clipboardProcessors_) {
        ProcessorGraphicsItem* p = procGuiItem->clone();
        connect(p, SIGNAL(pressedPropertyGraphicsItem(PropertyGraphicsItem*)), this, SLOT(pressedPropertyGraphicsItem(PropertyGraphicsItem*)));
        connect(p, SIGNAL(portConnectionsChanged()), this, SLOT(processorConnectionsChanged()));
        addProcessor(p);
        p->loadMeta();
        p->setPos(rightClickPosition_);
    }

    emit pasteSignal();
}

void NetworkEditor::deleteActionSlot() {
    removeSelectedItems();

    emit processorsSelected(std::vector<Processor*>());
}

void NetworkEditor::renameActionSlot() {
    QList<QGraphicsItem*> selected = scene()->selectedItems();
    if (selected.size() > 0)
        // static_cast is save here, because context menu only appears for GuiItems
        static_cast<ProcessorGraphicsItem*>(selected[0])->enterRenameMode();
}


// ------------------------------------------------------------------------------------------------
// event methods
// ------------------------------------------------------------------------------------------------

void NetworkEditor::dragMoveEvent(QDragMoveEvent*) { /*empty, required for drag'n'drop*/ }

void NetworkEditor::dragLeaveEvent(QDragLeaveEvent*) { /*empty, required for drag'n'drop*/ }

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
        ITooltip* tooltip = dynamic_cast<ITooltip*>(item);
        if (item && // Is there an item at the cursor's position?
            tooltip)  // Does it have a custom tooltip?
        {
            lastItemWithTooltip_ = tooltip;
            ttimer_->resetIfDistant(event->pos(),500);
        }
        else {
             if (ttimer_->isDistant(event->pos()))
                 hideTooltip();
        }

        foreach (ProcessorGraphicsItem* procItem, selectedProcessorItems())
            procItem->adjustArrows();

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

void NetworkEditor::keyPressEvent(QKeyEvent* event) {
    QGraphicsView::keyPressEvent(event);

    if (!event->isAccepted() && (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)) {
        deleteActionSlot();
    }
    else if (!event->isAccepted() && event->key() == Qt::Key_Plus) {
        QPointF p = mapToScene(QCursor::pos());
        translate(-p.x(), -p.y());
        float factor = keyPressScaleFactor;
        scale(factor, factor);
        translate(p.x(), p.y());
    }
    else if (!event->isAccepted() && event->key() == Qt::Key_Minus) {
        QPointF p = mapToScene(QCursor::pos());
        translate(-p.x(), -p.y());
        float factor = 1.f / keyPressScaleFactor;
        scale(factor, factor);
        translate(p.x(), p.y());
    }
}

void NetworkEditor::contextMenuEvent(QContextMenuEvent* event) {
    lastItemWithTooltip_ = 0;
    event->accept();
    rightClickPosition_ = mapToScene(event->pos());
    QGraphicsItem* item = itemAt(event->pos());
    if (item) {
        switch (item->type()) {
        case ProcessorGraphicsItem::Type:
            break;
        case TextGraphicsItem::Type:
            item = item->parentItem();
            break;
        case ArrowGraphicsItem::Type:
            return;
        case LinkArrowGraphicsItem::Type:
            selectedLink_ = qgraphicsitem_cast<LinkArrowGraphicsItem*>(item);
            rightClickLinkMenu_.popup(event->globalPos());
            return;
        default:
            return;
        }

        if (!item->isSelected()) {
            scene()->clearSelection();
            item->setSelected(true);
        }

        if (scene()->selectedItems().size() > 1)
            rightClickMenuMultiple_.popup(event->globalPos());
        else
            rightClickMenuSingle_.popup(event->globalPos());
    }
    else {
        // mouse cursor is not over a guiitem -> deselect all selected items
        scene()->clearSelection();
        rightClickMenuNone_.popup(event->globalPos());
    }
}

void NetworkEditor::dragEnterEvent(QDragEnterEvent* event) {
    if ((event->mimeData()->hasText()) || (event->mimeData()->hasFormat("application/x-voreenvolumesetpointer"))) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
    }
    else
        event->ignore();
}

void NetworkEditor::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasText()) {
        QString idString = event->mimeData()->text();
        QPoint position = event->pos();

        if (dynamic_cast<ProcessorListWidget*>(event->source()) != 0) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
            std::string(idString.toStdString());
            Processor* proc = ProcessorFactory::getInstance()->create(std::string(idString.toStdString()));
            if (proc) {
                std::string processorName = proc->getClassName();
                tgtAssert(!processorName.empty(), "Processor class name is empty");
                if (workspace_->getProcessorNetwork()->getProcessorByName(processorName)) {
                    // name not unique => add trailing number
                    int num = 2;
                    while (workspace_->getProcessorNetwork()->getProcessorByName(processorName)) {
                        std::ostringstream stream;
                        stream << processorName << " " << num;
                        std::string newName = stream.str();
                        if (!workspace_->getProcessorNetwork()->getProcessorByName(newName))
                            processorName = newName;
                        else
                            num++;
                    }
                }
                try {
                    workspace_->getProcessorNetwork()->addProcessor(proc, processorName);
                    ProcessorGraphicsItem* item = processorItemMap_[proc];
                    item->moveBy(mapToScene(position).x(), mapToScene(position).y());
                    item->saveMeta();
                    if (evaluator_)
                        evaluator_->onNetworkChange();
                }
                catch (VoreenException& e) {
                    LERROR(e.what());
                }
            }
            else {
                LERROR("Processor with type " << std::string(idString.toStdString()) << " could not be created");
            }
        }
        else {
            event->acceptProposedAction();
        }
    }
    else {
        event->ignore();
    }
}

void NetworkEditor::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);

    layoutLayerButtons();
    if (isVisible() && needsScale_) {
        scaleView(1.f);
        needsScale_ = false;
    }
}

// ------------------------------------------------------------------------------------------------
// tooltip methods
// ------------------------------------------------------------------------------------------------

void NetworkEditor::showTooltip(const QPoint& pos, ITooltip* ITooltip) {
    if (!activeTooltip_ && evaluator_ && !evaluator_->isLocked() && ITooltip) {
        activeTooltip_ = ITooltip->tooltip();
        if (activeTooltip_) {
            activeTooltip_->setPos(mapToScene(pos));
            activeTooltip_->setZValue(50); // above the rest
            scene()->addItem(activeTooltip_);
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

void NetworkEditor::removeSelectedItems() {
    hideTooltip();

    // make sure the evaluator does not operate on a temporarily inconsistent network
    evaluator_->lock();

    QList<QGraphicsItem*> selectedItems = scene()->selectedItems();

    // sort selectedItems by their type to not delete a port/arrow-item
    // that has already been deleted indirectly with the guiitem
    // so at first kick out the ports:
    foreach (QGraphicsItem* item, selectedItems) {
        switch (item->type()) {
        case PortGraphicsItem::Type:
        case TextGraphicsItem::Type:
        case PropertyGraphicsItem::Type:
            selectedItems.removeOne(item);
        }
    }

    // next delete arrows
    foreach (QGraphicsItem* item, selectedItems) {
        if (item->type() == ArrowGraphicsItem::Type) {
            ArrowGraphicsItem* arrow = qgraphicsitem_cast<ArrowGraphicsItem*>(item);
            removeArrowItem(arrow);
            selectedItems.removeOne(item);
        }
    }

    foreach (QGraphicsItem* item, selectedItems) {
        if (item->type() == LinkArrowGraphicsItem::Type) {

            LinkArrowGraphicsItem* arrow = qgraphicsitem_cast<LinkArrowGraphicsItem*>(item);
            removeArrowItem(arrow);
            selectedItems.removeOne(item);
        }
    }

    // finally delete processor items
    foreach (QGraphicsItem* item, selectedItems) {
        if (item->type() == ProcessorGraphicsItem::Type) {
            ProcessorGraphicsItem* processorItem = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);
            removeProcessorItem(processorItem);
            selectedItems.removeOne(item);
        }
    }

    resetCachedContent();

    // unlock and update evaluator
    evaluator_->unlock();
    evaluator_->onNetworkChange();
}

// ------------------------------------------------------------------------------------------------
// modify graphic items
// ------------------------------------------------------------------------------------------------

void NetworkEditor::addProcessor(ProcessorGraphicsItem* processor, const QPoint& pos) {
    addProcessor(processor);
    processor->moveBy(mapToScene(pos).x(), mapToScene(pos).y());
    processor->saveMeta();
}

void NetworkEditor::addProcessor(ProcessorGraphicsItem* processor) {
    // find unique name for processor
    std::string processorName = processor->getProcessor()->getClassName();
    tgtAssert(!processorName.empty(), "Processor class name is empty");
    if (workspace_->getProcessorNetwork()->getProcessorByName(processorName)) {
        // name not unique => add trailing number
        int num = 2;
        while (workspace_->getProcessorNetwork()->getProcessorByName(processorName)) {
            std::ostringstream stream;
            stream << processorName << " " << num;
            std::string newName = stream.str();
            if (!workspace_->getProcessorNetwork()->getProcessorByName(newName))
                processorName = newName;
        }
    }
    try {
        processorItemMap_[processor->getProcessor()] = processor;
        workspace_->getProcessorNetwork()->addProcessor(processor->getProcessor(), processorName);
        scene()->addItem(processor);
    }
    catch (VoreenException& e) {
        processorItemMap_.remove(processor->getProcessor());
        LERROR(e.what());
    }
	processor->setName(processorName);

    if (evaluator_)
        evaluator_->onNetworkChange();

    processorItemMap_[processor->getProcessor()] = processor;
    processor->saveMeta();

    processor->setLayer(currentLayer_);
}

void NetworkEditor::removeProcessorItem(ProcessorGraphicsItem* processorItem) {
    // remove connected arrows, also removes the connections in the backend
    processorItem->disconnectAll();

    foreach (LinkArrowGraphicsItem* arrow, linkArrows_) {
        if ((arrow->firstPropertyItem()->getProcessorGraphicsItem() == processorItem) || (arrow->secondPropertyItem()->getProcessorGraphicsItem() == processorItem))
            removeArrowItem(arrow);
    }

    // remove item from the scene
    scene()->removeItem(processorItem);

    // remove item from processor map
    processorItemMap_.remove(processorItem->getProcessor());

    workspace_->getProcessorNetwork()->removeProcessor(processorItem->getProcessor());
    evaluator_->onNetworkChange();

    delete processorItem;
}

void NetworkEditor::removeArrowItem(LinkArrowGraphicsItem* arrow) {
    linkArrows_.removeOne(arrow);
    //scene()->removeItem(arrow);
    workspace_->getProcessorNetwork()->removePropertyLink(arrow->getLink());

    if (arrow->hasSecondLink())
        workspace_->getProcessorNetwork()->removePropertyLink(arrow->getSecondLink());


    ProcessorGraphicsItem* sourceItem = arrow->firstPropertyItem()->getProcessorGraphicsItem();
    ProcessorGraphicsItem* destinationItem = arrow->secondPropertyItem()->getProcessorGraphicsItem();

    sourceItem->setLinkExists(processorHasPropertyLinks(sourceItem));
    destinationItem->setLinkExists(processorHasPropertyLinks(destinationItem));

    delete arrow;
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

void NetworkEditor::removePropertyLink(const PropertyLink* link) {
    foreach (LinkArrowGraphicsItem* arrow, linkArrows_) {
        if (arrow->getLink() == link) {
            if (arrow->hasSecondLink()) {
                arrow->switchLinks();
                arrow->setSecondLink(0);
            }
            else
                removeArrowItem(arrow);
        }
        else if (arrow->getSecondLink() == link) {
            arrow->setSecondLink(0);
        }
    }
}

void NetworkEditor::adjustLinkArrowGraphicsItems() {
    if (currentLayer_ == NetworkEditorLayerLinking) {
        foreach (LinkArrowGraphicsItem* arrow, linkArrows_) {
            arrow->show();
            arrow->adjust();
        }
    }
    else {
        foreach (LinkArrowGraphicsItem* arrow, linkArrows_) {
            arrow->hide();
        }
    }
    //scene()->invalidate();
}

// delete propertylink
void NetworkEditor::deletePropertyLinkSlot() {
    tgtAssert(selectedLink_, "stored pointer was 0");
    removeArrowItem(selectedLink_);
    selectedLink_ = 0;
}

// edit propertylink
void NetworkEditor::editPropertyLinkSlot(){
    PropertyLinkDialog::PropertyLinkSelectedButton linkType;
    if (selectedLink_->hasSecondLink())
        linkType = PropertyLinkDialog::BUTTON_BIDIRECTIONAL;
    else
        linkType = PropertyLinkDialog::BUTTON_TO_RIGHT;
        
    PropertyLinkDialog* dialog = new PropertyLinkDialog(this, selectedLink_->firstPropertyItem(),
                                                        selectedLink_->secondPropertyItem(), linkType);

    connect(dialog, SIGNAL(create(PropertyGraphicsItem*, PropertyGraphicsItem*, LinkEvaluatorBase*)),
            this, SLOT(editPropertyLink(PropertyGraphicsItem*, PropertyGraphicsItem*, LinkEvaluatorBase*)));
    dialog->exec();
}

void NetworkEditor::editPropertyLink(PropertyGraphicsItem* sourceItem, PropertyGraphicsItem* destinationItem,
                                     LinkEvaluatorBase* linkEvaluator) {
    // delete the old propertyLink
    if (selectedLink_)
        deletePropertyLinkSlot();

    // create new the link
    createNewLink(sourceItem, destinationItem, linkEvaluator);
}

void NetworkEditor::pressedPropertyGraphicsItem(PropertyGraphicsItem* propertyGraphicsItem) {
    if (selectedPropertyGraphicsItem_ == 0) {
        selectedPropertyGraphicsItem_ = propertyGraphicsItem;
    }
    else if (selectedPropertyGraphicsItem_ == propertyGraphicsItem) {
        propertyGraphicsItem->deselect();
        selectedPropertyGraphicsItem_ = 0;
    }
    else {
        selectedPropertyGraphicsItem_->deselect();
        propertyGraphicsItem->deselect();
        showLinkDialog(selectedPropertyGraphicsItem_, propertyGraphicsItem);
        selectedPropertyGraphicsItem_ = 0;
    }
}

void NetworkEditor::createNewLink(PropertyGraphicsItem* sourceItem, PropertyGraphicsItem* destinationItem, LinkEvaluatorBase* linkEvaluator) {
    foreach (LinkArrowGraphicsItem* arrow, linkArrows_) {
        if ((arrow->firstPropertyItem() == sourceItem) && (arrow->secondPropertyItem() == destinationItem))
            // there is a LinkArrowGraphicsItem for this exact pair of source and destination items
            return;
    }

    // the callback from the observer will go to NetworkEditor::propertyLinkAdded which will cause the arrow to be created
    PropertyLink* link = workspace_->getProcessorNetwork()->addPropertyLink(sourceItem->getProperty(), destinationItem->getProperty(), linkEvaluator);

    if (!link) {
        QString text = tr("The link between \"%1\" and \"%2\" could not be created.\nMost likely the property types cannot be linked or a link already exists.\nSee Log for detailed information").arg(
            QString::fromStdString(sourceItem->getProperty()->getGuiText()), QString::fromStdString(destinationItem->getProperty()->getGuiText()));
        QMessageBox::warning(this, tr("Error in link creation"), text);
    }
}

LinkArrowGraphicsItem* NetworkEditor::createLinkArrowForPropertyLink(const PropertyLink* link) {
    if (link == 0)
        return 0;

    // find propertygraphicsitem
    PropertyGraphicsItem* sourceProp = 0;
    PropertyGraphicsItem* destinationProp = 0;

    foreach (ProcessorGraphicsItem* procItem, processorItemMap_.values()) {
        sourceProp = procItem->getProperty(link->getSourceProperty());
        if (sourceProp)
            break;
    }

    foreach (ProcessorGraphicsItem* procItem, processorItemMap_.values()) {
        destinationProp = procItem->getProperty(link->getDestinationProperty());
        if (destinationProp)
            break;
    }

    // if there is a LinkArrowGraphicsItem which has the source and destination item reversed, we need to add the new link to that one
    foreach (LinkArrowGraphicsItem* arrow, linkArrows_) {
        if ((arrow->firstPropertyItem() == destinationProp) && (arrow->secondPropertyItem() == sourceProp)) {
            if (!arrow->hasSecondLink()) {
                arrow->setSecondLink(link);
                return arrow;
            }
            else
                return 0;
        }
    }

    LinkArrowGraphicsItem* result = new LinkArrowGraphicsItem(sourceProp, destinationProp, link);
    linkArrows_.append(result);

    if (currentLayer_ == NetworkEditorLayerLinking)
        result->show();
    else
        result->hide();
    scene()->addItem(result);
    sourceProp->getProcessorGraphicsItem()->setLinkExists(true);
    destinationProp->getProcessorGraphicsItem()->setLinkExists(true);

    return result;
}

void NetworkEditor::showLinkDialog(PropertyGraphicsItem* propertyItem1, PropertyGraphicsItem* propertyItem2) {
    PropertyLinkDialog* dialog =  new PropertyLinkDialog(this, propertyItem1, propertyItem2);
    connect(dialog, SIGNAL(create(PropertyGraphicsItem*, PropertyGraphicsItem*, LinkEvaluatorBase*)),
            this, SLOT(createNewLink(PropertyGraphicsItem*, PropertyGraphicsItem*, LinkEvaluatorBase*)));
    dialog->exec();
}

void NetworkEditor::linkCameras() {
    
    tgtAssert(workspace_, "No workspace");
    ProcessorNetwork* processorNetwork = workspace_->getProcessorNetwork();
    if (!processorNetwork)
        return;

    if (QMessageBox::question(this, tr("VoreenVE"), tr("Link all cameras in the current network?"),
        QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) {
            return;
    }

    // get camera properties
    std::vector<CameraProperty*> camProps;
    std::vector<Processor*> processors = processorNetwork->getProcessors();
    for (size_t i=0; i<processors.size(); ++i) {
        std::vector<Property*> props = processors[i]->getProperties();
        for (size_t j=0; j<props.size(); ++j) {
            if (dynamic_cast<CameraProperty*>(props[j]))
                camProps.push_back(static_cast<CameraProperty*>(props[j]));
        }
    }

    if (camProps.empty())
        return;
    else if (camProps.size() > 2)
        camProps.push_back(camProps[0]);

    int numLinks = 0;
    for (size_t i=0; i<camProps.size()-1; ++i) {
        if (!processorNetwork->arePropertiesLinked(camProps[i], camProps[i+1]))
            if (processorNetwork->addPropertyLink(camProps[i], camProps[i+1]))
                numLinks++;
        if (!processorNetwork->arePropertiesLinked(camProps[i+1], camProps[i]))
            if (processorNetwork->addPropertyLink(camProps[i+1], camProps[i]))
                numLinks++;
    }

    if (numLinks > 0)
        LINFO("Created " << numLinks << " links between " << camProps.size() << " camera properties");

}

void NetworkEditor::linkCamerasOfProcessor(const Processor* processor) {
    tgtAssert(workspace_, "No workspace");
    ProcessorNetwork* processorNetwork = workspace_->getProcessorNetwork();
    if (!processorNetwork)
        return;

    // get camera properties of processor
    std::vector<CameraProperty*> camPropsProcessor;
    std::vector<Property*> props = processor->getProperties();
    for (size_t j=0; j<props.size(); ++j) {
        if (dynamic_cast<CameraProperty*>(props[j]))
            camPropsProcessor.push_back(static_cast<CameraProperty*>(props[j]));
    }

    // get camera properties of other processor in the network
    std::vector<CameraProperty*> camPropsNetwork;
    std::vector<Processor*> processors = processorNetwork->getProcessors();
    for (size_t i=0; i<processors.size(); ++i) {
        if (processors[i] == processor)
            continue;
        std::vector<Property*> props = processors[i]->getProperties();
        for (size_t j=0; j<props.size(); ++j) {
            if (dynamic_cast<CameraProperty*>(props[j]))
                camPropsNetwork.push_back(static_cast<CameraProperty*>(props[j]));
        }
    }

    if (camPropsProcessor.empty() || camPropsNetwork.empty())
        return;

    // connect each processor camera with all cameras in the network
    int numLinks = 0;
    for (size_t i=0; i<camPropsProcessor.size(); ++i) {
        for (size_t j=0; j<camPropsNetwork.size(); ++j) {
            if (!processorNetwork->arePropertiesLinked(camPropsNetwork[j], camPropsProcessor[i])) {
                if (processorNetwork->addPropertyLink(camPropsNetwork[j], camPropsProcessor[i]))
                    numLinks++;
            }
            if (!processorNetwork->arePropertiesLinked(camPropsProcessor[i], camPropsNetwork[j]))
                if (processorNetwork->addPropertyLink(camPropsProcessor[i], camPropsNetwork[j]))
                    numLinks++;
        }
    }

    LINFO("Created " << numLinks << " links between " << (camPropsProcessor.size() + camPropsNetwork.size()) << " camera properties");
}

void NetworkEditor::removePropertyLinks() {
    tgtAssert(workspace_, "No workspace");

    if (QMessageBox::question(this, tr("VoreenVE"), tr("Remove all property links from the current network?"),
        QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) {
            return;
    }

    ProcessorNetwork* processorNetwork = workspace_->getProcessorNetwork();
    if (!processorNetwork)
        return;

    std::vector<PropertyLink*> links = processorNetwork->getPropertyLinks();
    for (size_t i=0; i<links.size(); ++i) {
        processorNetwork->removePropertyLink(links[i]);
    }

    LINFO("Removed " << links.size() << " property links");


}


// ------------------------------------------------------------------------------------------------
// layer methods
// ------------------------------------------------------------------------------------------------

void NetworkEditor::setLayer(NetworkEditorLayer layer) {
    currentLayer_ = layer;

    foreach (ProcessorGraphicsItem* procItem, processorItemMap_.values()) {
        procItem->setLinkExists(processorHasPropertyLinks(procItem));
        procItem->setLayer(layer);
    }
    selectedPropertyGraphicsItem_ = 0;

    if (layer == NetworkEditorLayerLinking) {
        linkCamerasButton_->setVisible(true);
        removePropertyLinksButton_->setVisible(true);
        linkCamerasAutoButton_->setVisible(true);
    }
    else {
        linkCamerasButton_->setVisible(false);
        removePropertyLinksButton_->setVisible(false);
        linkCamerasAutoButton_->setVisible(false);
    }

    adjustLinkArrowGraphicsItems();
    resetCachedContent();

}

void NetworkEditor::setLayerToDataflow() {
    setLayer(NetworkEditorLayerDataflow);
}

void NetworkEditor::setLayerToLinking() {
    setLayer(NetworkEditorLayerLinking);
}

NetworkEditorLayer NetworkEditor::currentLayer() const {
    return currentLayer_;
}

// ------------------------------------------------------------------------------------------------
// networkobserver methods
// ------------------------------------------------------------------------------------------------

void NetworkEditor::networkChanged() {
    //newNetwork();
}

void NetworkEditor::processorAdded(const Processor* processor) {
    if (processorItemMap_.find(const_cast<Processor*>(processor)) == processorItemMap_.end()) {
        ProcessorGraphicsItem* item = createProcessorGraphicsItem(const_cast<Processor*>(processor));
        scene()->addItem(item);
        if (linkCamerasAutoButton_->isChecked())
            linkCamerasOfProcessor(processor);
    }
}

void NetworkEditor::processorRemoved(const Processor* processor) {
    if (processorItemMap_.find(const_cast<Processor*>(processor)) != processorItemMap_.end()) {
        Processor* nonConst = const_cast<Processor*>(processor);
        ProcessorGraphicsItem* item = processorItemMap_.value(nonConst);
        scene()->removeItem(item);
        processorItemMap_.remove(nonConst);
    }

    //QRectF sceneRect = scene()->itemsBoundingRect();
    //scene()->setSceneRect(sceneRect);
}

//void NetworkEditor::connectionsChanged() {
//}

void NetworkEditor::propertyLinkAdded(const PropertyLink* link) {
    /*LinkArrowGraphicsItem* arrow = */createLinkArrowForPropertyLink(link);
    //arrow->firstPropertyItem()->getProcessorGraphicsItem()->setLayer(currentLayer_);
    //arrow->secondPropertyItem()->getProcessorGraphicsItem()->setLayer(currentLayer_);
}

void NetworkEditor::propertyLinkRemoved(const PropertyLink* link) {
    removePropertyLink(link);
}


} // namespace voreen
