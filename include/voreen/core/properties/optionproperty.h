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

#ifndef VRN_OPTIONPROPERTY_H
#define VRN_OPTIONPROPERTY_H

#include "voreen/core/properties/templateproperty.h"
#include "voreen/core/properties/condition.h"
#include "voreen/core/properties/propertywidgetfactory.h"

#include "tgt/tgt_gl.h"

#include <map>

namespace voreen {

/**
 * Non-generic base type for option properties.
 *
 * @see OptionProperty
 */
class VRN_CORE_API OptionPropertyBase : public TemplateProperty<std::string> {
public:
    OptionPropertyBase(const std::string& id, const std::string& guiText,
                       int invalidationLevel=Processor::INVALID_RESULT)
        : TemplateProperty<std::string>(id, guiText, "", invalidationLevel)
    {}

    virtual void select(const std::string& key) = 0;
    virtual void selectByKey(const std::string& key) = 0;
    virtual void selectByIndex(int index) = 0;
    virtual const std::string& getKey() const = 0;
    virtual int getSelectedIndex() const = 0;
    virtual bool isSelected(const std::string& key) const = 0;
    virtual bool hasKey(const std::string& key) const = 0;

    virtual std::vector<std::string> getKeys() const = 0;
    virtual std::vector<std::string> getDescriptions() const = 0;

    virtual std::string getOptionDescription(const std::string& key) const = 0;
    virtual void setOptionDescription(const std::string& key, const std::string& desc) = 0;
};

// ----------------------------------------------------------------------------

template<class T>
struct Option : public Serializable {
    Option(const std::string& key, const std::string& description, const T& value)
        : key_(key)
        , description_(description)
        , value_(value)
    {}
    Option() {}

    // cannot serialize value in general, must be specialized (see functions below)
    virtual void serialize(XmlSerializer& s) const {
        tgtAssert(false, "Cannot serialize option, unspecialized for this type!");
    }

    virtual void deserialize(XmlDeserializer& s) {
        tgtAssert(false, "Cannot deserialize option, unspecialized for this type!");
    }


    std::string key_;
    std::string description_;
    T value_;
};

template<> VRN_CORE_API void Option<int>::serialize(XmlSerializer& s) const;
template<> VRN_CORE_API void Option<int>::deserialize(XmlDeserializer& s);
template<> VRN_CORE_API void Option<float>::serialize(XmlSerializer& s) const;
template<> VRN_CORE_API void Option<float>::deserialize(XmlDeserializer& s);
template<> VRN_CORE_API void Option<GLenum>::serialize(XmlSerializer& s) const;
template<> VRN_CORE_API void Option<GLenum>::deserialize(XmlDeserializer& s);
template<> VRN_CORE_API void Option<std::string>::serialize(XmlSerializer& s) const;
template<> VRN_CORE_API void Option<std::string>::deserialize(XmlDeserializer& s);

// ----------------------------------------------------------------------------

/**
 * Generic option property allowing the user to select
 * one out of multiple options.
 */
template<class T>
class OptionProperty : public OptionPropertyBase {
public:
    OptionProperty(const std::string& id, const std::string& guiText,
        int invalidationLevel=Processor::INVALID_RESULT, bool serializeOptions = false);
    OptionProperty();
    virtual ~OptionProperty() {}

    virtual Property* create() const;

    virtual std::string getClassName() const       { return "OptionProperty"; }
    virtual std::string getTypeDescription() const { return "OptionProperty"; }

    virtual void addOption(const std::string& key, const std::string& description, const T& value);
    virtual bool removeOption(const std::string& key);

    virtual void select(const std::string& key);
    virtual void selectByKey(const std::string& key);
    virtual void selectByIndex(int index);
    virtual void selectByValue(const T& value);
    virtual bool isSelected(const std::string& key) const;
    virtual void reset();

    virtual const std::string& getKey() const { return get(); }
    virtual int getSelectedIndex() const;
    std::string getDescription() const;
    T getValue() const;

    virtual std::string getOptionDescription(const std::string& key) const;
    virtual void setOptionDescription(const std::string& key, const std::string& desc);

    const std::vector<Option<T> >& getOptions() const { return options_; }
    void setOptions(const std::vector<Option<T> >& options) { options_ = options; }
    virtual std::vector<std::string> getKeys() const;
    virtual bool hasKey(const std::string& key) const;
    virtual std::vector<T> getValues() const;
    virtual std::vector<std::string> getDescriptions() const;

    virtual void serialize(XmlSerializer& s) const;
    virtual void deserialize(XmlDeserializer& s);

protected:
    const Option<T>* getOption(const std::string& key) const;
    Option<T>* getOption(const std::string& key);

