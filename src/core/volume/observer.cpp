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

#include "voreen/core/volume/observer.h"

namespace voreen {

Observer::Observer() {
}

Observer::~Observer() {
    clearObserverds();
}

bool Observer::addObserved(Observable* const observed) {
    if (observed == 0)
        return false;

    std::pair<std::set<Observable*>::iterator, bool> pr = observeds_.insert(observed);
    if( pr.second == true )
        observed->addObserver(this);

    return pr.second;
}

Observable* Observer::findObserved(Observable* const observed) {
    if( observed == 0 )
        return 0;

    std::set<Observable*>::iterator it = observeds_.find(observed);
    if( it != observeds_.end() )
        return *it;

    return 0;
}

bool Observer::removeObserved(Observable* const observed) {
    if (observed == 0)
        return false;

    // Calling the private version removeObserverFriend() of the friend
    // class Observable prevents from the need of calling removeObserver().
    // If the latter method would be called from here, the methods removeObserved()
    // and removeObserver() would call each other mutually and to hook up
    // in an infinite loop!
    //
    observed->removeObserverFriend(this);

    observeds_.erase(observed);
    return true;
}

void Observer::clearObserverds() {
    std::set<Observable*>::iterator it = observeds_.begin();
    for( ; it != observeds_.end(); ++it ) {
        if( (*it) != 0 )
            // Do not call removeObserver() as that would end
            // up in an infinite loop!
            //
            (*it)->removeObserverFriend(this);
    }

    observeds_.clear();
}

int Observer::getNumObserveds() const {
    return static_cast<int>(observeds_.size());
}

// private methods
//

bool Observer::removeObservedFriend(Observable* const observed) {
    // Only remove the observable object from the set! Do not
    // try to remove this on the observed object, as this method
    // is only to be called from Observable, so the removal is
    // not necessary or has already been done!
    //
    if (observed == 0)
        return false;
    observeds_.erase(observed);
    return true;
}

// ---------------------------------------------------------------------------

Observable::Observable() {
}

Observable::~Observable() {
    clearObservers();
}

bool Observable::addObserver(Observer* const observer) {
    if (observer == 0)
        return false;

    std::pair<std::set<Observer*>::iterator, bool> pr = observers_.insert(observer);
    if( (pr.second == true) && (observer->findObserved(this) == 0) )
        observer->addObserved(this);

    return pr.second;
}

Observer* Observable::findObserver(Observer* const observer) {
    if( observer == 0 )
        return 0;

    std::set<Observer*>::iterator it = observers_.find(observer);
    if( it != observers_.end() )
        return *it;

    return 0;
}

bool Observable::removeObserver(Observer* const observer) {
    if (observer == 0)
        return false;

    // Calling the private version removeObserverFriend() of the friend
    // class Observable prevents from the need of calling removeObserver().
    // If the latter method would be called from here, the methods removeObserved()
    // and removeObserver() would call each other mutually and to hook up
    // in an infinite loop!
    //
    observer->removeObservedFriend(this);

    observers_.erase(observer);
    return true;
}

void Observable::clearObservers() {
    std::set<Observer*>::iterator it = observers_.begin();
    for( ; it != observers_.end(); ++it ) {
        if( (*it) != 0 )
            // Do not call removeObserved(this) as it would
            // end up in an infinite loop!
            //
            (*it)->removeObservedFriend(this);
    }

    observers_.clear();
}

int Observable::getNumObservers() const {
    return static_cast<int>(observers_.size());
}

void Observable::notifyObservers() const {
    if (observers_.empty())
        return;

    // must work on a copy as notify() might delete elements
    std::set<Observer*> observers = observers_;   
    std::set<Observer*>::const_iterator it = observers.begin();
    for (std::set<Observer*>::const_iterator it = observers.begin();
        it != observers.end(); ++it)
    {
        (*it)->notify(this);
    }
}

// private methods
//

bool Observable::removeObserverFriend(Observer* const observer) {
    // Only remove the observing object from the set! Do not
    // try to remove this on the observing object, as this method
    // is only to be called from Observer, so the removal is
    // not necessary or has already been done!
    //
    if (observer == 0)
        return false;
    observers_.erase(observer);
    return true;
}

} // namespace
