/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "networkeditor.h"

#include "voreen/qt/widgets/processor/qprocessorwidgetfactory.h"

#include "aggregationgraphicsitem.h"
#include "arrowheadselectiongraphicsitem.h"
#include "linkarrowgraphicsitem.h"
#include "portarrowgraphicsitem.h"
#include "portgraphicsitem.h"
#include "processorgraphicsitem.h"
#include "progressbargraphicsitem.h"
#include "propertygraphicsitem.h"
#include "propertylinkdialog.h"
#include "textgraphicsitem.h"

#include "voreen/core/io/serialization/meta/aggregationmetadata.h"
#include "voreen/core/io/serialization/meta/primitivemetadata.h"
#include "voreen/core/io/serialization/meta/selectionmetadata.h"
#include "voreen/core/io/serialization/meta/zoommetadata.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/processors/processorfactory.h"
#include "voreen/core/ports/port.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/propertyowner.h"
#include "voreen/core/properties/link/dependancylinkevaluatorbase.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "voreen/core/network/workspace.h"
#include "voreen/core/io/serialization/xmlserializer.h"
#include "voreen/core/io/serialization/xmldeserializer.h"
#include "voreen/core/processors/processorfactory.h"
#include "voreen/core/datastructures/transfunc/transfuncfactory.h"
#include "voreen/modules/base/processors/datasource/volumesource.h"
#include "voreen/modules/base/processors/datasource/volumecollectionsource.h"

#include "hastooltip.h"

#include <QApplication>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QClipboard>
#include <QComboBox>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPoint>
#include <QTimer>
#include <QToolButton>
#include <QTransform>
#include <QSpinBox>

#include <cmath>
#include <iostream>

namespace {
    const QSize minimumSize = QSize(200, 200);
    const qreal tooltiptimerRadius = 3.f;
    const QSize layerButtonSize = QSize(30, 30);
    const QSize autoLinkingSize = QSize(30, 30);
    const QSize otherButtonSize = QSize(30, 30);
    const int layerButtonSpacingX = 10;
    const int layerButtonSpacingY = 10;
    const int otherButtonSpacingX = 10;
    const int otherButtonSpacingY = 10;
    const QRectF sceneRectSpacing = QRect(-10, -10, 10, 10);
    const qreal scaleFactorFactor = 0.9f;
    const float minimalScaleFactor = 1.25f;
    const qreal keyPressScaleFactor = pow(2.f, 0.35f);

    tgt::Matrix3d QTransformToMat3(const QTransform& transform) {
        return tgt::mat3(transform.m11(), transform.m12(), transform.m13(),
                         transform.m21(), transform.m22(), transform.m23(),
                         transform.m31(), transform.m32(), transform.m33());
    }

    QTransform Mat3ToQTransform(const tgt::Matrix3d& matrix) {
        return QTransform(matrix.t00, matrix.t01, matrix.t02,
                          matrix.t10, matrix.t11, matrix.t12,
                          matrix.t20, matrix.t21, matrix.t22);
    }
}

namespace voreen {

namespace {
    bool isEqual(AggregationMetaData* metaData, AggregationGraphicsItem* graphicsItem) {
        std::vector<Processor*> metaDataProcessors = metaData->getProcessors();
        QList<Processor*> graphicsItemProcessors = graphicsItem->getProcessors();

        foreach (Processor* metaDataProcessor, metaDataProcessors) {
            int index = graphicsItemProcessors.indexOf(metaDataProcessor);
            if (index == -1)
                return false;
        }

        return true;
    }

    void disconnectPorts(PortGraphicsItem* outport, PortGraphicsItem* inport) {
        tgtAssert(inport, "inport is null");
        tgtAssert(outport->isOutport(), "disconnect called on inport");

        inport->removeConnection(outport);
        //// remove outport from inports connectedPorts
        //const QList<PortGraphicsItem*>& connected = inport->getConnectedPorts();
        //for (int i = 0; i < connected.size(); ++i) {
        //    if (connected[i] == outport)
        //        connected.erase(connected.begin() + i);
        //}

        outport->removeConnection(inport);
        //// remove inport from outports connectedPorts
        //connected = outport->getConnected();
        //for (int i = 0; i < connected.size(); ++i) {
        //    if (connected[i] == inport)
        //        connected.erase(connected.begin() + i);
        //}

        // delete arrow
        const QList<PortArrowGraphicsItem*>& arrowList = outport->getArrowList();
        foreach (PortArrowGraphicsItem* arrow, arrowList) {
            if (arrow->getDestinationItem() == inport) {
                outport->removeArrow(arrow);
                delete arrow;
            }
        }
    }

    bool clipboardHasValidContent() {
        QClipboard* clipboard = QApplication::clipboard();
        QString clip = clipboard->text();
        std::stringstream stream(clip.toStdString());

        bool singleProcessors = true;
        bool aggregations = true;

        XmlDeserializer d;
        d.registerFactory(ProcessorFactory::getInstance());
        d.registerFactory(TransFuncFactory::getInstance());
        try {
            d.read(stream);
        }
        catch (SerializationException&) {
            return false;
        }
        try {
            std::vector<Processor*> processors;
            d.deserialize("SingleProcessors", processors);
        }
        catch (SerializationException&) {
            singleProcessors = false;
        }

        try {
            std::vector<AggregationMetaData> aggregationMetaData;
            d.deserialize("Aggregations", aggregationMetaData);
        }
        catch (SerializationException&) {
            aggregations = false;
        }

        if (!singleProcessors && !aggregations)
            return false;
        else
            return true;
    }

} // namespace

// Helper class for tooltip timer events
class TooltipTimer : public QTimer {
public:
    TooltipTimer(QPoint p, qreal radius, QObject* parent = 0)
        : QTimer(parent)
        , p_(p)
        , radius_(radius)
        {}

    qreal radius() {
        return radius_;
    }

    void setRadius(qreal radius) {
        radius_ = radius;
    }

    QPointF point() {
        return p_;
    }

    void setPoint(QPoint p) {
        p_ = p;
    }

    qreal distance(const QPoint & p) const {
        return std::sqrt(pow(p_.x()-p.x(),2.)+pow(p_.y()-p.y(),2.));
    }

    /**
     * This will stop the timer if p is farer away from the initialisation point than radius and returns true
     * otherwise false
     */
    bool isDistant(const QPoint & p) {
        if (distance(p) > radius_) {
            stop();
            return true;
        }
        else {
            return false;
        }
    }