    std::vector<Option<T> > options_;
    // if options depend on run time information, such as incoming data, it is possible to serialize the options themselves along
    // with the currently selected value. This behaviour can be enabled in the constructor and is saved in this boolean.
    bool serializeOptions_;
};

// ----------------------------------------------------------------------------
// template implementations

template<class T>
OptionProperty<T>::OptionProperty(const std::string& id, const std::string& guiText,
                                  int invalidationLevel, bool serializeOptions)
    : OptionPropertyBase(id, guiText, invalidationLevel)
    , serializeOptions_(serializeOptions)
{
    addValidation(OptionPropertyValidation(this)); // is at position 0 in the validations_ vector
}

template<class T>
voreen::OptionProperty<T>::OptionProperty()
    : OptionPropertyBase("", "", Processor::INVALID_RESULT), serializeOptions_(false)
{}

template<class T>
Property* voreen::OptionProperty<T>::create() const {
    return new OptionProperty<T>();
}

template<class T>
void voreen::OptionProperty<T>::reset() {
    if(hasKey(defaultValue_)){
        select(defaultValue_);
    }
    else
        if(options_.size() == 0){
            setDefaultValue("");
            set("");
        } else{
            setDefaultValue(getKeys()[0]);
            set(defaultValue_);
        }
}

template<class T>
void OptionProperty<T>::addOption(const std::string& key, const std::string& description, const T& value) {
    std::vector<std::string> keys = getKeys();
    if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
        options_.push_back(Option<T>(key, description, value));
        if (options_.size() == 1){
            set(key);
            setDefaultValue(key);
        }
    }
    else {
        LERRORC("OptionProperty", "Key '" << key << "' already inserted.");
    }
}

template<class T>
bool OptionProperty<T>::removeOption(const std::string& key) {
    //option property must have at least one option
    if(options_.size() == 1) {
        return false;
    }
    //find option
    for(typename std::vector<Option<T> >::iterator it = options_.begin(); it < options_.end(); it++) {
        if(it->key_ == key){
            if(isSelected(key)) {
                //if it is default value, replace it by first option in line
                if(defaultValue_ == key) {
                    if(options_[0].key_ == key)
                        setDefaultValue(options_[1].key_);
                    else
                        setDefaultValue(options_[0].key_);
                }
                set(defaultValue_);
            }
            options_.erase(it);
            return true;
        }
    }
    //option does not exist
    return false;
}

template<class T>
void OptionProperty<T>::select(const std::string& key) {
    set(key);
}

template<class T>
void OptionProperty<T>::selectByKey(const std::string& key) {
    select(key);
}

template<class T>
void OptionProperty<T>::selectByValue(const T& value) {
    // find the option that fits the value
    for (size_t i = 0; i < options_.size(); ++i) {
        if (options_[i].value_ == value) {
            selectByKey(options_[i].key_);
            return;
        }
    }
    // if nothing was set the value was not valid
    LERROR("Unknown value selected: " << value);
    throw Condition::ValidationFailed();
}

template<class T>
void voreen::OptionProperty<T>::selectByIndex(int index)  {
    if (index >= 0 && index < options_.size()) {
        selectByKey(options_.at(index).key_);
        return;
    }
    LERROR("Invalid option index: " << index);
    throw Condition::ValidationFailed();
}

template<class T>
bool OptionProperty<T>::isSelected(const std::string& key) const {
    std::vector<std::string> keys = getKeys();
    if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
        LWARNINGC("OptionProperty", "Unknown key: " << key);
    }
    return (get() == key);
}

template<class T>
T OptionProperty<T>::getValue() const {
    if (options_.empty()) {
        LWARNINGC("OptionProperty", "OptionProperty is empty (no options)");
        throw (VoreenException("OptionProperty is empty (no options)"));
    }
    const Option<T>* curOption = getOption(get());
    if (curOption)
        return curOption->value_;
    else
        return options_.front().value_;  // Just to silence the compiler
}

template<class T>
std::string OptionProperty<T>::getOptionDescription(const std::string& key) const {
    if (options_.empty()) {
        LWARNINGC("OptionProperty", "OptionProperty is empty (no options)");
        throw (VoreenException("OptionProperty is empty (no options)"));
    }
    const Option<T>* curOption = getOption(key);
    if (curOption)
        return curOption->description_;
    else
        return "";
}

template<class T>
void OptionProperty<T>::setOptionDescription(const std::string& key, const std::string& desc) {
    if (options_.empty()) {
        LWARNINGC("OptionProperty", "OptionProperty is empty (no options)");
        throw (VoreenException("OptionProperty is empty (no options)"));
    }
    Option<T>* curOption = getOption(key);
    if (curOption)
        curOption->description_ = desc;
}

template<class T>
std::string OptionProperty<T>::getDescription() const {
    if (options_.empty()) {
        LWARNINGC("OptionProperty", "OptionProperty is empty (no options)");
        throw (VoreenException("OptionProperty is empty (no options)"));
    }
    const Option<T>* curOption = getOption(get());
    if (curOption)
        return curOption->description_;
    else
        return options_.front().description_;  // Just to silence the compiler
}

template<class T>
int OptionProperty<T>::getSelectedIndex() const {
    const Option<T>* curOption = getOption(get());
    for (size_t i=0; i<options_.size(); i++) {
        if (options_.at(i).key_ == curOption->key_)
            return (int)i;
    }

    return -1;
}

template<class T>
std::vector<std::string> OptionProperty<T>::getKeys() const {
    std::vector<std::string> keys;
    for (size_t i = 0; i < options_.size(); ++i)
        keys.push_back(options_[i].key_);

    return keys;
}

