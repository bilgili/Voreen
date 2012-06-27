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

#ifndef VRN_UNDOABLEANIMATION_H
#define VRN_UNDOABLEANIMATION_H

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

class Animation;

/**
 * An abstract class for undo- and redo-functions in the animation core.
 * Classes containing data and provide undo- and redo-functions for this data can implement this class.
 */
class VRN_CORE_API UndoableAnimation : public AbstractSerializable {
public:

    /**
     * This function provides an undo-functionality (timeline-level).
     */
    virtual void undo() = 0;

    /**
     * This function provides a redo-functionality (timeline-level) and is basically inverse to undo.
     */
    virtual void redo() = 0;

    /*
     * This function deletes all redo-states.
     */
    virtual void clearRedoStates() = 0;

    /**
     * This function deletes all states.
     */
    virtual void clearAllStates() = 0;

    /**
     * This function removes the oldest undo-state.
     * If the number of possible undo-steps is decreased, this function adapts the related stack.
     */
    virtual void removeOldestUndoState() = 0;

    /**
     * Register an undoObserver.
     * After a change was made, observer->animationChanged(undoAbleAnimation) can be called.
     * This observer manages all undo-states of the whole animation and calls the undo and redo-functions if needed.
     */
    virtual void registerUndoObserver(Animation* observer) = 0;

    /**
     * Creates a new undo-state
     */
    virtual void setNewUndoState() = 0;
};

} // namespace voreen

#endif
