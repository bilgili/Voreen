/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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
    RptGraphWidget(QWidget* parent=0);
    ~RptGraphWidget();

    /** Clears the scene for reusing the GraphWidget.
     */
    void clearScene();

    /**
     * Adds an item to the scene, and moves it by given pos.
     */
    void addItem(RptGuiItem* item, QPoint pos);
    /**
     * Adds an item to the scene, using position of the item.
     */
    void addItem(RptGuiItem* item);

    /**
     * Adds a Widget that drop events should be accepted from.
     */
    // modified in order to allow ANY widget to drop into GraphWidget
    // (dirk)
    //void addAllowedWidget(QTreeWidget* widget) {
    void addAllowedWidget(QWidget* widget) {
        allowedWidgets_.push_back(widget);
    }

    void updateSelectedItems();

    // set center of graph widget
    void setCenter(QPointF);
    // centers graph widget
    void center();

    /**
     * Returns the scene.
     */
	QGraphicsScene* getScene(){return scene_;}

    /** Returns the number of RptProcessorItems currently selected
     * in the widget's scene or -1 if the latter one is NULL.
     */
    int countSelectedProcessorItems() const;

    void hideRptTooltip();


signals:
    /**
     * Sent when a processor item is dropped. (connected in mainwindow)
     */
    void processorAdded(Identifier id, QPoint pos);
    void aggregationAdded(std::string filename, QPoint pos);

    void sendProcessor(Processor*, QVector<int>);
    /**
     * Sent by context menu (connected in mainwindow).
     */
    void copySignal();
    void pasteSignal();

    /**
     * Sent by double click on item.
     */
    void showPropertiesSignal();

private slots:
    /**
     * Action for context menu. Sends copySignal (connected in mainwindow).
     */
    void copyActionSlot();
    void pasteActionSlot();
    void updateScene(const QList<QRectF> &rects );

    void showRptTooltip();

protected:
    void createContextMenu();
    void paintEvent(QPaintEvent *event);

    void scaleView(qreal scaleFactor);
    void wheelEvent(QWheelEvent *event);

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);

    void mousePressEvent(QMouseEvent  *event);
    void mouseMoveEvent(QMouseEvent  *event);
    void mouseReleaseEvent(QMouseEvent  *event);
    void mouseDoubleClickEvent(QMouseEvent  *event);

    void contextMenuEvent(QContextMenuEvent *event);

    void showRptTooltip(const QPoint & pos, HasRptTooltip* hastooltip);

    QMenu contextMenu_;
private:
    void createConnections();

    QGraphicsScene* scene_;
    // construed as translation vector
    QPointF sceneTranslate_;
    // needed, because Qt::MouseMove returns always Qt::NoButton (setting variable on MousePress)
    bool translateScene_;
    //RptGraphicsScene* scene_;

    // center of scene
    QPointF center_;

    // modified in order to allow ANY widget to drop into GraphWidget
    // (dirk)
    //std::vector<QTreeWidget*> allowedWidgets_;
    std::vector<QWidget*> allowedWidgets_;

    // For Tooltips
    QGraphicsItem* activeRptTooltip_;
    QPoint lastMousePosition_; // (I hope) there is no need to initialize it
    HasRptTooltip* lastItemWithTooltip_; // Simmilar hopes here...
    RptTooltipTimer* ttimer_;
};

} //namespace voreen

#endif // RPT_GRAPHWIDGET_H
