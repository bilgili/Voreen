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

#include "voreen/core/observer.h"

namespace voreen {

Observer::~Observer() {
    clearObserveds();
}

bool Observer::observes(const ObservableBase* observable) const {
    return (observeds_.find(const_cast<ObservableBase*>(observable)) != observeds_.end());
}

void Observer::clearObserveds() {
    while (!observeds_.empty())
        removeObserved(*observeds_.begin());
}

bool Observer::addObserved(ObservableBase* observed) {

    if (!observed)
        return false;

    std::set<ObservableBase*>::iterator it = observeds_.find(observed);
    if (it != observeds_.end())
        return false;

    std::pair<std::set<ObservableBase*>::iterator, bool> pr = observeds_.insert(observed);
    if (pr.second == true) {
        observed->addObserver(this);
    }
    return pr.second;

}

bool Observer::removeObserved(ObservableBase* observed) {

    if (!observed)
        return false;

    if (observeds_.find(observed) != observeds_.end()) {
        observeds_.erase(observed);
        observed->removeObserver(this);
        return true;
    }
    else {
        return false;
    }
}

void Observer::stopObservation(ObservableBase* observable) {
    removeObserved(observable);
}

// ---------------------------------------------------------------------------

ObservableBase::~ObservableBase() {
    clearObservers();
}

void ObservableBase::clearObservers() {
    while (!observers_.empty())
        removeObserver(*observers_.begin());
}

bool ObservableBase::addObserver(Observer* observer) {

    if (!observer)
        return false;

    std::pair<std::set<Observer*>::iterator, bool> pr = observers_.insert(observer);
    if (pr.second)
        observer->addObserved(this);

    return pr.second;
}

bool ObservableBase::removeObserver(Observer* observer) {

    if (!observer)
        return false;

    if (observers_.find(observer) != observers_.end()) {
        observers_.erase(observer);
        observer->removeObserved(this);
        return true;
    }
    else {
        return false;
    }
}

} // namespace
