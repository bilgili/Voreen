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

#ifndef VRN_NETWORKEDITOR_H
#define VRN_NETWORKEDITOR_H

#include "voreen/core/vis/network/processornetwork.h"

#include "voreen/qt/widgets/network/editor/openpropertylistbutton.h"

//#include "processorgraphicsitem.h"
//#include "tooltiptimer.h"
#include <QGraphicsView>
#include <QMenu>
#include <map>
#include <vector>

class QToolButton;

namespace voreen {

class Workspace;
class NetworkEvaluator;
class LinkArrowGraphicsItem;
class LinkEvaluatorBase;
class PropertyGraphicsItem;
class ProcessorGraphicsItem;
class ITooltip;
class ArrowGraphicsItem;
class TooltipTimer;

enum NetworkEditorLayer {
    NetworkEditorLayerDataflow,
    NetworkEditorLayerLinking
};

/**
 * Widget responsible for the qgraphicsscene and all the qgraphicsitems.
 * Also handles drag & drop of processor items.
 */
class NetworkEditor : public QGraphicsView , public ProcessorNetworkObserver {
Q_OBJECT
public:
    NetworkEditor(QWidget* parent = 0, Workspace* workspace = 0, NetworkEvaluator* evaluator = 0);
    ~NetworkEditor();

    ProcessorNetwork* getProcessorNetwork();

    /**
     * Adds a processor to the scene, using its current position.
     */
    void addProcessor(ProcessorGraphicsItem* processor);

    virtual void processorAdded(const Processor* processor);
    virtual void processorRemoved(const Processor* processor);
    //virtual void connectionsChanged();
    virtual void propertyLinkAdded(const PropertyLink* link);
    virtual void propertyLinkRemoved(const PropertyLink* link);
    virtual void networkChanged();

    NetworkEditorLayer currentLayer() const;

public slots:
    /// Called if the workspace network has been reloaded.
    void newNetwork();
    void pressedPropertyGraphicsItem(PropertyGraphicsItem* propertyGraphicsItem);

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

    void deleteActionSlot();
    void renameActionSlot();

    void showTooltip();

    void editPropertyLinkSlot();

    void deletePropertyLinkSlot();

    void createNewLink(PropertyGraphicsItem* sourceItem, PropertyGraphicsItem* destinationItem, LinkEvaluatorBase* linkEvaluator);
    void editPropertyLink(PropertyGraphicsItem* sourceItem, PropertyGraphicsItem* destinationItem, LinkEvaluatorBase* linkEvaluator);

    /// Links all cameras in the network.
    void linkCameras();

    /// Removes all links from the network.
    void removePropertyLinks();

    /// Links the processor's cameras with the cameras of all other processors in the network
    void linkCamerasOfProcessor(const Processor* processor);

signals:
    void removePropertyLinksButtonPressed();

    /**
     * Sent when a processor or a set of processors were selected or deselected.
     * The parameter is empty, when not processors are selected.
     */
    void processorsSelected(const std::vector<Processor*>& processors);

    /**
     * Sent by context menu
     */
    void copySignal();
    void pasteSignal();
    void showPropertiesSignal();
    void update();

protected:
    void generateGraphicsItems();
    void generatePropertyLinkItems();
    void createContextMenus();
    void createTooltipTimer();
    void createLayerButtons();
    void layoutLayerButtons();

    void setLayer(NetworkEditorLayer layer);

    ProcessorGraphicsItem* createProcessorGraphicsItem(Processor* processor, QPoint position = QPoint());

    /**
     * Adds a processor to the scene and moves it to the given position.
     */
    void addProcessor(ProcessorGraphicsItem* processor, const QPoint& pos);

    LinkArrowGraphicsItem* createLinkArrowForPropertyLink(const PropertyLink* link);

    void removeSelectedItems();
    void removeProcessorItem(ProcessorGraphicsItem* processorItem);

    void removeArrowItem(ArrowGraphicsItem* arrow);
    void removeArrowItem(LinkArrowGraphicsItem* arrow);

    /**
     * Clears the scene for reusing the GraphWidget.
     */
    void clearScene();

    void updateSelectedItems();

    /// set center of graph widget
    void setCenter(QPointF);

    /// centers graph widget
    void center();

    /**
     * Returns the number of ProcessorGraphicsItems currently selected
     * in the widget's scene or -1 if the latter one is NULL.
     */
    int countSelectedProcessorItems() const;
    QList<ProcessorGraphicsItem*> selectedProcessorItems() const;

    void hideTooltip();

    QSize sizeHint() const;

    void scaleView(float maxFactor);

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

    void showTooltip(const QPoint& pos, ITooltip* ITooltip);

    void showLinkDialog(PropertyGraphicsItem* propertyItem1, PropertyGraphicsItem* propertyItem2);

    void clearClipboard();

    bool processorHasPropertyLinks(ProcessorGraphicsItem* processor);

    void removePropertyLink(const PropertyLink* link);

private:
    Workspace* workspace_;
    NetworkEvaluator* evaluator_;

    QMenu rightClickMenuNone_;
    QMenu rightClickMenuSingle_;
    QMenu rightClickMenuMultiple_;
    QMenu rightClickLinkMenu_;
    QPointF rightClickPosition_;

    QList<LinkArrowGraphicsItem*> linkArrows_;

    LinkArrowGraphicsItem* selectedLink_;
    PropertyGraphicsItem* selectedPropertyGraphicsItem_;

    // Maps from the processors of the current network to their graphic items.
    QMap<Processor*,ProcessorGraphicsItem*> processorItemMap_;

    QList<ProcessorGraphicsItem*> clipboardProcessors_;
    //QList<ArrowGraphicsItem*> clipboardArrows_;
    //QList<LinkArrowGraphicsItem*> clipboardLinkArrows_;

    // construed as translation vector
    QPointF sceneTranslate_;
    // needed, because Qt::MouseMove returns always Qt::NoButton (setting variable on MousePress)
    bool translateScene_;
    bool needsScale_;

    // center of scene
    QPointF center_;

    // For Tooltips
    QGraphicsItem* activeTooltip_;
    QPoint lastMousePosition_; // (I hope) there is no need to initialize it
    ITooltip* lastItemWithTooltip_; // Simmilar hopes here...
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

} // namespace voreen

#endif // VRN_NETWORKEDITOR_H
