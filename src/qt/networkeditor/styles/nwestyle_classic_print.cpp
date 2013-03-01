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

#include "voreen/qt/networkeditor/styles/nwestyle_classic_print.h"
#include "voreen/qt/networkeditor/editor_settings.h"

#include "voreen/qt/voreenapplicationqt.h"

//gi
#include "voreen/qt/networkeditor/graphicitems/core/aggregationgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/core/portgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/core/processorgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/core/propertygraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/core/propertylistgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/connections/portarrowgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/connections/portsizelinkarrowgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/connections/portownerlinkarrowgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/connections/propertylinkarrowgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/utils/progressbargraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/utils/propertylistbuttongraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/utils/widgettogglebuttongraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/tooltips/tooltipportgraphicsitem.h"

//core
#include "voreen/core/ports/port.h"
#include "voreen/core/ports/coprocessorport.h"
#include "voreen/core/ports/renderport.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/processors/processorwidget.h"

//qt
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QStyleOption>

namespace voreen{

/*********************************************************************
 *                       General Color Defines
 ********************************************************************/
//aggregation
const QColor NWEStyle_Classic_Print::NWEStyle_AggregationColor1 = Qt::white;
//processor
const QColor NWEStyle_Classic_Print::NWEStyle_ProcessorColor1 = Qt::white;

/*********************************************************************
 *                       General Functions
 ********************************************************************/
NWEStyle_Classic_Print::NWEStyle_Classic_Print()
    : NWEStyle_Classic()
    {}

NWEStyle_Classic_Print::~NWEStyle_Classic_Print()
    {}

    /*********************************************************************
     *                       Core Elements
     ********************************************************************/

//-------------------------------------
//      AggregationGraphicsItem
//-------------------------------------
void NWEStyle_Classic_Print::AggregationGI_initializePaintSettings(AggregationGraphicsItem* item) {
    //set text
    item->getNameLabel()->setPlainText(item->getGuiName());
    item->getNameLabel()->setFont(QFont("Helvetica", 10*(static_cast<float>(VoreenApplicationQt::qtApp()->getProcessorFontScale())/100.f)));
    item->getNameLabel()->setDefaultTextColor(Qt::black);
}

void NWEStyle_Classic_Print::AggregationGI_paint(AggregationGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget, NWEItemSettings& setting) {
    //get bounding rect
    QRectF br = AggregationGI_boundingRect(item);
    //set color settings
    painter->setOpacity(setting.opacity_);
    item->setZValue(setting.zValue_);
        //gradient
    QLinearGradient gradient(0, 0, 0, br.height());
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, setting.color1_);
    gradient.setColorAt(0.4, Qt::white);
    gradient.setColorAt(0.6, Qt::white);
    gradient.setColorAt(1.0, setting.color1_);
    QBrush brush(gradient);
    painter->setBrush(brush);
    painter->setPen(QPen(QBrush(setting.color1_), 2.0));

    //draw aggregation
    painter->setClipPath(AggregationGI_shape(item));
    painter->drawRect(br);
}


//-------------------------------------
//      ProcessorGraphicsItem
//-------------------------------------
void NWEStyle_Classic_Print::ProcessorGI_initializePaintSettings(ProcessorGraphicsItem* item) {
    //set text
    item->getNameLabel()->setPlainText(item->getGuiName());
    item->getNameLabel()->setFont(QFont("Helvetica", 10*(static_cast<float>(VoreenApplicationQt::qtApp()->getProcessorFontScale())/100.f)));
    item->getNameLabel()->setDefaultTextColor(Qt::black);
}

void NWEStyle_Classic_Print::ProcessorGI_paint(ProcessorGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget, NWEItemSettings& setting) {

    QRectF br = ProcessorGI_boundingRect(item);
    //set color settings
    painter->setOpacity(setting.opacity_);
    item->setZValue(setting.zValue_);
        //gradient
    QLinearGradient gradient(0, 0, 0, br.height());
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, setting.color1_);
    gradient.setColorAt(0.4, Qt::white);
    gradient.setColorAt(0.6, Qt::white);
    gradient.setColorAt(1.0, setting.color1_);
    QBrush brush(gradient);
    painter->setBrush(brush);
    painter->setPen(QPen(QBrush(Qt::black), 2.0));

    //draw processor
    painter->drawRect(br);
}

} // namespace voreen
