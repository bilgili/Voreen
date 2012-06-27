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

#include <map>
#include <QtGui>

#include "voreen/core/vis/processors/processornetwork.h"

#include "processorgraphicsitem.h"
#include "tooltiptimer.h"

namespace voreen {

class NetworkEvaluator;

/**
 * Interface for GraphicsItems with custom Tooltips
 */
class HasTooltip {
public:
    virtual ~HasTooltip() {}

    /**
     * Returns the custom Tooltip or 0 if none is available
     */
    virtual QGraphicsItem* tooltip() const = 0;
};

//---------------------------------------------------------------------------

/**
 * Widget responsible for the qgraphicsscene and all the qgraphicsitems.
 * Also handles drag & drop of processor items.
 */
class NetworkEditor : public QGraphicsView {
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    NetworkEditor(QWidget* parent = 0, ProcessorNetwork* processorNet = 0, NetworkEvaluator* evaluator = 0);
    ~NetworkEditor();

    /** Clears the scene for reusing the GraphWidget.
     */
    void clearScene();

    /**
     * Adds a processor to the scene and moves it to the given position.
     */
    void addProcessor(ProcessorGraphicsItem* processor, const QPoint& pos);

    /**
     * Adds a processor to the scene, using its current position.
     */
    void addProcessor(ProcessorGraphicsItem* processor);

    /**
     * Adds a Widget that drop events should be accepted from.
     */
    void addAllowedWidget(QWidget* widget) {
        allowedWidgets_.push_back(widget);
    }

    void updateSelectedItems();

    /// set center of graph widget
    void setCenter(QPointF);

    /// centers graph widget
    void center();

    /**
     * Returns the scene.
     */
    QGraphicsScene* getScene(){ return scene_; }

    /**
     * Returns the number of ProcessorGraphicsItems currently selected
     * in the widget's scene or -1 if the latter one is NULL.
     */
    int countSelectedProcessorItems() const;

    void hideTooltip();

    NetworkEvaluator* getEvaluator() const { return evaluator_; }

    QSize sizeHint() const { return QSize(400, 600); }

    void scaleView(float maxFactor);

public slots:

    /**
     * Sets the current network.
     */
    void setNetwork(ProcessorNetwork* network);

signals:

    /**
     * Sent when a processor or a set of processors were selected or deselected.
     * The parameter processor is 0 when no processor was selected.
     */
    void processorSelected(Processor* processor);

    /**
     * Emitted when the user has changed a processor's name.
     * The modified processor is passed as parameter.
     */
    void processorNameChanged(Processor* processor);

    /**
     * Sent by context menu
     */
    void copySignal();
    void pasteSignal();
    void showPropertiesSignal();

private slots:
    void copyActionSlot();
    void pasteActionSlot();

    void deleteActionSlot();
    void renameActionSlot();
    
    void showTooltip();

protected:

    void generateGraphicsItems();

    void removeSelectedItems();

    void removeProcessorItem(Processor* processor);

    void removeArrowItem(ArrowGraphicsItem* arrow);

    void createContextMenus();
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);

    void wheelEvent(QWheelEvent* event);

    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);

    void keyReleaseEvent(QKeyEvent* event);

    void contextMenuEvent(QContextMenuEvent* event);

    void showTooltip(const QPoint& pos, HasTooltip* hastooltip);

private:
    QMenu contextMenu_;
    QMenu rightClickMenuSingle_;
    QMenu rightClickMenuMultiple_;

    // Maps from the processors of the current network to their graphic items.
    std::map<Processor*,ProcessorGraphicsItem*> processorItemMap_;

    QGraphicsScene* scene_;
    // construed as translation vector
    QPointF sceneTranslate_;
    // needed, because Qt::MouseMove returns always Qt::NoButton (setting variable on MousePress)
    bool translateScene_;

    // center of scene
    QPointF center_;

    std::vector<QWidget*> allowedWidgets_;

    // For Tooltips
    QGraphicsItem* activeTooltip_;
    QPoint lastMousePosition_; // (I hope) there is no need to initialize it
    HasTooltip* lastItemWithTooltip_; // Simmilar hopes here...
    TooltipTimer* ttimer_;

    NetworkEvaluator* evaluator_;
    ProcessorNetwork* processorNetwork_;
    bool needsScale_;
};

} // namespace voreen

#endif // VRN_NETWORKEDITOR_H
