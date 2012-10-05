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

#include "nwestyle_classic.h"
#include "../networkeditor_settings.h"

//gi
#include "../graphicitems/core/aggregationgraphicsitem.h"
#include "../graphicitems/core/portgraphicsitem.h"
#include "../graphicitems/core/processorgraphicsitem.h"
#include "../graphicitems/core/propertygraphicsitem.h"
#include "../graphicitems/core/propertylistgraphicsitem.h"
#include "../graphicitems/connections/portarrowgraphicsitem.h"
#include "../graphicitems/connections/portsizelinkarrowgraphicsitem.h"
#include "../graphicitems/connections/portownerlinkarrowgraphicsitem.h"
#include "../graphicitems/connections/propertylinkarrowgraphicsitem.h"
#include "../graphicitems/utils/progressbargraphicsitem.h"
#include "../graphicitems/utils/propertylistbuttongraphicsitem.h"
#include "../graphicitems/utils/widgettogglebuttongraphicsitem.h"
#include "../graphicitems/tooltips/tooltipportgraphicsitem.h"
//core
#include "voreen/core/ports/port.h"
#include "voreen/core/ports/coprocessorport.h"
#include "voreen/core/ports/renderport.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/processors/processorwidget.h"

namespace voreen{

    /*********************************************************************
     *                       General Color Defines                        
     ********************************************************************/

    //editor
    const QColor NWEStyle_Base::NWEStyle_NWEButtonBackgroundColor = QColor(50,50,50,100);
    const QBrush NWEStyle_Base::NWEStyle_NWEBackgroundBrush =  QBrush(Qt::darkGray);

    const QColor NWEStyle_Base::NWEStyle_SelectionColor = QColor(0,0,237,255);
    const QColor NWEStyle_Base::NWEStyle_HoverColor = QColor(0,0,97,255);

    const QColor NWEStyle_Base::NWEStyle_ConnectionYes = Qt::green;
    const QColor NWEStyle_Base::NWEStyle_ConnectionMaybe = Qt::yellow;
    const QColor NWEStyle_Base::NWEStyle_ConnectionNo = Qt::red;

    //tool tips
    const QColor NWEStyle_Base::NWEStyle_ToolTipPortBackgroundColor = QColor(Qt::white);

    /*********************************************************************
     *                       General Functions                        
     ********************************************************************/
    NWEStyle_Classic::NWEStyle_Classic() 
    : NWEStyle_Base()
{}

    QPolygonF createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirection direction, const QPointF& basePoint) {
    double angle = -tgt::PI / 2.0;

    QPointF arrowP1;
    QPointF arrowP2;
    switch (direction) {
    case ConnectionBaseGraphicsItem::ArrowHeadDirectionNone:
        return QPolygonF();
    case ConnectionBaseGraphicsItem::ArrowHeadDirectionNS:
        arrowP1 = basePoint + QPointF(sin(angle - tgt::PI / 3.0) * cbArrowHeadSize          , cos(angle - tgt::PI / 3.f) * cbArrowHeadSize);
        arrowP2 = basePoint + QPointF(sin(angle - tgt::PI + tgt::PI / 3.0) * cbArrowHeadSize, cos(angle - tgt::PI + tgt::PI / 3.0) * cbArrowHeadSize);
        break;
    case ConnectionBaseGraphicsItem::ArrowHeadDirectionSN:
        arrowP1 = basePoint - QPointF(sin(angle - tgt::PI / 3.0) * cbArrowHeadSize          , cos(angle - tgt::PI / 3.0) * cbArrowHeadSize);
        arrowP2 = basePoint - QPointF(sin(angle - tgt::PI + tgt::PI / 3.0) * cbArrowHeadSize, cos(angle - tgt::PI + tgt::PI / 3.0) * cbArrowHeadSize);
        break;
    case ConnectionBaseGraphicsItem::ArrowHeadDirectionWE:
        arrowP1 = basePoint + QPointF(cos(angle - tgt::PI / 3.0) * cbArrowHeadSize          , sin(angle - tgt::PI / 3.0) * cbArrowHeadSize);
        arrowP2 = basePoint + QPointF(cos(angle - tgt::PI + tgt::PI / 3.0) * cbArrowHeadSize, sin(angle - tgt::PI + tgt::PI / 3.0) * cbArrowHeadSize);
        break;
    case ConnectionBaseGraphicsItem::ArrowHeadDirectionEW:
        arrowP1 = basePoint - QPointF(cos(angle - tgt::PI / 3.0) * cbArrowHeadSize          , sin(angle - tgt::PI / 3.0) * cbArrowHeadSize);
        arrowP2 = basePoint - QPointF(cos(angle - tgt::PI + tgt::PI / 3.0) * cbArrowHeadSize, sin(angle - tgt::PI + tgt::PI / 3.0) * cbArrowHeadSize);
        break;
    default:
        tgtAssert(false, "shouldn't get here");
    }
    return QPolygonF() << basePoint << arrowP1 << arrowP2 << basePoint;
}

QPolygonF createArrowHead(QPointF helpPoint, const QPointF& basePoint) {
    double angle = -tgt::PI / 2.0;

    QPointF arrowP1;
    QPointF arrowP2;
        arrowP1 = basePoint + QPointF(sin(angle - tgt::PI / 3.0) * cbArrowHeadSize          , cos(angle - tgt::PI / 3.f) * cbArrowHeadSize);
        arrowP2 = basePoint + QPointF(sin(angle - tgt::PI + tgt::PI / 3.0) * cbArrowHeadSize, cos(angle - tgt::PI + tgt::PI / 3.0) * cbArrowHeadSize);
    
    QPolygonF poly = QPolygonF() << basePoint << arrowP1 << arrowP2 << basePoint;

    QPointF p = helpPoint - basePoint;
    qreal sp = sqrt(p.x()*p.x()+p.y()*p.y());
    if(sp == 0)
        angle= 0;
    else if(helpPoint.x() < basePoint.x())
            angle = - acos(-p.y()/sp);
         else
            angle = acos(-p.y()/sp);

    QMatrix mat;
    mat.translate(basePoint.x(),basePoint.y());
    mat.rotate(angle * 180.f / tgt::PI);
    mat.translate(-basePoint.x(),-basePoint.y());

    return mat.map(poly);
}

    /*********************************************************************
     *                       Core Elements                                
     ********************************************************************/

//-------------------------------------
//      AggregationGraphicsItem        
//-------------------------------------
QRectF NWEStyle_Classic::AggregationGI_boundingRect(const AggregationGraphicsItem* item) const {
    QRectF rect = item->getNameLabel()->boundingRect();

    if (rect.width() < drawingRectMinimumWidth)
        rect.setWidth(drawingRectMinimumWidth);

    if (rect.width() < item->getMinimumWidthForPorts())
        rect.setWidth(item->getMinimumWidthForPorts());

    if (rect.height() < drawingRectMinimumHeight)
        rect.setHeight(drawingRectMinimumHeight);

    if (rect.width() < item->getMinimumHeightForPorts())
        rect.setWidth(item->getMinimumHeightForPorts());

    rect.setWidth(rect.width() + 20.f);

    return rect;
}

QPainterPath NWEStyle_Classic::AggregationGI_shape(const AggregationGraphicsItem* item) const {
    QPainterPath path;
    path.addRect(AggregationGI_boundingRect(item));
    return path;
}

void NWEStyle_Classic::AggregationGI_initializePaintSettings(AggregationGraphicsItem* item) {
    //set text
    item->getNameLabel()->setPlainText(item->getGuiName());
    item->getNameLabel()->setFont(QFont("Helvetica", 10));
    item->getNameLabel()->setDefaultTextColor(Qt::white);
}

