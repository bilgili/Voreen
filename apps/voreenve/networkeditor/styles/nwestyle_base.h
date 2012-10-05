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

#ifndef VRN_NWESTYLE_BASE_H
#define VRN_NWESTYLE_BASE_H

#include <QRect>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

class QColor;
class QBrush;

namespace voreen {

class AggregationGraphicsItem;
class PortGraphicsItem;
class ProcessorGraphicsItem;
class PropertyGraphicsItem;
class PropertyListGraphicsItem;
class PortArrowGraphicsItem;
class PortOwnerLinkArrowGraphicsItem;
class PropertyLinkArrowGraphicsItem;
class WidgetToggleButtonGraphicsItem;
class ProgressBarGraphicsItem;
class PropertyListButtonGraphicsItem;
class PortSizeLinkArrowGraphicsItem;
class ToolTipPortGraphicsItem;

class NWEStyle_Base {
public:
    NWEStyle_Base(){};

    /*********************************************************************
     *                       General Color Defines                        
     ********************************************************************/
    //editor
    static const QColor NWEStyle_NWEButtonBackgroundColor;
    static const QBrush NWEStyle_NWEBackgroundBrush;

    //graphicsitems
    static const QColor NWEStyle_SelectionColor;
    static const QColor NWEStyle_HoverColor;

    static const QColor NWEStyle_ConnectionYes;
    static const QColor NWEStyle_ConnectionMaybe;
    static const QColor NWEStyle_ConnectionNo;

    //tool tips
    static const QColor NWEStyle_ToolTipPortBackgroundColor;


    /*********************************************************************
     *                       General Functions                            
     ********************************************************************/


