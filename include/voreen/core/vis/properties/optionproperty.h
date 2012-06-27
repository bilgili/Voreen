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

#ifndef VRN_OPTIONPROPERTY_H
#define VRN_OPTIONPROPERTY_H

#include "voreen/core/vis/properties/templateproperty.h"
#include "voreen/core/vis/properties/condition.h"
#include "voreen/core/vis/propertywidgetfactory.h"
#include <map>

namespace voreen {

// This base class is needed for the widget to have an interface
class OptionPropertyBase : public TemplateProperty<std::string> {
public:
    OptionPropertyBase(const std::string& id, const std::string& guiText, const std::string& value,
                       bool invalidate = true, bool invalidateShader = false)
        : TemplateProperty<std::string>(id, guiText, value, invalidate, invalidateShader)
    {}

    virtual void setById(const std::string& id) = 0;
    virtual const std::string& getId() const = 0;

    virtual std::vector<std::string> getIds() const = 0;
    virtual std::map<std::string, std::string> getDescriptions() const = 0;

    virtual void updateFromXml(TiXmlElement* propElem);
    virtual std::string toString() const { return value_; }
};

// ----------------------------------------------------------------------------

template<class T>
struct Option {
    std::string id;
    std::string description;
    T value;
};

// ----------------------------------------------------------------------------

template<class T>
class OptionProperty : public OptionPropertyBase {
public:
    OptionProperty(const std::string& id, const std::string& guiText, const std::vector<Option<T> >& options,
        bool invalidate = true, bool invalidateShader = false);
    virtual ~OptionProperty() {}

    virtual void setById(const std::string& id) { set(id); }
    void setByValue(const T& value);

    virtual const std::string& getId() const { return get(); }
    const T& getValue() const;

    const std::vector<Option<T> >& getOptions() const { return options_; }
    void setOptions(const std::vector<Option<T> >& options) { options_ = options; }
    std::set<T> allowedValues() const;
    std::set<std::string> allowedIds() const;
    virtual std::vector<std::string> getIds() const;
    virtual std::map<std::string, std::string> getDescriptions() const;

    virtual TiXmlElement* serializeToXml() const;

protected:
    PropertyWidget* createWidget(PropertyWidgetFactory* f);

    std::vector<Option<T> > options_;
};

// ----------------------------------------------------------------------------

template<class T>
OptionProperty<T>::OptionProperty(const std::string& id, const std::string& guiText,
                                  const std::vector<Option<T> >& options, bool invalidate, bool invalidateShader)
    : OptionPropertyBase(id, guiText, options.begin()->id, invalidate, invalidateShader)
{
    options_ = options;
    addValidation(OptionPropertyValidation<T>(this)); // is at position 0 in the validations_ vector
}

template<class T>
void OptionProperty<T>::setByValue(const T& value) {
    // find the option that fits the value
    for (size_t i = 0; i < options_.size(); ++i) {
        if (options_[i].value == value) {
            set(options_[i].id);
            return;
        }
    }
    // if nothing was set the value was not valid
    throw Condition::ValidationFailed();
}

template<class T>
const T& OptionProperty<T>::getValue() const {
    // find the option that fits the id
    for(size_t i = 0; i < options_.size(); ++i) { // could be faster if map id -> (val, desc) is used
        if (options_[i].id == value_)
            return options_[i].value;
    }
    return options_.front().value;  // Just to silence the compiler
}

template<class T>
std::set<T> OptionProperty<T>::allowedValues() const {
    std::set<T> values;
    for (size_t i = 0; i < options_.size(); ++i)
        values.insert(options_[i].value);

    return values;
}

template<class T>
std::set<std::string> OptionProperty<T>::allowedIds() const {
    std::set<std::string> ids;
    for (size_t i = 0; i < options_.size(); ++i)
        ids.insert(options_[i].id);

    return ids;
}

template<class T>
std::vector<std::string> OptionProperty<T>::getIds() const {
    std::vector<std::string> ids;
    for (size_t i = 0; i < options_.size(); ++i)
        ids.push_back(options_[i].id);

    return ids;
}

template<class T>
std::map<std::string, std::string> OptionProperty<T>::getDescriptions() const {
    std::map<std::string, std::string> descriptions;
    for (size_t i = 0; i < options_.size(); ++i)
        descriptions[options_[i].id] = options_[i].description;

    return descriptions;
}

template<class T>
PropertyWidget* OptionProperty<T>::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

template<typename T>
TiXmlElement* OptionProperty<T>::serializeToXml() const {
    TiXmlElement* propElem = Property::serializeToXml();
    propElem->SetAttribute("value", getId());

    if (getSerializeMetaData()) {
        propElem->SetAttribute("class", "OptionProperty");
        for (size_t i = 0; i < options_.size(); ++i) {
            TiXmlElement* allowed = new TiXmlElement("allowedValue");
            allowed->SetAttribute("value", options_[i].id);
            allowed->SetAttribute("label", options_[i].description);
            propElem->LinkEndChild(allowed);
        }
    }

    return propElem;
}

} // namespace voreen

#endif // VRN_OPTIONPROPERTY_H