void NWEStyle_Classic::AggregationGI_paint(AggregationGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    /*
       8_____________________________7
      /                               \
      1                               6
      |                               |
      |                               |
      |                               |
      |                               |
      |                               |
      2                               5
      \3_____________________________4/
*/
    QRectF br = AggregationGI_boundingRect(item);
    qreal left = br.left();
    qreal right = br.right();
    qreal top = br.top();
    qreal bottom = br.bottom();
    
    QPointF point1(left, top + 20.f);
    QPointF point2(left, bottom - 20.f);
    QPointF point3(left + 20.f, bottom);
    QPointF point4(right - 20.f, bottom);
    QPointF point5(right, bottom - 20.f);
    QPointF point6(right, top + 20.f);
    QPointF point7(right - 20.f, top);
    QPointF point8(left + 20.f, top);

    QPainterPath path;
    path.moveTo(point1);
    path.lineTo(point2);
    path.cubicTo(QPointF(left, bottom), QPointF(left,bottom), point3);
    path.lineTo(point4);
    path.cubicTo(QPointF(right, bottom), QPointF(right, bottom), point5);
    path.lineTo(point6);
    path.cubicTo(QPointF(right, top), QPointF(right, top), point7);
    path.lineTo(point8);
    path.cubicTo(QPointF(left, top), QPointF(left, top), point1);

    painter->setClipPath(path); 

    QColor button_color = QColor(50,50,50,255);

    // frame indicates selected process
    if (option->state & QStyle::State_Selected) {
        button_color = NWEStyle_SelectionColor;
        item->setZValue(ZValuesSelectedPortOwnerGraphicsItem);
    }
    // hover effect
    else if (option->state & QStyle::State_MouseOver){
        button_color = NWEStyle_HoverColor;
        item->setZValue(ZValuesSelectedPortOwnerGraphicsItem);
    }
    else {
       item->setZValue(ZValuesPortOwnerGraphicsItem);
    }

    //gradient
    QLinearGradient gradient(0, 0, 0, br.height());
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, button_color);
    gradient.setColorAt(0.4, Qt::black);
    gradient.setColorAt(0.6, Qt::black);
    gradient.setColorAt(1.0, button_color);

    QBrush brush(gradient);
    painter->setBrush(brush);
    painter->setPen(QPen(QBrush(button_color), 2.0));

    bool hasSpecialProp = false;
    switch(item->currentLayer()){
    case NetworkEditorLayerCameraLinking:
        foreach (Processor* processor, item->getProcessors()){
            if(!processor->getPropertiesByType<CameraProperty>().empty()){
                hasSpecialProp = true;
                break;
            }
        }
        if(hasSpecialProp)
            painter->setOpacity(1.0);
        else
            painter->setOpacity(0.20);
        break;
    case NetworkEditorLayerPortSizeLinking:
        foreach (Port* port, item->getPorts()){
            if(!port->getPropertiesByType<RenderSizeOriginProperty>().empty() ||
               !port->getPropertiesByType<RenderSizeReceiveProperty>().empty()){
                hasSpecialProp = true;
                break;
            }
        }
        if(hasSpecialProp)
            painter->setOpacity(1.0);
        else
            painter->setOpacity(0.20);
        break;
        painter->setOpacity(0.20);
        break;
    default:
        painter->setOpacity(1.0);
        break;
    }

    painter->drawRect(br);

    //glass highlight
    painter->setBrush(QBrush(Qt::white));
    painter->setPen(QPen(QBrush(Qt::white), 0.01));
    painter->setOpacity(0.30);
    br.setHeight(br.height()/2.0);
    painter->drawRect(br);
}

//-------------------------------------
//      PortGraphicsItem                        
//-------------------------------------
QRectF NWEStyle_Classic::PortGI_boundingRect(const PortGraphicsItem* item) const {
    QRectF rect(0,0,10,10);
    if (item->getPort()->allowMultipleConnections() && !item->getPort()->isOutport()) {
        if (dynamic_cast<CoProcessorPort*>(item->getPort()))
            rect.setHeight(rect.height() * 1.75f);
        else
            rect.setWidth(rect.width() * 1.75f);
    }
    return rect;
}

QPainterPath NWEStyle_Classic::PortGI_shape(const PortGraphicsItem* item) const {
     QPainterPath path;
     path.addRect(PortGI_boundingRect(item));
     return path;
}

void NWEStyle_Classic::PortGI_initializePaintSettings(PortGraphicsItem* item) {}

void NWEStyle_Classic::PortGI_paint(PortGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    QColor color = item->getColor();
    painter->setPen(QPen(color, 2));
    painter->setBrush(portColorBrush);
    
    switch(item->currentLayer()){
    case NetworkEditorLayerDataFlow:
        painter->setOpacity(1.0);
        break;
    case NetworkEditorLayerGeneralLinking:
    case NetworkEditorLayerCameraLinking:
        painter->setOpacity(0.2);
        break;
    case NetworkEditorLayerPortSizeLinking:
        if(!item->getPort()->getPropertiesByType<RenderSizeOriginProperty>().empty() || !item->getPort()->getPropertiesByType<RenderSizeReceiveProperty>().empty())
            painter->setOpacity(1.0);
        else
            painter->setOpacity(0.2);
        break;
    default:
        tgtAssert(false,"should not get here");
        break;
    }

    if (option->state & QStyle::State_Sunken)
        painter->setBrush(color.darker(150));
    if (option->state & QStyle::State_MouseOver)
        painter->setBrush(color);

    QRectF br = PortGI_boundingRect(item);

    if (item->getPort()->isLoopPort()) {
        if (item->getPort()->isOutport())
            painter->drawEllipse(br);
        else {
            QPolygonF triangle;
            const QPointF& topLeftPoint = br.topLeft();
            const QPointF bottomPoint = QPointF((br.left() + br.right()) / 2.f, br.bottom());
            const QPointF& topRightPoint = br.topRight();
            triangle << topLeftPoint << bottomPoint << topRightPoint;
            painter->drawConvexPolygon(triangle);
        }
    }
    else {
        painter->drawRect(br);
    }
}

//-------------------------------------
//      ProcessorGraphicsItem                   
//-------------------------------------
QRectF NWEStyle_Classic::ProcessorGI_boundingRect(const ProcessorGraphicsItem* item) const{
    QRectF rect = item->getNameLabel()->boundingRect();

    if (rect.width() < drawingRectMinimumWidth)
        rect.setWidth(drawingRectMinimumWidth);

    if (rect.width() < item->getMinimumWidthForPorts())
        rect.setWidth(item->getMinimumWidthForPorts());

    if (rect.height() < drawingRectMinimumHeight)
        rect.setHeight(drawingRectMinimumHeight);

    if (rect.width() < item->getMinimumHeightForPorts())
        rect.setWidth(item->getMinimumHeightForPorts());

    rect.setWidth(rect.width() + 20.f);

    return rect;
}

QPainterPath NWEStyle_Classic::ProcessorGI_shape(const ProcessorGraphicsItem* item) const{
     QPainterPath path;
     path.addRect(ProcessorGI_boundingRect(item));
     return path;
}

void NWEStyle_Classic::ProcessorGI_initializePaintSettings(ProcessorGraphicsItem* item) {
    //set text
    item->getNameLabel()->setPlainText(item->getGuiName());
    item->getNameLabel()->setFont(QFont("Helvetica", 10));
    item->getNameLabel()->setDefaultTextColor(Qt::white);
}

void NWEStyle_Classic::ProcessorGI_paint(ProcessorGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {

    QRectF button_rect = ProcessorGI_boundingRect(item);

    QColor button_color = QColor(50,50,50,255);

    // frame indicates selected process
    if (option->state & QStyle::State_Selected) {
        button_color = NWEStyle_SelectionColor;
        item->setZValue(ZValuesSelectedPortOwnerGraphicsItem);
    }
    // hover effect
    else if (option->state & QStyle::State_MouseOver){
        button_color = NWEStyle_HoverColor;
        item->setZValue(ZValuesSelectedPortOwnerGraphicsItem);
    }
    else {
       item->setZValue(ZValuesPortOwnerGraphicsItem);
    }

    //gradient
    QLinearGradient gradient(0, 0, 0, button_rect.height());
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, button_color);
    gradient.setColorAt(0.4, Qt::black);
    gradient.setColorAt(0.6, Qt::black);
    gradient.setColorAt(1.0, button_color);

    QBrush brush(gradient);
    painter->setBrush(brush);
    painter->setPen(QPen(QBrush(button_color), 2.0));

    bool hasSpecialProp = false;
    switch(item->currentLayer()){
    case NetworkEditorLayerCameraLinking:
        foreach (Processor* processor, item->getProcessors()){
            if(!processor->getPropertiesByType<CameraProperty>().empty()){
                hasSpecialProp = true;
                break;
            }
        }
        if(hasSpecialProp)
            painter->setOpacity(1.0);
        else
            painter->setOpacity(0.20);
        break;
    case NetworkEditorLayerPortSizeLinking:
        foreach (Port* port, item->getPorts()){
            if(!port->getPropertiesByType<RenderSizeOriginProperty>().empty() ||
               !port->getPropertiesByType<RenderSizeReceiveProperty>().empty()){
                hasSpecialProp = true;
                break;
            }
        }
        if(hasSpecialProp)
            painter->setOpacity(1.0);
        else
            painter->setOpacity(0.20);
        break;
        painter->setOpacity(0.20);
        break;
    default:
        painter->setOpacity(1.0);
        break;
    }

    painter->drawRect(button_rect);

    //glass highlight
    painter->setBrush(QBrush(Qt::white));
    painter->setPen(QPen(QBrush(Qt::white), 0.01));
    painter->setOpacity(0.30);
    button_rect.setHeight(button_rect.height()/2.0);
    painter->drawRect(button_rect);
}

