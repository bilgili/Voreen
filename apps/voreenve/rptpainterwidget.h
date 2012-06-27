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

#ifndef VRN_RPTPAINTERWIDGET_H
#define VRN_RPTPAINTERWIDGET_H

#include <QtGui>
#include "tgt/qt/qtcanvas.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/trackballnavigation.h"
#include "voreen/core/vis/flythroughnavigation.h"

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/networkevaluator.h"


namespace voreen {

class RptPainterWidget : public tgt::QtCanvas, tgt::EventListener {
    Q_OBJECT
public:

    enum CameraNavigation {
        TRACKBALL_NAVIGATION,
        FLYTHROUGH_NAVIGATION
    };

    RptPainterWidget(QWidget* parent = 0, CameraNavigation navigation = TRACKBALL_NAVIGATION);
    ~RptPainterWidget();

    void init(TextureContainer* tc, tgt::Camera* camera);
    TextureContainer* getTextureContainer();
    bool setEvaluator(NetworkEvaluator* evaluator);

    void closeEvent(QCloseEvent* e);
    void hideEvent(QHideEvent* e);

    TrackballNavigation* getTrackballNavigation() const;
    FlythroughNavigation* getFlythroughNavigation() const;
    
    void setCurrentNavigation(CameraNavigation navi);
    CameraNavigation getCurrentNavigation() const;

    VoreenPainter* getPainter();

    NetworkEvaluator* eval;
    

signals:
    void detachSignal();
    void attachSignal();

private:
    TextureContainer* tc_;
    
    TrackballNavigation* trackNavi_;
    FlythroughNavigation* flythroughNavi_;
    CameraNavigation currentNavigation_;

    VoreenPainter* painter_;

    bool canvasDetached_;

protected:
    void mouseDoubleClickEvent(QMouseEvent* event);

};

} //namespace voreen

#endif //VRN_RPTPAINTERWIDGET_H
