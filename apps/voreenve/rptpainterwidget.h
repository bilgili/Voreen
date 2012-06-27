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

#ifndef VRN_RPTPAINTERWIDGET_H
#define VRN_RPTPAINTERWIDGET_H

#include "tgt/qt/qtcanvas.h"

class QMouseEvent;
class QWidget;

namespace voreen {

class FlythroughNavigation;
class NetworkEvaluator;
class TrackballNavigation;
class VoreenPainter;

class RptPainterWidget : public tgt::QtCanvas {
    Q_OBJECT
public:
    enum CameraNavigation {
        TRACKBALL_NAVIGATION,
        FLYTHROUGH_NAVIGATION
    };

    RptPainterWidget(QWidget* parent = 0, CameraNavigation navigation = TRACKBALL_NAVIGATION);
    ~RptPainterWidget();

    void init(NetworkEvaluator* eval, tgt::Camera* camera);
    void initializeGL();

    TrackballNavigation* getTrackballNavigation() const;
    FlythroughNavigation* getFlythroughNavigation() const;

    void setCurrentNavigation(CameraNavigation navi);
    CameraNavigation getCurrentNavigation() const;

private:
    TrackballNavigation* trackNavi_;
    FlythroughNavigation* flythroughNavi_;
    CameraNavigation currentNavigation_;

    NetworkEvaluator* evaluator_;
};

} // namespace voreen

#endif // VRN_RPTPAINTERWIDGET_H
