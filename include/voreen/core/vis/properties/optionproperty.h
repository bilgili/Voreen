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
#include "voreen/core/vis/properties/propertywidgetfactory.h"
#include <map>

namespace voreen {

// This base class is needed for the widget to have an interface
class OptionPropertyBase : public TemplateProperty<std::string> {
public:
    OptionPropertyBase(const std::string& id, const std::string& guiText,
                       Processor::InvalidationLevel invalidationLevel=Processor::INVALID_RESULT)
        : TemplateProperty<std::string>(id, guiText, "", invalidationLevel)
    {}

    virtual void selectByKey(const std::string& key) = 0;
    virtual const std::string& getKey() const = 0;

    virtual std::vector<std::string> getKeys() const = 0;
    virtual std::map<std::string, std::string> getDescriptions() const = 0;

    /**
     * @see Property::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    virtual std::string toString() const { return value_; }
};

// ----------------------------------------------------------------------------

template<class T>
struct Option {

    Option(const std::string& key, const std::string& description, const T& value) :
        key_(key),
        description_(description),
        value_(value)
    {}

    std::string key_;
    std::string description_;
    T value_;
};

// ----------------------------------------------------------------------------

template<class T>
class OptionProperty : public OptionPropertyBase {
public:
    OptionProperty(const std::string& id, const std::string& guiText,
        Processor::InvalidationLevel invalidationLevel=Processor::INVALID_RESULT);
    virtual ~OptionProperty() {}

    virtual void addOption(const std::string& key, const std::string& description, const T& value);

    virtual void selectByKey(const std::string& key) { set(key); }
    void selectByValue(const T& value);

    virtual const std::string& getKey() const { return get(); }
    const std::string& getDescription() const;
    const T& getValue() const;

    const std::vector<Option<T> >& getOptions() const { return options_; }
    void setOptions(const std::vector<Option<T> >& options) { options_ = options; }
    std::set<T> allowedValues() const;
    std::set<std::string> allowedKeys() const;
    virtual std::vector<std::string> getKeys() const;
    virtual std::map<std::string, std::string> getDescriptions() const;

    /**
     * @see Property::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

protected:
    PropertyWidget* createWidget(PropertyWidgetFactory* f);

    std::vector<Option<T> > options_;
};

// ----------------------------------------------------------------------------

template<class T>
OptionProperty<T>::OptionProperty(const std::string& id, const std::string& guiText,
                                  Processor::InvalidationLevel invalidationLevel)
    : OptionPropertyBase(id, guiText, invalidationLevel)
{
    addValidation(OptionPropertyValidation<T>(this)); // is at position 0 in the validations_ vector
}

template<class T>
void OptionProperty<T>::addOption(const std::string& key, const std::string& description, const T& value) {
    options_.push_back(Option<T>(key, description, value));
    if (options_.size() == 1)
        set(key);
}

template<class T>
void OptionProperty<T>::selectByValue(const T& value) {
    // find the option that fits the value
    for (size_t i = 0; i < options_.size(); ++i) {
        if (options_[i].value_ == value) {
            set(options_[i].key_);
            return;
        }
    }
    // if nothing was set the value was not valid
    throw Condition::ValidationFailed();
}

template<class T>
const std::string& OptionProperty<T>::getDescription() const {
    // find the option that fits the id
    for(size_t i = 0; i < options_.size(); ++i) { // could be faster if map id -> (val, desc) is used
        if (options_[i].key_ == value_)
            return options_[i].description_;
    }
    return options_.front().description_;  // Just to silence the compiler
}

template<class T>
const T& OptionProperty<T>::getValue() const {
    // find the option that fits the id
    for(size_t i = 0; i < options_.size(); ++i) { // could be faster if map id -> (val, desc) is used
        if (options_[i].key_ == value_)
            return options_[i].value_;
    }
    return options_.front().value_;  // Just to silence the compiler
}

template<class T>
std::set<T> OptionProperty<T>::allowedValues() const {
    std::set<T> values;
    for (size_t i = 0; i < options_.size(); ++i)
        values.insert(options_[i].value_);

    return values;
}

template<class T>
std::set<std::string> OptionProperty<T>::allowedKeys() const {
    std::set<std::string> keys;
    for (size_t i = 0; i < options_.size(); ++i)
        keys.insert(options_[i].key_);

    return keys;
}

template<class T>
std::vector<std::string> OptionProperty<T>::getKeys() const {
    std::vector<std::string> keys;
    for (size_t i = 0; i < options_.size(); ++i)
        keys.push_back(options_[i].key_);

    return keys;
}

template<class T>
std::map<std::string, std::string> OptionProperty<T>::getDescriptions() const {
    std::map<std::string, std::string> descriptions;
    for (size_t i = 0; i < options_.size(); ++i)
        descriptions[options_[i].key_] = options_[i].description_;

    return descriptions;
}

template<class T>
PropertyWidget* OptionProperty<T>::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}



template<typename T>
void OptionProperty<T>::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    s.serialize("value", getKey());
}

typedef OptionProperty<int> IntOptionProperty;
typedef OptionProperty<float> FloatOptionProperty;

// since option ids are already strings, an additional value is not necessarily required for string option properties
class StringOptionProperty : public OptionProperty<std::string> {

public:

    StringOptionProperty(const std::string& id, const std::string& guiText,
                         Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT) :
        OptionProperty<std::string>(id, guiText, invalidationLevel)
    {}

    virtual void addOption(const std::string& key, const std::string& description) {
        addOption(key, description, key);
    }

    virtual void addOption(const std::string& key, const std::string& description, const std::string& value) {
        OptionProperty<std::string>::addOption(key, description, value);
    }

};

} // namespace voreen

#endif // VRN_OPTIONPROPERTY_H
