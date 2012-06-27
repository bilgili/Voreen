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

#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/condition.h"
#include "voreen/core/datastructures/transfunc/transfuncfactory.h"
#include "voreen/core/datastructures/transfunc/transfuncintensity.h"
#include "voreen/core/datastructures/transfunc/transfuncintensitygradient.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"
#include "voreen/core/properties/propertywidgetfactory.h"

#include "voreen/core/datastructures/volume/volumehandle.h"

namespace voreen {

const std::string TransFuncProperty::loggerCat_("voreen.TransFuncProperty");

TransFuncProperty::TransFuncProperty(const std::string& ident, const std::string& guiText, Processor::InvalidationLevel invalidationLevel,
                             TransFuncProperty::Editors editors, bool lazyEditorInstantiation)
    : TemplateProperty<TransFunc*>(ident, guiText, 0, invalidationLevel)
    , volumeHandle_(0)
    , editors_(editors)
    , lazyEditorInstantiation_(lazyEditorInstantiation)
{
}

TransFuncProperty::~TransFuncProperty() {
    if (value_) {
        LWARNING(getFullyQualifiedGuiName() << " has not been deinitialized before destruction.");
    }
}

void TransFuncProperty::enableEditor(TransFuncProperty::Editors editor) {
    editors_ |= editor;
}

void TransFuncProperty::disableEditor(TransFuncProperty::Editors editor) {
    editors_ &= ~editor;
}

bool TransFuncProperty::isEditorEnabled(TransFuncProperty::Editors editor) {
    return (editors_ & editor);
}

void TransFuncProperty::set(TransFunc* tf) {

    //Normally this method is only called when the type of transfer function
    //changes. The plugins are working with a pointer and thus they only call
    //notifyChange() when the transfer function changes.

    // tf object already assigned
    if (tf == value_) {
        return;
    }

    // new tf object assigned -> check if contents are equal
    TransFuncIntensity* tf_int = dynamic_cast<TransFuncIntensity*>(tf);
    TransFuncIntensity* value_int = dynamic_cast<TransFuncIntensity*>(value_);
    TransFuncIntensityGradient* tf_grad = dynamic_cast<TransFuncIntensityGradient*>(tf);
    TransFuncIntensityGradient* value_grad = dynamic_cast<TransFuncIntensityGradient*>(value_);

    // assign new object, but store previous one for deletion
    TransFunc* oldValue = value_;
    value_ = tf;

    // both transfer functions are of type TransFuncIntensity
    if (tf_int && value_int) {
        if (*tf_int == *value_int) {
            // keys are equal -> only inform widgets about new object
            updateWidgets();
        }
        else {
            // the pointer is the same but keys are different -> notify change
            notifyChange();
        }
    }
    // both transfer functions are of type TransFuncIntensityGradient
    else if (tf_grad && value_grad) {
        if (*tf_grad == *value_grad) {
            // primitives are equal ->only inform widgets about new object
            updateWidgets();
        }
        else {
            // primitives are different -> notify change
            notifyChange();
        }
    }
    // last case: tf type has changed -> additionally shaders may have to be recompiled
    else {
        invalidateOwner(Processor::INVALID_PROGRAM);
        notifyChange();
    }

    // finally delete previously assigned object
    delete oldValue;
}

void TransFuncProperty::setVolumeHandle(VolumeHandle* handle) {

    if (volumeHandle_ != handle) {

        volumeHandle_ = handle;
        if (volumeHandle_) {

            // Resize texture of tf according to bitdepth of volume
            int bits = volumeHandle_->getVolume()->getBitsStored() / volumeHandle_->getVolume()->getNumChannels();
            if (bits > 16)
                bits = 16; // handle float data as if it was 16 bit to prevent overflow

            int max = static_cast<int>(pow(2.f, bits));
            if (dynamic_cast<TransFuncIntensity*>(value_))
                value_->resize(max);
            else if (dynamic_cast<TransFuncIntensityGradient*>(value_)) {
                // limit 2D tfs to 10 bit for reducing memory consumption
                max = std::min(max, 1024);
                value_->resize(max, max);
            }
        }

        updateWidgets();
    }
}

VolumeHandle* TransFuncProperty::getVolumeHandle() const {
    return volumeHandle_;
}

void TransFuncProperty::notifyChange() {
    executeLinks();

    // check if conditions are met and exec actions
    for (size_t j = 0; j < conditions_.size(); ++j)
        conditions_[j]->exec();

    updateWidgets();

    // invalidate owner:
    invalidateOwner();
}

void TransFuncProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    s.serialize("TransferFunction", value_);
}

void TransFuncProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    TransFunc* tf = 0;
    s.deserialize("TransferFunction", tf);
    set(tf);
}

PropertyWidget* TransFuncProperty::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

void TransFuncProperty::initialize() throw (VoreenException) {

    TemplateProperty<TransFunc*>::initialize();

    // create initial transfer function, if it has not been created during deserialization
    if (!value_) {
        set(new TransFuncIntensity());
        LGL_ERROR;
    }
}

void TransFuncProperty::deinitialize() throw (VoreenException) {
    if (value_) {
        delete value_;
        value_ = 0;
        LGL_ERROR;
    }

    TemplateProperty<TransFunc*>::deinitialize();
}

std::string TransFuncProperty::getTypeString() const {
    return "TransferFunction";
}

} // namespace voreen