//-------------------------------------
//      PropertyGraphicsItem                    
//-------------------------------------
QRectF NWEStyle_Classic::PropertyGI_boundingRect(const PropertyGraphicsItem* item) const {
    QRectF rect = item->getPropertyLabel()->boundingRect();
    rect.setWidth(item->getBoundingWidth());
    return rect;
}

QPainterPath NWEStyle_Classic::PropertyGI_shape(const PropertyGraphicsItem* item) const {
     QPainterPath path;
     path.addEllipse(PropertyGI_boundingRect(item));
     return path;
}

void NWEStyle_Classic::PropertyGI_initializePaintSettings(PropertyGraphicsItem* item) {
        //set text
        QString labelText = "";
        labelText.append(QString::fromStdString(item->getProperty()->getGuiName()));
        labelText.append("<br><\br>");
        QString type = QString::fromStdString(item->getProperty()->getTypeDescription());

        QString typeLine("<span style=\"font-size:7pt;color:#BDBDBD\"> %1 </span>");
        labelText.append(typeLine.arg(type));
        item->getPropertyLabel()->setHtml(labelText);
        item->resetSize(item->getPropertyLabel()->boundingRect().width());
}

void NWEStyle_Classic::PropertyGI_paint(PropertyGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(Qt::white);
    painter->drawRect(PropertyGI_boundingRect(item));
}

//-------------------------------------
//      PropertyListGraphicsItem                    
//-------------------------------------
QRectF NWEStyle_Classic::PropertyListGI_boundingRect(const PropertyListGraphicsItem* item) const {
    qreal height = 0.0;
    if(item->getParentLabel()->isVisible())
        height += item->getHeaderMode();
    foreach (PropertyGraphicsItem* propertyItem, item->getVisiblePropertyGraphicsItems()) {
        height += propertyItem->boundingRect().height();
    }
    foreach(PropertyListGraphicsItem* listItem, item->getSubPropertyLists()){
        height += listItem->boundingRect().height();
    }
    return QRect(0 ,0 , static_cast<int>(item->getCurrentWidth()), static_cast<int>(height));
}

QPainterPath NWEStyle_Classic::PropertyListGI_shape(const PropertyListGraphicsItem* item) const {
     QPainterPath path;
     path.addEllipse(PropertyListGI_boundingRect(item));
     return path;
}

void NWEStyle_Classic::PropertyListGI_initializePaintSettings(PropertyListGraphicsItem* item) {}

void NWEStyle_Classic::PropertyListGI_paint(PropertyListGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    item->getParentLabel()->setFont(QFont("Helvetica", 10));
    item->getParentLabel()->setDefaultTextColor(Qt::white);
    if(!painter) return;
    if((item->getHeaderMode() != PropertyListGraphicsItem::NO_HEADER) && !item->getVisiblePropertyGraphicsItems().empty()) {
        if(item->getHeaderMode() == PropertyListGraphicsItem::SMALL_HEADER){
            painter->setBrush(QBrush(Qt::black));
            painter->drawRect(0,0,item->getCurrentWidth(), PropertyListGraphicsItem::SMALL_HEADER);
        } else {
            painter->setBrush(QBrush(Qt::black));
            painter->drawRect(0,0,item->getCurrentWidth(), PropertyListGraphicsItem::BIG_HEADER);
            //glass highlight
            painter->setBrush(QBrush(Qt::white));
            painter->setPen(QPen(QBrush(Qt::white), 0.01));
            painter->setOpacity(0.30);
            painter->drawRect(0,0,item->getCurrentWidth(), PropertyListGraphicsItem::BIG_HEADER/2.0);
        }
    }
}

    /*********************************************************************
     *                       Util Elements                                
     ********************************************************************/

//-------------------------------------
//      ProgressBarGraphicsItem 
//-------------------------------------
QRectF NWEStyle_Classic::ProgressBarGI_boundingRect(const ProgressBarGraphicsItem* item) const {
    return QRectF(0, 0, item->getWidth(), item->getHeight());
}

QPainterPath NWEStyle_Classic::ProgressBarGI_shape(const ProgressBarGraphicsItem* item) const {
    QPainterPath path;
    path.addEllipse(ProgressBarGI_boundingRect(item));
    return path;
}

void NWEStyle_Classic::ProgressBarGI_initializePaintSettings(ProgressBarGraphicsItem* item) {}

void NWEStyle_Classic::ProgressBarGI_paint(ProgressBarGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
       /*
        General Clip Path

   c1     1________________________________________________________4      c4
         /                                                          \
        |                                                            |
        |                                                            |
         \__________________________________________________________/
   c2     2                                                        3      c3

        c1-c4: control points for splines constructing the curved edges
    */
    QRectF boundingRect_ = ProgressBarGI_boundingRect(item);
    QPainterPath clipPath;
    const qreal cutOffValue = 5.0;
    const qreal clipRatio = 5.0;
    const QPointF p1 = QPointF(boundingRect_.x() + cutOffValue, boundingRect_.y());
    const QPointF p2 = QPointF(boundingRect_.x() + cutOffValue, boundingRect_.bottom());
    const QPointF p3 = QPointF(boundingRect_.right() - cutOffValue, boundingRect_.bottom());
    const QPointF p4 = QPointF(boundingRect_.right() - cutOffValue, boundingRect_.y());

    const QPointF c1 = QPointF(p1.x() - clipRatio, p1.y());
    const QPointF c2 = QPointF(p2.x() - clipRatio, p2.y());
    const QPointF c3 = QPointF(p3.x() + clipRatio, p3.y());
    const QPointF c4 = QPointF(p4.x() + clipRatio, p4.y());

    clipPath.moveTo(p1);
    clipPath.cubicTo(c1, c2, p2);
    clipPath.lineTo(p3);
    clipPath.cubicTo(c3, c4, p4);
    clipPath.lineTo(p1);
    //clipPath.addRoundedRect(boundingRect_, 4.0, 4.0);
    painter->setClipPath(clipPath);

    // paint the background
    QLinearGradient upperBackgroundGradient(boundingRect_.x(), boundingRect_.y(), boundingRect_.x(), boundingRect_.y() + boundingRect_.height() / 2.0);
    //upperBackgroundGradient.setColorAt(0.0, QColor(163, 163, 163));
    //upperBackgroundGradient.setColorAt(1.0, QColor(208, 208, 208));
    upperBackgroundGradient.setColorAt(0.0, item->getBackgroundColor1());
    upperBackgroundGradient.setColorAt(0.0, item->getBackgroundColor2());

    QBrush upperBackgroundBrush(upperBackgroundGradient);
    painter->setBrush(upperBackgroundBrush);
    painter->setPen(Qt::NoPen);
    painter->drawRect(boundingRect_.x(), boundingRect_.y(), boundingRect_.width(), boundingRect_.height() / 2.0);

    //painter->setBrush(QBrush(Qt::white));
    painter->setBrush(QBrush(Qt::black));
    painter->drawRect(boundingRect_.x(), boundingRect_.y() + boundingRect_.height() / 2.0, boundingRect_.width(), boundingRect_.height() / 2.0);

    // now the progress overlay
    QRectF progressRect = boundingRect_;
    progressRect.setWidth(boundingRect_.width() * item->getProgress());

    QLinearGradient upperForegroundGradient(progressRect.x(), progressRect.y(), progressRect.x(), progressRect.y() + progressRect.height() / 2.0);
    upperForegroundGradient.setColorAt(0.0, item->getUpperForegroundColor1());
    upperForegroundGradient.setColorAt(1.0, item->getUpperForegroundColor2());

    QBrush upperForegroundBrush(upperForegroundGradient);
    painter->setBrush(upperForegroundBrush);
    painter->drawRect(progressRect.x(), progressRect.y(), progressRect.width(), progressRect.height() / 2.0 + 1.0);

    QBrush lowerForegroundBrush(item->getLowerForegroundColor());
    painter->setBrush(lowerForegroundBrush);
    painter->drawRect(progressRect.x(), progressRect.y() + progressRect.height() / 2.0, progressRect.width(), progressRect.height() / 2.0);

}

//-------------------------------------
//      PropertyListButtonGraphicsItem 
//-------------------------------------
QRectF NWEStyle_Classic::PropertyListButtonGI_boundingRect(const PropertyListButtonGraphicsItem* item) const {
    return QRect(0, 0, 10, 10);
}

QPainterPath NWEStyle_Classic::PropertyListButtonGI_shape(const PropertyListButtonGraphicsItem* item) const {
    QPainterPath path;
    path.addEllipse(PropertyListButtonGI_boundingRect(item));
    return path;
}

void NWEStyle_Classic::PropertyListButtonGI_initializePaintSettings(PropertyListButtonGraphicsItem* item) {}

