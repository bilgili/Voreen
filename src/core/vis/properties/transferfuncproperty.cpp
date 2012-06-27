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

#include "voreen/core/vis/properties/transferfuncproperty.h"
#include "voreen/core/vis/properties/condition.h"
#include "voreen/core/vis/transfunc/transfuncfactory.h"
#include "voreen/core/vis/transfunc/transfuncintensity.h"
#include "voreen/core/vis/transfunc/transfuncintensitygradient.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"
#include "voreen/core/vis/properties/propertywidgetfactory.h"

#include "voreen/core/volume/volumehandle.h"

namespace voreen {

TransFuncProperty::TransFuncProperty(const std::string& ident, const std::string& guiText, Processor::InvalidationLevel invalidationLevel,
                             TransFuncProperty::Editors editors, bool lazyEditorInstantiation)
    : TemplateProperty<TransFunc*>(ident, guiText, 0, invalidationLevel)
    , volumeHandle_(0)
    , editors_(editors)
    , lazyEditorInstantiation_(lazyEditorInstantiation)
{
    //start with intensity transfer function
    value_ = new TransFuncIntensity();
}

TransFuncProperty::~TransFuncProperty() {
    delete value_;
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
    if (!tf)
        return;

    TransFuncIntensity* tf_int = dynamic_cast<TransFuncIntensity*>(tf);
    TransFuncIntensity* value_int = dynamic_cast<TransFuncIntensity*>(value_);
    TransFuncIntensityGradient* tf_grad = dynamic_cast<TransFuncIntensityGradient*>(tf);
    TransFuncIntensityGradient* value_grad = dynamic_cast<TransFuncIntensityGradient*>(value_);

    if (tf_int && value_int) {
        // both transfer functions are TransFuncIntensity
        if ((tf_int == value_int) && (*tf_int == *value_int)) {
            // the pointer is the same and keys are equal -> do nothing
            return;
        }
        if ((tf_int == value_int) && (*tf_int != *value_int)) {
            // the pointer is the same but keys are different -> only inform about change in tf
            notifyChange();
            return;
        }
        //last possible case: pointer is different -> update to new tf and inform about change
        delete value_;
        value_ = tf;
        notifyChange();
    }

    if (tf_grad && value_grad) {
        // both transfer functions are TransFuncIntensityGradient
        if ((tf_grad == value_grad) && (*tf_grad == *value_grad)) {
            // the pointer is the same and keys are equal -> do nothing
            return;
        }
        if ((tf_grad == value_grad) && (*tf_grad != *value_grad)) {
            // the pointer is the same but keys are different -> only inform about change in tf
            notifyChange();
        }
        //last possible case: pointer is different -> update to new tf and inform about change
        delete value_;
        value_ = tf;
        notifyChange();
    }

    if ((tf_int && value_grad) || (tf_grad && value_int)) {
        // one tf is TransFuncIntensity and the other one is TransFuncIntensityGradient
        // -> update to new tf and inform about change
        delete value_;
        value_ = tf;
        invalidateOwner(Processor::INVALID_PROGRAM);
        notifyChange();
    }
}

void TransFuncProperty::setVolumeHandle(VolumeHandle* handle) {

    if (volumeHandle_ != handle) {

        volumeHandle_ = handle;

        if (volumeHandle_) {

            // Resize texture of tf according to bitdepth of volume
            int bits = volumeHandle_->getVolume()->getBitsStored() / volumeHandle_->getVolume()->getNumChannels();
            int max = static_cast<int>(pow(2.f, bits));
            if (dynamic_cast<TransFuncIntensity*>(value_))
                value_->setTextureDimension(max, 1);
            else
                value_->setTextureDimension(max, max);
        }

        updateWidgets();
    }
}

VolumeHandle* TransFuncProperty::getVolumeHandle() const {
    return volumeHandle_;
}

Volume* TransFuncProperty::getVolume() const {
    if (volumeHandle_)
        return volumeHandle_->getVolume();
    else
        return 0;
}

void TransFuncProperty::notifyChange() {
    TransFuncIntensity* tfi = dynamic_cast<TransFuncIntensity*>(value_);

    if (tfi) {
        ChangeData changeData;
        BoxObject oldBO(tfi);
        BoxObject newBO(tfi);
        changeData.setOldValue(oldBO);
        changeData.setNewValue(newBO);

        for (std::vector<PropertyLink*>::iterator it = links_.begin(); it != links_.end(); it++)
            (*it)->onChange(changeData);
    }

    // check if conditions are met and exec actions
    for (size_t j = 0; j < conditions_.size(); ++j)
        conditions_[j]->exec();

    //updateWidgets();  // disable for now, as it kills 2D TFs. joerg

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

} // namespace voreen
