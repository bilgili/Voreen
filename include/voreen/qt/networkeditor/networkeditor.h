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

#ifndef VRN_NETWORKEDITOR_H
#define VRN_NETWORKEDITOR_H

#include "voreen/core/network/processornetwork.h"
#include "voreen/qt/widgets/screenshotplugin.h"
#include "voreen/qt/voreenqtapi.h"

#include <QGraphicsView>
#include <QMap>

class QToolButton;
class QAction;

namespace voreen {
//voreen core
class NetworkEditor;
class NetworkEvaluator;
class AggregationMetaData;
//style
class NWEStyle_Base;
//graph layouts
class NWEGL_Base;
//graphic items
class AggregationGraphicsItem;
class ProcessorGraphicsItem;
class PropertyGraphicsItem;
class PortGraphicsItem;
class PortOwnerGraphicsItem;
class PortArrowGraphicsItem;
class PropertyLinkArrowGraphicsItem;
class PortOwnerLinkArrowGraphicsItem;
class PortSizeLinkArrowGraphicsItem;

/// Editor Layers
enum NetworkEditorLayer {
    NetworkEditorLayerUndefined,
    NetworkEditorLayerDataFlow,
    NetworkEditorLayerLinking,
    NetworkEditorLayerGeneralLinking,
    NetworkEditorLayerCameraLinking,
    NetworkEditorLayerPortSizeLinking
};
enum NetworkEditorCursorMode {
    NetworkEditorCursorSelectMode,
    NetworkEditorCursorMoveMode
};


class VRN_QT_API NetworkEditor : public QGraphicsView, public ProcessorNetworkObserver {
Q_OBJECT
public:
    NetworkEditor(QWidget* parent = 0, ProcessorNetwork* workspace = 0, NetworkEvaluator* evaluator = 0);
    ~NetworkEditor();

//---------------------------------------------------------------------------------------------------------------
//                  general members and functions
//---------------------------------------------------------------------------------------------------------------
protected:
    ProcessorNetwork* processorNetwork_;    /// network
    NetworkEvaluator* evaluator_;           /// evaluator

    /// connection between processors and their graphicitems
    QMap<Processor*,ProcessorGraphicsItem*> processorItemMap_;
    QList<AggregationGraphicsItem*> aggregationItems_;

