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

#ifndef VRN_RPTAGGREGATIONITEM_H
#define VRN_RPTAGGREGATIONITEM_H

#include <QtGui>
#include "rptguiitem.h"


namespace voreen {

class Processor;
class RptProcessorItem;

/**
 * This class summarizes several RptProcessorItems to one item in the scene.
 */
class RptAggregationItem : public RptGuiItem {

    Q_OBJECT

public:
    /**
     * Constructor.
     * @param processors gui items that shall be summarized
     * @param scene scene to which the aggregation will be added; needed to create port items
     * @name same of the aggregation
     */
    RptAggregationItem(std::vector<RptProcessorItem*> processors, QGraphicsScene* scene, std::string name = "Aggregation", QGraphicsItem* parent=0);
    RptAggregationItem(std::vector<RptProcessorItem*> processors, std::string name = "Aggregation", QGraphicsItem* parent=0);
    ~RptAggregationItem();

    /**
    * Can be called after added to scene
    */
    void initialize();

    /**
     * Puts the ProcessorItems from the aggregation back to the scene.
     * @return the items held by this aggregation
     */
    std::vector<RptProcessorItem*> deaggregate();

    /**
     * Remove the items from the aggregation.
     */
    void clear() { processors_.clear(); }

    /**
     * Returns true, if the aggregation is set to show its content.
     */
    bool isShowContent() { return showContent_; }

    /**
     * Show/Hide the content of the aggregation.
     */
    void showContent(bool show);

    /**
     * Returns the aggregated items.
     */
    std::vector<RptProcessorItem*> getProcessorItems() { return processors_; }

    /**
     * Returns the processor the given port belongs to.
     */
    Processor* getProcessor(RptPortItem* port);

    /**
     * Returns true if the given item is contained by this aggregation.
     */
    bool contains(RptGuiItem* item);

    /**
     * Used to identfy an item in the scene as RptAggregationItem.
     */
    enum { Type = UserType + 3 };

    /**
     * Returns the type that identifies this item as RptAggregationItem.
     */
    int type() const { return Type; }

    /**
     * Calls the methods to update this aggregation item due to changes like
     * aggregation movement and showing / hiding aggregation content.
     */
    void updateGeometry();

    QRectF boundingRect() const;

public slots:
    /**
     * Action for context menu. Triggers showContent.
     */
    void showContentActionSlot();
    /**
     * Action for context menu. Sends deaggregateSignal (connected in mainwindow).
     */
    void deaggregateActionSlot();
    void saveActionSlot();

signals:
    /**
     * Send when deaggregate is clicked in context menu.
     */
    void deaggregateSignal();

    void saveSignal(RptAggregationItem*);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    // Create ports and coprocessor ports.
    void createAndConnectPorts();
    void createAndConnectCoProcessorPorts();

    // Find ports that are unconnected or connected to items outside the aggregation.
    std::vector<RptPortItem*> getUnconnectedPortItems();
    std::vector<RptPortItem*> getOutwardsConnectedPortItems();
    std::vector<RptPortItem*> getUnconnectedCoProcessorPortItems();
    std::vector<RptPortItem*> getOutwardsConnectedCoProcessorPortItems();

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    // event handling
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    void createContextMenu();

private:

    QPainterPath shapePath(bool isShadow, int offset) const;

    // color
    QColor color_;

    // aggregated items
    std::vector<RptProcessorItem*> processors_;

    bool showContent_;

    // actions for context menu
    QAction* showContentAction_;
    QAction* deaggregateAction_;
    QAction* deleteAction_;

    QPointF oldPos_;

};

} //namespace voreen

#endif // VRN_RPTAGGREGATIONITEM_H
