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

#ifndef TGT_SINGLETON_H
#define TGT_SINGLETON_H

#include <iostream>

#include "tgt/assert.h"
#include "tgt/types.h"

namespace tgt {

/**
    This class helps to build the singleton design pattern.
    Here you have full control over construction and deconstruction
    of the object.
*/
template<class T>
class Singleton {
public:
    /**
     * Init the actual singleton.
     * Must be called BEFORE the class is used, like this:
     */
    static void init() {
        tgtAssert( !singletonClass_, "singletonClass_ has already been initialized." );
        singletonClass_ = new T;
    }

    /**
     * Deinit the actual singleton.
     * Must be done at last.
     */
    static void deinit() {
        tgtAssert( singletonClass_ != 0, "singletonClass_ has already been deinitialized." );
        delete singletonClass_;
        singletonClass_ = 0;
    }

    /**
     * Get Pointer of the actual class
     * @return Pointer of the actual class
     */
    static T* getPtr() {
        tgtAssert( singletonClass_ != 0, "singletonClass_ has not been intitialized." );
        return singletonClass_;
    }

    /**
     * Get reference of the actual class
     * @return reference of the actual class
    */
    static T& getRef() {
        tgtAssert( singletonClass_ != 0 , "singletonClass_ has not been intitialized." );
        return *singletonClass_;
    }

    /**
     * Has the actual class been inited?
     */
    static bool isInited() {
        return (singletonClass_ != 0);
    }

private:
    static T* singletonClass_;
};

/// init static pointers with 0
template<class T>
T* Singleton<T>::singletonClass_ = 0;

} // namespace

#endif // TGT_SINGLETON_H