    /**
     * This will restart the timer if p is farer away from the initialisation point than radius and set the point to p
     */
    void resetIfDistant(const QPoint & p, int msec) {
        if (isDistant(p)) {
            p_ = p;
            start(msec);
        }
    }

protected:
    QPoint p_;
    qreal radius_;
};

class LinkEvaluatorBase;

const std::string NetworkEditor::loggerCat_("voreen.NetworkEditor");

NetworkEditor::NetworkEditor(QWidget* parent, ProcessorNetwork* network, NetworkEvaluator* evaluator)
    : QGraphicsView(parent)
    , processorNetwork_(network)
    , evaluator_(evaluator)
    , selectedPortArrow_(0)
    , selectedLinkArrow_(0)
    , needsScale_(false)
    , networkEvaluatorIsLockedByButton_(false)
    , activateTooltips_(true)
    , activeTooltip_(0)
    , layerButtonContainer_(0)
    , dataflowLayerButton_(0)
    , linkingLayerButton_(0)
    , otherButtonContainer_(0)
    , stopNetworkEvaluatorButton_(0)
    , currentLayer_(NetworkEditorLayerUndefined)
{
    tgtAssert(evaluator_ != 0, "passed null pointer");
    //tgtAssert(getProcessorNetwork() != 0, "no network available");

    setScene(new QGraphicsScene(this));

    translateScene_ = false;

    setBackgroundBrush(QBrush(Qt::darkGray));
    setCacheMode(CacheBackground);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setResizeAnchor(AnchorViewCenter);
    setMouseTracking(true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setMinimumSize(QSize(200, 200));

    createContextMenuActions();
    createTimer();
    createLayerButtons();
    createOtherButtons();
}

NetworkEditor::~NetworkEditor() {
    //clearClipboard();
    //hideTooltip();
    //delete ttimer_;
}

// ------------------------------------------------------------------------------------------------
// initialization methods
// ------------------------------------------------------------------------------------------------

void NetworkEditor::generateGraphicsItems() {

    if (!getProcessorNetwork())
        return;

    foreach (Processor* proc, getProcessorNetwork()->getProcessors())
        createProcessorGraphicsItem(proc);

    foreach (Processor* proc, getProcessorNetwork()->getProcessors()) {
        std::vector<Port*> outports = proc->getOutports();
        std::vector<CoProcessorPort*> coprocessoroutports = proc->getCoProcessorOutports();
        // append coprocessoroutports to outports because we can handle them identically
        outports.insert(outports.end(), coprocessoroutports.begin(), coprocessoroutports.end());

        foreach (Port* port, outports) {
            std::vector<Port*> connectedPorts = port->getConnected();

            foreach (Port* connectedPort, connectedPorts)
                portConnectionAdded(port, connectedPort);
        }
    }

    foreach (ProcessorGraphicsItem* procItem, processorItemMap_.values())
        scene()->addItem(procItem);

    foreach (PropertyLink* link, getProcessorNetwork()->getPropertyLinks()) {
        createLinkArrowForPropertyLink(link);
    }
}

void NetworkEditor::createContextMenuActions() {
    copyAction_ = new QAction(QIcon(":/voreenve/icons/edit-copy.png"), tr("Copy"), this);
    pasteAction_ = new QAction(QIcon(":/voreenve/icons/edit-paste.png"), tr("Paste"), this);
    replaceAction_ = new QAction(QIcon(":/voreenve/icons/edit-paste.png"), tr("Replace"), this);
    deleteAction_ = new QAction(QIcon(":/voreenve/icons/eraser.png"), tr("Delete"), this);
    deleteLinkAction_ = new QAction(QIcon(":/voreenve/icons/eraser.png"), tr("Delete"), this);
    renameAction_ = new QAction(QIcon(":/voreenve/icons/rename.png"), tr("Rename"), this);
    editLinkAction_ = new QAction(tr("Edit"), this);
    aggregateAction_ = new QAction(QIcon(":/voreenve/icons/aggregate.png"), tr("Aggregate"), this);
    deaggregateAction_ = new QAction(QIcon(":/voreenve/icons/deaggregate.png"), tr("Deaggregate"), this);  // this action will be added to the menus on demand
    clearDependancyHistoryAction_ = new QAction(QIcon(":/voreenve/icons/clear_dependancy.png"), tr("Clear dependancy history"), this); // this action will be added to the menus on demand

    connect(copyAction_, SIGNAL(triggered()), this, SLOT(copyActionSlot()));
    connect(pasteAction_, SIGNAL(triggered()), this, SLOT(pasteActionSlot()));
    connect(replaceAction_, SIGNAL(triggered()), this, SLOT(replaceActionSlot()));
    connect(deleteAction_, SIGNAL(triggered()), this, SLOT(deleteActionSlot()));
    connect(renameAction_, SIGNAL(triggered()), this, SLOT(renameActionSlot()));
    connect(deleteLinkAction_, SIGNAL(triggered()), this, SLOT(deletePropertyLinkSlot()));
    connect(editLinkAction_, SIGNAL(triggered()), this, SLOT(editPropertyLinkSlot()));
    connect(aggregateAction_, SIGNAL(triggered()), this, SLOT(aggregateActionSlot()));
    connect(deaggregateAction_, SIGNAL(triggered()), this, SLOT(deaggregateActionSlot()));
    connect(clearDependancyHistoryAction_, SIGNAL(triggered()), this, SLOT(clearDependancyHistory()));
}

void NetworkEditor::createTimer() {
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
    connect(linkCamerasAutoButton_, SIGNAL(clicked()), this, SLOT(linkCamerasAutoChanged()));
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

void NetworkEditor::createOtherButtons() {
    otherButtonContainer_ = new QWidget(this);
    QBoxLayout* otherLayout = new QHBoxLayout(otherButtonContainer_);

    stopNetworkEvaluatorButton_ = new QToolButton;
    stopNetworkEvaluatorButton_->setIcon(QIcon(":/voreenve/icons/player-pause.png"));
    stopNetworkEvaluatorButton_->setIconSize(otherButtonSize);
    stopNetworkEvaluatorButton_->setToolTip(tr("Stop the automatic evaluation of the network"));
    connect(stopNetworkEvaluatorButton_, SIGNAL(clicked()), this, SLOT(toggleNetworkEvaluator()));
    stopNetworkEvaluatorButton_->setCheckable(true);
    otherLayout->addWidget(stopNetworkEvaluatorButton_);

    layoutOtherButtons();
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

void NetworkEditor::layoutOtherButtons() {
    int x = size().width() - otherButtonContainer_->size().width() - otherButtonSpacingX;;
    int y = size().height() - otherButtonContainer_->size().height() - otherButtonSpacingY;

    otherButtonContainer_->move(x,y);
}

ProcessorGraphicsItem* NetworkEditor::createProcessorGraphicsItem(Processor* processor) {
    ProcessorGraphicsItem* result = new ProcessorGraphicsItem(processor, this);
    processorItemMap_[processor] = result;
    result->setLayer(currentLayer_);
    result->loadMeta();

    connect(result, SIGNAL(createLink(RootGraphicsItem*, RootGraphicsItem*)), this, SLOT(createLink(RootGraphicsItem*, RootGraphicsItem*)));
    connect(result, SIGNAL(startedArrow()), this, SLOT(disableTooltips()));
    connect(result, SIGNAL(endedArrow()), this, SLOT(enableTooltips()));

    return result;
}

void NetworkEditor::resetScene() {
    emit update();
    processorItemMap_.clear();
    aggregationItems_.clear();
    hideTooltip(); // prevent double-free

    // deletion is necessary because the QGraphicsScene's sceneRect will consider all added items
    delete scene();
    setScene(new QGraphicsScene(this));
}

void NetworkEditor::setProcessorNetwork(ProcessorNetwork* network) {

    hideTooltip();

    // a locked evaluator leads to a crash if a new network is loaded
    evaluator_->unlock();
    networkEvaluatorIsLockedByButton_ = false;
    stopNetworkEvaluatorButton_->setIcon(QIcon(":/voreenve/icons/player-pause.png"));
    stopNetworkEvaluatorButton_->setToolTip(tr("Stop the automatic evaluation of the network"));
    stopNetworkEvaluatorButton_->setChecked(false);

    if (processorNetwork_)
        processorNetwork_->removeObserver(this);

    processorNetwork_ = network;

    if (processorNetwork_)
        processorNetwork_->addObserver(this);

    resetScene();
    linkMap_.clear();

    generateGraphicsItems();

    foreach (ProcessorGraphicsItem* processorItem, processorItemMap_.values())
        processorItem->removeAllAggregationPrefixes();

    foreach (AggregationGraphicsItem* aggregationItem, aggregationItems_)
        aggregationItem->removeAllAggregationPrefixes();

    if (getProcessorNetwork()) {
        AggregationMetaDataContainer* aggregationMetaDataContainer = dynamic_cast<AggregationMetaDataContainer*>(getProcessorNetwork()->getMetaDataContainer().getMetaData("Aggregation"));
        if (aggregationMetaDataContainer) {
            std::vector<AggregationMetaData*> aggregations = aggregationMetaDataContainer->getAggregations();
            aggregationMetaDataContainer->clearAggregations();

            foreach (AggregationMetaData* metaData, aggregations)
                createAggregationGraphicsItem(metaData);
        }
    }

    resetMatrix();
    resetTransform();

    // do not scale immediately when we might be in visualization mode and this window might
    // not have its final size yet
    if (isVisible()) {
        scaleView();
        needsScale_ = false;
    } else {
        needsScale_ = true;
    }

    dataflowLayerButton_->setChecked(true);
    setLayer(NetworkEditorLayerDataflow);

    // set state of cam auto-linking button according to stored meta data, if present
    BoolMetaData* cameraLinkMeta = 0;
    if (getProcessorNetwork()) {
        cameraLinkMeta = dynamic_cast<BoolMetaData*>(getProcessorNetwork()->getMetaDataContainer().getMetaData("autoLinkCameras"));
    }
    if (cameraLinkMeta)
        linkCamerasAutoButton_->setChecked(cameraLinkMeta->getValue());
    else
        linkCamerasAutoButton_->setChecked(true);
}

void NetworkEditor::selectPreviouslySelectedProcessors() {

    if (!getProcessorNetwork())
        return;

    SelectionMetaData<Processor*>* selectionMetaData = dynamic_cast<SelectionMetaData<Processor*>*>(getProcessorNetwork()->getMetaDataContainer().getMetaData("ProcessorSelection"));
    if (selectionMetaData) {
        QList<Processor*> selectedProcessors = stdVectorToQList(selectionMetaData->getValues());
        foreach (Processor* proc, selectedProcessors) {
            bool alreadyFound = false;

            // first search all the Aggregations if any Aggregation contains the processor
            foreach (AggregationGraphicsItem* aggItem, aggregationItems_) {
                if (aggItem->contains(proc)) {
                    alreadyFound = true;
                    aggItem->setSelected(true);
                    break;
                }
            }

            if (alreadyFound)
                continue;

            // we get here if no AggregationGraphicsItem contains the processor
            ProcessorGraphicsItem* item = getProcessorGraphicsItem(proc);

            // item might not exist (e.g. network was build with a voreen application using different defines)
            if (item)
                item->setSelected(true);
        }

        emit processorsSelected(selectedProcessors);
    }
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

ProcessorNetwork* NetworkEditor::getProcessorNetwork() {
    return processorNetwork_;
}

void NetworkEditor::processorConnectionsChanged() {
    // evaluator_->onNetworkChange();  // happens implicitly through network invalidations
}

QSize NetworkEditor::sizeHint() const {
    return QSize(400, 600);
}

void NetworkEditor::updateSelectedItems() {

    if (!getProcessorNetwork())
        return;

    QList<QGraphicsItem*> selectedItems = scene()->selectedItems();

    QList<Processor*> selectedProcessors;
    foreach (QGraphicsItem* selectedItem, selectedItems) {
        bool isRootItem = (selectedItem->type() == ProcessorGraphicsItem::Type) || (selectedItem->type() == AggregationGraphicsItem::Type);

        if (isRootItem) {
            RootGraphicsItem* rootItem = static_cast<RootGraphicsItem*>(selectedItem);
            selectedProcessors += rootItem->getProcessors();
            rootItem->saveMeta();
        }
    }

    if (selectedProcessors.isEmpty())
        getProcessorNetwork()->getMetaDataContainer().removeMetaData("ProcessorSelection");
    else {
        std::vector<Processor*> selectedProc = qListToStdVector(selectedProcessors);
        getProcessorNetwork()->getMetaDataContainer().addMetaData("ProcessorSelection", new SelectionMetaData<Processor*>(selectedProc));
    }

    emit processorsSelected(selectedProcessors);
}

void NetworkEditor::scaleView() {

    if (!getProcessorNetwork())
        return;

    QRectF sceneRect;
    // scene()->itemsBoundingRect() will consider all invisible (i.e. properties) as well, so we have to to it by ourselves
    foreach (QGraphicsItem* item, scene()->items()) {
        if (item->isVisible()) {
#if QT_VERSION >= 0x040500
            QRectF iRect = item->mapRectToScene(item->boundingRect());
#else
            QRectF old = item->boundingRect();
            QRectF iRect(item->mapToScene(old.topLeft()), item->mapToScene(old.bottomRight()));
#endif
            sceneRect = sceneRect.united(iRect);
        }
    }

    // set standard rect, if network is empty
    if (sceneRect.isEmpty()) {
        int viewportWidth = tgt::iround(width() * 0.9f);
        int viewportHeight = tgt::iround(height() * 0.9f);
        sceneRect.setCoords( -viewportWidth / 2, -viewportHeight / 2, viewportWidth / 2, viewportHeight / 2);
    }

    sceneRect.adjust(sceneRectSpacing.x(), sceneRectSpacing.y(), sceneRectSpacing.width(), sceneRectSpacing.height());
    //scene()->setSceneRect(sceneRect);


    ZoomMetaData* meta = dynamic_cast<ZoomMetaData*>(getProcessorNetwork()->getMetaDataContainer().getMetaData("Zoom"));
    if (meta) {
        setTransform(Mat3ToQTransform(meta->getTransform()));
    }
    else {
        QPointF center = QPointF(sceneRect.x() + sceneRect.width() / 2.f, sceneRect.y() + sceneRect.height() / 2.f);
        centerOn(center);

        // calculate scale factors, take size of scroll bars into account
        float widthRatio = static_cast<qreal>(width()) / sceneRect.width();
        float heightRatio = static_cast<qreal>(height()) / sceneRect.height();
        float scaleFactor = std::min(widthRatio, heightRatio);

        // create some space around the objects
        scaleFactor = std::min(scaleFactor, minimalScaleFactor);
        scaleFactor *= scaleFactorFactor;

        scale(scaleFactor, scaleFactor);
    }

    invalidateScene(QRectF(), QGraphicsScene::ForegroundLayer);
}

void NetworkEditor::scale(qreal sx, qreal sy) {

    if (getProcessorNetwork()) {
        QTransform transformMatrix = transform();
        if (transformMatrix.isIdentity())
            getProcessorNetwork()->getMetaDataContainer().removeMetaData("Zoom");
        else {
            tgtAssert(sx == sy, "no rectangular zoom performed");
            if (getProcessorNetwork()->getMetaDataContainer().hasMetaData("Zoom")) {
                MetaDataBase* base = getProcessorNetwork()->getMetaDataContainer().getMetaData("Zoom");
                ZoomMetaData* meta = dynamic_cast<ZoomMetaData*>(base);
                meta->setTransform(QTransformToMat3(transformMatrix));
            } else
                getProcessorNetwork()->getMetaDataContainer().addMetaData("Zoom", new ZoomMetaData(QTransformToMat3(transformMatrix)));
        }
    }
    QGraphicsView::scale(sx, sy);
}

void NetworkEditor::translate(qreal dx, qreal dy) {
    if (getProcessorNetwork()) {
        if (getProcessorNetwork()->getMetaDataContainer().hasMetaData("Zoom")) {
            MetaDataBase* base = getProcessorNetwork()->getMetaDataContainer().getMetaData("Zoom");
            ZoomMetaData* meta = dynamic_cast<ZoomMetaData*>(base);
            meta->setTransform(QTransformToMat3(transform()));
        }
        else {
            getProcessorNetwork()->getMetaDataContainer().addMetaData("Zoom", new ZoomMetaData(QTransformToMat3(transform())));
        }
    }

    QGraphicsView::translate(dx, dy);
}

ProcessorGraphicsItem* NetworkEditor::getProcessorGraphicsItem(Processor* processor) {
    tgtAssert(processorItemMap_.contains(processor), "processorItemMap didn't contain the processor");
    return processorItemMap_[processor];
}

// ------------------------------------------------------------------------------------------------
// slots
// ------------------------------------------------------------------------------------------------

void NetworkEditor::copyActionSlot() {
    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so copying is not allowed"));
        return;
    }

    std::stringstream stream;

    XmlSerializer s;
    s.registerFactory(ProcessorFactory::getInstance());
    s.registerFactory(TransFuncFactory::getInstance());

    std::vector<Processor*> processors;
    std::vector<AggregationMetaData> aggregations;

    // these maps store the old values of the corresponding processors
    // so we are able to restore the old values after the serialization
    QMap<Processor*, VolumeHandle*> volumeSourceMap;
    QMap<Processor*, VolumeCollection*> volumeCollectionSourceMap;

    foreach (QGraphicsItem* item, scene()->selectedItems()) {
        switch (item->type()) {
        case ProcessorGraphicsItem::Type:
            {
                ProcessorGraphicsItem* procItem = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);
                Processor* processor = procItem->getProcessor();

                // special treatment for VolumeSource and VolumeCollection because they shouldn't
                // serialize their volume on a copy'n'paste
                VolumeSource* volumeSource = dynamic_cast<VolumeSource*>(processor);
                if (volumeSource) {
                    volumeSourceMap.insert(processor, volumeSource->getVolumeHandle());
                    volumeSource->setVolumeHandle(0);
                }

                VolumeCollectionSource* volumeCollection = dynamic_cast<VolumeCollectionSource*>(processor);
                if (volumeCollection) {
                    volumeCollectionSourceMap.insert(processor, volumeCollection->getVolumeCollection());
                    volumeCollection->setVolumeCollection(0);
                }

                processors.push_back(processor);
                break;
            }
        case AggregationGraphicsItem::Type:
            {
                AggregationGraphicsItem* aggItem = qgraphicsitem_cast<AggregationGraphicsItem*>(item);
                std::vector<Processor*> processors = qListToStdVector(aggItem->getProcessors());
                AggregationMetaData agg;
                agg.setProcessors(processors);
                agg.setName(aggItem->getName().toStdString());
                aggregations.push_back(agg);
                break;
            }
        }
    }
    s.serialize("SingleProcessors", processors);
    s.serialize("Aggregations", aggregations);

    s.write(stream);

    QString value = QString::fromStdString(stream.str());

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(value);

    // now we can reset the VolumeSources and VolumeCollections
    foreach (Processor* processor, volumeSourceMap.keys()) {
        VolumeSource* volumeSource = static_cast<VolumeSource*>(processor);
        volumeSource->setVolumeHandle(volumeSourceMap[processor]);
    }

    foreach (Processor* processor, volumeCollectionSourceMap.keys()) {
        VolumeCollectionSource* volumeCollection = static_cast<VolumeCollectionSource*>(processor);
        volumeCollection->setVolumeCollection(volumeCollectionSourceMap[processor]);
    }
}

void NetworkEditor::pasteActionSlot() {
    if (!getProcessorNetwork())
        return;

    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so pasting is not allowed"));
        return;
    }

    QClipboard* clipboard = QApplication::clipboard();
    QString clip = clipboard->text();
    std::stringstream stream(clip.toStdString());

    XmlDeserializer d;
    d.registerFactory(ProcessorFactory::getInstance());
    d.registerFactory(TransFuncFactory::getInstance());
    d.read(stream);
    try {
        std::vector<Processor*> processors;
        d.deserialize("SingleProcessors", processors);
        foreach (Processor* proc, processors) {
            getProcessorNetwork()->addProcessor(proc);
            ProcessorGraphicsItem* p = processorItemMap_[proc];
            p->loadMeta();
            p->setPos(rightClickPosition_);
        }
    }
    catch (SerializationException& e) {
        QMessageBox::critical(this, tr("An error occured during paste operation"), tr("No processor could be created from the text in the clipboard.\nError message: ") + QString::fromStdString(e.what()));
    }

    try {
        std::vector<AggregationMetaData> aggregationMetaData;
        d.deserialize("Aggregations", aggregationMetaData);

        foreach (const AggregationMetaData& aggregation, aggregationMetaData) {
            QList<RootGraphicsItem*> processorItems;

            foreach (Processor* processor, aggregation.getProcessors()) {
                getProcessorNetwork()->addProcessor(processor);
                ProcessorGraphicsItem* p = processorItemMap_[processor];
                processorItems.push_back(p);
            }

            AggregationGraphicsItem* aggregationItem = aggregateItems(processorItems);
            aggregationItem->setPos(rightClickPosition_);
            aggregationItem->setName(QString::fromStdString(aggregation.getName()));
        }
    }
    catch (SerializationException& e) {
        QMessageBox::critical(this, tr("An error occured during paste operation"), tr("No processor could be created from the text in the clipboard.\nError message: ") + QString::fromStdString(e.what()));
    }

    evaluator_->initializeNetwork();
}

void NetworkEditor::replaceActionSlot() {

    if (!getProcessorNetwork())
        return;

    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so replacing is not allowed"));
        return;
    }

    QList<ProcessorGraphicsItem*> selected = selectedProcessorItems();

    tgtAssert(selected.size() != 0, "no ProcessorGraphicsItem was selected and this method should be appear only on single selection");
    tgtAssert(selected.size() == 1, "multiple ProcessorGraphicsItem were selected and this method should be appear only on single selection");

    //tgtAssert(clipboardProcessors_.size() == 1, "there were no or multiple items in the clipboard");

    QPointF oldPosition = selected[0]->pos();

    QClipboard* clipboard = QApplication::clipboard();
    QString clip = clipboard->text();
    std::stringstream stream(clip.toStdString());

    XmlDeserializer d;
    d.registerFactory(ProcessorFactory::getInstance());
    d.registerFactory(TransFuncFactory::getInstance());
    try {
        d.read(stream);
        Processor* proc = 0;
        d.deserialize("SingleProcessor", proc);
        getProcessorNetwork()->replaceProcessor(selected[0]->getProcessor(), proc);
        ProcessorGraphicsItem* p = processorItemMap_[proc];
        connect(p, SIGNAL(createLink(RootGraphicsItem*, RootGraphicsItem*)), this, SLOT(createLink(RootGraphicsItem*, RootGraphicsItem*)));
        p->loadMeta();
        p->setPos(rightClickPosition_);
    }
    catch (SerializationException& e) {
        QMessageBox::critical(this, tr("An error occured during replacement operation"), tr("No processor could be created from the text in the clipboard.\nError message: ") + QString::fromStdString(e.what()));
    }
}

void NetworkEditor::deleteActionSlot() {
    removeItems(scene()->selectedItems());
    updateSelectedItems();

    emit processorsSelected(QList<Processor*>());
}

void NetworkEditor::renameActionSlot() {
    QList<QGraphicsItem*> selected = scene()->selectedItems();
    if (selected.size() > 0) {
        RootGraphicsItem* rootItem = qgraphicsitem_cast<RootGraphicsItem*>(selected[0]);
        tgtAssert(rootItem, "no root item present and rename action triggered");
        rootItem->enterRenameMode();
    }
}

void NetworkEditor::aggregateActionSlot() {
    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so aggregating is not allowed"));
        return;
    }

