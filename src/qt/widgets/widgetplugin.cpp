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

#include "voreen/qt/widgets/widgetplugin.h"

#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

QIcon WidgetPlugin::getIcon() {
    return icon_;
}

void WidgetPlugin::setMessageReceiver(MessageReceiver* receiver) {
    msgReceiver_ = receiver;
}

void WidgetPlugin::startTracking() {
    MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, true));
    repaintCanvases();
}

void WidgetPlugin::stopTracking() {
	MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false));
    repaintCanvases();
}

void WidgetPlugin::repaintCanvases() {
    MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

void WidgetPlugin::postMessage(Message* msg, const Identifier& dest) {
    if (msgReceiver_)
        msgReceiver_->postMessage(msg, dest);
}

} // namespace voreen