    static const std::string loggerCat_;    /// loggarcat

public:
    ProcessorNetwork* getProcessorNetwork() const;
    void setProcessorNetwork(ProcessorNetwork* network);

//---------------------------------------------------------------------------------------------------------------
//                  ProcessorNetworkObserver functions
//---------------------------------------------------------------------------------------------------------------
public:
    void networkChanged();
    void processorAdded(const Processor* processor);
    void processorRemoved(const Processor* processor);
    void propertyLinkAdded(const PropertyLink* link);
    void propertyLinkRemoved(const PropertyLink* link);
    void processorRenamed(const Processor* processor, const std::string& prevName);
    void portConnectionAdded(const Port* outport, const Port* inport);
    void portConnectionRemoved(const Port* outport, const Port* inport);

//---------------------------------------------------------------------------------------------------------------
//                  scene transformations
//---------------------------------------------------------------------------------------------------------------
private:
    bool needsScale_; /// true if scene has to be scaled
public:
    void scale(qreal sx, qreal sy);
    void translate(qreal dx, qreal dy);
    QSize sizeHint() const;
protected:
    void scaleView();

//---------------------------------------------------------------------------------------------------------------
//                  create and handle graphicitems
//---------------------------------------------------------------------------------------------------------------
public:
    ProcessorGraphicsItem* getProcessorGraphicsItem(const Processor* processor) const;
    void selectPreviouslySelectedProcessors();

protected:
        //getter
    QList<AggregationGraphicsItem*> getSelectedAggregationGraphicsItems();
    QList<PortOwnerGraphicsItem*> getSelectedPortOwnerGraphicsItems();
    QList<ProcessorGraphicsItem*> getSelectedProcessorGraphicsItems();
    PortGraphicsItem* getPortGraphicsItem(const Port* port) const;
    PropertyGraphicsItem* getPropertyGraphicsItem(const Property* prop) const;
        //create
    void generateGraphicsItems();
    ProcessorGraphicsItem* createProcessorGraphicsItem(Processor* processor);
    AggregationGraphicsItem* createAggregationGraphicsItem(AggregationMetaData* metaData);
    AggregationGraphicsItem* createAggregationGraphicsItem(const QList<PortOwnerGraphicsItem*>& items);
    void createLinkArrowForPropertyLink(const PropertyLink* link);
        //delete
    void resetScene();
    void removeItems(QList<QGraphicsItem*> items);
    void removeItemsInDataFlow(QList<QGraphicsItem*> items);
    void removeItemsInGeneralLinking(QList<QGraphicsItem*> items);
    void removeItemsInCameraLinking(QList<QGraphicsItem*> items);
    void removeItemsInPortSizeLinking(QList<QGraphicsItem*> items);
    void removePortOwnerGraphicsItem(PortOwnerGraphicsItem* poItem);
    void removePortArrowGraphicsItem(PortArrowGraphicsItem* arrow);
    void removePropertyLinkArrowGraphicsItem(PropertyLinkArrowGraphicsItem* arrow);
    void removePortSizeLinkArrowGraphicsItem(PortSizeLinkArrowGraphicsItem* arrow);
    void removePortOwnerLinkArrowGraphicsItem(PortOwnerLinkArrowGraphicsItem* arrow);
private slots:
    void updateSelectedItems();

signals:
    void processorsSelected(const QList<Processor*>& processors);

//---------------------------------------------------------------------------------------------------------------
//                  style, layer and cursor management
//---------------------------------------------------------------------------------------------------------------
private:
    NetworkEditorLayer currentLayer_;
    NetworkEditorCursorMode currentCursorMode_;
    NWEStyle_Base* currentStyle_;
    NWEGL_Base* currentGraphLayout_;
    bool currentToolTipMode_;

public:
    NetworkEditorLayer getCurrentLayer() const;
    NetworkEditorCursorMode getCurrentCursorMode() const;
    NWEStyle_Base* getCurrentStyle() const;
    bool getCurrentToolTipMode() const;
protected:
    void setLayer(NetworkEditorLayer layer);
    void setCursorMode(NetworkEditorCursorMode mode);
    void setStyle(NWEStyle_Base* style);
        void styleOnChange();
        void updateGraphLayout();
    void processorFontOnChange();
    void setToolTipMode(bool mode);
public slots:
    void toggleToolTip();
//---------------------------------------------------------------------------------------------------------------
//                  button management
//---------------------------------------------------------------------------------------------------------------
private:
    QWidget* layerButtonContainer_;
        QToolButton* dataFlowLayerButton_;
        QToolButton* linkingLayerButton_;
    QWidget* linkingLayerButtonContainer_;
        QToolButton* generalLinkingButton_;
        QToolButton* cameraLinkingButton_;
        QToolButton* portSizeLinkingButton_;
    QWidget* generalLinkingLayerButtonContainer_;
        QToolButton* hideCameraLinksButton_;
        QToolButton* hidePortSizeLinksButton_;
        QToolButton* removeAllPropertyLinksButton_;
    QWidget* cameraLinkingLayerButtonContainer_;
        QToolButton* linkCamerasAutoButton_;
        QToolButton* linkCamerasButton_;
        QToolButton* removeAllCameraLinksButton_;
    QWidget* portSizeLinkingLayerButtonContainer_;
        QToolButton* linkPortSizeAutoButton_;
        QToolButton* linkPortSizeButton_;
        QToolButton* removeAllPortSizeLinksButton_;
    QWidget* stopButtonContainer_;
        QToolButton* stopNetworkEvaluatorButton_;
        bool networkEvaluatorIsLockedByButton_;
    QWidget* navigationButtonContainer_;
        QToolButton* selectCursorButton_;
        QToolButton* moveCursorButton_;
    QWidget* layoutButtonContainer_;
        QToolButton* centerViewButton_;
        QToolButton* graphLayoutButton_;

public:
    bool cameraLinksHidden();
    bool portSizeLinksHidden();
protected:
    void initilizeEditorButtons();
    void layoutEditorButtons();
public slots:
    void setLayerToDataFlow();
    void setLayerToLinking();
    void setLayerToGeneralLinking();
    void setLayerToCameraLinking();
    void setLayerToPortSizeLinking();
private slots:
    void toggleNetworkEvaluator();
    void hideCameraLinks();
    void hidePortSizeLinks();
    void removeAllPropertyLinks();
    void linkCameras();
    void linkCamerasAutoChanged();
    void removeAllCameraLinks();
    void linkPortSize();
    void linkPortSizeAutoChanged();
    void removeAllPortSizeLinks();
    void sortEntireNetwork();
    void sortSubNetwork();
    void setViewCenter();
    void setCursorSelect();
    void setCursorMove();
    void linkCamerasOfProcessor(const Processor* processor);

//---------------------------------------------------------------------------------------------------------------
//                  events
//---------------------------------------------------------------------------------------------------------------
protected:
    //resize
    void resizeEvent(QResizeEvent* event);
    //wheel
    void wheelEvent(QWheelEvent* event);
    //mouse
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
        //members
        bool translateScene_; /// true, if we move scene by mouse
        QPointF translateSceneVector_; // translation vector for scene movement
        QPointF lastTranslateCenter_;
        QPointF rightClickPosition_;
    //key
    void keyPressEvent(QKeyEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
        //members
        PortArrowGraphicsItem* selectedPortArrow_;

//---------------------------------------------------------------------------------------------------------------
//                  action slots and context menu
//---------------------------------------------------------------------------------------------------------------
protected:
    void createContextMenuActions();
    void contextMenuEvent(QContextMenuEvent* event);

    QAction* copyAction_;
    QAction* pasteAction_;
    QAction* deleteAction_;
    QAction* editLinkAction_;
    QAction* deleteAllLinksAction_;
    QAction* deleteAllCameraLinksAction_;
    QAction* deleteAllPortSizeLinksAction_;
    QAction* createAllCameraLinksAction_;
    QAction* createAllPortSizeLinksAction_;
    QAction* deleteInnerLinksAction_;
    QAction* deleteInnerCameraLinksAction_;
    QAction* deleteInnerPortSizeLinksAction_;
    QAction* createInnerCameraLinksAction_;
    QAction* createInnerPortSizeLinksAction_;
    QAction* deletePortOwnerLinksAction_;
    QAction* deletePortOwnerCameraLinksAction_;
    QAction* deletePortOwnerPortSizeLinksAction_;
    QAction* createPortOwnerCameraLinksAction_;
    QAction* createPortOwnerPortSizeLinksAction_;
    QAction* aggregateAction_;
    QAction* deaggregateAction_;
    QAction* sortSubNetworkAction_;
    //QAction* bundleAction_;
    //QAction* unbundleAction_;
    //QAction* addHandleAction_;

private slots:
    void copyActionSlot();
    void pasteActionSlot();
        bool clipboardHasValidContent();
    void deleteActionSlot();
    void editLinkActionSlot();
    void deleteLinksActionSlot();
    void deleteCameraLinksActionSlot();
    void deletePortSizeLinksActionSlot();
    void createCameraLinksActionSlot();
    void createPortSizeLinksActionSlot();
    void aggregateActionSlot();
    void deaggregateActionSlot();
        void deaggregateItems(const QList<AggregationGraphicsItem*>& items);

    void openPropertyLinkDialog(PortOwnerGraphicsItem* src, PortOwnerGraphicsItem* dest);
};


/**
 * Widget for taking screenshots of the network graph
 */
class VRN_QT_API NetworkScreenshotPlugin : public ScreenshotPlugin {
    Q_OBJECT
public:
    NetworkScreenshotPlugin(QWidget* parent, NetworkEditor* networkEditorWidget);

public slots:
    void sizeComboChanged(int);

protected:
    void saveScreenshot(const QString& filename);
    void saveScreenshot(const QString& filename, int width, int height);

