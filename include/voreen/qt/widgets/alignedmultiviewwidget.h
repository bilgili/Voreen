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

#ifndef VRN_ALIGNEDMULTIVIEWWIDGET_H
#define VRN_ALIGNEDMULTIVIEWWIDGET_H

#include <QWidget>

#include "voreen/core/vis/trackballnavigation.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/container/alignedmultiview.h"
#include "voreen/core/vis/voreenpainter.h"
#include "tgt/qt/qtcanvas.h"


class tgt::Camera;

namespace voreen {

/**
 * This class capsulates a widget with four canvases.
 * It destroys all owned objects by itself.
 */
class AlignedMultiViewWidget : public QWidget {
    Q_OBJECT

    class AMV3dView : public tgt::QtCanvas, public voreen::MessageReceiver {
    public:
        AMV3dView(QWidget* parent, Processor* processor, tgt::Camera* camera3D)
            : tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, parent, true, 0)
            , voreen::MessageReceiver("amv1")
        {
            camera_ = camera3D;
            tgt::Trackball* track = new tgt::Trackball(this, false);
            painter_ = new VoreenPainter(this, track, "amv1");
            //RPTMERGE
            //((VoreenPainter*)painter_)->setProcessor(processor);
            eventHandler_->addListenerToBack(new TrackballNavigation(track, true));
            MsgDistr.insert(this);
            startTimer(10);
        }
        ~AMV3dView() {
            MsgDistr.remove(this);
            //RPTMERGE
            //((VoreenPainter*)painter_)->getProcessor()->deinit();
            delete painter_;
        }
        void processMessage(Message* msg, const Identifier& dest = Identifier::all);
    };

    class AMVSliceView : public tgt::QtCanvas, public tgt::EventListener, public voreen::MessageReceiver {
    public:
        AMVSliceView(QWidget* parent,
            Processor* processor,
            AMV3dView* amv3dView,
            AlignedMultiView* alignedMultiView,
            size_t index
        )
            : tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, parent, true, 0)
            , voreen::MessageReceiver("amv1")
            , amv3dView_(amv3dView)
            , alignedMultiView_(alignedMultiView)
            , index_(index)
            , leftPressed_(false)
            , oldMouseY_(0)
        {
            camera_ = new tgt::Camera();
            tgt::Trackball* track = new tgt::Trackball(this, false);
            painter_ = new VoreenPainter(this, track, "amv1");
            //RPTMERGE
            //((VoreenPainter*)painter_)->setProcessor(processor);
            eventHandler_->addListenerToBack(this);
        }
        ~AMVSliceView() {
            MsgDistr.remove(this);
            //RPTMERGE
            //((VoreenPainter*)painter_)->getProcessor()->deinit();
            delete painter_;
        }

        virtual void processMessage(Message* msg, const Identifier& dest = Identifier::all);

        virtual void mousePressEvent(tgt::MouseEvent* e);
        virtual void mouseReleaseEvent(tgt::MouseEvent* e);
        virtual void mouseMoveEvent(tgt::MouseEvent* e);
        virtual void mouseDoubleClickEvent(tgt::MouseEvent* e);
        virtual void wheelEvent(tgt::MouseEvent* e);

    private:

        AMV3dView* amv3dView_;
        AlignedMultiView* alignedMultiView_;
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

    AlignedMultiViewWidget(tgt::Camera* camera, tgt::EventListener* eventListener, QWidget* parent = 0);
    ~AlignedMultiViewWidget();

    void setCamera3D(tgt::Camera* cam) {
        canvases_[0]->setCamera(cam);
    }

    tgt::Camera* getCamera3D() {
        return canvases_[0]->getCamera();
    }

private:

    tgt::GLCanvas* canvases_[4];
    AlignedMultiView* alignedMultiView_;
};

} // namespace voreen

#endif // VRN_ALIGNEDMULTIVIEWWIDGET_H
