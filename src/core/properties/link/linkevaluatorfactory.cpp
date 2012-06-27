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

#include "voreen/core/properties/link/linkevaluatorfactory.h"

#include "voreen/core/properties/link/dependencylinkevaluators.h"
#include "voreen/core/properties/link/linkevaluatorboolinvert.h"
#include "voreen/core/properties/link/linkevaluatorid.h"
#include "voreen/core/properties/link/linkevaluatoridnormalized.h"
#include "voreen/core/properties/link/linkevaluatorplotselection.h"

namespace voreen {

LinkEvaluatorFactory* LinkEvaluatorFactory::instance_ = 0;

LinkEvaluatorFactory* LinkEvaluatorFactory::getInstance() {
    if (!instance_)
        instance_ = new LinkEvaluatorFactory();
    return instance_;
}

LinkEvaluatorFactory::LinkEvaluatorFactory() {
    //Generic Id evaluators:
    registerClass(new LinkEvaluatorBoolId());
    registerClass(new LinkEvaluatorIntId());
    registerClass(new LinkEvaluatorFloatId());

    registerClass(new LinkEvaluatorFloatIntId());
    registerClass(new LinkEvaluatorFloatBoolId());
    registerClass(new LinkEvaluatorIntBoolId());

    registerClass(new LinkEvaluatorStringId());

    registerClass(new LinkEvaluatorMat2Id());
    registerClass(new LinkEvaluatorMat3Id());
    registerClass(new LinkEvaluatorMat4Id());

    registerClass(new LinkEvaluatorVec2Id());
    registerClass(new LinkEvaluatorVec3Id());
    registerClass(new LinkEvaluatorVec4Id());

    registerClass(new LinkEvaluatorIVec2Id());
    registerClass(new LinkEvaluatorIVec3Id());
    registerClass(new LinkEvaluatorIVec4Id());

    registerClass(new LinkEvaluatorShaderId());

    registerClass(new LinkEvaluatorVolumeHandleId());

    registerClass(new LinkEvaluatorColorMapId());
    registerClass(new LinkEvaluatorPlotEntitiesId());

    registerClass(new LinkEvaluatorCameraId());

    registerClass(new LinkEvaluatorTransFuncId());

    registerClass(new LinkEvaluatorButtonId());

    //Dependancy links:
    //registerClass(new DependancyLinkEvaluatorBool());
    //registerClass(new DependancyLinkEvaluatorInt());
    //registerClass(new DependancyLinkEvaluatorIVec2());
    //registerClass(new DependancyLinkEvaluatorIVec3());
    //registerClass(new DependancyLinkEvaluatorIVec4());
    //registerClass(new DependancyLinkEvaluatorFileDialog());
    //registerClass(new DependancyLinkEvaluatorString());
    registerClass(new DependencyLinkEvaluatorVolumeHandle());

    //Other:
    //registerClass(new LinkEvaluatorId()); //is only created in create method
    registerClass(new LinkEvaluatorBoolInvert());
    registerClass(new LinkEvaluatorIdNormalized());
    registerClass(new LinkEvaluatorPlotSelection());
}

std::vector<std::string> LinkEvaluatorFactory::listFunctionNames() {
    std::vector<std::string> result;
    for (std::map<std::string, LinkEvaluatorBase*>::const_iterator it = classList_.begin();
        it != classList_.end(); ++it)
    {
        result.push_back(it->second->getClassName());
    }
    sort(result.begin(), result.end());
    return result;
}

const std::string LinkEvaluatorFactory::getTypeString(const std::type_info& type) const {
    for (std::map<std::string, LinkEvaluatorBase*>::const_iterator it = classList_.begin();
        it != classList_.end(); ++it)
    {
        if (type == typeid(*(it->second)))
            return it->first;
    }
    return "";
}

Serializable* LinkEvaluatorFactory::createType(const std::string& typeString) {
    return create(typeString);
}

LinkEvaluatorBase* LinkEvaluatorFactory::create(const std::string& typeString) {
    std::map<std::string, LinkEvaluatorBase*>::iterator it = classList_.find(typeString);
    if (it != classList_.end() && it->second != 0) {
        LinkEvaluatorBase* le = it->second->create();
        tgtAssert(le, "No LinkEvaluator created");
        return le;
    }

    //Temporary solution:
    if(typeString == "LinkEvaluatorId") {
        LDEBUGC("voreen.LinkEvaluatorFactory", "Creating deprecated LinkEvaluatorId (using BoxObject)");
        return new LinkEvaluatorId();
    }

    return 0;
}

bool LinkEvaluatorFactory::arePropertiesLinkable(const Property* p1, const Property* p2, bool bidirectional) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");

    for (std::map<std::string, LinkEvaluatorBase*>::const_iterator it = classList_.begin();
        it != classList_.end(); ++it)
    {
        if(it->second->arePropertiesLinkable(p1, p2)) {
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
    for (std::map<std::string, LinkEvaluatorBase*>::const_iterator it = classList_.begin();
        it != classList_.end(); ++it)
    {
        if(it->second->arePropertiesLinkable(p1, p2))
            compatible.push_back(std::pair<std::string, std::string>(it->first, it->second->name()));
    }
    return compatible;
}

bool LinkEvaluatorFactory::isLinkEvaluatorKnown(const std::string& className) const {
    return (classList_.find(className) != classList_.end());
}


void LinkEvaluatorFactory::registerClass(LinkEvaluatorBase* const newClass) {
    tgtAssert(newClass, "null pointer passed");

    if (!isLinkEvaluatorKnown(newClass->getClassName())) {
        classList_.insert(std::make_pair(newClass->getClassName(), newClass));

        knownClasses_.push_back(newClass->getClassName());
    }
    else {
        LWARNINGC("voreen.LinkEvaluatorFactory", "Link Evaluator class '" << newClass->getClassName() << "' has already been registered. Skipping.");
    }
}

} // namespace
