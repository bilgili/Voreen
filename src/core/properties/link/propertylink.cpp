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

#include "voreen/core/properties/link/propertylink.h"

#include "voreen/core/properties/link/linkevaluatorbase.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "voreen/core/properties/link/linkevaluatorid.h"
#include "voreen/core/properties/allproperties.h"
#include "voreen/core/properties/property.h"
#include "voreen/core/datastructures/transfunc/transfuncintensity.h"
#include <vector>
#include <map>
#include <typeinfo>

namespace voreen {

const std::string PropertyLink::loggerCat_("voreen.PropertyLink");

std::vector<Property*> PropertyLink::visitedProperties_;

PropertyLink::PropertyLink(Property* src, Property* dest, LinkEvaluatorBase* linkEvaluator)
    : src_(src)
    , dest_(dest)
    , evaluator_(linkEvaluator)
{
    tgtAssert(src_, "No source property");
    tgtAssert(dest_, "No destination property");

    if (linkEvaluator)
        evaluator_ = linkEvaluator;
    else {
        std::vector<std::pair<std::string, std::string> > availableFunctions = LinkEvaluatorFactory::getInstance()->getCompatibleLinkEvaluators(src, dest);
        std::string evalType;
        for(std::vector<std::pair<std::string, std::string> >::iterator i=availableFunctions.begin(); i!=availableFunctions.end(); i++) {
            if(evalType == "")
                evalType = i->first;
            else {
                if(i->second == "id")
                    evalType = i->first;
            }
        }
        evaluator_ = LinkEvaluatorFactory::getInstance()->create(evalType);
        //evaluator_ = LinkEvaluatorFactory::getInstance()->create("LinkEvaluatorId");
    }

    evaluator_->propertiesChanged(src_, dest_);
}

PropertyLink::PropertyLink()
    : src_(0)
    , dest_(0)
    , evaluator_(0)
{}

PropertyLink::~PropertyLink() {
    if (src_)
        src_->removeLink(this);
}

/*
 * Call to test if the link will work to find out about failures in an early state.
 */
bool PropertyLink::testPropertyLink() {
    tgtAssert(src_, "No source property");
    tgtAssert(dest_, "No destination property");

    try {
        onChange();
    }
    catch (...) {
        return false;
    }
    return true;
}


void PropertyLink::onChange()
    throw (VoreenException) {

    tgtAssert(src_, "No source property");
    tgtAssert(dest_, "No destination property");

    // links without evaluator must not exist, but do a graceful check
    if (!evaluator_) {
        tgtAssert(false, "PropertyLink::onChange(): no link evaluator");
        LERROR("onChange(): no link evaluator" << " (src='" + src_->getFullyQualifiedID()
            << "', dest='" << dest_->getFullyQualifiedID() << "')");
        return;
    }

    bool isInitiator = visitedProperties_.empty();
    if (isInitiator) {
        visitedProperties_.push_back(src_);
    }
    else if (std::find(visitedProperties_.begin(), visitedProperties_.end(), dest_) != visitedProperties_.end()) {
        return;
    }
    else {
        visitedProperties_.push_back(dest_);
    }

    try {
        evaluator_->eval(src_, dest_);
    }
    catch (const VoreenException& e) {
        if (isInitiator) {
            visitedProperties_.clear();
        }

        throw VoreenException(std::string(e.what()) +
            " (src='" + src_->getFullyQualifiedID() +
            "', dest='" + dest_->getFullyQualifiedID() + "')");
    }

    if (isInitiator) {
        visitedProperties_.clear();
    }
}

void PropertyLink::setLinkEvaluator(LinkEvaluatorBase* evaluator) {
    tgtAssert(evaluator, "Null pointer passed");
    evaluator_ = evaluator;

    if(evaluator_) {
        evaluator_->propertiesChanged(src_, dest_);
    }
}

LinkEvaluatorBase* PropertyLink::getLinkEvaluator() const {
    return evaluator_;
}

Property* PropertyLink::getSourceProperty() const {
    return src_;
}

Property* PropertyLink::getDestinationProperty() const {
    return dest_;
}

void PropertyLink::serialize(XmlSerializer& s) const {
    // Serialize source property reference...
    s.serialize("SourceProperty", src_);

    // Serialize destination property reference...
    s.serialize("DestinationProperty", dest_);

    // Serialize link evaluator...
    s.serialize("Evaluator", evaluator_);
}

void PropertyLink::deserialize(XmlDeserializer& s) {
    // Deserialize source property reference...
    s.deserialize("SourceProperty", src_);

    // Deserialize destination property reference...
    s.deserialize("DestinationProperty", dest_);

    // Was either the source or the destination property not deserialized?
    if (!src_ || !dest_) {
        std::string addOn;
        if (src_) {
            addOn = "Link source: '";
            if (src_->getOwner())
                addOn += src_->getOwner()->getName() + "::";
            addOn += src_->getGuiName() + "'";
        }
        else if (dest_) {
            addOn = "Link dest: '";
            if (dest_->getOwner())
                addOn += dest_->getOwner()->getName() + "::";
            addOn += dest_->getGuiName() + "'";
        }
        s.raise(XmlSerializationMemoryAllocationException("Property link could not be established. " + addOn));
    }

    src_->registerLink(this);

    // Deserialize link evaluator...
    s.deserialize("Evaluator", evaluator_);

    if(evaluator_) {
        // auto-convert old LinkEvaluatorId:
        if(evaluator_->getClassName() == "LinkEvaluatorId") {
            std::vector<std::pair<std::string, std::string> > availableFunctions = LinkEvaluatorFactory::getInstance()->getCompatibleLinkEvaluators(src_, dest_);
            std::string evalType = "";
            for(std::vector<std::pair<std::string, std::string> >::iterator i=availableFunctions.begin(); i!=availableFunctions.end(); i++) {
                if(i->second == "id")
                    evalType = i->first;
            }
            if(!evalType.empty()) {
                //delete evaluator_;
                evaluator_ = LinkEvaluatorFactory::getInstance()->create(evalType);
                LINFO("Replaced deprecated link evaluator with " << evaluator_->getClassName());
            }
            else {
                LERROR("Could not find and alternative for old LinkEvaluatorId between " << src_->getTypeString() << " and " << dest_->getTypeString());
            }
        }
        // --------------------------------

        evaluator_->propertiesChanged(src_, dest_);
    }
}


} // namespace
