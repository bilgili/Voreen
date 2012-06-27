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

#include "voreen/core/properties/link/dependencylinkevaluator.h"
#include "voreen/core/properties/link/linkevaluatorbase.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "voreen/core/properties/link/linkevaluatorid.h"
#include "voreen/core/properties/link/linkevaluatorpython.h"
#include "voreen/core/properties/allproperties.h"
#include "voreen/core/properties/property.h"
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
    else
        evaluator_ = LinkEvaluatorFactory::getInstance()->createLinkEvaluator("id");
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
        ChangeData changeData = ChangeData();
        if (BoolProperty* srcCast = dynamic_cast<BoolProperty*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (FileDialogProperty* srcCast = dynamic_cast<FileDialogProperty*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (FloatProperty* srcCast = dynamic_cast<FloatProperty*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (IntProperty* srcCast = dynamic_cast<IntProperty*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (StringProperty* srcCast = dynamic_cast<StringProperty*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (FloatVec2Property* srcCast = dynamic_cast<FloatVec2Property*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (FloatVec3Property* srcCast = dynamic_cast<FloatVec3Property*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (FloatVec4Property* srcCast = dynamic_cast<FloatVec4Property*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (FloatMat2Property* srcCast = dynamic_cast<FloatMat2Property*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (FloatMat3Property* srcCast = dynamic_cast<FloatMat3Property*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (FloatMat4Property* srcCast = dynamic_cast<FloatMat4Property*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (IntVec2Property* srcCast = dynamic_cast<IntVec2Property*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (IntVec3Property* srcCast = dynamic_cast<IntVec3Property*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (IntVec4Property* srcCast = dynamic_cast<IntVec4Property*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (CameraProperty* srcCast = dynamic_cast<CameraProperty*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (TransFuncProperty* srcCast = dynamic_cast<TransFuncProperty*>(src_)) {
            TransFunc* tf = srcCast->get();
            TransFuncIntensity* tfi = dynamic_cast<TransFuncIntensity*>(tf);
            if (tfi) {
                changeData.setOldValue(BoxObject(tfi));
                changeData.setNewValue(BoxObject(tfi));
            }
            else {
                throw VoreenException("PropertyLink: TransferFuncProperty supports only TransFuncIntensity");
            }
        }
        else if (OptionPropertyBase* srcCast = dynamic_cast<OptionPropertyBase*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (ShaderProperty* srcCast = dynamic_cast<ShaderProperty*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (VolumeHandleProperty* srcCast = dynamic_cast<VolumeHandleProperty*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (/*VolumeCollectionProperty* srcCast = */dynamic_cast<VolumeCollectionProperty*>(src_)) {
            throw VoreenException("PropertyLink execution failed: Linking of VolumeCollections is currently not supported");
        }
        else if (dynamic_cast<ButtonProperty*>(src_)) {
        }

        else if (PlotPredicateProperty* srcCast = dynamic_cast<PlotPredicateProperty*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }
        else if (PlotSelectionProperty* srcCast = dynamic_cast<PlotSelectionProperty*>(src_)) {
            changeData.setOldValue(BoxObject(srcCast->get()));
            changeData.setNewValue(BoxObject(srcCast->get()));
        }

        else {
            throw VoreenException("PropertyLink: Unsupported property type (" + src_->getFullyQualifiedID() + ")");
        }
        onChange(changeData);
    }
    catch (...) {
        return false;
    }
    return true;
}


void PropertyLink::onChange(ChangeData& data)
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
        if (BoolProperty* destCast = dynamic_cast<BoolProperty*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getBool());
        }
        else if (FileDialogProperty* destCast = dynamic_cast<FileDialogProperty*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getString());
        }
        else if (FloatProperty* destCast = dynamic_cast<FloatProperty*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getFloat());
        }
        else if (IntProperty* destCast = dynamic_cast<IntProperty*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getInt());
        }
        else if (StringProperty* destCast = dynamic_cast<StringProperty*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getString());
        }
        else if (FloatVec2Property* destCast = dynamic_cast<FloatVec2Property*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getVec2());
        }
        else if (FloatVec3Property* destCast = dynamic_cast<FloatVec3Property*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getVec3());
        }
        else if (FloatVec4Property* destCast = dynamic_cast<FloatVec4Property*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getVec4());
        }
        else if (FloatMat2Property* destCast = dynamic_cast<FloatMat2Property*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getMat2());
        }
        else if (FloatMat3Property* destCast = dynamic_cast<FloatMat3Property*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getMat3());
        }
        else if (FloatMat4Property* destCast = dynamic_cast<FloatMat4Property*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getMat4());
        }
        else if (IntVec2Property* destCast = dynamic_cast<IntVec2Property*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getIVec2());
        }
        else if (IntVec3Property* destCast = dynamic_cast<IntVec3Property*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getIVec3());
        }
        else if (IntVec4Property* destCast = dynamic_cast<IntVec4Property*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getIVec4());
        }
        else if (CameraProperty* destCast = dynamic_cast<CameraProperty*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(*newValue.getCamera());
        }
        else if (TransFuncProperty* destCast = dynamic_cast<TransFuncProperty*>(dest_)) {
            TransFunc* tf = destCast->get();
            TransFuncIntensity* tfi = dynamic_cast<TransFuncIntensity*>(tf);
            if (tfi) {
                const BoxObject& oldSrcValue = data.getOldValue();
                const BoxObject& newSrcValue = data.getNewValue();
                BoxObject targetOld = BoxObject(tfi);
                BoxObject newValue = evaluator_->eval(oldSrcValue, newSrcValue, targetOld, src_, dest_);
                const TransFuncIntensity* newTF = dynamic_cast<const TransFuncIntensity*>(newValue.getTransFunc());
                if (newTF) {
                    tfi->updateFrom(*newTF);
                    destCast->notifyChange();
                }
                else {
                    throw VoreenException("PropertyLink execution failed: TransFuncIntensity expected as return type");
                }
            }
            else {
                throw VoreenException("PropertyLink execution failed: Transfer function linking currently only supported for TransFuncIntensity");
            }
        }
        else if (OptionPropertyBase* destCast = dynamic_cast<OptionPropertyBase*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getString());
        }
        else if (ShaderProperty* destCast = dynamic_cast<ShaderProperty*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getShader());
            destCast->invalidate();
        }
        else if (ButtonProperty* destCast = dynamic_cast<ButtonProperty*>(dest_) ) {
            destCast->clicked();
        }
        else if (VolumeHandleProperty* destCast = dynamic_cast<VolumeHandleProperty*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(const_cast<VolumeHandle*>(newValue.getVolumeHandle()));
        }
        else if (dynamic_cast<VolumeCollectionProperty*>(dest_)) {
            throw VoreenException("PropertyLink execution failed: Linking of VolumeCollections not supported");
        }
        else if (PlotPredicateProperty* destCast = dynamic_cast<PlotPredicateProperty*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getPlotPredicateVector());
        }
        else if (PlotSelectionProperty* destCast = dynamic_cast<PlotSelectionProperty*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(destCast->get()), src_, dest_);
            destCast->set(newValue.getPlotZoom());
        }
        else {
            visitedProperties_.clear();
            throw VoreenException("PropertyLink execution failed: Unsupported property type.");
        }
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

