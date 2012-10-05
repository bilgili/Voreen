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

#ifndef VRN_TOOLTIPPORTGRAPHICSITEM_H
#define VRN_TOOLTIPPORTGRAPHICSITEM_H

#include "tooltipbasegraphicsitem.h"

class QImage;

namespace voreen {

    class PortGraphicsItem;

class ToolTipPortGraphicsItem : public ToolTipBaseGraphicsItem {
public:
    //constructor
    ToolTipPortGraphicsItem(PortGraphicsItem* item);

    //---------------------------------------------------------------------------------
    //      nwebasegraphicsitem functions                                              
    //---------------------------------------------------------------------------------
    int type() const {return UserTypesToolTipPortGraphicsItem;}

    //style
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
protected:
    void initializePaintSettings();
    void prePaint(QPainter* painter, const QStyleOptionGraphicsItem* option);

    //---------------------------------------------------------------------------------
    //      tooltipbase functions                                                      
    //---------------------------------------------------------------------------------
public:
    void updateToolTip(qreal x, qreal y);
    //getter and setter
    const QImage* getImage() const;
    const float getImageAspect() const;

protected:
    //member
    QImage* image_;
    float image_aspect_;

};

} //namespace voreen

#endif // VRN_TOOLTIPPORTGRAPHICSITEM_H