    QList<RootGraphicsItem*> items = convertQList<QGraphicsItem*, RootGraphicsItem*>(scene()->selectedItems());

    aggregateItems(items);
}

void NetworkEditor::deaggregateActionSlot() {
    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so deaggregating is not allowed"));
        return;
    }

    QList<AggregationGraphicsItem*> items = convertQList<QGraphicsItem*, AggregationGraphicsItem*>(scene()->selectedItems());

    deaggregateItems(items);
}

void NetworkEditor::toggleNetworkEvaluator() {
    if (networkEvaluatorIsLockedByButton_) {
        evaluator_->unlock();
        networkEvaluatorIsLockedByButton_ = false;
        stopNetworkEvaluatorButton_->setIcon(QIcon(":/voreenve/icons/player-pause.png"));
        stopNetworkEvaluatorButton_->setToolTip(tr("Stop the automatic evaluation of the network"));
        evaluator_->process();
    }
    else {
        evaluator_->lock();
        stopNetworkEvaluatorButton_->setIcon(QIcon(":/voreenve/icons/player-start.png"));
        stopNetworkEvaluatorButton_->setToolTip(tr("Start the automatic evaluation of the network"));
        networkEvaluatorIsLockedByButton_ = true;
    }
}

void NetworkEditor::clearDependancyHistory() {
    const ArrowLinkInformation& info = linkMap_[selectedLinkArrow_];
    LinkEvaluatorBase* eval = info.first->getLinkEvaluator();
    DependancyLinkEvaluatorBase* depEva = dynamic_cast<DependancyLinkEvaluatorBase*>(eval);
    depEva->clearDependancyMap();
}

