/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/condition.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/core/datastructures/transfunc/transfunc2dprimitives.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"

#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumeminmax.h"

#include "tgt/gpucapabilities.h"

namespace voreen {

const std::string TransFuncProperty::loggerCat_("voreen.TransFuncProperty");

TransFuncProperty::TransFuncProperty(const std::string& ident, const std::string& guiText, int invalidationLevel,
                             TransFuncProperty::Editors editors, bool lazyEditorInstantiation)
    : TemplateProperty<TransFunc*>(ident, guiText, 0, invalidationLevel)
    , volume_(0)
    , channel_(0)
    , editors_(editors)
    , lazyEditorInstantiation_(lazyEditorInstantiation)
    , domainFittingStrategy_(FIT_DOMAIN_INITIAL)
    , fitToDomainPending_(false)
{}

TransFuncProperty::TransFuncProperty()
    : TemplateProperty<TransFunc*>("", "", 0, Processor::INVALID_RESULT)
    , volume_(0)
    , channel_(0)
    , fitToDomainPending_(false)
{}

TransFuncProperty::~TransFuncProperty() {
/*    if (value_) {
        LWARNING(getFullyQualifiedGuiName() << " has not been deinitialized before destruction.");
    } */

    delete value_;
    value_ = 0;
}

Property* TransFuncProperty::create() const {
    return new TransFuncProperty();
}

void TransFuncProperty::reset() {
    if(value_)
        value_->reset();
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

    fitToDomainPending_ = false;

    // new tf object assigned -> check if contents are equal
    TransFunc1DKeys* tf_int = dynamic_cast<TransFunc1DKeys*>(tf);
    TransFunc1DKeys* value_int = dynamic_cast<TransFunc1DKeys*>(value_);
    TransFunc2DPrimitives* tf_grad = dynamic_cast<TransFunc2DPrimitives*>(tf);
    TransFunc2DPrimitives* value_grad = dynamic_cast<TransFunc2DPrimitives*>(value_);

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

void TransFuncProperty::setVolumeHandle(const VolumeBase* handle, size_t channel) {

    if (volume_ != handle || channel_ != channel) {

        volume_ = handle;

        fitToDomainPending_ = false;

        if (volume_) {
            if(!handle || handle->getNumChannels() <= channel) {
                LWARNING("Volume has not requested channel " << itos(channel) << ". Channel 0 will be used instead!");
                channel = 0;
            }
            channel_ = channel;

            handle->addObserver(this);

            // assign volume transfer function, if present
            if (volume_->hasMetaData("Transfunc")) {
                const TransFuncMetaData* tfMetaData = dynamic_cast<const TransFuncMetaData*>(volume_->getMetaData("Transfunc"));
                if (tfMetaData) {
                    if(channel < tfMetaData->getNumChannels()) {
                        if (tfMetaData->getTransferFunction(channel_)) {
                            //domainFittingStrategy_ = FIT_DOMAIN_NEVER;
                            set(tfMetaData->getTransferFunction(channel_)->clone());
                        }
                        else {
                            LWARNING("TransFuncMetaData has no transfer function");
                        }
                    }
                } else {
                    LWARNING("Meta data item with key 'transfunc' is not of expected type 'TransFuncMetaData'");
                }
            }

            // Resize texture of tf according to bitdepth of volume
            size_t bits = (volume_->getBytesPerVoxel() * 8) / volume_->getNumChannels();
            if (bits > 16)
                bits = 16; // handle float data as if it was 16 bit to prevent overflow

            int max = static_cast<int>(pow(2.f, (int)bits));
            if (tgt::Singleton<tgt::GpuCapabilities>::isInited())
                max = std::min(max, GpuCaps.getMaxTextureSize());

            if (TransFunc1DKeys* tfi = dynamic_cast<TransFunc1DKeys*>(value_)) {
                max = std::min(max, 4096); //< limit 1D tfs to 12 bit for reducing memory consumption
                value_->resize(max);
                //RealWorldMapping rwm = volume_->getRealWorldMapping();
                //if ((((rwm.getOffset() != 0.0f) || (rwm.getScale() != 1.0f) || (rwm.getUnit() != "")) && *tfi == TransFunc1DKeys()) || alwaysFitDomain_)
                //    fitDomainToData();
                if ( domainFittingStrategy_ == FIT_DOMAIN_ALWAYS ||
                    (domainFittingStrategy_ == FIT_DOMAIN_INITIAL && value_->getDomain() == tgt::vec2(0.f, 1.f)) )
                {
                    fitDomainToData();
                }
            }
            else if (dynamic_cast<TransFunc2DPrimitives*>(value_)) {
                max = std::min(max, 1024); //< limit 2D tfs to 10 bit for reducing memory consumption
                value_->resize(max, max);
            }

        }

        updateWidgets();
    }
}

void TransFuncProperty::fitDomainToData() {
    if (TransFunc1DKeys* tfi = dynamic_cast<TransFunc1DKeys*>(value_)) {
        //set gamma value to default
        tfi->setGammaValue(1.f);

        //set default, if no volume is present
        if(!volume_) {
            tfi->setDomain(tgt::vec2(0.f, 1.f));
            invalidate();
            return;
        }

        RealWorldMapping rwm = volume_->getRealWorldMapping();
        if (volume_->hasDerivedData<VolumeMinMax>() &&
            volume_->getDerivedData<VolumeMinMax>()->getNumChannels() > channel_) { //< if volume min/max values already computed, use them
            float min = rwm.normalizedToRealWorld(volume_->getDerivedData<VolumeMinMax>()->getMinNormalized(channel_));
            float max = rwm.normalizedToRealWorld(volume_->getDerivedData<VolumeMinMax>()->getMaxNormalized(channel_));
            tfi->setDomain(tgt::vec2(min, max));
            fitToDomainPending_ = false;
        }
        else {
            // if min/max values not available, compute them asynchronously
            // and fit to full normalized range in the mean time
            volume_->getDerivedDataThreaded<VolumeMinMax>();
            fitToDomainPending_ = true;

            float min = rwm.normalizedToRealWorld(0.f);
            float max = rwm.normalizedToRealWorld(1.f);
            tfi->setDomain(tgt::vec2(min, max));
        }
        invalidate();
    }
}

const VolumeBase* TransFuncProperty::getVolumeHandle() const {
    return volume_;
}

const size_t TransFuncProperty::getVolumeChannel() const {
    return channel_;
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

void TransFuncProperty::volumeDelete(const VolumeBase* source) {
    if (volume_ == source) {
        volume_ = 0;
        channel_ = 0;
        fitToDomainPending_ = false;
        updateWidgets();
    }
}

void TransFuncProperty::volumeChange(const VolumeBase* source) {
    if (volume_ == source) {
        fitToDomainPending_ = false;
        updateWidgets();
    }
}

void TransFuncProperty::derivedDataThreadFinished(const VolumeBase* /*source*/, const VolumeDerivedData* derivedData) {
    if (dynamic_cast<const VolumeMinMax*>(derivedData) && fitToDomainPending_)
        fitDomainToData();

    updateWidgets();
}

void TransFuncProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    s.serialize("TransferFunction", value_);
    s.serialize("domainFittingStrategy", domainFittingStrategy_);
}

void TransFuncProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    TransFunc* tf = 0;
    s.deserialize("TransferFunction", tf);
    set(tf);

    try {
        int fittingStrategy = domainFittingStrategy_;
        s.deserialize("domainFittingStrategy", fittingStrategy);
        domainFittingStrategy_ = (DomainAutoFittingStrategy)fittingStrategy;
    }
    catch (XmlSerializationNoSuchDataException&) {
        s.removeLastError();
    }
}

void TransFuncProperty::initialize() throw (tgt::Exception) {

    TemplateProperty<TransFunc*>::initialize();

    // create initial transfer function, if it has not been created during deserialization
    if (!value_) {
        set(new TransFunc1DKeys());
        LGL_ERROR;
    }
}

void TransFuncProperty::deinitialize() throw (tgt::Exception) {
    if (value_) {
        value_->deleteTexture();
        LGL_ERROR;
    }

    TemplateProperty<TransFunc*>::deinitialize();
}

void TransFuncProperty::setDomainFittingStrategy(TransFuncProperty::DomainAutoFittingStrategy strategy) {
    domainFittingStrategy_ = strategy;
}

TransFuncProperty::DomainAutoFittingStrategy TransFuncProperty::getDomainFittingStrategy() const {
    return domainFittingStrategy_;
}

} // namespace voreen
