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

#include <QPainter>
#include <math.h>

#include "rptarrow.h"
#include "rptprocessoritem.h"
#include "rptaggregationitem.h"
#include "rptpropertysetitem.h"
#include "rpttcttooltip.h"

//#include <QGraphicsProxyWidget>
//#include <QPushButton>
//#include <sstream>

namespace voreen {

extern NetworkEvaluator* uglyglobalevaluator; // FIXME: Remove after we found a way to access the netev for tooltips

//FIXME: use other Pi ;)
static const double Pi = 3.14159265358979323846264338327950288419717;
//static double TwoPi = 2.0 * Pi;

RptArrow::RptArrow(QGraphicsItem *sourceNode, QGraphicsItem *destNode)
    : QGraphicsItem()
    , arrowSize_(10) //, tooltip_(0)
{
    source_ = sourceNode;
    dest_ = destNode;

    color_ = Qt::black;

    //setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    if (source_->type() == RptPortItem::Type)
        setZValue(0.2);
    else
        setZValue(0);
    // activate hover events for highlighting
    if (!acceptsHoverEvents())
        setAcceptsHoverEvents(true);
    
    //setToolTip("ZOMG has Tooltip!");
}

RptArrow::~RptArrow() {
    //this->setParentItem(0);
    // check if arrow still in scene, because removeItem will crash then
    
    if (scene() != 0) {
        static_cast<RptGraphWidget*>(scene()->views()[0])->hideRptTooltip(); // FIXME: Hack to fix a crash - make this prettier!
        scene()->removeItem(this);
    }
    //delete tooltip_;
    //tooltip_ = 0;
}

void RptArrow::setDestNode(QGraphicsItem* node) {
    dest_ = node;
    color_ = Qt::black;
    adjust();
}

void RptArrow::adjust() {
    if (!source_ || !dest_)
        return;

    QLineF line(mapFromItem(source_, 0, 0), mapFromItem(dest_, 0, 0));
   
    prepareGeometryChange();
    sourcePoint_ = line.p1();
    
    // sourcePoint_ points to the top left corner of the source node, which is now corrected according to the nodes type
    if (source_->type() == RptPortItem::Type) {
        if (static_cast<RptPortItem*>(source_)->getPortType().getSubString(0) != "coprocessor") {
            sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height());
            sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width()/2);
        }
        else {
            sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height()/2);
            sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width());
        }
    }
    else {
        sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height()/2);
        sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width()/2);
    }

    destPoint_ = line.p2();

    // same story as above
    if (source_->type() == RptPortItem::Type) {
        if (static_cast<RptPortItem*>(source_)->getPortType().getSubString(0) != "coprocessor")
            destPoint_.setX(destPoint_.x() + dest_->boundingRect().width()/2);
        else 
            destPoint_.setY(destPoint_.y() + dest_->boundingRect().height()/2);
    }
    else {
        destPoint_.setX(destPoint_.x() + dest_->boundingRect().width()/2);
        destPoint_.setY(destPoint_.y() + dest_->boundingRect().height()/2);
    }

    update();
}

void RptArrow::adjust(QPointF dest) {
    QLineF line(mapFromItem(source_, 0, 0), mapFromScene(dest));
   
    QGraphicsItem* item = scene()->itemAt(dest);
    if (source_->type() == RptPortItem::Type && item) {
        RptPortItem* srcPort = static_cast<RptPortItem*>(source_);
        switch (item->type()) {
            case RptPortItem::Type:
                if (srcPort->getParent()->testConnect(srcPort, static_cast<RptPortItem*>(item)))
                    color_ = Qt::green;
                else
                    color_ = Qt::red;
                break;
            case RptProcessorItem::Type:
                if (srcPort->getParent()->testConnect(srcPort, static_cast<RptProcessorItem*>(item)))
                    color_ = Qt::green;
                else
                    color_ = Qt::red;
                break;
            case RptAggregationItem::Type:
                if (srcPort->getParent()->testConnect(srcPort, static_cast<RptAggregationItem*>(item)))
                    color_ = Qt::green;
                else
                    color_ = Qt::red;
                break;
            case RptPropertySetItem::Type:
                color_ = Qt::red;
                break;
            default:
                color_ = Qt::black;
        }
    }
    else
        color_ = Qt::black;

    prepareGeometryChange();
    sourcePoint_ = line.p1();

    // sourcePoint_ points to the top left corner of the source node, which is now corrected according to the nodes type
    if (source_->type() == RptPortItem::Type) {
        if (static_cast<RptPortItem*>(source_)->getPortType().getSubString(0) != "coprocessor") {
            sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height());
            sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width()/2);
        }
        else {
            sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height()/2);
            sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width());
        }
    }
    else {
        sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height()/2);
        sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width()/2);
    }

    destPoint_ = line.p2(); 
    update();
}

