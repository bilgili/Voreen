/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#ifndef VRN_INTERPOLATIONFUNCTIONFACTORY_H
#define VRN_INTERPOLATIONFUNCTIONFACTORY_H

#include <vector>
#include <string>
#include "voreen/core/animation/interpolationfunction.h"
#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

template <class T> class InterpolationFunction;

/**
 * Manages all possible interpolation functions for a given InterpolationFunction<T>
 */
template <class T> class InterpolationFunctionFactory : public SerializableFactory {
public:
    /**
     * Constructor.
     */
    InterpolationFunctionFactory();

    /**
     * Destructor.
     */
    ~InterpolationFunctionFactory();

    /**
     * Returns a vector of names of all possible interpolation functions for this type of property.
     */
    const std::vector<std::string> getListOfFunctionNames() const;

    /**
     * Returns a vector of possible interpolation functions for this type of property.
     */
    const std::vector<InterpolationFunction<T>*>& getListOfFunctions() const;

    /**
     * Returns a new instance of an interpolation function of the given name.
     * If there is no such function, the the default function is returned.
     */
    InterpolationFunction<T>* getFunctionByName(const std::string& name) const;

    /**
     * @see SerializableFactory::getTypeString
     */
    virtual const std::string getTypeString(const std::type_info& type) const;

    /**
     * @see SerializableFactory::createType
     */
    virtual Serializable* createType(const std::string& typeString);

    static InterpolationFunctionFactory<T>* getInstance();

private:
    /**
     * Registers a new interpolation function to this factory.
     * If the property type is consistent, the given function will be added to the internal list, otherwise it will be deleted.
     */
    void registerFunction(InterpolationFunctionBase* func);

    std::vector<InterpolationFunction<T>*> functions_;

    static InterpolationFunctionFactory<T>* instance_;
};

} // namespace voreen

#endif