void NWEStyle_Classic::PropertyListButtonGI_paint(PropertyListButtonGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    // add gloss effect
    QRectF glossArea = PropertyListButtonGI_boundingRect(item);
    glossArea.setLeft(glossArea.left() + glossArea.width() * 0.05);
    glossArea.setHeight(glossArea.height() * 0.5);
    glossArea.setWidth(glossArea.width() * 0.9);
    painter->setPen(QPen(QColor(0,0,0,0), 0));
    painter->setBrush(QColor(255,255,255,128));
    painter->drawEllipse(glossArea);

    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(QColor(255,255,255,128));

    if (option->state & QStyle::State_Sunken)
        painter->setBrush(QColor(170,100,100,128));
    if (option->state & QStyle::State_MouseOver)
        painter->setBrush(QColor(150,100,100,128));

    painter->drawEllipse(PropertyListButtonGI_boundingRect(item));
}

//-------------------------------------
//      WidgetToggleButtonGraphicsItem 
//-------------------------------------
QRectF NWEStyle_Classic::WidgetToggleButtonGI_boundingRect(const WidgetToggleButtonGraphicsItem* item) const {
    return QRect(0, 0, 12, 8);
}

QPainterPath NWEStyle_Classic::WidgetToggleButtonGI_shape(const WidgetToggleButtonGraphicsItem* item) const {
    QPainterPath path;
    path.addRect(WidgetToggleButtonGI_boundingRect(item));
    return path;
}

void NWEStyle_Classic::WidgetToggleButtonGI_initializePaintSettings(WidgetToggleButtonGraphicsItem* item) {}

void NWEStyle_Classic::WidgetToggleButtonGI_paint(WidgetToggleButtonGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setPen(QPen(Qt::black, 3.5));
    painter->drawRect(WidgetToggleButtonGI_boundingRect(item));

    if (option->state & QStyle::State_MouseOver)
        painter->setPen(QPen(QColor(Qt::darkGray).darker(), 2));
    else
        painter->setPen(QPen(Qt::darkGray, 2));

    int visibleWidgetNum = 0;
    foreach (ProcessorWidget* widget, item->getWidgets()) {
        if (widget->isVisible())
            visibleWidgetNum++;
    }

    if (visibleWidgetNum > 0) {
        if (visibleWidgetNum == item->getWidgets().size())
            painter->setBrush(QColor(160, 241, 164, 175));
        else
            painter->setBrush(QColor(254, 196, 79, 175));
    }

    painter->drawRect(WidgetToggleButtonGI_boundingRect(item));
}

    /*********************************************************************
     *                       Connection Elements                          
     ********************************************************************/

//-------------------------------------
//      port arrow                     
//-------------------------------------
QRectF NWEStyle_Classic::PortArrowGI_boundingRect(const PortArrowGraphicsItem* item) const {
    QRectF rect = PortArrowGI_shape(item).boundingRect();
    // add a few extra pixels for the arrow and the pen
    if(item->getSourceItem()->getPort()->isLoopPort())
        return rect;

    qreal extra = (1 + cbArrowHeadSize) / 2.0;

    if (dynamic_cast<CoProcessorPort*>(item->getSourceItem()->getPort()))
            rect.adjust(0, -extra, 0, extra);
        else
            rect.adjust(-extra, 0, extra, 0);

    rect.adjust(-extra, -extra, extra, extra);

    return rect;
}

