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

#ifndef VRN_LINKEVALUATORFACTORY_H
#define VRN_LINKEVALUATORFACTORY_H

#include "voreen/core/io/serialization/serialization.h"

#include "voreen/core/vis/properties/link/linkevaluatorbase.h"
#include "voreen/core/vis/properties/link/linkevaluatorfactory.h"
#include "voreen/core/vis/properties/link/linkevaluatorpython.h"

#include <map>
#include <list>
#include <string>

namespace voreen {

class LinkEvaluatorFactory : public SerializableFactory {
public:
    static LinkEvaluatorFactory* getInstance();

    LinkEvaluatorBase* createLinkEvaluator(std::string functionName);
    std::string getFunctionName(LinkEvaluatorBase* linkEvaluator);
    std::list<std::string> listFunctionNames();

    void registerLinkEvaluatorPython(std::string functionName, std::string script, bool overwrite=false);
    void registerLinkEvaluatorPython(LinkEvaluatorPython* linkEvaluator);

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

    std::list<std::string> functionNameList_;
    std::map<std::string, LinkEvaluatorBase*> linkEvaluatorCache_;
};

} // namespace

#endif // VRN_LINKEVALUATORFACTORY_H
