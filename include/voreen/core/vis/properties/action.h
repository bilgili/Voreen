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

#ifndef VRN_ACTION_H
#define VRN_ACTION_H

#include "voreen/core/vis/properties/property.h"

namespace voreen {

class Processor;

/**
 * An Action can be executed by a Property when a Condition is met.
 */
class Action {
public:
    virtual ~Action() {}
    virtual Action* clone() const = 0;
    virtual void exec() = 0;
    virtual bool isNoAction() const { return false; }
};

// ----------------------------------------------------------------------------

/**
 * This Action does nothing.
 */
class NoAction : public Action {
public:
    virtual ~NoAction() {}
    void exec() {} // do nothing
    virtual bool isNoAction() const { return true; }
    virtual NoAction* clone() const { return new NoAction(); }
};

} // namespace voreen

#endif // VRN_ACTION_H
