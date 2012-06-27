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

#include "voreen/core/vis/processors/processor.h"

namespace tgt {
    class EventListener;
}

namespace voreen {

class Processor;
class Property;

/**
 * Abstract superclass for interaction handlers that can be attached to processors
 * and react to user events.
 */
class InteractionHandler : public tgt::EventListener {
friend class Processor;
public:
    InteractionHandler();
    virtual ~InteractionHandler() {};

    virtual void onEvent(tgt::Event* e) = 0;

    /**
     * Returns the processor this handler is assigned to.
     */
    Processor* getOwner() const;

    /**
     * Returns all registered properties of this handler.
     *
     * \sa addProperty
     */
    virtual const std::vector<Property*>& getProperties() const;

protected:
    void setOwner(Processor* p);

    /**
     * Registers a property at the interaction handler. Should
     * be called in the constructor.
     */
    virtual void addProperty(Property* prop);
    virtual void addProperty(Property& prop);

    /// The processor this interaction handler is associated with
    Processor* owner_;

    /// Properties of this handler.
    std::vector<Property*> properties_;
};

} // namespace

#endif // VRN_INTERACTIONHANDLER_H
