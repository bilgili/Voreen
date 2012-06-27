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
#define VRN_OBSERVER_H

#include <set>

namespace voreen {

/*
 * (Simple) classes for observer design pattern from software engineering.
 */
class Observable;

class Observer {
public:
    virtual ~Observer() {}

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
    virtual void notify(const Observable* const /*source = 0*/) = 0;
};

// ---------------------------------------------------------------------------

class Observable {
public:
    Observable();
    virtual ~Observable();

    bool addObserver(Observer* const observer);
    bool removeObserver(Observer* const observer);
    void clearObservers();
    int getNumObservers() const;
    virtual void notifyObservers() const;

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
};

}   // namespace

#endif
