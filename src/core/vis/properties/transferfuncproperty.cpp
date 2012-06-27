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
#include "voreen/core/vis/transfunc/transfuncintensity.h"
#include "voreen/core/vis/transfunc/transfuncintensitygradient.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"
#include "voreen/core/vis/propertywidgetfactory.h"

#include "voreen/core/volume/volumehandle.h"
#include "voreen/core/volume/volumeseries.h"
#include "voreen/core/volume/volumeset.h"
#include "voreen/core/volume/volumesetcontainer.h"

namespace voreen {

TransFuncProp::TransFuncProp(const Identifier& ident, const std::string& guiText, bool invalidate, bool invalidateShader, 
                             TransFuncProp::Editors editors, bool lazyEditorInstantiation)
    : TemplateProperty<TransFunc*>(ident.getName(), guiText, 0, invalidate, invalidateShader)
    , volumeHandle_(0)
    , editors_(editors)
    , manualRepaint_(false)
    , lazyEditorInstantiation_(lazyEditorInstantiation)
{
    //start with intensity transfer function
    value_ = new TransFuncIntensity();
}

TransFuncProp::~TransFuncProp() {
    delete value_;
}

void TransFuncProp::enableEditor(TransFuncProp::Editors editor) {
    editors_ |= editor;
}

void TransFuncProp::disableEditor(TransFuncProp::Editors editor) {
    editors_ &= ~editor;
}

bool TransFuncProp::isEditorEnabled(TransFuncProp::Editors editor) {
    return (editors_ & editor);
}

void TransFuncProp::setManualRepaint(bool manual) {
    manualRepaint_ = manual;
}

bool TransFuncProp::getManualRepaint() {
    return manualRepaint_;
}

void TransFuncProp::set(TransFunc* tf) {
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
        // temporary action for invalidation of shader (needed to update the header of the shader)
        InvalidateOwnerShaderAction shaderInvalidation(this);
        shaderInvalidation.exec();
        notifyChange();
    }
}

