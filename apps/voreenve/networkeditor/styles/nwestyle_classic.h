/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#ifndef VRN_NWESTYLE_CLASSIC_H
#define VRN_NWESTYLE_CLASSIC_H

#include "nwestyle_base.h"

namespace voreen {

class NWEStyle_Classic : public NWEStyle_Base{
public:
    NWEStyle_Classic();

    /*********************************************************************
     *                       General Functions                            
     ********************************************************************/


    /*********************************************************************
     *                       Core Elements                                
     ********************************************************************/
    //aggregation
    QRectF AggregationGI_boundingRect(const AggregationGraphicsItem* item) const;
    QPainterPath AggregationGI_shape(const AggregationGraphicsItem* item) const;
    void AggregationGI_initializePaintSettings(AggregationGraphicsItem* item);
    void AggregationGI_paint(AggregationGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    //port
    QRectF PortGI_boundingRect(const PortGraphicsItem* item) const;
    QPainterPath PortGI_shape(const PortGraphicsItem* item) const;
    void PortGI_initializePaintSettings(PortGraphicsItem* item);
    void PortGI_paint(PortGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    //processor
    QRectF ProcessorGI_boundingRect(const ProcessorGraphicsItem* item) const;
    QPainterPath ProcessorGI_shape(const ProcessorGraphicsItem* item) const;
    void ProcessorGI_initializePaintSettings(ProcessorGraphicsItem* item);
    void ProcessorGI_paint(ProcessorGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    //property
    QRectF PropertyGI_boundingRect(const PropertyGraphicsItem* item) const;
    QPainterPath PropertyGI_shape(const PropertyGraphicsItem* item) const;
    void PropertyGI_initializePaintSettings(PropertyGraphicsItem* item);
    void PropertyGI_paint(PropertyGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    //property list
    QRectF PropertyListGI_boundingRect(const PropertyListGraphicsItem* item) const;
    QPainterPath PropertyListGI_shape(const PropertyListGraphicsItem* item) const;
    void PropertyListGI_initializePaintSettings(PropertyListGraphicsItem* item);
    void PropertyListGI_paint(PropertyListGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    /*********************************************************************
     *                       Util Elements                                
     ********************************************************************/
    //ProgressBarGraphicsItem
    QRectF ProgressBarGI_boundingRect(const ProgressBarGraphicsItem* item) const;
    QPainterPath ProgressBarGI_shape(const ProgressBarGraphicsItem* item) const;
    void ProgressBarGI_initializePaintSettings(ProgressBarGraphicsItem* item);
    void ProgressBarGI_paint(ProgressBarGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    //PropertyListButtonGraphicsItem
    QRectF PropertyListButtonGI_boundingRect(const PropertyListButtonGraphicsItem* item) const;
    QPainterPath PropertyListButtonGI_shape(const PropertyListButtonGraphicsItem* item) const;
    void PropertyListButtonGI_initializePaintSettings(PropertyListButtonGraphicsItem* item);
    void PropertyListButtonGI_paint(PropertyListButtonGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    //WidgetToggleButtonGraphicsItem
    QRectF WidgetToggleButtonGI_boundingRect(const WidgetToggleButtonGraphicsItem* item) const;
    QPainterPath WidgetToggleButtonGI_shape(const WidgetToggleButtonGraphicsItem* item) const;
    void WidgetToggleButtonGI_initializePaintSettings(WidgetToggleButtonGraphicsItem* item);
    void WidgetToggleButtonGI_paint(WidgetToggleButtonGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    /*********************************************************************
     *                       Connection Elements                          
     ********************************************************************/
    //port arrow
    QRectF PortArrowGI_boundingRect(const PortArrowGraphicsItem* item) const;
    QPainterPath PortArrowGI_shape(const PortArrowGraphicsItem* item) const;
    void PortArrowGI_initializePaintSettings(const PortArrowGraphicsItem* item);
    void PortArrowGI_paint(PortArrowGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    //processor link arrow
    QRectF PortOwnerLinkArrowGI_boundingRect(const PortOwnerLinkArrowGraphicsItem* item) const;
    QPainterPath PortOwnerLinkArrowGI_shape(const PortOwnerLinkArrowGraphicsItem* item) const;
    void PortOwnerLinkArrowGI_initializePaintSettings(PortOwnerLinkArrowGraphicsItem* item);
    void PortOwnerLinkArrowGI_paint(PortOwnerLinkArrowGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    //property link arrow
    QRectF PropertyLinkArrowGI_boundingRect(const PropertyLinkArrowGraphicsItem* item) const;
    QPainterPath PropertyLinkArrowGI_shape(const PropertyLinkArrowGraphicsItem* item) const;
    void PropertyLinkArrowGI_initializePaintSettings(const PropertyLinkArrowGraphicsItem* item);
    void PropertyLinkArrowGI_paint(PropertyLinkArrowGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    //port size link arrow
    QRectF PortSizeLinkArrowGI_boundingRect(const PortSizeLinkArrowGraphicsItem* item) const;
    QPainterPath PortSizeLinkArrowGI_shape(const PortSizeLinkArrowGraphicsItem* item) const;
    void PortSizeLinkArrowGI_initializePaintSettings(PortSizeLinkArrowGraphicsItem* item);
    void PortSizeLinkArrowGI_paint(PortSizeLinkArrowGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    /*********************************************************************
     *                       ToolTip Elements                             
     ********************************************************************/
    //port tooltip
    QRectF ToolTipPortGI_boundingRect(const ToolTipPortGraphicsItem* item) const;
    QPainterPath ToolTipPortGI_shape(const ToolTipPortGraphicsItem* item) const;
    void ToolTipPortGI_initializePaintSettings(ToolTipPortGraphicsItem* item);
    void ToolTipPortGI_paint(ToolTipPortGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
};

} //namespace voreen

#endif // VRN_NWESTYLE_CLASSIC_H

