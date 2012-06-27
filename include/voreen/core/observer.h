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

#ifndef VRN_OBSERVER_H
#define VRN_OBSERVER_H

#include "tgt/assert.h"
#include "tgt/logmanager.h"

#include <set>
#include <vector>

namespace voreen {

class ObservableBase;

/**
 * \brief Quite general implementation of the observer design pattern, takes
 * care of relationship management between observers and observed objects.
 *
 * Derive from this class in order to define a specialized Observer with
 * custom notification functions that can be registered at Observable objects
 * of matching type.
 *
 * @see Observable
 */
class Observer {

friend class ObservableBase; // for allowing ObservableBase to call addObserved() and removeObserved()

public:

    /**
     * Destructor which removes this Observer object from all Observable objects
     * which it is registered at. Thus, the user does not need to remove this
     * object from any Observable object, unless he wants to explicitly
     * stop observing it.
     */
    virtual ~Observer();

    /**
     * Returns whether this Observer currently observes the
     * passed Observable.
     */
    bool observes(const ObservableBase* observable) const;

    /**
     * Causes the Observer to stop observation of the passed
     * Observable.
     *
     * @note It is safe to call this function even if
     *  the passed Observable is not observed by this object
     *  or the Observable has already been deleted. It also
     *  allowed to pass the null pointer.
     */
    void stopObservation(ObservableBase* observable);

private:

    /**
     * Adds the passed Observable object to those objects that are observed by
     * this observable and registers itself at the passed object.
     * An observer can only be added once.
     *
     * @param   observed the object that is to be observed
     * @return  true if the object has been inserted successfully, false
     *          if the null pointer are on an already registered object was passed.
     */
    bool addObserved(ObservableBase* observed);

    /**
     * Removes the passed Observable object from the list of observed objects, if
     * it has been observed, and deregisters itself at the passed object.
     *
     * @param   observed the observed object that is to be removed
     * @return  true if the passed object was removed, false if it
     *          was not registered.
     */
    bool removeObserved(ObservableBase* observed);

    /**
     * Clears the list of all observed objects. Like the destructor, this method
     * removes this object itself from Observable objects.
     */
    void clearObserveds();

    /// Set of observed objects.
    std::set<ObservableBase*> observeds_;

};


// ---------------------------------------------------------------------------

/**
 * Base class for Observable classes. Do not subclass it directly!
 *
 * @see Observable
 */
class ObservableBase {

friend class Observer; // for allowing Observer to call addObserver() and removeObserver()

public:

    /// @see Observable
    virtual ~ObservableBase();

protected:

    /// @see Observable
    virtual bool addObserver(Observer* observer);

    /// @see Observable
    virtual bool removeObserver(Observer* observer);

     /// @see Observable
    void clearObservers();

    /// Set of observers.
    std::set<Observer*> observers_;

};

// ---------------------------------------------------------------------------

/**
 * \brief Base class for Observable classes, takes care of relationship management
 * between observers and observed objects. Subclass it in order to allow
 * Observers of a certain type to monitor instances of the derived class.
 *
 * @tparam T Type of observers that are to be allowed to register
 *      at the derived class.
 *
 * @see Observer
 */
template<class T>
class Observable : public ObservableBase {

public:

    /**
     * Besides simply destroying this object, the destructor removes this object
     * from the list of all Observer objects watching it. The user does therefore
     * not need to manually remove this object from all its observers.
     */
    virtual ~Observable() {}

    /**
     * Adds the passed Observer to the list of observing ones and registers
     * this object at the passed observer.
     *
     * @note The observer must be of type T and can only be inserted once.
     *
     * @param observer Observer of type T to be inserted
     * @return true if the insertion has been successful, false otherwise
     */
    bool addObserver(Observer* observer);

    /**
     * Removes the passed Observed object from the list of observers observing
     * this object and de-registers this object from the passed observer.
     *
     * @param observer the observer to be removed
     * @return true if the observer was removed, false if the passed object
     *  is not registered at this observable.
     */
    bool removeObserver(Observer* observer);

    /**
     * Returns whether this object is observed by the
     * passed one.
     */
    bool isObservedBy(Observer* observer) const;

protected:

    /**
     * Returns all observers currently registered at this observable.
     */
    const std::vector<T*> getObservers() const;

};


// ---------------------------------------------------------------------------
//   Template Definitions
// ---------------------------------------------------------------------------

template<class T>
bool Observable<T>::addObserver(Observer* observer) {

    if (!dynamic_cast<T*>(observer)) {
        tgtAssert(false, "Object of template type expected!");
        LERRORC("voreen.Observable<T>", "addObserver() : Object of type T expected");
        return false;
    }

    return ObservableBase::addObserver(observer);
}

template<class T>
bool Observable<T>::removeObserver(Observer* observer) {
    return ObservableBase::removeObserver(observer);
}

template<class T>
bool Observable<T>::isObservedBy(Observer* observer) const {
    return (observers_.find(observer) != observers_.end());
}

template<class T>
const std::vector<T*> Observable<T>::getObservers() const {

    std::vector<T*> typedObservers;
    std::set<Observer*>::const_iterator it;
    for (it = observers_.begin(); it != observers_.end(); ++it) {
        // note: the only way to get elements into the observers_ vector is by
        // calling addObserver() that does a type check => static_cast is safe here
        typedObservers.push_back(static_cast<T*>(*it));
    }

    return typedObservers;
}

}  // namespace

#endif
