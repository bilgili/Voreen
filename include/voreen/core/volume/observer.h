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

#include <set>

namespace voreen {

/*
 * Quite general immplementation of the observer design pattern.
 *
 * @author  Dirk Feldmann, University of Münster
 */
class Observable;

class Observer {

friend class Observable;    // needed to allow calling removeObservedFriend() from there

public:
    Observer();

    /**
     * Dtor which removes this Observer object from all Observable objects which
     * might be spectating. The user does not need to remove this object from
     * any Observable object this object is watching!
     */
    virtual ~Observer();

    /**
     * Adds the passed Observalbel object to those objects which are observed by
     * this one. An object can only be added once and must not be NULL.
     *
     * @param   observed    the object which shall be observed
     * @return "true" if the object has been inserted successfully or "false"
     *         otherwise
     */
    bool addObserved(Observable* const observed);

    /**
     * Returns the local pointer which is held in the list of observed objects for
     * the given one or NULL, if the passed object is not observed by this.
     *
     * @param   observed    the observed object which shall be found
     * @return  pointer to the object from the internal list or NULL if
     *          either observed has been NULL or it is not contained
     */
    Observable* findObserved(Observable* const observed);

    /**
     * Removes the passed Observable object from the list of observed objects if
     * it has been observed.
     *
     * @param   observed    the observed object which shall be removed
     * @return  "true" is return if the passed object was not NULL, "false" otherwise.
     */
    bool removeObserved(Observable* const observed);

    /**
     * Clears the list of all observed objects. Like the dtor, this method
     * removes this object itself from Observable objects.
     */
    void clearObserverds();

    /**
     * Returns the number of observed objects.
     */
    int getNumObserveds() const;

    /**
     * This method is called by the observed object which should inherit from
     * <code>Observable</code>. The action performed of course depends on the
     * implementation of this method.
     *
     * @param   source  Pointer to the object which called this method. This
     *      might be usefull if one observer observes several observable
     *      objects and the action to perform depends on the caller.
     *      Please note, that the parameter is a constant pointer to a constant
     *      object as the task of an observer is to observe, not to manipulate!
     */
    virtual void notify(const Observable* const source = 0) = 0;

protected:
    std::set<Observable*> observeds_;

private:
    /**
     * This method is to be called from Observable::removeObserver(). Unlike the
     * public method removeObserved(), this method does not try to remove the object
     * itself (this) from its role as an observer on the passed Observable pointer.
     * If it did so, the methods removeObserver() and removeObserved() would call each
     * other mutually and hook up in an infinite loop!
     */
    bool removeObservedFriend(Observable* const observed);
};

// ---------------------------------------------------------------------------

class Observable {

friend class Observer;  // needed to allow calling removeObserverFriend() from there

public:
    Observable();

    /**
     * Besides simply destroying this object, the dtor removes this object
     * from the list of all Observer objects observing it. The user therefore
     * does not need to remove this object manually from all its observers.
     */
    virtual ~Observable();

    /**
     * Adds the passed Observer to the list of observing ones. An observer must
     * not be NULL and can only be inserted once.
     *
     * @param   observer    Observer to be inserted
     * @return  "true" if the insertion has been successful and "false" if the
     *          observer was NULL or already contained.
     */
    bool addObserver(Observer* const observer);

    /**
     * Return the internal pointer to the passed Observer if it is within the
     * list of the object observing this or NULL otherwise.
     *
     * @param   observer    the observer to be found
     * @return  pointer to the observer if contained, or NULL otherwise
     */
    Observer* findObserver(Observer* const observer);

    /**
     * Removes the passed Observed object from the list of observers observing
     * this object.
     *
     * @param   observer    the observer to be removed
     * @return  "true" if observer is not NULL, "false" otherwise
     */
    bool removeObserver(Observer* const observer);

    /**
     * Clears the list of all Observer objects and - like the dtor -
     * removes this object itself from the Observers' lists.
     */
    void clearObservers();

    /**
     * Returns the number of observed Observer object.
     */
    int getNumObservers() const;

    /**
     * Notifies all Observer objects held in the internal list by
     * calling <code>notify()</code> on them.
     */
    virtual void notifyObservers() const;

    /**
     * Notifies only Observer object from the list matchint the
     * typeid() of the passed template parameter by calling
     * <code>notify()</code> on them.
     */
    template<class T>
    void notifyCertainObservers() const {
        std::set<Observer*>::const_iterator it = observers_.begin();
        for ( ; it != observers_.end(); ++it) {
            T obj = dynamic_cast<T>(*it);
            if (obj != 0)
                obj->notify(this);
        }
    }

protected:
    std::set<Observer*> observers_;

private:
    /**
     * This method is called from Observer::removeObserved(). Unlike the
     * public method removeObserver(), this method does not try to remove
     * the object itself (this) from its role as an obseved object on the passed
     * Observer* pointer. If it did so, removeObserver() and removeObserved() would
     * call each other mutually and hook up in an infinite loop!
     */
    bool removeObserverFriend(Observer* const observer);
};

}   // namespace

#endif