QPainterPath NWEStyle_Classic::PortArrowGI_shape(const PortArrowGraphicsItem* item) const {
    QPointF s = item->getSourcePoint();
    QPointF d = item->getDestinationPoint();

    QPainterPath path(s);
    qreal defl = fabs(s.y() - d.y()) / 2.f + fabs(s.x() - d.x()) / 2.f;;
    PortGraphicsItem* portItem = item->getSourceItem();

    qreal p = portArrowHoverSize;
    
    if (dynamic_cast<CoProcessorPort*>(portItem->getPort())) {
        d.setX(d.x()-cbArrowHeadSize);
        if(d.x() > s.x()){
            if(d.y() < s.y()) {
                    path.moveTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                    path.cubicTo(QPointF(s.x()-portArrowHoverSize+defl/2.0, s.y()-portArrowHoverSize),
                             QPointF(d.x()-portArrowHoverSize-defl/2.0, d.y()-portArrowHoverSize),
                             QPointF(d.x()-portArrowHoverSize, d.y()-portArrowHoverSize));
                    path.lineTo(d.x()+portArrowHoverSize,d.y()-portArrowHoverSize);
                    path.lineTo(d.x()+portArrowHoverSize,d.y()+portArrowHoverSize);
                    path.cubicTo(QPointF(d.x()+portArrowHoverSize-defl/2.0, d.y()+portArrowHoverSize),
                                QPointF(s.x()+portArrowHoverSize+defl/2.0, s.y()+portArrowHoverSize),
                                QPointF(s.x()+portArrowHoverSize, s.y()+portArrowHoverSize));
                    path.lineTo(s.x()-portArrowHoverSize,s.y()+portArrowHoverSize);
                    path.lineTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                }else {
                    path.moveTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                    path.lineTo(s.x()+portArrowHoverSize,s.y()-portArrowHoverSize);
                    path.cubicTo(QPointF(s.x()+portArrowHoverSize+defl/2.0, s.y()-portArrowHoverSize),
                             QPointF(d.x()+portArrowHoverSize-defl/2.0, d.y()-portArrowHoverSize),
                             QPointF(d.x()+portArrowHoverSize, d.y()-portArrowHoverSize));
                    path.lineTo(d.x()+portArrowHoverSize,d.y()+portArrowHoverSize);                
                    path.lineTo(d.x()-portArrowHoverSize,d.y()+portArrowHoverSize);                
                    path.cubicTo(QPointF(d.x()-portArrowHoverSize-defl/2.0, d.y()+portArrowHoverSize),
                                QPointF(s.x()-portArrowHoverSize+defl/2.0, s.y()+portArrowHoverSize),
                                QPointF(s.x()-portArrowHoverSize, s.y()+portArrowHoverSize));
                    path.lineTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                }
            } else 
               if(d.y() > s.y()) {
                    path.moveTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                    path.cubicTo(QPointF(s.x()-portArrowHoverSize+defl/2.0, s.y()-portArrowHoverSize),
                             QPointF(d.x()-portArrowHoverSize-defl/2.0, d.y()-portArrowHoverSize),
                             QPointF(d.x()-portArrowHoverSize, d.y()-portArrowHoverSize));
                    path.lineTo(d.x()+portArrowHoverSize,d.y()-portArrowHoverSize);
                    path.lineTo(d.x()+portArrowHoverSize,d.y()+portArrowHoverSize);
                    path.cubicTo(QPointF(d.x()+portArrowHoverSize-defl/2.0, d.y()+portArrowHoverSize),
                                QPointF(s.x()+portArrowHoverSize+defl/2.0, s.y()+portArrowHoverSize),
                                QPointF(s.x()+portArrowHoverSize, s.y()+portArrowHoverSize));
                    path.lineTo(s.x()-portArrowHoverSize,s.y()+portArrowHoverSize);
                    path.lineTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                }else {
                    path.moveTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                    path.lineTo(s.x()+portArrowHoverSize,s.y()-portArrowHoverSize);
                    path.cubicTo(QPointF(s.x()+portArrowHoverSize+defl/2.0, s.y()-portArrowHoverSize),
                             QPointF(d.x()+portArrowHoverSize-defl/2.0, d.y()-portArrowHoverSize),
                             QPointF(d.x()+portArrowHoverSize, d.y()-portArrowHoverSize));
                    path.lineTo(d.x()+portArrowHoverSize,d.y()+portArrowHoverSize);
                    path.lineTo(d.x()-portArrowHoverSize,d.y()+portArrowHoverSize);
                    path.cubicTo(QPointF(d.x()-portArrowHoverSize-defl/2.0, d.y()+portArrowHoverSize),
                                QPointF(s.x()-portArrowHoverSize+defl/2.0, s.y()+portArrowHoverSize),
                                QPointF(s.x()-portArrowHoverSize, s.y()+portArrowHoverSize));
                    path.lineTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                }
    }
    else {
        Port* sourcePort = item->getSourceItem()->getPort();

        if (sourcePort->isLoopPort()) {
            defl *= 0.9;
            path.moveTo(s.x()-portArrowHoverSize,s.y());
            path.cubicTo(QPointF(s.x()-portArrowHoverSize+defl, s.y()+defl+5.0),
                            QPointF(d.x()-portArrowHoverSize+defl, d.y()-defl),
                            QPointF(d.x()-portArrowHoverSize, d.y()));
            path.lineTo(d.x()+portArrowHoverSize,d.y());
            path.cubicTo(QPointF(d.x()+portArrowHoverSize+defl, d.y()-defl),
                            QPointF(s.x()+portArrowHoverSize+defl, s.y()+defl+5.0),
                            QPointF(s.x()+portArrowHoverSize, s.y()));
            path.lineTo(s.x()-portArrowHoverSize,s.y());
        }
        else if(false) { //bundeled_
            /*QList<QPointF> bundlePoints = bundle_->getBundlePoints();

            float xoff = 5.f * (bundle_->getArrowIndex(this) - bundle_->arrowList_.size() / 2);
            for(int i = 0; i < bundlePoints.size(); i++)
                bundlePoints[i].rx() += xoff;
            float distS = 0.5f * (fabs(bundlePoints.front().x() + xoff - s.x()) + fabs(bundlePoints.front().y() - s.y()));

            path.cubicTo(s + QPointF(0.f, distS / 2.f),
                         bundlePoints.front() - QPointF(0.f, distS / 2.f),
                         bundlePoints.front());

            for(int i = 0; i < bundlePoints.size() - 1; i++) {
                QPointF bS = bundlePoints.at(i);
                QPointF bD = bundlePoints.at(i + 1);
                float distB = 0.5f * (fabs(bD.x() - bS.x()) + fabs(bD.y() - bS.y()));
                path.cubicTo(
                             bS + QPointF(0, distB /2.f),
                             bD - QPointF(0, distB /2.f),
                             bD);
            }

            float distD = 0.5f * (fabs(bundlePoints.back().x() - d.x()) + fabs(bundlePoints.back().y() - d.y()));
            path.cubicTo(bundlePoints.back() + QPointF(0, distD /2.0),
                         d - QPointF(0, distD /2.0),
                         d - QPointF(0, arrowHeadSize_));
            path.lineTo(d);*/
        }
        else {
            d.setY(d.y()-cbArrowHeadSize);
            if(d.y() > s.y()){
                if(d.x() < s.x()) {
                    path.moveTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                    path.cubicTo(QPointF(s.x()-portArrowHoverSize, s.y()+defl/2.0-portArrowHoverSize),
                             QPointF(d.x()-portArrowHoverSize, d.y()-defl/2.0-portArrowHoverSize),
                             QPointF(d.x()-portArrowHoverSize, d.y()-portArrowHoverSize));
                    path.lineTo(d.x()-portArrowHoverSize,d.y()+portArrowHoverSize);
                    path.lineTo(d.x()+portArrowHoverSize,d.y()+portArrowHoverSize);
                    path.cubicTo(QPointF(d.x()+portArrowHoverSize, d.y()-defl/2.0+portArrowHoverSize),
                                QPointF(s.x()+portArrowHoverSize, s.y()+defl/2.0+portArrowHoverSize),
                                QPointF(s.x()+portArrowHoverSize, s.y()+portArrowHoverSize));
                    path.lineTo(s.x()+portArrowHoverSize,s.y()-portArrowHoverSize);
                    path.lineTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                }else {
                    path.moveTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                    path.lineTo(s.x()-portArrowHoverSize,s.y()+portArrowHoverSize);
                    path.cubicTo(QPointF(s.x()-portArrowHoverSize, s.y()+defl/2.0+portArrowHoverSize),
                             QPointF(d.x()-portArrowHoverSize, d.y()-defl/2.0+portArrowHoverSize),
                             QPointF(d.x()-portArrowHoverSize, d.y()+portArrowHoverSize));
                    path.lineTo(d.x()+portArrowHoverSize,d.y()+portArrowHoverSize);
                    path.lineTo(d.x()+portArrowHoverSize,d.y()-portArrowHoverSize);                
                    path.cubicTo(QPointF(d.x()+portArrowHoverSize, d.y()-defl/2.0-portArrowHoverSize),
                                QPointF(s.x()+portArrowHoverSize, s.y()+defl/2.0-portArrowHoverSize),
                                QPointF(s.x()+portArrowHoverSize, s.y()-portArrowHoverSize));
                    path.lineTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                }
            } else 
                if(d.x() < s.x()) {
                    path.moveTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                    path.lineTo(s.x()-portArrowHoverSize,s.y()+portArrowHoverSize);
                    path.cubicTo(QPointF(s.x()-portArrowHoverSize, s.y()+defl/2.0+portArrowHoverSize),
                             QPointF(d.x()-portArrowHoverSize, d.y()-defl/2.0+portArrowHoverSize),
                             QPointF(d.x()-portArrowHoverSize, d.y()+portArrowHoverSize));
                    path.lineTo(d.x()+portArrowHoverSize,d.y()+portArrowHoverSize);
                    path.lineTo(d.x()+portArrowHoverSize,d.y()-portArrowHoverSize);
                    path.cubicTo(QPointF(d.x()+portArrowHoverSize, d.y()-defl/2.0-portArrowHoverSize),
                                QPointF(s.x()+portArrowHoverSize, s.y()+defl/2.0-portArrowHoverSize),
                                QPointF(s.x()+portArrowHoverSize, s.y()-portArrowHoverSize));
                    path.lineTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                }else {
                    path.moveTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                    path.cubicTo(QPointF(s.x()-portArrowHoverSize, s.y()+defl/2.0-portArrowHoverSize),
                             QPointF(d.x()-portArrowHoverSize, d.y()-defl/2.0-portArrowHoverSize),
                             QPointF(d.x()-portArrowHoverSize, d.y()-portArrowHoverSize));
                    path.lineTo(d.x()-portArrowHoverSize,d.y()+portArrowHoverSize);
                    path.lineTo(d.x()+portArrowHoverSize,d.y()+portArrowHoverSize);
                    path.cubicTo(QPointF(d.x()+portArrowHoverSize, d.y()-defl/2.0+portArrowHoverSize),
                                QPointF(s.x()+portArrowHoverSize, s.y()+defl/2.0+portArrowHoverSize),
                                QPointF(s.x()+portArrowHoverSize, s.y()+portArrowHoverSize));
                    path.lineTo(s.x()+portArrowHoverSize,s.y()-portArrowHoverSize);
                    path.lineTo(s.x()-portArrowHoverSize,s.y()-portArrowHoverSize);
                }
        }
    }
    //adding arrowhead
    if (item->getDestinationArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
        QPolygonF poly = createArrowHead(item->getDestinationArrowHeadDirection(), item->getDestinationPoint());
        path.moveTo(poly.at(0));
        path.addPolygon(poly);
    }

    return path;
}

void NWEStyle_Classic::PortArrowGI_initializePaintSettings(const PortArrowGraphicsItem* item) {}

