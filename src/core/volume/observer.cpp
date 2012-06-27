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

#ifndef VRN_OBSERVER_H

#include "voreen/core/volume/observer.h"

namespace voreen {

Observable::Observable() {
}

Observable::~Observable() {
}

bool Observable::addObserver(Observer* const observer) {
    if (observer == 0)
        return false;
    std::pair<std::set<Observer*>::iterator, bool> pr = observers_.insert(observer);
    return pr.second;
}

bool Observable::removeObserver(Observer* const observer) {
    if (observer == 0)
        return false;

    observers_.erase(observer);
    return true;
}

void Observable::clearObservers() {
    observers_.clear();
}

int Observable::getNumObservers() const {
    return static_cast<int>(observers_.size());
}

void Observable::notifyObservers() const {
    if (observers_.empty())
        return;

    std::set<Observer*>::const_iterator it = observers_.begin();
    for ( ; it != observers_.end(); ++it) {
        if(*it != 0)
            (*it)->notify(this);
    }
}

} // namespace

#endif
