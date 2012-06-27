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

#ifndef VRN_LINKEVALUATORFACTORY_H
#define VRN_LINKEVALUATORFACTORY_H

#include "voreen/core/io/serialization/serialization.h"

#include "voreen/core/properties/link/linkevaluatorbase.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "voreen/core/properties/link/linkevaluatorpython.h"
#include "voreen/core/properties/link/scriptmanagerlinking.h"

#include <map>
#include <string>

namespace voreen {

class LinkEvaluatorFactory : public SerializableFactory {
public:
    /*
     * Gets the singleton instance of the factory
     */
    static LinkEvaluatorFactory* getInstance();

    /*
     * Returns a LinkEvaluator instance identified by the
     * provided function name or throws an Exception
     */
    LinkEvaluatorBase* createLinkEvaluator(std::string functionName);

    /*
     * Get the function name that identifies the provided link evaluator instance
     */
    std::string getFunctionName(LinkEvaluatorBase* linkEvaluator);

    /*
     * Returns the complete list of functions registered with the factory including python scripts
     */
    std::vector<std::string> listFunctionNames();

    /*
     * Register and create a new LinkEvaluator by the given parameters
     */
    void registerLinkEvaluatorPython(std::string functionName, std::string script, bool overwrite=false);
    /*
     * Register a previously created link evaluator instance
     */
    void registerLinkEvaluatorPython(LinkEvaluatorPython* linkEvaluator);

    /*
     * Set the script manager to use
     */
    void setScriptManager(ScriptManagerLinking* scriptManagerLinking);

    /*
     * Get the script manager
     */
    ScriptManagerLinking* getScriptManager();

    /**
     * @see SerializableFactory::getTypeString
     */
    virtual const std::string getTypeString(const std::type_info& type) const;

    /**
     * @see SerializableFactory::createType
     */
    virtual Serializable* createType(const std::string& typeString);

private:
    LinkEvaluatorFactory();
    static LinkEvaluatorFactory* instance_;
    ScriptManagerLinking* scriptManagerLinking_;
};

} // namespace

#endif // VRN_LINKEVALUATORFACTORY_H
