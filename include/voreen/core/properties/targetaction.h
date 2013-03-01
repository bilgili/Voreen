/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#ifndef VRN_TARGETACTION_H
#define VRN_TARGETACTION_H

#include "voreen/core/properties/action.h"

namespace voreen {

/**
 * Inherit from a TagetAction<YourTarget> and you will get an action
 * that acts on a target.
 */
template<class T>
class TargetAction : public Action {
public:
    TargetAction() : target_(0) {}
    TargetAction(T* target) : target_(target) {}
    virtual ~TargetAction() {}

    /**
     * Returns a pointer to the target of the action.
     */
    T* target() const { return target_; }

protected:
    T* target_;
};

// ============================================================================

}   // namespace

#endif