    /*********************************************************************
     *                       Core Elements                                
     ********************************************************************/
    //aggregation
    virtual QRectF AggregationGI_boundingRect(const AggregationGraphicsItem* item) const = 0;
    virtual QPainterPath AggregationGI_shape(const AggregationGraphicsItem* item) const = 0;
    virtual void AggregationGI_initializePaintSettings(AggregationGraphicsItem* item) = 0;
    virtual void AggregationGI_paint(AggregationGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    //port
    virtual QRectF PortGI_boundingRect(const PortGraphicsItem* item) const = 0;
    virtual QPainterPath PortGI_shape(const PortGraphicsItem* item) const = 0;
    virtual void PortGI_initializePaintSettings(PortGraphicsItem* item) = 0;
    virtual void PortGI_paint(PortGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    //processor
    virtual QRectF ProcessorGI_boundingRect(const ProcessorGraphicsItem* item) const = 0;
    virtual QPainterPath ProcessorGI_shape(const ProcessorGraphicsItem* item) const = 0;
    virtual void ProcessorGI_initializePaintSettings(ProcessorGraphicsItem* item) = 0;
    virtual void ProcessorGI_paint(ProcessorGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    //property
    virtual QRectF PropertyGI_boundingRect(const PropertyGraphicsItem* item) const = 0;
    virtual QPainterPath PropertyGI_shape(const PropertyGraphicsItem* item) const = 0;
    virtual void PropertyGI_initializePaintSettings(PropertyGraphicsItem* item) = 0;
    virtual void PropertyGI_paint(PropertyGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    //property list
    virtual QRectF PropertyListGI_boundingRect(const PropertyListGraphicsItem* item) const = 0;
    virtual QPainterPath PropertyListGI_shape(const PropertyListGraphicsItem* item) const = 0;
    virtual void PropertyListGI_initializePaintSettings(PropertyListGraphicsItem* item) = 0;
    virtual void PropertyListGI_paint(PropertyListGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    /*********************************************************************
     *                       Util Elements                                
     ********************************************************************/
    //ProgressBarGraphicsItem
    virtual QRectF ProgressBarGI_boundingRect(const ProgressBarGraphicsItem* item) const = 0;
    virtual QPainterPath ProgressBarGI_shape(const ProgressBarGraphicsItem* item) const = 0;
    virtual void ProgressBarGI_initializePaintSettings(ProgressBarGraphicsItem* item) = 0;
    virtual void ProgressBarGI_paint(ProgressBarGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    //PropertyListButtonGraphicsItem
    virtual QRectF PropertyListButtonGI_boundingRect(const PropertyListButtonGraphicsItem* item) const = 0;
    virtual QPainterPath PropertyListButtonGI_shape(const PropertyListButtonGraphicsItem* item) const = 0;
    virtual void PropertyListButtonGI_initializePaintSettings(PropertyListButtonGraphicsItem* item) = 0;
    virtual void PropertyListButtonGI_paint(PropertyListButtonGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    //WidgetToggleButtonGraphicsItem
    virtual QRectF WidgetToggleButtonGI_boundingRect(const WidgetToggleButtonGraphicsItem* item) const = 0;
    virtual QPainterPath WidgetToggleButtonGI_shape(const WidgetToggleButtonGraphicsItem* item) const = 0;
    virtual void WidgetToggleButtonGI_initializePaintSettings(WidgetToggleButtonGraphicsItem* item) = 0;
    virtual void WidgetToggleButtonGI_paint(WidgetToggleButtonGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    /*********************************************************************
     *                       Connection Elements                          
     ********************************************************************/
    //port arrow
    virtual QRectF PortArrowGI_boundingRect(const PortArrowGraphicsItem* item) const = 0;
    virtual QPainterPath PortArrowGI_shape(const PortArrowGraphicsItem* item) const = 0;
    virtual void PortArrowGI_initializePaintSettings(const PortArrowGraphicsItem* item) = 0;
    virtual void PortArrowGI_paint(PortArrowGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    //processor link arrow
    virtual QRectF PortOwnerLinkArrowGI_boundingRect(const PortOwnerLinkArrowGraphicsItem* item) const = 0;
    virtual QPainterPath PortOwnerLinkArrowGI_shape(const PortOwnerLinkArrowGraphicsItem* item) const = 0;
    virtual void PortOwnerLinkArrowGI_initializePaintSettings(PortOwnerLinkArrowGraphicsItem* item) = 0;
    virtual void PortOwnerLinkArrowGI_paint(PortOwnerLinkArrowGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    //property link arrow
    virtual QRectF PropertyLinkArrowGI_boundingRect(const PropertyLinkArrowGraphicsItem* item) const = 0;
    virtual QPainterPath PropertyLinkArrowGI_shape(const PropertyLinkArrowGraphicsItem* item) const = 0;
    virtual void PropertyLinkArrowGI_initializePaintSettings(const PropertyLinkArrowGraphicsItem* item) = 0;
    virtual void PropertyLinkArrowGI_paint(PropertyLinkArrowGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    //port size link arrow
    virtual QRectF PortSizeLinkArrowGI_boundingRect(const PortSizeLinkArrowGraphicsItem* item) const = 0;
    virtual QPainterPath PortSizeLinkArrowGI_shape(const PortSizeLinkArrowGraphicsItem* item) const = 0;
    virtual void PortSizeLinkArrowGI_initializePaintSettings(PortSizeLinkArrowGraphicsItem* item) = 0;
    virtual void PortSizeLinkArrowGI_paint(PortSizeLinkArrowGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    /*********************************************************************
     *                       ToolTip Elements                             
     ********************************************************************/
    //port tooltip
    virtual QRectF ToolTipPortGI_boundingRect(const ToolTipPortGraphicsItem* item) const = 0;
    virtual QPainterPath ToolTipPortGI_shape(const ToolTipPortGraphicsItem* item) const = 0;
    virtual void ToolTipPortGI_initializePaintSettings(ToolTipPortGraphicsItem* item) = 0;
    virtual void ToolTipPortGI_paint(ToolTipPortGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;
};

} //namespace voreen

#endif // VRN_NWESTYLE_BASE_H

