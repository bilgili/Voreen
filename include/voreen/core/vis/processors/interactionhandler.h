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

#ifndef VRN_INTERACTIONHANDLER_H
#define VRN_INTERACTIONHANDLER_H

#include "tgt/event/eventlistener.h"

#include "voreen/core/vis/voreenpainter.h"

class EventListener;

namespace voreen{

/**
  * This class provides the possibility to activate and deactivate a special EventListern.
  * A deactivated class won't get any Events any more
  * An active class can change about which Events it wants to be informed.
  * For detailed information how to select the Events take a look at event.h,
  * you will find a spezified information about the bitmask "defaultEventTypes"
  */
class InteractionHandler: public tgt::EventListener {
    
public:
    InteractionHandler();
    ~InteractionHandler();

    /// Activates the class for the Events which are given by defaultEventTypes.
    void activate();
    /// Deactivates the class so that he doesn't get any events any more.
    void deactivate();

    /// Sets the default EventTypes to the BitMask \a defaultEventTypes.
    void setDefaultEventTypes(int defaultEventTypes);
    /// Returns the BitMask containing the default EventTypes.
    int getDefaultEventTypes();


private:
    /// BitMask which indicates to which EventTypes this class listens
    int defaultEventTypes_;

};

} // namespace

#endif // VRN_INTERACTIONHANDLER_H
