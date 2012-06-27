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

#ifndef VRN_CHANGEACTION_H
#define VRN_CHANGEACTION_H

#include <vector>

#include "voreen/core/vis/properties/action.h"
#include "voreen/core/vis/properties/link/changedata.h"
#include "voreen/core/vis/properties/link/propertylink.h"

namespace voreen {

typedef PropertyLink* ChangeListenerType;
typedef std::vector<ChangeListenerType> ChangeListenerList;

/**
 * The ChangeAction class handles the ChangeListener issues to inform
 * PropertyLinks about value changes if the set-function was called.
 */
class ChangeAction : public Action {
public:
    /**
     * Default constructor.
     */
    ChangeAction();

    /**
     * see Action::clone().
     */
    virtual ChangeAction* clone() const;

    /**
     * Calls OnChange-callback function on every listener.
     *
     * see also Action::exec().
     */
    virtual void exec();

    /**
     * Registers a change listener.
     */
    void registerLink(const ChangeListenerType& listener);

    /**
     * Unregisters a change listener.
     */
    void unregisterLink(const ChangeListenerType& listener);

    /**
     * Sets old value of property.
     */
    void setOldValue(const BoxObject& value);

    /**
     * Sets new value of property.
     */
    void setNewValue(const BoxObject& value);

protected:
    ChangeData data_;
    ChangeListenerList changeListeners_;
};

} // namespace

#endif // VRN_CHANGEACTION_H
