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

#ifndef VRN_MULTIVIEWWIDGET_H
#define VRN_MULTIVIEWWIDGET_H

#include <QWidget>

#include "tgt/camera.h"
#include "tgt/qt/qtcanvas.h"

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/container/multiview.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/trackballnavigation.h"
#include "voreen/core/vis/borderoverlay.h"

namespace voreen {

/**
 * This class capsulates a widget with four canvases.
 * It destroys all owned objects by itself.
 */
class MultiViewWidget : public QWidget {
    Q_OBJECT

    class MV3dView : public tgt::QtCanvas, public voreen::MessageReceiver {
    public:
        MV3dView(QWidget* parent, tgt::Camera* camera)
            : tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, parent, true, 0)
            , voreen::MessageReceiver("mv1")
        {
            setCamera(camera);
            tgt::Trackball* track = new tgt::Trackball(this, false);
            painter_ = new VoreenPainter(this, track, "mv1");
            eventHandler_->addListenerToBack(new TrackballNavigation(track, true, 0.05f, 15.f));
            MsgDistr.insert(this);
            startTimer(10);
        }
        ~MV3dView() {
            MsgDistr.remove(this);
            delete painter_;
        }
        void processMessage(Message* msg, const Identifier& dest = Identifier::all);
    };

    class MVSliceView : public tgt::QtCanvas, public tgt::EventListener, public voreen::MessageReceiver {
    public:

        MVSliceView(QWidget* parent, RptNetworkEvaluator* evaluator, MultiView* multiView, size_t index,
            const tgt::Color &borderColor, int borderWidth)
            : tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, parent, true, 0)
            , voreen::MessageReceiver("mv1")
            , multiView_(multiView)
            , index_(index)
            , leftPressed_(false)
            , oldMouseY_(0)
        {
            camera_ = new tgt::Camera();
            trackball_ = new tgt::Trackball(this, false);
            painter_ = new VoreenPainter(this, trackball_, "mv1");
            ((VoreenPainter*)painter_)->setEvaluator(evaluator);
            eventHandler_->addListenerToBack(this);
            MsgDistr.insert(this);

            // add border overlay
            borderOverlay_ = new BorderOverlay(this, borderColor, borderWidth);
            ((VoreenPainter*)painter_)->postMessage(
                new TemplateMessage<VoreenPainterOverlay*>(VoreenPainter::addCanvasOverlay_,
                    borderOverlay_));
        }
        ~MVSliceView() {
            MsgDistr.remove(this);
            delete painter_;
            delete camera_;
//             delete borderOverlay_;
//             delete trackball_;
        }

        virtual void processMessage(Message* msg, const Identifier& dest = Identifier::all);

        virtual void mousePressEvent(tgt::MouseEvent* e);
        virtual void mouseReleaseEvent (tgt::MouseEvent* e);
        virtual void mouseMoveEvent(tgt::MouseEvent* e);
        virtual void mouseDoubleClickEvent(tgt::MouseEvent* e);
        virtual void wheelEvent(tgt::MouseEvent* e);

    private:

        tgt::Trackball* trackball_;
        BorderOverlay* borderOverlay_;
        MultiView* multiView_;
        size_t index_;
        bool leftPressed_;
        float oldMouseY_;
    };

public:
    enum {
        TOP_LEFT,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT
    };

    MultiViewWidget(tgt::Camera* camera3D, tgt::EventListener* eventListener, VolumeContainer* volumeContainer, QWidget* parent = 0, bool multimodal=false);
    ~MultiViewWidget();

    void setCamera3D(tgt::Camera* cam) {
        canvases_[0]->setCamera(cam);
    }

    tgt::Camera* getCamera3D() {
        return canvases_[0]->getCamera();
    }

private:

    tgt::GLCanvas* canvases_[4];
    MultiView* multiView_;
};

} // namespace voreen

#endif // VRN_MULTIVIEWWIDGET_H
