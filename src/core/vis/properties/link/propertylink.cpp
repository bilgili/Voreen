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

#include "voreen/core/vis/properties/link/propertylink.h"
#include "voreen/core/vis/properties/link/linkevaluatorbase.h"
#include "voreen/core/vis/properties/link/linkevaluatorfactory.h"
#include "voreen/core/vis/properties/link/linkevaluatorid.h"
#include "voreen/core/vis/properties/link/linkevaluatorpython.h"
#include "voreen/core/vis/properties/allproperties.h"
#include "voreen/core/vis/properties/property.h"
#include <vector>
#include <map>
#include <typeinfo>

namespace voreen {

std::vector<Property*> PropertyLink::visitedProperties_;

PropertyLink::PropertyLink(Property* src, Property* dest, LinkEvaluatorBase* linkEvaluator)
        : src_(src)
        , dest_(dest)
        , evaluator_(linkEvaluator)
{
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

bool PropertyLink::testPropertyLink() {
    try {
        ChangeData changeData = ChangeData();
        if (typeid(BoolProperty) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((BoolProperty*)src_)->get()));
            changeData.setNewValue(BoxObject(((BoolProperty*)src_)->get()));
        }
        else if (typeid(ColorProperty) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((ColorProperty*)src_)->get()));
            changeData.setNewValue(BoxObject(((ColorProperty*)src_)->get()));
        }
        else if (typeid(FileDialogProperty) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((FileDialogProperty*)src_)->get()));
            changeData.setNewValue(BoxObject(((FileDialogProperty*)src_)->get()));
        }
        else if (typeid(FloatProperty) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((FloatProperty*)src_)->get()));
            changeData.setNewValue(BoxObject(((FloatProperty*)src_)->get()));
        }
        else if (typeid(IntProperty) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((IntProperty*)src_)->get()));
            changeData.setNewValue(BoxObject(((IntProperty*)src_)->get()));
        }
        else if (typeid(StringProperty) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((StringProperty*)src_)->get()));
            changeData.setNewValue(BoxObject(((StringProperty*)src_)->get()));
        }
        else if (typeid(FloatVec2Property) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((FloatVec2Property*)src_)->get()));
            changeData.setNewValue(BoxObject(((FloatVec2Property*)src_)->get()));
        }
        else if (typeid(FloatVec3Property) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((FloatVec3Property*)src_)->get()));
            changeData.setNewValue(BoxObject(((FloatVec3Property*)src_)->get()));
        }
        else if (typeid(FloatVec4Property) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((FloatVec4Property*)src_)->get()));
            changeData.setNewValue(BoxObject(((FloatVec4Property*)src_)->get()));
        }
        else if (typeid(IntVec2Property) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((IntVec2Property*)src_)->get()));
            changeData.setNewValue(BoxObject(((IntVec2Property*)src_)->get()));
        }
        else if (typeid(IntVec3Property) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((IntVec3Property*)src_)->get()));
            changeData.setNewValue(BoxObject(((IntVec3Property*)src_)->get()));
        }
        else if (typeid(IntVec4Property) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((IntVec4Property*)src_)->get()));
            changeData.setNewValue(BoxObject(((IntVec4Property*)src_)->get()));
        }
        else if (dynamic_cast<OptionPropertyBase*>(src_)) {
            changeData.setOldValue(BoxObject(((OptionPropertyBase*)src_)->get()));
            changeData.setNewValue(BoxObject(((OptionPropertyBase*)src_)->get()));
        }
        else if (typeid(CameraProperty) == typeid(*src_)) {
            changeData.setOldValue(BoxObject(((CameraProperty*)src_)->get()));
            changeData.setNewValue(BoxObject(((CameraProperty*)src_)->get()));
        }
        else if (typeid(TransFuncProperty) == typeid(*src_)) {
            TransFunc* tf = ((TransFuncProperty*)src_)->get();
            TransFuncIntensity* tfi = dynamic_cast<TransFuncIntensity*>(tf);
            if (tfi) {
                changeData.setOldValue(BoxObject(tfi));
                changeData.setNewValue(BoxObject(tfi));
            }
            else {
                throw VoreenException("PropertyLink: TransferFuncProp supports only TransFuncIntensity");
            }
        }
        else {
            throw VoreenException("PropertyLink: Unsupported property type.");
        }
        onChange(changeData);
    }
    catch (...) {
        return false;
    }
    return true;
}

