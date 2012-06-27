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

#ifndef VRN_STACKVIEWWIDGET_H
#define VRN_STACKVIEWWIDGET_H

#include <QWidget>

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/container/stackview.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/trackballnavigation.h"
#include "tgt/qt/qtcanvas.h"


namespace tgt {
    class Camera;
};

namespace voreen {

class StackViewWidget : public QWidget {
    Q_OBJECT

    class Stack3dView : public tgt::EventListener, public tgt::QtCanvas {
    public:
        Stack3dView(QWidget* parent, Processor* processor, tgt::Camera* camera)
            : tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, parent, true, 0)
        {
            camera_ = camera;
            tgt::Trackball* track = new tgt::Trackball(this, false);
            painter_ = new VoreenPainter(this, track, "sv1");
            //RPTMERGE
            //((VoreenPainter*)painter_)->setProcessor(processor);
            MsgDistr.insert((VoreenPainter*)painter_);
            eventHandler_->addListenerToBack(this);
            eventHandler_->addListenerToBack(new TrackballNavigation(track, true));
            startTimer(10);
            setMouseTracking(true);
        }
        ~Stack3dView() {
            MsgDistr.remove((VoreenPainter*)painter_);
            //RPTMERGE
            //((VoreenPainter*)painter_)->getProcessor()->deinit();
            delete painter_;
        }
        virtual void mouseMoveEvent(tgt::MouseEvent* e);

        void setStackView(StackView* stackView) {
            stackView_ = stackView;
        }

    private:

        StackView* stackView_;
    };

    class StackSliceView : public tgt::EventListener, public tgt::QtCanvas, public voreen::MessageReceiver {
    public:
        StackSliceView(QWidget* parent,
            Processor* processor,
            Stack3dView* stack3dView
        )
            : tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, parent, true, 0)
            , voreen::MessageReceiver("sv1")
            , stack3dView_(stack3dView)
        {
            camera_ = new tgt::Camera();
            tgt::Trackball* track = new tgt::Trackball(this, false);
            painter_ = new VoreenPainter(this, track, "sv1");
            //RPTMERGE
            //((VoreenPainter*)painter_)->setProcessor(processor);
            MsgDistr.insert(this);
        }
        ~StackSliceView() {
            MsgDistr.remove(this);
            //RPTMERGE
            //((VoreenPainter*)painter_)->getProcessor()->deinit();
            delete painter_;
        }

        virtual void processMessage(Message* msg, const Identifier& dest = Identifier::all);

    private:
        StackView* stackView_;
        Stack3dView* stack3dView_;
    };

public:

    StackViewWidget(tgt::Camera* camera3D, tgt::EventListener* eventListener, QWidget* parent = 0);
    ~StackViewWidget();

    void setCamera3D(tgt::Camera* cam) {
        canvases_[0]->setCamera(cam);
    }

    tgt::Camera* getCamera3D() {
        return canvases_[0]->getCamera();
    }

private:

    tgt::GLCanvas* canvases_[8];
    StackView* stackView_;
};

} // namespace voreen

#endif // VRN_STACKVIEWWIDGET_H