void NWEStyle_Classic::PortArrowGI_paint(PortArrowGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    QColor tmpColor;

    switch (item->getColorConnectableMode()) {
    case ConnectionBaseGraphicsItem::CCM_DEFAULT:
        if (item->isSelected()){
            tmpColor = NWEStyle_SelectionColor;
            item->setZValue(ZValuesSelectedPortArrowGraphicsItem);
        } else {
            tmpColor = Qt::black;
            item->setZValue(ZValuesPortArrowGraphicsItem);
        }

        if ((option->state & QStyle::State_MouseOver) || item->getIsHovered()) {
            if (tmpColor == Qt::black) {    // Qt is unable to brighten up Qt::black
                tmpColor = NWEStyle_HoverColor;
                item->setZValue(ZValuesSelectedPortArrowGraphicsItem);
            } else {
                tmpColor = tmpColor.light();
            }
        }
        break;
    case ConnectionBaseGraphicsItem::CCM_NO:
        tmpColor = NWEStyle_ConnectionNo;
        break;
    case ConnectionBaseGraphicsItem::CCM_MAYBE:
        tmpColor = NWEStyle_ConnectionMaybe;
        break;
    case ConnectionBaseGraphicsItem::CCM_YES:
        tmpColor = NWEStyle_ConnectionYes;
        break;
    }

     if(item->currentLayer() == NetworkEditorLayerDataFlow)
        painter->setOpacity(1.0);
    else
        painter->setOpacity(0.2);

    QPointF s = item->getSourcePoint();
    QPointF d = item->getDestinationPoint();
    qreal defl = fabs(s.y() - d.y()) / 2.f + fabs(s.x() - d.x()) / 2.f;
    QPainterPath path(s);

    //draw shape
    if (dynamic_cast<CoProcessorPort*>(item->getSourceItem()->getPort())) {
        path.cubicTo(QPointF(s.x()+defl/2.0, s.y()),
                     QPointF(d.x()-defl/2.0-cbArrowHeadSize, d.y()),
                     QPointF(d.x()-cbArrowHeadSize,d.y()));
    }
    else {
        Port* sourcePort = item->getSourceItem()->getPort();

        if (sourcePort->isLoopPort()) {
            defl *= 0.9;
            path.cubicTo(QPointF(s.x()+defl, s.y()+ defl + 5.0),
                         QPointF(d.x()+defl, d.y()-defl),
                         d );
        }
        else if(false) { //bundeled_
            /*QList<QPointF> bundlePoints = bundle_->getBundlePoints();

            float xoff = 5.f * (bundle_->getArrowIndex(this) - bundle_->arrowList_.size() / 2);
            for(int i = 0; i < bundlePoints.size(); i++)
                bundlePoints[i].rx() += xoff;
            float distS = 0.5f * (fabs(bundlePoints.front().x() + xoff - s.x()) + fabs(bundlePoints.front().y() - s.y()));

            path.cubicTo(s + QPointF(0.f, distS / 2.f),
                         bundlePoints.front() - QPointF(0.f, distS / 2.f),
                         bundlePoints.front());

            for(int i = 0; i < bundlePoints.size() - 1; i++) {
                QPointF bS = bundlePoints.at(i);
                QPointF bD = bundlePoints.at(i + 1);
                float distB = 0.5f * (fabs(bD.x() - bS.x()) + fabs(bD.y() - bS.y()));
                path.cubicTo(
                             bS + QPointF(0, distB /2.f),
                             bD - QPointF(0, distB /2.f),
                             bD);
            }

            float distD = 0.5f * (fabs(bundlePoints.back().x() - d.x()) + fabs(bundlePoints.back().y() - d.y()));
            path.cubicTo(bundlePoints.back() + QPointF(0, distD /2.0),
                         d - QPointF(0, distD /2.0),
                         d - QPointF(0, arrowHeadSize_));
            path.lineTo(d);*/
        }
        else {
            path.cubicTo(QPointF(s.x(), s.y()+defl/2.0),
                     QPointF(d.x(), d.y()-cbArrowHeadSize-defl/2.0),
                     QPointF(d.x(), d.y()-cbArrowHeadSize));
        }
    }
    //draw path
    painter->setPen(QPen(tmpColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawPath(path);
    //draw head
    if (item->getDestinationArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone){
        painter->setBrush(QBrush(tmpColor));
        painter->drawPolygon(createArrowHead(item->getDestinationArrowHeadDirection(), item->getDestinationPoint()));
    }
}

//-------------------------------------
//      PortOwnerLinkArrowGraphicsItem          
//-------------------------------------
QRectF NWEStyle_Classic::PortOwnerLinkArrowGI_boundingRect(const PortOwnerLinkArrowGraphicsItem* item) const {
    return PortOwnerLinkArrowGI_shape(item).boundingRect();
}

QPainterPath NWEStyle_Classic::PortOwnerLinkArrowGI_shape(const PortOwnerLinkArrowGraphicsItem* item) const {
    QPointF sourcePoint = item->getSourcePoint();
    QPointF destinationPoint = item->getDestinationPoint();

    qreal defl = fabs(sourcePoint.y() - destinationPoint.y()) / 2.0 + fabs(sourcePoint.x() - destinationPoint.x()) / 2.0;

    QPainterPath path(sourcePoint);

    if (item->getSourceItem() == item->getDestinationItem()) {
        path.cubicTo(QPointF(sourcePoint.x() - 20.f, sourcePoint.y() - 50.f),
                     QPointF(destinationPoint.x() + 20.f, destinationPoint.y() - 50.f),
                     QPointF(destinationPoint.x(), destinationPoint.y()));
        path.lineTo(destinationPoint);
    } else {
        path.lineTo(QPointF(sourcePoint.x(), sourcePoint.y() - 10.f));
        path.cubicTo(QPointF(sourcePoint.x(), sourcePoint.y() - defl / 2.f),
                     QPointF(destinationPoint.x(), destinationPoint.y() - defl / 2.f),
                     QPointF(destinationPoint.x(), destinationPoint.y() - 10.f));
        path.lineTo(destinationPoint);
    }

    return path;
}

void NWEStyle_Classic::PortOwnerLinkArrowGI_initializePaintSettings(PortOwnerLinkArrowGraphicsItem* item) {}

void NWEStyle_Classic::PortOwnerLinkArrowGI_paint(PortOwnerLinkArrowGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {

    QColor tmpColor;

    switch (item->getColorConnectableMode()) {
    case ConnectionBaseGraphicsItem::CCM_DEFAULT:
        if (item->isSelected())
            tmpColor = NWEStyle_SelectionColor;
        else
            tmpColor = Qt::lightGray;

        if (option->state & QStyle::State_MouseOver) {
            if (tmpColor == Qt::lightGray)
                tmpColor = NWEStyle_HoverColor;
            else
                tmpColor = tmpColor.light();
        }
        break;
    case ConnectionBaseGraphicsItem::CCM_NO:
        tmpColor = NWEStyle_ConnectionNo;
        break;
    case ConnectionBaseGraphicsItem::CCM_MAYBE:
        tmpColor = NWEStyle_ConnectionMaybe;
        break;
    case ConnectionBaseGraphicsItem::CCM_YES:
        tmpColor = NWEStyle_ConnectionYes;
        break;
    }

    QPointF sourcePoint = item->getSourcePoint();
    QPointF destinationPoint = item->getDestinationPoint();

    qreal defl = fabs(sourcePoint.y() - destinationPoint.y()) / 2.0 + fabs(sourcePoint.x() - destinationPoint.x()) / 2.0;

    QPainterPath path(sourcePoint);

    if (item->getSourceItem() == item->getDestinationItem()) {
        path.cubicTo(QPointF(sourcePoint.x() - 20.f, sourcePoint.y() - 50.f),
                     QPointF(destinationPoint.x() + 20.f, destinationPoint.y() - 50.f),
                     QPointF(destinationPoint.x(), destinationPoint.y()));
        path.lineTo(destinationPoint);
    } else {
        path.lineTo(QPointF(sourcePoint.x(), sourcePoint.y() - 10.f));
        path.cubicTo(QPointF(sourcePoint.x(), sourcePoint.y() - defl / 2.f),
                     QPointF(destinationPoint.x(), destinationPoint.y() - defl / 2.f),
                     QPointF(destinationPoint.x(), destinationPoint.y() - 10.f));
        path.lineTo(destinationPoint);
    }

    painter->setPen(QPen(tmpColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawPath(path);
}

//-------------------------------------
//      PropertyLinkArrowrGraphicsItem 
//-------------------------------------
QRectF NWEStyle_Classic::PropertyLinkArrowGI_boundingRect(const PropertyLinkArrowGraphicsItem* item) const {
    return PropertyLinkArrowGI_shape(item).boundingRect();
}

QPainterPath NWEStyle_Classic::PropertyLinkArrowGI_shape(const PropertyLinkArrowGraphicsItem* item) const {
    PropertyLinkArrowGraphicsItem::DockPositions dp = item->getDockPositions();
    QPointF s = item->getSourcePoint();
    QPointF d = item->getDestinationPoint();
    QPainterPath path(s);
    qreal defl = fabs(s.y() - d.y()) / 2.f + fabs(s.x() - d.x()) / 2.f;
    qreal p = portArrowHoverSize;

    switch (dp) {
    case PropertyLinkArrowGraphicsItem::DockPositionsLeftLeft:
        //setPos
        s.setX(s.x() - cbArrowHeadSize);
        d.setX(d.x() - cbArrowHeadSize);
        //src
        if (item->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            QPolygonF poly = createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionWE, item->getSourcePoint());
            path.addPolygon(poly);
            path.moveTo(s);
        } else {
            path.addRect(s.x(), s.y() - p, cbArrowHeadSize, 2*p);
            path.lineTo(s);
        }
        //curve
        if(s.y() >= d.y()){
            s.setY(s.y()-p);
            d.setY(d.y()+p);
            path.moveTo(s);
            path.cubicTo(QPointF(s.x()+p - defl, s.y()),
                         QPointF(d.x()+p - defl, d.y()),
                         d);
            s.setY(s.y()+2*p);
            d.setY(d.y()-2*p);
            path.lineTo(d);
            path.cubicTo(QPointF(d.x()-p - defl, d.y()),
                         QPointF(s.x()-p - defl, s.y()),
                         s);
            path.lineTo(s.x(),s.y()-2*p);
            path.moveTo(d.x(),d.y()+p);
        } else {
            s.setY(s.y()+p);
            d.setY(d.y()-p);
            path.moveTo(s);
            path.cubicTo(QPointF(s.x()+p - defl, s.y()),
                         QPointF(d.x()+p - defl, d.y()),
                         d);
            s.setY(s.y()-2*p);
            d.setY(d.y()+2*p);
            path.lineTo(d);
            path.cubicTo(QPointF(d.x()-p - defl, d.y()),
                         QPointF(s.x()-p - defl, s.y()),
                         s);
            path.lineTo(s.x(),s.y()+2*p);
            path.moveTo(d.x(),d.y()-p);
        }
        //dst
        if (item->getDestinationArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            QPolygonF poly = createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionWE, item->getDestinationPoint());
            path.moveTo(poly.at(0));
            path.addPolygon(poly);
        } else {
            path.lineTo(item->getDestinationPoint());
        }
        break;
    case PropertyLinkArrowGraphicsItem::DockPositionsLeftRight:
        //setPos
        s.setX(s.x() - cbArrowHeadSize);
        d.setX(d.x() + cbArrowHeadSize);
        //src
        if (item->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            QPolygonF poly = createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionWE, item->getSourcePoint());
            path.addPolygon(poly);
            path.moveTo(s);
        } else {
            path.addRect(s.x(),s.y()-p,cbArrowHeadSize,2*p);
            path.lineTo(s);
        }
        //curve
        if(s.y() >= d.y()){
            s.setY(s.y()-p);
            d.setY(d.y()-p);
            path.moveTo(s);
            path.cubicTo(QPointF(s.x()+2*p - defl, s.y()),
                         QPointF(d.x()+2*p + defl, d.y()),
                         d);
            s.setY(s.y()+2*p);
            d.setY(d.y()+2*p);
            path.lineTo(d);
            path.cubicTo(QPointF(d.x()-2*p + defl, d.y()),
                         QPointF(s.x()-2*p - defl, s.y()),
                         s);
            path.lineTo(s.x(),s.y()-2*p);
            path.moveTo(d.x(),d.y()-p);
        } else {
            s.setY(s.y()-p);
            d.setY(d.y()-p);
            path.moveTo(s);
            path.cubicTo(QPointF(s.x()-2*p - defl, s.y()),
                         QPointF(d.x()-2*p + defl, d.y()),
                         d);
            s.setY(s.y()+2*p);
            d.setY(d.y()+2*p);
            path.lineTo(d);
            path.cubicTo(QPointF(d.x()+2*p + defl, d.y()),
                         QPointF(s.x()+2*p - defl, s.y()),
                         s);
            path.lineTo(s.x(),s.y()-2*p);
            path.moveTo(d.x(),d.y()-p);
        }
        //dst
        if (item->getDestinationArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            QPolygonF poly = createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW, item->getDestinationPoint());
            path.moveTo(poly.at(0));
            path.addPolygon(poly);
        } else {
            path.lineTo(item->getDestinationPoint());
        }
        break;
    case PropertyLinkArrowGraphicsItem::DockPositionsRightLeft:
        //setPos
        s.setX(s.x() + cbArrowHeadSize);
        d.setX(d.x() - cbArrowHeadSize);
        //src
        if (item->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            QPolygonF poly = createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW, item->getSourcePoint());
            path.addPolygon(poly);
            path.moveTo(s);
        } else {
            path.addRect(s.x()-cbArrowHeadSize,s.y()-p,cbArrowHeadSize,2*p);
            path.moveTo(s);
        }
        //curve
        if(s.y() >= d.y()){
            s.setY(s.y()-p);
            d.setY(d.y()-p);
            path.moveTo(s);
            path.cubicTo(QPointF(s.x()-2*p + defl, s.y()),
                         QPointF(d.x()-2*p - defl, d.y()),
                         d);
            s.setY(s.y()+2*p);
            d.setY(d.y()+2*p);
            path.lineTo(d);
            path.cubicTo(QPointF(d.x()+2*p - defl, d.y()),
                         QPointF(s.x()+2*p + defl, s.y()),
                         s);
            path.lineTo(s.x(),s.y()-2*p);
            path.moveTo(d.x(),d.y()-p);
        } else {
            s.setY(s.y()-p);
            d.setY(d.y()-p);
            path.moveTo(s);
            path.cubicTo(QPointF(s.x()+2*p + defl, s.y()),
                         QPointF(d.x()+2*p - defl, d.y()),
                         d);
            s.setY(s.y()+2*p);
            d.setY(d.y()+2*p);
            path.lineTo(d);
            path.cubicTo(QPointF(d.x()-2*p - defl, d.y()),
                         QPointF(s.x()-2*p + defl, s.y()),
                         s);
            path.lineTo(s.x(),s.y()-2*p);
            path.moveTo(d.x(),d.y()-p);
        }
        //dst
        if (item->getDestinationArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            QPolygonF poly = createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionWE, item->getDestinationPoint());
            path.moveTo(poly.at(0));
            path.addPolygon(poly);
        } else {
            path.lineTo(item->getDestinationPoint());
        }
        break;
    case PropertyLinkArrowGraphicsItem::DockPositionsRightRight:
        //setPos
        s.setX(s.x() + cbArrowHeadSize);
        d.setX(d.x() + cbArrowHeadSize);
        //src
        if (item->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            QPolygonF poly = createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW, item->getSourcePoint());
            path.addPolygon(poly);
            path.moveTo(s);
        } else {
            path.addRect(s.x()-cbArrowHeadSize,s.y()-p,cbArrowHeadSize,2*p);
            path.moveTo(s);
        }
        //curve
       if(s.y() <= d.y()){
            s.setY(s.y()-p);
            d.setY(d.y()+p);
            path.moveTo(s);
            path.cubicTo(QPointF(s.x()+p + defl, s.y()),
                         QPointF(d.x()+p + defl, d.y()),
                         d);
            s.setY(s.y()+2*p);
            d.setY(d.y()-2*p);
            path.lineTo(d);
            path.cubicTo(QPointF(d.x()-p + defl, d.y()),
                         QPointF(s.x()-p + defl, s.y()),
                         s);
            path.lineTo(s.x(),s.y()-2*p);
            path.moveTo(d.x(),d.y()+p);
        } else {
            s.setY(s.y()+p);
            d.setY(d.y()-p);
            path.moveTo(s);
            path.cubicTo(QPointF(s.x()+p + defl, s.y()),
                         QPointF(d.x()+p + defl, d.y()),
                         d);
            s.setY(s.y()-2*p);
            d.setY(d.y()+2*p);
            path.lineTo(d);
            path.cubicTo(QPointF(d.x()-p + defl, d.y()),
                         QPointF(s.x()-p + defl, s.y()),
                         s);
            path.lineTo(s.x(),s.y()+2*p);
            path.moveTo(d.x(),d.y()-p);
        }
        //dst
        if (item->getDestinationArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            QPolygonF poly = createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW, item->getDestinationPoint());
            path.moveTo(poly.at(0));
            path.addPolygon(poly);
        } else {
            path.lineTo(item->getDestinationPoint());
        }
        break;
    default:
        tgtAssert(false, "shouldn't get here");
    }

    return path;
}

