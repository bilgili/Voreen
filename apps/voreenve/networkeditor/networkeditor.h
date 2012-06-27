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

#ifndef VRN_NETWORKEDITOR_H
#define VRN_NETWORKEDITOR_H

#include "voreen/core/network/processornetwork.h"
#include "voreen/qt/widgets/snapshotplugin.h"

#include <QGraphicsView>
#include <QMenu>
#include <map>
#include <vector>

class QToolButton;
class QTransform;

namespace voreen {

class AggregationGraphicsItem;
class AggregationMetaData;
class HasToolTip;
class LinkArrowGraphicsItem;
class LinkEvaluatorBase;
class NetworkEvaluator;
class PortArrowGraphicsItem;
class ProcessorGraphicsItem;
class PropertyGraphicsItem;
class RootGraphicsItem;
class TooltipTimer;
class Workspace;

enum NetworkEditorLayer {
    NetworkEditorLayerUndefined,
    NetworkEditorLayerDataflow,
    NetworkEditorLayerLinking
};

/**
 * Widget responsible for the qgraphicsscene and all the qgraphicsitems.
 * Also handles drag & drop of processor items.
 */
class NetworkEditor : public QGraphicsView, public ProcessorNetworkObserver {
Q_OBJECT
public:
    NetworkEditor(QWidget* parent = 0, ProcessorNetwork* workspace = 0, NetworkEvaluator* evaluator = 0);
    ~NetworkEditor();

    ProcessorNetwork* getProcessorNetwork();

    /**
     * Adds a processor to the scene, using its current position.
     */
    //void addProcessor(ProcessorGraphicsItem* processor);

    ProcessorGraphicsItem* getProcessorGraphicsItem(Processor* processor);

    virtual void processorAdded(const Processor* processor);
    virtual void processorRemoved(const Processor* processor);
    virtual void propertyLinkAdded(const PropertyLink* link);
    virtual void propertyLinkRemoved(const PropertyLink* link);
    virtual void portConnectionAdded(const Port* outport, const Port* inport);
    virtual void portConnectionRemoved(const Port* outport, const Port* inport);
    virtual void networkChanged();
    virtual void processorRenamed(const Processor* processor, const std::string& prevName);

    NetworkEditorLayer currentLayer() const;

    void scale(qreal sx, qreal sy);
    void translate(qreal dx, qreal dy);

    void selectPreviouslySelectedProcessors();

public slots:
    void setProcessorNetwork(ProcessorNetwork* network);

    /**
     * Informs the NetworkEvalutor held by this object about changes of any
     * of the processor connections. This method is intended to be called
     * by ProcessorGraphicsItem objects whenever connections between their
     * ports are made.
     */
    void processorConnectionsChanged();

    void adjustLinkArrowGraphicsItems();

    void setLayerToDataflow();
    void setLayerToLinking();

private slots:
    void copyActionSlot();
    void pasteActionSlot();
    void replaceActionSlot();
    void deleteActionSlot();
    void renameActionSlot();
    void aggregateActionSlot();
    void deaggregateActionSlot();

    void editPropertyLinkSlot();
    void deletePropertyLinkSlot();

    void showTooltip();
    void updateSelectedItems();

    void createLink(RootGraphicsItem* src, RootGraphicsItem* dest);
    void createNewLink(const Property* sourceProp, const Property* destinationProp, LinkEvaluatorBase* linkEvaluator);
    void editPropertyLink(const Property* sourceProp, const Property* destinationProp, LinkEvaluatorBase* linkEvaluator);
    void removePropertyLink(PropertyLink* propertyLink);

    /// Links all cameras in the network.
    void linkCameras();

    /// Removes all links from the network.
    void removePropertyLinks();

    /// Links the processor's cameras with the cameras of all other processors in the network
    void linkCamerasOfProcessor(const Processor* processor);

    void disableTooltips();
    void enableTooltips();

    // Saves the current camera auto-linking state in the network's meta data.
    void linkCamerasAutoChanged();

signals:
    /**
     * Sent when a processor or a set of processors were selected or deselected.
     * The parameter is empty, when not processors are selected.
     */
    void processorsSelected(const QList<Processor*>& processors);

protected:
    void generateGraphicsItems();
    void generatePropertyLinkItems();
    void createContextMenus();
    void createTimer();
    void createLayerButtons();
    void layoutLayerButtons();

