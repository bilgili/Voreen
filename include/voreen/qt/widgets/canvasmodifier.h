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

#ifndef CANVASMODIFIER_H
#define CANVASMODIFIER_H

#include "tgt/qt/qtcanvas.h"
#include "tgt/event/keyevent.h"
#include "tgt/event/eventlistener.h"

namespace voreen {

/**
 * Widget for performance tests. "D" detaches canvas from application
 */
class CanvasModifier : public QObject, public tgt::EventListener {
    Q_OBJECT
public:
    CanvasModifier(tgt::GLCanvas* canvas, bool useSignals = false);

    void connect();
    void disconnect();

signals:
    // used for tab widgets
    void detachFromTab();
    void attachToTab();

protected:
    virtual void keyEvent(tgt::KeyEvent* e);
    tgt::GLCanvas* canvas_;
    QWidget* canvasWidget_;

    // detach canvas
    static const tgt::KeyEvent::KeyCode keyShowWidget_ = tgt::KeyEvent::K_D;
    // print current size to console
    static const tgt::KeyEvent::KeyCode keyPrint_ = tgt::KeyEvent::K_P;

private:
    bool canvasDetached_;
    bool useSignals_;
    int index_;
};

} // namespace voreen

#endif //CANVASMODIFIER_H