void NWEStyle_Classic::PropertyLinkArrowGI_initializePaintSettings(const PropertyLinkArrowGraphicsItem* item) {}

void NWEStyle_Classic::PropertyLinkArrowGI_paint(PropertyLinkArrowGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    
    QColor tmpColor = Qt::lightGray;

    switch (item->getColorConnectableMode()) {
    case ConnectionBaseGraphicsItem::CCM_DEFAULT:
        if (item->isSelected()){
            tmpColor = NWEStyle_SelectionColor;
            item->setZValue(ZValuesSelectedPropertyLinkArrowGraphicsItem);
        } else {
            tmpColor = Qt::lightGray;
            item->setZValue(ZValuesPropertyLinkArrowGraphicsItem);
        }

        if (option->state & QStyle::State_MouseOver) {
            if (tmpColor == Qt::lightGray)
                tmpColor = NWEStyle_HoverColor;
            else
                tmpColor = tmpColor.light();
        }

        if(item->getIsHovered()){
            item->setZValue(ZValuesSelectedPropertyLinkArrowGraphicsItem);
            tmpColor = NWEStyle_HoverColor;
        }

        break;
    case ConnectionBaseGraphicsItem::CCM_NO:
        tmpColor = NWEStyle_ConnectionNo;
        break;
    case ConnectionBaseGraphicsItem::CCM_MAYBE:
        tmpColor = NWEStyle_ConnectionMaybe;
        break;
    case ConnectionBaseGraphicsItem::CCM_YES:
        tmpColor = NWEStyle_ConnectionYes;
        break;
    }

    painter->setPen(QPen(tmpColor,2));
    painter->setBrush(QBrush(tmpColor));

    PropertyLinkArrowGraphicsItem::DockPositions dp = item->getDockPositions();
    QPointF s = item->getSourcePoint();
    QPointF d = item->getDestinationPoint();
    QPainterPath path(s);
    qreal defl = fabs(s.y() - d.y()) / 2.f + fabs(s.x() - d.x()) / 2.f;

    switch (dp) {
    case PropertyLinkArrowGraphicsItem::DockPositionsLeftLeft:
        //setPos
        s.setX(s.x() - cbArrowHeadSize);
        d.setX(d.x() - cbArrowHeadSize);
        //src
        if (item->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            painter->drawPolygon(createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionWE, item->getSourcePoint()));
            path.moveTo(s);
        } else {
            path.lineTo(s);
        }
        //curve
        path.cubicTo(QPointF(s.x() - defl, s.y()),
                     QPointF(d.x() - defl, d.y()),
                     d);
        //dst
        if (item->getDestinationArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            painter->drawPolygon(createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionWE, item->getDestinationPoint()));
        } else {
            path.lineTo(item->getDestinationPoint());
        }
        break;
    case PropertyLinkArrowGraphicsItem::DockPositionsLeftRight:
        //setPos
        s.setX(s.x() - cbArrowHeadSize);
        d.setX(d.x() + cbArrowHeadSize);
        //src
        if (item->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            painter->drawPolygon(createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionWE, item->getSourcePoint()));
            path.moveTo(s);
        } else {
            path.lineTo(s);
        }
        //curve
        path.cubicTo(QPointF(s.x() - defl, s.y()),
                     QPointF(d.x() + defl, d.y()),
                     d);
        //dst
        if (item->getDestinationArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            painter->drawPolygon(createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW, item->getDestinationPoint()));
        } else {
            path.lineTo(item->getDestinationPoint());
        }
        break;
    case PropertyLinkArrowGraphicsItem::DockPositionsRightLeft:
        //setPos
        s.setX(s.x() + cbArrowHeadSize);
        d.setX(d.x() - cbArrowHeadSize);
        //src
        if (item->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            painter->drawPolygon(createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW, item->getSourcePoint()));
            path.moveTo(s);
        } else {
            path.lineTo(s);
        }
        //curve
        path.cubicTo(QPointF(s.x() + defl, s.y()),
                     QPointF(d.x() - defl, d.y()),
                     d);
        //dst
        if (item->getDestinationArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            painter->drawPolygon(createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionWE, item->getDestinationPoint()));
        } else {
            path.lineTo(item->getDestinationPoint());
        }
        break;
    case PropertyLinkArrowGraphicsItem::DockPositionsRightRight:
        //setPos
        s.setX(s.x() + cbArrowHeadSize);
        d.setX(d.x() + cbArrowHeadSize);
        //src
        if (item->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            painter->drawPolygon(createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW, item->getSourcePoint()));
            path.moveTo(s);
        } else {
            path.lineTo(s);
        }
        //curve
        path.cubicTo(QPointF(s.x() + defl, s.y()),
                     QPointF(d.x() + defl, d.y()),
                     d);
        //dst
        if (item->getDestinationArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
            painter->drawPolygon(createArrowHead(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW, item->getDestinationPoint()));
        } else {
            path.lineTo(item->getDestinationPoint());
        }
        break;
    default:
        tgtAssert(false, "shouldn't get here");
    }

    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
}

