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

//super class
#include "voreen/qt/networkeditor/networkeditor.h"
#include "voreen/qt/networkeditor/editor_settings.h"
//general includes
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/network/processornetwork.h"
#include "voreen/core/voreenapplication.h"
#include "voreen/qt/voreenapplicationqt.h"

//meta informations
#include "voreen/core/datastructures/meta/selectionmetadata.h"
#include "voreen/core/datastructures/meta/aggregationmetadata.h"

//processors
#include "voreen/core/processors/processor.h"
//properties
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/link/linkevaluatorid.h"
#include "voreen/core/properties/callmemberaction.h"

//dialogs
#include "voreen/qt/networkeditor/dialogs/propertylinkdialog.h"

//styles
#include "voreen/qt/networkeditor/styles/nwestyle_base.h"
#include "voreen/qt/networkeditor/styles/nwestyle_classic.h"
#include "voreen/qt/networkeditor/styles/nwestyle_classic_print.h"

//graph layouts
#include "voreen/qt/networkeditor/graphlayouts/nwegl_base.h"
#include "voreen/qt/networkeditor/graphlayouts/nwegl_sugiyama.h"

//graphic items
    //core
#include "voreen/qt/networkeditor/graphicitems/core/processorgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/core/aggregationgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/core/portgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/core/portownergraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/core/propertygraphicsitem.h"
    //connections
#include "voreen/qt/networkeditor/graphicitems/connections/portarrowgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/connections/propertylinkarrowgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/connections/portownerlinkarrowgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/connections/portsizelinkarrowgraphicsitem.h"
    // utils
#include "voreen/qt/networkeditor/graphicitems/utils/widgettogglebuttongraphicsitem.h"

#include "voreen/qt/networkeditor/graphicitems/utils/progressbargraphicsitem.h"

// editor buttons
#include <QToolButton>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QMessageBox>
#include <QTextStream>
#include <QComboBox>
#include <QSpinBox>
//events
#include <QWheelEvent>
#include <QScrollBar>

#include <QGraphicsTextItem>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QList>
#include <iostream>

namespace voreen {

const std::string NetworkEditor::loggerCat_("voreenve.NetworkEditor");

NetworkEditor::NetworkEditor(QWidget* parent, ProcessorNetwork* network, NetworkEvaluator* evaluator)
    : QGraphicsView(parent), ProcessorNetworkObserver()
    //general members
    , processorNetwork_(network)
    , evaluator_(evaluator)
    //style, layer and cursor
    , currentLayer_(NetworkEditorLayerUndefined)
    , currentCursorMode_(NetworkEditorCursorSelectMode)
    , currentStyle_(0)
    , currentGraphLayout_(0)
    , currentToolTipMode_(true)
    //editor buttons
    , layerButtonContainer_(0), dataFlowLayerButton_(0), linkingLayerButton_(0)
    , linkingLayerButtonContainer_(0), generalLinkingButton_(0), cameraLinkingButton_(0), portSizeLinkingButton_(0)
    , generalLinkingLayerButtonContainer_(0), hideCameraLinksButton_(0), hidePortSizeLinksButton_(0), removeAllPropertyLinksButton_(0)
    , cameraLinkingLayerButtonContainer_(0), linkCamerasAutoButton_(0), linkCamerasButton_(0), removeAllCameraLinksButton_(0)
    , portSizeLinkingLayerButtonContainer_(0), linkPortSizeAutoButton_(0), linkPortSizeButton_(0), removeAllPortSizeLinksButton_(0)
    , stopButtonContainer_(0), stopNetworkEvaluatorButton_(0), networkEvaluatorIsLockedByButton_(false)
    , navigationButtonContainer_(0), selectCursorButton_(0), moveCursorButton_(0)
    , layoutButtonContainer_(0), centerViewButton_(0), graphLayoutButton_(0)
    //scale
    , needsScale_(false)
    //navigation
    , translateScene_(false)
    , translateSceneVector_(0.0,0.0)
    , lastTranslateCenter_(0.0,0.0)
    //drag&drop
    , selectedPortArrow_(0)
{
    tgtAssert(evaluator_ != 0, "passed null pointer");
    tgtAssert(processorNetwork_ != 0, "no network available");

    setScene(new QGraphicsScene(this));
    scene()->setSceneRect(sceneRectSpacing.x(), sceneRectSpacing.y(), sceneRectSpacing.width(), sceneRectSpacing.height());
    scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
    //connect(scene(), SIGNAL(selectionChanged()), this, SLOT(selectionChangeSlot()));

    //get current Style and link properties
    if(VoreenApplicationQt* appQt = dynamic_cast<VoreenApplicationQt*>(VoreenApplicationQt::app())){
        //switch style depending on appQt prop
        if(OptionProperty<NetworkEditorStyles>* prop = dynamic_cast<OptionProperty<NetworkEditorStyles>*>(appQt->getProperty("networkEditorStyleProperty"))){
            switch(prop->getValue()){
            case NWESTYLE_CLASSIC:
                currentStyle_ = new NWEStyle_Classic();
                break;
            case NWESTYLE_CLASSIC_PRINT:
                currentStyle_ = new NWEStyle_Classic_Print();
                break;
            default:
                tgtAssert(false,"Unknown NetworkEditorStyle!!! Style not changed.");
                LERROR("Unknown NetworkEditorStyle!!! Style not changed.");
                break;
            }
            //register nwe to style->onchange
            ON_PROPERTY_CHANGE((*prop),NetworkEditor,styleOnChange);
        }
        //register nwe to fontsize->onchange
        if(IntProperty* prop = dynamic_cast<IntProperty*>(appQt->getProperty("scaleProcessorFontSize"))){
            ON_PROPERTY_CHANGE((*prop),NetworkEditor,processorFontOnChange);
        }
        //
        if(OptionProperty<NetworkEditorGraphLayouts>* prop = dynamic_cast<OptionProperty<NetworkEditorGraphLayouts>*>(appQt->getProperty("networkEditorGraphLayoutsProperty"))){
            switch(prop->getValue()){
            case NWEGL_SUGIYAMA: {
                currentGraphLayout_ = new NWEGL_Sugiyama();
                qreal shift = 300.f; bool overlap = false, median = true, portflush = true;
                if(FloatProperty* shiftProp = dynamic_cast<FloatProperty*>(appQt->getProperty("sugiShiftXProperty"))) {
                    shift = shiftProp->get();
                    ON_PROPERTY_CHANGE((*shiftProp),NetworkEditor,updateGraphLayout);
                }
                if(BoolProperty* overlapProp = dynamic_cast<BoolProperty*>(appQt->getProperty("sugiOverlapProperty"))) {
                    overlap = overlapProp->get();
                    ON_PROPERTY_CHANGE((*overlapProp),NetworkEditor,updateGraphLayout);
                }
                if(BoolProperty* medianProp = dynamic_cast<BoolProperty*>(appQt->getProperty("sugiMedianProperty"))) {
                    median = medianProp->get();
                    ON_PROPERTY_CHANGE((*medianProp),NetworkEditor,updateGraphLayout);
                }
                if(BoolProperty* portflushProp = dynamic_cast<BoolProperty*>(appQt->getProperty("sugiPortFlushProperty"))) {
                    portflush = portflushProp->get();
                    ON_PROPERTY_CHANGE((*portflushProp),NetworkEditor,updateGraphLayout);
                }
                static_cast<NWEGL_Sugiyama*>(currentGraphLayout_)->setSortParameter(shift,overlap,median,portflush);
            } break;
            default:
                tgtAssert(false,"Unknown NetworkEditorGraphLayout!!!");
                LERROR("Unknown NetworkEditorGraphLayout!!!");
                break;
            }
        }
    }

    //paint settings
    setBackgroundBrush(currentStyle_->NWEStyle_NWEBackgroundBrush);
    setCacheMode(CacheBackground);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setMouseTracking(true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setMinimumSize(QSize(200, 200));
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //initilize buttons
    initilizeEditorButtons();

    //context menu
    createContextMenuActions();
}

NetworkEditor::~NetworkEditor() {
    delete scene();
    setScene(0);
    delete currentStyle_;
    delete currentGraphLayout_;
}
//---------------------------------------------------------------------------------------------------------------
//                  general members and functions
//---------------------------------------------------------------------------------------------------------------
ProcessorNetwork* NetworkEditor::getProcessorNetwork() const {
    return processorNetwork_;
}

void NetworkEditor::setProcessorNetwork(ProcessorNetwork* network) {
    //hideTooltip();

    // a locked evaluator leads to a crash if a new network is loaded
    evaluator_->unlock();
    networkEvaluatorIsLockedByButton_ = false;
    stopNetworkEvaluatorButton_->setIcon(QIcon(":/qt/icons/player-pause.png"));
    stopNetworkEvaluatorButton_->setToolTip(tr("Stop automatic network evaluation"));
    stopNetworkEvaluatorButton_->setChecked(false);

    if (processorNetwork_)
        processorNetwork_->removeObserver(this);

    resetScene();
    //linkMap_.clear();

    processorNetwork_ = network;

    if (processorNetwork_)
        processorNetwork_->addObserver(this);

    generateGraphicsItems();

    /*foreach (ProcessorGraphicsItem* processorItem, processorItemMap_.values())
        processorItem->removeAllAggregationPrefixes();*/

    /*foreach (AggregationGraphicsItem* aggregationItem, aggregationItems_)
        aggregationItem->removeAllAggregationPrefixes();*/

    if (getProcessorNetwork()) {
        AggregationMetaDataContainer* aggregationMetaDataContainer = dynamic_cast<AggregationMetaDataContainer*>(getProcessorNetwork()->getMetaDataContainer().getMetaData("Aggregation"));
        if (aggregationMetaDataContainer) {
            std::vector<AggregationMetaData*> aggregations = aggregationMetaDataContainer->getAggregations();
            aggregationMetaDataContainer->clearAggregations();

            foreach (AggregationMetaData* metaData, aggregations)
                createAggregationGraphicsItem(metaData);
        }
        //readBundlesFromMetaData();
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

    dataFlowLayerButton_->setChecked(true);
    setLayer(NetworkEditorLayerDataFlow);

    // set state of cam auto-linking button according to stored meta data, if present
    BoolMetaData* autoMeta = 0;
    if (processorNetwork_)
        autoMeta = dynamic_cast<BoolMetaData*>(processorNetwork_->getMetaDataContainer().getMetaData("autoLinkCameras"));
    if (autoMeta)
        linkCamerasAutoButton_->setChecked(autoMeta->getValue());
    else
        linkCamerasAutoButton_->setChecked(true);
    autoMeta = 0;
    if (processorNetwork_)
        autoMeta = dynamic_cast<BoolMetaData*>(processorNetwork_->getMetaDataContainer().getMetaData("autoLinkPortSize"));
    if (autoMeta)
        linkPortSizeAutoButton_->setChecked(autoMeta->getValue());
    else
        linkPortSizeAutoButton_->setChecked(true);

    // camera auto linking
    if (processorNetwork_ && linkCamerasAutoButton_->isChecked()) {
        int numCreated = processorNetwork_->createPropertyLinksWithinSubNetwork<CameraProperty>(
            processorNetwork_->getProcessors(), std::vector<std::string>(), new LinkEvaluatorCameraId());
        if (numCreated)
            LINFO("Created " << numCreated << " camera property links.");
    }

    // render size auto linking
    if (processorNetwork_ && linkPortSizeAutoButton_->isChecked()) {
        int numCreated = processorNetwork_->createRenderSizeLinksWithinSubNetwork(processorNetwork_->getProcessors(), false);
        if (numCreated)
            LINFO("Created " << numCreated << " render size links.");
    }
}

void NetworkEditor::createContextMenuActions() {
    copyAction_ = new QAction(QIcon(":/qt/icons/edit-copy.png"), tr("Copy Items"), this);
    pasteAction_ = new QAction(QIcon(":/qt/icons/edit-paste.png"), tr("Paste Items"), this);
    deleteAction_ = new QAction(QIcon(":/qt/icons/eraser.png"), tr("Delete Items"), this);
    editLinkAction_ = new QAction(tr("Edit Link"), this);
    deleteAllLinksAction_ = new QAction(QIcon(":/qt/icons/eraser.png"), tr("Delete All Property Links"), this);
    deleteAllCameraLinksAction_ = new QAction(QIcon(":/qt/icons/eraser.png"), tr("Delete All Camera Links"), this);
    deleteAllPortSizeLinksAction_ = new QAction(QIcon(":/qt/icons/eraser.png"), tr("Delete All Render Size Links"), this);
    createAllCameraLinksAction_ = new QAction(tr("Create All Camera Links"), this);
    createAllPortSizeLinksAction_ = new QAction(tr("Create All Render Size Links"), this);
    deleteInnerLinksAction_ = new QAction(QIcon(":/qt/icons/eraser.png"), tr("Delete Inner Property Links"), this);
    deleteInnerCameraLinksAction_ = new QAction(QIcon(":/qt/icons/eraser.png"), tr("Delete Inner Camera Links"), this);
    deleteInnerPortSizeLinksAction_ = new QAction(QIcon(":/qt/icons/eraser.png"), tr("Delete Inner Render Size Links"), this);
    createInnerCameraLinksAction_ = new QAction(tr("Create Inner Camera Links"), this);
    createInnerPortSizeLinksAction_ = new QAction(tr("Create Inner Render Size Links"), this);
    deletePortOwnerLinksAction_ = new QAction(QIcon(":/qt/icons/eraser.png"), tr("Delete Processor Property Links"), this);
    deletePortOwnerCameraLinksAction_ = new QAction(QIcon(":/qt/icons/eraser.png"), tr("Delete Processor Camera Links"), this);
    deletePortOwnerPortSizeLinksAction_ = new QAction(QIcon(":/qt/icons/eraser.png"), tr("Delete Processor Render Size Links"), this);
    createPortOwnerCameraLinksAction_ = new QAction(tr("Create Processor Camera Links"), this);
    createPortOwnerPortSizeLinksAction_ = new QAction(tr("Create Processor Render Size Links"), this);
    aggregateAction_ = new QAction(QIcon(":/qt/icons/aggregate.png"), tr("Aggregate"), this);
    deaggregateAction_ = new QAction(QIcon(":/qt/icons/deaggregate.png"), tr("Deaggregate"), this);  // this action will be added to the menus on demand
    sortSubNetworkAction_ = new QAction(QIcon(":/qt/icons/sortGraph.png"), tr("Sort Selected Processors"), this);
    //bundleAction_ = new QAction(QIcon(":/voreenve/icons/aggregate.png"), tr("Bundle links"), this);
    //unbundleAction_ = new QAction(QIcon(":/voreenve/icons/deaggregate.png"), tr("Unbundle links"), this);
    //addHandleAction_ = new QAction(QIcon(":/qt/icons/edit_add.png"), tr("Add handle"), this);

    connect(copyAction_, SIGNAL(triggered()), this, SLOT(copyActionSlot()));
    connect(pasteAction_, SIGNAL(triggered()), this, SLOT(pasteActionSlot()));
    connect(deleteAction_, SIGNAL(triggered()), this, SLOT(deleteActionSlot()));
    connect(editLinkAction_, SIGNAL(triggered()), this, SLOT(editLinkActionSlot()));
    connect(deleteAllLinksAction_, SIGNAL(triggered()), this, SLOT(deleteLinksActionSlot()));
    connect(deleteAllCameraLinksAction_, SIGNAL(triggered()), this, SLOT(deleteCameraLinksActionSlot()));
    connect(deleteAllPortSizeLinksAction_, SIGNAL(triggered()), this, SLOT(deletePortSizeLinksActionSlot()));
    connect(createAllCameraLinksAction_, SIGNAL(triggered()), this, SLOT(createCameraLinksActionSlot()));
    connect(createAllPortSizeLinksAction_, SIGNAL(triggered()), this, SLOT(createPortSizeLinksActionSlot()));
    connect(deleteInnerLinksAction_, SIGNAL(triggered()), this, SLOT(deleteLinksActionSlot()));
    connect(deleteInnerCameraLinksAction_, SIGNAL(triggered()), this, SLOT(deleteCameraLinksActionSlot()));
    connect(deleteInnerPortSizeLinksAction_, SIGNAL(triggered()), this, SLOT(deletePortSizeLinksActionSlot()));
    connect(createInnerCameraLinksAction_, SIGNAL(triggered()), this, SLOT(createCameraLinksActionSlot()));
    connect(createInnerPortSizeLinksAction_, SIGNAL(triggered()), this, SLOT(createPortSizeLinksActionSlot()));
    connect(deletePortOwnerLinksAction_, SIGNAL(triggered()), this, SLOT(deleteLinksActionSlot()));
    connect(deletePortOwnerCameraLinksAction_, SIGNAL(triggered()), this, SLOT(deleteCameraLinksActionSlot()));
    connect(deletePortOwnerPortSizeLinksAction_, SIGNAL(triggered()), this, SLOT(deletePortSizeLinksActionSlot()));
    connect(createPortOwnerCameraLinksAction_, SIGNAL(triggered()), this, SLOT(createCameraLinksActionSlot()));
    connect(createPortOwnerPortSizeLinksAction_, SIGNAL(triggered()), this, SLOT(createPortSizeLinksActionSlot()));
    connect(aggregateAction_, SIGNAL(triggered()), this, SLOT(aggregateActionSlot()));
    connect(deaggregateAction_, SIGNAL(triggered()), this, SLOT(deaggregateActionSlot()));
    connect(sortSubNetworkAction_, SIGNAL(triggered()), this, SLOT(sortSubNetwork()));
    //connect(bundleAction_, SIGNAL(triggered()), this, SLOT(bundleLinksSlot()));
    //connect(unbundleAction_, SIGNAL(triggered()), this, SLOT(unbundleLinksSlot()));
    //connect(addHandleAction_, SIGNAL(triggered()), this, SLOT(addHandleSlot()));

    copyAction_->setShortcut(Qt::CTRL + Qt::Key_C);
    pasteAction_->setShortcut(Qt::CTRL + Qt::Key_V);
    //replaceAction_
    deleteAction_->setShortcut(Qt::Key_Delete);
    //editLinkAction_
    //aggregateAction_
    //deaggregateAction_
    //bundleAction_
    //unbundleAction_
    //addHandleAction_

}

//---------------------------------------------------------------------------------------------------------------
//                  ProcessorNetworkObserver functions
//---------------------------------------------------------------------------------------------------------------
void NetworkEditor::networkChanged() {}

void NetworkEditor::processorAdded(const Processor* processor) {
    if (processorItemMap_.find(const_cast<Processor*>(processor)) == processorItemMap_.end()) {
        ProcessorGraphicsItem* item = createProcessorGraphicsItem(const_cast<Processor*>(processor));
        if (linkCamerasAutoButton_->isChecked())
            linkCamerasOfProcessor(processor);
    }
}

void NetworkEditor::processorRemoved(const Processor* processor) {
    if (processorItemMap_.find(const_cast<Processor*>(processor)) != processorItemMap_.end()) {
        Processor* nonConst = const_cast<Processor*>(processor);
        ProcessorGraphicsItem* item = processorItemMap_.value(nonConst);
        setUpdatesEnabled(false);
        scene()->removeItem(item);
        delete item;
        setUpdatesEnabled(true);
        processorItemMap_.remove(nonConst);
    }
}

void NetworkEditor::propertyLinkAdded(const PropertyLink* link) {
    createLinkArrowForPropertyLink(link);
}

void NetworkEditor::propertyLinkRemoved(const PropertyLink* link) {
    PropertyGraphicsItem* srcProp = 0, *dstProp = 0;
    PortOwnerGraphicsItem* srcPOItem = 0, *dstPOItem = 0;

    //find src items
    foreach(ProcessorGraphicsItem* item, processorItemMap_) {
        if(item->getPropertyList()->getPropertyItem(link->getSourceProperty())){
            srcProp = item->getPropertyList()->getPropertyItem(link->getSourceProperty());
            srcPOItem = item;
            break;
        }
        foreach(PortGraphicsItem* portItem, item->getPortGraphicsItems()){
            if(portItem->getPropertyList()->getPropertyItem(link->getSourceProperty())){
                srcProp = portItem->getPropertyList()->getPropertyItem(link->getSourceProperty());
                srcPOItem = portItem->getPortOwner();
                break;
            }
        }
        if(srcProp)
            break;
    }

    //find dst items
    foreach(ProcessorGraphicsItem* item, processorItemMap_) {
        if(item->getPropertyList()->getPropertyItem(link->getDestinationProperty())){
            dstProp = item->getPropertyList()->getPropertyItem(link->getDestinationProperty());
            dstPOItem = item;
            break;
        }
        foreach(PortGraphicsItem* portItem, item->getPortGraphicsItems()){
            if(portItem->getPropertyList()->getPropertyItem(link->getDestinationProperty())){
                dstProp = portItem->getPropertyList()->getPropertyItem(link->getDestinationProperty());
                dstPOItem = portItem->getPortOwner();
                break;
            }
        }
        if(dstProp)
            break;
    }

    srcProp->removeGraphicalLink(dstProp);

    //remove portsizelinkarrow if exists
    if(((srcProp->getProperty()->getClassName() == "RenderSizeOriginProperty") ||
        (srcProp->getProperty()->getClassName() == "RenderSizeReceiveProperty")) &&
       ((dstProp->getProperty()->getClassName() == "RenderSizeOriginProperty") ||
        (dstProp->getProperty()->getClassName() == "RenderSizeReceiveProperty")))
        dynamic_cast<PortGraphicsItem*>(srcProp->getPropertyOwnerItem())->removePortSizeLinkArrow(srcProp, dstProp);


    //remove portownerlinkarrow if no longer needed
    if(!srcPOItem->isPortOwnerLinkNeeded(dstPOItem))
        srcPOItem->removeGraphicalLinkArrow(dstPOItem);
    else if(currentLayer_ == NetworkEditorLayerCameraLinking) {//test if it is still visible in camera mode
        if(!srcPOItem->isPortOwnerLinkNeeded(dstPOItem, currentLayer_)) {
            foreach(PortOwnerLinkArrowGraphicsItem* arrow, srcPOItem->getPortOwnerLinkArrows())
                if(arrow->getDestinationItem()->parent() == dstPOItem)
                    arrow->setVisible(false);
            foreach(PortOwnerLinkArrowGraphicsItem* arrow, dstPOItem->getPortOwnerLinkArrows())
                if(arrow->getDestinationItem()->parent() == srcPOItem)
                    arrow->setVisible(false);
        }
    }

    srcPOItem->getPropertyList()->setVisible(srcPOItem->getPropertyList()->isVisible());
    dstPOItem->getPropertyList()->setVisible(dstPOItem->getPropertyList()->isVisible());
}

void NetworkEditor::processorRenamed(const Processor* processor, const std::string& /*prevName*/) {
    if (processorItemMap_.find(const_cast<Processor*>(processor)) != processorItemMap_.end()) {
        Processor* nonConst = const_cast<Processor*>(processor);
        ProcessorGraphicsItem* item = processorItemMap_.value(nonConst);
    }
}

void NetworkEditor::portConnectionAdded(const Port* outport, const Port* inport) {
    getPortGraphicsItem(outport)->addGraphicalConnection(getPortGraphicsItem(inport));

    // render size linking
    if (linkPortSizeAutoButton_->isChecked()) {
        const RenderPort* renderOutport = dynamic_cast<const RenderPort*>(outport);
        const RenderPort* renderInport = dynamic_cast<const RenderPort*>(inport);
        if (renderOutport && renderInport) {
            int numCreated = processorNetwork_->createRenderSizeLinksOverConnection(
                const_cast<RenderPort*>(renderOutport), const_cast<RenderPort*>(renderInport), false);
            if (numCreated)
                LINFO("Created " << numCreated << " render size links.");
        }
    }
}

void NetworkEditor::portConnectionRemoved(const Port* outport, const Port* inport) {
    getPortGraphicsItem(outport)->removeGraphicalConnection(getPortGraphicsItem(inport));

    // render size linking
    if (linkPortSizeAutoButton_->isChecked()) {
        const RenderPort* renderOutport = dynamic_cast<const RenderPort*>(outport);
        const RenderPort* renderInport = dynamic_cast<const RenderPort*>(inport);
        if (renderOutport && renderInport) {
            int numRemoved = processorNetwork_->removeRenderSizeLinksOverConnection(
                const_cast<RenderPort*>(renderOutport), const_cast<RenderPort*>(renderInport));
            if (numRemoved)
                LINFO("Removed " << numRemoved << " render size links.");
        }
    }

}

//---------------------------------------------------------------------------------------------------------------
//                  scene transformations
//---------------------------------------------------------------------------------------------------------------
void NetworkEditor::scaleView() {
    if (!getProcessorNetwork())
        return;

    QRectF visibleRect;
    // scene()->itemsBoundingRect() will consider all invisible (i.e. properties) as well, so we have to to it by ourselves
    foreach (QGraphicsItem* item, scene()->items()) {
        if (item->isVisible()) {
            QRectF iRect = item->mapRectToScene(item->boundingRect());
            visibleRect = visibleRect.united(iRect);
        }
    }

    //set scale
    FloatMetaData* metaFactor = dynamic_cast<FloatMetaData*>(getProcessorNetwork()->getMetaDataContainer().getMetaData("ZoomFactor"));
    if (metaFactor) {
        QGraphicsView::scale(metaFactor->getValue(),metaFactor->getValue());
    }
    else {
        qreal scaleFactor;
        if (!visibleRect.isEmpty()) {
            QSizeF vps = mapToScene(viewport()->rect()).boundingRect().size();
            QSizeF vs = visibleRect.size();
            scaleFactor = std::min(vps.width()/vs.width(), vps.height()/vs.height());
            QGraphicsView::scale(scaleFactor-0.1,scaleFactor-0.1);
        }
    }

    //set center
    Vec2MetaData* metaCenter = dynamic_cast<Vec2MetaData*>(getProcessorNetwork()->getMetaDataContainer().getMetaData("ZoomCenter"));
    if (metaCenter) {
        centerOn(metaCenter->getValue().x, metaCenter->getValue().y);
    }
    else {
        QPointF center;
        // set center
        if (visibleRect.isEmpty()) {
            center = QPointF(0.0,0.0);
        } else {
            center = QPointF(visibleRect.x() + visibleRect.width() / 2.f, visibleRect.y() + visibleRect.height() / 2.f);
        }
        centerOn(center);
        if (getProcessorNetwork()->getMetaDataContainer().hasMetaData("ZoomCenter")) {
            MetaDataBase* base = getProcessorNetwork()->getMetaDataContainer().getMetaData("ZoomCenter");
            Vec2MetaData* meta = dynamic_cast<Vec2MetaData*>(base);
            meta->setValue(tgt::vec2(center.x(),center.y()));
        } else {
            getProcessorNetwork()->getMetaDataContainer().addMetaData("ZoomCenter", new Vec2MetaData(tgt::vec2(center.x(),center.y())));
        }
    }

    invalidateScene(QRectF(), QGraphicsScene::ForegroundLayer);
}

void NetworkEditor::scale(qreal sx, qreal sy) {
    tgtAssert(sx == sy, "no rectangular zoom performed");
    QPolygonF sv = mapToScene(viewport()->rect());
    QRectF vr;
    foreach (QGraphicsItem* item, scene()->items()) {
        if (item->isVisible()) {
            QRectF iRect = item->mapRectToScene(item->boundingRect());
            vr = vr.united(iRect);
        }
    }
    //QRectF sr = scene()->itemsBoundingRect();
    vr.setCoords(vr.left()-vr.width()/2.f,vr.top()-vr.height()/2.f,vr.right()-vr.width()/2.f,vr.bottom()-vr.height()/2.f);

    if(sx < 1.f && sv.boundingRect().size().width() > 3*vr.size().width() && sv.boundingRect().size().height() > 3*vr.size().height())
        return;

    QGraphicsView::scale(sx, sy);

    if (getProcessorNetwork()) {
        if (getProcessorNetwork()->getMetaDataContainer().hasMetaData("ZoomFactor")) {
            MetaDataBase* base = getProcessorNetwork()->getMetaDataContainer().getMetaData("ZoomFactor");
            FloatMetaData* meta = dynamic_cast<FloatMetaData*>(base);
            meta->setValue(transform().m11());
        } else {
            getProcessorNetwork()->getMetaDataContainer().addMetaData("ZoomFactor", new FloatMetaData(transform().m11()));
        }
        QPointF center = mapToScene(viewport()->rect().center());
        if (getProcessorNetwork()->getMetaDataContainer().hasMetaData("ZoomCenter")) {
            MetaDataBase* base = getProcessorNetwork()->getMetaDataContainer().getMetaData("ZoomCenter");
            Vec2MetaData* meta = dynamic_cast<Vec2MetaData*>(base);
            meta->setValue(tgt::vec2(center.x(),center.y()));
        } else {
            getProcessorNetwork()->getMetaDataContainer().addMetaData("ZoomCenter", new Vec2MetaData(tgt::vec2(center.x(),center.y())));
        }
    }
}

void NetworkEditor::translate(qreal dx, qreal dy) {
    QGraphicsView::translate(dx, dy);
}

QSize NetworkEditor::sizeHint() const {
    return QSize(400, 600);
}

//---------------------------------------------------------------------------------------------------------------
//                  create and handle graphicitems
//---------------------------------------------------------------------------------------------------------------
void NetworkEditor::resetScene() {
    emit update();
    foreach(ProcessorGraphicsItem* item, processorItemMap_) {
        delete item;
    }
    processorItemMap_.clear();
    aggregationItems_.clear();
    //hideTooltip(); // prevent double-free

    // deletion is necessary because the QGraphicsScene's sceneRect will consider all added items
    setUpdatesEnabled(false);
    scene()->blockSignals(true);
    scene()->clear();
    delete scene();
    setScene(new QGraphicsScene(this));
    scene()->setSceneRect(sceneRectSpacing.x(), sceneRectSpacing.y(), sceneRectSpacing.width(), sceneRectSpacing.height());
    scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
    //connect(scene(), SIGNAL(selectionChanged()), this, SLOT(selectionChangeSlot()));
    setUpdatesEnabled(true);
}

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
            std::vector<const Port*> connectedPorts = port->getConnected();

            foreach (const Port* connectedPort, connectedPorts)
                getPortGraphicsItem(port)->addGraphicalConnection(getPortGraphicsItem(connectedPort));
        }
    }