// ------------------------------------------------------------------------------------------------
// event methods
// ------------------------------------------------------------------------------------------------

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
        HasToolTip* tooltip = dynamic_cast<HasToolTip*>(item);
        if (item && tooltip && activateTooltips_) {
            //if (item->type() != PortGraphicsItem::Type) {
            lastItemWithTooltip_ = tooltip;
            ttimer_->resetIfDistant(event->pos(),500);
            //}
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

void NetworkEditor::keyPressEvent(QKeyEvent* event) {
    QGraphicsView::keyPressEvent(event);

    if (!event->isAccepted() && (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)) {
        deleteActionSlot();
    }
    else if (!event->isAccepted() && (event->key() == Qt::Key_Plus)) {
        QPointF p = mapToScene(QCursor::pos());
        translate(-p.x(), -p.y());
        float factor = keyPressScaleFactor;
        scale(factor, factor);
        translate(p.x(), p.y());
    }
    else if (!event->isAccepted() && (event->key() == Qt::Key_Minus)) {
        QPointF p = mapToScene(QCursor::pos());
        translate(-p.x(), -p.y());
        float factor = 1.f / keyPressScaleFactor;
        scale(factor, factor);
        translate(p.x(), p.y());
    }
    else if (!event->isAccepted() && (event->key() == Qt::Key_C) && (event->modifiers() & Qt::CTRL))
        copyActionSlot();
    else if (!event->isAccepted() && (event->key() == Qt::Key_V) && (event->modifiers() & Qt::CTRL)) {
        rightClickPosition_ = mapToScene(mapFromGlobal(QCursor::pos()));
        pasteActionSlot();
    }
}

void NetworkEditor::contextMenuEvent(QContextMenuEvent* event) {
    QMenu currentMenu;
    lastItemWithTooltip_ = 0;
    event->accept();
    rightClickPosition_ = mapToScene(event->pos());
    QGraphicsItem* item = itemAt(event->pos());
    if (item) {
        QList<QGraphicsItem*> selectedItems = scene()->selectedItems();

        switch (item->type()) {
        case TextGraphicsItem::Type:
        case OpenPropertyListButton::Type:
            item = item->parentItem();
        case ProcessorGraphicsItem::Type:
        case AggregationGraphicsItem::Type:
            {
                if (!item->isSelected()) {
                    scene()->clearSelection();
                    selectedItems.clear();
                    item->setSelected(true);
                    selectedItems.append(item);
                }

                foreach (QGraphicsItem* i, selectedItems) {
                    if ((i->type() != ProcessorGraphicsItem::Type) && (i->type() != AggregationGraphicsItem::Type))
                        selectedItems.removeOne(i);
                }
                currentMenu.addAction(deleteAction_);
                currentMenu.addAction(copyAction_);

                if (selectedItems.size() > 1)
                    currentMenu.addAction(aggregateAction_);
                else
                    currentMenu.addAction(renameAction_);

                foreach (QGraphicsItem* selectedItem, selectedItems) {
                    if (selectedItem->type() == AggregationGraphicsItem::Type) {
                        currentMenu.addAction(deaggregateAction_);
                        break;
                    }
                }

                if ((item->type() == ProcessorGraphicsItem::Type) && (selectedItems.size() == 1) && clipboardHasValidContent())
                    currentMenu.addAction(replaceAction_);

                QList<QAction*> widgetActions;
                if (selectedItems.size() == 1) {
                    RootGraphicsItem* root = qgraphicsitem_cast<RootGraphicsItem*>(item);
                    widgetActions = root->getProcessorWidgetContextMenuActions();
                    currentMenu.addSeparator();
                    currentMenu.addActions(widgetActions);
                }

                break;
            }
        case PortArrowGraphicsItem::Type:
            return;
        case LinkArrowGraphicsItem::Type:
            {
                selectedLinkArrow_ = qgraphicsitem_cast<LinkArrowGraphicsItem*>(item);
                currentMenu.addAction(deleteLinkAction_);
                currentMenu.addAction(editLinkAction_);

                tgtAssert(linkMap_.contains(selectedLinkArrow_), "linkMap didn't contain the selected link");
                const ArrowLinkInformation& info = linkMap_[selectedLinkArrow_];
                if (dynamic_cast<DependancyLinkEvaluatorBase*>(info.first->getLinkEvaluator())) {
                    tgtAssert(info.second == 0, "a dependancy link arrow shouldn't contain two PropertyLinks");
                    currentMenu.addAction(clearDependancyHistoryAction_);
                }
                break;
            }
        case WidgetIndicatorButton::Type:
            {
                RootGraphicsItem* root = qgraphicsitem_cast<RootGraphicsItem*>(item->parentItem());
                QList<QAction*> widgetActions = root->getProcessorWidgetContextMenuActions();
                currentMenu.addActions(widgetActions);
                break;
            }
        default:
            return;
        }

        //if (!item->isSelected()) {
        //    scene()->clearSelection();
        //    item->setSelected(true);
        //    selectedItems.append(item);
        //}

        //foreach (QGraphicsItem* selectedItem, selectedItems) {
        //    if (selectedItem->type() == AggregationGraphicsItem::Type) {
        //        currentMenu.addAction(deaggregateAction_);
        //        break;
        //    }
        //}

        //if ((item->type() == ProcessorGraphicsItem::Type) && (selectedItems.size() == 1))
        //    currentMenu.addAction(replaceAction_);

        //QList<QAction*> widgetActions;
        //if ((item->type() == ProcessorGraphicsItem::Type) || (item->type() == RootGraphicsItem::Type)) {
        //    RootGraphicsItem* root = qgraphicsitem_cast<RootGraphicsItem*>(item);
        //    widgetActions = root->getProcessorWidgetContextMenuActions();
        //    currentMenu.addSeparator();
        //    currentMenu.addActions(widgetActions);
        //}
    }
    else {
        // mouse cursor is not over a guiitem -> deselect all selected items
        scene()->clearSelection();

        if (clipboardHasValidContent())
            currentMenu.addAction(pasteAction_);
    }
    currentMenu.exec(event->globalPos());
}

void NetworkEditor::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);

    layoutLayerButtons();
    layoutOtherButtons();
    if (isVisible() && needsScale_) {
        scaleView();
        needsScale_ = false;
    }
}

// ------------------------------------------------------------------------------------------------
// drag and drop methods
// ------------------------------------------------------------------------------------------------

void NetworkEditor::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasText()) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
    }
    else
        event->ignore();
}

void NetworkEditor::dragMoveEvent(QDragMoveEvent* event) {
    QGraphicsItem* item = itemAt(event->pos());

    if (item && (item->type() == PortArrowGraphicsItem::Type)) {
        PortArrowGraphicsItem* arrow = qgraphicsitem_cast<PortArrowGraphicsItem*>(item);
        selectedPortArrow_ = arrow;
        arrow->setSelected(true);
    }
    else {
        if (selectedPortArrow_)
            selectedPortArrow_->setSelected(false);
        selectedPortArrow_ = 0;
    }
}

void NetworkEditor::dragLeaveEvent(QDragLeaveEvent*) {}