#ifdef VRN_WITH_PYTHON
    // Is a python evaluator?
    if (typeid(LinkEvaluatorPython*) == typeid(evaluator_)) {
        LinkEvaluatorPython* pythonEvaluator = dynamic_cast<LinkEvaluatorPython*>(evaluator_);
        // Register python evaluator in factory...
        LinkEvaluatorFactory::getInstance()->registerLinkEvaluatorPython(pythonEvaluator);
        // Get the python evaluator instance which was created during registration...
        evaluator_ = LinkEvaluatorFactory::getInstance()->createLinkEvaluator(pythonEvaluator->getFunctionName());
        // Free allocated memory for our dummy python evaluator...
        delete pythonEvaluator;
    }
#endif // VRN_WITH_PYTHON
}

bool PropertyLink::arePropertiesLinkable(const Property* p1, const Property* p2) {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");

    if (typeid(*p1) == typeid(*p2))
        return true;

    if (dynamic_cast<const BoolProperty*>(p1) || dynamic_cast<const FloatProperty*>(p1) ||
        dynamic_cast<const IntProperty*>(p1)  || dynamic_cast<const StringProperty*>(p1) )
    {
        if (dynamic_cast<const BoolProperty*>(p2))
            return true;
        else if (dynamic_cast<const IntProperty*>(p2))
            return true;
        else if (dynamic_cast<const FloatProperty*>(p2))
            return true;
        else if (dynamic_cast<const StringProperty*>(p2))
            return true;
        else
            return false;
    }
    else if (dynamic_cast<const IntVec2Property*>(p1)   || dynamic_cast<const IntVec3Property*>(p1)   || dynamic_cast<const IntVec4Property*>(p1)   ||
             dynamic_cast<const FloatVec2Property*>(p1) || dynamic_cast<const FloatVec3Property*>(p1) || dynamic_cast<const FloatVec4Property*>(p1) )
    {
        if (dynamic_cast<const IntVec2Property*>(p2))
            return true;
        else if (dynamic_cast<const IntVec3Property*>(p2))
            return true;
        else if (dynamic_cast<const IntVec4Property*>(p2))
            return true;
        else if (dynamic_cast<const FloatVec2Property*>(p2))
            return true;
        else if (dynamic_cast<const FloatVec3Property*>(p2))
            return true;
        else if (dynamic_cast<const FloatVec4Property*>(p2))
            return true;
        else
            return false;
    }

    return false;
}

} // namespace
