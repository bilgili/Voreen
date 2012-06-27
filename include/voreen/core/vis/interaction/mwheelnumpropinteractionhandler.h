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

#ifndef VRN_MWHEELNUMPROPINTERACTIONHANDLER_H
#define VRN_MWHEELNUMPROPINTERACTIONHANDLER_H

#include "voreen/core/vis/interaction/interactionhandler.h"
#include "voreen/core/vis/properties/numericproperty.h"
#include "voreen/core/vis/properties/boolproperty.h"

#include "tgt/event/mouseevent.h"

namespace voreen {

template<typename T>
class MWheelNumPropInteractionHandler: public InteractionHandler {
public:
    MWheelNumPropInteractionHandler(NumericProperty<T>* numProp);
    virtual ~MWheelNumPropInteractionHandler() {}

    virtual void onEvent(tgt::Event* e);

private:
    NumericProperty<T>* numProp_;
    BoolProperty active_;
};

template<typename T>
MWheelNumPropInteractionHandler<T>::MWheelNumPropInteractionHandler(NumericProperty<T>* numProp) :
    InteractionHandler(),
    active_("handler.active", "Mouse wheel handler active?", true, Processor::VALID) {

    tgtAssert(numProp, "No numeric property");

    numProp_ = numProp;
    addProperty(active_);
}

template<typename T>
void MWheelNumPropInteractionHandler<T>::onEvent(tgt::Event* eve) {
    tgtAssert(numProp_, "No numeric property");

    // ignore event, if handler not active
    if (active_.get() == false) {
        eve->ignore();
        return;
    }

    if (tgt::MouseEvent* mouseEve = dynamic_cast<tgt::MouseEvent*>(eve)) {
        if (mouseEve->action() == tgt::MouseEvent::WHEEL) {
            if(mouseEve->button() == tgt::MouseEvent::MOUSE_WHEEL_UP) {
                if (numProp_->get() != numProp_->getMaxValue()) {
                    if ((numProp_->get() + numProp_->getStepping()) > numProp_->getMaxValue())
                        numProp_->set(numProp_->getMaxValue());
                    else
                        numProp_->increase();
                }
            }
            else if(mouseEve->button() == tgt::MouseEvent::MOUSE_WHEEL_DOWN) {
                if (numProp_->get() != numProp_->getMinValue()) {
                    if ((numProp_->get() - numProp_->getStepping()) < numProp_->getMinValue())
                        numProp_->set(numProp_->getMinValue());
                    else
                        numProp_->decrease();
                }
            }
        }
        eve->accept();
    }
}
} // namespace

#endif // VRN_MWHEELNUMPROPINTERACTIONHANDLER_H