void NetworkEditor::dropEvent(QDropEvent* event) {

    if (!getProcessorNetwork())
        return;

    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so dropping processors is not allowed"));
        return;
    }

    QGraphicsItem* lowerItem = itemAt(event->pos());
    if (event->mimeData()->hasText()) {
        event->setDropAction(Qt::CopyAction);
        event->accept();

        QString idString = event->mimeData()->text();

        Processor* proc = ProcessorFactory::getInstance()->create(event->mimeData()->text().toStdString());
        tgtAssert(proc, "processor creation failed");

        std::string processorName = proc->getClassName();
        tgtAssert(!processorName.empty(), "Processor class name is empty");

        ProcessorGraphicsItem* item;
        QPointF p;
        if (lowerItem) {
            switch (lowerItem->type()) {
                case PortGraphicsItem::Type:
                case TextGraphicsItem::Type:
                case OpenPropertyListButton::Type:
                    lowerItem = lowerItem->parentItem();
                case ProcessorGraphicsItem::Type:
                    {
                        p = lowerItem->pos();
                        ProcessorGraphicsItem* i = qgraphicsitem_cast<ProcessorGraphicsItem*>(lowerItem);
                        if (i)
                            getProcessorNetwork()->replaceProcessor(i->getProcessor(), proc);
                        else
                            return;
                        break;
                    }
                case ArrowHeadSelectionGraphicsItem::Type:
                    lowerItem = lowerItem->parentItem();
                case PortArrowGraphicsItem::Type:
                    {
                        PortArrowGraphicsItem* arrow = qgraphicsitem_cast<PortArrowGraphicsItem*>(lowerItem);
                        PortGraphicsItem* srcItem = arrow->getSourceItem();
                        PortGraphicsItem* dstItem = arrow->getDestinationItem();
                        getProcessorNetwork()->addProcessorInConnection(srcItem->getPort(), dstItem->getPort(), proc);
                        p = mapToScene(event->pos());
                        selectedPortArrow_ = 0;
                        break;
                    }
                default:
                    tgtAssert(false, "should not get here");
            }
        }
        else {
            getProcessorNetwork()->addProcessor(proc, processorName);
            p = mapToScene(event->pos());
        }
        tgtAssert(processorItemMap_.contains(proc), "processorItemMap didn't contain the processor");
        item = processorItemMap_[proc];
        tgtAssert(item, "no ProcessorGraphicsItem for proc");
        item->setPos(p);
        item->saveMeta();
        adjustLinkArrowGraphicsItems();
        // make sure that the added processor is initialized
        evaluator_->initializeNetwork();
    }
    else {
        event->ignore();
    }
}

// ------------------------------------------------------------------------------------------------
// tooltip methods
// ------------------------------------------------------------------------------------------------

void NetworkEditor::showTooltip(const QPoint& pos, HasToolTip* toolTip) {
    if (!activeTooltip_ && evaluator_ && !evaluator_->isLocked() && toolTip) {
        activeTooltip_ = toolTip->tooltip();
        if (activeTooltip_) {
            activeTooltip_->setPos(mapToScene(pos));
            activeTooltip_->setZValue(50); // above the rest
            scene()->addItem(activeTooltip_);
        }
    }
}

void NetworkEditor::showTooltip() {
    if (activateTooltips_)
        showTooltip(lastMousePosition_, lastItemWithTooltip_);
}

void NetworkEditor::hideTooltip() {
    ttimer_->stop();
    delete activeTooltip_;
    activeTooltip_ = 0;
}

void NetworkEditor::enableTooltips() {
    activateTooltips_ = true;
}

void NetworkEditor::disableTooltips() {
    activateTooltips_ = false;
}

void NetworkEditor::linkCamerasAutoChanged() {

    if (!getProcessorNetwork())
        return;

    tgtAssert(getProcessorNetwork(), "No processor network");
    if (getProcessorNetwork()->getMetaDataContainer().hasMetaData("autoLinkCameras")) {
        MetaDataBase* base = getProcessorNetwork()->getMetaDataContainer().getMetaData("autoLinkCameras");
        BoolMetaData* meta = dynamic_cast<BoolMetaData*>(base);
        if (!meta) {
            LWARNING("Meta data object not of expected type 'BoolMetaData'");
            return;
        }
        meta->setValue(linkCamerasAutoButton_->isChecked());
    }
    else {
        BoolMetaData* meta = new BoolMetaData;
        meta->setValue(linkCamerasAutoButton_->isChecked());
        getProcessorNetwork()->getMetaDataContainer().addMetaData("autoLinkCameras", meta);
    }
}

// ------------------------------------------------------------------------------------------------
// modify graphic items
// ------------------------------------------------------------------------------------------------

void NetworkEditor::removeItems(QList<QGraphicsItem*> items) {
    hideTooltip();

    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("Network is being evaluated, so no delete operation is allowed"));
        return;
    }

    // make sure the evaluator does not operate on a temporarily inconsistent network
    evaluator_->lock();

    // sort items by their type to not delete a port/arrow-item
    // that has already been deleted indirectly with the guiitem
    // so at first kick out the ports:
    foreach (QGraphicsItem* item, items) {
        switch (item->type()) {
        case PortGraphicsItem::Type:
        case TextGraphicsItem::Type:
        case PropertyGraphicsItem::Type:
            items.removeOne(item);
        }
    }

    // next delete arrows
    foreach (QGraphicsItem* item, items) {
        if (item->type() == PortArrowGraphicsItem::Type) {
            PortArrowGraphicsItem* arrow = qgraphicsitem_cast<PortArrowGraphicsItem*>(item);
            removeArrowItem(arrow);
            items.removeOne(item);
        }
    }

    foreach (QGraphicsItem* item, items) {
        if (item->type() == LinkArrowGraphicsItem::Type) {
            LinkArrowGraphicsItem* arrow = qgraphicsitem_cast<LinkArrowGraphicsItem*>(item);
            items.removeOne(item);
            removeArrowItem(arrow);
        }
    }

    // finally delete processor items
    foreach (QGraphicsItem* item, items) {
        if ((item->type() == ProcessorGraphicsItem::Type) || (item->type() == AggregationGraphicsItem::Type)) {
            RootGraphicsItem* rootItem = static_cast<RootGraphicsItem*>(item);
            removeRootGraphicsItem(rootItem);
            items.removeOne(item);
        }
    }

    resetCachedContent();

    // unlock evaluator (update is triggered automatically by network invalidations)
    // only unlock, if the appropriate button is not checked
    if (!networkEvaluatorIsLockedByButton_)
        evaluator_->unlock();
}

void NetworkEditor::removeRootGraphicsItem(RootGraphicsItem* rootItem) {

    if (!getProcessorNetwork())
        return;

    foreach (LinkArrowGraphicsItem* arrow, linkMap_.keys()) {
        if ((arrow->getSourceItem()->getRootGraphicsItem() == rootItem) || (arrow->getDestinationItem()->getRootGraphicsItem() == rootItem))
            removeArrowItem(arrow);
    }

    int rootItemType = rootItem->type();

    foreach (Processor* processor, rootItem->getProcessors())
        getProcessorNetwork()->removeProcessor(processor);

    if (rootItemType == AggregationGraphicsItem::Type) {
        AggregationGraphicsItem* aggItem = static_cast<AggregationGraphicsItem*>(rootItem);
        foreach (RootGraphicsItem* childItem, aggItem->getRootGraphicsItems())
            delete childItem;

        aggregationItems_.removeOne(aggItem);

        delete rootItem;
    }
}

void NetworkEditor::removeArrowItem(LinkArrowGraphicsItem* arrow) {
    //if (!linkMap_.contains(arrow))
    //    return;

    if (!getProcessorNetwork())
        return;

    ArrowLinkInformation links = linkMap_[arrow];
    linkMap_.remove(arrow);
    getProcessorNetwork()->removePropertyLink(const_cast<PropertyLink*>(links.first));

    if (links.second)
        getProcessorNetwork()->removePropertyLink(const_cast<PropertyLink*>(links.second));


    RootGraphicsItem* sourceItem = arrow->getSourceItem()->getRootGraphicsItem();
    RootGraphicsItem* destinationItem = arrow->getDestinationItem()->getRootGraphicsItem();

    if (!getProcessorNetwork()->isPropertyLinked(arrow->getSourceItem()->getProperty()))
        sourceItem->removePropertyGraphicsItem(arrow->getSourceItem());

    if (!getProcessorNetwork()->isPropertyLinked(arrow->getDestinationItem()->getProperty()))
        destinationItem->removePropertyGraphicsItem(arrow->getDestinationItem());

    delete arrow;
}

void NetworkEditor::removeArrowItem(PortArrowGraphicsItem* arrow) {

    if (!getProcessorNetwork())
        return;

    if (arrow->getDestinationItem() != 0) {
        getProcessorNetwork()->disconnectPorts(arrow->getSourceItem()->getPort(), arrow->getDestinationItem()->getPort());
    }
}

void NetworkEditor::adjustLinkArrowGraphicsItems() {
    foreach (LinkArrowGraphicsItem* arrow, linkMap_.keys())
        arrow->setVisible(currentLayer_ == NetworkEditorLayerLinking);
    scene()->invalidate();
}

// delete propertylink
void NetworkEditor::deletePropertyLinkSlot() {
    tgtAssert(selectedLinkArrow_, "stored pointer was 0");
    removeArrowItem(selectedLinkArrow_);
    selectedLinkArrow_ = 0;
}

// edit propertylink
void NetworkEditor::editPropertyLinkSlot() {
    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so editing links is not allowed"));
        return;
    }

    PropertyLinkDialog::PropertyLinkDirection linkType;
    if (linkMap_[selectedLinkArrow_].second)
        linkType = PropertyLinkDialog::PropertyLinkDirectionBidirectional;
    else
        linkType = PropertyLinkDialog::PropertyLinkDirectionToRight;

    PropertyLinkDialog* dialog = new PropertyLinkDialog(this, selectedLinkArrow_->getSourceItem(), selectedLinkArrow_->getDestinationItem(), linkMap_[selectedLinkArrow_].first, linkType);
    connect(dialog, SIGNAL(createLink(const Property*, const Property*, LinkEvaluatorBase*)), this, SLOT(editPropertyLink(const Property*, const Property*, LinkEvaluatorBase*)));
    connect(dialog, SIGNAL(removeLink(PropertyLink*)), this, SLOT(removePropertyLink(PropertyLink*)));
    dialog->exec();
}

