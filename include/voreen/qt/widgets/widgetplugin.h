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

#ifndef VRN_WIDGETPLUGIN_H
#define VRN_WIDGETPLUGIN_H

#include <list>

#include <QWidget>
#include <QIcon>

#include "voreen/core/vis/message.h"

namespace voreen {

class WidgetPlugin : public QWidget {
    Q_OBJECT
public:
    WidgetPlugin(QWidget* parent = 0, MessageReceiver* msgReceiver = 0)
        : QWidget(parent), msgReceiver_(msgReceiver)
    {}
    
    virtual ~WidgetPlugin() {}

    QIcon getIcon();
    virtual void createWidgets() = 0;
    virtual void createConnections() = 0;
    virtual void setMessageReceiver(MessageReceiver* receiver);

    virtual bool usable(const std::vector<Processor*>& processors);

public slots:
    virtual void startTracking();
    virtual void stopTracking();
    virtual void repaintCanvas();

protected:
    virtual void postMessage(Message* msg, const Identifier& dest = Message::all_);

    QIcon icon_;
    MessageReceiver* msgReceiver_;
};

} // namespace voreen

#endif // VRN_WIDGETPLUGIN_H
