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

#ifdef VRN_WITH_PYTHON
// Python header must be included before other system headers
#include <Python.h>
#endif

#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "voreen/core/properties/link/linkevaluatorboolinvert.h"
#include "voreen/core/properties/link/linkevaluatorid.h"
#include "voreen/core/properties/link/linkevaluatoridnormalized.h"
#include "voreen/core/properties/link/boxobject.h"
#include "voreen/core/properties/allproperties.h"
#include "voreen/core/properties/property.h"
#include "voreen/core/properties/link/linkevaluatorpython.h"
#include "voreen/core/properties/link/scriptmanagerlinking.h"

namespace voreen {

LinkEvaluatorFactory* LinkEvaluatorFactory::instance_ = 0;

LinkEvaluatorFactory* LinkEvaluatorFactory::getInstance() {
    if (!instance_)
        instance_ = new LinkEvaluatorFactory();
    return instance_;
}

LinkEvaluatorFactory::LinkEvaluatorFactory() {
}

LinkEvaluatorBase* LinkEvaluatorFactory::createLinkEvaluator(std::string functionName) {
    if(std::strcmp("boolInvert", functionName.c_str()) == 0)
        return new LinkEvaluatorBoolInvert();
    else if(std::strcmp("id", functionName.c_str()) == 0)
        return new LinkEvaluatorId();
    else if(std::strcmp("idNormalized", functionName.c_str()) == 0)
        return new LinkEvaluatorIdNormalized();
    else
        return new LinkEvaluatorPython(functionName);
}

std::string LinkEvaluatorFactory::getFunctionName(LinkEvaluatorBase* linkEvaluator) {
    if(typeid(LinkEvaluatorBoolInvert) == typeid(*linkEvaluator))
        return "boolInvert";
    else if(typeid(LinkEvaluatorId) == typeid(*linkEvaluator))
        return "id";
    else if(typeid(LinkEvaluatorIdNormalized) == typeid(*linkEvaluator))
        return "idNormalized";
    else if(typeid(LinkEvaluatorPython) == typeid(*linkEvaluator))
        return static_cast<LinkEvaluatorPython*>(linkEvaluator)->getFunctionName();
    else
        return "";
}

std::vector<std::string> LinkEvaluatorFactory::listFunctionNames() {
    std::vector<std::string> result(getScriptManager()->listPythonFunctionNames());
    result.push_back("boolInvert");
    result.push_back("id");
    result.push_back("idNormalized");
    sort(result.begin(), result.end());
    return result;
}

void LinkEvaluatorFactory::registerLinkEvaluatorPython(std::string functionName, std::string script, bool overwrite) {
    std::vector<std::string> names = listFunctionNames();
    if(overwrite || !scriptManagerLinking_->existPythonScript(functionName))
        scriptManagerLinking_->setPythonScript(functionName, script);
}

void LinkEvaluatorFactory::registerLinkEvaluatorPython(LinkEvaluatorPython* linkEvaluator) {
    registerLinkEvaluatorPython(linkEvaluator->getFunctionName(), linkEvaluator->getScript(), true);
}

void LinkEvaluatorFactory::setScriptManager(ScriptManagerLinking* scriptManagerLinking) {
    scriptManagerLinking_ = scriptManagerLinking;
}

ScriptManagerLinking* LinkEvaluatorFactory::getScriptManager() {
    return scriptManagerLinking_;
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
        return new LinkEvaluatorId();
    else if (typeString == "LinkEvaluatorBoolInvert")
        return new LinkEvaluatorBoolInvert();
    else if (typeString == "LinkEvaluatorIdNormalized")
        return new LinkEvaluatorIdNormalized();
    else if (typeString == "LinkEvaluatorPython")
        return new LinkEvaluatorPython();
    else
        return 0;
}

} // namespace
