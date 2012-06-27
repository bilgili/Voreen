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

#ifndef VRN_SPLITVIEWWIDGET_H
#define VRN_SPLITVIEWWIDGET_H

#include <vector>

#include <QWidget>

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/container/splitview.h"
#include "voreen/core/vis/voreenpainter.h"
#include "tgt/qt/qtcanvas.h"
#include "voreen/core/vis/trackballnavigation.h"

namespace tgt {
    class Camera;
}

namespace voreen {

/**
 * This class capsulates a widget with two canvases.
 * It destroys all owned objects by itself.
*/
class SplitViewWidget : public QWidget {
    Q_OBJECT

    class Split3dView : public tgt::EventListener, public tgt::QtCanvas, public voreen::MessageReceiver {
    public:
        Split3dView(QWidget* parent, Processor* processor, tgt::Camera* camera)
            : tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, parent, true, 0)
            , voreen::MessageReceiver("splitview_1")
        {
            camera_ = camera;
            tgt::Trackball* track = new tgt::Trackball(this, false);
            painter_ = new VoreenPainter(this, track, "splitview_1");
            //RPTMERGE
            //((VoreenPainter*)painter_)->setProcessor(processor);
            eventHandler_->addListenerToBack(this);
            eventHandler_->addListenerToBack(new TrackballNavigation(track, true));
            MsgDistr.insert(this);
            setMouseTracking(true);
        }
        ~Split3dView() {
            MsgDistr.remove(this);
            //RPTMERGE
            //((VoreenPainter*)painter_)->getProcessor()->deinit();
            delete painter_;
        }

        virtual void mouseMoveEvent(tgt::MouseEvent* e);

        void setSplitView(SplitView* splitView) {
            splitView_ = splitView;
        }

    private:

        SplitView* splitView_;
    };

    class SplitEmphasizedView : public tgt::QtCanvas, public voreen::MessageReceiver {
    public:
        SplitEmphasizedView(QWidget* parent,
            Processor* processor,
            Split3dView* split3dView
        )
            : tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, parent, true, 0)
            , voreen::MessageReceiver("splitview_1")
            , split3dView_(split3dView)
        {
            camera_ = new tgt::Camera();
            tgt::Trackball* track = new tgt::Trackball(this, false);
            painter_ = new VoreenPainter(this, track, "splitview_1");
            //RPTMERGE
            //((VoreenPainter*)painter_)->setProcessor(processor);
            MsgDistr.insert(this);
            startTimer(10);
        }
        ~SplitEmphasizedView() {
            MsgDistr.remove(this);
            //RPTMERGE
            //((VoreenPainter*)painter_)->getProcessor()->deinit();
            delete painter_;
        }

        virtual void processMessage(Message* msg, const Identifier& dest = Identifier::all);

    private:

        SplitView* splitView_;
        Split3dView* split3dView_;
    };

public:

    SplitViewWidget(tgt::Camera* camera3D, tgt::EventListener* eventListener, QWidget* parent = 0);
    ~SplitViewWidget();

    void setCamera3D(tgt::Camera* cam) {
        canvases_[0]->setCamera(cam);
    }

    tgt::Camera* getCamera3D() {
        return canvases_[0]->getCamera();
    }

private:

    tgt::GLCanvas* canvases_[2];
    SplitView* splitView_;
};

} // namespace voreen

#endif // VRN_SPLITVIEWWIDGET_H
