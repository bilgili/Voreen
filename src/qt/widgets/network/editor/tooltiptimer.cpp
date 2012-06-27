/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/qt/widgets/network/editor/tooltiptimer.h"

#include <cmath>

namespace voreen {

TooltipTimer::TooltipTimer(QPoint p, qreal radius, QObject* parent)
    : QTimer(parent)
    , p_(p)
    , radius_(radius)
{}

void TooltipTimer::setRadius(qreal radius) {
    radius_ = radius;
}

void TooltipTimer::setPoint(QPoint p) {
    p_ = p;
}

qreal TooltipTimer::radius() {
    return radius_;
}

QPointF TooltipTimer::point() {
    return p_;
}

bool TooltipTimer::isDistant(const QPoint& p) {
    if (distance(p) > radius_) {
        stop();
        return true;
    }
    else {
        return false;
    }
}

void TooltipTimer::resetIfDistant(const QPoint& p, int msec) {
    if (isDistant(p)) {
        p_ = p;
        start(msec);
    }
}

qreal TooltipTimer::distance(const QPoint& p) const {
    return std::sqrt(pow(p_.x()-p.x(),2.)+pow(p_.y()-p.y(),2.)); // Should I use tgt?
}

} //namespace voreen

