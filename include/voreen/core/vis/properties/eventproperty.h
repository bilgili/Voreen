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

#ifndef VRN_EVENTPROPERTY_H
#define VRN_EVENTPROPERTY_H

#include "voreen/core/vis/properties/property.h"
#include "tgt/event/event.h"
#include "tgt/event/keyevent.h"
#include "tgt/event/mouseevent.h"

namespace voreen {

class EventProperty : public Property {
public:
    EventProperty(const std::string& guiText, tgt::Event::Modifier modifier, tgt::KeyEvent::KeyCode keyCode);
    EventProperty(const std::string& guiText, tgt::Event::Modifier modifier, tgt::MouseEvent::MouseButtons mouseButtons);

    tgt::Event::Modifier getModifier();
    void setModifier(tgt::Event::Modifier value);

    tgt::MouseEvent::MouseButtons getMouseButtons();
    void setMouseButtons(tgt::MouseEvent::MouseButtons value); 

    tgt::KeyEvent::KeyCode getKeyCode();
    void setKeyCode(tgt::KeyEvent::KeyCode value);

    bool accepts(tgt::MouseEvent* e);
    bool accepts(tgt::KeyEvent* e);

    bool isMouseEvent();
    bool isKeyEvent();

    virtual std::string toString() const;

protected:
    bool isMouseEvent_;
    tgt::Event::Modifier modifier_;
    tgt::MouseEvent::MouseButtons mouseButtons_;
    tgt::KeyEvent::KeyCode keyCode_;
};

} // namespace

#endif // VRN_EVENTPROPERTY_H