qreal RptArrow::deflection() const {
    return fabs(sourcePoint_.y() - destPoint_.y()) / 2.f + fabs(sourcePoint_.x() - destPoint_.x()) / 2.f;
}

QRectF RptArrow::boundingRect() const {
    qreal penWidth = 1;
    qreal extra = (penWidth + arrowSize_) / 2.0;
    qreal defl = deflection();
    QRectF brect = QRectF(sourcePoint_, QSizeF(destPoint_.x() - sourcePoint_.x(), destPoint_.y() - sourcePoint_.y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
    RptPortItem* portitem = dynamic_cast<RptPortItem*>(source_);
    if (portitem && portitem->getPortType().getSubString(0) != "coprocessor") {
        return brect.adjusted(0, -defl, 0, 2*defl); // bounding rect is bigger due to bezier stuff
    }
    else {
        return brect.adjusted(-defl, 0, 2*defl, 0); // bounding rect is bigger due to bezier stuff
    }
}

QPointF RptArrow::center() const {
    return QPointF((sourcePoint_.x()+destPoint_.x())/2,(sourcePoint_.y()+destPoint_.y())/2);
}

QPainterPath RptArrow::shape() const {

    QPainterPath path(sourcePoint_);

    qreal defl = deflection();
    //QPointF center((sourcePoint_+destPoint_)/2);
    if (source_->type() == RptPortItem::Type) {
        if (static_cast<RptPortItem*>(source_)->getPortType().getSubString(0) == "coprocessor") {
            path.cubicTo(QPointF(sourcePoint_)+QPointF(defl,0), QPointF(destPoint_)-QPointF(defl,0), QPointF(destPoint_)-QPointF(0,0));
        }
        else {
            path.cubicTo(QPointF(sourcePoint_)+QPointF(0,defl), QPointF(destPoint_)-QPointF(0,defl), QPointF(destPoint_)-QPointF(0,arrowSize_));
            // shift center to the right if s.y > d.y
            //float variance = 10;
            //float amplitude = 1000;
            //center.rx() += amplitude/(sqrt(2*Pi)*variance)*exp(-pow(sourcePoint_.x()-destPoint_.x()/variance,2));
            //if (sourcePoint_.y()>destPoint_.y()) center.rx() += 20;
            //path.cubicTo(QPointF(sourcePoint_)+QPointF(0,defl), center, center);
            //path.cubicTo(center, QPointF(destPoint_)-QPointF(0,defl), QPointF(destPoint_)-QPointF(0,arrowSize_));
            //some space for the arrow's head
            path.lineTo(destPoint_);
        }
    }
    else {
        path.lineTo(destPoint_);
    }

 
    /* I see no reason to add the arrow's head to the shape - d_kirs04
    // Draw the arrows if there's enough room
    QLineF line(sourcePoint_, destPoint_);

    double angle = ::acos(line.dx() / line.length());
    if (line.dy() >= 0)
        angle = TwoPi - angle;

    QPointF sourceArrowP1 = sourcePoint_ + QPointF(sin(angle + Pi / 3) * arrowSize_,
        cos(angle + Pi / 3) * arrowSize_);
    QPointF sourceArrowP2 = sourcePoint_ + QPointF(sin(angle + Pi - Pi / 3) * arrowSize_,
        cos(angle + Pi - Pi / 3) * arrowSize_);
    QPointF destArrowP1 = destPoint_ + QPointF(sin(angle - Pi / 3) * arrowSize_,
        cos(angle - Pi / 3) * arrowSize_);
    QPointF destArrowP2 = destPoint_ + QPointF(sin(angle - Pi + Pi / 3) * arrowSize_,
        cos(angle - Pi + Pi / 3) * arrowSize_);

    path.addPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);            std::cout << "Args" << std::endl;


    path.addPolygon(QPolygonF() << sourceArrowP1 << sourceArrowP2 << destArrowP2 << destArrowP1);
    */

    return path;
}

void RptArrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    // Draw the line itself
    // QLineF line(sourcePoint_, destPoint_);
    QColor tmpColor;    // needed for hover effect
    if (isSelected())
        tmpColor = Qt::red;
    else
        tmpColor = color_;
    if (option->state & QStyle::State_MouseOver) {
        if (tmpColor == Qt::black)    // Qt is unable to brighten up Qt::black
			tmpColor = Qt::darkGray;
		else
			tmpColor = tmpColor.light();
    }
	painter->setPen(QPen(tmpColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    //painter->drawLine(line);

	QPainterPath path(sourcePoint_);
    path.addPath(shape());
 	painter->drawPath(path);

    // Draw the arrows if there's enough room // The actually isn't any check for room...
    /*
    double angle = ::acos(line.dx() / line.length());
    if (line.dy() >= 0)
        angle = TwoPi - angle;
    */
	// Right now I want a fixed angle for the bezier stuff
	double angle = -Pi / 2;

    QPointF sourceArrowP1 = sourcePoint_ + QPointF(sin(angle + Pi / 3) * arrowSize_,
        cos(angle + Pi / 3) * arrowSize_);
    QPointF sourceArrowP2 = sourcePoint_ + QPointF(sin(angle + Pi - Pi / 3) * arrowSize_,
        cos(angle + Pi - Pi / 3) * arrowSize_);
    QPointF destArrowP1 = destPoint_ + QPointF(sin(angle - Pi / 3) * arrowSize_,
        cos(angle - Pi / 3) * arrowSize_);
    QPointF destArrowP2 = destPoint_ + QPointF(sin(angle - Pi + Pi / 3) * arrowSize_,
        cos(angle - Pi + Pi / 3) * arrowSize_);

    painter->setBrush(tmpColor);

    if (source_->type() == RptPortItem::Type) {
        if (static_cast<RptPortItem*>(source_)->getPortType().getSubString(0) != "coprocessor") {
            //painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
            painter->drawPolygon(QPolygonF() << destPoint_ << destArrowP1 << destArrowP2);
        }
    }
  
    // draw bounding rect
    //painter->setBrush(Qt::NoBrush);
    //painter->drawRect(boundingRect());
}


int RptArrow::getSourceTextureContainerTarget(NetworkEvaluator* neteval) const {
    RptPortItem* sourceportitem = dynamic_cast<RptPortItem*>(source_); // Only has TCTarget if it is a Port
    if  (sourceportitem) {
        try {
            return neteval->getTextureContainerTarget(sourceportitem->getPort());
        }
        catch (std::exception& /*e*/) {
            // cout << e.what() << endl; // FIXME: I have to log the error somehow - d_kirs04
            return -1;
        }
    }
    else {
        return -1;
    }
}

/* This is done in the rptgraphwidget
bool RptArrow::sceneEvent(QEvent* event) {
    // FIXME: IMHO this should be done with GraphicsSceneHelp but those Events never reach the Items :/
    if (event->type() == QEvent::GraphicsSceneHoverEnter) {
        //QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        showTCTTooltip();
    } else if (event->type() == QEvent::GraphicsSceneHoverLeave) {
        hideTCTTooltip();
    }
    
    // Used this to figure out which events actually arrive here
    // std::ostringstream s;
    // s << event->type();
    //std::cout << s.str() << std::endl;

    return QGraphicsItem::sceneEvent(event);
}
*/

QGraphicsItem* RptArrow::rptTooltip() const {
    // Where do I get the networkevaluator from?
    NetworkEvaluator* neteval = uglyglobalevaluator; // FIXME: Need to get the NetworkEvaluator from different source
    int tctarget = getSourceTextureContainerTarget(neteval);
    // If there is a tctarget and the network is valid I can show a Tooltip
    if (tctarget != -1 && neteval->isValid()) {
        /*
        if (!tooltip_) {
            tooltip_ = new RptTCTTooltip(0,0,100,100);
            static_cast<RptTCTTooltip*>(tooltip_)->initialize(tctarget, neteval->getTextureContainer());
        }
        */
        RptTCTTooltip* tooltip = new RptTCTTooltip(-100,-100,100,100);
        tooltip->initialize(tctarget, neteval->getTextureContainer());
        return tooltip;
    } else {
        //QGraphicsRectItem* notooltip = new QGraphicsRectItem(-112, -20, 122, 20);
        QGraphicsSimpleTextItem* notooltiptext = new QGraphicsSimpleTextItem("no tooltip available");
        QGraphicsRectItem* notooltip = new QGraphicsRectItem((notooltiptext->boundingRect()).adjusted(-4,0,4,0));
        notooltiptext->setParentItem(notooltip);
        notooltip->translate(-notooltip->rect().width(),-notooltip->rect().height());
        notooltip->setBrush(QBrush(QColor(253, 237, 212),Qt::SolidPattern));
        return notooltip;
    }
}

} //namespace voreen
