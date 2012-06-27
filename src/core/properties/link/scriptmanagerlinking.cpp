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

#include "voreen/core/properties/link/scriptmanagerlinking.h"

namespace voreen {

ScriptManagerLinking::ScriptManagerLinking() {
}

std::string ScriptManagerLinking::getPythonScript(std::string functionName) {
    if (pythonFunctionScriptMap_.find(functionName) == pythonFunctionScriptMap_.end()) {
        throw VoreenException("FunctionName does not exist. Please use setPythonScript script before usage.");
    }
    return pythonFunctionScriptMap_.find(functionName)->second;
}

std::vector<std::string> ScriptManagerLinking::listPythonFunctionNames() {
    std::vector<std::string> list;
    std::map<std::string, std::string>::iterator iter;
    for( iter = pythonFunctionScriptMap_.begin(); iter != pythonFunctionScriptMap_.end(); iter++ ) {
        list.push_back(iter->first);
    }
    return list;
}

void ScriptManagerLinking::setPythonScript(std::string functionName, std::string script) {
    if (existPythonScript(functionName))
        pythonFunctionScriptMap_.erase(pythonFunctionScriptMap_.find(functionName));
    pythonFunctionScriptMap_.insert(std::pair<std::string, std::string>(functionName, script));
}

bool ScriptManagerLinking::existPythonScript(std::string functionName) {
    return (pythonFunctionScriptMap_.find(functionName) != pythonFunctionScriptMap_.end());
}

void ScriptManagerLinking::serialize(XmlSerializer& s) const {
    s.serialize("pythonFunctionScriptMap", pythonFunctionScriptMap_);
}

void ScriptManagerLinking::deserialize(XmlDeserializer& s) {
    try {
        s.deserialize("pythonFunctionScriptMap", pythonFunctionScriptMap_);

        std::map<std::string, std::string>::iterator iter;
        for( iter = pythonFunctionScriptMap_.begin(); iter != pythonFunctionScriptMap_.end(); iter++ ) {
            // Forcing unix-style line endings since only these are accepted
            // by the Python interpreter on unix.
            std::string tmp = iter->second;
            std::string filteredScript = "";
            for (size_t i=0; i < tmp.size(); i++)
                if (tmp[i] != '\r')
                    filteredScript += tmp[i];
            iter->second = filteredScript;
        }
    }
    catch (XmlSerializationNoSuchDataException&) {
        // No data found in ScriptManagerLinking element...
        s.removeLastError();
    }
}

} // namespace