    NetworkEditor* networkEditorWidget_;
};




/*    ProcessorNetwork* getProcessorNetwork();

    ProcessorGraphicsItem* getProcessorGraphicsItem(Processor* processor);


public slots:
    void setProcessorNetwork(ProcessorNetwork* network);

    /**
     * Informs the NetworkEvalutor held by this object about changes of any
     * of the processor connections. This method is intended to be called
     * by ProcessorGraphicsItem objects whenever connections between their
     * ports are made.
     */
    /*void processorConnectionsChanged();

    void adjustLinkArrowGraphicsItems();

    void updateCurrentBundles();


private slots:



    void createLink(RootGraphicsItem* src, RootGraphicsItem* dest);
    void createNewLink(const Property* sourceProp, const Property* destinationProp, LinkEvaluatorBase* linkEvaluator);
    void editPropertyLink(const Property* sourceProp, const Property* destinationProp, LinkEvaluatorBase* linkEvaluator);
    void removePropertyLink(PropertyLink* propertyLink);

    /// Links the processor's cameras with the cameras of all other processors in the network
    void linkCamerasOfProcessor(const Processor* processor);

protected:
    LinkArrowGraphicsItem* createLinkArrowForPropertyLink(const PropertyLink* link);

    AggregationGraphicsItem* aggregateItems(const QList<RootGraphicsItem*>& items);
    void deaggregateItems(const QList<AggregationGraphicsItem*>& items);


    void removeArrowItem(LinkArrowGraphicsItem* arrow);
    ConnectionBundle* bundleLinks(const QList<PortArrowGraphicsItem*>& items);
    void removeConnectionFromBundles(const Port* outport, const Port* inport);
    void updateBundleMetaData();
    void readBundlesFromMetaData();



    QList<ProcessorGraphicsItem*> selectedProcessorItems() const;

    void hideTooltip();


    void contextMenuEvent(QContextMenuEvent* event);

    void showTooltip(const QPoint& pos, HasToolTip* toolTip);

    void showLinkDialog(PropertyGraphicsItem* propertyItem1, PropertyGraphicsItem* propertyItem2);

    //void clearClipboard();

    PortGraphicsItem* getPortGraphicsItem(const Port* port) const;



private:
    typedef std::pair<const PropertyLink*, const PropertyLink*> ArrowLinkInformation;

    void centerView();



    QMenu contextMenuNone_;
    QMenu contextMenuSingle_;
    QMenu contextMenuMultiple_;
    QMenu contextMenuLink_;
    QPointF rightClickPosition_;



    QMap<LinkArrowGraphicsItem*, ArrowLinkInformation> linkMap_;

    PortArrowGraphicsItem* selectedPortArrow_;
    LinkArrowGraphicsItem* selectedLinkArrow_;

    QImage* temporaryRenderPortImage_;

    // Maps from the processors of the current network to their graphic items.
    QMap<PortArrowGraphicsItem*, ConnectionBundle* > bundleMap_;
    QList<ConnectionBundle*> bundles_;

    // construed as translation vector
    QPointF sceneTranslate_;
*/

} // namespace voreen

#endif // VRN_NETWORKEDITOR_H