template<class T>
bool OptionProperty<T>::hasKey(const std::string& key) const {
    std::vector<std::string> keys = getKeys();
    return (std::find(keys.begin(), keys.end(), key) != keys.end());
}

template<class T>
std::vector<T> OptionProperty<T>::getValues() const {
    std::vector<T> values;
    for (size_t i = 0; i < options_.size(); ++i)
        values.push_back(options_[i].value_);

    return values;
}

template<class T>
std::vector<std::string> OptionProperty<T>::getDescriptions() const {
    std::vector<std::string> descriptions;
    for (size_t i = 0; i < options_.size(); ++i)
        descriptions.push_back(options_[i].description_);

    return descriptions;
}

template<class T>
const Option<T>* voreen::OptionProperty<T>::getOption(const std::string& key) const {
    for (size_t i=0; i<options_.size(); ++i)
        if (options_[i].key_ == key)
            return &options_[i];
    return 0;
}

template<class T>
Option<T>* voreen::OptionProperty<T>::getOption(const std::string& key) {
    for (size_t i=0; i<options_.size(); ++i)
        if (options_[i].key_ == key)
            return &options_[i];
    return 0;
}

template<typename T>
void OptionProperty<T>::serialize(XmlSerializer& s) const {
    Property::serialize(s);
    if(serializeOptions_)
        s.serialize("Options", options_, "Option");

    s.serialize("value", getKey());
}

template<typename T>
void OptionProperty<T>::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);
    if(serializeOptions_) {
        try {
            std::vector<Option<T> > tmpOptions;
            s.deserialize("Options", tmpOptions, "Option");
            options_.clear();
            options_ = tmpOptions;
        } catch(SerializationException&) {
            s.removeLastError();
        }
    }

    std::string id;
    s.deserialize("value", id);
    try {
        select(id);
    }
    catch (Condition::ValidationFailed& /*e*/) {
        s.addError("Invalid option value: " + id);
    }
}

//
// concrete types
//

class VRN_CORE_API IntOptionProperty : public OptionProperty<int> {
public:
    IntOptionProperty(const std::string& id, const std::string& guiText,
                      int invalidationLevel = Processor::INVALID_RESULT,
                      bool serializeOptions = false) :
        OptionProperty<int>(id, guiText, invalidationLevel, serializeOptions)
    {}

    IntOptionProperty() :
        OptionProperty<int>("", "", Processor::INVALID_RESULT)
    {}

    virtual Property* create() const {
        return new IntOptionProperty();
    }

    virtual std::string getClassName() const       { return "IntOptionProperty"; }
    virtual std::string getTypeDescription() const { return "IntegerOption"; }
};

class VRN_CORE_API FloatOptionProperty : public OptionProperty<float> {
public:
    FloatOptionProperty(const std::string& id, const std::string& guiText,
                        int invalidationLevel = Processor::INVALID_RESULT,
                        bool serializeOptions = false) :
        OptionProperty<float>(id, guiText, invalidationLevel, serializeOptions)
    {}

    FloatOptionProperty() :
        OptionProperty<float>("", "", Processor::INVALID_RESULT)
    {}

    virtual std::string getClassName() const       { return "FloatOptionProperty"; }
    virtual std::string getTypeDescription() const { return "FloatOption"; }
};

class VRN_CORE_API GLEnumOptionProperty : public OptionProperty<GLenum> {
public:
    GLEnumOptionProperty(const std::string& id, const std::string& guiText,
                         int invalidationLevel = Processor::INVALID_RESULT,
                         bool serializeOptions = false) :
        OptionProperty<GLenum>(id, guiText, invalidationLevel, serializeOptions)
    {}

    GLEnumOptionProperty() :
        OptionProperty<GLenum>("", "", Processor::INVALID_RESULT)
    {}

    virtual Property* create() const {
        return new GLEnumOptionProperty();
    }

    virtual std::string getClassName() const       { return "GLEnumOptionProperty"; }
    virtual std::string getTypeDescription() const { return "GLenumOption"; }
};

// since option ids are already strings, an additional value is not necessarily required for string option properties
class VRN_CORE_API StringOptionProperty : public OptionProperty<std::string> {
public:
    StringOptionProperty(const std::string& id, const std::string& guiText,
                         int invalidationLevel = Processor::INVALID_RESULT,
                         bool serializeOptions = false) :
        OptionProperty<std::string>(id, guiText, invalidationLevel, serializeOptions)
    {}

    StringOptionProperty() :
        OptionProperty<std::string>("", "", Processor::INVALID_RESULT)
    {}


    virtual Property* create() const {
        return new StringOptionProperty();
    }

    virtual std::string getClassName() const       { return "StringOptionProperty"; }
    virtual std::string getTypeDescription() const { return "StringOption"; }

    virtual void addOption(const std::string& key, const std::string& description) {
        addOption(key, description, key);
    }

    virtual void addOption(const std::string& key, const std::string& description, const std::string& value) {
        OptionProperty<std::string>::addOption(key, description, value);
    }

};

} // namespace voreen

#endif // VRN_OPTIONPROPERTY_H
