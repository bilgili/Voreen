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

#ifndef VRN_SINGLECOMMAND_H
#define VRN_SINGLECOMMAND_H

#include "voreen/core/cmdparser/command.h"

namespace voreen {

/**
 * This class represents a command which can occur only once in a given commandline and can have
 * 1-4 arguments of types T, U, V and U.
 * This class simply tries to convert the parameters to the appropriate values and stores them in it
 * The template must be usable within a stringstream (necessary for conversion)
 * \sa SingleCommandZeroArguments
 */
template<class T, class U = T, class V = U, class W = V>
class SingleCommand : public Command {
public:
    SingleCommand(T* ptr1,
        const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string parameterList = "")
        : Command(name, shortName, infoText, parameterList, 1, false)
        , ptr1_(ptr1)
        , ptr2_(0)
        , ptr3_(0)
        , ptr4_(0)
    {}

    SingleCommand(T* ptr1, U* ptr2,
        const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string parameterList = "")
        : Command(name, shortName, infoText, parameterList, 2, false)
        , ptr1_(ptr1)
        , ptr2_(ptr2)
        , ptr3_(0)
        , ptr4_(0)
    {}

    SingleCommand(T* ptr1, U* ptr2, V* ptr3,
        const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string parameterList = "")
        : Command(name, shortName, infoText, parameterList, 3, false)
        , ptr1_(ptr1)
        , ptr2_(ptr2)
        , ptr3_(ptr3)
        , ptr4_(0)
    {}

    SingleCommand(T* ptr1, U* ptr2, V* ptr3, W* ptr4,
        const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string parameterList = "")
        : Command(name, shortName, infoText, parameterList, 4, false)
        , ptr1_(ptr1)
        , ptr2_(ptr2)
        , ptr3_(ptr3)
        , ptr4_(ptr4)
    {}

    bool execute(const std::vector<std::string>& parameters) {
        *ptr1_ = cast<T>(parameters[0]);
        if (ptr2_ != 0)
            *ptr2_ = cast<U>(parameters[1]);
        if (ptr3_ != 0)
            *ptr3_ = cast<V>(parameters[2]);
        if (ptr4_ != 0)
            *ptr4_ = cast<W>(parameters[3]);

        return true;
    }

    bool checkParameters(const std::vector<std::string>& parameters) {
        bool result = parameters.size() == static_cast<size_t>(argumentNum_);

        result &= is<T>(parameters[0]);
        if (ptr2_ != 0)
            result &= is<U>(parameters[1]);
        if (ptr3_ != 0)
            result &= is<V>(parameters[2]);
        if (ptr4_ != 0)
            result &= is<W>(parameters[3]);

        return result;
    }

protected:
    T* ptr1_;
    U* ptr2_;
    V* ptr3_;
    W* ptr4_;
};


/**
 * This class represents a command with zero arguments which can only occur once in a given commandline.
 * The pointer will be set to "true", if the command is executed
 * The template must be usable within a stringstream (necessary for conversion)
 * \sa SingleCommand
 */
class SingleCommandZeroArguments : public Command {
public:
    SingleCommandZeroArguments(bool* ptr, const std::string& name, const std::string& shortName = "",
        const std::string& infoText = "")
        : Command(name, shortName, infoText, "", 0, false)
        , ptr_(ptr)
    {}

    bool execute(const std::vector<std::string>& /*parameters*/) {
        *ptr_ = true;
        return true;
    }

protected:
    bool* ptr_;
};

} // namespace

#endif // VRN_SINGLECOMMAND_H