    foreach (PropertyLink* link, getProcessorNetwork()->getPropertyLinks()) {
        createLinkArrowForPropertyLink(link);
    }
}


ProcessorGraphicsItem* NetworkEditor::createProcessorGraphicsItem(Processor* processor) {
    ProcessorGraphicsItem* result = 0;
    {
        result = new ProcessorGraphicsItem(processor, this);
    }
    processorItemMap_[processor] = result;
    result->updateNWELayerAndCursor();
    result->loadMeta();

    connect(result, SIGNAL(openPropertyLinkDialog(PortOwnerGraphicsItem*, PortOwnerGraphicsItem*)), this, SLOT(openPropertyLinkDialog(PortOwnerGraphicsItem*, PortOwnerGraphicsItem*)));
    //connect(result, SIGNAL(startedArrow()), this, SLOT(disableTooltips()));
    //connect(result, SIGNAL(endedArrow()), this, SLOT(enableTooltips()));

    return result;
}

bool isEqual(AggregationMetaData* metaData, AggregationGraphicsItem* graphicsItem) {/*
    std::vector<Processor*> metaDataProcessors = metaData->getProcessors();
    QList<Processor*> graphicsItemProcessors = graphicsItem->getProcessors();

    foreach (Processor* metaDataProcessor, metaDataProcessors) {
        int index = graphicsItemProcessors.indexOf(metaDataProcessor);
        if (index == -1)
            return false;
    }
    */
    return true;
}

AggregationGraphicsItem* NetworkEditor::createAggregationGraphicsItem(AggregationMetaData* metaData) {/*
    QList<Processor*> internalProcessors = stdVectorToQList(metaData->getProcessors());
    QList<PortOwnerGraphicsItem*> internalGraphicsItems;

    foreach (Processor* processor, internalProcessors) {
        ProcessorGraphicsItem* processorItem = getProcessorGraphicsItem(processor);
        internalGraphicsItems.append(processorItem);
        processorItem->hide();
    }

    foreach (AggregationMetaData* internalMetaData, metaData->getAggregations()) {
        AggregationGraphicsItem* internalAggregation = createAggregationGraphicsItem(internalMetaData);
        internalGraphicsItems.append(internalAggregation);
    }

    AggregationGraphicsItem* aggregation = createAggregationGraphicsItem(internalGraphicsItems);
    aggregation->setPos(metaData->getPosition().first, metaData->getPosition().second);
    std::string name = metaData->getName();
    aggregation->setGuiName(QString::fromStdString(name));
    aggregation->getNameLabel()->setPlainText(aggregation->getGuiName());
    aggregation->getPropertyList()->updateParentLabel();
    aggregation->layoutChildItems();
    connect(aggregation, SIGNAL(openPropertyLinkDialog(PortOwnerGraphicsItem*, PortOwnerGraphicsItem*)), this, SLOT(openPropertyLinkDialog(PortOwnerGraphicsItem*, PortOwnerGraphicsItem*)));
    //connect(aggregation, SIGNAL(startedArrow()), this, SLOT(disableTooltips()));
    //connect(aggregation, SIGNAL(endedArrow()), this, SLOT(enableTooltips()));
    */
    return 0;
}

