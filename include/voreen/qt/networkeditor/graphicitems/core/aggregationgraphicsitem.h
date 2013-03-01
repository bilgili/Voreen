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

#ifndef VRN_AGGREGATIONGRAPHICSITEM_H
#define VRN_AGGREGATIONGRAPHICSITEM_H

#include "processorgraphicsitem.h"

namespace voreen {

class Aggregation;
class AggregationMetaData;

class AggregationGraphicsItem : public ProcessorGraphicsItem {
Q_OBJECT
public:
    //constructor + destructor
    AggregationGraphicsItem(Processor* aggregation, QList<PortOwnerGraphicsItem*>* items, voreen::NetworkEditor* networkEditor);
    ~AggregationGraphicsItem();

    //---------------------------------------------------------------------------------
    //      nwebasegraphicsitem functions
    //---------------------------------------------------------------------------------
    int type() const {return UserTypesAggregationGraphicsItem;}

    //style
    QRectF boundingRect() const;
    QPainterPath shape() const;
protected:
    void initializePaintSettings();
    void mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget, NWEItemSettings& setting);

    //---------------------------------------------------------------------------------
    //      portownergraphicsitem functions
    //---------------------------------------------------------------------------------
public:
    QList<Processor*> getProcessors() const;
    void saveMeta();
    void loadMeta();
protected:
    void registerPorts();
    void renameFinished();
    void toggleProcessorWidget();
    void togglePropertyList();

    //---------------------------------------------------------------------------------
    //      processorobserver functions
    //---------------------------------------------------------------------------------
public:
    void processorWidgetCreated(const Processor* processor);
    void processorWidgetDeleted(const Processor* processor);
    void portsChanged(const Processor* processor);
    void propertiesChanged(const PropertyOwner* owner);

    //---------------------------------------------------------------------------------
    //      aggregation functions
    //---------------------------------------------------------------------------------
public:
    //getter and setter
    QList<PortOwnerGraphicsItem*> getContainingItems() const;
    bool contains(PortOwnerGraphicsItem* poItem) const;
        //meta
    void setAggregationMetaData(AggregationMetaData* metaData);
    AggregationMetaData* getAggregationMetaData() const;

    void prepareDeaggregation();
protected:
    void registerConnections();
    void deregisterConnections();
private:
    //member
    QList<PortOwnerGraphicsItem*>* portOwnerItems_;
    QMap<PortOwnerGraphicsItem*, QPointF> relativePositionMap_;
    AggregationMetaData* metaData_;
};

} // namespace

#endif // VRN_AGGREGATIONGRAPHICSITEM_H


/*
    bool hasProperty(const Property* prop) const;

    QList<RootGraphicsItem*> getRootGraphicsItems() const;
    void addInternalPortArrow(PortArrowGraphicsItem* arrow);
    QList<PortArrowGraphicsItem*> getInternalPortArrows() const;
    void addInternalLinkArrow(LinkArrowGraphicsItem* arrow, const ArrowLinkInformation& linkInformation);
    QList<QPair<LinkArrowGraphicsItem*, ArrowLinkInformation> > getInternalLinkArrows() const;
    QList<Processor*> getProcessors() const;
    void renameFinished(bool changeChildItems = true);
    void processorWidgetCreated(const Processor* processor);
    void processorWidgetDeleted(const Processor* processor);
    void propertiesChanged(const PropertyOwner*);
    void portsChanged(const Processor*);
    QList<QAction*> getProcessorWidgetContextMenuActions();


protected slots:
    void toggleSingleProcessorWidget();

protected:

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);


    void toggleProcessorWidget();

private:
/// the list of all internally maintained \sa PortArrowGraphicsItems
    QList<PortArrowGraphicsItem*> internalPortArrows_;
    /// list list of all internally maintained \sa LinkArrowGraphicsItems with their PropertyLinks
    QList<QPair<LinkArrowGraphicsItem*, ArrowLinkInformation> > internalLinkArrows_;

    /// this ivar maps every action to its processor widget, used in toggleSingleProcessorWidget
    QMap<QAction*, ProcessorWidget*> processorWidgetMap_;
*/

