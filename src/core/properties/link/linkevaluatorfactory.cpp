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

#include "voreen/core/properties/link/linkevaluatorfactory.h"

namespace voreen {

std::vector<std::string> LinkEvaluatorFactory::listFunctionNames() const {
    std::vector<std::string> result;
    std::map<std::string, const LinkEvaluatorBase*>::const_iterator it;
    for (it = typeMap_.begin(); it != typeMap_.end(); ++it) {
        result.push_back(it->second->getClassName());
    }
    sort(result.begin(), result.end());
    return result;
}

LinkEvaluatorBase* LinkEvaluatorFactory::createEvaluator(const std::string& typeString) {
    return dynamic_cast<LinkEvaluatorBase*>(createType(typeString));
}

bool LinkEvaluatorFactory::arePropertiesLinkable(const Property* p1, const Property* p2, bool bidirectional) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");

    std::map<std::string, const LinkEvaluatorBase*>::const_iterator it;
    for (it = typeMap_.begin(); it != typeMap_.end(); ++it) {
        if (it->second->arePropertiesLinkable(p1, p2)) {
            if(!bidirectional)
                return true;
            else
                return it->second->arePropertiesLinkable(p2, p1);
        }
    }
    return false;
}

std::vector<std::pair<std::string, std::string> > LinkEvaluatorFactory::getCompatibleLinkEvaluators(const Property* p1, const Property* p2) const {
    std::vector<std::pair<std::string, std::string> > compatible;
    std::map<std::string, const LinkEvaluatorBase*>::const_iterator it;
    for (it = typeMap_.begin(); it != typeMap_.end(); ++it) {
        if (it->second->arePropertiesLinkable(p1, p2))
            compatible.push_back(std::pair<std::string, std::string>(it->first, it->second->getGuiName()));
    }
    return compatible;
}

} // namespace