AggregationGraphicsItem* NetworkEditor::createAggregationGraphicsItem(const QList<PortOwnerGraphicsItem*>& items){
/*
    if (!getProcessorNetwork())
        return 0;

    AggregationGraphicsItem* newAggregation = new AggregationGraphicsItem(new Aggregation(),const_cast<QList<PortOwnerGraphicsItem*>*>(&items), this);
    connect(newAggregation, SIGNAL(openPropertyLinkDialog(PortOwnerGraphicsItem*, PortOwnerGraphicsItem*)), this, SLOT(openPropertyLinkDialog(PortOwnerGraphicsItem*, PortOwnerGraphicsItem*)));
    foreach(PortOwnerGraphicsItem* item, items){
        switch (item->type()) {
        case UserTypesProcessorGraphicsItem:
            {
                ProcessorGraphicsItem* processorItem = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);
                Processor* processor = processorItem->getProcessor();
                tgtAssert(processor, "processor was null pointer");
                delete processorItem->getProgressBar();
                processor->setProgressBar(0);
                break;
            }
        case UserTypesAggregationGraphicsItem:
            {
                AggregationGraphicsItem* aggregationItem = qgraphicsitem_cast<AggregationGraphicsItem*>(item);
                aggregationItems_.removeOne(aggregationItem);
                break;
            }
        default:
            tgtAssert(false, "shouldn't get here");
        }
        item->setVisible(false);
    }

    newAggregation->layoutChildItems();
    aggregationItems_.append(newAggregation);
    scene()->clearSelection();

    //set meta information
    std::vector<Processor*> internalProcessors;
    std::vector<AggregationGraphicsItem*> internalAggregations;
    foreach (PortOwnerGraphicsItem* i, items) {
        bool isProcessorItem = i->type() == UserTypesProcessorGraphicsItem;
        bool isAggregationItem = i->type() == UserTypesAggregationGraphicsItem;

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
    metaData->setName(newAggregation->getGuiName().toStdString());
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
    */
    return 0;
}

void NetworkEditor::createLinkArrowForPropertyLink(const PropertyLink* link) {
    if (link == 0)
        return;

    PortOwnerGraphicsItem* srcPOItem = 0;
    PropertyGraphicsItem* srcPropItem = 0;
    //source prop belongs to processor
    if (dynamic_cast<Processor*>(link->getSourceProperty()->getOwner())){
        Processor* srcProc = static_cast<Processor*>(link->getSourceProperty()->getOwner());

        if (processorItemMap_.contains(srcProc))
            srcPOItem = processorItemMap_[srcProc];
        else {
           foreach (AggregationGraphicsItem* item, aggregationItems_) {
                if (item->contains(dynamic_cast<PortOwnerGraphicsItem*>(srcProc))) {
                    srcPOItem = item;
                    break;
                }
            }
        }
        tgtAssert(srcPOItem, "source item was not found");

        srcPropItem = srcPOItem->getPropertyList()->getPropertyItem(link->getSourceProperty());
    }
    //source prop belongs to port
    else if (dynamic_cast<Port*>(link->getSourceProperty()->getOwner())){
        Port* srcPort = static_cast<Port*>(link->getSourceProperty()->getOwner());

        PropertyOwnerGraphicsItem* srcPortItem = 0;
        srcPortItem = getPortGraphicsItem(srcPort);
        tgtAssert(srcPortItem, "source item was not found");

        srcPOItem = dynamic_cast<PortOwnerGraphicsItem*>(srcPortItem->parent());
        srcPropItem = srcPortItem->getPropertyList()->getPropertyItem(link->getSourceProperty());

    } else {
        LWARNING("createLinkArrowForPropertyLink(): unknown link source property owner");
        return;
    }
    tgtAssert(srcPropItem, "no src prop gi");

    PortOwnerGraphicsItem* dstPOItem = 0;
    PropertyGraphicsItem* dstPropItem = 0;
    //destination prop belongs to processor
    if (dynamic_cast<Processor*>(link->getDestinationProperty()->getOwner())){
        Processor* dstProc = static_cast<Processor*>(link->getDestinationProperty()->getOwner());

        if (processorItemMap_.contains(dstProc))
            dstPOItem = processorItemMap_[dstProc];
        else {
           foreach (AggregationGraphicsItem* item, aggregationItems_) {
                if (item->contains(dynamic_cast<PortOwnerGraphicsItem*>(dstProc))) {
                    dstPOItem = item;
                    break;
                }
            }
        }
        tgtAssert(dstPOItem, "destination item was not found");

        dstPropItem = dstPOItem->getPropertyList()->getPropertyItem(link->getDestinationProperty());
    }
    //source prop belongs to port
    else if (dynamic_cast<Port*>(link->getDestinationProperty()->getOwner())){
        Port* dstPort = static_cast<Port*>(link->getDestinationProperty()->getOwner());

        PropertyOwnerGraphicsItem* dstPortItem = 0;
        dstPortItem = getPortGraphicsItem(dstPort);
        tgtAssert(dstPortItem, "destination item was not found");

        dstPOItem = dynamic_cast<PortOwnerGraphicsItem*>(dstPortItem->parent());
        dstPropItem = dstPortItem->getPropertyList()->getPropertyItem(link->getDestinationProperty());

    } else {
        LWARNING("createLinkArrowForPropertyLink(): unknown link destination property owner");
        return;
    }
    tgtAssert(dstPropItem, "no dst prop gi");

    srcPropItem->addGraphicalLink(dstPropItem);

    //create portsizelinkarrow if needed
    if(link->getLinkEvaluator()->getClassName() == "LinkEvaluatorRenderSize")
        dynamic_cast<PortGraphicsItem*>(srcPropItem->getPropertyOwnerItem())->addPortSizeLinkArrow(srcPropItem, dstPropItem);

    //create portownerlinkarrow if needed
    if(srcPOItem->isPortOwnerLinkNeeded(dstPOItem))
        srcPOItem->addGraphicalLinkArrow(dstPOItem);

    if(currentLayer_ == NetworkEditorLayerCameraLinking) {//test if it is still visible in camera mode
        if(srcPOItem->isPortOwnerLinkNeeded(dstPOItem, currentLayer_)) {
            foreach(PortOwnerLinkArrowGraphicsItem* arrow, srcPOItem->getPortOwnerLinkArrows())
                if(arrow->getDestinationItem()->parent() == dstPOItem)
                    arrow->setVisible(true);
            foreach(PortOwnerLinkArrowGraphicsItem* arrow, dstPOItem->getPortOwnerLinkArrows())
                if(arrow->getDestinationItem()->parent() == srcPOItem)
                    arrow->setVisible(true);
        }
    }
}

void NetworkEditor::removeItems(QList<QGraphicsItem*> items) {
    //hideTooltip();

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
        case UserTypesPortGraphicsItem:
        case QGraphicsTextItem::Type:
        case UserTypesRenamableTextGraphicsItem:
        case UserTypesPropertyGraphicsItem:
            items.removeOne(item);
        }
    }

    switch(currentLayer_){
    case NetworkEditorLayerDataFlow:
        removeItemsInDataFlow(items);
        break;
    case NetworkEditorLayerGeneralLinking:
        removeItemsInGeneralLinking(items);
        break;
    case NetworkEditorLayerCameraLinking:
        removeItemsInCameraLinking(items);
        break;
    case NetworkEditorLayerPortSizeLinking:
        removeItemsInPortSizeLinking(items);
        break;
    default:
        tgtAssert(false,"shouldn't get here");
        break;
    }

    resetCachedContent();
    scene()->clearSelection();

    // unlock evaluator (update is triggered automatically by network invalidations)
    // only unlock, if the appropriate button is not checked
    if (!networkEvaluatorIsLockedByButton_)
        evaluator_->unlock();
}


void NetworkEditor::removeItemsInDataFlow(QList<QGraphicsItem*> items){
    // next delete port arrows
    foreach (QGraphicsItem* item, items) {
        if (item->type() == UserTypesPortArrowGraphicsItem) {
            PortArrowGraphicsItem* arrow = qgraphicsitem_cast<PortArrowGraphicsItem*>(item);
            items.removeOne(item);
            removePortArrowGraphicsItem(arrow);
        }
    }

    // finally delete processor items
    foreach (QGraphicsItem* item, items) {
        if ((item->type() == UserTypesProcessorGraphicsItem) ) {//|| (item->type() == UserTypeAggregationGraphicsItem)) {
            PortOwnerGraphicsItem* poItem = static_cast<PortOwnerGraphicsItem*>(item);
            items.removeOne(item);
            removePortOwnerGraphicsItem(poItem);
        }
    }
}

