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

#ifndef VRN_SCRIPTMANAGERLINKING_H
#define VRN_SCRIPTMANAGERLINKING_H

#include "voreen/core/io/serialization/serialization.h"

#include <map>
#include <string>

namespace voreen {

class ScriptManagerLinking : public Serializable {
public:
    ScriptManagerLinking();

    /*
     * Returns a python script identified by the
     * provided function name or throws an Exception
     */
    std::string getPythonScript(std::string functionName);

    /*
     * Returns the complete list of python functions registered with the factory
     */
    std::vector<std::string> listPythonFunctionNames();

    /*
     * Add a python script
     */
    void setPythonScript(std::string functionName, std::string script);

    /*
     * Returns true is functionName  exists
     */
    bool existPythonScript(std::string functionName);

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

private:
    std::map<std::string, std::string> pythonFunctionScriptMap_;
};

} // namespace

#endif // VRN_ScriptManagerLinking_H
