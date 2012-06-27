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

#ifdef VRN_WITH_PYTHON
// Python header must be included before other system headers
#include <Python.h>
#endif

#include "voreen/core/vis/properties/link/linkevaluatorfactory.h"
#include "voreen/core/vis/properties/link/linkevaluatorboolinvert.h"
#include "voreen/core/vis/properties/link/linkevaluatorid.h"
#include "voreen/core/vis/properties/link/linkevaluatoridnormalized.h"
#include "voreen/core/vis/properties/link/boxobject.h"
#include "voreen/core/vis/properties/allproperties.h"
#include "voreen/core/vis/properties/property.h"
#include "voreen/core/vis/properties/link/linkevaluatorpython.h"

namespace voreen {

LinkEvaluatorFactory* LinkEvaluatorFactory::instance_ = 0;

LinkEvaluatorFactory* LinkEvaluatorFactory::getInstance() {
    if (!instance_) {
        instance_ = new LinkEvaluatorFactory();
        instance_->functionNameList_.push_back("boolInvert");
        instance_->linkEvaluatorCache_.insert(std::pair<std::string, LinkEvaluatorBase*>("boolInvert", new LinkEvaluatorBoolInvert()));
        instance_->functionNameList_.push_back("id");
        instance_->linkEvaluatorCache_.insert(std::pair<std::string, LinkEvaluatorBase*>("id", new LinkEvaluatorId()));
        instance_->functionNameList_.push_back("idNormalized");
        instance_->linkEvaluatorCache_.insert(std::pair<std::string, LinkEvaluatorBase*>("idNormalized", new LinkEvaluatorIdNormalized()));
    }

    return instance_;
}

LinkEvaluatorFactory::LinkEvaluatorFactory() {
}

LinkEvaluatorBase* LinkEvaluatorFactory::createLinkEvaluator(std::string functionName) {
    if (linkEvaluatorCache_.find(functionName) == linkEvaluatorCache_.end()) {
        throw VoreenException("FunctionName does not exist. Please register before usage.");
    }
    return linkEvaluatorCache_.find(functionName)->second;
}

std::string LinkEvaluatorFactory::getFunctionName(LinkEvaluatorBase* linkEvaluator) {
    std::map<std::string, LinkEvaluatorBase*>::iterator it = linkEvaluatorCache_.begin();
    while (it != linkEvaluatorCache_.end()) {
        if (it->second == linkEvaluator) {
            return it->first;
        }
        it++;
    }
    return "";
}

std::list<std::string> LinkEvaluatorFactory::listFunctionNames() {
    return functionNameList_;
}

void LinkEvaluatorFactory::registerLinkEvaluatorPython(std::string functionName, std::string script, bool overwrite) {
    if (linkEvaluatorCache_.find(functionName) == linkEvaluatorCache_.end()) {
        functionNameList_.push_back(functionName);
        functionNameList_.sort();
        LinkEvaluatorBase* lep = new LinkEvaluatorPython(functionName, script);
        linkEvaluatorCache_.insert(std::pair<std::string, LinkEvaluatorBase*>(functionName, lep));
    }
    else if (overwrite) {
        LinkEvaluatorBase* base = linkEvaluatorCache_.find(functionName)->second;
        if (typeid(*base) == typeid(LinkEvaluatorPython)) {
            ((LinkEvaluatorPython*)base)->setScript(script);
        } else {
            throw VoreenException("Could not replace script. Not a python LinkEvaluator.");
        }
    }
}

void LinkEvaluatorFactory::registerLinkEvaluatorPython(LinkEvaluatorPython* linkEvaluator) {
    registerLinkEvaluatorPython(linkEvaluator->getFunctionName(), linkEvaluator->getScript(), true);
}

const std::string LinkEvaluatorFactory::getTypeString(const std::type_info& type) const {
    if (type == typeid(LinkEvaluatorId))
        return "LinkEvaluatorId";
    else if (type == typeid(LinkEvaluatorBoolInvert))
        return "LinkEvaluatorBoolInvert";
    else if (type == typeid(LinkEvaluatorIdNormalized))
        return "LinkEvaluatorIdNormalized";
    else if (type == typeid(LinkEvaluatorPython))
        return "LinkEvaluatorPython";
    else
        return "";
}

Serializable* LinkEvaluatorFactory::createType(const std::string& typeString) {
    if (typeString == "LinkEvaluatorId")
        return createLinkEvaluator("id");
    else if (typeString == "LinkEvaluatorBoolInvert")
        return createLinkEvaluator("boolInvert");
    else if (typeString == "LinkEvaluatorIdNormalized")
        return createLinkEvaluator("idNormalized");
    else if (typeString == "LinkEvaluatorPython")
        return new LinkEvaluatorPython();
    else
        return 0;
}

} // namespace
