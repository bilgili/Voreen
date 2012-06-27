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

#ifndef VRN_OVERVIEWWIDGET_H
#define VRN_OVERVIEWWIDGET_H

#include <QWidget>

#include "tgt/camera.h"
#include "tgt/qt/qtcanvas.h"

#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

class TextureContainer;
class OverviewRenderer;

class OverViewWidget : public tgt::QtCanvas, public tgt::EventListener, public voreen::MessageReceiver {
public:
    OverViewWidget(tgt::EventListener* eventListener, QWidget* parent, TextureContainer* tc = 0, tgt::Camera* cam = 0);
    ~OverViewWidget();

    virtual void processMessage(Message* msg, const Identifier& dest = Message::all_);

    virtual void mousePressEvent(tgt::MouseEvent* e);
    virtual void mouseReleaseEvent (tgt::MouseEvent* e);
    virtual void mouseMoveEvent(tgt::MouseEvent* e);
    virtual void mouseDoubleClickEvent(tgt::MouseEvent* e);
    virtual void wheelEvent(tgt::MouseEvent* e);

private:

    /*
        Perhaps you wonder, why the OverviewRenderer has a small 'v'.
        Well the Overview renderer does not know about something like views or viewwidgets.
        It is just an "overview" renderer.
    */
    OverviewRenderer* or_;
    bool leftPressed_;
    bool rightPressed_;   
    float oldMouseX_;
    float oldMouseY_;
    tgt::vec3 translation_;
    float scale_;
};

} // namespace voreen

#endif // VRN_OVERVIEWWIDGET_H
