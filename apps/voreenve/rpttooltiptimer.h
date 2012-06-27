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

#ifndef VRN_RPTTOOLTIPTIMER_H
#define VRN_RPTTOOLTIPTIMER_H

#include <QTimer>
#include <QPoint>

namespace voreen {

/**
 * This class TODO
 */
class RptTooltipTimer : public QTimer {

public:
    RptTooltipTimer(QPoint p, qreal radius, QObject* parent = 0);
    qreal radius() { return radius_; }
    void setRadius(qreal radius);
    QPointF point() { return p_; }
    void setPoint(QPoint p);
    qreal distance(const QPoint & p) const;
    
public slots:
    /**
     * This will stop the timer if p is farer away from the initialisation point than radius and returns true
     * otherwise false
     */
    bool isDistant(const QPoint & p);
    /**
     * This will restart the timer if p is farer away from the initialisation point than radius and set the point to p
     */
    void resetIfDistant(const QPoint & p, int msec);
    
protected:

    QPoint p_;
    qreal radius_;
};
    
} //namespace voreen

#endif // VRN_RPTTOOLTIPTIMER_H
