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

#ifndef VRN_CONNECTIONBASEGRAPHICSITEM_H
#define VRN_CONNECTIONBASEGRAPHICSITEM_H

#include "../nwebasegraphicsitem.h"

namespace voreen {

class ConnectionBaseGraphicsItem : public NWEBaseGraphicsItem {
public:

    enum ColorConnectableMode {
        CCM_DEFAULT,
        CCM_NO,
        CCM_MAYBE,
        CCM_YES
    };

    enum ArrowHeadDirection {
        ArrowHeadDirectionNone,
        ArrowHeadDirectionNS,
        ArrowHeadDirectionSN,
        ArrowHeadDirectionWE,
        ArrowHeadDirectionEW
    };

    //constructor+destructor
    ConnectionBaseGraphicsItem(NWEBaseGraphicsItem* sourceItem, NWEBaseGraphicsItem* destinationItem = 0);
    ~ConnectionBaseGraphicsItem();

    //---------------------------------------------------------------------------------
    //      nwebasegraphicsitem functions                                                       
    //---------------------------------------------------------------------------------
    virtual void updateNWELayerAndCursor() {};

    virtual void layoutChildItems() {};
protected:
    virtual void createChildItems() {};
    virtual void deleteChildItems() {};

    //---------------------------------------------------------------------------------
    //      connectionbaseebasegraphicsitem functions                                                       
    //---------------------------------------------------------------------------------
public:
    //getter and setter
    ColorConnectableMode getColorConnectableMode() const { return colorConnectableMode_;}
    void setColorConnectableMode(ColorConnectableMode ccm) {colorConnectableMode_ = ccm;} 
    
    ArrowHeadDirection getSourceArrowHeadDirection() const;
    void setSourceArrowHeadDirection(ArrowHeadDirection ahd);
    ArrowHeadDirection getDestinationArrowHeadDirection() const;
    void setDestinationArrowHeadDirection(ArrowHeadDirection ahd);

    virtual QPointF getSourcePoint() const;
    void setSourcePoint(const QPointF& sourcePoint);
    virtual QPointF getDestinationPoint() const;
    void setDestinationPoint(const QPointF& destinationPoint);

    void setSourceItem(NWEBaseGraphicsItem* item);
    void setDestinationItem(NWEBaseGraphicsItem* item);

    //Hack to get new arrow hoverd
    bool getIsHovered();
    void setIsHovered(bool b);
protected:
    //member
    ColorConnectableMode colorConnectableMode_;
    ArrowHeadDirection sourceArrowHeadDirection_;
    ArrowHeadDirection destinationArrowHeadDirection_;
    bool isHovered_; //(hack)

    NWEBaseGraphicsItem* sourceItem_;
    NWEBaseGraphicsItem* destinationItem_;
    QPointF sourcePoint_;
    QPointF destinationPoint_;
};

} // namespace voreen

#endif // VRN_ARROWGRAPHICSITEM_H
