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

#ifndef VRN_RPTPROPERTYSETITEM_H
#define VRN_RPTPROPERTYSETITEM_H

#include "rptprocessoritem.h"
#include "rptarrow.h"
#include "voreen/core/vis/processors/propertyset.h"

namespace voreen {

class RptPropertyPort;
class RptAggregationItem;

/**
 * Representation of a PropertySet in the GUI.
 */
class RptPropertySetItem : public RptGuiItem {

Q_OBJECT

public:
    /**
     * Constructor.
     * @param scene scene the item will be added to; needed to create port
     */
    RptPropertySetItem(QGraphicsScene* scene=0, QGraphicsItem* parent=0);
    /**
     * Constructor.
     * @param processor items that will share properties
     * @param scene scene the item will be added to; needed to create port
     */
    RptPropertySetItem(std::vector<RptGuiItem*> processors, QGraphicsScene* scene=0, QGraphicsItem* parent=0);
    RptPropertySetItem(PropertySet* propertySet, const std::map<Processor*,RptProcessorItem*> processorMap, QGraphicsScene* scene=0, QGraphicsItem* parent=0);

    ~RptPropertySetItem();

    /**
     * Used to identfy an item in the scene as RptPropertySetItem
     */
    enum { Type = UserType + 5 };

    /**
     * Returns the type that identifies this item as an RptPropertySetItem.
     */
    int type() const { return Type; }

    void setName(std::string name);
    /**
     * Returns the propertySet.
     * @param port has no meaning here; relict from superclass
     */
    Processor* getProcessor(RptPortItem* port=0) { return propertySet_; }
    /**
     * Returns the propertySet.
     */
    PropertySet* getPropertySet() const {return propertySet_;}
    
    RptPropertySetItem& saveMeta();
    RptPropertySetItem& loadMeta();


    /**
     * Connects the given item to this property set.
     */
    bool connectGuiItem(RptGuiItem* item);
    /**
     * Disconnects the given item from this property set.
     */
    bool disconnectGuiItem(QGraphicsItem* item);
    /**
     * Disconnect everything from this property set.
     */
    void disconnectAll();

    /**
     * Takes care of removing arrows from items that get aggregated
     * and redrawing them to the given aggregation.
     */
    void aggregate(RptAggregationItem* aggregation);
    /**
     * If an aggregation, some items of this property set belong to, gets deaggregated,
     * this method is responsible for redrawing the arrows to the individual items.
     */
    void deaggregate(RptAggregationItem* aggregation);

    /**
     * Adds the arrows connected to this item to the scene.
     */
    virtual void showAllArrows();

    /**
     * Adjust the arrows.
     */
    virtual void adjustArrows();

    /**
     * TODO: This method should return the indices of the properties of the given item
     * that differ from the properties of this property set.
     */
    QVector<int> getUnequalEntries(RptProcessorItem* item);

    QRectF boundingRect() const;
    QPainterPath propertySetItemPath(QRectF rect) const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	std::vector<RptGuiItem*> getGuiItems() {return guiItems_;}

signals:
    /**
     * Sends a PropertySet containing the selected processors on ItemSelectedChange.
     */
    void sendProcessor(Processor*,QVector<int>);

private slots:
    void equalizeSlot();

protected:
    void createContextMenu();

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    void updateToolTip();

    PropertySet* propertySet_;
    std::vector<RptGuiItem*> guiItems_;
    RptPropertyPort* port_;
    std::vector<RptArrow*> arrows_;
    std::vector<RptArrow*> aggregationArrows_;
	QColor color_;

};

//---------------------------------------------------------------------------

class RptPropertyPort : public QGraphicsItem {

public:
    RptPropertyPort(QGraphicsItem* parent=0);
    
    /**
     * Used to identfy an item in the scene as RptPropertyPort
     */  
    enum { Type = UserType + 10 };

    /**
     * Returns the type that identifies this item as a RptPropertyPort.
     */
    int type() const { return Type; }

    QRectF boundingRect() const;
    //QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    QGraphicsLineItem* line_;

};

} //namespcae voreen

#endif //VRN_RPTPROPERTYSETITEM_H
