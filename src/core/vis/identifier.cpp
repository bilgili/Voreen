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

#include "voreen/core/vis/identifier.h"
#include <vector>

namespace voreen {

const std::string& Identifier::getName() const {
    return name_;
}

const std::string Identifier::getSubString(int index) const {
    std::vector<int> points;
    std::string subString;
 
    points.push_back(-1);
    for (size_t i = 0; i<name_.length(); ++i) {
        if (name_.substr(i,1) == ".")
            points.push_back(i); 
    }
    points.push_back(name_.length());

	subString = name_;
    if (index >= (static_cast<int>(points.size())-1) )
        return subString; 
    return subString.substr(points.at(index)+1,points.at(index+1) - points.at(index) - 1); 
}

} // namespace voreen