void TransFuncProp::setVolumeHandle(VolumeHandle* handle) {
    if ((!volumeHandle_) || (volumeHandle_ && !volumeHandle_->isIdentical(handle))) {
        clearObserverds();

        volumeHandle_ = handle;

        if (volumeHandle_) {
            // Add observer so we get notified when the volume is about to be deleted
            if (handle->getParentSeries() && handle->getParentSeries()->getParentSet() &&
                handle->getParentSeries()->getParentSet()->getParentContainer())
            {
                addObserved(handle->getParentSeries()->getParentSet()->getParentContainer());
            }
        
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

VolumeHandle* TransFuncProp::getVolumeHandle() const {
    return volumeHandle_;
}

Volume* TransFuncProp::getVolume() const {
    if (volumeHandle_)
        return volumeHandle_->getVolume();
    else
        return 0;
}

void TransFuncProp::notify(const Observable* const /*source*/) {
    // Gets called when our VolumeSet is deleted
    if (volumeHandle_) {
        if (!volumeHandle_->getParentSeries() ||
            !volumeHandle_->getParentSeries()->getParentSet() ||
            !volumeHandle_->getParentSeries()->getParentSet()->getParentContainer())
        {
            // When it has no parent any more it was just removed from its series and is about
            // to be deleted, so remove it here, too.
            setVolumeHandle(0);
        }
    }
}

void TransFuncProp::notifyChange() {    
    // check if conditions are met and exec actions
    for (size_t j = 0; j < conditions_.size(); ++j)
        conditions_[j]->exec();

    //updateWidgets();  // disable for now, as it kills 2D TFs. joerg
}

void TransFuncProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);

    std::string type = "TransFuncIntensity";
    // try to read type of transfer function
    if (propElem->Attribute("tfType") != 0) {
        type = propElem->Attribute("tfType");
    }
    if (type == "TransFuncIntensity") {
        TransFuncIntensity* newTf = new TransFuncIntensity();
        TiXmlElement* keys = propElem->FirstChildElement("Keys");
        if (keys) {
            newTf->loadKeys(keys);
            newTf->loadThreshold(propElem);
        }
        else {
            // network file is outdated
            // but for convenience we try to read tf from previous format
            newTf->clearKeys();
            TiXmlElement* markerElem;
            for (markerElem = propElem->FirstChildElement("Marker");
                 markerElem;
                 markerElem = markerElem->NextSiblingElement("Marker"))
            {
                //first get the color
                float value, dest;
                tgt::col4 color;
                tgt::ivec4 tmp;
                if (markerElem->QueryFloatAttribute("source", &value) == TIXML_SUCCESS &&
                    markerElem->QueryFloatAttribute("dest", &dest) == TIXML_SUCCESS &&
                    markerElem->QueryIntAttribute("r", &tmp.r) == TIXML_SUCCESS &&
                    markerElem->QueryIntAttribute("g", &tmp.g) == TIXML_SUCCESS &&
                    markerElem->QueryIntAttribute("b", &tmp.b) == TIXML_SUCCESS &&
                    markerElem->QueryIntAttribute("a", &tmp.a) == TIXML_SUCCESS)
                {
                    color.r = static_cast<uint8_t>(tmp.r);
                    color.g = static_cast<uint8_t>(tmp.g);
                    color.b = static_cast<uint8_t>(tmp.b);
                    color.a = static_cast<uint8_t>(tmp.a);
                    TransFuncMappingKey* myKey = new TransFuncMappingKey(value, color);
                    myKey->setAlphaL(dest);
                    if (markerElem->QueryFloatAttribute("splitdest", &dest) == TIXML_SUCCESS &&
                        markerElem->QueryIntAttribute("splitr", &tmp.r) == TIXML_SUCCESS &&
                        markerElem->QueryIntAttribute("splitg", &tmp.g) == TIXML_SUCCESS &&
                        markerElem->QueryIntAttribute("splitb", &tmp.b) == TIXML_SUCCESS &&
                        markerElem->QueryIntAttribute("splita", &tmp.a) == TIXML_SUCCESS)
                    {
                        myKey->setSplit(true);
                        color.r = static_cast<uint8_t>(tmp.r);
                        color.g = static_cast<uint8_t>(tmp.g);
                        color.b = static_cast<uint8_t>(tmp.b);
                        color.a = static_cast<uint8_t>(tmp.a);
                        myKey->setColorR(color);
                        myKey->setAlphaR(dest);
                    }
                    else
                        myKey->setSplit(false);

                    newTf->addKey(myKey);
                }
                else
                    errors_.store(XmlElementException("A key for TransFuncIntensity could not be read correctly."));
            }
        }
        // set new transfer function
        set(newTf);
    }
    else if (type == "TransFuncIntensityGradient") {
        TransFuncIntensityGradient* newTf = new TransFuncIntensityGradient();

        TiXmlElement* elem = propElem->FirstChildElement("Primitives");
        if (!elem)
            errors_.store(XmlElementException("Primitive child not found. No primitives loaded."));
        else
            newTf->loadPrimitivesFromXml(elem);

        newTf->textureUpdateNeeded();
        set(newTf);
    }
}

TiXmlElement* TransFuncProp::serializeToXml() const {
    TiXmlElement* root = Property::serializeToXml();

    if (getSerializeTypeInformation())
        root->SetAttribute("class", "TransFuncProp");

    if (typeid(*value_) == typeid(TransFuncIntensity)) {
        // serialize type
        root->SetAttribute("tfType", "TransFuncIntensity");

        TransFuncIntensity* tf = static_cast<TransFuncIntensity*>(value_);
        // serialize keys
        TiXmlElement* keys = new TiXmlElement("Keys");
        tf->saveKeys(keys);
        root->LinkEndChild(keys);
        // serialize thresholds
        tf->saveThreshold(root);
    }
    else if (typeid(*value_) == typeid(TransFuncIntensityGradient)) {
        // serialize type
        root->SetAttribute("tfType", "TransFuncIntensityGradient");
        TransFuncIntensityGradient* tf = static_cast<TransFuncIntensityGradient*>(value_);

        // serialize primitives
        TiXmlElement* primitives = new TiXmlElement("Primitives");
        tf->savePrimitivesToXml(primitives);

        root->LinkEndChild(primitives);
    }

    return root;
}

PropertyWidget* TransFuncProp::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

} // namespace voreen
