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

#ifndef VRN_RPTGRAPHWIDGET_H
#define VRN_RPTGRAPHWIDGET_H

#include <QtGui>
#include "rptprocessorlistwidget.h"
#include "rptprocessoritem.h"
#include "rptaggregationitem.h"
#include "rpttooltiptimer.h"

namespace voreen {

class NetworkEvaluator;
class RptNetwork;

/**
 * Interface for GraphicsItems with custom Tooltips
 */
class HasRptTooltip {
public:
    virtual ~HasRptTooltip() {}

    /**
     * Returns the custom Tooltip or 0 if none is available
     */
    virtual QGraphicsItem* rptTooltip() const = 0;
};

//---------------------------------------------------------------------------

/**
 * Widget responsible for the qgraphicsscene and all the qgraphicsitems.
 * Also handles drag & drop of processor items.
 */
class RptGraphWidget : public QGraphicsView {
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    RptGraphWidget(QWidget* parent = 0, NetworkEvaluator* evaluator = 0);
    ~RptGraphWidget();

    /** Clears the scene for reusing the GraphWidget.
     */
    void clearScene();

    /**
     * Adds a processor to the scene and moves it to the given position.
     */
    void addProcessor(RptProcessorItem* processor, const QPoint& pos);

    /**
     * Adds a processor to the scene, using its current position.
     */
    void addProcessor(RptProcessorItem* processor);

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
     * Returns the number of RptProcessorItems currently selected
     * in the widget's scene or -1 if the latter one is NULL.
     */
    int countSelectedProcessorItems() const;

    void hideRptTooltip();

    NetworkEvaluator* getEvaluator() const { return evaluator_; }

    QSize sizeHint() const { return QSize(400, 600); }

    void scaleView(float maxFactor);

    void setNetwork(RptNetwork* network);
    RptNetwork* getNetwork() const { return rptnet_; }
    
signals:
    /**
     * Sent when a processor item is dropped. (connected in mainwindow)
     */
    void processorAdded(Identifier id, QPoint pos);

    /**
    * Sent when a processor item is dropped. (connected in mainwindow)
    */
    void processorAdded(RptProcessorItem* procItem);

    /**
     * Sent when an aggregation is dropped to the graphWidget. (connected in mainwindow)
     */
    void aggregationAdded(std::string filename, QPoint pos);

    /**
     * Sent when a processor or a set of processors were selected or deselected.
     * The parameter processor is 0 when no processor was selected.
     */
    void processorSelected(Processor* processor);

    /**
     * Sent by context menu
     */
    void copySignal();
    void pasteSignal();

    void deleteSignal();
    
    void showPropertiesSignal();

private slots:
    void copyActionSlot();
    void pasteActionSlot();

    void deleteActionSlot();
    void renameActionSlot();
    
    void showRptTooltip();

protected:
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

    void showRptTooltip(const QPoint& pos, HasRptTooltip* hastooltip);

private:
    QMenu contextMenu_;
    QMenu rightClickMenuSingle_;
    QMenu rightClickMenuMultiple_;

    QGraphicsScene* scene_;
    // construed as translation vector
    QPointF sceneTranslate_;
    // needed, because Qt::MouseMove returns always Qt::NoButton (setting variable on MousePress)
    bool translateScene_;

    // center of scene
    QPointF center_;

    std::vector<QWidget*> allowedWidgets_;

    // For Tooltips
    QGraphicsItem* activeRptTooltip_;
    QPoint lastMousePosition_; // (I hope) there is no need to initialize it
    HasRptTooltip* lastItemWithTooltip_; // Simmilar hopes here...
    RptTooltipTimer* ttimer_;

    NetworkEvaluator* evaluator_;
    RptNetwork* rptnet_;
    bool needsScale_;
};

} // namespace voreen

#endif // RPT_GRAPHWIDGET_H
