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

#ifndef VRN_TOOLTIPBASEGRAPHICSITEM_H
#define VRN_TOOLTIPBASEGRAPHICSITEM_H

#include "../nwebasegraphicsitem.h"
#include "../../networkeditor_settings.h"

class QGraphicsTextItem;
class QTimer;

namespace voreen {

class ToolTipBaseGraphicsItem : public NWEBaseGraphicsItem {
Q_OBJECT
public:
    //constructor
    ToolTipBaseGraphicsItem(NWEBaseGraphicsItem* item);

    //---------------------------------------------------------------------------------
    //      nwebasegraphicsitem functions                                                       
    //---------------------------------------------------------------------------------
    virtual void updateNWELayerAndCursor(){};
    //handling child elements
    virtual void layoutChildItems(){};
protected:
    virtual void createChildItems(){};
    virtual void deleteChildItems(){};

    //---------------------------------------------------------------------------------
    //      tooltipgraphicsitem functions                                                       
    //---------------------------------------------------------------------------------
public:
    virtual void updateToolTip(qreal x, qreal y);
    //timer
    void startTimer();
    void stopTimer();
protected slots:
    void timerTimeout();
public:
    //getter and setter
    const QGraphicsTextItem* getToolTipTextItem() const;
    void setToolTipTimerTriggertMousePosition(QPointF& point);

protected:
    //member
    QGraphicsTextItem* toolTipText_;
    QTimer* timer_;
    QPointF toolTipTimerTriggertMousePosition_;
};

} //namespace voreen

#endif // VRN_NWEBASETOOLTIPBASEGRAPHICSITEM_H

