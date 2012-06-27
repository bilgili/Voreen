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

#ifndef VRN_RPTTCTTOOLTIP_H

#include <QGraphicsRectItem>
#include "voreen/core/opengl/texturecontainer.h"


namespace voreen {

/**
 * TODO Proper docs
 * Class for Tooltip for Texture Containers
 */
class RptTCTTooltip : public QGraphicsRectItem {

public:
    /**
     * Constructors inherited.
     */
    RptTCTTooltip ( QGraphicsItem * parent = 0);
    RptTCTTooltip ( const QRectF & rect, QGraphicsItem * parent = 0);
    RptTCTTooltip ( qreal x, qreal y, qreal width, qreal height, QGraphicsItem * parent = 0);
    ~RptTCTTooltip();

    void initialize(int id, TextureContainer* tc);
    
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void initImage();

    QImage* image_;
    TextureContainer* tc_;
    int id_; // TextureContainerTargetID
};

} //namespace voreen

#endif // VRN_RPTGLGRAPHICSITEM_H