void NetworkEditor::removeItemsInGeneralLinking(QList<QGraphicsItem*> items){
    foreach (QGraphicsItem* item, items) {
        switch (item->type()) {
        case UserTypesPortArrowGraphicsItem:
        case UserTypesPortSizeLinkArrowGraphicsItem:
            items.removeOne(item);
        default:
            break;
        }
    }

    //delete property link arrows
    QList<PropertyLinkArrowGraphicsItem*> linkList;
    foreach (QGraphicsItem* item, items) {
        if (item->type() == UserTypesPropertyLinkArrowGraphicsItem) {
            linkList.append(qgraphicsitem_cast<PropertyLinkArrowGraphicsItem*>(item));
            items.removeOne(item);
        }
    }
    while(!linkList.empty()){
        PropertyLinkArrowGraphicsItem* item = linkList.first();
        linkList.removeOne(item);
        //remove dual link if exists
        foreach (PropertyLinkArrowGraphicsItem* dual, linkList) {
            if(dual->getDestinationItem() == item->getSourceItem() && dual->getSourceItem() == item->getDestinationItem()) {
                linkList.removeOne(dual);
            } else
            if(dual->getSourceItem() == item->getSourceItem() && dual->getDestinationItem() == item->getDestinationItem()) {
                linkList.removeOne(dual);
            }
        }
        removePropertyLinkArrowGraphicsItem(item);
    }

    //delete links between processors
    QList<ProcessorGraphicsItem*> processorList;
    foreach (QGraphicsItem* item, items) {
        if (item->type() == UserTypesProcessorGraphicsItem) {
            processorList.append(qgraphicsitem_cast<ProcessorGraphicsItem*>(item));
            items.removeOne(item);
        }
    }
    if(processorList.size() == 1){
        foreach(PropertyGraphicsItem* prop, processorList.first()->getPropertyList()->getAllPropertyItems()){
            foreach(PropertyLinkArrowGraphicsItem* arrow, prop->getSourceLinkList())
                removePropertyLinkArrowGraphicsItem(arrow);
            foreach(PropertyLinkArrowGraphicsItem* arrow, prop->getDestinationLinkList())
                removePropertyLinkArrowGraphicsItem(arrow);
        }
    } else {
        foreach(ProcessorGraphicsItem* proc1, processorList){
            foreach(ProcessorGraphicsItem* proc2, processorList){
                foreach(PropertyGraphicsItem* prop1, proc1->getPropertyList()->getAllPropertyItems()){
                    foreach(PropertyLinkArrowGraphicsItem* arrow, prop1->getSourceLinkList()){
                        foreach(PropertyGraphicsItem* prop2, proc2->getPropertyList()->getAllPropertyItems()){
                            if(arrow->getDestinationItem() == prop2){
                                removePropertyLinkArrowGraphicsItem(arrow);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void NetworkEditor::removeItemsInCameraLinking(QList<QGraphicsItem*> items) {
    foreach (QGraphicsItem* item, items) {
        switch (item->type()) {
        case UserTypesPortArrowGraphicsItem:
        case UserTypesPortSizeLinkArrowGraphicsItem:
            items.removeOne(item);
        default:
            break;
        }
    }

    //delete property link arrows
    QList<PropertyLinkArrowGraphicsItem*> linkList;
    foreach (QGraphicsItem* item, items) {
        if (item->type() == UserTypesPropertyLinkArrowGraphicsItem) {
            linkList.append(qgraphicsitem_cast<PropertyLinkArrowGraphicsItem*>(item));
            items.removeOne(item);
        }
    }
    while(!linkList.empty()){
        PropertyLinkArrowGraphicsItem* item = linkList.first();
        linkList.removeOne(item);
        //remove dual link if exists
        foreach (PropertyLinkArrowGraphicsItem* dual, linkList) {
            if(dual->getSourceItem() == item->getSourceItem() && dual->getDestinationItem() == item->getDestinationItem()) {
                linkList.removeOne(dual);
            } else
            if(dual->getDestinationItem() == item->getSourceItem() && dual->getSourceItem() == item->getDestinationItem()) {
                linkList.removeOne(dual);
            }
        }
        removePropertyLinkArrowGraphicsItem(item);
    }

    //delete links between processors
    QList<ProcessorGraphicsItem*> processorList;
    foreach (QGraphicsItem* item, items) {
        if (item->type() == UserTypesProcessorGraphicsItem) {
            processorList.append(qgraphicsitem_cast<ProcessorGraphicsItem*>(item));
            items.removeOne(item);
        }
    }
    if(processorList.size() == 1){
        foreach(PropertyGraphicsItem* prop, processorList.first()->getPropertyList()->getAllPropertyItems()){
            foreach(PropertyLinkArrowGraphicsItem* arrow, prop->getSourceLinkList())
                if(arrow->getSourceItem()->getProperty()->getClassName() == "CameraProperty" &&
                   arrow->getDestinationItem()->getProperty()->getClassName() == "CameraProperty")
                    removePropertyLinkArrowGraphicsItem(arrow);
            foreach(PropertyLinkArrowGraphicsItem* arrow, prop->getDestinationLinkList())
                if(arrow->getSourceItem()->getProperty()->getClassName() == "CameraProperty" &&
                   arrow->getDestinationItem()->getProperty()->getClassName() == "CameraProperty")
                    removePropertyLinkArrowGraphicsItem(arrow);
        }
    } else {
        foreach(ProcessorGraphicsItem* proc1, processorList){
            foreach(ProcessorGraphicsItem* proc2, processorList){
                foreach(PropertyGraphicsItem* prop1, proc1->getPropertyList()->getAllPropertyItems()){
                    if(prop1->getProperty()->getClassName() == "CameraProperty"){
                        foreach(PropertyLinkArrowGraphicsItem* arrow, prop1->getSourceLinkList()){
                            foreach(PropertyGraphicsItem* prop2, proc2->getPropertyList()->getAllPropertyItems()){
                                if(prop2->getProperty()->getClassName() == "CameraProperty" &&
                                    arrow->getDestinationItem() == prop2){
                                    removePropertyLinkArrowGraphicsItem(arrow);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void NetworkEditor::removeItemsInPortSizeLinking(QList<QGraphicsItem*> items) {
    foreach (QGraphicsItem* item, items) {
        switch (item->type()) {
        case UserTypesPortArrowGraphicsItem:
            items.removeOne(item);
        default:
            break;
        }
    }

    //delete port size link arrows
    foreach (QGraphicsItem* item, items) {
        if (item->type() == UserTypesPortSizeLinkArrowGraphicsItem) {
            PortSizeLinkArrowGraphicsItem* arrow = qgraphicsitem_cast<PortSizeLinkArrowGraphicsItem*>(item);
            items.removeOne(item);
            removePortSizeLinkArrowGraphicsItem(arrow);
        }
    }

    //delete links between processors
    QList<ProcessorGraphicsItem*> processorList;
    foreach (QGraphicsItem* item, items) {
        if (item->type() == UserTypesProcessorGraphicsItem) {
            processorList.append(qgraphicsitem_cast<ProcessorGraphicsItem*>(item));
            items.removeOne(item);
        }
    }
    if(processorList.size() == 1){
        foreach(PropertyGraphicsItem* prop, processorList.first()->getPropertyList()->getAllPropertyItems()){
            foreach(PropertyLinkArrowGraphicsItem* arrow, prop->getSourceLinkList())
                if(arrow->getSourceItem()->getProperty()->getLink(arrow->getDestinationItem()->getProperty())
                   ->getLinkEvaluator()->getClassName() == "LinkEvaluatorRenderSize")
                   removePropertyLinkArrowGraphicsItem(arrow);
            foreach(PropertyLinkArrowGraphicsItem* arrow, prop->getDestinationLinkList())
                if(arrow->getSourceItem()->getProperty()->getLink(arrow->getDestinationItem()->getProperty())
                   ->getLinkEvaluator()->getClassName() == "LinkEvaluatorRenderSize")
                    removePropertyLinkArrowGraphicsItem(arrow);
        }
    } else {
        foreach(ProcessorGraphicsItem* proc1, processorList){
            foreach(ProcessorGraphicsItem* proc2, processorList){
                foreach(PropertyGraphicsItem* prop1, proc1->getPropertyList()->getAllPropertyItems()){
                    if(prop1->getProperty()->getClassName() == "RenderSizeOriginProperty" ||
                       prop1->getProperty()->getClassName() == "RenderSizeReceiveProperty"){
                        foreach(PropertyLinkArrowGraphicsItem* arrow, prop1->getSourceLinkList()){
                            foreach(PropertyGraphicsItem* prop2, proc2->getPropertyList()->getAllPropertyItems()){
                                if(arrow->getDestinationItem() == prop2 &&
                                  (prop2->getProperty()->getClassName() == "RenderSizeOriginProperty" ||
                                   prop2->getProperty()->getClassName() == "RenderSizeReceiveProperty")){
                                    removePropertyLinkArrowGraphicsItem(arrow);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void NetworkEditor::removePortOwnerGraphicsItem(PortOwnerGraphicsItem* poItem) {
    if (!getProcessorNetwork())
        return;

    int poItemType = poItem->type();

    if(poItemType == UserTypesProcessorGraphicsItem)
        foreach (Processor* processor, poItem->getProcessors())
            getProcessorNetwork()->removeProcessor(processor);
    else
    if (poItemType == UserTypesAggregationGraphicsItem) {
        AggregationGraphicsItem* aggItem = static_cast<AggregationGraphicsItem*>(poItem);
        //setUpdatesEnabled(false);
        foreach (PortOwnerGraphicsItem* childItem, aggItem->getContainingItems())
            delete childItem;
        delete aggItem;
        aggregationItems_.removeOne(aggItem);
        //setUpdatesEnabled(true);
        //scene()->clearSelection();
    }
}

void NetworkEditor::removePortArrowGraphicsItem(PortArrowGraphicsItem* arrow) {
    if (!getProcessorNetwork())
        return;

    if (arrow->getDestinationItem() != 0) {
        getProcessorNetwork()->disconnectPorts(arrow->getSourceItem()->getPort(), arrow->getDestinationItem()->getPort());
    }
}

void NetworkEditor::removePropertyLinkArrowGraphicsItem(PropertyLinkArrowGraphicsItem* arrow) {
    if (!getProcessorNetwork())
        return;

    if (arrow->getDestinationItem() != 0) {
        const Property* src = arrow->getSourceItem()->getProperty();
        const Property* dst = arrow->getDestinationItem()->getProperty();
        getProcessorNetwork()->removePropertyLink(src->getLink(dst));
        //remove other link if exists
        if(dst->getLink(src))
            getProcessorNetwork()->removePropertyLink(dst->getLink(src));
    }
}

void NetworkEditor::removePortSizeLinkArrowGraphicsItem(PortSizeLinkArrowGraphicsItem* arrow) {
    if (!getProcessorNetwork())
        return;

    if (arrow->getDestinationItem()) {
        const Property* src = arrow->getSourceItem()->getProperty();
        const Property* dst = arrow->getDestinationItem()->getProperty();
        getProcessorNetwork()->removePropertyLink(src->getLink(dst));
        //remove other link if exists
        if(dst->getLink(src))
            getProcessorNetwork()->removePropertyLink(dst->getLink(src));
    }
}

void NetworkEditor::removePortOwnerLinkArrowGraphicsItem(PortOwnerLinkArrowGraphicsItem* arrow) {
    if (!getProcessorNetwork())
        return;

    if(arrow->getDestinationItem()) {
        LERROR("Not Implemented Yet!!!!!");
    }
}


ProcessorGraphicsItem* NetworkEditor::getProcessorGraphicsItem(const Processor* processor) const {
    tgtAssert(processor, "null pointer passed");
    return processorItemMap_.find(const_cast<Processor*>(processor)).value();
}

PortGraphicsItem* NetworkEditor::getPortGraphicsItem(const Port* port) const {
    tgtAssert(port, "null pointer passed");

    PortGraphicsItem* portItem = 0;
    if (processorItemMap_.contains(port->getProcessor())) {
        ProcessorGraphicsItem* outportProcItem = processorItemMap_[port->getProcessor()];
        portItem = outportProcItem->getPortGraphicsItem(port);
    }
    else {
        /*foreach (AggregationGraphicsItem* i, aggregationItems_) {
            if (i->contains(port->getProcessor()))
                portItem = i->getPortGraphicsItem(port);
        }*/
    }

    tgtAssert(portItem, "no portgraphicsitem found");
    return portItem;
}

PropertyGraphicsItem* NetworkEditor::getPropertyGraphicsItem(const Property* prop) const {
    tgtAssert(prop, "null pointer passed");

    PropertyGraphicsItem* propItem = 0;
    foreach(ProcessorGraphicsItem* item, processorItemMap_) {
        if(item->getPropertyList()->getPropertyItem(prop))
            return item->getPropertyList()->getPropertyItem(prop);
        foreach(PortGraphicsItem* portItem, item->getPortGraphicsItems()){
            if(portItem->getPropertyList()->getPropertyItem(prop))
                return portItem->getPropertyList()->getPropertyItem(prop);
        }
    }

    tgtAssert(propItem, "no portgraphicsitem found");
    return propItem;
}

void NetworkEditor::updateSelectedItems() {
    if (!getProcessorNetwork())
        return;

    QList<QGraphicsItem*> selectedItems = scene()->selectedItems();

    QList<Processor*> selectedProcessors;
    foreach (QGraphicsItem* selectedItem, selectedItems) {
        bool isPortOwner = (selectedItem->type() == UserTypesProcessorGraphicsItem) || (selectedItem->type() == UserTypesAggregationGraphicsItem);

        if (isPortOwner) {
            PortOwnerGraphicsItem* portOwnerItem = static_cast<PortOwnerGraphicsItem*>(selectedItem);
            selectedProcessors += portOwnerItem->getProcessors();
            portOwnerItem->saveMeta();
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

void NetworkEditor::selectPreviouslySelectedProcessors() {
    if (!getProcessorNetwork())
        return;

    SelectionMetaData<Processor*>* selectionMetaData = dynamic_cast<SelectionMetaData<Processor*>*>(getProcessorNetwork()->getMetaDataContainer().getMetaData("ProcessorSelection"));
    if (selectionMetaData) {
        QList<Processor*> selectedProcessors = stdVectorToQList(selectionMetaData->getValues());
        foreach (Processor* proc, selectedProcessors) {
            bool alreadyFound = false;

            // first search all the Aggregations if any Aggregation contains the processor
            /*foreach (AggregationGraphicsItem* aggItem, aggregationItems_) {
                if (aggItem->contains(proc)) {
                    alreadyFound = true;
                    aggItem->setSelected(true);
                    break;
                }
            }*/

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

QList<AggregationGraphicsItem*> NetworkEditor::getSelectedAggregationGraphicsItems() {
    QList<AggregationGraphicsItem*> aggregations;
    foreach(QGraphicsItem* item, scene()->selectedItems()) {
        if(item->type() == UserTypesAggregationGraphicsItem )
            aggregations.append(dynamic_cast<AggregationGraphicsItem*>(item));
    }
    return aggregations;
}

QList<PortOwnerGraphicsItem*> NetworkEditor::getSelectedPortOwnerGraphicsItems() {
    QList<PortOwnerGraphicsItem*> portOwners;
    foreach(QGraphicsItem* item, scene()->selectedItems()) {
        if(item->type() == UserTypesProcessorGraphicsItem ||
           item->type() == UserTypesAggregationGraphicsItem )
            portOwners.append(dynamic_cast<PortOwnerGraphicsItem*>(item));
    }
    return portOwners;
}

QList<ProcessorGraphicsItem*> NetworkEditor::getSelectedProcessorGraphicsItems() {
    QList<ProcessorGraphicsItem*> processors;
    foreach(QGraphicsItem* item, scene()->selectedItems()) {
        if(item->type() == UserTypesProcessorGraphicsItem )
            processors.append(dynamic_cast<ProcessorGraphicsItem*>(item));
    }
    return processors;
}

//---------------------------------------------------------------------------------------------------------------
//                  style, layer and cursor management
//---------------------------------------------------------------------------------------------------------------
NetworkEditorLayer NetworkEditor::getCurrentLayer() const {
    return currentLayer_;
}

NetworkEditorCursorMode NetworkEditor::getCurrentCursorMode() const {
    return currentCursorMode_;
}

NWEStyle_Base* NetworkEditor::getCurrentStyle() const {
    return currentStyle_;
}

bool NetworkEditor::getCurrentToolTipMode() const {
    return currentToolTipMode_;
}

void NetworkEditor::setLayer(NetworkEditorLayer layer) {
    if (layer == currentLayer_)
        return;

    switch(layer){
    case NetworkEditorLayerDataFlow:
        linkingLayerButtonContainer_->setVisible(false);
        generalLinkingLayerButtonContainer_->setVisible(false);
        cameraLinkingLayerButtonContainer_->setVisible(false);
        portSizeLinkingLayerButtonContainer_->setVisible(false);
        break;
    case NetworkEditorLayerLinking:
        if(generalLinkingButton_->isChecked())
            setLayer(NetworkEditorLayerGeneralLinking);
        else if(cameraLinkingButton_->isChecked())
            setLayer(NetworkEditorLayerCameraLinking);
        else if(portSizeLinkingButton_->isChecked())
            setLayer(NetworkEditorLayerPortSizeLinking);
        else
            tgtAssert(false,"no button checked");
        return;
        break;
    case NetworkEditorLayerGeneralLinking:
        linkingLayerButtonContainer_->setVisible(true);
        generalLinkingLayerButtonContainer_->setVisible(true);
        cameraLinkingLayerButtonContainer_->setVisible(false);
        portSizeLinkingLayerButtonContainer_->setVisible(false);
        break;
    case NetworkEditorLayerCameraLinking:
        linkingLayerButtonContainer_->setVisible(true);
        generalLinkingLayerButtonContainer_->setVisible(false);
        cameraLinkingLayerButtonContainer_->setVisible(true);
        portSizeLinkingLayerButtonContainer_->setVisible(false);
        break;
    case NetworkEditorLayerPortSizeLinking:
        linkingLayerButtonContainer_->setVisible(true);
        generalLinkingLayerButtonContainer_->setVisible(false);
        cameraLinkingLayerButtonContainer_->setVisible(false);
        portSizeLinkingLayerButtonContainer_->setVisible(true);
        break;
    default:
        tgtAssert(false,"Unknown Layer");
        break;
    }

    currentLayer_ = layer;

    //set layer for each processor
    foreach (ProcessorGraphicsItem* procItem, processorItemMap_.values()) {
        if (procItem)
            procItem->updateNWELayerAndCursor();
    }

    //set layer for each aggregation
    foreach (AggregationGraphicsItem* aggItem, aggregationItems_)
        aggItem->updateNWELayerAndCursor();

    resetCachedContent(); //needed?
}

void NetworkEditor::setCursorMode(NetworkEditorCursorMode mode) {
    if (currentCursorMode_ == mode)
        return;

    currentCursorMode_ = mode;

    //set cursor mode for each processor
    foreach (ProcessorGraphicsItem* procItem, processorItemMap_.values()) {
        if (procItem)
            procItem->updateNWELayerAndCursor();
    }

    switch(currentCursorMode_){
    case NetworkEditorCursorSelectMode:
        setCursor(Qt::ArrowCursor);
        break;
    case NetworkEditorCursorMoveMode:
        setCursor(Qt::OpenHandCursor);
        scene()->clearSelection();
        emit processorsSelected(QList<Processor*>());
        break;
    default:
        tgtAssert(false, "should not get here");
        break;
    }
}

void NetworkEditor::processorFontOnChange() {
    NWEBaseGraphicsItem* base = 0;
    foreach(QGraphicsItem* item,scene()->items()){
        if(base = dynamic_cast<NWEBaseGraphicsItem*>(item))
            base->resetPaintInitialization();
    }
    scene()->update();
}

void NetworkEditor::styleOnChange() {
    if(VoreenApplicationQt* appQt = dynamic_cast<VoreenApplicationQt*>(VoreenApplicationQt::app())){
        if(OptionProperty<NetworkEditorStyles>* prop = dynamic_cast<OptionProperty<NetworkEditorStyles>*>(appQt->getProperty("networkEditorStyleProperty"))){
            switch(prop->getValue()){
            case NWESTYLE_CLASSIC:
                setStyle(new NWEStyle_Classic());
            break;
            case NWESTYLE_CLASSIC_PRINT:
                setStyle(new NWEStyle_Classic_Print());
                break;
            default:
                tgtAssert(false,"Unknown NetworkEditorStyle!!! Style not changed.");
                LERROR("Unknown NetworkEditorStyle!!! Style not changed.");
                break;
            }
        }
    }
}

void NetworkEditor::updateGraphLayout() {
    if(VoreenApplicationQt* appQt = dynamic_cast<VoreenApplicationQt*>(VoreenApplicationQt::app())){
        if(OptionProperty<NetworkEditorGraphLayouts>* prop = dynamic_cast<OptionProperty<NetworkEditorGraphLayouts>*>(appQt->getProperty("networkEditorGraphLayoutsProperty"))){
            switch(prop->getValue()){
            case NWEGL_SUGIYAMA: {
                qreal shift = 300.f; bool overlap = false, median = true, portflush = true;
                if(FloatProperty* shiftProp = dynamic_cast<FloatProperty*>(appQt->getProperty("sugiShiftXProperty")))
                    shift = shiftProp->get();
                if(BoolProperty* overlapProp = dynamic_cast<BoolProperty*>(appQt->getProperty("sugiOverlapProperty")))
                    overlap = overlapProp->get();
                if(BoolProperty* medianProp = dynamic_cast<BoolProperty*>(appQt->getProperty("sugiMedianProperty")))
                    median = medianProp->get();
                if(BoolProperty* portflushProp = dynamic_cast<BoolProperty*>(appQt->getProperty("sugiPortFlushProperty")))
                    portflush = portflushProp->get();
                static_cast<NWEGL_Sugiyama*>(currentGraphLayout_)->setSortParameter(shift,overlap,median,portflush);
            } break;
            default:
                tgtAssert(false,"Unknown NetworkEditorGraphLayout!!!");
                LERROR("Unknown NetworkEditorGraphLayout!!!");
                break;
            }
        }
    }
}

void NetworkEditor::setStyle(NWEStyle_Base* style) {
    delete currentStyle_;
    currentStyle_ = style;
    NWEBaseGraphicsItem* base = 0;
    foreach(QGraphicsItem* item,scene()->items()){
        if(base = dynamic_cast<NWEBaseGraphicsItem*>(item))
            base->resetPaintInitialization();
    }
    //set background color
    setBackgroundBrush(currentStyle_->NWEStyle_NWEBackgroundBrush);
    //set button background
    QPalette pal = generalLinkingLayerButtonContainer_->palette();
    pal.setColor(generalLinkingLayerButtonContainer_->backgroundRole(), currentStyle_->NWEStyle_NWEButtonBackgroundColor);
    generalLinkingLayerButtonContainer_->setPalette(pal);
    pal = cameraLinkingLayerButtonContainer_->palette();
    pal.setColor(cameraLinkingLayerButtonContainer_->backgroundRole(), currentStyle_->NWEStyle_NWEButtonBackgroundColor);
    cameraLinkingLayerButtonContainer_->setPalette(pal);
    pal = portSizeLinkingLayerButtonContainer_->palette();
    pal.setColor(portSizeLinkingLayerButtonContainer_->backgroundRole(), currentStyle_->NWEStyle_NWEButtonBackgroundColor);
    portSizeLinkingLayerButtonContainer_->setPalette(pal);
    pal = linkingLayerButtonContainer_->palette();
    pal.setColor(linkingLayerButtonContainer_->backgroundRole(), currentStyle_->NWEStyle_NWEButtonBackgroundColor);
    linkingLayerButtonContainer_->setPalette(pal);

    update();
}

void NetworkEditor::setToolTipMode(bool mode) {
    if(currentToolTipMode_ == mode) return;

    currentToolTipMode_ = mode;
}

void NetworkEditor::toggleToolTip() {
    currentToolTipMode_ = !currentToolTipMode_;
}

//---------------------------------------------------------------------------------------------------------------
//                  button management
//---------------------------------------------------------------------------------------------------------------
void NetworkEditor::initilizeEditorButtons() {
//general linking
    generalLinkingLayerButtonContainer_ = new QWidget(this);
    generalLinkingLayerButtonContainer_->setAutoFillBackground(true);
    QPalette pal = generalLinkingLayerButtonContainer_->palette();
    pal.setColor(generalLinkingLayerButtonContainer_->backgroundRole(), currentStyle_->NWEStyle_NWEButtonBackgroundColor);
    generalLinkingLayerButtonContainer_->setPalette(pal);
    QBoxLayout* generalLinkingLayerButtonLayout = new QHBoxLayout(generalLinkingLayerButtonContainer_);
    generalLinkingLayerButtonLayout->setMargin(NWEButtonBackgroundMargin);
    //create buttons
    hideCameraLinksButton_ = new QToolButton;
        hideCameraLinksButton_->setIcon(QIcon(":/qt/icons/hide-linking-camera.png"));
        hideCameraLinksButton_->setIconSize(NWESub2ButtonSize);
        hideCameraLinksButton_->setToolTip(tr("hides all camera links"));
        hideCameraLinksButton_->setCheckable(true);
        hideCameraLinksButton_->setChecked(false);
        connect(hideCameraLinksButton_, SIGNAL(clicked()), this, SLOT(hideCameraLinks()));
        generalLinkingLayerButtonLayout->addWidget(hideCameraLinksButton_);
    hidePortSizeLinksButton_ = new QToolButton;
        hidePortSizeLinksButton_->setIcon(QIcon(":/qt/icons/hide-linking-port.png"));
        hidePortSizeLinksButton_->setIconSize(NWESub2ButtonSize);
        hidePortSizeLinksButton_->setToolTip(tr("hides all port size links"));
        hidePortSizeLinksButton_->setCheckable(true);
        hidePortSizeLinksButton_->setChecked(true);
        connect(hidePortSizeLinksButton_, SIGNAL(clicked()), this, SLOT(hidePortSizeLinks()));
        generalLinkingLayerButtonLayout->addWidget(hidePortSizeLinksButton_);
    removeAllPropertyLinksButton_ = new QToolButton;
        removeAllPropertyLinksButton_->setIcon(QIcon(":/qt/icons/linking-remove.png"));
        removeAllPropertyLinksButton_->setIconSize(NWESub2ButtonSize);
        removeAllPropertyLinksButton_->setToolTip(tr("remove all property links from the network"));
        connect(removeAllPropertyLinksButton_, SIGNAL(clicked()), this, SLOT(removeAllPropertyLinks()));
        generalLinkingLayerButtonLayout->addWidget(removeAllPropertyLinksButton_);

//camera linking
    cameraLinkingLayerButtonContainer_ = new QWidget(this);
    cameraLinkingLayerButtonContainer_->setAutoFillBackground(true);
    pal = cameraLinkingLayerButtonContainer_->palette();
    pal.setColor(cameraLinkingLayerButtonContainer_->backgroundRole(), currentStyle_->NWEStyle_NWEButtonBackgroundColor);
    cameraLinkingLayerButtonContainer_->setPalette(pal);
    QBoxLayout* cameraLinkingLayerButtonLayout = new QHBoxLayout(cameraLinkingLayerButtonContainer_);
    cameraLinkingLayerButtonLayout->setMargin(NWEButtonBackgroundMargin);
    //create buttons
    linkCamerasAutoButton_ = new QToolButton;
        linkCamerasAutoButton_->setIcon(QIcon(":/qt/icons/linking-camera-auto.png"));
        linkCamerasAutoButton_->setIconSize(NWESub2ButtonSize);
        linkCamerasAutoButton_->setToolTip(tr("link cameras of processors when they are added to the network"));
        linkCamerasAutoButton_->setCheckable(true);
        linkCamerasAutoButton_->setChecked(true);
        connect(linkCamerasAutoButton_, SIGNAL(clicked()), this, SLOT(linkCamerasAutoChanged()));
        cameraLinkingLayerButtonLayout->addWidget(linkCamerasAutoButton_);
    linkCamerasButton_ = new QToolButton;
        linkCamerasButton_->setIcon(QIcon(":/qt/icons/linking-camera.png"));
        linkCamerasButton_->setIconSize(NWESub2ButtonSize);
        linkCamerasButton_->setToolTip(tr("link all cameras in the network"));
        connect(linkCamerasButton_, SIGNAL(clicked()), this, SLOT(linkCameras()));
        cameraLinkingLayerButtonLayout->addWidget(linkCamerasButton_);
    removeAllCameraLinksButton_ = new QToolButton;
        removeAllCameraLinksButton_->setIcon(QIcon(":/qt/icons/linking-remove.png"));
        removeAllCameraLinksButton_->setIconSize(NWESub2ButtonSize);
        removeAllCameraLinksButton_->setToolTip(tr("remove all camera property links from the network"));
        connect(removeAllCameraLinksButton_, SIGNAL(clicked()), this, SLOT(removeAllCameraLinks()));
        cameraLinkingLayerButtonLayout->addWidget(removeAllCameraLinksButton_);

//port size linking
    portSizeLinkingLayerButtonContainer_ = new QWidget(this);
    portSizeLinkingLayerButtonContainer_->setAutoFillBackground(true);
    pal = portSizeLinkingLayerButtonContainer_->palette();
    pal.setColor(portSizeLinkingLayerButtonContainer_->backgroundRole(), currentStyle_->NWEStyle_NWEButtonBackgroundColor);
    portSizeLinkingLayerButtonContainer_->setPalette(pal);
    QBoxLayout* portSizeLinkingLayerButtonLayout = new QHBoxLayout(portSizeLinkingLayerButtonContainer_);
    portSizeLinkingLayerButtonLayout->setMargin(NWEButtonBackgroundMargin);
    //create buttons
    linkPortSizeAutoButton_ = new QToolButton;
        linkPortSizeAutoButton_->setIcon(QIcon(":/qt/icons/linking-port-auto.png"));
        linkPortSizeAutoButton_->setIconSize(NWESub2ButtonSize);
        linkPortSizeAutoButton_->setToolTip(tr("link render port sizes of processors when they are added to the network"));
        linkPortSizeAutoButton_->setCheckable(true);
        linkPortSizeAutoButton_->setChecked(true);
        connect(linkPortSizeAutoButton_, SIGNAL(clicked()), this, SLOT(linkPortSizeAutoChanged()));
        portSizeLinkingLayerButtonLayout->addWidget(linkPortSizeAutoButton_);
    linkPortSizeButton_ = new QToolButton;
        linkPortSizeButton_->setIcon(QIcon(":/qt/icons/linking-port.png"));
        linkPortSizeButton_->setIconSize(NWESub2ButtonSize);
        linkPortSizeButton_->setToolTip(tr("link all cameras in the network"));
        connect(linkPortSizeButton_, SIGNAL(clicked()), this, SLOT(linkPortSize()));
        portSizeLinkingLayerButtonLayout->addWidget(linkPortSizeButton_);
    removeAllPortSizeLinksButton_ = new QToolButton;
        removeAllPortSizeLinksButton_->setIcon(QIcon(":/qt/icons/linking-remove.png"));
        removeAllPortSizeLinksButton_->setIconSize(NWESub2ButtonSize);
        removeAllPortSizeLinksButton_->setToolTip(tr("remove all port size property links from the network"));
        connect(removeAllPortSizeLinksButton_, SIGNAL(clicked()), this, SLOT(removeAllPortSizeLinks()));
        portSizeLinkingLayerButtonLayout->addWidget(removeAllPortSizeLinksButton_);

//linking layers
    linkingLayerButtonContainer_ = new QWidget(this);
    linkingLayerButtonContainer_->setAutoFillBackground(true);
    pal = linkingLayerButtonContainer_->palette();
    pal.setColor(linkingLayerButtonContainer_->backgroundRole(), currentStyle_->NWEStyle_NWEButtonBackgroundColor);
    linkingLayerButtonContainer_->setPalette(pal);
    linkingLayerButtonContainer_->setMinimumSize(NWEButtonBackgroundMargin*2+NWEMainButtonSize.width(),1);

    QBoxLayout* linkingLayerButtonLayout = new QVBoxLayout(linkingLayerButtonContainer_);
    linkingLayerButtonLayout->setMargin(NWEButtonBackgroundMargin);
    linkingLayerButtonLayout->addSpacing(NWEMainButtonSize.height()+NWEButtonBackgroundMargin+NWEMarginLayerToLinking);
    //create buttons
    generalLinkingButton_ = new QToolButton;
        generalLinkingButton_->setIcon(QIcon(":/qt/icons/linking-general.png"));
        generalLinkingButton_->setIconSize(NWESub1ButtonSize);
        generalLinkingButton_->setToolTip(tr("shows all property links"));
        generalLinkingButton_->setCheckable(true);
        generalLinkingButton_->setChecked(true);
        connect(generalLinkingButton_, SIGNAL(clicked()), this, SLOT(setLayerToGeneralLinking()));
        linkingLayerButtonLayout->addWidget(generalLinkingButton_);
    cameraLinkingButton_ = new QToolButton;
        cameraLinkingButton_->setIcon(QIcon(":/qt/icons/linking-camera.png"));
        cameraLinkingButton_->setIconSize(NWESub1ButtonSize);
        cameraLinkingButton_->setToolTip(tr("shows all camera property links"));
        cameraLinkingButton_->setCheckable(true);
        connect(cameraLinkingButton_, SIGNAL(clicked()), this, SLOT(setLayerToCameraLinking()));
        linkingLayerButtonLayout->addWidget(cameraLinkingButton_);
    portSizeLinkingButton_ = new QToolButton;
        portSizeLinkingButton_->setIcon(QIcon(":/qt/icons/linking-port.png"));
        portSizeLinkingButton_->setIconSize(NWESub1ButtonSize);
        portSizeLinkingButton_->setToolTip(tr("shows all port size property links"));
        portSizeLinkingButton_->setCheckable(true);
        connect(portSizeLinkingButton_, SIGNAL(clicked()), this, SLOT(setLayerToPortSizeLinking()));
        linkingLayerButtonLayout->addWidget(portSizeLinkingButton_);
    // add to button group, so only one can be checked at the same time
    QButtonGroup* linkingLayerButtonGroup = new QButtonGroup(this);
        linkingLayerButtonGroup->addButton(generalLinkingButton_);
        linkingLayerButtonGroup->addButton(cameraLinkingButton_);
        linkingLayerButtonGroup->addButton(portSizeLinkingButton_);

//layer buttons
    layerButtonContainer_ = new QWidget(this);
    QBoxLayout* layerButtonLayout = new QHBoxLayout(layerButtonContainer_);
    layerButtonLayout->setMargin(NWEButtonBackgroundMargin);
    //create buttons
    dataFlowLayerButton_ = new QToolButton;
        dataFlowLayerButton_->setIcon(QIcon(":/qt/icons/dataflow-mode.png"));
        dataFlowLayerButton_->setIconSize(NWEMainButtonSize);
        dataFlowLayerButton_->setToolTip(tr("Switch to data flow mode (ctrl+1)"));
        dataFlowLayerButton_->setCheckable(true);
        dataFlowLayerButton_->setShortcut(Qt::CTRL + Qt::Key_1);
        connect(dataFlowLayerButton_, SIGNAL(clicked()), this, SLOT(setLayerToDataFlow()));
        layerButtonLayout->addWidget(dataFlowLayerButton_);
    linkingLayerButton_ = new QToolButton;
        linkingLayerButton_->setIcon(QIcon(":/qt/icons/linking-mode.png"));
        linkingLayerButton_->setIconSize(NWEMainButtonSize);
        linkingLayerButton_->setToolTip(tr("Switch to linking mode (ctrl+2)"));
        linkingLayerButton_->setCheckable(true);
        linkingLayerButton_->setShortcut(Qt::CTRL + Qt::Key_2);
        connect(linkingLayerButton_, SIGNAL(clicked()), this, SLOT(setLayerToLinking()));
        layerButtonLayout->addWidget(linkingLayerButton_);
    // add to button group, so only one can be checked at the same time
    QButtonGroup* layerButtonGroup = new QButtonGroup(this);
        layerButtonGroup->addButton(dataFlowLayerButton_);
        layerButtonGroup->addButton(linkingLayerButton_);

//stop network button
    stopButtonContainer_ = new QWidget(this);
    QBoxLayout* stopButtonLayout = new QHBoxLayout(stopButtonContainer_);
    stopButtonLayout->setMargin(NWEButtonBackgroundMargin);
    stopNetworkEvaluatorButton_ = new QToolButton;
    stopNetworkEvaluatorButton_->setIcon(QIcon(":/qt/icons/player-pause.png"));
    stopNetworkEvaluatorButton_->setIconSize(NWEMainButtonSize);
    stopNetworkEvaluatorButton_->setToolTip(tr("Stop the automatic evaluation of the network"));
    connect(stopNetworkEvaluatorButton_, SIGNAL(clicked()), this, SLOT(toggleNetworkEvaluator()));
    stopNetworkEvaluatorButton_->setCheckable(true);
    stopButtonLayout->addWidget(stopNetworkEvaluatorButton_);

//navigation
    navigationButtonContainer_ = new QWidget(this);
    QHBoxLayout* navigationButtonContainerLayout = new QHBoxLayout(navigationButtonContainer_);
    navigationButtonContainerLayout->setMargin(NWEButtonBackgroundMargin);
    //create buttons
    selectCursorButton_ = new QToolButton;
        selectCursorButton_->setIcon(QIcon(":/qt/icons/cursor_arrow.svg"));
        selectCursorButton_->setIconSize(NWEMainButtonSize);
        selectCursorButton_->setToolTip(tr("Select network items"));
        selectCursorButton_->setCheckable(true);
        selectCursorButton_->setChecked(true);
        connect(selectCursorButton_, SIGNAL(clicked()), this, SLOT(setCursorSelect()));
        navigationButtonContainerLayout->addWidget(selectCursorButton_);
    moveCursorButton_ = new QToolButton;
        moveCursorButton_->setIcon(QIcon(":/qt/icons/cursor_hand.svg"));
        moveCursorButton_->setIconSize(NWEMainButtonSize);
        moveCursorButton_->setToolTip(tr("Navigate in network (shift)"));
        moveCursorButton_->setCheckable(true);
        connect(moveCursorButton_, SIGNAL(clicked()), this, SLOT(setCursorMove()));
        navigationButtonContainerLayout->addWidget(moveCursorButton_);
    QButtonGroup* navigationButtonGroup = new QButtonGroup(this);
        navigationButtonGroup->addButton(selectCursorButton_);
        navigationButtonGroup->addButton(moveCursorButton_);

//layout
    layoutButtonContainer_ = new QWidget(this);
    QHBoxLayout* layoutButtonContainerLayout = new QHBoxLayout(layoutButtonContainer_);
    layoutButtonContainerLayout->setMargin(NWEButtonBackgroundMargin);

    centerViewButton_ = new QToolButton;
        centerViewButton_->setIcon(QIcon(":/qt/icons/center.png"));
        centerViewButton_->setIconSize(NWEMainButtonSize);
        centerViewButton_->setToolTip(tr("Overview entire network"));
        connect(centerViewButton_, SIGNAL(clicked()), this, SLOT(setViewCenter()));
        layoutButtonContainerLayout->addWidget(centerViewButton_);

    graphLayoutButton_ = new QToolButton;
        graphLayoutButton_->setIcon(QIcon(":/qt/icons/sortGraph.png"));
        graphLayoutButton_->setIconSize(NWEMainButtonSize);
        graphLayoutButton_->setToolTip(tr("Lay out network graph"));
        connect(graphLayoutButton_, SIGNAL(clicked()), this, SLOT(sortEntireNetwork()));
        layoutButtonContainerLayout->addWidget(graphLayoutButton_);

    layoutEditorButtons();
}

void NetworkEditor::layoutEditorButtons() {
//layer buttons
    int x = size().width() - layerButtonContainer_->size().width() - NWEButtonBorderSpacingX;
    int y = NWEButtonBorderSpacingY;
    layerButtonContainer_->move(x,y);
//linking buttons
    x = size().width() - NWEMainButtonSize.width() - NWEButtonBorderSpacingX - NWEButtonBackgroundMargin*3;
    linkingLayerButtonContainer_->move(x,y);
//general linking buttons
    x -= generalLinkingLayerButtonContainer_->width();
    y += layerButtonContainer_->size().height() + NWEMarginLayerToLinking - NWEButtonBackgroundMargin*2;
    generalLinkingLayerButtonContainer_->move(x,y);
//camera linking buttons
    y += generalLinkingLayerButtonContainer_->height() + linkingLayerButtonContainer_->layout()->contentsMargins().top();
    cameraLinkingLayerButtonContainer_->move(x,y);
//port size linking buttons
    y += portSizeLinkingLayerButtonContainer_->height() + linkingLayerButtonContainer_->layout()->contentsMargins().top();
    portSizeLinkingLayerButtonContainer_->move(x,y);
//stop network button
    x = size().width() - stopButtonContainer_->size().width() - NWEButtonBorderSpacingX;
    y = size().height() - stopButtonContainer_->size().height() - NWEButtonBorderSpacingY;
    stopButtonContainer_->move(x,y);
//navigation buttons
    x = NWEButtonBorderSpacingX;
    y = NWEButtonBorderSpacingY;
    navigationButtonContainer_->move(x,y);
//layout button
    x = NWEButtonBorderSpacingX;
    y = size().height() - layoutButtonContainer_->size().height() - NWEButtonBorderSpacingY;
    layoutButtonContainer_->move(x,y);
}

bool NetworkEditor::cameraLinksHidden() {
    return hideCameraLinksButton_->isChecked();
}

bool NetworkEditor::portSizeLinksHidden() {
    return hidePortSizeLinksButton_->isChecked();
}

void NetworkEditor::setLayerToDataFlow() {
    setLayer(NetworkEditorLayerDataFlow);
}

void NetworkEditor::setLayerToLinking() {
    setLayer(NetworkEditorLayerLinking);
}

void NetworkEditor::setLayerToGeneralLinking() {
    setLayer(NetworkEditorLayerGeneralLinking);
}

void NetworkEditor::setLayerToCameraLinking() {
    setLayer(NetworkEditorLayerCameraLinking);
}

void NetworkEditor::setLayerToPortSizeLinking() {
    setLayer(NetworkEditorLayerPortSizeLinking);
}

void NetworkEditor::setCursorSelect() {
    setCursorMode(NetworkEditorCursorSelectMode);
}

void NetworkEditor::setCursorMove() {
    setCursorMode(NetworkEditorCursorMoveMode);
}

void NetworkEditor::hideCameraLinks() {
    foreach(ProcessorGraphicsItem* processorItem, processorItemMap_) {
        if(processorItem->getPropertyList()->getIsVisibleInEditor()){
            processorItem->getPropertyList()->prepareGeometryChange();
            if(cameraLinksHidden() && portSizeLinksHidden())
                processorItem->getPropertyList()->setPropertyVisibleModifier(PropertyListGraphicsItem::HIDE_CAMERA_AND_SIZE_PROPERTIES);
            else if(cameraLinksHidden())
                processorItem->getPropertyList()->setPropertyVisibleModifier(PropertyListGraphicsItem::HIDE_CAMERA_PROPERTIES);
            else if(portSizeLinksHidden())
                processorItem->getPropertyList()->setPropertyVisibleModifier(PropertyListGraphicsItem::HIDE_SIZE_PROPERTIES);
            else
                processorItem->getPropertyList()->setPropertyVisibleModifier(PropertyListGraphicsItem::HIDE_NO_PROPERTIES);
            processorItem->getPropertyList()->setVisible(true);
        }
        foreach(PortOwnerLinkArrowGraphicsItem* arrow, processorItem->getPortOwnerLinkArrows())
            arrow->setVisible(true);
    }
}

void NetworkEditor::hidePortSizeLinks() {
    foreach(ProcessorGraphicsItem* processorItem, processorItemMap_) {
        if(processorItem->getPropertyList()->getIsVisibleInEditor()){
            processorItem->getPropertyList()->prepareGeometryChange();
            if(cameraLinksHidden() && portSizeLinksHidden())
                processorItem->getPropertyList()->setPropertyVisibleModifier(PropertyListGraphicsItem::HIDE_CAMERA_AND_SIZE_PROPERTIES);
            else if(cameraLinksHidden())
                processorItem->getPropertyList()->setPropertyVisibleModifier(PropertyListGraphicsItem::HIDE_CAMERA_PROPERTIES);
            else if(portSizeLinksHidden())
                processorItem->getPropertyList()->setPropertyVisibleModifier(PropertyListGraphicsItem::HIDE_SIZE_PROPERTIES);
            else
                processorItem->getPropertyList()->setPropertyVisibleModifier(PropertyListGraphicsItem::HIDE_NO_PROPERTIES);
            processorItem->getPropertyList()->setVisible(true);
        }
        foreach(PortOwnerLinkArrowGraphicsItem* arrow, processorItem->getPortOwnerLinkArrows())
            arrow->setVisible(true);
    }
}

void NetworkEditor::removeAllPropertyLinks() {
    if (!processorNetwork_)
        return;

    if (QMessageBox::question(this, tr("VoreenVE"), tr("Remove all property links from the current network?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
        int numLinks = processorNetwork_->removeAllPropertyLinks();
        if (numLinks)
            LINFO("Removed " << numLinks << " property links");
    }
}

void NetworkEditor::linkCamerasAutoChanged() {
    if (!processorNetwork_)
        return;

    tgtAssert(getProcessorNetwork(), "No processor network");
    if (processorNetwork_->getMetaDataContainer().hasMetaData("autoLinkCameras")) {
        MetaDataBase* base = processorNetwork_->getMetaDataContainer().getMetaData("autoLinkCameras");
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
        processorNetwork_->getMetaDataContainer().addMetaData("autoLinkCameras", meta);
    }
}

void NetworkEditor::linkCameras() {
    if (!processorNetwork_)
        return;

    if (QMessageBox::question(this, tr("VoreenVE"), tr("Link all cameras in the current network?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
        int numLinks = getProcessorNetwork()->createPropertyLinksWithinSubNetwork<CameraProperty>(
            processorNetwork_->getProcessors(), std::vector<std::string>(), new LinkEvaluatorCameraId());
        if (numLinks)
            LINFO("Created " << numLinks << " camera property links");
    }
}

void NetworkEditor::removeAllCameraLinks() {
    if (!processorNetwork_)
        return;

    if (QMessageBox::question(this, tr("VoreenVE"), tr("Remove all camera links from the current network?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
        int numRemoved = processorNetwork_->removePropertyLinksFromSubNetwork<CameraProperty>(processorNetwork_->getProcessors());
        if (numRemoved)
            LINFO("Removed " << numRemoved << " camera property links");
    }
}

void NetworkEditor::linkPortSizeAutoChanged() {
    if (!processorNetwork_)
        return;

    tgtAssert(processorNetwork_, "No processor network");
    if (processorNetwork_->getMetaDataContainer().hasMetaData("autoLinkPortSize")) {
        MetaDataBase* base = getProcessorNetwork()->getMetaDataContainer().getMetaData("autoLinkPortSize");
        BoolMetaData* meta = dynamic_cast<BoolMetaData*>(base);
        if (!meta) {
            LWARNING("Meta data object not of expected type 'BoolMetaData'");
            return;
        }
        meta->setValue(linkCamerasAutoButton_->isChecked());
    }
    else {
        BoolMetaData* meta = new BoolMetaData;
        meta->setValue(linkPortSizeAutoButton_->isChecked());
        getProcessorNetwork()->getMetaDataContainer().addMetaData("autoLinkPortSize", meta);
    }
}

void NetworkEditor::linkPortSize() {
    if (!processorNetwork_)
        return;

    if (QMessageBox::question(this, tr("VoreenVE"), tr("Create render size links for all RenderPorts in the current network?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
        int numCreated = processorNetwork_->createRenderSizeLinksWithinSubNetwork(processorNetwork_->getProcessors(), false);
        if (numCreated)
            LINFO("Created " << numCreated << " render size links.");
    }
}

void NetworkEditor::removeAllPortSizeLinks() {
    if (!processorNetwork_)
        return;

    if (QMessageBox::question(this, tr("VoreenVE"), tr("Remove all render size links from the current network?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
        int numRemoved = processorNetwork_->removeRenderSizeLinksFromSubNetwork(processorNetwork_->getProcessors());
        if (numRemoved)
            LINFO("Removed " << numRemoved << " render size links.");
    }
}

void NetworkEditor::toggleNetworkEvaluator() {
    if (networkEvaluatorIsLockedByButton_) {
        evaluator_->unlock();
        networkEvaluatorIsLockedByButton_ = false;
        stopNetworkEvaluatorButton_->setIcon(QIcon(":/qt/icons/player-pause.png"));
        stopNetworkEvaluatorButton_->setToolTip(tr("Stop automatic network evaluation"));
        evaluator_->process();
    }
    else {
        evaluator_->lock();
        stopNetworkEvaluatorButton_->setIcon(QIcon(":/qt/icons/player-start.png"));
        stopNetworkEvaluatorButton_->setToolTip(tr("Start automatic network evaluation"));
        networkEvaluatorIsLockedByButton_ = true;
    }
}

void NetworkEditor::sortEntireNetwork() {
    //return, if nothing to do
    if(!getProcessorNetwork() || getProcessorNetwork()->getProcessors().empty())
        return;

    tgtAssert(currentGraphLayout_, "No GraphLayout");

    //list that saves all positions of processors before repositioning
    std::vector<std::pair<QPointF,Processor*> > savePosList;
    //the current network processors
    std::vector<Processor*> processors = getProcessorNetwork()->getProcessors();
    //save position of all processors before sorting anything
    for (size_t j = 0; j < processors.size(); j++) {
        QMap<Processor*,ProcessorGraphicsItem*>::const_iterator i = processorItemMap_.constBegin();
        while (i != processorItemMap_.constEnd()) {
            if(processors[j] == i.key()) {
                QPointF pos = i.value()->pos();
                std::pair<QPointF,Processor*> savePos = std::make_pair(pos,processors[j]);
                savePosList.push_back(savePos);
                ++i;
            }
            else ++i;
        }
    }

    // empty vector to indicate a sort of the entire network
    std::vector<Processor*> selectedProc;
    currentGraphLayout_->sort(getProcessorNetwork(),&selectedProc,&processorItemMap_);

    //center view
    setViewCenter();

    //popup that asks if you want to keep changes. if not, reset position of all processors
    if (QMessageBox::information(NULL, "Network Layout", "An automatic network layout has been created. Do you want to keep the result?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
        //undo changes
        for(size_t it = 0; it < savePosList.size(); it++) {
            for (size_t it2 = 0; it2 < processors.size(); it2++) {
                if (savePosList[it].second == processors[it2]) {
                    QMap<Processor*,ProcessorGraphicsItem*>::const_iterator i = processorItemMap_.constBegin();
                    while (i != processorItemMap_.constEnd()) {
                        if (i.key() == processors[it2]) {
                            QPointF pos = savePosList[it].first;
                            i.value()->setPos(pos);
                            ++i;
                        }
                        else ++i;
                    }
                }
            }
        }
        setViewCenter();
    } else {
        //save changes
        foreach (ProcessorGraphicsItem* processorItem, processorItemMap_.values()) {
            processorItem->saveMeta();
        }
    }
}

void NetworkEditor::sortSubNetwork() {
    tgtAssert(currentGraphLayout_, "No GraphLayout");

    //get selected Processors
    std::vector<Processor*> selectedProc;
    //list that saves positions of delected processors before repositioning
    std::vector<std::pair<QPointF,Processor*> > savePosList;
    //the current network processors
    std::vector<Processor*> processors = getProcessorNetwork()->getProcessors();
    foreach (QGraphicsItem* item, scene()->selectedItems()) {
        if(item->type() == UserTypesProcessorGraphicsItem) {
            ProcessorGraphicsItem* procItem = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);
            Processor* processor = procItem->getProcessor();
            selectedProc.push_back(processor);
            std::pair<QPointF,Processor*> savePos = std::make_pair(procItem->pos(),processor);
            savePosList.push_back(savePos);
        }
    }
    tgtAssert(selectedProc.size() > 1, "sortSubNetwork called on less then 2 items!");

    //sort
    currentGraphLayout_->sort(getProcessorNetwork(),&selectedProc,&processorItemMap_);

    //popup that asks if you want to keep changes. if not, reset position of all processors
    if (QMessageBox::information(NULL, "Sorting Network", "The processor items have been sorted. Do you want to keep the result?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
        //undo changes
        for(size_t it = 0; it < savePosList.size(); it++) {
            for (size_t it2 = 0; it2 < processors.size(); it2++) {
                if (savePosList[it].second == processors[it2]) {
                    QMap<Processor*,ProcessorGraphicsItem*>::const_iterator i = processorItemMap_.constBegin();
                    while (i != processorItemMap_.constEnd()) {
                        if (i.key() == processors[it2]) {
                            QPointF pos = savePosList[it].first;
                            i.value()->setPos(pos);
                            ++i;
                        }
                        else ++i;
                    }
                }
            }
        }
    } else {
        //save changes
        foreach (QGraphicsItem* item, scene()->selectedItems()) {
            if(item->type() == UserTypesProcessorGraphicsItem)
            {
                ProcessorGraphicsItem* procItem = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);
                procItem->saveMeta();
            }
        }
    }
}


void NetworkEditor::setViewCenter() {
    resetMatrix();
    resetTransform();
    if (!processorNetwork_){
        centerOn(0.0,0.0);
    } else {
        getProcessorNetwork()->getMetaDataContainer().removeMetaData("ZoomFactor");
        getProcessorNetwork()->getMetaDataContainer().removeMetaData("ZoomCenter");
        scaleView();
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

//---------------------------------------------------------------------------------------------------------------
//                  events
//---------------------------------------------------------------------------------------------------------------
void NetworkEditor::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);

    layoutEditorButtons();
    if (isVisible() && needsScale_) {
        scaleView();
        needsScale_ = false;
    }
}

void NetworkEditor::wheelEvent(QWheelEvent *event) {
    float factor = pow(2.0, event->delta() / 360.0);
    scale(factor, factor);
}

void NetworkEditor::mousePressEvent(QMouseEvent* event) {
    // shift and left button activate translation of scene.
    if (event->button() == Qt::LeftButton && (event->modifiers() == Qt::ShiftModifier || currentCursorMode_ == NetworkEditorCursorMoveMode)) {
        translateScene_ = true;
        translateSceneVector_ = mapToScene(event->pos());
        lastTranslateCenter_ =  mapToScene(viewport()->rect().center());
        setCursor(Qt::ClosedHandCursor);
    }
    else
        if(!(event->button() & Qt::RightButton))
            QGraphicsView::mousePressEvent(event);
}

void NetworkEditor::mouseMoveEvent(QMouseEvent* event) {
    if (translateScene_) {
        //set new center
        translateSceneVector_ -= mapToScene(event->pos());
        lastTranslateCenter_ += translateSceneVector_;
        centerOn(lastTranslateCenter_);
        translateSceneVector_ = mapToScene(event->pos());
        //save new center
        if (getProcessorNetwork()) {
            if (getProcessorNetwork()->getMetaDataContainer().hasMetaData("ZoomCenter")) {
                MetaDataBase* base = getProcessorNetwork()->getMetaDataContainer().getMetaData("ZoomCenter");
                Vec2MetaData* meta = dynamic_cast<Vec2MetaData*>(base);
                meta->setValue(tgt::vec2(lastTranslateCenter_.x(),lastTranslateCenter_.y()));
            } else {
                getProcessorNetwork()->getMetaDataContainer().addMetaData("ZoomCenter", new Vec2MetaData(tgt::vec2(lastTranslateCenter_.x(),lastTranslateCenter_.y())));
            }
        }
        QGraphicsView::mouseMoveEvent(event);
    }
    else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void NetworkEditor::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton){
        if(translateScene_){
            if(currentCursorMode_ == NetworkEditorCursorMoveMode)
                setCursor(Qt::OpenHandCursor);
            else
                setCursor(Qt::ArrowCursor);
            translateScene_ = false;
        } else {
            QGraphicsView::mouseReleaseEvent(event);
            if(!currentCursorMode_ == NetworkEditorCursorMoveMode){
                updateSelectedItems();
            }
        }
    } else {
       QGraphicsView::mouseReleaseEvent(event);
    }
}

void NetworkEditor::keyPressEvent(QKeyEvent* event) {
    QGraphicsView::keyPressEvent(event);

    if (!event->isAccepted() && (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)) {
        deleteActionSlot();
    }
    else if (!event->isAccepted() && (event->key() == Qt::Key_Plus)) {
        float factor = keyPressScaleFactor;
        scale(factor, factor);
    }
    else if (!event->isAccepted() && (event->key() == Qt::Key_Minus)) {
        float factor = 1.f / keyPressScaleFactor;
        scale(factor, factor);
    }
    else if (!event->isAccepted() && (event->key() == Qt::Key_C) && (event->modifiers() & Qt::CTRL))
        copyActionSlot();
    else if (!event->isAccepted() && (event->key() == Qt::Key_V) && (event->modifiers() & Qt::CTRL)) {
        rightClickPosition_ = mapToScene(mapFromGlobal(QCursor::pos()));
        pasteActionSlot();
    }
    else if (!event->isAccepted() && (event->key() == Qt::Key_F2) && currentLayer_ == NetworkEditorLayerDataFlow) {
        QList<QGraphicsItem*> items = scene()->selectedItems();
        if (items.count() == 1) {
            QGraphicsItem* item = items[0];
            PortOwnerGraphicsItem* poItem = dynamic_cast<PortOwnerGraphicsItem*>(item);
            if (poItem)
                poItem->enterRenameMode();
        }
    }
}

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

    if (item && (item->type() == UserTypesPortArrowGraphicsItem)) {
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

        Processor* proc = dynamic_cast<Processor*>(VoreenApplication::app()->createSerializableType(event->mimeData()->text().toStdString()));
        tgtAssert(proc, "processor creation failed");

        std::string processorName = proc->getClassName();
        tgtAssert(!processorName.empty(), "Processor class name is empty");

        ProcessorGraphicsItem* item;
        QPointF p;
        if (lowerItem) {
            switch (lowerItem->type()) {
                case UserTypesPortGraphicsItem:
                case QGraphicsTextItem::Type:
                case UserTypesRenamableTextGraphicsItem:
                case UserTypesPropertyListButtonGraphicsItem:
                    lowerItem = lowerItem->parentItem();
                case UserTypesProcessorGraphicsItem:
                //case AggregationGraphicsItem::Type:
                    {
                        p = lowerItem->pos();
                        if(ProcessorGraphicsItem* i = qgraphicsitem_cast<ProcessorGraphicsItem*>(lowerItem))
                            getProcessorNetwork()->replaceProcessor(i->getProcessor(), proc);
                        /*else if(AggregationGraphicsItem* i = qgraphicsitem_cast<AggregationGraphicsItem*>(lowerItem)) {
                            QList<QGraphicsItem*> a;
                            a.append(i);
                            removeItems(a);
                            getProcessorNetwork()->addProcessor(proc, processorName);
                        }*/ else
                            return;
                        updateSelectedItems();
                        break;
                    }
                case UserTypesPortArrowGraphicsItem:
                    {
                        PortGraphicsItem* srcItem = selectedPortArrow_->getSourceItem();
                        PortGraphicsItem* dstItem = selectedPortArrow_->getDestinationItem();
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
        //adjustLinkArrowGraphicsItems();
        // make sure that the added processor is initialized
        evaluator_->initializeNetwork();
    }
    else {
        event->ignore();
    }
}

//---------------------------------------------------------------------------------------------------------------
//                  action slots
//---------------------------------------------------------------------------------------------------------------
bool NetworkEditor::clipboardHasValidContent() {
    QClipboard* clipboard = QApplication::clipboard();
    QString clip = clipboard->text();
    std::stringstream stream(clip.toStdString());

    XmlDeserializer d;
    d.setUseAttributes(true);
    try {
        d.read(stream);
    }
    catch (SerializationException&) {
        return false;
    }

    ProcessorNetwork* tmp = 0;
    try {
        d.deserialize("ProcessorNetwork", tmp);
    }
    catch (SerializationException& e){
        LWARNINGC("voreen.NetworkEditor", "Error during deserialization of temporary network:" << e.what());
        delete tmp;
        return false;
    }
    delete tmp;
    return true;
}

void NetworkEditor::deleteActionSlot() {
    removeItems(scene()->selectedItems());
    updateSelectedItems();

    emit processorsSelected(QList<Processor*>());
}

void NetworkEditor::copyActionSlot() {
    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so copying is not allowed"));
        return;
    }

    std::stringstream stream;

    XmlSerializer s;
    s.setUseAttributes(true);

    std::vector<Processor*> processors;

    foreach (QGraphicsItem* item, scene()->selectedItems()) {
        switch (item->type()) {
        case UserTypesProcessorGraphicsItem:
            {
                ProcessorGraphicsItem* procItem = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);
                Processor* processor = procItem->getProcessor();
                processors.push_back(processor);
                break;
            }
        /*case AggregationGraphicsItem::Type:
            {
                AggregationGraphicsItem* aggItem = qgraphicsitem_cast<AggregationGraphicsItem*>(item);
                std::vector<Processor*> aggProcs = qListToStdVector(aggItem->getProcessors());
                processors.insert(processors.end(), aggProcs.begin(), aggProcs.end());
                break;
            }*/
        }
    }

    ProcessorNetwork* copyNetwork = getProcessorNetwork()->cloneSubNetwork(processors);

    s.serialize("ProcessorNetwork",copyNetwork);

    s.write(stream);

    QString value = QString::fromStdString(stream.str());

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(value);

    delete copyNetwork;
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
    d.setUseAttributes(true);

    ProcessorNetwork* pasteNetwork; //= new ProcessorNetwork();

    try{
        d.read(stream);
        d.deserialize("ProcessorNetwork", pasteNetwork);
    }
    catch (SerializationException& e){
        LWARNING("Failed to deserialize network from clipboard: " << e.what());
        return;
    }

    PositionMetaData* pos;
    if (!dynamic_cast<PositionMetaData*>(pasteNetwork->getProcessors()[0]->getMetaDataContainer().getMetaData("ProcessorGraphicsItem"))){
        tgtAssert(false, "No Position MetaData");
    }
    else {
        pos = dynamic_cast<PositionMetaData*>(pasteNetwork->getProcessors()[0]->getMetaDataContainer().getMetaData("ProcessorGraphicsItem"));
    }

    int minX = pos->getX(); int maxX = pos->getX();
    int minY = pos->getY(); int maxY = pos->getY();
    for (size_t i = 1; i < pasteNetwork->getProcessors().size(); ++i){
        if (!dynamic_cast<PositionMetaData*>(pasteNetwork->getProcessors()[i]->getMetaDataContainer().getMetaData("ProcessorGraphicsItem"))){
            tgtAssert(false,"No Position MetaData");
        }
        else {
            pos = dynamic_cast<PositionMetaData*>(pasteNetwork->getProcessors()[i]->getMetaDataContainer().getMetaData("ProcessorGraphicsItem"));
        }

        if(pos->getX() < minX) minX = pos->getX();
        if(pos->getX() > maxX) maxX = pos->getX();
        if(pos->getY() < minY) minY = pos->getY();
        if(pos->getY() > maxY) maxY = pos->getY();
    }
    QPointF center((minX+maxX)/2,(minY+maxY)/2);
    center = rightClickPosition_-center;
    for (size_t i = 0; i < pasteNetwork->getProcessors().size(); ++i){
        if (!dynamic_cast<PositionMetaData*>(pasteNetwork->getProcessors()[i]->getMetaDataContainer().getMetaData("ProcessorGraphicsItem"))){
            tgtAssert(false, "No Position MetaData");
        }
        else {
            pos = dynamic_cast<PositionMetaData*>(pasteNetwork->getProcessors()[i]->getMetaDataContainer().getMetaData("ProcessorGraphicsItem"));
        }
        pos->setX(pos->getX()+static_cast<int>(center.rx()));
        pos->setY(pos->getY()+static_cast<int>(center.ry()));
    }

    getProcessorNetwork()->mergeSubNetwork(pasteNetwork);

    evaluator_->initializeNetwork();
}

void NetworkEditor::openPropertyLinkDialog(PortOwnerGraphicsItem* src, PortOwnerGraphicsItem* dest) {
    if (!getProcessorNetwork())
        return;

    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so creating links is not allowed"));
        return;
    }

    //switch all propertylists off
    QList<ProcessorGraphicsItem*> list;
    foreach(ProcessorGraphicsItem* item, processorItemMap_){
        if(item != src && item != dest && item->getPropertyList()->getIsVisibleInEditor()){
            list.append(item);
            item->togglePropertyList();
        }
    }

    PropertyLinkDialog* dialog = new PropertyLinkDialog(this, src, dest);
    dialog->exec(); //deletes dialog on return

    //switch lists back on
    foreach(ProcessorGraphicsItem* item, list)
        item->togglePropertyList();

    src->getPropertyList()->setIsVisibleInEditor(!src->getPropertyList()->getIsVisibleInEditor());
    src->togglePropertyList();
    if(src != dest){
        dest->getPropertyList()->setIsVisibleInEditor(!dest->getPropertyList()->getIsVisibleInEditor());
        dest->togglePropertyList();
    }
}

void NetworkEditor::editLinkActionSlot() {
    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so editing links is not allowed"));
        return;
    }

    tgtAssert(scene()->selectedItems().size() == 1,"to many items for this slot");
    tgtAssert(dynamic_cast<PropertyLinkArrowGraphicsItem*>(scene()->selectedItems().first()) ||
              dynamic_cast<PortSizeLinkArrowGraphicsItem*>(scene()->selectedItems().first()),"wrong type of selected item");

    PropertyLinkArrowGraphicsItem* plItem = 0;
    PortSizeLinkArrowGraphicsItem* pslItem = 0;
    if(plItem = dynamic_cast<PropertyLinkArrowGraphicsItem*>(scene()->selectedItems().first())){
        PortOwnerGraphicsItem* src = 0, *dst = 0;
        if(src = dynamic_cast<PortOwnerGraphicsItem*>(plItem->getSourceItem()->getPropertyOwnerItem())) {}
        else
            src = dynamic_cast<PortOwnerGraphicsItem*>(plItem->getSourceItem()->getPropertyOwnerItem()->parent());
        if(dst = dynamic_cast<PortOwnerGraphicsItem*>(plItem->getDestinationItem()->getPropertyOwnerItem())) {}
        else
            dst = dynamic_cast<PortOwnerGraphicsItem*>(plItem->getDestinationItem()->getPropertyOwnerItem()->parent());
        openPropertyLinkDialog(src,dst);
    }
    else if(pslItem = dynamic_cast<PortSizeLinkArrowGraphicsItem*>(scene()->selectedItems().first())) {
        openPropertyLinkDialog(pslItem->getSourcePort()->getPortOwner(),pslItem->getDestinationPort()->getPortOwner());
    }
}

void NetworkEditor::deleteLinksActionSlot() {
    if(!processorNetwork_) return;
    QList<ProcessorGraphicsItem*> processors = this->getSelectedProcessorGraphicsItems();

    int number = 0;
    if(processors.size() == 0) {//do it for all
        removeAllPropertyLinks();
    }
    else if(processors.size() == 1) {
        number = processorNetwork_->removePropertyLinksFromProcessor(processors.first()->getProcessor());
    }
    else {
        std::vector<Processor*> vec;
        foreach(ProcessorGraphicsItem* item, processors)
            vec.push_back(item->getProcessor());
        number = processorNetwork_->removePropertyLinksFromSubNetwork(vec);
    }

    if (number)
        LINFO("Removed " << number << " property links.");
}

void NetworkEditor::deleteCameraLinksActionSlot() {
    if(!processorNetwork_) return;
    QList<ProcessorGraphicsItem*> processors = this->getSelectedProcessorGraphicsItems();

    int number = 0;
    if (processors.size() == 0) //do it for all
        removeAllCameraLinks();
    else if(processors.size() == 1) {
        number = processorNetwork_->removePropertyLinksFromProcessor<CameraProperty>(processors.first()->getProcessor());
    }
    else {
        std::vector<Processor*> vec;
        foreach(ProcessorGraphicsItem* item, processors)
            vec.push_back(item->getProcessor());
       number = processorNetwork_->removePropertyLinksFromSubNetwork<CameraProperty>(vec);
    }

    if (number)
        LINFO("Removed " << number << " camera property links.");
}

void NetworkEditor::deletePortSizeLinksActionSlot() {
    if(!processorNetwork_) return;
    QList<ProcessorGraphicsItem*> processors = this->getSelectedProcessorGraphicsItems();

    int number = 0;
    if (processors.size() == 0) //do it for all
        removeAllPortSizeLinks();
    else if (processors.size() == 1){
        number = processorNetwork_->removeRenderSizeLinksFromProcessor(processors.first()->getProcessor());
    }
    else {
        std::vector<Processor*> vec;
        foreach(ProcessorGraphicsItem* item, processors)
            vec.push_back(item->getProcessor());
        number = processorNetwork_->removeRenderSizeLinksFromSubNetwork(vec);
    }

    if (number)
        LINFO("Removed " << number << " render size links.");
}

void NetworkEditor::createCameraLinksActionSlot() {
    if(!processorNetwork_) return;
    QList<ProcessorGraphicsItem*> processors = this->getSelectedProcessorGraphicsItems();

    int number = 0;
    if (processors.size() == 0) //do it for all
        number = processorNetwork_->createPropertyLinksWithinSubNetwork<CameraProperty>(processorNetwork_->getProcessors(),std::vector<std::string>());
    else if (processors.size() == 1) {
        number = processorNetwork_->createPropertyLinksForProcessor<CameraProperty>(processors.first()->getProcessor());
    }
    else {
        std::vector<Processor*> vec;
        foreach(ProcessorGraphicsItem* item, processors)
            vec.push_back(item->getProcessor());
        number = processorNetwork_->createPropertyLinksWithinSubNetwork<CameraProperty>(vec, std::vector<std::string>());
    }

    if (number)
        LINFO("Created " << number << " camera property links.");
}

void NetworkEditor::createPortSizeLinksActionSlot() {
    if(!processorNetwork_) return;
    QList<ProcessorGraphicsItem*> processors = this->getSelectedProcessorGraphicsItems();

    int number = 0;
    if(processors.size() == 0)//do it for all
        number = processorNetwork_->createRenderSizeLinksWithinSubNetwork(processorNetwork_->getProcessors());
    else if (processors.size() == 1) {
        number = processorNetwork_->createRenderSizeLinksForProcessor(processors.first()->getProcessor());
    }
    else {
        std::vector<Processor*> vec;
        foreach(ProcessorGraphicsItem* item, processors)
            vec.push_back(item->getProcessor());
        number = processorNetwork_->createRenderSizeLinksWithinSubNetwork(vec);
    }

    if (number)
        LINFO("Created " << number << " render size links.");
}

void NetworkEditor::aggregateActionSlot() {
    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so aggregating is not allowed"));
        return;
    }

    if (QMessageBox::warning(this, tr("Warning"), QString::fromStdString("<b>You are trying to aggregate network editor items. </b><br> <br>"\
               "This feature has not been tested in this version. The use can cause nondeterministic behavior. <br><br> "\
               "<b>Are you sure you still want to aggregate these items?</b>"), QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Cancel) {
                return;
    }

    QList<PortOwnerGraphicsItem*> items = getSelectedPortOwnerGraphicsItems();

    createAggregationGraphicsItem(items);
}

void NetworkEditor::deaggregateActionSlot() {
    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so deaggregating is not allowed"));
        return;
    }

    if (QMessageBox::warning(this, tr("Warning"), QString::fromStdString("<b>You are trying to deaggregate network editor items. </b><br> <br>"\
               "This feature has not been tested in this version. The use can cause nondeterministic behavior. <br><br> "\
               "<b>Are you sure you still want to deaggregate these items?</b>"), QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Cancel) {
                return;
    }

    QList<AggregationGraphicsItem*> items = getSelectedAggregationGraphicsItems();

    deaggregateItems(items);
}

void NetworkEditor::deaggregateItems(const QList<AggregationGraphicsItem*>& items) {

    if (!getProcessorNetwork())
        return;

    foreach (AggregationGraphicsItem* aggregationItem, items) {
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
        QList<PortOwnerGraphicsItem*> childItems = aggregationItem->getContainingItems();

        foreach (PortOwnerGraphicsItem* childItem, childItems) {

            switch (childItem->type()) {
            case UserTypesProcessorGraphicsItem:
                {
                    ProcessorGraphicsItem* procItem = qgraphicsitem_cast<ProcessorGraphicsItem*>(childItem);
                    Processor* processor = procItem->getProcessor();
                    tgtAssert(processor, "cannot add null pointer to processor");
                    //processor->setProgressBar(procItem->getProgressBar());
                    break;
                }
            case UserTypesAggregationGraphicsItem:
                {
                     AggregationGraphicsItem* aggItem = qgraphicsitem_cast<AggregationGraphicsItem*>(childItem);
                     aggregationItems_.append(aggItem);
                     break;
                }
            default:
                tgtAssert(false, "shouldn't get here");
            }
        }


        foreach (PortOwnerGraphicsItem* childItem, childItems) {
            childItem->layoutChildItems();
        }

        //scene()->removeItem(aggregationItem);
        aggregationItems_.removeOne(aggregationItem);

        setUpdatesEnabled(false);
        aggregationItem->prepareDeaggregation();
        delete aggregationItem;
        setUpdatesEnabled(true);

        scene()->clearSelection();
    }
}

void NetworkEditor::contextMenuEvent(QContextMenuEvent* event) {
    //create new menu
    QMenu currentMenu;
    event->accept();

    //empty action
    QAction* emptyAction = new QAction(QString("no action"),&currentMenu);
    emptyAction->setEnabled(false);

    QList<QGraphicsItem*> selectedItems = scene()->selectedItems();

    //HACK to handle right click, while an arrow moved
    if(selectedItems.size() == 1 && dynamic_cast<PortGraphicsItem*>(selectedItems.first())){
        scene()->clearSelection(); //still needs improvement :(
        selectedItems.clear();
    }

    QGraphicsItem* item = itemAt(event->pos());

    //move to parent item
    if (item) {
        switch (item->type()) {
        case UserTypesRenamableTextGraphicsItem:
        case UserTypesPropertyListButtonGraphicsItem:
            item = item->parentItem();
        }
    }

    //action belonging to just one item
    if((item && item->isSelected() && scene()->selectedItems().size() == 1) ||
       (item && !item->isSelected() && item->acceptHoverEvents())) {
        scene()->clearSelection();
        selectedItems.clear();
        item->setSelected(true);
        selectedItems.append(item);

        switch (item->type()) {
        case UserTypesProcessorGraphicsItem: {
        case UserTypesAggregationGraphicsItem:
            dynamic_cast<NWEBaseGraphicsItem*>(item)->addActionsToContextMenu(&currentMenu);

            currentMenu.addSeparator();
            currentMenu.addAction(createPortOwnerCameraLinksAction_);
            currentMenu.addAction(createPortOwnerPortSizeLinksAction_);
            currentMenu.addAction(deletePortOwnerLinksAction_);
            currentMenu.addAction(deletePortOwnerCameraLinksAction_);
            currentMenu.addAction(deletePortOwnerPortSizeLinksAction_);

            if(item->type() == UserTypesAggregationGraphicsItem) {
                currentMenu.addSeparator();
                currentMenu.addAction(deaggregateAction_);
            }

            currentMenu.addSeparator();
            currentMenu.addAction(deleteAction_);
            currentMenu.addAction(copyAction_);

            PortOwnerGraphicsItem* poItem = dynamic_cast<PortOwnerGraphicsItem*>(item);
            if(!poItem->getWidgetToggleButton().getWidgets().empty()) {
                currentMenu.addSeparator();
                poItem->getWidgetToggleButton().addActionsToContextMenu(&currentMenu);
            }

            } break;
        case UserTypesPortArrowGraphicsItem:
            {
            /*bool isPartOfBundle = false;
            QList<PortArrowGraphicsItem*> items = convertQList<QGraphicsItem*, PortArrowGraphicsItem*>(selectedItems);
            foreach(PortArrowGraphicsItem* arrow, items) {
                if(arrow->isBundled()) {
                    isPartOfBundle = true;
                    break;
                }
            }
            if(isPartOfBundle) {
                currentMenu.addAction(unbundleAction_);
                currentMenu.addAction(addHandleAction_);
            }*/
            currentMenu.addAction(deleteAction_);
            break;
            }
        case UserTypesPropertyLinkArrowGraphicsItem:
            {
            currentMenu.addAction(deleteAction_);
            currentMenu.addAction(editLinkAction_);
            break;
            }
        case UserTypesPortSizeLinkArrowGraphicsItem:
            {
            currentMenu.addAction(deleteAction_);
            currentMenu.addAction(editLinkAction_);
            break;
            }
        case UserTypesWidgetToggleButtonGraphicsItem:
            {
            WidgetToggleButtonGraphicsItem* wtb = dynamic_cast<WidgetToggleButtonGraphicsItem*>(item);
            if(!wtb->getWidgets().empty())
                wtb->addActionsToContextMenu(&currentMenu);
            break;
            }
        case UserTypesPortGraphicsItem:
            {
            qgraphicsitem_cast<PortGraphicsItem*>(item)->addActionsToContextMenu(&currentMenu);
            if(currentMenu.actions().empty())
                currentMenu.addAction(emptyAction);
            break;
            }
        default:
            return;
        }
    } else
    //action with multiple items
    if(item && item->isSelected()) {
        QList<PortOwnerGraphicsItem*> list = getSelectedPortOwnerGraphicsItems();
        if(!list.empty()){
            if(list.size() == 1) {
                currentMenu.addAction(createPortOwnerCameraLinksAction_);
                currentMenu.addAction(createPortOwnerPortSizeLinksAction_);
                currentMenu.addAction(deletePortOwnerLinksAction_);
                currentMenu.addAction(deletePortOwnerCameraLinksAction_);
                currentMenu.addAction(deletePortOwnerPortSizeLinksAction_);
            } else
            if(list.size() > 1) {
                currentMenu.addAction(createInnerCameraLinksAction_);
                currentMenu.addAction(createInnerPortSizeLinksAction_);
                currentMenu.addAction(deleteInnerLinksAction_);
                currentMenu.addAction(deleteInnerCameraLinksAction_);
                currentMenu.addAction(deleteInnerPortSizeLinksAction_);
                currentMenu.addSeparator();
                currentMenu.addAction(sortSubNetworkAction_);
                currentMenu.addSeparator();
                currentMenu.addAction(aggregateAction_);
            }
        }

        if(!getSelectedAggregationGraphicsItems().empty()) {
            currentMenu.addAction(deaggregateAction_);
        }

        currentMenu.addSeparator();
        currentMenu.addAction(deleteAction_);
        currentMenu.addAction(copyAction_);

    }
    // no items have been hit
    else {
        scene()->clearSelection();
        currentMenu.addAction(deleteAllLinksAction_);
        currentMenu.addAction(deleteAllCameraLinksAction_);
        currentMenu.addAction(deleteAllPortSizeLinksAction_);
        currentMenu.addAction(createAllCameraLinksAction_);
        currentMenu.addAction(createAllPortSizeLinksAction_);

        if(clipboardHasValidContent()){
            currentMenu.addSeparator();
            rightClickPosition_ = mapToScene(mapFromGlobal(QCursor::pos()));
            currentMenu.addAction(pasteAction_);
        }
    }
    currentMenu.exec(event->globalPos());
}







/***********************************************************************************************
 ***********************************************************************************************
 ***                                                                                         ***
 ***                              NetworkScreenshotPlugin                                    ***
 ***                                                                                         ***
 ***********************************************************************************************
 ***********************************************************************************************/
NetworkScreenshotPlugin::NetworkScreenshotPlugin(QWidget* parent, NetworkEditor* networkEditorWidget)
    : ScreenshotPlugin(parent, 0)
    , networkEditorWidget_(networkEditorWidget)
{
    setWindowTitle(tr("Network Screenshot"));
    resolutions_.push_back("native");
    sizeCombo_->insertItem(0,"native");
    int index = sizeCombo_->findText("native");
    if ( index != -1 ) { // -1 for not found
        sizeCombo_->setCurrentIndex(index);
    }
}

void NetworkScreenshotPlugin::saveScreenshot(const QString& filename) {
    QRectF visibleRect;
    foreach (QGraphicsItem* item, networkEditorWidget_->scene()->items()) {
        if (item->isVisible()) {
            QRectF iRect = item->mapRectToScene(item->boundingRect());
            visibleRect = visibleRect.united(iRect);
        }
    }
    saveScreenshot(filename, visibleRect.width(), visibleRect.height());
}

void NetworkScreenshotPlugin::sizeComboChanged(int index) {
    spWidth_->blockSignals(true);
    spHeight_->blockSignals(true);
    ScreenshotPlugin::sizeComboChanged(index);
    if(sizeCombo_->currentText().contains("native")) {

        QRectF visibleRect;
        foreach (QGraphicsItem* item, networkEditorWidget_->scene()->items()) {
            if (item->isVisible()) {
                QRectF iRect = item->mapRectToScene(item->boundingRect());
                visibleRect = visibleRect.united(iRect);
            }
        }
        spWidth_->setValue(visibleRect.width());
        spHeight_->setValue(visibleRect.height());

        spWidth_->setEnabled(false);
        spHeight_->setEnabled(false);
    }

    spWidth_->blockSignals(false);
    spHeight_->blockSignals(false);

}

void NetworkScreenshotPlugin::saveScreenshot(const QString& filename, int width, int height) {
    if (!networkEditorWidget_)
        return;

    QRectF visibleRect;
    foreach (QGraphicsItem* item, networkEditorWidget_->scene()->items()) {
        if (item->isVisible()) {
            QRectF iRect = item->mapRectToScene(item->boundingRect());
            visibleRect = visibleRect.united(iRect);
        }
    }

    QPixmap pixmap(width, height);
    pixmap.fill(QColor(255, 255, 255, 0));

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    networkEditorWidget_->scene()->render(&painter,QRectF(),visibleRect);

    pixmap.save(filename);
}

} // namespace voreen



/*
    /*

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

*/

//} // namespace




/*














void NetworkEditor::bundleLinksSlot() {
    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so link bundling is not allowed"));
        return;
    }

    QList<PortArrowGraphicsItem*> items = convertQList<QGraphicsItem*, PortArrowGraphicsItem*>(scene()->selectedItems());
    bundleLinks(items);
    updateBundleMetaData();
}

void NetworkEditor::unbundleLinksSlot() {
    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so link unbundling is not allowed"));
        return;
    }

    QList<PortArrowGraphicsItem*> items = convertQList<QGraphicsItem*, PortArrowGraphicsItem*>(scene()->selectedItems());
    if(items.empty())
        return;

    QList<ConnectionBundle*> bundlesToUnbundle;
    foreach(PortArrowGraphicsItem* arrow, items) {
        if(!arrow->isBundled())
            continue;

        ConnectionBundle* bundle = bundleMap_.value(arrow);
        tgtAssert(bundle, "Bundle not present in map");

        if(bundlesToUnbundle.contains(bundle))
            continue;

        bundlesToUnbundle.append(bundle);
    }

    foreach(ConnectionBundle* bundle, bundlesToUnbundle) {
        foreach(PortArrowGraphicsItem* arrow, (*bundle).arrowList_) {
            arrow->setBundleInfo(false);
            bundleMap_.remove(arrow);
        }
        bundles_.removeAll(bundle);
        delete bundle;
    }

    updateBundleMetaData();
    scene()->update();
}

void NetworkEditor::addHandleSlot() {
    if (evaluator_->isLocked() && !networkEvaluatorIsLockedByButton_) {
        QMessageBox::information(this, tr("Network Locked"), tr("The network is being evaluated, so link unbundling is not allowed"));
        return;
    }

    QList<PortArrowGraphicsItem*> items = convertQList<QGraphicsItem*, PortArrowGraphicsItem*>(scene()->selectedItems());
    if(items.empty())
        return;

    ConnectionBundle* targetBundle = 0;

    //QList<ConnectionBundle*> bundlesToUnbundle;
    foreach(PortArrowGraphicsItem* arrow, items) {
        if(!arrow->isBundled())
            continue;

        targetBundle = bundleMap_.value(arrow);
        break;
    }

    tgtAssert(targetBundle, "Bundle not present in map");
    targetBundle->addHandle(rightClickPosition_);

    updateBundleMetaData();
    scene()->update();
}




// ------------------------------------------------------------------------------------------------
// aggregation methods
// ------------------------------------------------------------------------------------------------



void exchangePortsInConnection(PortGraphicsItem* originalPort, PortGraphicsItem* newPort, PortGraphicsItem* counterPort) {
    originalPort->removeConnection(counterPort);
    newPort->addConnection(counterPort);
    counterPort->removeConnection(originalPort);
    counterPort->addConnection(newPort);
}



ConnectionBundle* NetworkEditor::bundleLinks(const QList<PortArrowGraphicsItem*>& items) {

    if (!getProcessorNetwork())
        return 0;

    ConnectionBundle* newBundle = new ConnectionBundle(items, this);
    bundles_.append(newBundle);
    foreach(PortArrowGraphicsItem* arrow, items)
        bundleMap_.insert(arrow, newBundle);
    //newBundle->setBundlePointsDefault();
    return newBundle;
}

void NetworkEditor::updateBundleMetaData() {
    MetaDataContainer& container = getProcessorNetwork()->getMetaDataContainer();
    container.removeMetaData("Bundles");

    container.addMetaData("Bundles", new SelectionMetaData<ConnectionBundleMetaData>());
    SelectionMetaData<ConnectionBundleMetaData>* metaData = static_cast<SelectionMetaData<ConnectionBundleMetaData>* >(container.getMetaData("Bundles"));

    foreach(ConnectionBundle* bundle, bundles_)
        metaData->addValue(ConnectionBundleMetaData(bundle));
}

void NetworkEditor::readBundlesFromMetaData() {
    bundles_.clear();
    bundleMap_.clear();

    SelectionMetaData<ConnectionBundleMetaData>* bundleMetaData;
    bundleMetaData = dynamic_cast<SelectionMetaData<ConnectionBundleMetaData>* >(getProcessorNetwork()->getMetaDataContainer().getMetaData("Bundles"));
    if(!bundleMetaData)
        return;

    std::vector<ConnectionBundleMetaData> bundles = bundleMetaData->getValues();
    for(size_t i = 0; i < bundles.size(); i++) {
        QList<PortArrowGraphicsItem*> bundleArrows;
        for(size_t j = 0; j < bundles.at(i).getConnections().size(); j++) {
            QList<PortArrowGraphicsItem*> arrowList = getPortGraphicsItem(bundles.at(i).getConnections().at(j).getOutport())->getArrowList();
            foreach (PortArrowGraphicsItem* arrow, arrowList) {
                if(arrow->getDestinationItem()->getPort() == bundles.at(i).getConnections().at(j).getInport()) {
                    bundleArrows.append(arrow);
                }
            }
        }
        ConnectionBundle* newBundle = bundleLinks(bundleArrows);
        if(!newBundle)
            return;
        std::vector<tgt::vec2> bundlePoints = bundles.at(i).getBundlePoints();
        if(bundlePoints.size() < 2)
            return;
        newBundle->startHandle_->setPos(bundlePoints.front().x, bundlePoints.front().y);
        newBundle->startHandle_->setDetached(bundles.at(i).isStartDetached());
        for(size_t j = 1; j < bundlePoints.size() - 1; j++)
            newBundle->addHandle(QPointF(bundlePoints.at(j).x, bundlePoints.at(j).y));
        newBundle->endHandle_->setPos(bundlePoints.back().x, bundlePoints.back().y);
        newBundle->endHandle_->setDetached(bundles.at(i).isEndDetached());
    }
}

void NetworkEditor::updateCurrentBundles() {
    QList<QGraphicsItem*> selectedItems = scene()->selectedItems();
    foreach (QGraphicsItem* it, selectedItems) {
        if(RootGraphicsItem* rgrit = dynamic_cast<RootGraphicsItem*>(it)) {
            QList<PortGraphicsItem*> pgis = rgrit->getPortGraphicsItems();
            foreach(PortGraphicsItem* pgi, pgis) {
                if(pgi->isOutport()) {
                    QList<PortArrowGraphicsItem*> pagis = pgi->getArrowList();
                    foreach(PortArrowGraphicsItem* pagi, pagis) {
                        if(bundleMap_.contains(pagi))
                            bundleMap_.value(pagi)->setBundlePointsDefault();
                    }
                } else {
                    QList<PortGraphicsItem*> pgiIns = pgi->getConnectedPorts();
                    foreach(PortGraphicsItem* pgiIn, pgiIns) {
                        QList<PortArrowGraphicsItem*> pagis = pgiIn->getArrowList();
                        foreach(PortArrowGraphicsItem* pagi, pagis) {
                            if(bundleMap_.contains(pagi) && pagi->getDestinationItem() == pgi)
                                bundleMap_.value(pagi)->setBundlePointsDefault();
                        }
                    }
                }
            }
        } else if(dynamic_cast<ConnectionBundleHandle*>(it)) {
            updateBundleMetaData();
        }
    }
}

void NetworkEditor::removeConnectionFromBundles(const Port* outport, const Port* inport) {
    const QList<PortArrowGraphicsItem*>& arrowList = getPortGraphicsItem(outport)->getArrowList();
    foreach (PortArrowGraphicsItem* arrow, arrowList) {

        if(arrow->getDestinationItem() && arrow->getDestinationItem()->getPort() == inport && bundleMap_.contains(arrow)) {
            ConnectionBundle* curBundle = bundleMap_.value(arrow);
            curBundle->arrowList_.removeAll(arrow);
            if(curBundle->arrowList_.size() <= 1) {
                if(curBundle->arrowList_.size() == 1)
                    curBundle->arrowList_.front()->setBundleInfo(false);
                bundles_.removeAll(curBundle);
                delete curBundle;
                bundleMap_.remove(arrow);
                updateBundleMetaData();
            }
            else
                bundleMap_[arrow]->setBundlePointsDefault();
        }
    }
}

*/