void PropertyLink::onChange(ChangeData& data) {
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
        if (typeid(BoolProperty) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((BoolProperty*)dest_)->get()), src_, dest_);
            ((BoolProperty*)dest_)->set(newValue.getBool());
        }
        else if (typeid(ColorProperty) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((ColorProperty*)dest_)->get()), src_, dest_);
            ((ColorProperty*)dest_)->set(newValue.getVec4());
        }
        else if (typeid(FileDialogProperty) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((FileDialogProperty*)dest_)->get()), src_, dest_);
            ((FileDialogProperty*)dest_)->set(newValue.getString());
        }
        else if (typeid(FloatProperty) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((FloatProperty*)dest_)->get()), src_, dest_);
            ((FloatProperty*)dest_)->set(newValue.getFloat());
        }
        else if (typeid(IntProperty) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((IntProperty*)dest_)->get()), src_, dest_);
            ((IntProperty*)dest_)->set(newValue.getInt());
        }
        else if (typeid(StringProperty) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((StringProperty*)dest_)->get()), src_, dest_);
            ((StringProperty*)dest_)->set(newValue.getString());
        }
        else if (typeid(FloatVec2Property) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((FloatVec2Property*)dest_)->get()), src_, dest_);
            ((FloatVec2Property*)dest_)->set(newValue.getVec2());
        }
        else if (typeid(FloatVec3Property) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((FloatVec3Property*)dest_)->get()), src_, dest_);
            ((FloatVec3Property*)dest_)->set(newValue.getVec3());
        }
        else if (typeid(FloatVec4Property) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((FloatVec4Property*)dest_)->get()), src_, dest_);
            ((FloatVec4Property*)dest_)->set(newValue.getVec4());
        }
        else if (typeid(IntVec2Property) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((IntVec2Property*)dest_)->get()), src_, dest_);
            ((IntVec2Property*)dest_)->set(newValue.getIVec2());
        }
        else if (typeid(IntVec3Property) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((IntVec3Property*)dest_)->get()), src_, dest_);
            ((IntVec3Property*)dest_)->set(newValue.getIVec3());
        }
        else if (typeid(IntVec4Property) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((IntVec4Property*)dest_)->get()), src_, dest_);
            ((IntVec4Property*)dest_)->set(newValue.getIVec4());
        }
        else if (dynamic_cast<OptionPropertyBase*>(dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((OptionPropertyBase*)dest_)->get()), src_, dest_);
            ((OptionPropertyBase*)dest_)->set(newValue.getString());
        }
        else if (typeid(CameraProperty) == typeid(*dest_)) {
            BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(((CameraProperty*)dest_)->get()), src_, dest_);
            ((CameraProperty*)dest_)->set(newValue.getCamera());
            // prevent the box object from deleting the camera object.
            newValue.releaseValue();
        }
        else if (typeid(TransFuncProperty) == typeid(*dest_)) {
            TransFunc* tf = ((TransFuncProperty*)dest_)->get();
            TransFuncIntensity* tfi = dynamic_cast<TransFuncIntensity*>(tf);
            if (tfi) {
                BoxObject newValue = evaluator_->eval(data.getOldValue(), data.getNewValue(), BoxObject(tfi), src_, dest_);
                TransFuncIntensity* newTF = newValue.getTransFunc();
                tfi->updateFrom(*newTF);
                dest_->invalidate();
            } else {
                throw VoreenException("PropertyLink: TransferFuncProp does only support TransFuncIntensity");
            }
        }
        else {
            visitedProperties_.clear();
            throw VoreenException("PropertyLink: Unsupported property type.");
        }
    }
    catch (...) {
        if(isInitiator) {
            visitedProperties_.clear();
        }
        throw;
    }

    if (isInitiator) {
        visitedProperties_.clear();
    }
}

void PropertyLink::setLinkEvaluator(LinkEvaluatorBase* evaluator) {
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
    if (!src_ || !dest_)
        s.raise(XmlSerializationMemoryAllocationException("Since either the source or the destination property is missing, property link allocation is impossible."));

    src_->registerLink(this);

    // Deserialize link evaluator...
    s.deserialize("Evaluator", evaluator_);

#ifdef VRN_WITH_PYTHON
    LinkEvaluatorPython* pythonEvaluator = dynamic_cast<LinkEvaluatorPython*>(evaluator_);
    // Is a python evaluator?
    if (pythonEvaluator) {
        // Register python evaluator in factory...
        LinkEvaluatorFactory::getInstance()->registerLinkEvaluatorPython(pythonEvaluator);
        // Get the python evaluator instance which was created during registration...
        evaluator_ = LinkEvaluatorFactory::getInstance()->createLinkEvaluator(pythonEvaluator->getFunctionName());
        // Free allocated memory for our dummy python evaluator...
        delete pythonEvaluator;
    }
#endif // VRN_WITH_PYTHON
}

} // namespace