//-------------------------------------
//      PortSizeLinkArrowGraphicsItem  
//-------------------------------------
QRectF NWEStyle_Classic::PortSizeLinkArrowGI_boundingRect(const PortSizeLinkArrowGraphicsItem* item) const {
    return PortSizeLinkArrowGI_shape(item).boundingRect();
}

QPainterPath NWEStyle_Classic::PortSizeLinkArrowGI_shape(const PortSizeLinkArrowGraphicsItem* item) const {
    QPointF sourcePoint = item->getSourcePoint();
    QPointF destinationPoint = item->getDestinationPoint();

    qreal defl = fabs(sourcePoint.y() - destinationPoint.y()) / 2.1 + fabs(sourcePoint.x() - destinationPoint.x()) / 2.1;
    qreal p = portArrowHoverSize;

    QPainterPath path(sourcePoint);
    path.moveTo(sourcePoint.x()-p,sourcePoint.y());
    path.quadTo(QPointF((sourcePoint.x()+destinationPoint.x()) / 2.f-p, (sourcePoint.y()+destinationPoint.y()) / 2.f - defl),
                QPointF(destinationPoint.x()-p,destinationPoint.y()));
    path.lineTo(destinationPoint.x()+p,destinationPoint.y());
    path.quadTo(QPointF((sourcePoint.x()+destinationPoint.x()) / 2.f+p, (sourcePoint.y()+destinationPoint.y()) / 2.f - defl),
                QPointF(sourcePoint.x()+p,sourcePoint.y()));
    path.lineTo(sourcePoint.x()-p,sourcePoint.y());

    path.addPolygon(createArrowHead(QPointF((sourcePoint.x()+destinationPoint.x()) / 2.f, (sourcePoint.y()+destinationPoint.y()) / 2.f - defl)
                                         , item->getDestinationPoint()));

    return path;
}

void NWEStyle_Classic::PortSizeLinkArrowGI_initializePaintSettings(PortSizeLinkArrowGraphicsItem* item) {}

void NWEStyle_Classic::PortSizeLinkArrowGI_paint(PortSizeLinkArrowGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {

    QColor tmpColor;

    switch (item->getColorConnectableMode()) {
    case ConnectionBaseGraphicsItem::CCM_DEFAULT:
        if (item->isSelected())
            tmpColor = NWEStyle_SelectionColor;
        else
            tmpColor = Qt::black;

        if ((option->state & QStyle::State_MouseOver) || item->getIsHovered()) {
            if (tmpColor == Qt::black)
                tmpColor = NWEStyle_HoverColor;
            else
                tmpColor = tmpColor.light();
        }
        break;
    case ConnectionBaseGraphicsItem::CCM_NO:
        tmpColor = NWEStyle_ConnectionNo;
        break;
    case ConnectionBaseGraphicsItem::CCM_MAYBE:
        tmpColor = NWEStyle_ConnectionMaybe;
        break;
    case ConnectionBaseGraphicsItem::CCM_YES:
        tmpColor = NWEStyle_ConnectionYes;
        break;
    }

    QPointF sourcePoint = item->getSourcePoint();
    QPointF destinationPoint = item->getDestinationPoint();

    qreal defl = fabs(sourcePoint.y() - destinationPoint.y()) / 2.1 + fabs(sourcePoint.x() - destinationPoint.x()) / 2.1;

    QPainterPath path(sourcePoint);

    path.quadTo(QPointF((sourcePoint.x()+destinationPoint.x()) / 2.f, (sourcePoint.y()+destinationPoint.y()) / 2.f - defl),
                 destinationPoint);

    painter->setPen(QPen(tmpColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawPath(path);

    painter->setBrush(QBrush(tmpColor));
    painter->drawPolygon(createArrowHead(QPointF((sourcePoint.x()+destinationPoint.x()) / 2.f, (sourcePoint.y()+destinationPoint.y()) / 2.f - defl)
                                         , item->getDestinationPoint()));
}

    /*********************************************************************
     *                       ToolTip Elements                             
     ********************************************************************/

//-------------------------------------
//      ToolTipPortGraphicsItem        
//-------------------------------------
QRectF NWEStyle_Classic::ToolTipPortGI_boundingRect(const ToolTipPortGraphicsItem* item) const {
    QRectF bRect = item->getToolTipTextItem()->boundingRect();
    if(item->getImage()) {
        float imageHeight = bRect.width() - 6.f;
        if(item->getImageAspect() > 1)
            imageHeight = imageHeight / item->getImageAspect();
        // create render tooltip
        bRect.setHeight(bRect.height() + imageHeight + 6.f);
    }
    return bRect;
}

QPainterPath NWEStyle_Classic::ToolTipPortGI_shape(const ToolTipPortGraphicsItem* item) const {
    QPainterPath path;
    path.addEllipse(ToolTipPortGI_boundingRect(item));
    return path;
}

void NWEStyle_Classic::ToolTipPortGI_initializePaintSettings(ToolTipPortGraphicsItem* item) {}

void NWEStyle_Classic::ToolTipPortGI_paint(ToolTipPortGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    QRectF bRect = ToolTipPortGI_boundingRect(item);
    painter->setBrush(NWEStyle_ToolTipPortBackgroundColor);
    painter->drawRect(bRect);
    if (item->getImage()) {

        float imageHeight = bRect.width() - 6.f;
        float imageWidth = bRect.width() - 6.f;
        if(item->getImageAspect() > 1)
            imageHeight = imageHeight / item->getImageAspect();
        else
            imageWidth = imageWidth * item->getImageAspect();

        // checkers background
        QPixmap pm(20, 20);
        QPainter pmp(&pm);
        pmp.fillRect(0, 0, 10, 10, Qt::lightGray);
        pmp.fillRect(10, 10, 10, 10, Qt::lightGray);
        pmp.fillRect(0, 10, 10, 10, Qt::darkGray);
        pmp.fillRect(10, 0, 10, 10, Qt::darkGray);
        pmp.end();
        painter->setBrush(QBrush(pm));
        painter->drawRect((bRect.width() - imageWidth)/2.f,item->getToolTipTextItem()->boundingRect().height()+3.f,imageWidth, imageHeight);
        painter->drawImage(QRectF((bRect.width() - imageWidth)/2.f,item->getToolTipTextItem()->boundingRect().height()+3.f,imageWidth, imageHeight)
                            , *item->getImage());
    }
}


} // namespace voreen
