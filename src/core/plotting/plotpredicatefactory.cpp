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

#include "voreen/core/plotting/plotpredicatefactory.h"

#include "voreen/core/plotting/plotpredicate.h"

namespace voreen {

PlotPredicateFactory* PlotPredicateFactory::instance_ = 0;

PlotPredicateFactory::PlotPredicateFactory() {
}

PlotPredicateFactory* PlotPredicateFactory::getInstance() {
    if (!instance_)
        instance_ = new PlotPredicateFactory();

    return instance_;
}

const std::string PlotPredicateFactory::getTypeString(const std::type_info& type) const {
    if (type == typeid(PlotPredicateLess))
        return "PlotPredicateLess";
    else if (type == typeid(PlotPredicateEqual))
        return "PlotPredicateEqual";
    else if (type == typeid(PlotPredicateGreater))
        return "PlotPredicateGreater";
    else if (type == typeid(PlotPredicateContains))
        return "PlotPredicateContains";
    else if (type == typeid(PlotPredicateBetween))
        return "PlotPredicateBetween";
    else if (type == typeid(PlotPredicateEmpty))
        return "PlotPredicateEmpty";
    else if (type == typeid(PlotPredicateNotEmpty))
        return "PlotPredicateNotEmpty";
    else if (type == typeid(PlotPredicateIsSubStr))
        return "PlotPredicateIsSubStr";
    else if (type == typeid(PlotPredicateAlphaNumeric))
        return "PlotPredicateAlphaNumeric";
    else if (type == typeid(PlotPredicateNotAlphaNumeric))
        return "PlotPredicateNotAlphaNumeric";
    else
        return "";
}

Serializable* PlotPredicateFactory::createType(const std::string& typeString) {
    if (typeString == "PlotPredicateLess")
        return new PlotPredicateLess();
    else if (typeString == "PlotPredicateEqual")
        return new PlotPredicateEqual();
    else if (typeString == "PlotPredicateGreater")
        return new PlotPredicateGreater();
    else if (typeString == "PlotPredicateContains")
        return new PlotPredicateContains();
    else if (typeString == "PlotPredicateBetween")
        return new PlotPredicateBetween();
    else if (typeString == "PlotPredicateEmpty")
        return new PlotPredicateEmpty();
    else if (typeString == "PlotPredicateNotEmpty")
        return new PlotPredicateNotEmpty();
    else if (typeString == "PlotPredicateIsSubStr")
        return new PlotPredicateIsSubStr();
    else if (typeString == "PlotPredicateAlphaNumeric")
        return new PlotPredicateAlphaNumeric();
    else if (typeString == "PlotPredicateNotAlphaNumeric")
        return new PlotPredicateNotAlphaNumeric();
    else
        return 0;
}

} // namespace voreen