void NetworkEditor::editPropertyLink(const Property* sourceProp, const Property* destinationProp, LinkEvaluatorBase* linkEvaluator) {
    // delete the old propertyLink
    if (selectedLinkArrow_)
        deletePropertyLinkSlot();

    // create new the link (if existing)
    if (sourceProp && destinationProp && linkEvaluator)
        createNewLink(sourceProp, destinationProp, linkEvaluator);
}

void NetworkEditor::createLink(RootGraphicsItem* src, RootGraphicsItem* dest) {

    if (!getProcessorNetwork())
        return;

    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so creating links is not allowed"));
        return;
    }
    // first find all existing links between those processors and pass them to the dialog
    QList<PropertyLink*> links;
    foreach (PropertyLink* link, getProcessorNetwork()->getPropertyLinks()) {
        Property* srcProp = link->getSourceProperty();
        Processor* srcProc = static_cast<Processor*>(srcProp->getOwner());
        tgtAssert(srcProc, "owner was no processor");

        Property* destProp = link->getDestinationProperty();
        Processor* destProc = static_cast<Processor*>(destProp->getOwner());
        tgtAssert(destProc, "owner was no processor");

        QList<Processor*> sourceProcessors = src->getProcessors();
        QList<Processor*> destinationProcessors = dest->getProcessors();

        if ((sourceProcessors.contains(srcProc) && destinationProcessors.contains(destProc)) || (sourceProcessors.contains(destProc) && destinationProcessors.contains(srcProc)))
            links.append(link);
    }

    PropertyLinkDialog* dialog = new PropertyLinkDialog(this, src, dest, links);
    connect(dialog, SIGNAL(createLink(const Property*, const Property*, LinkEvaluatorBase*)), this, SLOT(createNewLink(const Property*, const Property*, LinkEvaluatorBase*)));
    connect(dialog, SIGNAL(removeLink(PropertyLink*)), this, SLOT(removePropertyLink(PropertyLink*)));
    dialog->exec();
}

void NetworkEditor::createNewLink(const Property* sourceProp, const Property* destinationProp, LinkEvaluatorBase* linkEvaluator) {

    if (!getProcessorNetwork())
        return;

    if (sourceProp && destinationProp && linkEvaluator) {
        if (!getProcessorNetwork()->containsLink(sourceProp, destinationProp, linkEvaluator)) {
            // the callback from the observer will go to NetworkEditor::propertyLinkAdded which will cause the arrow to be created
            PropertyLink* link = getProcessorNetwork()->createPropertyLink(const_cast<Property*>(sourceProp), const_cast<Property*>(destinationProp), linkEvaluator);

            if (!link) {
                QString text = tr("The link between \"%1\" and \"%2\" could not be created.\nMost likely the property types cannot be linked or a link already exists.\nSee Log for detailed information").arg(
                    QString::fromStdString(sourceProp->getGuiName()), QString::fromStdString(destinationProp->getGuiName()));
                QMessageBox::warning(this, tr("Error in link creation"), text);
            }
        }
    }
}

void NetworkEditor::removePropertyLink(PropertyLink* propertyLink) {

    if (!getProcessorNetwork())
        return;

    getProcessorNetwork()->removePropertyLink(propertyLink);
}

LinkArrowGraphicsItem* NetworkEditor::createLinkArrowForPropertyLink(const PropertyLink* link) {
    if (link == 0)
        return 0;

    Processor* srcProc = static_cast<Processor*>(link->getSourceProperty()->getOwner());
    tgtAssert(srcProc, "property owner was no processor");

    RootGraphicsItem* srcItem = 0;
    if (processorItemMap_.contains(srcProc))
        srcItem = processorItemMap_[srcProc];
    else {
        foreach (AggregationGraphicsItem* item, aggregationItems_) {
            if (item->contains(srcProc)) {
                srcItem = item;
                break;
            }
        }

    }
    tgtAssert(srcItem, "source item was not found");

    PropertyGraphicsItem* sourceProp = srcItem->getPropertyGraphicsItem(link->getSourceProperty());

    Processor* dstProc = static_cast<Processor*>(link->getDestinationProperty()->getOwner());
    tgtAssert(dstProc, "property owner was no processor");

    RootGraphicsItem* dstItem = 0;
    if (processorItemMap_.contains(dstProc))
        dstItem = processorItemMap_[dstProc];
    else {
        foreach (AggregationGraphicsItem* item, aggregationItems_) {
            if (item->contains(dstProc)) {
                dstItem = item;
                break;
            }
        }
    }
    tgtAssert(dstItem, "destination item was not found");

    PropertyGraphicsItem* destinationProp = dstItem->getPropertyGraphicsItem(link->getDestinationProperty());

    // if there is a LinkArrowGraphicsItem which has the source and destination item reversed, we need to add the new link to that one
    foreach (LinkArrowGraphicsItem* arrow, linkMap_.keys()) {
        if ((arrow->getSourceItem() == destinationProp) && (arrow->getDestinationItem() == sourceProp)) {
            if (!linkMap_[arrow].second) {
                linkMap_[arrow].second = link;
                arrow->showSourceArrowHead(true);
                return arrow;
            }
            else
                return 0;
        }
    }

    LinkArrowGraphicsItem* result = new LinkArrowGraphicsItem(sourceProp, destinationProp);
    std::string functionname = link->getLinkEvaluator()->name();
    result->setToolTip(QString::fromStdString(functionname));

    if (dynamic_cast<DependancyLinkEvaluatorBase*>(link->getLinkEvaluator()))
        result->setNormalColor(Qt::cyan);

    ArrowLinkInformation l = std::make_pair(link, static_cast<const PropertyLink*>(0));
    linkMap_.insert(result, l);

    scene()->addItem(result);
    if (currentLayer_ == NetworkEditorLayerLinking)
        result->show();
    else
        result->hide();

    return result;
}

