/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/property.h"
#include "voreen/core/vis/processors/propertyset.h"
#include "voreen/core/vis/transfunc/transfuncintensitykeys.h"

namespace voreen {

PropertySet::PropertySet(bool equalize)
: VolumeRaycaster(""), equalize_(equalize), name_("PropertySet")
{
    props_.clear();
    meta_ = MetaSerializer();
}

PropertySet::PropertySet(std::vector<Processor*> processors, bool equalize)
: VolumeRaycaster(""),
  equalize_(equalize), name_("PropertySet") {    
    props_.clear();
    setProcessors(processors);
    meta_ = MetaSerializer();
}

void PropertySet::setProcessors(std::vector<Processor*> processors) {
    processors_ = processors;
    createProperties();
}

void PropertySet::addProcessor(Processor* processor) {
    processors_.push_back(processor);
    createProperties();
}

bool PropertySet::removeProcessor(Processor* processor) {
    for (size_t i=0; i<processors_.size(); i++) {
        if (processors_[i] == processor) {
            processors_.erase(processors_.begin() + i);
            createProperties();
            return true;
        }
    }
    return false;
}

void PropertySet::clear() {
    props_.clear();
    processors_.clear();
}

TransFunc* PropertySet::getTransFunc() {
    for (size_t i=0; i<processors_.size(); i++) {
        if (processors_[i]->getClassName().getSubString(0) == "Raycaster") {
            VolumeRenderer* vr = dynamic_cast<VolumeRenderer*>(processors_[i]);
            if (vr) {
                return vr->getTransFunc();
                break;
            }
        }
    }
    return 0;
}

void PropertySet::createProperties() {
    props_.clear();

    // create props from first processor
    if (processors_.size() > 0) {
        std::vector<Property*> props = processors_[0]->getProperties();
        for (size_t i=0; i<props.size(); i++) {
            Property* p;

            bool found = true;
            switch(props[i]->getType()) {
                case Property::FLOAT_PROP:
                    p = new FloatProp(*static_cast<FloatProp*>(props[i]));
                    break;
                case Property::BOOL_PROP:
                    p = new BoolProp(*static_cast<BoolProp*>(props[i]));
                    break;
                case Property::INT_PROP:
                    p = new IntProp(*static_cast<IntProp*>(props[i]));
                    break;
                case Property::COLOR_PROP:
                    p = new ColorProp(*static_cast<ColorProp*>(props[i]));
                    break;
                case Property::ENUM_PROP:
                    p = new EnumProp(*static_cast<EnumProp*>(props[i]));
                    break;
                // ? ---
               /* case BUTTON_PROP:
                    p = new Property();
                    break;
                case FILEDIALOG_PROP:
                    p = new Property();
                    break;
                case CONDITION_PROP:
                    p = new Property();
                    break;
                case GROUP_PROP:
                    p = new Property();
                    break;*/
                // ---
                case Property::TRANSFUNC_PROP:
                    p = new TransFuncProp(*static_cast<TransFuncProp*>(props[i]));
                    break;
                case Property::TRANSFUNC_ALPHA_PROP:
                    p = new TransFuncAlphaProp(*static_cast<TransFuncAlphaProp*>(props[i]));
                    break;
                case Property::FLOAT_VEC2_PROP:
                    p = new FloatVec2Prop(*static_cast<FloatVec2Prop*>(props[i]));
                    break;
                case Property::FLOAT_VEC3_PROP:
                    p = new FloatVec3Prop(*static_cast<FloatVec3Prop*>(props[i]));
                    break;
                case Property::FLOAT_VEC4_PROP:
                    p = new FloatVec4Prop(*static_cast<FloatVec4Prop*>(props[i]));
                    break;
                case Property::INTEGER_VEC2_PROP:
                    p = new IntVec2Prop(*static_cast<IntVec2Prop*>(props[i]));
                    break;
                case Property::INTEGER_VEC3_PROP:
                    p = new IntVec3Prop(*static_cast<IntVec3Prop*>(props[i]));
                    break;
                case Property::INTEGER_VEC4_PROP:
                    p = new IntVec4Prop(*static_cast<IntVec4Prop*>(props[i]));
                    break;
                default:
                    found = false;
            }

            if (found)
                addProperty(p);
        }
    }
    else
        return;

    // go through processors_ and erase properties from props_ that do not exist in every single processor
    // in other words: this->props_ is the intersection of the properties from the processors_-vector
    for (size_t i=0; i<processors_.size(); i++) {
        for (size_t k=0; k<props_.size(); k++) {
            bool found = false;
            for (size_t j=0; j<processors_[i]->getProperties().size(); j++) {
                if (processors_[i]->getProperties()[j]->getIdent() == props_[k]->getIdent()) {
                    found = true;
                    break;
                }
            }
            if (!found)
                props_.erase(props_.begin() + k--);
        }
    }

    if (equalize_)
        setProperties(Identifier("all"));

}


void PropertySet::setProperties(Identifier id) {
    for (size_t i=0; i<processors_.size(); i++) {
        for (size_t k=0; k<props_.size(); k++) {
            if (id == "all" || id == props_[k]->getIdent()) {
                for (size_t j=0; j<processors_[i]->getProperties().size(); j++) {
                    if (processors_[i]->getProperties()[j]->getIdent() == props_[k]->getIdent()) {                    
                    
                        switch (props_[k]->getType()){
                            case Property::INT_PROP:
                                {
                                    IntProp* prop = dynamic_cast<IntProp*>(processors_[i]->getProperties()[j]);
                                    IntProp* prop2 = dynamic_cast<IntProp*>(props_[k]);
                                    prop->set(prop2->get());
                                    processors_[i]->postMessage(new IntMsg(prop->getIdent(), prop2->get()));
                                    break;
                                }
                            case Property::FLOAT_PROP:
                                {
                                    FloatProp* prop = dynamic_cast<FloatProp*>(processors_[i]->getProperties()[j]);
                                    FloatProp* prop2 = dynamic_cast<FloatProp*>(props_[k]);

                                    prop->set(prop2->get());
                                    processors_[i]->postMessage(new FloatMsg(prop->getIdent(), prop2->get()));  
                                    break;
                                }
                            case Property::ENUM_PROP:
                                {
                                    EnumProp* prop = dynamic_cast<EnumProp*>(processors_[i]->getProperties()[j]);
                                    EnumProp* prop2 = dynamic_cast<EnumProp*>(props_[k]);

                                    prop->set(prop2->get());  
                                    if (prop->getSendStringMsg() ) {
                                        processors_[i]->postMessage(new StringMsg(prop->getIdent(),prop2->getStrings().at(prop->get()))); 
                                    }
                                    else {
                                        processors_[i]->postMessage(new IntMsg(prop->getIdent(),prop2->get()));
                                    }
                                    break;
                                }
                            case Property::BOOL_PROP:
                                {
                                    BoolProp* prop = dynamic_cast<BoolProp*>(processors_[i]->getProperties()[j]);
                                    BoolProp* prop2 = dynamic_cast<BoolProp*>(props_[k]);

                                    prop->set(prop2->get());
                                    processors_[i]->postMessage(new BoolMsg(prop->getIdent(), prop2->get()));  
                                    break;
                                }
                            case Property::FLOAT_VEC2_PROP:
                                {
                                    FloatVec2Prop* prop = dynamic_cast<FloatVec2Prop*>(processors_[i]->getProperties()[j]);
                                    FloatVec2Prop* prop2 = dynamic_cast<FloatVec2Prop*>(props_[k]);

                                    prop->set(prop2->get());  
                                    processors_[i]->postMessage(new Vec2Msg(prop->getIdent(), prop2->get()));  
                                    break;
                                }
                            case Property::FLOAT_VEC3_PROP:
                                {
                                    FloatVec3Prop* prop = dynamic_cast<FloatVec3Prop*>(processors_[i]->getProperties()[j]);
                                    FloatVec3Prop* prop2 = dynamic_cast<FloatVec3Prop*>(props_[k]);

                                    prop->set(prop2->get());  
                                    processors_[i]->postMessage(new Vec3Msg(prop->getIdent(), prop2->get()));  
                                    break;
                                }
                            case Property::FLOAT_VEC4_PROP:
                                {
                                    FloatVec4Prop* prop = dynamic_cast<FloatVec4Prop*>(processors_[i]->getProperties()[j]);
                                    FloatVec4Prop* prop2 = dynamic_cast<FloatVec4Prop*>(props_[k]);

                                    prop->set(prop2->get()); 
                                    processors_[i]->postMessage(new Vec4Msg(prop->getIdent(), prop2->get()));  
                                    break;
                                }
                            case Property::COLOR_PROP:
                                {
                                    ColorProp* prop = dynamic_cast<ColorProp*>(processors_[i]->getProperties()[j]);
                                    ColorProp* prop2 = dynamic_cast<ColorProp*>(props_[k]);

                                    prop->set(prop2->get()); 
                                    processors_[i]->postMessage(new ColorMsg(prop->getIdent(), prop2->get()));  
                                    break;
                                }
                            case Property::INTEGER_VEC2_PROP:
                                {
                                    IntVec2Prop* prop = dynamic_cast<IntVec2Prop*>(processors_[i]->getProperties()[j]);
                                    IntVec2Prop* prop2 = dynamic_cast<IntVec2Prop*>(props_[k]);

                                    prop->set(prop2->get()); 
                                    processors_[i]->postMessage(new IVec2Msg(prop->getIdent(), prop2->get()));  
                                    break;
                                }
                            case Property::INTEGER_VEC3_PROP:
                                {
                                    IntVec3Prop* prop = dynamic_cast<IntVec3Prop*>(processors_[i]->getProperties()[j]);
                                    IntVec3Prop* prop2 = dynamic_cast<IntVec3Prop*>(props_[k]);

                                    prop->set(prop2->get()); 
                                    processors_[i]->postMessage(new IVec3Msg(prop->getIdent(), prop2->get()));  
                                    break;
                                }
                            case Property::INTEGER_VEC4_PROP:
                                {
                                    IntVec4Prop* prop = dynamic_cast<IntVec4Prop*>(processors_[i]->getProperties()[j]);
                                    IntVec4Prop* prop2 = dynamic_cast<IntVec4Prop*>(props_[k]);

                                    prop->set(prop2->get()); 
                                    processors_[i]->postMessage(new IVec4Msg(prop->getIdent(), prop2->get()));  
                                    break;
                                } 
                            default:
                                break;
                        }
                    }      
                }
            }
        }
    }
}

void PropertySet::processMessage(Message* msg , const Identifier& dest=Message::all_) {

    setProperties(msg->id_);
	// moved here from super class VolumeRenderer. Eliminate on removing old VolumeContainer
    if (msg->id_ == setCurrentDataset_) {
        int oldDataset = currentDataset_;
        msg->getValue(currentDataset_);
        if (currentDataset_ != oldDataset) {
            invalidate();
        }
    }
    else if (msg->id_ == setVolumeContainer_) {
        VolumeContainer* oldContainer = volumeContainer_;
        msg->getValue(volumeContainer_);
        if (volumeContainer_ != oldContainer) {
            invalidate();
        }
    }

	if (msg->id_ == VolumeRenderer::setTransFunc_) {
		TransFunc* transferFunction= msg->getValue<TransFunc*>();
		TransFuncIntensityKeys* tf = dynamic_cast<TransFuncIntensityKeys*>(transferFunction);
		if (tf) {
			for (size_t j=0; j<processors_.size(); j++) {
				TransFuncIntensityKeys* newTransferFunction = new TransFuncIntensityKeys();
				newTransferFunction->clearKeys();
				for (int i=0; i<tf->getNumKeys(); i++) {
					TransFuncMappingKey* key = new TransFuncMappingKey(*(tf->getKey(i)));
					newTransferFunction->addKey(key);
				}
				newTransferFunction->updateTexture();
				processors_[j]->postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc_, newTransferFunction), dest);   
			}
		}
	}
	else {
		for (size_t i=0; i<processors_.size(); i++) {
	        processors_[i]->processMessage(msg, dest);   
		}
	}
}

