/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_PROPERTYTIMELINE_H
#define VRN_PROPERTYTIMELINE_H

#include <string>
#include "voreen/core/animation/undoableanimation.h"
#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

class Animation;
class Property;

/**
 * Interface for the templated PropertyTimelines.
 */
class VRN_CORE_API PropertyTimeline : public UndoableAnimation {

public:

    /**
     * Calls the render-method of the timeline.
     */
    virtual void renderAt(float time) = 0;

    /**
     * Returns the name of the animated property.
     */
    virtual std::string getPropertyName() const = 0;

    /// returns true if there ist no keyvalue
    virtual bool isEmpty() = 0;

    /**
     * The current setting of the corresponding property in the rendernetwork is
     * added as a keyvalue at the given time into the timeline.
     */
    virtual void setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) = 0;

     /**
     * Returns if the property is animated on rendering.
     */
    virtual bool getActiveOnRendering() const = 0;

    /**
    * Sets the rendering state for the corresponding property
    */
    virtual void setActiveOnRendering(bool) = 0;

    /**
     * Called by the animation-class to set the interactionmode.
     */
    virtual void setInteractionMode(bool interactionmode, void* source) = 0;

    /**
     * Called explicitly to force the creation of a new undostep.
     */
    virtual void setNewUndoState() = 0;

    /**
     * Called from animation-class.
     * All keyvalues after the duration-time will be deleted.
     */
    virtual void setDuration(float duration) = 0;

    // from undoAbleAnimation:
    /**
     * This method provides an undo-functionality (timeline-level).
     */
    virtual void undo() = 0;

    /**
     * This method provides a redo-functionality (timeline-level) and is basically inverse to undo.
     */
    virtual void redo() = 0;

    /*
     * This method deletes all redo-states.
     */
    virtual void clearRedoStates() = 0;

    /**
     * This method deletes all states.
     */
    virtual void clearAllStates() = 0;

    /**
     * This method removes the oldest undo-state.
     * If the number of possible undo-steps is decreased, this method adapts the related stack.
     */
    virtual void removeOldestUndoState() = 0;

    /**
     * Returns true if the timeline was changed.
     */
    virtual bool isChanged() const = 0;

    virtual Property* getProperty() const = 0;

protected:
    float duration_;

};

} // namespace voreen

#endif // VRN_PROPERTYTIMELINE_H