    void setLayer(NetworkEditorLayer layer);

    ProcessorGraphicsItem* createProcessorGraphicsItem(Processor* processor);

    AggregationGraphicsItem* createAggregationGraphicsItem(AggregationMetaData* metaData);

    /**
     * Adds a processor to the scene and moves it to the given position.
     */
    //void addProcessor(ProcessorGraphicsItem* processor, const QPoint& pos);

    LinkArrowGraphicsItem* createLinkArrowForPropertyLink(const PropertyLink* link);

    AggregationGraphicsItem* aggregateItems(const QList<RootGraphicsItem*>& items);
    void deaggregateItems(const QList<AggregationGraphicsItem*>& items);

    void removeItems(QList<QGraphicsItem*> items);
    void removeRootGraphicsItem(RootGraphicsItem* rootItem);

    void removeArrowItem(PortArrowGraphicsItem* arrow);
    void removeArrowItem(LinkArrowGraphicsItem* arrow);

    void resetScene();

    QList<ProcessorGraphicsItem*> selectedProcessorItems() const;

    void hideTooltip();

    QSize sizeHint() const;

    void scaleView();

    void resizeEvent(QResizeEvent* event);

    void wheelEvent(QWheelEvent* event);

    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void keyPressEvent(QKeyEvent* event);

    void contextMenuEvent(QContextMenuEvent* event);

    void showTooltip(const QPoint& pos, HasToolTip* toolTip);

    void showLinkDialog(PropertyGraphicsItem* propertyItem1, PropertyGraphicsItem* propertyItem2);

    void clearClipboard();

private:
    typedef std::pair<const PropertyLink*, const PropertyLink*> ArrowLinkInformation;

    void centerView();

    ProcessorNetwork* processorNetwork_;
    NetworkEvaluator* evaluator_;

    QMenu contextMenuNone_;
    QMenu contextMenuSingle_;
    QMenu contextMenuMultiple_;
    QMenu contextMenuLink_;
    QPointF rightClickPosition_;

    QAction* copyAction;
    QAction* pasteAction;
    QAction* replaceAction;
    QAction* deleteAction;
    QAction* deleteLinkAction;
    QAction* renameAction;
    QAction* editAction;
    QAction* aggregateAction;
    QAction* deaggregateAction_;

    QMap<LinkArrowGraphicsItem*, ArrowLinkInformation> linkMap_;

    LinkArrowGraphicsItem* selectedLink_;

    // Maps from the processors of the current network to their graphic items.
    QMap<Processor*,ProcessorGraphicsItem*> processorItemMap_;
    QList<AggregationGraphicsItem*> aggregationItems_;
    //QMap<Processor*,AggregationGraphicsItem*> aggregationItemMap_;

    QList<RootGraphicsItem*> clipboardProcessors_;
    //QList<ArrowGraphicsItem*> clipboardArrows_;
    //QList<LinkArrowGraphicsItem*> clipboardLinkArrows_;

    // construed as translation vector
    QPointF sceneTranslate_;
    // needed, because Qt::MouseMove returns always Qt::NoButton (setting variable on MousePress)
    bool translateScene_;
    bool needsScale_;

    // For Tooltips
    bool showTooltips_;
    QGraphicsItem* activeTooltip_;
    QPoint lastMousePosition_; // (I hope) there is no need to initialize it
    HasToolTip* lastItemWithTooltip_; // Simmilar hopes here...
    TooltipTimer* ttimer_;

    QWidget* layerButtonContainer_;
    QToolButton* dataflowLayerButton_;
    QToolButton* linkingLayerButton_;
    QWidget* autoLinkingContainer_;
    QToolButton* linkCamerasButton_;
    QToolButton* linkCamerasAutoButton_;
    QToolButton* removePropertyLinksButton_;

    NetworkEditorLayer currentLayer_;

    static const std::string loggerCat_;
};

/**
 * Widget for taking snapshots of the network graph
 */
class NetworkSnapshotPlugin : public SnapshotPlugin {
    Q_OBJECT
public:
    NetworkSnapshotPlugin(QWidget* parent, NetworkEditor* networkEditorWidget);

protected:
    void saveSnapshot(const QString& filename);
    void saveSnapshot(const QString& filename, int width, int height);

    NetworkEditor* networkEditorWidget_;
};

} // namespace voreen

#endif // VRN_NETWORKEDITOR_H