const std::string PropertySet::XmlElementName = "PropertySet";

std::string PropertySet::getXmlElementName() const {
    return XmlElementName;
}

TiXmlElement* PropertySet::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propertysetElem = new TiXmlElement(XmlElementName);
    // metadata
    TiXmlElement* metaElem = meta_.serializeToXml();
    propertysetElem->LinkEndChild(metaElem);
    
    return propertysetElem;
}

TiXmlElement* PropertySet::serializeToXml(const std::map<Processor*, int> idMap) const {
    TiXmlElement* propertysetElem = serializeToXml();
    // add Processors
    std::vector<Processor*> processors = getProcessors();
    for (size_t i=0; i<processors.size(); i++) {
        TiXmlElement* processor = new TiXmlElement("Processor");
        processor->SetAttribute("id", idMap.find(processors[i])->second);
        propertysetElem->LinkEndChild(processor);
    }
    return propertysetElem;
}

void PropertySet::updateFromXml(TiXmlElement* propertysetElem) {
    // meta
    TiXmlElement* metaElem = propertysetElem->FirstChildElement(meta_.getXmlElementName());
    if (metaElem)
        meta_.updateFromXml(metaElem);
    else
        throw XmlElementException("Metadata missing!");
}

void PropertySet::updateFromXml(TiXmlElement* propertysetElem, const std::map<int, Processor*> idMap) {
    updateFromXml(propertysetElem);
    
    TiXmlElement* processorElem;
    for (processorElem = propertysetElem->FirstChildElement(Processor::XmlElementName);
        processorElem;
        processorElem = processorElem->NextSiblingElement(Processor::XmlElementName))
    {
        int id;
        processorElem->QueryIntAttribute("id", &id);
        std::map<int, Processor*>::const_iterator it = idMap.find(id);
        if (it == idMap.end()) throw SerializerException("Invalid processorid in PropertySet!");
        addProcessor(it->second);
    }
}

PropertySet* PropertySet::getTmpPropSet() {
    if (!tmpPropSet_)
        tmpPropSet_ = new PropertySet(false);
    
    return tmpPropSet_;
}

PropertySet* PropertySet::tmpPropSet_ = 0;

} // namespace
