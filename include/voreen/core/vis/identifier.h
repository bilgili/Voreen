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

#ifndef VRN_IDENTIFIER_H
#define VRN_IDENTIFIER_H

#include <string>
#include <iostream>

namespace voreen {

/**
  * Stores an Identifier. Identifier are unique strings, that are used for example in the
  * messaging system.
  */
class Identifier {
public:
    Identifier();
    /**
     * Constructs an Identifier with a std::string.
     */
    Identifier(const std::string& name);
    /**
     * Constructs an Identifier with a char*.
     */
    Identifier(const char* str);
    /**
     * Constructs an Identifier with an Identifier.
     */
    Identifier(const Identifier& ident);
    /**
     * Compares two Identifier.
     */
    friend bool operator==(const Identifier &x, const Identifier &y);
    friend bool operator!=(const Identifier &x, const Identifier &y);
    friend bool operator <(const Identifier& i1, const Identifier& i2);

    friend std::ostream& operator<<(std::ostream& os, const Identifier& i);

    const std::string& getName() const;
    /// returns substrings of strings divided by "."
    const std::string getSubString(int index) const;

    operator std::string() const { return name_; }

protected:
    std::string name_;
};

inline Identifier::Identifier()
    : name_("")
{}

inline Identifier::Identifier(const Identifier& ident)
    : name_(ident.name_)
{}

inline Identifier::Identifier(const char* str)
    : name_(str)
{}

inline Identifier::Identifier(const std::string& name)
    : name_(name)
{}

inline bool operator==(const Identifier &x, const Identifier &y) {
    return x.name_.compare(y.name_) == 0;
}

inline bool operator!=(const Identifier &x, const Identifier &y) {
    return !(x == y);
}

inline bool operator <(const Identifier& i1, const Identifier& i2) {
    return i1.name_ < i2.name_;
}

inline std::ostream& operator<<(std::ostream& os, const Identifier& i) {
    return os << i.name_;
}

} // namespace voreen

#endif
