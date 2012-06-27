/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/core/animation/animatedprocessor.h"
#include "voreen/core/animation/templatepropertytimeline.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/volumeurllistproperty.h"
#include "voreen/core/properties/propertyvector.h"

#include "tgt/camera.h"

namespace voreen {

const std::string AnimatedProcessor::loggerCat_("voreen.AnimatedProcessor");

AnimatedProcessor::AnimatedProcessor(Processor* processor)
    : processor_(processor)
{
    // construction of all the propertytimelines
    const std::vector<Property*>& tempproperties = processor_->getProperties();
    for (std::vector<Property*>::const_iterator it = tempproperties.begin(); it != tempproperties.end(); it++) {
        addTemplatePropertyTimeline(*it);
    }
}

void AnimatedProcessor::addTemplatePropertyTimeline(Property* prop) {
    // search by dynamic_cast for the right type of property and create the corresponding timeline

    TemplateProperty<float>* myprop1 = dynamic_cast<TemplateProperty<float>*>(prop);
    if (myprop1 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<float>(myprop1));
        return;
    }
    TemplateProperty<int>* myprop2 = dynamic_cast<TemplateProperty<int>*>(prop);
    if (myprop2 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<int>(myprop2));
        return;
    }
    TemplateProperty<bool>* myprop3 = dynamic_cast<TemplateProperty<bool>*>(prop);
    if (myprop3 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<bool>(myprop3));
        return;
    }

    TemplateProperty<tgt::ivec2>* myprop4 = dynamic_cast<TemplateProperty<tgt::ivec2>*>(prop);
    if (myprop4 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<tgt::ivec2>(myprop4));
        return;
    }

    TemplateProperty<tgt::ivec3>* myprop5 = dynamic_cast<TemplateProperty<tgt::ivec3>*>(prop);
    if (myprop5 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<tgt::ivec3>(myprop5));
        return;
    }

    TemplateProperty<tgt::ivec4>* myprop6 = dynamic_cast<TemplateProperty<tgt::ivec4>*>(prop);
    if (myprop6 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<tgt::ivec4>(myprop6));
        return;
    }
    TemplateProperty<tgt::vec2>* myprop7 = dynamic_cast<TemplateProperty<tgt::vec2>*>(prop);
    if (myprop7 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<tgt::vec2>(myprop7));
        return;
    }
    TemplateProperty<tgt::vec3>* myprop8 = dynamic_cast<TemplateProperty<tgt::vec3>*>(prop);
    if (myprop8 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<tgt::vec3>(myprop8));
        return;
    }
    TemplateProperty<tgt::vec4>* myprop9 = dynamic_cast<TemplateProperty<tgt::vec4>*>(prop);
    if (myprop9 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<tgt::vec4>(myprop9));
        return;
    }
    TemplateProperty<tgt::Camera>* myprop10 = dynamic_cast<TemplateProperty<tgt::Camera>*>(prop);
    if (myprop10 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<tgt::Camera>(myprop10));
        return;
    }
    TemplateProperty<std::string>* myprop11 = dynamic_cast<TemplateProperty<std::string>*>(prop);
    if (myprop11 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<std::string>(myprop11));
        return;
    }
    TemplateProperty<ShaderSource>* myprop12 = dynamic_cast<TemplateProperty<ShaderSource>*>(prop);
    if (myprop12 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<ShaderSource>(myprop12));
        return;
    }
    TemplateProperty<TransFunc*>* myprop13 = dynamic_cast<TemplateProperty<TransFunc*>*>(prop);
    if (myprop13 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<TransFunc*>(myprop13));
        return;
    }

    PropertyVector* myprop14 = dynamic_cast<PropertyVector*>(prop);
    if (myprop14 !=0) {
        const std::vector<Property*>& tempproperties = myprop14->getProperties();
        for (std::vector<Property*>::const_iterator it = tempproperties.begin(); it != tempproperties.end(); it++) {
            addTemplatePropertyTimeline(*it);
        }
        return;
    }

    TemplateProperty<tgt::mat2>* myprop16 = dynamic_cast<TemplateProperty<tgt::mat2>*>(prop);
    if (myprop16 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<tgt::mat2>(myprop16));
        return;
    }

    TemplateProperty<tgt::mat3>* myprop17 = dynamic_cast<TemplateProperty<tgt::mat3>*>(prop);
    if (myprop17 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<tgt::mat3>(myprop17));
        return;
    }

    TemplateProperty<tgt::mat4>* myprop18 = dynamic_cast<TemplateProperty<tgt::mat4>*>(prop);
    if (myprop18 !=0) {
        properties_.push_back(new TemplatePropertyTimeline<tgt::mat4>(myprop18));
        return;
    }

    //LWARNING("The Property \"" << prop->getGuiName() << "\" is of unknown type and cannot be animated.");
}

AnimatedProcessor::AnimatedProcessor() {}

AnimatedProcessor::~AnimatedProcessor() {
    std::vector<PropertyTimeline*>::const_iterator it;
    for (it = properties_.begin(); it!=properties_.end(); it++)
        delete (*it);

    properties_.clear();
}

const std::vector<PropertyTimeline*>& AnimatedProcessor::getPropertyTimelines() const {
    return properties_;
}

void AnimatedProcessor::renderAt(float time) {
    time = floor(time * 10000.f) / 10000.f;
    std::vector<PropertyTimeline*>::const_iterator it;
    for (it = properties_.begin(); it!=properties_.end(); it++) {
        try {
            (*it)->renderAt(time);
        }
        catch (const std::exception& e) {
            LERROR("renderAt(): " << e.what() << " (" << getProcessorName() << ")");
        }
    }
}

const std::string AnimatedProcessor::getProcessorName() const {
    return processor_->getName();
}

const Processor* AnimatedProcessor::getCorrespondingProcessor() const {
    return processor_;
}

void AnimatedProcessor::serialize(XmlSerializer& s) const {
    s.serialize("processor", processor_);
    s.serialize("properties", properties_, "Property");
}

void AnimatedProcessor::deserialize(XmlDeserializer& s) {
    s.deserialize("processor", processor_);
    s.deserialize("properties", properties_, "Property");

    // We need to remove timelines for properties that do not exist (any more)
    std::vector<PropertyTimeline*>::iterator it = properties_.begin();
    while (it != properties_.end()) {
        if ((*it)->getProperty() == 0) {
            delete *it;
            it = properties_.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace voreen