void NetworkEditor::linkCameras() {

    if (!getProcessorNetwork())
        return;

    if (QMessageBox::question(this, tr("VoreenVE"), tr("Link all cameras in the current network?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
        int numLinks = getProcessorNetwork()->linkProperties<CameraProperty>(std::vector<Processor*>(), std::vector<std::string>());

        if (numLinks > 0)
            LINFO("Created " << numLinks << " camera property links");
    }
}

void NetworkEditor::linkCamerasOfProcessor(const Processor* processor) {

    if (!getProcessorNetwork())
        return;

    // get camera properties of processor to link
    std::vector<CameraProperty*> camPropsProcessor = processor->getPropertiesByType<CameraProperty>();
    if (camPropsProcessor.empty())
        return;

    // get camera properties of network processors
    std::vector<CameraProperty*> camPropsNetwork = getProcessorNetwork()->getPropertiesByType<CameraProperty>();
    // remove processor's properties from network prop vector
    for (size_t i=0; i<camPropsProcessor.size(); ++i) {
        std::vector<CameraProperty*>::iterator iter = std::find(camPropsNetwork.begin(), camPropsNetwork.end(), camPropsProcessor[i]);
        if (iter != camPropsNetwork.end())
            camPropsNetwork.erase(iter);
    }

    if (camPropsNetwork.empty())
        return;

    int numLinks = 0;
    for (size_t i=0; i<camPropsProcessor.size(); ++i) {

        // create link with first network camera property
        if (!camPropsNetwork.front()->isLinkedWith(camPropsProcessor[i])) {
            if (getProcessorNetwork()->createPropertyLink(camPropsNetwork.front(), camPropsProcessor[i]))
                numLinks++;
        }
        if (!camPropsProcessor[i]->isLinkedWith(camPropsNetwork.front()))
            if (getProcessorNetwork()->createPropertyLink(camPropsProcessor[i], camPropsNetwork.front()))
                numLinks++;

        if (camPropsNetwork.size() == 1)
            continue;

        // create link with last network camera property
        if (!camPropsNetwork.back()->isLinkedWith(camPropsProcessor[i])) {
            if (getProcessorNetwork()->createPropertyLink(camPropsNetwork.back(), camPropsProcessor[i]))
                numLinks++;
        }
        if (!camPropsProcessor[i]->isLinkedWith(camPropsNetwork.back()))
            if (getProcessorNetwork()->createPropertyLink(camPropsProcessor[i], camPropsNetwork.back()))
                numLinks++;

        // create link with all network camera properties,
        // unless they are already (directly or indirectly) linked
        for (size_t j=0; j<camPropsNetwork.size(); ++j) {
            if (!camPropsNetwork[j]->isLinkedWith(camPropsProcessor[i], true)) {
                if (getProcessorNetwork()->createPropertyLink(camPropsNetwork[j], camPropsProcessor[i]))
                    numLinks++;
            }
            if (!camPropsProcessor[i]->isLinkedWith(camPropsNetwork[j], true))
                if (getProcessorNetwork()->createPropertyLink(camPropsProcessor[i], camPropsNetwork[j]))
                    numLinks++;
        }
    }

    LINFO("Created " << numLinks << " camera property links");
}

void NetworkEditor::removePropertyLinks() {

    if (!getProcessorNetwork())
        return;

    if (QMessageBox::question(this, tr("VoreenVE"), tr("Remove all property links from the current network?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
        size_t numLinks = getProcessorNetwork()->getPropertyLinks().size();
        if (numLinks > 0) {
            getProcessorNetwork()->clearPropertyLinks();
            LINFO("Removed " << numLinks << " property links");
        }
    }
}


// ------------------------------------------------------------------------------------------------
// layer methods
// ------------------------------------------------------------------------------------------------

void NetworkEditor::setLayer(NetworkEditorLayer layer) {
    if (layer == currentLayer_)
        return;

    currentLayer_ = layer;

    foreach (ProcessorGraphicsItem* procItem, processorItemMap_.values()) {
        if (procItem)
            procItem->setLayer(layer);
    }

    foreach (AggregationGraphicsItem* aggItem, aggregationItems_)
        aggItem->setLayer(layer);

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

void NetworkEditor::networkChanged() {}

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
        delete item;
        processorItemMap_.remove(nonConst);
    }
}

void NetworkEditor::propertyLinkAdded(const PropertyLink* link) {
    createLinkArrowForPropertyLink(link);
}

void NetworkEditor::propertyLinkRemoved(const PropertyLink* link) {
    foreach (LinkArrowGraphicsItem* arrow, linkMap_.keys()) {
        ArrowLinkInformation& l = linkMap_[arrow];
        if (l.first == link) {
            if (l.second) {
                l.first = l.second;
                l.second = 0;
                arrow->showDestinationArrowHead(false);
            }
            else
                removeArrowItem(arrow);
        }
        else if (l.second == link) {
            l.second = 0;
            arrow->showDestinationArrowHead(false);
        }
    }
}

void NetworkEditor::processorRenamed(const Processor* processor, const std::string& /*prevName*/) {
    if (processorItemMap_.find(const_cast<Processor*>(processor)) != processorItemMap_.end()) {
        Processor* nonConst = const_cast<Processor*>(processor);
        ProcessorGraphicsItem* item = processorItemMap_.value(nonConst);
        item->setName(QString::fromStdString(processor->getName()));
        //item->updateName();
    }
}

void NetworkEditor::portConnectionAdded(const Port* outport, const Port* inport) {
    PortGraphicsItem* outportPortItem = getPortGraphicsItem(outport);
    PortGraphicsItem* inportPortItem = getPortGraphicsItem(inport);

    PortArrowGraphicsItem* arrow = new PortArrowGraphicsItem(outportPortItem);
    outportPortItem->addArrow(arrow);
    outportPortItem->addConnection(inportPortItem);
    inportPortItem->addConnection(outportPortItem);
    outportPortItem->getArrowList().back()->setDestinationItem(inportPortItem);
    scene()->addItem(arrow);
}

void NetworkEditor::portConnectionRemoved(const Port* outport, const Port* inport) {
    PortGraphicsItem* outportPortItem = getPortGraphicsItem(outport);
    PortGraphicsItem* inportPortItem = getPortGraphicsItem(inport);

    disconnectPorts(outportPortItem, inportPortItem);
}

// ------------------------------------------------------------------------------------------------
// aggregation methods
// ------------------------------------------------------------------------------------------------

AggregationGraphicsItem* NetworkEditor::createAggregationGraphicsItem(AggregationMetaData* metaData) {
    QList<Processor*> internalProcessors = stdVectorToQList(metaData->getProcessors());
    QList<RootGraphicsItem*> internalGraphicsItems;

    foreach (Processor* processor, internalProcessors) {
        ProcessorGraphicsItem* processorItem = getProcessorGraphicsItem(processor);
        internalGraphicsItems.append(processorItem);
    }
    foreach (AggregationMetaData* internalMetaData, metaData->getAggregations()) {
        AggregationGraphicsItem* internalAggregation = createAggregationGraphicsItem(internalMetaData);
        internalGraphicsItems.append(internalAggregation);
    }

    AggregationGraphicsItem* aggregation = aggregateItems(internalGraphicsItems);
    aggregation->setPos(metaData->getPosition().first, metaData->getPosition().second);
    std::string name = metaData->getName();
    aggregation->setName(QString::fromStdString(name));

    aggregation->nameChanged();

    return aggregation;
}

void exchangePortsInConnection(PortGraphicsItem* originalPort, PortGraphicsItem* newPort, PortGraphicsItem* counterPort) {
    originalPort->removeConnection(counterPort);
    newPort->addConnection(counterPort);
    counterPort->removeConnection(originalPort);
    counterPort->addConnection(newPort);
}

AggregationGraphicsItem* NetworkEditor::aggregateItems(const QList<RootGraphicsItem*>& items) {

    if (!getProcessorNetwork())
        return 0;

    AggregationGraphicsItem* newAggregation = new AggregationGraphicsItem(items, this);

    foreach (LinkArrowGraphicsItem* arrow, linkMap_.keys()) {
        bool containsSource = items.contains(arrow->getSourceItem()->getRootGraphicsItem());
        bool containsDestination = items.contains(arrow->getDestinationItem()->getRootGraphicsItem());

        if (containsSource && containsDestination) {
            newAggregation->addInternalLinkArrow(arrow, linkMap_[arrow]);
            linkMap_.remove(arrow);
            arrow->hide();
            scene()->removeItem(arrow);
        }
        else if (containsSource) {
            PropertyGraphicsItem* newSourceItem = newAggregation->getPropertyGraphicsItem(arrow->getSourceItem()->getProperty());
            arrow->setSourceItem(newSourceItem);
        }
        else if (containsDestination) {
            PropertyGraphicsItem* newDestinationItem = newAggregation->getPropertyGraphicsItem(arrow->getDestinationItem()->getProperty());
            arrow->setDestinationItem(newDestinationItem);
        }
    }

    foreach (RootGraphicsItem* item, items) {
        tgtAssert(item, "cannot aggregate null pointer to RootGraphicsItems");

        const QString& aggregationName = newAggregation->getName();

        //if (item->type() == ProcessorGraphicsItem::Type)
            item->addAggregationPrefix(aggregationName);

        foreach (PortGraphicsItem* itemPort, item->getPortGraphicsItems()) {
            PortGraphicsItem* aggregationPort = newAggregation->getPortGraphicsItem(itemPort->getPort());
            if (itemPort->isOutport()) {
                foreach (PortArrowGraphicsItem* arrow, itemPort->getArrowList()) {
                    bool arrowSourceItemInAggregation = newAggregation->contains(arrow->getSourceItem()->getParent());
                    bool arrowDestinationItemInAggregation = newAggregation->contains(arrow->getDestinationItem()->getParent());
                    bool correctPortItem = aggregationPort->getPort() == arrow->getSourceItem()->getPort();

                    if (!arrowDestinationItemInAggregation && arrowSourceItemInAggregation && correctPortItem) {
                        exchangePortsInConnection(itemPort, aggregationPort, arrow->getDestinationItem());
                        arrow->setSourceItem(aggregationPort);
                        aggregationPort->addArrow(arrow);
                        itemPort->removeArrow(arrow);
                    }
                    else if (arrowDestinationItemInAggregation && arrowSourceItemInAggregation) {
                        newAggregation->addInternalPortArrow(arrow);
                        arrow->hide();
                        if (arrow->scene()) // arrows will be deleted twice otherwise
                            scene()->removeItem(arrow);
                    }
                }
            } else {
                foreach (PortGraphicsItem* connectedPort, itemPort->getConnectedPorts()) {
                    foreach (PortArrowGraphicsItem* arrow, connectedPort->getArrowList()) {
                        bool arrowSourceItemInAggregation = newAggregation->contains(arrow->getSourceItem()->getParent());
                        bool arrowDestinationItemInAggregation = newAggregation->contains(arrow->getDestinationItem()->getParent());
                        bool correctPortItem = aggregationPort->getPort() == arrow->getDestinationItem()->getPort();

                        if (arrowDestinationItemInAggregation && !arrowSourceItemInAggregation && correctPortItem) {
                            exchangePortsInConnection(itemPort, aggregationPort, arrow->getSourceItem());
                            arrow->setDestinationItem(aggregationPort);
                        }
                        else if (arrowDestinationItemInAggregation && arrowSourceItemInAggregation) {
                            newAggregation->addInternalPortArrow(arrow);
                            arrow->hide();
                            if (arrow->scene()) // arrows will be deleted twice otherwise
                                scene()->removeItem(arrow);
                        }
                    }
                }
            }
        }

        switch (item->type()) {
        case ProcessorGraphicsItem::Type:
            {
                ProcessorGraphicsItem* processorItem = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);
                Processor* processor = processorItem->getProcessor();
                tgtAssert(processor, "processor was null pointer");
                processor->setProgressBar(0);
                processorItemMap_.remove(processor);
                break;
            }
        case AggregationGraphicsItem::Type:
            {
                AggregationGraphicsItem* aggregationItem = qgraphicsitem_cast<AggregationGraphicsItem*>(item);
                aggregationItems_.removeOne(aggregationItem);
                break;
            }
        default:
            tgtAssert(false, "shouldn't get here");
        }

        item->saveMeta();
        item->hide();
        scene()->removeItem(item);
    }
    newAggregation->layoutChildItems();
    newAggregation->setLayer(currentLayer_);
    adjustLinkArrowGraphicsItems();
    newAggregation->setSelected(true);
    connect(newAggregation, SIGNAL(createLink(RootGraphicsItem*, RootGraphicsItem*)), this, SLOT(createLink(RootGraphicsItem*, RootGraphicsItem*)));
    connect(newAggregation, SIGNAL(startedArrow()), this, SLOT(disableTooltips()));
    connect(newAggregation, SIGNAL(endedArrow()), this, SLOT(enableTooltips()));

    aggregationItems_.append(newAggregation);
    scene()->addItem(newAggregation);
    scene()->clearSelection();

    ///////////////////////

    std::vector<Processor*> internalProcessors;
    std::vector<AggregationGraphicsItem*> internalAggregations;
    foreach (RootGraphicsItem* i, items) {
        bool isProcessorItem = i->type() == ProcessorGraphicsItem::Type;
        bool isAggregationItem = i->type() == AggregationGraphicsItem::Type;

        if (isProcessorItem)
            internalProcessors.push_back(dynamic_cast<ProcessorGraphicsItem*>(i)->getProcessor());
        else if (isAggregationItem)
            internalAggregations.push_back(dynamic_cast<AggregationGraphicsItem*>(i));
    }

    MetaDataContainer& container = getProcessorNetwork()->getMetaDataContainer();
    if (!container.hasMetaData("Aggregation")) {
        MetaDataBase* metaData = new AggregationMetaDataContainer;
        container.addMetaData("Aggregation", metaData);
    }
    AggregationMetaDataContainer* aggregationContainer = static_cast<AggregationMetaDataContainer*>(container.getMetaData("Aggregation"));

    std::vector<AggregationMetaData*> allAggregations = aggregationContainer->getAggregations();

    AggregationMetaData* metaData = new AggregationMetaData;
    metaData->setProcessors(internalProcessors);
    metaData->setName(newAggregation->getName().toStdString());
    metaData->setPosition(static_cast<int>(newAggregation->pos().x()), static_cast<int>(newAggregation->pos().y()));
    newAggregation->setAggregationMetaData(metaData);

    foreach (AggregationMetaData* metaDataAgg, allAggregations) {
        foreach (AggregationGraphicsItem* graphicsItemAgg, internalAggregations) {
            if (isEqual(metaDataAgg, graphicsItemAgg)) {
                metaData->addAggregation(metaDataAgg);
                aggregationContainer->removeAggregation(metaDataAgg);
            }
        }
    }

    aggregationContainer->addAggregation(metaData);

    return newAggregation;
}

void NetworkEditor::deaggregateItems(const QList<AggregationGraphicsItem*>& aggregationItems) {

    if (!getProcessorNetwork())
        return;

    foreach (AggregationGraphicsItem* aggregationItem, aggregationItems) {
        MetaDataContainer& container = getProcessorNetwork()->getMetaDataContainer();
        tgtAssert(container.hasMetaData("Aggregation"), "MetaDataContainer didn't have \"Aggregation\"");
        AggregationMetaDataContainer* aggregationContainer = static_cast<AggregationMetaDataContainer*>(container.getMetaData("Aggregation"));

        std::vector<AggregationMetaData*> allAggregations = aggregationContainer->getAggregations();

        foreach (AggregationMetaData* aggregationMetaData, allAggregations) {
            if (isEqual(aggregationMetaData, aggregationItem)) {
                foreach (AggregationMetaData* innerMetaData, aggregationMetaData->getAggregations())
                    aggregationContainer->addAggregation(innerMetaData);
                aggregationContainer->removeAggregation(aggregationMetaData);
                break;
            }
        }

        if (aggregationContainer->isEmpty())
            container.removeMetaData("Aggregation");

        //////////////////////////////////////////////

        aggregationItems_.removeOne(aggregationItem);
        QList<RootGraphicsItem*> childItems = aggregationItem->getRootGraphicsItems();

        foreach (RootGraphicsItem* childItem, childItems) {
            //if (childItem->type() == ProcessorGraphicsItem::Type)
                childItem->removeAggregationPrefix();

            scene()->addItem(childItem);
            childItem->show();
            childItem->loadMeta();
            childItem->setLayer(currentLayer_);

            switch (childItem->type()) {
            case ProcessorGraphicsItem::Type:
                {
                    ProcessorGraphicsItem* procItem = qgraphicsitem_cast<ProcessorGraphicsItem*>(childItem);
                    Processor* processor = procItem->getProcessor();
                    tgtAssert(processor, "cannot add null pointer to processor");
                    processor->setProgressBar(procItem->getProgressBar());
                    processorItemMap_.insert(processor, procItem);
                    break;
                }
            case AggregationGraphicsItem::Type:
                {
                     AggregationGraphicsItem* aggItem = qgraphicsitem_cast<AggregationGraphicsItem*>(childItem);
                     aggregationItems_.append(aggItem);
                     break;
                }
            default:
                tgtAssert(false, "shouldn't get here");
            }
        }

        foreach (PortArrowGraphicsItem* arrow, aggregationItem->getInternalPortArrows()) {
            arrow->show();
            scene()->addItem(arrow);
        }

        QPair<LinkArrowGraphicsItem*, ArrowLinkInformation> arrowPair;
        foreach (arrowPair, aggregationItem->getInternalLinkArrows()) {
            if (currentLayer_ == NetworkEditorLayerLinking)
                arrowPair.first->show();

            linkMap_.insert(arrowPair.first, arrowPair.second);
            scene()->addItem(arrowPair.first);
        }

        foreach (RootGraphicsItem* childItem, childItems) {
            if (currentLayer_ == NetworkEditorLayerLinking)
                childItem->togglePropertyList();
        }

        foreach (PortGraphicsItem* aggregationPortItem, aggregationItem->getPortGraphicsItems()) {
            Port* p = aggregationPortItem->getPort();
            PortGraphicsItem* otherPortItem = 0;

            foreach (RootGraphicsItem* rootItem, processorItemMap_.values()) {
                otherPortItem = rootItem->getPortGraphicsItem(p);

                if (otherPortItem)
                    break;
            }

            if (otherPortItem == 0) {
                foreach (RootGraphicsItem* rootItem, aggregationItems_) {
                    otherPortItem = rootItem->getPortGraphicsItem(p);

                    if (otherPortItem)
                        break;
                }
            }

            tgtAssert(otherPortItem, "didn't find portgraphicsitem for port p");

            if (aggregationPortItem->isOutport()) {
                foreach (PortArrowGraphicsItem* arrow, aggregationPortItem->getArrowList()) {
                    if (aggregationItem->getInternalPortArrows().contains(arrow))
                        continue;

                    exchangePortsInConnection(aggregationPortItem, otherPortItem, arrow->getDestinationItem());
                    arrow->setSourceItem(otherPortItem);
                    otherPortItem->addArrow(arrow);
                    aggregationPortItem->removeArrow(arrow);
                }

            }
            else {
                foreach (PortGraphicsItem* connectedPortItem, aggregationPortItem->getConnectedPorts()) {
                    foreach (PortArrowGraphicsItem* arrow, connectedPortItem->getArrowList()) {
                        if (aggregationItem->getInternalPortArrows().contains(arrow))
                            continue;

                        if (arrow->getDestinationItem() == aggregationPortItem) {
                            exchangePortsInConnection(aggregationPortItem, otherPortItem, connectedPortItem);
                            arrow->setDestinationItem(otherPortItem);
                        }
                    }
                }
            }
        }

        foreach (LinkArrowGraphicsItem* arrow, linkMap_.keys()) {
            if (arrow->getSourceItem()->getRootGraphicsItem() == aggregationItem) {
                foreach (RootGraphicsItem* childItem, childItems) {
                    if (childItem->hasProperty(arrow->getSourceItem()->getProperty())) {
                        PropertyGraphicsItem* newSourceItem = childItem->getPropertyGraphicsItem(arrow->getSourceItem()->getProperty());
                        arrow->setSourceItem(newSourceItem);
                    }
                }
            }
            else if (arrow->getDestinationItem()->getRootGraphicsItem() == aggregationItem) {
                foreach (RootGraphicsItem* childItem, childItems) {
                    if (childItem->hasProperty(arrow->getDestinationItem()->getProperty())) {
                        PropertyGraphicsItem* newDestinationItem = childItem->getPropertyGraphicsItem(arrow->getDestinationItem()->getProperty());
                        arrow->setDestinationItem(newDestinationItem);
                    }
                }
            }
        }

        foreach (RootGraphicsItem* childItem, childItems) {
            childItem->layoutChildItems();
        }

        scene()->removeItem(aggregationItem);
        delete aggregationItem;
        aggregationItem = 0;
    }
}

PortGraphicsItem* NetworkEditor::getPortGraphicsItem(const Port* port) const {

    tgtAssert(port, "null pointer passed");

    PortGraphicsItem* portItem = 0;
    if (processorItemMap_.contains(port->getProcessor())) {
        ProcessorGraphicsItem* outportProcItem = processorItemMap_[port->getProcessor()];
        portItem = outportProcItem->getPortGraphicsItem(port);
    }
    else {
        foreach (AggregationGraphicsItem* i, aggregationItems_) {
            if (i->contains(port->getProcessor()))
                portItem = i->getPortGraphicsItem(port);
        }
    }

    tgtAssert(portItem, "no portgraphicsitem found");
    return portItem;
}

// ------------------------------------------------------------------------------------------------

NetworkSnapshotPlugin::NetworkSnapshotPlugin(QWidget* parent, NetworkEditor* networkEditorWidget)
    : SnapshotPlugin(parent, 0)
    , networkEditorWidget_(networkEditorWidget)
{
    setWindowTitle(tr("Network Snapshot"));
    resolutions_.push_back("native");
    sizeCombo_->addItem("native");
}

void NetworkSnapshotPlugin::saveSnapshot(const QString& filename) {
    QGraphicsScene* scene = networkEditorWidget_->scene();
    QRectF sceneRect = scene->sceneRect();

    saveSnapshot(filename, sceneRect.width(), sceneRect.height());
}

void NetworkSnapshotPlugin::sizeComboChanged(int index) {
    spWidth_->blockSignals(true);
    spHeight_->blockSignals(true);
    SnapshotPlugin::sizeComboChanged(index);
    if(sizeCombo_->currentText().contains("native")) {
        spWidth_->setValue(networkEditorWidget_->sceneRect().width());
        spHeight_->setValue(networkEditorWidget_->sceneRect().height());

        spWidth_->setEnabled(false);
        spHeight_->setEnabled(false);
    }

    spWidth_->blockSignals(false);
    spHeight_->blockSignals(false);

}

void NetworkSnapshotPlugin::saveSnapshot(const QString& filename, int width, int height) {
    if (!networkEditorWidget_)
        return;


    QPixmap pixmap(width, height);
    pixmap.fill(QColor(255, 255, 255, 0));

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    networkEditorWidget_->scene()->render(&painter);

    pixmap.save(filename);
}

} // namespace voreen
